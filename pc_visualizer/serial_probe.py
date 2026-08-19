#!/usr/bin/env python3
import argparse
import os
import re
import sys
import termios
import time


PACKET_RE = re.compile(
    r"^\s*(?:A|Ang):(?P<angle>\d+),(?:D|Dist):(?P<distance>\d+)\s*$"
)


BAUD_RATES = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
}


def configure_serial(fd, baud):
    if baud not in BAUD_RATES:
        raise ValueError(f"Unsupported baud rate: {baud}")

    attrs = termios.tcgetattr(fd)

    # iflag, oflag, cflag, lflag, ispeed, ospeed, cc
    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    attrs[3] = 0
    attrs[4] = BAUD_RATES[baud]
    attrs[5] = BAUD_RATES[baud]

    attrs[6][termios.VMIN] = 0
    attrs[6][termios.VTIME] = 1

    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    termios.tcflush(fd, termios.TCIOFLUSH)


def parse_packet(line):
    match = PACKET_RE.match(line)
    if not match:
        return None

    return {
        "angle": int(match.group("angle")),
        "distance": int(match.group("distance")),
    }


def read_lines(fd):
    buffer = bytearray()

    while True:
        chunk = os.read(fd, 128)
        if not chunk:
            continue

        buffer.extend(chunk)

        while b"\n" in buffer:
            raw_line, _, rest = buffer.partition(b"\n")
            buffer = bytearray(rest)
            yield raw_line.decode("utf-8", errors="replace").strip()


def main():
    parser = argparse.ArgumentParser(
        description="Read STM32 radar packets like Ang:30,Dist:52 from a serial port."
    )
    parser.add_argument("--port", default="/dev/cu.usbserial-0001")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--show-raw", action="store_true")
    args = parser.parse_args()

    try:
        fd = os.open(args.port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    except OSError as exc:
        print(f"Could not open {args.port}: {exc}", file=sys.stderr)
        return 1

    try:
        configure_serial(fd, args.baud)
        print(f"Listening on {args.port} at {args.baud} baud. Press Ctrl-C to stop.")

        valid_count = 0
        bad_count = 0
        last_print = time.monotonic()

        for line in read_lines(fd):
            packet = parse_packet(line)

            if packet is None:
                bad_count += 1
                if args.show_raw:
                    print(f"ignored: {line!r}")
                continue

            valid_count += 1
            now = time.monotonic()
            elapsed = now - last_print
            last_print = now

            print(
                f"angle={packet['angle']:3d} deg  "
                f"distance={packet['distance']:4d} cm  "
                f"dt={elapsed:0.3f}s  "
                f"valid={valid_count} ignored={bad_count}"
            )

    except KeyboardInterrupt:
        print("\nStopped.")
        return 0
    finally:
        os.close(fd)


if __name__ == "__main__":
    raise SystemExit(main())
