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

#include <time.h>

#include "data.h"
#include "colors.h"

const int TRIGGER_PIN = D8;
const int LIGHTS_PIN = D4;

constexpr int number_of_leds = 7;

Data data;
Colors led_colors(number_of_leds);
unsigned long needs_save = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(number_of_leds, LIGHTS_PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

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

    if (!wifiManager.startConfigPortal("konfiguracja_swiatelek"))
    {
        Serial.println("failed to connect and hit timeout");
    }
    Serial.println("Reconfiguration done");
}

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

void updateStorage()
{
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
        path += "index.html";                  // If a folder is requested, send the index file
    String contentType = getContentType(path); // Get the MIME type
    if (SPIFFS.exists(path))
    {                                                       // If the file exists
        File file = SPIFFS.open(path, "r");                 // Open it
        server.streamFile(file, contentType);               // And send it to the client
        file.close();                                       // Then close the file again
        return true;
    }
    Serial.println("\tFile Not Found");
    return false; // If the file doesn't exist, return false
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght)
{
    switch (type)
    {
    case WStype_DISCONNECTED: // if the websocket is disconnected
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    { // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        std::string send = "{\"server_ip\": \"";
        send += WiFi.localIP().toString().c_str();
        send += "\", " + data.to_string(true);
        send += "}";
        Serial.printf("Sending: %s\n", send.c_str());
        webSocket.sendTXT(num, send.c_str(), send.size());
        // rainbow = false;                  // Turn rainbow off when a new connection is established
    }
    break;
    case WStype_TEXT: // if new text data is received
        Serial.printf("[%u] get Text: %s\n", num, payload);
        data.from_string(std::string((const char *)payload, lenght));
        if (strip.numPixels() != data.number_of_leds())
        {
            strip.updateLength(data.number_of_leds());
        }
        needs_save = millis() + 5000;
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
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
        configureWiFi();
    }
    else
    {
        WiFi.begin();
        Serial.print("Connecting");
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        EEPROM.begin(Data::max_data_size);

        char buf[Data::max_data_size];
        EEPROM.get(0, buf);
        Serial.printf("Loading: %s\n", buf);
        data.from_string(buf);

        strip.begin();
        strip.show(); // Initialize all pixels to 'off'

        SPIFFS.begin();

        server.onNotFound([]() {
            if (!handleFileRead(server.uri()))
                server.send(404, "text/plain", "404: Not Found");
        });

        server.on("/", []() {
            handleFileRead("/index.html");
        });

        if (!MDNS.begin("swiatelka"))
        {
            Serial.println("Error setting up MDNS responder!");
        }

        configTime(1 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        Serial.println("\nWaiting for time");
        while (!time(nullptr))
        {
            Serial.print(".");
            delay(1000);
        }
        Serial.println("");

        webSocket.begin();
        webSocket.onEvent(webSocketEvent);
        Serial.println("WebSocket server started.");

        server.begin();
        Serial.println("HTTP server started");
    }
}

void save_data()
{
    if (needs_save > 0 && needs_save < millis())
    {
        needs_save = 0;

        std::string to_save = data.to_string(false);
        char buf[Data::max_data_size];
        memcpy(buf, to_save.c_str(), to_save.length());
        buf[to_save.length()] = '\0';
        Serial.printf("Saving: %s\n", buf);

        EEPROM.put(0, buf);
        EEPROM.commit();
    }
}

void handle_colors()
{
    if (colors(led_colors, data))
    {
        strip.setBrightness(data.brightness());
        for (uint16_t i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, led_colors[i]);
        }
        strip.show();
    }
}

void loop()
{
    server.handleClient();
    webSocket.loop();

    handle_colors();
    save_data();

}
