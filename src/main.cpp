
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EasyHTTP.h>

// OneWire setup for the DS18B20 sensor
const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// OLED display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// API connection setup
String baseURL = "http://192.168.1.5:5000";
EasyHTTP http("Baynosa", "Peninsula.1");

void setup()
{
	// Start the Serial Monitor
	Serial.begin(9600);
	// Start the DS18B20 sensor
	sensors.begin();
	// Start the OLED display
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			;
	};

	// Connect to the API
	http.connectWiFi();
	http.setBaseURL(baseURL);

	// Register as monitor to the api
	String response = http.post("/monitors");
}

void loop()
{
	// Start the DS18B20 sensor
	sensors.requestTemperatures();
	// Get the temperature in Celsius
	float tempC = sensors.getTempCByIndex(0);
	// Print the temperature in Celsius
	Serial.print("Temperature: ");
	Serial.print(tempC);
	Serial.println("°C");

	// Print in the center of the OLED display
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);
	display.print("Temperature: ");
	display.print(tempC);
	display.println("C");
	display.display();
	delay(500);
}