// #pragma once

#ifndef UDP_DIFF_HAL_H 
#define UDP_DIFF_HAL_H 

#include "robo/hal/robot_hal.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>

namespace robo {

// Реализация контроллера Webots "udp_diff".
//
// Протокол обмена:
//   Команды    : UDP -> cmd_host:cmd_port, полезная нагрузка = 
//                <float32 linear_x><float32 angular_z>
//   Телеметрия : TCP-сервер на tel_bind_host:tel_bind_port, кадры в формате
//                <uint32_le payload_len><payload>, где payload — это
//                "WBTG" | 9 * float32 | uint32_le N | N * float32


// Класс наследник RobotHAL, final запрет наследования - класс RoboHAL наследуется только один раз 	
class UdpDiffHAL final : public RobotHAL {
public:
    static constexpr std::size_t kMaxLidarBeams = 720;    // Максимальное число лучей лидара
    static constexpr std::size_t kRxBufferSize   = 8192;  // размер приёмного буфера, 
							  // 8 КБ хватит (720 х 4 = 2880 байт + заголовок)

    struct Config {
        const char*   cmd_host      = "127.0.0.1";
        std::uint16_t cmd_port      = 5555;
        const char*   tel_bind_host = "0.0.0.0";
        std::uint16_t tel_bind_port = 5600;
        int           accept_timeout_ms = 200;  // таймаут poll() на время ожидания клиента
    };

    explicit UdpDiffHAL(const Config& cfg) noexcept;
    ~UdpDiffHAL() noexcept override;

    bool send_velocity(float linear_x, float angular_z) noexcept override;
    bool poll_telemetry(Telemetry& out) noexcept override;

    [[nodiscard]] bool is_connected() const noexcept { return client_fd_ >= 0; }

private:
    bool setup_cmd_socket() noexcept;
    bool setup_tel_socket() noexcept;
    bool try_accept_client() noexcept;
    bool parse_payload(const std::uint8_t* data, std::size_t len) noexcept;
    void handle_disconnect() noexcept;

    Config cfg_{};

    int cmd_fd_    {-1};
    int listen_fd_ {-1};
    int client_fd_ {-1};

    sockaddr_in cmd_addr_{};

    std::array<std::uint8_t, kRxBufferSize> rx_buffer_{};
    std::size_t rx_len_{0};

    std::array<float, kMaxLidarBeams> lidar_buf_{};
    std::uint32_t lidar_count_{0};

    Telemetry last_{};
};

} // namespace robo

#endif // udp_diff_hal_h

