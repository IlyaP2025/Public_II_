#include "logic.h"
#include <stdlib.h>  // для rand()

// Основная логика принятия решений с использованием лидара
const char* decide_action(const Telemetry* tel) {
     double now = get_current_time();

    // Проверка данных лидара
    if (tel->ranges_count == 0) {
        return "NO_LIDAR_DATA";
    }

    // Анализ данных лидара
    Lidar_ray lidar = {0};
    lidar_analysis(tel, &lidar);

    // ИСПОЛЬЗУЕМ ПРАВИЛЬНОЕ min_dist ИЗ 360 ТОЧЕК
    printf("[SAFETY] Min: %.3fm (360pts), Front: %.2f, Best: index=%d, dist=%.2f\n", 
           lidar.min_dist, lidar.front_dist, lidar.best_direction_index, lidar.max_distance);
    
    // Задержка
    if (now < getState()->state_until) {
        return state_to_string(getState()->state);
    }
    
    if (getState()->state == ARC_LEFT) { 
       return state_arc_turn_right();
    } else {
       return state_arc_turn_left();
    }
}

