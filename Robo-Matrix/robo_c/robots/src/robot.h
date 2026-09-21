#ifndef ROBOT_H
#define ROBOT_H
#include <arpa/inet.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "state_types.h"

// === КОНСТАНТЫ И ПАРАМЕТРЫ ===
#define M_PI 3.14159265358979323846
#define CMD_HOST_DEFAULT "127.0.0.1"
#define CMD_PORT_DEFAULT 5555
#define TEL_HOST_DEFAULT "0.0.0.0"
#define TEL_PORT_DEFAULT 5600
#define PROTO_DEFAULT "tcp"

// === СТРУКТУРЫ ДАННЫХ ===
typedef struct {
  float x, y, z;
} Vector3;

typedef struct {
  float odom_x, odom_y, odom_th;
  Vector3 velocity;
  Vector3 gyro;
  float* ranges;
  size_t ranges_count;
} Telemetry;

// Функции управления жизненным циклом
void robot_init(void);
void robot_cleanup(void);
int get_telemetry_socket(void);

// Основные функции робота 
void send_command(float v, float w);
void update_robot_state(const Telemetry* tel);
Telemetry* recv_telemetry(int sock, const char* proto);
void print_status(const Telemetry* tel, const char* action);

#endif
