#ifndef STATES_H
#define STATES_H

#include "state_types.h"
#include "init_robot.h"
#include <stddef.h>    // для NULL

// Простые функции состояний
char* state_move_forward(float available_distance);     // Движение в перед
char* state_move_back(void);                            // Движение назад
char* state_turn_left(void);                            // Поворот на лево 
char* state_turn_right(void);                           // Поворот на право
char* state_arc_turn_left(void);                        // Плавный поворот налево с движением вперед
char* state_arc_turn_right(void);                       // Плавный поворот направо с движением вперед

// Непрерывные функции с PID-регулятором
char* state_move_forward_pid(float front_distance);     // Движение вперед с PID
char* state_move_continuous(float v, float w);          // Непрерывное движение

// Вспомогательные функции
float adaptive_forward_time(float available_distance);  // Выбор временной задержки от ростояния до препядствия
float calculate_pid_speed(float distance);              // Вычисление скорости с помощью PID

// char* state_handle_stuck(void);                       // Антизастревание - чередование действий
//char* state_idle(void);
//char* state_recover(float left_dist, float right_dist);

#endif
