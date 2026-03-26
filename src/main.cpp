#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <qrcode.h>
#include <LittleFS.h>

// =====================================================
// WiFi hotspot settings
// =====================================================
const char* AP_SSID = "S.T.A.P.";
const char* AP_PASSWORD = NULL;   // Open network

IPAddress apIP(192, 168, 4, 1);

WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// =====================================================
// OLED
// SSD1306 128x64, HW I2C, SCL=6, SDA=5
// =====================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE,
    /* SCL = */ 6,
    /* SDA = */ 5
);

// =====================================================
// State
// =====================================================
int currentClients = 0;
unsigned long lastClientPollMs = 0;
unsigned long lastDisplayRefreshMs = 0;
unsigned long lastScreenToggleMs = 0;
bool showQrScreen = false;

// =====================================================
// Helpers
// =====================================================
bool isIp(const String& str) {
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (!(c == '.' || (c >= '0' && c <= '9'))) {
      return false;
    }
  }
  return true;
}

bool captivePortalRedirect() {
  String host = server.hostHeader();

  if (!isIp(host) && host != apIP.toString()) {
    server.sendHeader("Location", String("http://") + apIP.toString(), true);
    server.send(302, "text/plain", "");
    return true;
  }
  return false;
}

bool serveFile(const char* path, const char* contentType) {
  File file = LittleFS.open(path, "r");
  if (!file || file.isDirectory()) {
    return false;
  }

  server.streamFile(file, contentType);
  file.close();
  return true;
}

// =====================================================
// Web handlers
// =====================================================
void handleRoot() {
  if (captivePortalRedirect()) return;

  if (!serveFile("/index.html", "text/html")) {
    server.send(500, "text/plain", "Missing /index.html");
  }
}

void handleIco() {
  if (captivePortalRedirect()) return;

  if (!serveFile("/favicon.ico", "image/x-icon")) {
    server.send(404, "text/plain", "Missing /favicon.ico");
  }
}

void handleStyle() {
  if (!serveFile("/style.css", "text/css")) {
    server.send(404, "text/plain", "Missing /style.css");
  }
}

void handleScript() {
  if (!serveFile("/script.js", "application/javascript")) {
    server.send(404, "text/plain", "Missing /script.js");
  }
}

void handleGenerate204() {
  server.sendHeader("Location", String("http://") + apIP.toString(), true);
  server.send(302, "text/plain", "");
}

void handleHotspotDetect() {
  server.sendHeader("Location", String("http://") + apIP.toString(), true);
  server.send(302, "text/plain", "");
}

void handleNcsi() {
  server.sendHeader("Location", String("http://") + apIP.toString(), true);
  server.send(302, "text/plain", "");
}

void handleNotFound() {
  if (captivePortalRedirect()) return;

  if (!serveFile("/index.html", "text/html")) {
    server.send(404, "text/plain", "Not found");
  }
}

// =====================================================
// Display
// =====================================================
void initDisplay() {
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setBusClock(400000);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x13_tf);
  u8g2.drawStr(6, 12, "Starting hotspot...");
  u8g2.sendBuffer();
}

void drawStatusScreen(int clients) {
  char line1[24];
  char line2[24];

  snprintf(line1, sizeof(line1), "Clients: %d", clients);
  snprintf(line2, sizeof(line2), "%s", apIP.toString().c_str());

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x13_tf);
  u8g2.drawStr(30, 34, line1);
  u8g2.drawStr(30, 50, AP_SSID);
  u8g2.drawStr(30, 62, line2);
  u8g2.sendBuffer();
}

void drawQrScreen() {
  const String qrText = String("WIFI:T:nopass;S:") + AP_SSID + ";;";

  const uint8_t version = 3;

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, qrText.c_str());

  // Keep the QR inside the lower half of the screen
  const int maxScaleX = 128 / qrcode.size;
  const int maxScaleY = 32 / qrcode.size;
  int scale = min(maxScaleX, maxScaleY);
  if (scale < 1) scale = 1;

  const int qrPixels = qrcode.size * scale;
  const int xOffset = (128 - qrPixels) / 2;
  const int yOffset = 32;

  u8g2.clearBuffer();

  // White background square
  u8g2.drawBox(xOffset, yOffset, qrPixels, qrPixels);
  u8g2.setDrawColor(0);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        u8g2.drawBox(
          xOffset + x * scale,
          yOffset + y * scale,
          scale,
          scale
        );
      }
    }
  }

  u8g2.setDrawColor(1);
  u8g2.sendBuffer();
}

void renderCurrentScreen() {
  if (showQrScreen) {
    drawQrScreen();
  } else {
    drawStatusScreen(currentClients);
  }
}

// =====================================================
// Setup
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  initDisplay();

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.drawStr(6, 12, "LittleFS failed");
    u8g2.sendBuffer();
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
  if (!apStarted) {
    Serial.println("Failed to start hotspot");

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.drawStr(6, 12, "Hotspot failed");
    u8g2.sendBuffer();
    return;
  }

  Serial.print("Hotspot started. IP: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(DNS_PORT, "*", apIP);

  // Static files
  server.on("/", handleRoot);
  server.on("/index.html", handleRoot);
  server.on("/favicon.ico", handleIco); 
  server.on("/style.css", handleStyle);
  server.on("/script.js", handleScript);

  // Captive portal probes
  server.on("/generate_204", handleGenerate204);
  server.on("/gen_204", handleGenerate204);
  server.on("/hotspot-detect.html", handleHotspotDetect);
  server.on("/library/test/success.html", handleHotspotDetect);
  server.on("/ncsi.txt", handleNcsi);
  server.on("/connecttest.txt", handleNcsi);
  server.on("/redirect", handleNcsi);

  server.onNotFound(handleNotFound);
  server.begin();

  currentClients = WiFi.softAPgetStationNum();
  lastScreenToggleMs = millis();
  renderCurrentScreen();

  Serial.print("Initial clients: ");
  Serial.println(currentClients);
}

// =====================================================
// Loop
// =====================================================
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  const unsigned long now = millis();

  if (now - lastClientPollMs >= 1000) {
    lastClientPollMs = now;

    int newClientCount = WiFi.softAPgetStationNum();
    if (newClientCount != currentClients) {
      currentClients = newClientCount;

      Serial.print("Clients connected: ");
      Serial.println(currentClients);

      if (!showQrScreen) {
        drawStatusScreen(currentClients);
      }
    }
  }

  if (now - lastScreenToggleMs >= 5000) {
    lastScreenToggleMs = now;
    showQrScreen = !showQrScreen;
    renderCurrentScreen();
  }

  if (!showQrScreen && now - lastDisplayRefreshMs >= 5000) {
    lastDisplayRefreshMs = now;
    drawStatusScreen(currentClients);
  }

  delay(1);
}