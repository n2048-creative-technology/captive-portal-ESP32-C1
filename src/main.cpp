#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <U8g2lib.h>
#include <Wire.h>

// =====================================================
// WiFi hotspot settings
// =====================================================
const char* AP_SSID = "S.T.A.P.";
const char* AP_PASSWORD = NULL; //  "12345678";  // minimum 8 characters

IPAddress apIP(192, 168, 4, 1);

WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// =====================================================
// OLED: confirmed from your working project
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

// =====================================================
// HTML page
// =====================================================
String makeHtmlPage() {
  String ipStr = apIP.toString();

  String html = R"rawliteral(

<!DOCTYPE html>
<html>
  <head>
    	<title>smaller than a pixel</title>

	<meta charset="UTF-8">
  	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	
  <style>
   body {
	margin: 0;
	padding: 0;
	background-color: black;

/*    background-image: url('stap.png'); 
    background-repeat: no-repeat;
    background-position: center;
    background-attachment: fixed;
*/
}


video {
    display:none;
    max-width: 100%;
    max-height: 100%;
    width: auto;
    height: auto;
    margin: auto;
}

#video-container {
  z-index: 1;
}

#video-container:fullscreen video {
    display:none;
    max-width: 100%;
    max-height: 100%;
    width: auto;
    height: auto;
    margin: auto;  
}

#video-container:fullscreen instruction {
  position: fixed;
  height: 100vh;
  width: auto;
  position: fixed;
  top: calc(50vh - (80vh / 2));
  right: 10px;
}


#instruction,
#instruction:fullscreen
 {
        color: white;
	background:none;
        font-size: 10vw;
        line-height: 1.5;
        position: absolute; 
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        display: flex;
        align-items: center;
	justify-content: center;
	z-index: 2147483647;
        text-align: center;
}

.container {

	display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        height: 100vh; 
        text-align: center;
        position: relative;
}

/* Media query for larger screens (laptops, desktops) */
@media screen and (min-width: 768px) {
        .text {
                font-size: 10vw; /* Set font size relative to viewport width */
        }
}

.text {
    font-family: monospace;
    font-size: large;


	color: white;
        line-height: 1.5;
/*        opacity: 0; */
        animation-duration: 5s;
        animation-timing-function: ease-in-out;
        animation-fill-mode: forwards;
        animation-iteration-count: 1;
        position: absolute;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        display: flex;
        align-items: center;
        justify-content: center;
}

.text:nth-child(1) { animation-name: fadein1; }
.text:nth-child(2) { animation-name: fadein2; }
.text:nth-child(3) { animation-name: fadein3; }
.text:nth-child(4) { animation-name: fadein4; }
.text:nth-child(5) { animation-name: fadein5; }
.text:nth-child(6) { animation-name: fadein6; }
.text:nth-child(7) { animation-name: fadein7; }
.text:nth-child(8) { animation-name: fadein8; }
.text:nth-child(9) { animation-name: fadein9; }
.text:nth-child(10) {animation-name: fadein10; }


@keyframes fadein1 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 0; }
    40% { opacity: 0; }
    50% { opacity: 0; }
    60% { opacity: 0; }
    70% { opacity: 0; }
    80% { opacity: 0; }
    90% { opacity: 0; }
    100% { opacity: 0; }
}
@keyframes fadein2 {
    0% { opacity: 0; }
    10% { opacity: 1; }
    20% { opacity: 0;}
    30% { opacity: 0;}
    40% { opacity: 0;}
    50% { opacity: 0;}
    60% { opacity: 0;}
    70% { opacity: 0;}
    80% { opacity: 0;}
    90% { opacity: 0;}
    100% { opacity: 0;}
}
@keyframes fadein3 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 1; }
    30% { opacity: 0; }
    40% { opacity: 0; }
    50% { opacity: 0; }
    60% { opacity: 0; }
    70% { opacity: 0; }
    80% { opacity: 0; }
    90% { opacity: 0; }
    100% { opacity: 0;}
}
@keyframes fadein4 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 1; }
    40% { opacity: 0; }
    50% { opacity: 0; }
    60% { opacity: 0; }
    70% { opacity: 0; }
    80% { opacity: 0; }
    90% { opacity: 0; }
    100% { opacity: 0;}
}
@keyframes fadein5 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 0; }
    40% { opacity: 1; }
    50% { opacity: 0; }
    60% { opacity: 0; }
    70% { opacity: 0; }
    80% { opacity: 0; }
    90% { opacity: 0; }
    100% { opacity: 0;}
}
@keyframes fadein6 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 0; }
    40% { opacity: 0; }
    50% { opacity: 1; }
    60% { opacity: 0; }
    70% { opacity: 0; }
    80% { opacity: 0; }
    90% { opacity: 0; }
    100% { opacity: 0;}
}
@keyframes fadein7 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 0; }
    40% { opacity: 0; }
    50% { opacity: 0; }
    60% { opacity: 1; }
    70% { opacity: 0; }
    80% { opacity: 0; }
    90% { opacity: 0; }
    100% { opacity: 0;}
}
@keyframes fadein8 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 0; }
    40% { opacity: 0; }
    50% { opacity: 0; }
    60% { opacity: 0; }
    70% { opacity: 1; }
    80% { opacity: 1; }
    90% { opacity: 1; }
    100% { opacity: 1;}
}
@keyframes fadein9 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 0; }
    40% { opacity: 0; }
    50% { opacity: 0; }
    60% { opacity: 0; }
    70% { opacity: 0; }
    80% { opacity: 1; }
    90% { opacity: 0; }
    100% { opacity: 0; }
}
@keyframes fadein10 {
    0% { opacity: 0; }
    10% { opacity: 0; }
    20% { opacity: 0; }
    30% { opacity: 0; }
    40% { opacity: 0; }
    50% { opacity: 0; }
    60% { opacity: 0; }
    70% { opacity: 0; }
    80% { opacity: 0; }
    90% { opacity: 0; }
    100% { opacity: 1; }
}


</style>


  </head>
  <body>

    <div class="container" id="container">
        <p class="text"></p>
        <p class="text">SMALLER</p>
        <p class="text">THAN</p>
        <p class="text">A</p>
        <p class="text">PIXEL</p>
        <p class="text"></p>
        <p class="text"></p>
        <p class="text">SMALLER THAN A PIXEL</p>
    </div>

    	<script type="text/javascript">
      </script>
</html>

)rawliteral";

  return html;
}

// =====================================================
// Captive portal helpers
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

  if (!isIp(host) && host != String(apIP.toString())) {
    server.sendHeader("Location", String("http://") + apIP.toString(), true);
    server.send(302, "text/plain", "");
    return true;
  }
  return false;
}

// =====================================================
// Web handlers
// =====================================================
void handleRoot() {
  if (captivePortalRedirect()) return;
  server.send(200, "text/html", makeHtmlPage());
}

// Common captive portal detection endpoints
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
  server.send(200, "text/html", makeHtmlPage());
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

void drawDisplay(int clients) {
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

// =====================================================
// Setup
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  initDisplay();

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

  // DNS wildcard: all domains resolve to ESP32 AP IP
  dnsServer.start(DNS_PORT, "*", apIP);

  // Main page
  server.on("/", handleRoot);

  // Android captive portal checks
  server.on("/generate_204", handleGenerate204);
  server.on("/gen_204", handleGenerate204);

  // Apple captive portal checks
  server.on("/hotspot-detect.html", handleHotspotDetect);
  server.on("/library/test/success.html", handleHotspotDetect);

  // Windows captive portal checks
  server.on("/ncsi.txt", handleNcsi);
  server.on("/connecttest.txt", handleNcsi);
  server.on("/redirect", handleNcsi);

  server.onNotFound(handleNotFound);
  server.begin();

  currentClients = WiFi.softAPgetStationNum();
  drawDisplay(currentClients);

  Serial.print("Initial clients: ");
  Serial.println(currentClients);
}

// =====================================================
// Loop
// =====================================================
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  unsigned long now = millis();

  if (now - lastClientPollMs >= 1000) {
    lastClientPollMs = now;

    int newClientCount = WiFi.softAPgetStationNum();
    if (newClientCount != currentClients) {
      currentClients = newClientCount;

      Serial.print("Clients connected: ");
      Serial.println(currentClients);

      drawDisplay(currentClients);
    }
  }

  if (now - lastDisplayRefreshMs >= 5000) {
    lastDisplayRefreshMs = now;
    drawDisplay(currentClients);
  }

  delay(1);
}