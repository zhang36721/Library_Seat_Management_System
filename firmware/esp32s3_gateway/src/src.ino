#include "app_gateway.h"
#include <Arduino.h>

#define BOARD_RGB_LED_PIN 48

static void board_led_off_early()
{
    pinMode(BOARD_RGB_LED_PIN, OUTPUT);
    digitalWrite(BOARD_RGB_LED_PIN, LOW);
}

void setup()
{
    board_led_off_early();
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("=== ESP32S3 Gateway v0.9.1 ===");
    Serial.println("[BOARD] RGB LED off requested on GPIO48 before UART init");
    app_gateway_begin();
}

void loop()
{
    app_gateway_task();
    delay(2);
}
