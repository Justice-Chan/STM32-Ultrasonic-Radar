#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int kMaxAngle = 180;
constexpr int kDefaultMaxDistanceCm = 250;
constexpr int kCanvasWidth = 73;
constexpr int kCanvasHeight = 25;
constexpr double kPi = 3.14159265358979323846;

volatile std::sig_atomic_t g_stop = 0;

struct Packet {
    int angle = 0;
    int distance_cm = 0;
};

struct ScanBuffer {
    std::array<int, kMaxAngle + 1> distance{};
    std::array<bool, kMaxAngle + 1> valid{};
    int latest_angle = 0;
    int latest_distance = 0;
    int valid_count = 0;
    int ignored_count = 0;
};

void handle_signal(int) {
    g_stop = 1;
}

speed_t baud_to_termios(int baud) {
    switch (baud) {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        default:
            return 0;
    }
}

int open_serial(const std::string& port, int baud) {
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Could not open " << port << ": " << std::strerror(errno)
                  << "\n";
        return -1;
    }

    speed_t termios_baud = baud_to_termios(baud);
    if (termios_baud == 0) {
        std::cerr << "Unsupported baud rate: " << baud << "\n";
        close(fd);
        return -1;
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "tcgetattr failed: " << std::strerror(errno) << "\n";
        close(fd);
        return -1;
    }

    cfmakeraw(&tty);
    cfsetispeed(&tty, termios_baud);
    cfsetospeed(&tty, termios_baud);

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "tcsetattr failed: " << std::strerror(errno) << "\n";
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);
    return fd;
}

std::optional<Packet> parse_packet(const std::string& line) {
    int angle = 0;
    int distance = 0;
    char tail = '\0';

    std::istringstream input(line);
    char a_label = '\0';
    char colon1 = '\0';
    char comma = '\0';
    char d_label = '\0';
    char colon2 = '\0';

    if (!(input >> a_label >> colon1 >> angle >> comma >> d_label >> colon2 >>
          distance)) {
        return std::nullopt;
    }

    if (input >> tail) {
        return std::nullopt;
    }

    if (a_label != 'A' || colon1 != ':' || comma != ',' || d_label != 'D' ||
        colon2 != ':') {
        return std::nullopt;
    }

    if (angle < 0 || angle > kMaxAngle || distance < 0) {
        return std::nullopt;
    }

    return Packet{angle, distance};
}

std::vector<std::string> read_available_lines(int fd, std::string& buffer) {
    std::vector<std::string> lines;
    char chunk[256];

    while (true) {
        ssize_t n = read(fd, chunk, sizeof(chunk));
        if (n > 0) {
            buffer.append(chunk, static_cast<size_t>(n));
            continue;
        }

        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }

        if (n < 0) {
            std::cerr << "Serial read failed: " << std::strerror(errno) << "\n";
        }
        break;
    }

    size_t pos = 0;
    while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string line = buffer.substr(0, pos);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        buffer.erase(0, pos + 1);
        lines.push_back(line);
    }

    return lines;
}

void update_buffer(ScanBuffer& scan, const Packet& packet) {
    scan.distance[packet.angle] = packet.distance_cm;
    scan.valid[packet.angle] = true;
    scan.latest_angle = packet.angle;
    scan.latest_distance = packet.distance_cm;
    scan.valid_count++;
}

void render(const ScanBuffer& scan, int max_distance_cm) {
    std::vector<std::string> canvas(kCanvasHeight,
                                    std::string(kCanvasWidth, ' '));

    const int origin_x = kCanvasWidth / 2;
    const int origin_y = kCanvasHeight - 2;
    const int radius = std::min(origin_x - 2, kCanvasHeight - 3);

    auto plot = [&](int x, int y, char c) {
        if (x >= 0 && x < kCanvasWidth && y >= 0 && y < kCanvasHeight) {
            canvas[y][x] = c;
        }
    };

    for (int r = radius / 4; r <= radius; r += radius / 4) {
        for (int deg = 0; deg <= 180; deg += 2) {
            double theta = deg * kPi / 180.0;
            int x = origin_x + static_cast<int>(std::round(std::cos(theta) * r));
            int y = origin_y - static_cast<int>(std::round(std::sin(theta) * r));
            plot(x, y, '.');
        }
    }

    for (int deg : {0, 30, 60, 90, 120, 150, 180}) {
        double theta = deg * kPi / 180.0;
        for (int r = 0; r <= radius; ++r) {
            int x = origin_x + static_cast<int>(std::round(std::cos(theta) * r));
            int y = origin_y - static_cast<int>(std::round(std::sin(theta) * r));
            plot(x, y, '.');
        }
    }

    for (int angle = 0; angle <= kMaxAngle; ++angle) {
        if (!scan.valid[angle]) {
            continue;
        }

        double clipped = std::min(scan.distance[angle], max_distance_cm);
        double scaled = clipped / max_distance_cm;
        double theta = angle * kPi / 180.0;
        int r = static_cast<int>(std::round(scaled * radius));
        int x = origin_x + static_cast<int>(std::round(std::cos(theta) * r));
        int y = origin_y - static_cast<int>(std::round(std::sin(theta) * r));
        plot(x, y, '*');
    }

    double latest_theta = scan.latest_angle * kPi / 180.0;
    for (int r = 0; r <= radius; ++r) {
        int x = origin_x + static_cast<int>(std::round(std::cos(latest_theta) * r));
        int y = origin_y - static_cast<int>(std::round(std::sin(latest_theta) * r));
        plot(x, y, '|');
    }

    plot(origin_x, origin_y, '+');

    std::cout << "\033[H\033[2J";
    std::cout << "STM32 ultrasonic radar | latest angle="
              << scan.latest_angle << " deg distance=" << scan.latest_distance
              << " cm | valid=" << scan.valid_count
              << " ignored=" << scan.ignored_count << "\n";
    std::cout << "Range scale: 0.." << max_distance_cm
              << " cm | Ctrl-C to stop\n\n";

    for (const auto& row : canvas) {
        std::cout << row << "\n";
    }
    std::cout.flush();
}

}  // namespace

int main(int argc, char** argv) {
    std::string port = "/dev/cu.usbserial-0001";
    int baud = 115200;
    int max_distance_cm = kDefaultMaxDistanceCm;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = argv[++i];
        } else if (arg == "--baud" && i + 1 < argc) {
            baud = std::atoi(argv[++i]);
        } else if (arg == "--max-distance" && i + 1 < argc) {
            max_distance_cm = std::atoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0]
                      << " [--port /dev/cu.usbserial-0001]"
                      << " [--baud 115200] [--max-distance 250]\n";
            return 0;
        }
    }

    if (max_distance_cm <= 0) {
        std::cerr << "--max-distance must be positive\n";
        return 1;
    }

    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    int fd = open_serial(port, baud);
    if (fd < 0) {
        return 1;
    }

    ScanBuffer scan;
    std::string serial_buffer;

    render(scan, max_distance_cm);

    while (!g_stop) {
        for (const auto& line : read_available_lines(fd, serial_buffer)) {
            auto packet = parse_packet(line);
            if (!packet) {
                scan.ignored_count++;
                continue;
            }
            update_buffer(scan, *packet);
            render(scan, max_distance_cm);
        }
        usleep(15000);
    }

    close(fd);
    std::cout << "\033[0m\nStopped.\n";
    return 0;
}
