# Otter VerteilerBaustein

ESP32 firmware for the Otter model railroad control system. The VerteilerBaustein (distribution module) connects up to 5 modules and controls them via WiFi and MQTT.

## Supported module types (per slot)

| Module | Channels | Controlled via MQTT |
|--------|----------|---------------------|
| **TM** – Button module | 4 per module (up to 20 total) | `MqttTMT[n]` topics |
| **SM** – Signal module  | 2 per module (up to 8 total)  | `MqttSMS[n]` topics |
| **WM** – Turnout module       | 2 per module (up to 8 total)  | `MqttWMW[n]` topics |

Only one module type may be active per slot. A misconfiguration (more than one type active in the same slot) is detected at startup and logged as a fatal error.

## Requirements

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/) v6.x
- Target: ESP32
- IDF components (fetched automatically by `idf.py`):
  - `espressif/mqtt ^1.0.0`
  - `espressif/cjson >=1.0.0~1`

## Build & flash

```sh
# Set up the IDF environment (adjust path to your installation)
. ~/esp/esp-idf/export.sh

# First build — set the target
idf.py set-target esp32

# Build, flash and open the serial monitor
idf.py build flash monitor
```

## Configuration

Settings are stored in NVS and can be changed at runtime via the **serial configuration console** (115200 baud). Defaults compiled into the firmware are defined in [main/settings.cpp](main/settings.cpp).

| Setting | Description |
|---------|-------------|
| `ssid` / `password` | WiFi credentials |
| `mqtt_server` | MQTT broker IP address |
| `client_name` | MQTT client ID |
| `AbschNummer` / `VerteilerBaustein` | Last two octets of the static IP |
| `networkByte1/2` | First two octets of the network |
| `gatewayByte3/4` | Last two octets of the default gateway |
| `TM_active[n]` / `SM_active[n]` / `WM_active[n]` | Enable module type per slot |

The device uses a static IP: `networkByte1.networkByte2.AbschNummer.VerteilerBaustein`.

## Project structure

```
main/
├── main.cpp         – Entry point, WiFi & MQTT setup
├── tm.cpp / tm.h    – Signal/output module logic
├── sm.cpp / sm.h    – Switch/relay module logic
├── wm.cpp / wm.h    – Turnout module logic
├── settings.cpp/.h  – NVS-backed configuration
├── serial_config.cpp/.h – Serial configuration console
└── otter.cpp / otter.h  – Shared pin mapping & MQTT topic definitions
```
