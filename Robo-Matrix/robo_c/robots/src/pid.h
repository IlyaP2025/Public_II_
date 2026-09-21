#ifndef PID_H
#define PID_H

// Структура PID-регулятора
typedef struct {
    float kp;           // Пропорциональный коэффициент
    float ki;           // Интегральный коэффициент  
    float kd;           // Дифференциальный коэффициент
    float target;       // Целевое значение
    float integral;     // Накопленная интегральная ошибка
    float prev_error;   // Предыдущая ошибка
    float output_min;   // Минимальное выходное значение
    float output_max;   // Максимальное выходное значение
    unsigned long last_time;  // Время последнего вычисления
} PIDController;

// Функции PID-регулятора
void pid_init(PIDController* pid, float kp, float ki, float kd, float target, float min, float max);
float pid_update(PIDController* pid, float measurement, unsigned long current_time);
void pid_reset(PIDController* pid);

#endif
