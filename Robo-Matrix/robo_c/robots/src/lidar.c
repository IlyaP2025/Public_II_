#include "lidar.h"
#include "init_robot.h"

// Инициализация углов лидара (статически)
LidarDegrees deg = {
    .front = DEG_FRONT,
    .front_left = DEG_FRONT_LEFT,
    .front_right = DEG_FRONT_RIGHT,
    .left = DEG_LEFT,
    .right = DEG_RIGHT
};

// Фнкция инстализации, есле нужно
void init_lidar_degrees(void) {
    // Уже инициализировано статически
}

// Функция анализа лидара
void lidar_analysis(const Telemetry* tel, Lidar_ray* Lid) {
    if (!tel || !Lid || tel->ranges_count == 0) {
        return;
    }

    *Lid = (Lidar_ray){0};
    
    // Сканируем все точки
    full_lidar_scan(tel, Lid); 

    // Заполняем структуру лидара    
    Lid->front_dist = tel->ranges[(size_t)deg.front];             // Дистанция спереди 180гр
    Lid->left_dist = tel->ranges[(size_t)deg.left];               // Дистанция с лева 180+45 = 225гр
    Lid->right_dist = tel->ranges[(size_t)deg.right];             // Дистанция с права 180-45 = 135гр 
    Lid->front_left_dist = tel->ranges[(size_t)deg.front_left];   // Дистанция спереди-левей = 202гр
    Lid->front_right_dist = tel->ranges[(size_t)deg.front_right]; // Дистанция спереди-правей = 157гр

    // Определяем самое наименьшее растояние по лучам
    // Находим минимальное расстояние
   // Lid->min_dist = fminf(fminf(Lid->front_dist, Lid->left_dist), 
    //                     fminf(Lid->right_dist, fminf(Lid->front_left_dist, Lid->front_right_dist)));

    // ВЫЧИСЛЯЕМ ШИРИНУ ПРОХОДА И ПРОХОДИМОСТЬ
    Lid->passage_width = calculate_passage_width(Lid);
    Lid->passage_safe = is_passage_safe(Lid->passage_width); 

    // УЛУЧШЕННАЯ ЛОГИКА БЕЗОПАСНОСТИ:
    // 1. Фронтальная безопасность - учитываем ширину робота
    Lid->front_safe = (Lid->front_dist > SAFE_FRONT_DIST) &&
                     (Lid->front_left_dist > SAFE_FRONT_DIST * 0.7f) &&
                     (Lid->front_right_dist > SAFE_FRONT_DIST * 0.7f) &&
                     (Lid->passage_safe); 

    // 2. Безопасность поворотов - более либеральная
    Lid->left_safe = (Lid->left_dist > SAFE_LEFT_DIST) ||
                    (Lid->front_left_dist > SAFE_FRONT_LEFT_DIST);
    Lid->right_safe = (Lid->right_dist > SAFE_RIGHT_DIST) ||
                     (Lid->front_right_dist > SAFE_FRONT_RIGHT_DIST);

    // 3. УСОВЕРШЕНСТВОВАННЫЕ ФЛАГИ ВОЗМОЖНОСТЕЙ:
    
    // Можно двигаться вперед если:
    // - Прямо достаточно места И
    // - Хотя бы с одной стороны есть пространство для маневра
    Lid->can_move_forward = Lid->front_safe &&
                           Lid->passage_safe && 
                           (Lid->left_safe || Lid->right_safe);

    // Можно повернуть налево если:
    // - Левый край свободен ИЛИ передне-левый луч свободен
    // - И при этом не упираемся в стену спереди
    Lid->can_turn_left = Lid->left_safe && 
                        (Lid->front_dist > CRASH_DIST * 2.0f);

    // Можно повернуть направо если:
    // - Правый край свободен ИЛИ передне-правый луч свободен  
    // - И при этом не упираемся в стену спереди
    Lid->can_turn_right = Lid->right_safe &&
                         (Lid->front_dist > CRASH_DIST * 2.0f);

    // 4. ЭКСТРЕННЫЕ ФЛАГИ - для быстрых решений в тупиках
    Lid->emergency_left = (Lid->left_dist > SAFE_LEFT_DIST * 0.5f);
    Lid->emergency_right = (Lid->right_dist > SAFE_RIGHT_DIST * 0.5f);


   // ДЕБАГ ВЫВОД (можно уменьшить частоту после отладки)
    static int debug_counter = 0;
    if (debug_counter++ % 30 == 0) {
        printf("[PASSAGE] Width: %.3fm, Safe: %s, Min: %.3fm\n", 
               Lid->passage_width, 
               Lid->passage_safe ? "YES" : "NO",
               MIN_PASSAGE_WIDTH);
        printf("[CLEARANCE] Left: %.3fm, Right: %.3fm\n",
               Lid->left_clearance, Lid->right_clearance);
    }
}

// Расчет ширины прохода
float calculate_passage_width(const Lidar_ray* lidar) {
    // Используем тригонометрию для оценки ширины прохода
    // Углы между лучами: 45° между left/front_left и right/front_right
    
    float left_effective = fminf(lidar->left_dist, lidar->front_left_dist * 1.414f); // √2
    float right_effective = fminf(lidar->right_dist, lidar->front_right_dist * 1.414f);
    
    // Ширина прохода = сумма проекций на перпендикуляр к движению
    float width = left_effective * sinf(45.0f * M_PI / 180.0f) + 
                  right_effective * sinf(45.0f * M_PI / 180.0f);
    
    return width;
}

// Проверка возможности прохода
int is_passage_safe(float passage_width) {
    return (passage_width >= MIN_PASSAGE_WIDTH);
}

// Полное сканирование лидара
void full_lidar_scan(const Telemetry* tel, Lidar_ray* lidar) {
    lidar->max_distance = 0;
    lidar->min_dist = 10.0f; // 🔥 Инициализируем большим значением
    lidar->best_direction_index = 179;
    
    // Сканируем ВЕСЬ лидар (360 точек)
    for (int i = 0; i < (int)tel->ranges_count; i++) {
        float dist = tel->ranges[i];
        
        // 🔥 ПРАВИЛЬНАЯ ФИЛЬТРАЦИЯ: игнорируем некорректные значения
        if (dist > 0.05f && dist < 10.0f) {
            // Максимальное расстояние для навигации
            if (dist > lidar->max_distance) {
                lidar->max_distance = dist;
                lidar->best_direction_index = i;
            }
            
            // 🔥 МИНИМАЛЬНОЕ РАССТОЯНИЕ ДЛЯ БЕЗОПАСНОСТИ
            if (dist < lidar->min_dist) {
                lidar->min_dist = dist;
            }
        }
    }
    
    // Анализируем 8 секторов по 45°
    for (int sector = 0; sector < 8; sector++) {
        int start_idx = sector * 45;
        float sector_sum = 0;
        int valid_points = 0;
        
        for (int i = start_idx; i < start_idx + 45 && i < (int)tel->ranges_count; i++) {
            if (tel->ranges[i] > 0.1f && tel->ranges[i] < 10.0f) {
                sector_sum += tel->ranges[i];
                valid_points++;
            }
        }
        
        lidar->sector_quality[sector] = (valid_points > 0) ? (sector_sum / valid_points) : 0;
    }
}
