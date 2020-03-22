// --------------------------------------
//  Hex lamp WS2812 engine
// --------------------------------------
// based on perlin noise controlling H, S, V
//
// Build / Upload / Monitor it with
// $ platformio run -t upload -t monitor

// #include <ESP8266mDNS.h>
// #include <ArduinoOTA.h>

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "perlin.h"

#define PIN_DATA 3
#define N_LEDS 26

// WS2812 black LED strips
Adafruit_NeoPixel strip(N_LEDS, PIN_DATA, NEO_GRB + NEO_KHZ800);

int seed1, seed2, seed3;

void setup() {
	Serial.begin(115200);

	strip.begin();
	strip.setPixelColor(0, 0xFF, 0, 0);
	strip.show();

	WiFi.begin(WIFI_NAME, WIFI_PW);
	Serial.printf("\nThis is hexlamp, connecting to %s \n", WIFI_NAME);
	while (WiFi.status() != WL_CONNECTED) {
		delay(100);
		Serial.print(".");
	}

	strip.setPixelColor(0, 0, 0xFF, 0);
	strip.show();

	Serial.print("\nConnected! IP: ");
	Serial.println(WiFi.localIP());

	srand(RANDOM_REG32);

    seed1 = rand();
    seed2 = rand();
    seed3 = rand();
}

// clip b such that: a <= b <= c
#define LIMIT(a, b, c) ((a > b) ? a : (b > c) ? c : b)

void loop() {
	static int tick=0;
    float n1, n2, n3, n3_raw;

	for (int i=0; i<N_LEDS; i++) {
		// Range approximately -1 .. +1
        n1 = pnoise2d(3.1 * i, 0.002 * tick, 0.7, 3, seed1);
        n2 = pnoise2d(7.4 * i, 0.002 * tick, 0.7, 3, seed2);
        n3_raw = pnoise2d(8.9 * i, 0.004 * tick, 0.7, 3, seed3);

        // Color: Shift range to ~ 0 .. +1
        n1 = n1 / 2.0 + 0.5;

        // Saturation: The more positive offset, the more colorful
        n2 = n2 + 0.8;
		n2 = LIMIT(0.0, n2, 1.0);

        // Brightness: negative offset = less likely to light up a LED
        n3 = n3_raw * 3 - 2.2;
		n3 = LIMIT(0.0, n3, 1.0);

 		strip.setPixelColor(i, strip.ColorHSV(
			n1 * 0xFFFF,
			n2 * 0xFF,
			n3 * 0xFF
		));
	}

	// Serial.printf(
	// 	"%6d: %6.3f, %6.3f, %6.3f (%6.3f)\n",
	// 	tick, n1, n2, n3, n3_raw
	// );

	strip.show();
	tick++;
	delay(40);
}
