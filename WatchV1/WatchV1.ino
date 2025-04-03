#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <TimeLib.h> // Include the Time library

// OLED Display Config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WiFi Credentials
const char* ssid = "Das_House_lite";
const char* password = "2444666668888888";

// NTP Config
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 6 * 3600, 60000); // UTC+6

// EEPROM Size
#define EEPROM_SIZE 64
int lastSavedHour = -1;  // Store the last saved hour

#include <Fonts/FreeSansBold9pt7b.h> // Include a built-in font
#include <Fonts/Picopixel.h>

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    timeClient.begin();
    timeClient.update();
    lastSavedHour = EEPROM.read(0);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        while (1);
    }
    display.clearDisplay();
}

void loop() {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    setTime(epochTime);

    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    bool isPM = hours >= 12;
    hours = (hours > 12) ? hours - 12 : (hours == 0) ? 12 : hours;

    String formattedDate = formatDate(epochTime);
    displayTime(formattedDate, hours, minutes, isPM);

    if (WiFi.status() == WL_CONNECTED && timeClient.getHours() != lastSavedHour) {
        EEPROM.write(0, timeClient.getHours());
        EEPROM.write(1, minutes);
        EEPROM.commit();
        lastSavedHour = timeClient.getHours();
        Serial.printf("Time saved to EEPROM: %02d:%02d\n", hours, minutes);
    }

    delay(1000);
}

void displayTime(const String& date, int hours, int minutes, bool isPM) {
    display.clearDisplay();
    drawBatteryIcon(110, 5, 50); // Draw battery icon at (x:110, y:5)
    if (WiFi.status() == WL_CONNECTED) {
        drawWifiIcon(90, 5); // Draw Wi-Fi icon at (x:90, y:5)
    }

    display.setFont(&FreeSansBold9pt7b);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 62);
    display.printf("%s", date.c_str());

    display.setTextSize(2);
    display.setCursor(16, 40);
    display.printf("%02d:%02d", hours, minutes);

    display.setFont(&Picopixel);
    display.setTextSize(1);
    display.setCursor(110, 40);
    display.printf("%s", isPM ? "PM" : "AM");
    display.display();
}

void drawBatteryIcon(int x, int y, int level) {
    // Draw battery outline
    display.drawRect(x, y, 16, 8, SSD1306_WHITE); // Battery body
    display.fillRect(x + 16, y + 2, 3, 4, SSD1306_WHITE); // Battery terminal

    // Fill battery level based on level (0-100%)
    int fillWidth = map(level, 0, 100, 0, 16);
    display.fillRect(x + 1, y + 1, fillWidth, 6, SSD1306_WHITE);
}

void drawWifiIcon(int x, int y) {
    // Draw Wi-Fi icon using lines
    display.drawLine(x + 5, y + 2, x + 5, y + 2, SSD1306_WHITE); // Top line (dot)
    display.drawLine(x + 3, y + 4, x + 7, y + 4, SSD1306_WHITE); // Middle line
    display.drawLine(x + 1, y + 6, x + 9, y + 6, SSD1306_WHITE); // Bottom line

    
}

String formatDate(unsigned long epochTime) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d-%02d-%04d", day(epochTime), month(epochTime), year(epochTime));
    return String(buffer);
}
