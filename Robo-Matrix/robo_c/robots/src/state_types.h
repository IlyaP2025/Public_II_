#ifndef STATE_TYPES_H
#define STATE_TYPES_H

#include "pid.h"

// Типы состояний робота
typedef enum {
    FORWARD,
    BACK, 
    LEFT,
    RIGHT,
    ARC_LEFT,
    ARC_RIGHT
} State;

// Позиция робота
typedef struct {
    float x, y, th;
} Position;

// Состояние робота
typedef struct {
    State state;
    double state_until;
    Position last_pos;
    double start_time;
    double last_move_time;
    PIDController speed_pid;  // PID для управления скоростью
} RobotState;

// Глобальные функции для работы с состоянием
RobotState* getState(void); 
unsigned long get_current_time_ms(void);    // Получение времени в мили.сек
double get_current_time(void);              // Получение времени в сек.
const char* state_to_string(State state);

#endif
