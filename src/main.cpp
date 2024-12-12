
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
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
String API_URL = "http://192.168.9.171:5000";
EasyHTTP http("Baynosa", "Peninsula.1");

// Monitor setup
String monitorId;
boolean codeRequested = false;
String code;
boolean monitorStarted = false;

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
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.println("Connecting to API...");
	display.display();
	http.connectWiFi();
	http.setBaseURL(API_URL);

	{
		// Register as monitor to the api
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println("Registering monitor...");
		String response = http.post("/monitors");
		Serial.println(response);

		JsonDocument doc;
		DeserializationError error = deserializeJson(doc, response);
		if (error)
		{
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.c_str());
			display.clearDisplay();
			display.setCursor(0, 0);
			display.println("Failed to register monitor");
			display.display();
			return;
		};

		// Set the monitor ID
		monitorId = doc["id"].as<String>();
	};

	// Display the IP address
	display.clearDisplay();
	display.setCursor(0, 0);
	display.println(WiFi.localIP());
	display.display();

	delay(1000);

	// Request the code
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println("Requesting code...");
		display.display();
		String response = http.post("/monitors/code/" + monitorId);
		Serial.println(response);

		JsonDocument doc;
		DeserializationError error = deserializeJson(doc, response);
		if (error)
		{
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.c_str());
			display.clearDisplay();
			display.setCursor(0, 0);
			display.println("Failed to request code");
			display.display();
			return;
		};

		// Set the code
		code = doc["code"].as<String>();
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println("Enter code:");
		display.setTextSize(2);
		display.setCursor(0, 10);
		display.println(code);
		display.display();
		codeRequested = true;
	};
};

void loop() {
	if (!codeRequested) return;

	// Send the temperature to the API
	sensors.requestTemperatures();
	float temperature = sensors.getTempCByIndex(0);
	String response = http.post("/monitors/" + monitorId + "/temperature", "{\"temperature\":" + String(temperature) + "}");
	Serial.println(response);

	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, response);
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		display.clearDisplay();
		display.setCursor(0, 0);
		display.println("Failed to send temperature");
		display.display();
		return;
	};

	// Check if the monitor is connected
	if (doc["connected"].as<boolean>() == true)
	{
		monitorStarted = true;
	};

	if (monitorStarted) {// Display the temperature
		display.clearDisplay();
		display.setCursor(0, 0);
		display.setTextSize(1);
		display.println("Temperature:");
		display.setTextSize(2);
		display.setCursor(0, 10);
		display.println(temperature);
		display.display();
	};
};
