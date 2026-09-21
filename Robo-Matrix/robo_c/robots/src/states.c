#include "states.h"
#include <math.h>      // для fminf
#include <stdio.h>     // для snprintf
#include <stdlib.h>    // для rand, NULL
#include "robot.h"     // для send_comman

// Движение в перед
char* state_move_forward(float available_distance) {
    float v = fminf(FORWARD_SPEED, 0.1f + 0.5f * available_distance);
    getState()->state = FORWARD;
    send_command(v, 0.0f);
    
    static char action_str[32];
    snprintf(action_str, sizeof(action_str), "FORWARD(v=%.2f)", v);
    return action_str;
}

// Движение назад
char* state_move_back(void) {
    getState()->state = BACK;
    send_command(BACK_SPEED, 0.0f);

    static char action_str[32];
    snprintf(action_str, sizeof(action_str), "BACK");
    return action_str;
}

// Поворот на лево
char* state_turn_left(void) {
    getState()->state = LEFT;
    send_command(0.0f, TURN_SPEED);

    static char action_str[32];
    snprintf(action_str, sizeof(action_str), "LEFT");
    return action_str;
}

// Поворот на право
char* state_turn_right(void) {
    getState()->state = RIGHT;
    send_command(0.0f, -TURN_SPEED);

    static char action_str[32];
    snprintf(action_str, sizeof(action_str), "RIGHT");
    return action_str;
}

// Плавный поворот налево с движением вперед
char* state_arc_turn_left(void) {
    getState()->state = ARC_LEFT;
    send_command(FORWARD_SPEED * 0.3f, TURN_SPEED * 0.7f);  // 30% вперед, 70% поворот
    
    static char action_str[32];
    snprintf(action_str, sizeof(action_str), "ARC_LEFT");
    return action_str;
}

// Плавный поворот направо с движением вперед  
char* state_arc_turn_right(void) {
    getState()->state = ARC_RIGHT;
    send_command(FORWARD_SPEED * 0.45f, -TURN_SPEED * 0.55f);  // 30% вперед, 70% поворот
   
    getState()->state_until =  get_current_time() + TURN_TIME;
 
    static char action_str[32];
    snprintf(action_str, sizeof(action_str), "ARC_RIGHT");
    return action_str;
}

// Движение вперед с PID-регулятором
char* state_move_forward_pid(float front_distance) {
    float speed = calculate_pid_speed(front_distance);
    getState()->state = FORWARD;
    send_command(speed, 0.0f);
    
    static char action_str[64];
    snprintf(action_str, sizeof(action_str), "FORWARD_PID(dist=%.2f->v=%.2f)", 
             front_distance, speed);
    return action_str;
}

// Непрерывное движение с заданными параметрами
char* state_move_continuous(float v, float w) {
    getState()->state = (w > 0) ? LEFT : ((w < 0) ? RIGHT : FORWARD);
    send_command(v, w);
    
    static char action_str[32];
    if (w == 0) {
        snprintf(action_str, sizeof(action_str), "CONT_FORWARD(v=%.2f)", v);
    } else {
        snprintf(action_str, sizeof(action_str), "CONT_TURN(v=%.2f,w=%.2f)", v, w);
    }
    return action_str;
}

// Вспомогательные функции
// Адаптивное время движения
float adaptive_forward_time(float available_distance) {
    if (available_distance > SAFE_FRONT_DIST) {
        return FORWARD_LONG_TIME;  // Много места - двигаемся дольше
    } else if (available_distance > CRASH_DIST) {
        return FORWARD_TIME;       // Среднее расстояние - стандартное время
    } else {
        return FORWARD_SHORT_TIME; // Мало места - короткие движения
    }
}

float calculate_pid_speed(float distance) {
    // Аварийная остановка только при очень близких препятствиях
    if (distance <= CRASH_DIST) {
        pid_reset(&getState()->speed_pid);
        return 0.0f;
    }
    
    unsigned long current_time = get_current_time_ms();
    float speed = pid_update(&getState()->speed_pid, distance, current_time);
    
    // УСИЛЕННАЯ ЛОГИКА: на больших расстояниях - максимальная скорость
    if (distance > PID_TARGET_DIST * 1.5f) {  // Если > 3.0м
        speed = FORWARD_SPEED;  // Прямо максимальная скорость
    }
    // Гарантируем минимальную скорость если впереди есть пространство
    else if (distance > SAFE_FRONT_DIST && speed < MIN_MOVING_SPEED) {
        speed = MIN_MOVING_SPEED;
    }
    
    // Более агрессивное торможение только при реальной опасности
    if (distance < SAFE_FRONT_DIST) {
        float safety_factor = (distance - CRASH_DIST) / (SAFE_FRONT_DIST - CRASH_DIST);
        speed *= safety_factor;
        
        // Но даже вблизи препятствий даем шанс на маневр
        if (speed < 0.1f && distance > CRASH_DIST + 0.15f) {
            speed = 0.1f;
        }
    }
    
    return speed;
}



/*
// Обработка застревания - чередование действий
char* state_handle_stuck(void) {
    double stuck_time = get_current_time() - getState()->last_move_time;
    static int recovery_attempt = 0;  // Счетчик попыток восстановления
    static char action_str[32];
    
    if (stuck_time > STUCK_TIME_SEVERE) {
        // Сильное застревание - пробуем движение вперед
        getState()->state = FORWARD;
        getState()->state_until = get_current_time() + 1.0;
        send_command(FORWARD_SPEED * 0.5f, 0.0f);  // Медленно вперед
        recovery_attempt = 0;  // Сброс счетчика после сильного застревания
        snprintf(action_str, sizeof(action_str), "STUCK_FORWARD");
        return action_str;
    }
    else if (stuck_time > STUCK_TIME_MEDIUM) {
        // Среднее застревание - чередуем действия
        recovery_attempt++;
        
        if (recovery_attempt % 2 == 0) {
            // Четная попытка - движение назад
            getState()->state = BACK;
            getState()->state_until = get_current_time() + 0.8;
            send_command(BACK_SPEED * 0.6f, 0.0f);
            snprintf(action_str, sizeof(action_str), "STUCK_BACK");
        } else {
            // Нечетная попытка - поворот
            getState()->state = (rand() % 2) ? LEFT : RIGHT;
            getState()->state_until = get_current_time() + 0.5;
            send_command(0.0f, (getState()->state == LEFT) ? 1.0f : -1.0f);
            snprintf(action_str, sizeof(action_str), "STUCK_TURN");
        }
        return action_str;
    }
    else if (stuck_time > STUCK_TIME_LIGHT) {
        // Легкое застревание - случайные повороты
        if (getState()->state != LEFT && getState()->state != RIGHT) {
            return (rand() % 2) ? state_turn_left() : state_turn_right();
        }
    }
    
    // Сброс счетчика, если нет застревания
    recovery_attempt = 0;
    return NULL; // Нет застревания
}
*/

/*
char* state_idle(void) {
    getState()->state = IDLE;
    getState()->state_until = get_current_time() + 0.1;
    send_command(0.0f, 0.0f);
    return "IDLE";
}

char* state_recover(float left_dist, float right_dist) {
    float turn_dir = (right_dist > left_dist) ? 1.0f : -1.0f;
    getState()->state = BACK;
    getState()->state_until = get_current_time() + RECOVER_TIME;
    send_command(-0.15f, 1.0f * turn_dir);
    
    static char action_str[32];
    snprintf(action_str, sizeof(action_str), "RECOVER(dir=%.1f)", turn_dir);
    return action_str;
}
*/
