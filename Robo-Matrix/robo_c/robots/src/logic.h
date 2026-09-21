#ifndef LOGIC_H
#define LOGIC_H

#include "robot.h"
#include "lidar.h"
#include "init_robot.h"
#include "states.h" 

// Основная функция принятия решений
const char* decide_action(const Telemetry* tel);

#endif
