/*
#include <Arduino.h>


void setup() {
  pinMode(GPIO_NUM_2,OUTPUT);
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'
}

void loop() {
  digitalWrite(GPIO_NUM_2,HIGH);
  strip.setPixelColor(1, strip.Color(100,100,100));
  strip.show();
  delay(1000);
  digitalWrite(GPIO_NUM_2,LOW);
  strip.setPixelColor(1, strip.Color(0,0,0));
  strip.show();
  delay(1000);
}*/