/*
 * Wiring: (from taligentx/dscKeybusInterface)
 *      DSC Aux(+) --- 5v voltage regulator --- esp8266 development board 5v pin
 *
 *      DSC Aux(-) --- esp8266 Ground
 *
 *                                         +--- dscClockPin
 *      DSC Yellow --- 33k ohm resistor ---|
 *                                         +--- 10k ohm resistor --- Ground
 *
 *                                         +--- dscReadPin
 *      DSC Green ---- 33k ohm resistor ---|
 *                                         +--- 10k ohm resistor --- Ground
 *
 *  Virtual keypad (optional):
 *      DSC Green ---- NPN collector --\
 *                                      |-- NPN base --- 1k ohm resistor --- dscWritePin
 *            Ground --- NPN emitter --/
 *
 *  Virtual keypad uses an NPN transistor to pull the data line low - most small signal NPN transistors should
 *  be suitable, for example:
 *   -- 2N3904, BC547, BC548, BC549
 */

#include <ESP8266WiFi.h>
#include <dsc/dsc.h>

#define WIFI_SSID "wifi"
#define WIFI_PASSWORD "password"

const MQTTConfig mqttConfig {
	.ip = "192.168.0.2"
};

const HAConfig haConfig {
	.discoveryPrefix = "homeassistant"
};

const HADevice deviceConfig {
	.name = "DSC-ESP",
	.manufacturer = "Espressif",
	.model = "ESP8266"
};

const DSCConfig dscConfig {
	.wiring {
		.clockPin = 4,
		.readPin = 5,
		.writePin = 15,
	},
	.partitions {
		{
			.name = "Ground floor",
			.zones = {
				{
					.name = "Front door",
					.type = HABinarySensor_DeviceClass::door
				},
				{
					.name = "Back door",
					.type = HABinarySensor_DeviceClass::door
				}
			}
		}
	},
	.accessCode = "1234"
};

WiFiClient netClient;
DSC dsc(dscConfig, netClient, mqttConfig, haConfig, deviceConfig);

void setup() {
	Serial.begin(115200);
	Serial.println();

	//Network setup
	Serial.print(F("Connecting to: "));
	Serial.print(WIFI_SSID);
	WiFi.mode(WIFI_STA);
	WiFi.hostname(deviceConfig.name);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(F("."));
		delay(1000);
	}
	Serial.println();
	Serial.print(F("Network connected. Local IP: "));
	Serial.println(WiFi.localIP());
	String mac = WiFi.macAddress();

	dsc.setup(mac, mac);
}

void loop() {
	dsc.loop();
}
