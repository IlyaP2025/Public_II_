/**
 * demo.c - Управление роботом в Webots (аналог demo.py на C11)
 * Компиляция: cc -std=c11 -D_POSIX_C_SOURCE=199309L -Wall -Wextra -pedantic demo.c -lm -o demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// === КОНСТАНТЫ И ПАРАМЕТРЫ ===
#define M_PI 3.14159265358979323846
#define SAFE_DIST 0.5f
#define CRASH_DIST 0.3f
#define RECOVER_TIME 0.5f
#define TURN_TIME 0.5f
#define FORWARD_SPEED 0.6f

#define CMD_HOST_DEFAULT "127.0.0.1"
#define CMD_PORT_DEFAULT 5555
#define TEL_HOST_DEFAULT "0.0.0.0"
#define TEL_PORT_DEFAULT 5600
#define PROTO_DEFAULT "tcp"

// === СТРУКТУРЫ ДАННЫХ ===
typedef struct {
    float x, y, th;
} Position;

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float odom_x, odom_y, odom_th;
    Vector3 velocity;
    Vector3 gyro;
    float* ranges;
    size_t ranges_count;
} Telemetry;

typedef struct {
    char state[32];
    double state_until;
    Position last_pos;
    double start_time;
    double last_move_time;
} RobotState;

// === ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ===
int sock_cmd;
int sock_tel;
RobotState robot_state = {"FORWARD", 0.0, {0, 0, 0}, 0.0, 0.0};

// === ФУНКЦИИ РАБОТЫ С СЕТЬЮ ===
int create_udp_socket(const char* host, int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket UDP");
        return -1;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind UDP");
        close(sock);
        return -1;
    }
    
    return sock;
}

int create_tcp_server(const char* host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket TCP");
        return -1;
    }
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind TCP");
        close(sock);
        return -1;
    }
    
    if (listen(sock, 1) < 0) {
        perror("listen TCP");
        close(sock);
        return -1;
    }
    
    printf("[client] waiting for telemetry TCP on %s:%d...\n", host, port);
    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("accept TCP");
        close(sock);
        return -1;
    }
    
    close(sock); // закрываем слушающий сокет
    printf("[client] connected to telemetry\n");
    
    return client_sock;
}

void send_command(float v, float w) {
    float packet[2] = {v, w};
    struct sockaddr_in addr;
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CMD_PORT_DEFAULT);
    inet_pton(AF_INET, CMD_HOST_DEFAULT, &addr.sin_addr);
    
    sendto(sock_cmd, packet, sizeof(packet), 0, 
           (struct sockaddr*)&addr, sizeof(addr));
}

// === ФУНКЦИИ РАБОТЫ С ТЕЛЕМЕТРИЕЙ ===
int recv_all(int sock, void* buffer, size_t size) {
    size_t total_received = 0;
    char* buf_ptr = (char*)buffer;
    
    while (total_received < size) {
        ssize_t received = recv(sock, buf_ptr + total_received, 
                               size - total_received, 0);
        if (received <= 0) {
            return -1;
        }
        total_received += received;
    }
    
    return 0;
}

Telemetry* recv_telemetry(int sock, const char* proto) {
    static Telemetry tel;
    static float ranges_buffer[1024]; // буфер для измерений лидара
    
    uint8_t header[4];
    ssize_t received;
    
    if (strcmp(proto, "udp") == 0) {
        struct sockaddr_in src_addr;
        socklen_t addr_len = sizeof(src_addr);
        
        received = recvfrom(sock, header, sizeof(header), MSG_PEEK,
                           (struct sockaddr*)&src_addr, &addr_len);
        if (received != sizeof(header)) {
            return NULL;
        }
    } else {
        // TCP: сначала читаем размер пакета
        uint32_t pkt_size;
        if (recv_all(sock, &pkt_size, sizeof(pkt_size))) {
            return NULL;
        }
        
        // Читаем весь пакет
        uint8_t* packet = malloc(pkt_size);
        if (!packet) return NULL;
        
        if (recv_all(sock, packet, pkt_size)) {
            free(packet);
            return NULL;
        }
        
        memcpy(header, packet, sizeof(header));
        
        // Проверка заголовка
        if (memcmp(header, "WBTG", 4) != 0) {
            printf("WBT2 -> WBTG\n");
            free(packet);
            return NULL;
        }
        
        // Распаковка телеметрии
        if (pkt_size >= 40) {
            memcpy(&tel.odom_x, packet + 4, 9 * sizeof(float));
            
            uint32_t n;
            memcpy(&n, packet + 40, sizeof(n));
            
            tel.ranges_count = n;
            if (n > 0 && n < sizeof(ranges_buffer)/sizeof(float)) {
                memcpy(ranges_buffer, packet + 44, n * sizeof(float));
                tel.ranges = ranges_buffer;
            } else {
                tel.ranges = NULL;
                tel.ranges_count = 0;
            }
        }
        
        free(packet);
        return &tel;
    }
    
    // UDP реализация (упрощенная)
    if (memcmp(header, "WBTG", 4) != 0) {
        return NULL;
    }
    
    // Чтение основной телеметрии (9 float)
    float telemetry_data[9];
    if (recv_all(sock, telemetry_data, sizeof(telemetry_data))) {
        return NULL;
    }
    
    tel.odom_x = telemetry_data[0];
    tel.odom_y = telemetry_data[1];
    tel.odom_th = telemetry_data[2];
    tel.velocity.x = telemetry_data[3];
    tel.velocity.y = telemetry_data[4];
    tel.velocity.z = telemetry_data[5];
    tel.gyro.x = telemetry_data[6];
    tel.gyro.y = telemetry_data[7];
    tel.gyro.z = telemetry_data[8];
    
    // Чтение данных лидара
    uint32_t n;
    if (recv_all(sock, &n, sizeof(n))) {
        return NULL;
    }
    
    tel.ranges_count = n;
    if (n > 0 && n < sizeof(ranges_buffer)/sizeof(float)) {
        if (recv_all(sock, ranges_buffer, n * sizeof(float))) {
            return NULL;
        }
        tel.ranges = ranges_buffer;
    } else {
        tel.ranges = NULL;
        tel.ranges_count = 0;
    }
    
    return &tel;
}

// === ФУНКЦИИ УПРАВЛЕНИЯ РОБОТОМ ===
double get_current_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void update_robot_state(const Telemetry* tel) {
    double now = get_current_time();
    
    // Проверка движения (анти-застревание)
    if (robot_state.last_pos.x != 0 || robot_state.last_pos.y != 0) {
        float dx = tel->odom_x - robot_state.last_pos.x;
        float dy = tel->odom_y - robot_state.last_pos.y;
        float dist_moved = sqrtf(dx*dx + dy*dy);
        
        if (dist_moved > 0.02f) {
            robot_state.last_move_time = now;
        }
    }
    
    robot_state.last_pos.x = tel->odom_x;
    robot_state.last_pos.y = tel->odom_y;
    robot_state.last_pos.th = tel->odom_th;
}

const char* decide_action(const Telemetry* tel) {
    double now = get_current_time();
    
    // Если состояние еще активно - продолжаем
    if (now < robot_state.state_until) {
        return robot_state.state;
    }
    
    // Анализ данных лидара
    if (tel->ranges_count == 0) {
        return "NO_LIDAR_DATA";
    }
    
    size_t n = tel->ranges_count;
    size_t mid_idx = n / 2;
    size_t right_idx = (size_t)(n * 3.0f / 4.0f);
    size_t left_idx = (size_t)(n * 1.0f / 4.0f);
    
    float front_dist = tel->ranges[mid_idx];
    float right_dist = tel->ranges[right_idx];
    float left_dist = tel->ranges[left_idx];
    
    // Аварийная проверка столкновения
    if (front_dist < CRASH_DIST || right_dist < CRASH_DIST || left_dist < CRASH_DIST) {
        strcpy(robot_state.state, "RECOVER");
        robot_state.state_until = now + RECOVER_TIME;
        
        float turn_dir = (right_dist > left_dist) ? 1.0f : -1.0f;
        send_command(-0.15f, 1.0f * turn_dir);
        
        static char action[64];
        snprintf(action, sizeof(action), "RECOVER(dir=%.1f)", turn_dir);
        return action;
    }
    // Свободно вперед - движемся вперед
    else if (front_dist > SAFE_DIST) {
        float v = fminf(FORWARD_SPEED, 0.1f + 0.5f * front_dist);
        strcpy(robot_state.state, "FORWARD");
        robot_state.state_until = now + 0.3;
        send_command(v, 0.0f);
        
        static char action[64];
        snprintf(action, sizeof(action), "FORWARD(v=%.2f)", v);
        return action;
    }
    // Свободно справа - поворачиваем направо
    else if (right_dist > SAFE_DIST) {
        strcpy(robot_state.state, "TURN_RIGHT");
        robot_state.state_until = now + TURN_TIME;
        send_command(0.15f, -1.0f);
        return "TURN_RIGHT";
    }
    // Иначе - поворачиваем налево
    else {
        strcpy(robot_state.state, "TURN_LEFT");
        robot_state.state_until = now + TURN_TIME;
        send_command(0.15f, 1.0f);
        return "TURN_LEFT";
    }
}

void print_status(const Telemetry* tel, const char* action) {
    float th_deg = tel->odom_th * 180.0f / M_PI;
    
    printf("[client] state=%s pos=(%.2f,%.2f,θ=%.1f°) ",
           robot_state.state, tel->odom_x, tel->odom_y, th_deg);
    
    printf("vel=(%.2f,%.2f,%.2f) gyro=(%.2f,%.2f,%.2f) ",
           tel->velocity.x, tel->velocity.y, tel->velocity.z,
           tel->gyro.x, tel->gyro.y, tel->gyro.z);
    
    if (tel->ranges_count > 0) {
        size_t mid_idx = tel->ranges_count / 2;
        size_t right_idx = (size_t)(tel->ranges_count * 3.0f / 4.0f);
        size_t left_idx = (size_t)(tel->ranges_count * 1.0f / 4.0f);
        
        printf("front=%.2f right=%.2f left=%.2f → %s\n",
               tel->ranges[mid_idx], tel->ranges[right_idx], 
               tel->ranges[left_idx], action);
    } else {
        printf("no lidar → %s\n", action);
    }
}

// === ОСНОВНАЯ ФУНКЦИЯ ===
int main() {
    printf("=== Robot Controller C11 ===\n");
    
    // Инициализация времени
    robot_state.start_time = get_current_time();
    robot_state.last_move_time = robot_state.start_time;
    
    // Создание сокета команд
    sock_cmd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_cmd < 0) {
        perror("socket command");
        return 1;
    }
    
    // Подключение телеметрии
    const char* proto = PROTO_DEFAULT;
    
    if (strcmp(proto, "udp") == 0) {
        sock_tel = create_udp_socket(TEL_HOST_DEFAULT, TEL_PORT_DEFAULT);
    } else {
        sock_tel = create_tcp_server(TEL_HOST_DEFAULT, TEL_PORT_DEFAULT);
    }
    
    if (sock_tel < 0) {
        fprintf(stderr, "Failed to create telemetry socket\n");
        close(sock_cmd);
        return 1;
    }
    
    // Основной цикл управления
    printf("Starting control loop...\n");
    
    while (1) {
        Telemetry* tel = recv_telemetry(sock_tel, proto);
        if (!tel) {
            sleep(1); // 10ms
            continue;
        }
        
        if (tel->ranges_count == 0) {
            continue;
        }
        
        update_robot_state(tel);
        const char* action = decide_action(tel);
        print_status(tel, action);
        
        // usleep(100000); // 100ms (закомментировано как в оригинале)
    }
    
    // Очистка (обычно сюда не попадаем из-за бесконечного цикла)
    close(sock_tel);
    close(sock_cmd);
    
    return 0;
}
