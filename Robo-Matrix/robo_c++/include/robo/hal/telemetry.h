// #pragma once - гарантирует, что *.h файл подключится только 1 раз при компиляции.

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <cstddef> // Базовые определения, size_t, ptrdiff_t, nullptr_t, max_align_t, offsetof 
#include <cstdint> // Целые фиксированной ширины, int8_t-int64_t и т.д.
#include <span>    // Шаблонный класс, указатель + количество

namespace robo {

// Моментальный снимок одного кадра телеметрии с тренажера.
//
// Примечание о сроке службы:
//   Диапазон значений <const float> lidar_ranges указывает на хранилище, принадлежащее HAL.
//   Оно остается действительным ТОЛЬКО до следующего вызова RobotHAL::poll_telemetry().
//   Не копируйте span в долгоживущие структуры.

struct Telemetry {

    // Одометрия/поза робота: X, Y, угол от начального положения робота.
    float odom_x  {0.0F}; // Смещение по оси X, м
    float odom_y  {0.0F}; // Смещение по оси Y, м
    float odom_th {0.0F}; // Угол поворота робота, рад 

    // Скорости робота: линейные X/Y и угловая вокруг Z
    float vx  {0.0F}; // Скорость по X, м/с
    float vy  {0.0F}; // Скорость по Y, м/с
    float vth {0.0F}; // Угловая скорость, рад/с

    // Угловые скорости roll/pitch/yaw, IMU/гироскоп
    float wx  {0.0F}; // Угловая скорость по X (roll), рад/с
    float wy  {0.0F}; // Угловая скорость по Y (pitch), рад/с
    float wz  {0.0F}; // Угловая скорость по Z (yaw), рад/с

    // Лидар: число точек/лучей и массив дистанций в метрах
    std::span<const float> lidar_ranges{};
};

} // namespace robo

#endif // TELEMETRY_H 
