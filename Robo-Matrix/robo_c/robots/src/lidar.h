#ifndef LIDAR_H
#define LIDAR_H

#include "robot.h"

// Задаем основные лучи
#define  DEG_FRONT 179
#define  DEG_FRONT_LEFT 269
#define  DEG_FRONT_RIGHT 89
#define  DEG_LEFT 359
#define  DEG_RIGHT 0 

// Структура лидара
typedef struct {
   // Расстояния
   float front_dist;
   float front_left_dist;
   float front_right_dist;
   float left_dist;
   float right_dist;
   float min_dist;
   
   // Для проверки пройдет робот или нет
   int passage_safe;           // Проход достаточно широк для робота
   float passage_width;        // Расчетная ширина прохода
   float left_clearance;       // Свободное пространство слева
   float right_clearance;      // Свободное пространство справа
   
   // Флаги безопасности (1 - безопасно, 0 - опасно)
   int front_safe;
   int front_left_safe;
   int front_right_safe;
   int left_safe;
   int right_safe;
   
   // Общие флаги
   int can_move_forward;  // Можно двигаться вперед
   int can_turn_left;     // Можно повернуть налево
   int can_turn_right;    // Можно повернуть направо

   // Флаги для экстренных ситуаций
   int emergency_left;
   int emergency_right;

   // Для полного сканирования лидара
   float max_distance;          // Максимальное расстояние во всем секторе
   int best_direction_index;    // Индекс направления с максимальным расстоянием
   float sector_quality[8];     // Качество 8 секторов по 45°

} Lidar_ray;

// Структура для хранения углов лидара
typedef struct {
    int front;
    int front_left;
    int front_right;
    int left;
    int right;
} LidarDegrees;

// Глобальный экземпляр
extern LidarDegrees deg;

// Функции обработки лидара
void lidar_analysis(const Telemetry* tel, Lidar_ray* Lid);     // Функция анализа лидара
void init_lidar_degrees(void);                                 // Функция заполнения углов
float calculate_passage_width(const Lidar_ray* lidar);         // Функция расчет ширины прохода
int is_passage_safe(float passage_width);                      // Проверка возможности прохода
void full_lidar_scan(const Telemetry* tel, Lidar_ray* lidar);  // Полное сканирование лидара


 
#endif //lidar_h
