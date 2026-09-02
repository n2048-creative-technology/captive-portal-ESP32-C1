# Captive Portal on ESP32-C3

Opens a WiFi access point with no password, serves a captive-portal page
over LittleFS, and shows an SSD1306 OLED with the connected-client count and
a QR code to join the network. Aimed at ESP32-C3 boards (title says "C1",
the code targets the C3 — see Known limitations).

![example](https://github.com/user-attachments/assets/262f062a-7a67-43ac-a6e9-22671c049136)

## Hardware

- ESP32-C3 dev board
- SSD1306 128x64 OLED, I2C
  - `SCL` -> GPIO6
  - `SDA` -> GPIO5

No other external hardware — the AP, DNS captive-portal redirect, and web
server all run on the ESP32 itself.

## How it works

- Opens an open (no password) access point, SSID `S.T.A.P.`, fixed IP
  `192.168.4.1`.
- A `DNSServer` catch-all plus `/generate_204` and standard captive-portal
  probe handlers redirect any client's "is there internet" check back to
  `192.168.4.1`, so most phones/laptops pop the sign-in sheet automatically.
- The web page (`data/index.html`, `data/style.css`, `data/script.js`) is
  served from LittleFS, along with `data/favicon.ico`.
- The OLED shows the live connected-client count and can toggle to a QR
  code screen (`qrcode` library) encoding the AP's join info.

## Build & flash

Requires [PlatformIO](https://platformio.org/). The web assets live in
`data/` and must be uploaded to the board's filesystem separately from the
firmware itself:

```bash
~/.platformio/penv/bin/pio run --target upload
~/.platformio/penv/bin/pio run --target uploadfs
```

(Both steps are required — `upload` flashes the firmware, `uploadfs` flashes
`data/` as a LittleFS image. Re-run `uploadfs` any time you change the web
assets.)

## Known limitations

- Repo/board naming says "ESP32-C1" but there's no such Espressif part —
  the firmware (`platformio.ini`, `WiFi.h` usage) targets the ESP32-C3.
  Treat "C1" in the repo name as a typo.
- Open network by default (`AP_PASSWORD = NULL` in `src/main.cpp`) — fine
  for a public installation piece, not for anything that needs to keep
  people off the AP.
- No captive-portal auto-popup guarantee — behavior varies by phone/OS;
  `192.168.4.1` is the manual fallback if the sign-in sheet doesn't appear.
