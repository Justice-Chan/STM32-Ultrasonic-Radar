# STM32 Ultrasonic Radar PC Visualizer

Reads STM32 UART packets and renders a live radar display.

Expected packet format:

```text
Ang:90,Dist:53
```

Legacy format is also accepted:

```text
A:90,D:53
```

## Setup

When pyenv is installed, the committed `.python-version` file selects the
tested Python 3.13.0 interpreter automatically. Then create an isolated virtual
environment and install the dependencies:

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

## Serial Probe

Use this first to confirm the serial stream is parseable:

```bash
python serial_probe.py --port /dev/cu.usbserial-0001
```

## Radar Plot

```bash
python radar_plot.py --port /dev/cu.usbserial-0001
```

The plot drains all currently available serial lines before each redraw and keeps
only the newest sample per angle, so the sweep line stays near the live STM32
state instead of slowly replaying old buffered packets. It also uses `MOVE`
packets to update the sweep line before the distance result arrives.

Optional frame-rate control:

```bash
python radar_plot.py --port /dev/cu.usbserial-0001 --fps 30
```

The default radar range is 100 cm. Override it only when you need a wider view:

```bash
python radar_plot.py --port /dev/cu.usbserial-0001 --max-distance 200
```

## File Roles

```text
serial_probe.py  - serial read + parser smoke test
radar_plot.py    - matplotlib HUD-style radar display
radar_tui.cpp    - zero-dependency C++ terminal fallback viewer
requirements.txt - Python dependencies
```
