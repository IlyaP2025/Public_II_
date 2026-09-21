#ifndef INIT_ROBOT_H
#define INIT_ROBOT_H

// === КОНСТАНТЫ И ПАРАМЕТРЫ ===
// Размеры робота
#define ROBOT_WIDTH 0.19f                                    // Ширина робота в метрах
#define ROBOT_LENGTH 0.23f                                   // Длина робота в метрах
#define SAFETY_MARGIN 0.10f                                  // Запас безопасности
#define MIN_PASSAGE_WIDTH (ROBOT_WIDTH + 2 * SAFETY_MARGIN)  // Минимальная ширина прохода

#define SAFE_FRONT_DIST 0.5f              // Безопасная дистанция c переди
#define SAFE_FRONT_LEFT_DIST 0.35f         // Безопастная дистанция с переди-левея 
#define SAFE_FRONT_RIGHT_DIST 0.35f        // Безопастная дистанция с переди-левея 
#define SAFE_LEFT_DIST 0.4f               // Безопастная дистанция с лева (было 0,4) 
#define SAFE_RIGHT_DIST 0.4f              // Безопастная дистанция с права (было 0,4) 
#define CRASH_DIST 0.25f                  // Опасная дистанция

//#define RECOVER_TIME 0.5f                  // Время на выполнение движения возврата
#define TURN_TIME 0.5f                       // Время на разворот

// Временые задержки для движения в перед
#define FORWARD_TIME 0.4f                  // Время движения вперед по умолчанию
#define FORWARD_LONG_TIME 1.5f             // Длительное движение вперед (для антизастревания)
#define FORWARD_SHORT_TIME 0.3f            // Короткое движение вперед


#define FORWARD_SPEED 1.0f                 // Скорость движения вперед (было 0.3)
#define BACK_SPEED -0.2f                   // Скорость движения назад  (было 0.1)
#define TURN_SPEED 0.8f                    // Cкорость поворота (было 0.4)

#define STUCK_TIME_LIGHT 2.0f              // Легкое застревание (сек)
#define STUCK_TIME_MEDIUM 4.0f             // Среднее застревание (сек)  
#define STUCK_TIME_SEVERE 6.0f             // Сильное застревание (сек)
					   
// Параметры PID-регулятора
// PID_KP - отвечает за скорость реакции (больше = быстрее реакция, но возможны колебания)
// PID_KI - устраняет статическую ошибку (больше = быстрее уходит ошибка, но возможны перерегулирования)
// PID_KD - демпфирование (больше = более плавное поведение, но медленнее реакция)

#define PID_KP 1.2f                         // Пропорциональный коэффициент (было 0.8)
#define PID_KI 0.1f                         // Интегральный коэффициент (было 0.05) 
#define PID_KD 0.2f                         // Дифференциальный коэффициент (было 0.5)
#define PID_TARGET_DIST 2.0f                // Целевая дистанция для PID (метров) (было 1.5)

#define MIN_MOVING_SPEED 0.3f              // Минимальная скорость чтобы гарантировать движение (было 0,15)

// =================================

#endif //init_robot_h
