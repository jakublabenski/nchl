#include <EEPROM.h>
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

// needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#include <FS.h> // SPIFFS

#include <Adafruit_NeoPixel.h>

const int TRIGGER_PIN = D0;
const int LIGHTS_PIN = D6;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(50, LIGHTS_PIN, NEO_RGB + NEO_KHZ800);

Adafruit_NeoPixel status_led = Adafruit_NeoPixel(1, D2, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

struct Storage {
    int32_t color;
} storage;

void configureWiFi()
{
    Serial.println("Reconfiguration");

    WiFiManager wifiManager;

    // reset settings - for testing
    wifiManager.resetSettings();

    // sets timeout until configuration portal gets turned off
    // useful to make it all retry or go to sleep
    // in seconds
    // wifiManager.setTimeout(120);

    if (!wifiManager.startConfigPortal("nchl_configuration")) {
        Serial.println("failed to connect and hit timeout");
    }
    Serial.println("Reconfiguration done");
}

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

void updateStorage()
{
}

void changeColor(int32_t color)
{
    storage.color = color;
    updateStorage();

    String out("<h1>Chnaging color</h1>");

    //    out += f;

    server.send(200, "text/html", out);
}

String getContentType(String filename)
{
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html"; // If a folder is requested, send the index file
    String contentType = getContentType(path); // Get the MIME type
    if (SPIFFS.exists(path)) { // If the file exists
        File file = SPIFFS.open(path, "r"); // Open it
        size_t sent = server.streamFile(file, contentType); // And send it to the client
        file.close(); // Then close the file again
        return true;
    }
    Serial.println("\tFile Not Found");
    return false; // If the file doesn't exist, return false
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t lenght)
{ // When a WebSocket message is received
    Serial.printf("Websocket conneted\n");
    switch (type) {
    case WStype_DISCONNECTED: // if the websocket is disconnected
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED: { // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // rainbow = false;                  // Turn rainbow off when a new connection is established
    } break;
    case WStype_TEXT: // if new text data is received
        Serial.printf("[%u] get Text: %s\n", num, payload);
        if (payload[0] == '#') { // we get RGB data
            uint32_t rgb = (uint32_t)strtol((const char*)&payload[1], NULL, 16); // decode rgb data
            int r = ((rgb >> 20) & 0x3FF); // 10 bits per color, so R: bits 20-29
            int g = ((rgb >> 10) & 0x3FF); // G: bits 10-19
            int b = rgb & 0x3FF; // B: bits  0-9

            changeColor(strip.Color(r >> 2, g >> 2, b >> 2));
        } else if (payload[0] == 'R') { // the browser sends an R when the rainbow effect is enabled
            //rainbow = true;
        } else if (payload[0] == 'N') { // the browser sends an N when the rainbow effect is disabled
            //rainbow = false;
        }
        break;
    default:
        break;
    }
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("\n Starting");

    pinMode(TRIGGER_PIN, INPUT);
    if (digitalRead(TRIGGER_PIN) == LOW) {
        configureWiFi();
    } else {
        WiFi.begin();
        Serial.print("Connecting");
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        EEPROM.begin(128);

        EEPROM.get(0, storage);
        Serial.print("storage.color ");
        Serial.print(storage.color);
        Serial.println();

        // EEPROM.put(0, u);
        // EEPROM.commit();

        strip.begin();
        strip.show(); // Initialize all pixels to 'off'

        status_led.begin();
        status_led.show();

        SPIFFS.begin();

        server.onNotFound([]() {
            if (!handleFileRead(server.uri()))
                server.send(404, "text/plain", "404: Not Found");
        });

        server.on("/", []() {
            handleFileRead("/index.html");
        });

        server.on("/red", []() { changeColor(strip.Color(255, 0, 0)); });
        server.on("/green", []() { changeColor(strip.Color(0, 255, 0)); });
        server.on("/blue", []() { changeColor(strip.Color(0, 0, 255)); });

        if (!MDNS.begin("nchl")) {
            Serial.println("Error setting up MDNS responder!");
        }

        webSocket.begin();
        webSocket.onEvent(webSocketEvent);
        Serial.println("WebSocket server started.");

        server.begin();
        Serial.println("HTTP server started");
    }
}

void handleLights()
{
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, storage.color);
        strip.show();
        //    delay(wait);
    }
    status_led.setPixelColor(0, storage.color);
    status_led.show();
}

void loop()
{
    server.handleClient();
    webSocket.loop();
    handleLights();
}
