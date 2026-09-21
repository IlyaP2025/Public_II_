#include "pid.h"
#include <time.h>

// Инициализация PID-регулятора
void pid_init(PIDController* pid, float kp, float ki, float kd, float target, float min, float max) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->target = target;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output_min = min;
    pid->output_max = max;
    pid->last_time = 0;
}

// Сброс PID-регулятора
void pid_reset(PIDController* pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->last_time = 0;
}

// Обновление PID-регулятора
// Обновленная функция pid_update
float pid_update(PIDController* pid, float measurement, unsigned long current_time) {
    // ИНВЕРТИРОВАННАЯ ЛОГИКА: больше расстояние -> больше скорость
    float error = measurement - pid->target;  // ИЗМЕНЕНИЕ ЗДЕСЬ!
    
    // Вычисление dt
    float dt = 0.01f;
    if (pid->last_time > 0) {
        dt = (current_time - pid->last_time) / 1000.0f;
        if (dt <= 0) dt = 0.01f;
        if (dt > 1.0f) dt = 1.0f;
    }
    pid->last_time = current_time;
    
    // Пропорциональная составляющая
    float proportional = pid->kp * error;
    
    // Интегральная составляющая (только для положительных ошибок)
    if (error > 0) {
        pid->integral += error * dt;
    } else {
        pid->integral = 0;  // Сброс интегратора при отрицательной ошибке
    }
    
    // Ограничение интегральной составляющей
    float integral_max = 2.0f / (pid->ki != 0 ? pid->ki : 1.0f);
    if (pid->integral > integral_max) pid->integral = integral_max;
    if (pid->integral < -integral_max) pid->integral = -integral_max;
    
    float integral = pid->ki * pid->integral;
    
    // Дифференциальная составляющая
    float derivative = 0.0f;
    if (dt > 0) {
        derivative = pid->kd * (error - pid->prev_error) / dt;
    }
    
    pid->prev_error = error;
    
    // Суммируем все составляющие
    float output = proportional + integral + derivative;
    
    // Ограничиваем выходное значение
    if (output > pid->output_max) output = pid->output_max;
    if (output < pid->output_min) output = pid->output_min;
    
    return output;
}
