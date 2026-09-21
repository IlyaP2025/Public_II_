/**
 * demo.c - Управление роботом в Webots (аналог demo.py на C11)
 * Компиляция: gcc -std=c11 -D_POSIX_C_SOURCE=199309L -Wall -Wextra -pedantic
 * demo.c -lm -o demo
 */

#include "robot.h"
#include "init_robot.h"

static int sock_cmd = -1;
static int sock_tel = -1;

// Функции работы с сокетами
static int* getSockCmd() {
  return &sock_cmd;
}

int get_telemetry_socket() {
  return sock_tel;
}

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

  close(sock);  // закрываем слушающий сокет
  printf("[client] connected to telemetry\n");

  return client_sock;
}

void send_command(float v, float w) {
  if (*getSockCmd() < 0) return;
  
  float packet[2] = {v, w};
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(CMD_PORT_DEFAULT);
  inet_pton(AF_INET, CMD_HOST_DEFAULT, &addr.sin_addr);

  sendto(*getSockCmd(), packet, sizeof(packet), 0, (struct sockaddr*)&addr,
         sizeof(addr));
}

// === ФУНКЦИИ РАБОТЫ С ТЕЛЕМЕТРИЕЙ ===
int recv_all(int sock, void* buffer, size_t size) {
  size_t total_received = 0;
  char* buf_ptr = (char*)buffer;

  while (total_received < size) {
    ssize_t received =
        recv(sock, buf_ptr + total_received, size - total_received, 0);
    if (received <= 0) {
      return -1;
    }
    total_received += received;
  }

  return 0;
}

Telemetry* recv_telemetry(int sock, const char* proto) {
  static Telemetry tel;
  static float ranges_buffer[1024];  // буфер для измерений лидара

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
      if (n > 0 && n < sizeof(ranges_buffer) / sizeof(float)) {
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
  if (n > 0 && n < sizeof(ranges_buffer) / sizeof(float)) {
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
void update_robot_state(const Telemetry* tel) {
    // Сохраняем старую позицию ДО обновления
    float old_x = getState()->last_pos.x;
    float old_y = getState()->last_pos.y;
    
    // Обновление позиции
    getState()->last_pos.x = tel->odom_x;
    getState()->last_pos.y = tel->odom_y;
    getState()->last_pos.th = tel->odom_th;

    // Обновление времени движения (для антизастревания)
    // Используем старые значения для вычисления разницы
    float dx = tel->odom_x - old_x;
    float dy = tel->odom_y - old_y;
    float dist_moved = sqrtf(dx * dx + dy * dy);

    if (dist_moved > 0.02f) {
        getState()->last_move_time = get_current_time();
    }
}

void print_status(const Telemetry* tel, const char* action) {
  float th_deg = tel->odom_th * 180.0f / M_PI;

  printf("[client] state=%s pos=(%.2f,%.2f,θ=%.1f°) ",
         state_to_string(getState()->state), tel->odom_x, tel->odom_y, th_deg);

  printf("vel=(%.2f,%.2f,%.2f) gyro=(%.2f,%.2f,%.2f) ", tel->velocity.x,
         tel->velocity.y, tel->velocity.z, tel->gyro.x, tel->gyro.y,
         tel->gyro.z);

  if (tel->ranges_count > 0) {
    size_t mid_idx = tel->ranges_count / 2;
    size_t right_idx = (size_t)(tel->ranges_count * 3.0f / 4.0f);
    size_t left_idx = (size_t)(tel->ranges_count * 1.0f / 4.0f);

    printf("front=%.2f right=%.2f left=%.2f → %s\n", tel->ranges[mid_idx],
           tel->ranges[right_idx], tel->ranges[left_idx], action);
  } else {
    printf("no lidar %s\n", action);
  }
}

// === ИНИЦИАЛИЗАЦИЯ И ОЧИСТКА ===
void robot_init(void) {
  printf("=== Robot Controller C11 ===\n");

  // Инициализация времени
  getState()->start_time = get_current_time();
  getState()->last_move_time = getState()->start_time;

  // Инициализация PID-регулятора для скорости
  pid_init(&getState()->speed_pid, PID_KP, PID_KI, PID_KD, 
             PID_TARGET_DIST, 0.0f, FORWARD_SPEED);


  // Создание сокета команд
  *getSockCmd() = socket(AF_INET, SOCK_DGRAM, 0);
  if (*getSockCmd() < 0) {
    perror("socket command");
    return;
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
    close(*getSockCmd());
    return;
  }

  printf("Robot initialization complete\n");
}

void robot_cleanup(void) {
  printf("Cleaning up robot resources...\n");
  
  if (sock_tel >= 0) {
    close(sock_tel);
    sock_tel = -1;
  }
  
  if (*getSockCmd() >= 0) {
    close(*getSockCmd());
    *getSockCmd() = -1;
  }
  
  printf("Robot cleanup complete\n");
}
