#!/usr/bin/env python3
import argparse
import math
import os
import re
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parent
os.environ.setdefault("MPLCONFIGDIR", str(ROOT / ".matplotlib-cache"))
os.environ.setdefault("XDG_CACHE_HOME", str(ROOT / ".cache"))

import matplotlib.pyplot as plt
import serial


PACKET_RE = re.compile(
    r"^\s*(?:A|Ang):(?P<angle>\d+),(?:D|Dist):(?P<distance>\d+)\s*$"
)
MOVE_RE = re.compile(r"^\s*MOVE:(?P<angle>\d+)\s*$")


class SerialReader:
    def __init__(self, port: str, baud: int) -> None:
        self.serial = serial.Serial(port=port, baudrate=baud, timeout=0)
        self._buffer = bytearray()

    def read_available_lines(self) -> list[str]:
        waiting = self.serial.in_waiting
        if waiting:
            self._buffer.extend(self.serial.read(waiting))

        lines: list[str] = []
        while b"\n" in self._buffer:
            raw, _, rest = self._buffer.partition(b"\n")
            self._buffer = bytearray(rest)
            lines.append(raw.decode("utf-8", errors="replace").strip())

        return lines

    def close(self) -> None:
        self.serial.close()


def parse_packet(line: str) -> tuple[int, int] | None:
    match = PACKET_RE.match(line)
    if not match:
        return None

    angle = int(match.group("angle"))
    distance = int(match.group("distance"))

    if angle < 0 or angle > 180 or distance < 0:
        return None

    return angle, distance


def parse_move(line: str) -> int | None:
    match = MOVE_RE.match(line)
    if not match:
        return None

    angle = int(match.group("angle"))
    if angle < 0 or angle > 180:
        return None

    return angle


class ScanBuffer:
    def __init__(self) -> None:
        self.angles_deg: list[int] = []
        self.distances_cm: list[int] = []
        self.latest_angle = 0
        self.latest_distance = 0
        self.valid_count = 0
        self.ignored_count = 0
        self._latest_by_angle: dict[int, int] = {}

    def update(self, angle: int, distance: int) -> None:
        self._latest_by_angle[angle] = distance
        self.latest_angle = angle
        self.latest_distance = distance
        self.valid_count += 1

        ordered = sorted(self._latest_by_angle.items())
        self.angles_deg = [item[0] for item in ordered]
        self.distances_cm = [item[1] for item in ordered]

    def ignore(self) -> None:
        self.ignored_count += 1

    def move_to(self, angle: int) -> None:
        self.latest_angle = angle


class RadarPlot:
    def __init__(self, max_distance_cm: int) -> None:
        self.max_distance_cm = max_distance_cm

        plt.ion()
        self.fig = plt.figure(figsize=(9.5, 5.5), facecolor="#020806")
        self.ax = self.fig.add_subplot(111, projection="polar")
        self.ax.set_facecolor("#020806")

        self.scan_fill = None
        self.points, = self.ax.plot(
            [],
            [],
            "o",
            color="#ff4d2d",
            markeredgecolor="#ffd166",
            markeredgewidth=0.7,
            markersize=5,
            label="target blips",
        )
        self.line, = self.ax.plot([], [], "-", color="#19d96b", linewidth=1.3)
        self.sweep, = self.ax.plot(
            [], [], color="#49ff84", linewidth=2.0, label="current angle"
        )
        self.status = self.fig.text(
            0.025, 0.025, "", color="#8dffbf", fontsize=10, family="monospace"
        )
        self.header = self.fig.text(
            0.025,
            0.95,
            "STM32 ULTRASONIC RADAR",
            color="#49ff84",
            fontsize=14,
            weight="bold",
            family="monospace",
        )

        self._configure_axes()
        self.fig.tight_layout(rect=(0, 0.05, 1, 1))

    def _configure_axes(self) -> None:
        self.ax.set_theta_zero_location("E")
        self.ax.set_thetamin(0)
        self.ax.set_thetamax(180)
        self.ax.set_ylim(0, self.max_distance_cm)
        self.ax.set_rlabel_position(90)
        range_ticks = self._range_ticks()
        self.ax.set_rticks(range_ticks)
        self.ax.set_yticklabels([f"{tick:g} cm" for tick in range_ticks])
        self.ax.set_thetagrids(range(0, 181, 10))
        self.ax.grid(True, color="#0bcf63", alpha=0.45, linewidth=0.8)
        self.ax.spines["polar"].set_color("#49ff84")
        self.ax.spines["polar"].set_linewidth(2.0)
        self.ax.tick_params(colors="#8dffbf", labelsize=8)
        self.ax.yaxis.set_tick_params(colors="#8dffbf", labelsize=8)
        self.ax.legend(
            loc="upper right",
            bbox_to_anchor=(1.12, 1.12),
            facecolor="#020806",
            edgecolor="#0bcf63",
            labelcolor="#8dffbf",
        )

    def _range_ticks(self) -> list[int]:
        tick_step = 50 if self.max_distance_cm <= 200 else 100
        ticks = list(range(tick_step, self.max_distance_cm + 1, tick_step))

        if not ticks or ticks[-1] != self.max_distance_cm:
            ticks.append(self.max_distance_cm)

        return ticks

    def update(self, scan: ScanBuffer) -> None:
        theta = [math.radians(angle) for angle in scan.angles_deg]
        distances = [
            min(distance, self.max_distance_cm) for distance in scan.distances_cm
        ]

        if self.scan_fill is not None:
            self.scan_fill.remove()
            self.scan_fill = None

        if theta:
            self.scan_fill = self.ax.fill(
                theta,
                distances,
                color="#18d66d",
                alpha=0.22,
                linewidth=0,
            )[0]

        self.points.set_data(theta, distances)
        self.line.set_data(theta, distances)

        latest_theta = math.radians(scan.latest_angle)
        self.sweep.set_data(
            [latest_theta, latest_theta],
            [0, self.max_distance_cm],
        )

        self.status.set_text(
            f"ANG {scan.latest_angle:03d} DEG | "
            f"DIST {scan.latest_distance:04d} CM | "
            f"VALID {scan.valid_count:05d} | IGNORED {scan.ignored_count:03d}"
        )

        self.fig.canvas.draw_idle()
        self.fig.canvas.flush_events()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Live matplotlib radar view for STM32 packets like Ang:30,Dist:52."
    )
    parser.add_argument("--port", default="/dev/cu.usbserial-0001")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--max-distance", type=int, default=100)
    parser.add_argument("--fps", type=float, default=30.0)
    parser.add_argument("--show-raw", action="store_true")
    args = parser.parse_args()

    reader = SerialReader(args.port, args.baud)
    scan = ScanBuffer()
    plot = RadarPlot(args.max_distance)

    print(f"Listening on {args.port} at {args.baud} baud. Press Ctrl-C to stop.")

    try:
        render_interval = 1.0 / max(args.fps, 1.0)
        next_render_time = 0.0

        while plt.fignum_exists(plot.fig.number):
            for line in reader.read_available_lines():
                if not line:
                    continue

                packet = parse_packet(line)
                if packet is not None:
                    angle, distance = packet
                    scan.update(angle, distance)
                    continue

                move_angle = parse_move(line)
                if move_angle is not None:
                    scan.move_to(move_angle)
                    continue

                scan.ignore()
                if args.show_raw:
                    print(f"ignored: {line!r}")

            now = time.monotonic()
            if now >= next_render_time:
                plot.update(scan)
                next_render_time = now + render_interval

            plt.pause(0.001)

    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        reader.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
