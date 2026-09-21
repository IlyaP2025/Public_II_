#include "state_types.h"
#include <time.h>

// Реализация глобального состояния робота
RobotState* getState(void) {
    static RobotState robot_state = {
        .state = FORWARD,
        .state_until = 0.0,
        .last_pos = {0, 0, 0},
        .start_time = 0.0,
        .last_move_time = 0.0,
        .speed_pid = {
            .kp = 0,
            .ki = 0,
            .kd = 0,
            .target = 0,
            .integral = 0,
            .prev_error = 0,
            .output_min = 0,
            .output_max = 0,
            .last_time = 0     
        }
    };
    return &robot_state;
}

// Функция получения текущего времени в миллисекундах
unsigned long get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Функция получения текущего времени сукунду
double get_current_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Функция преобразования состояния в строку
const char* state_to_string(State state) {
    switch(state) {
        case FORWARD: return "FORWARD";
        case BACK: return "BACK";
        case LEFT: return "LEFT";
        case RIGHT: return "RIGHT";
        case ARC_LEFT: return "ARC_LEFT";
        case ARC_RIGHT: return "ARC_RIGHT";
        default: return "UNKNOWN";
    }
}
