#include "robot.h"
#include "logic.h"
#include "lidar.h"
#include <time.h>

static void delay_ms(int ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

int main() {
    // Инициализация робота
    robot_init();
    
    // Инициализация лидара
    init_lidar_degrees();
    
    // Основной цикл управления
    printf("Starting control loop with PID...\n");
    
    int running = 1;
    while (running) {
        // Получение телеметрии
        Telemetry* tel = recv_telemetry(get_telemetry_socket(), PROTO_DEFAULT);
        if (!tel) {
            delay_ms(10);
            continue;
        }

        // Пропускаем кадры без данных лидара
        if (tel->ranges_count == 0) {
            delay_ms(1);
            continue;
        }

        // Обновление состояния и принятие решения
        update_robot_state(tel);
        const char* action = decide_action(tel);
        print_status(tel, action);

        // Небольшая задержка для снижения нагрузки на CPU
        delay_ms(20);
    }

    // Очистка ресурсов
    robot_cleanup();
    return 0;
}
