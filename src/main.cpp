#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>

#include "secrets.h"

int servoPin = 4;
int buttonPin = 12;
int buttonState = 0;

Servo Servo1;
void setup() {
    delay(15000);

    Serial.begin(115200);
    Serial.println();

    //Not sure why the name isn't taking.
    WiFi.hostname(THINGNAME);

    Serial.printf("hostname set to: %s\n", WiFi.hostname().c_str());

    WiFi.begin(ssid, pass);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    Servo1.attach(servoPin);
    pinMode(buttonPin, INPUT);
}

void loop(){
    buttonState = digitalRead(buttonPin);

    if (buttonState == HIGH) {
        Servo1.write(90);
        //Serial.println("ON");
    } else {
        Servo1.write(0);
        Serial.println("push");
    }
}