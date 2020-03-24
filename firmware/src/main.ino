// --------------------------------------
//  Hex lamp WS2812 engine
// --------------------------------------
// based on perlin noise controlling H, S, V
//
// Build / Upload / Monitor it with
// $ platformio run -t upload -t monitor
//
// Rudimentary MQTT interface:
// mosquitto_pub -t "hex_lamp/intensity" -m 0.3
// printf '\x51\x11\x22' | mosquitto_pub -t "hex_lamp/raw" -l

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "perlin.h"

// WS2812 black LED strips
Adafruit_NeoPixel strip(N_LEDS, PIN_DATA, NEO_GRB + NEO_KHZ800);

// MQTT stuff
WiFiClient wifi_client;
PubSubClient client(wifi_client);

// how likely an LED is lit [0.0 .. 1.0]
float g_intensity = 0.5;

// timeout counter: how long to display hex_lamp/raw pixels
unsigned raw_mode = 0;

// Perlin noise seeds
int seed1, seed2, seed3;

// On received MQTT message
void mqtt_cb(char* topic, byte* payload, unsigned int length) {
	Serial.printf("MQTT [%s] ", topic);
	for (unsigned i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();

	if (strcmp(topic, MQTT_PREFIX "/raw") == 0) {
		// Write raw binary payload to LED strip
		unsigned n = length / 3;

		if (n <= 0)
			return;

		byte *p = payload;

		if (n == 1 || n > N_LEDS)
			n = N_LEDS;

		for (unsigned i=0; i<n; i++) {
			strip.setPixelColor(i, p[0], p[1], p[2]);
			if (length > 3)
				p += 3;
		}

		raw_mode = 1500;  // Raw mode timeout = 1 min
		strip.show();
		return;

	} else if (strcmp(topic, MQTT_PREFIX "/intensity") == 0) {
		// Convert payload to float, set g_intensity
		payload[length] = 0;  // nasty!

		float val = atof((char*)payload);

		if (val >= 0.0 && val <= 1.0) {
			g_intensity = val;
			Serial.printf("g_intensity = %f\n", val);
		}
	}
}

void setup() {
	Serial.begin(115200);

	strip.begin();
	strip.clear();
	strip.setPixelColor(0, 0xFF, 0, 0);
	strip.show();

	client.setServer(MQTT_BROKER, 1883);
	client.setCallback(mqtt_cb);

	WiFi.begin(WIFI_NAME, WIFI_PW);
	Serial.printf("\nThis is hexlamp, connecting to %s \n", WIFI_NAME);

	for (int i=0; i<=500; i++) {
		if (WiFi.status() == WL_CONNECTED) {
			strip.setPixelColor(0, 0, 0xFF, 0);
			strip.show();
			Serial.print("\nConnected! IP: ");
			Serial.println(WiFi.localIP());
			break;
		}
		delay(100);
		Serial.print(".");
	}

	srand(RANDOM_REG32);
	seed1 = rand();
	seed2 = rand();
	seed3 = rand();
}

void mqtt_reconnect() {
	if (WiFi.status() != WL_CONNECTED)
		return;
	Serial.printf("Connecting to MQTT broker %s ... ", MQTT_BROKER);
	if (client.connect(MQTT_BROKER)) {
		Serial.printf("connected!\n");
		Serial.printf("Subscribing to %s\n", MQTT_PREFIX "/#");
		client.subscribe(MQTT_PREFIX "/#");
		return;
	}
	Serial.printf("failed :(\n");
}

// clip b such that: a <= b <= c
static float limit(float a, float b, float c) {
	return ((a > b) ? a : (b > c) ? c : b);
}

void loop() {
	static int tick=0;
	float n1, n2, n3, n3_raw;

	if (!client.connected() && ((tick % 500) == 0)) {
		mqtt_reconnect();
	}
	client.loop();

	if (raw_mode > 0) {
		raw_mode--;
	} else {
		for (int i=0; i<N_LEDS; i++) {
			// Range approximately -1 .. +1
			n1 = pnoise2d(3.1 * i, 0.002 * tick, 0.7, 3, seed1);
			n2 = pnoise2d(7.4 * i, 0.002 * tick, 0.7, 3, seed2);
			n3_raw = pnoise2d(8.9 * i, 0.004 * tick, 0.7, 3, seed3);

			// Color: Shift range to ~ 0 .. +1
			n1 = n1 / 2.0 + 0.5;

			// Saturation: The more positive offset, the more colorful
			n2 = n2 + 0.8;
			n2 = limit(0.0, n2, 1.0);

			// Brightness: negative offset = less likely to light up a LED
			n3_raw = (n3_raw + (g_intensity - 0.75) * 2.0) * 3.0;
			n3 = limit(0.0, n3_raw, 1.0);

			strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(
				n1 * 0xFFFF,
				n2 * 0xFF,
				n3 * 0xFF
			)));
		}
		strip.show();
		Serial.printf(
			"%6d: %6.3f, %6.3f, %6.3f (%6.3f)\n",
			tick, n1, n2, n3, n3_raw
		);
	}

	tick++;
	delay(40);
}
