#include <Arduino.h>
#include <Servo.h>
//https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html
#include <ESP8266WiFi.h>
#include <time.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Ticker.h>
#include "secrets.h"

int servoPin = 4;
int buttonPin = 12;
int buttonState = 0;
int WIFI_INDICATOR = 15;

Servo servoMotor;
BearSSL::WiFiClientSecure client;

BearSSL::X509List clientCertList(client_cert);
BearSSL::PrivateKey key(privkey);
BearSSL::X509List serverTrustedCA(cacert);

PubSubClient pubSubClient(client);

const char MQTT_SUB_TOPIC[] = "$aws/things/" THINGNAME "/shadow/update";
const char MQTT_PUB_TOPIC[] = "$aws/things/" THINGNAME "/shadow/update";

void connectToWiFi()
{
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(WIFI_INDICATOR, HIGH);
        Serial.print(".");
        delay(1000);
        digitalWrite(WIFI_INDICATOR, LOW);
    }

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(WIFI_INDICATOR, HIGH);
}

void setClock()
{
  configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void pubSubErr(int8_t MQTTErr)
{
  if (MQTTErr == MQTT_CONNECTION_TIMEOUT)
    Serial.print("Connection tiemout");
  else if (MQTTErr == MQTT_CONNECTION_LOST)
    Serial.print("Connection lost");
  else if (MQTTErr == MQTT_CONNECT_FAILED)
    Serial.print("Connect failed");
  else if (MQTTErr == MQTT_DISCONNECTED)
    Serial.print("Disconnected");
  else if (MQTTErr == MQTT_CONNECTED)
    Serial.print("Connected");
  else if (MQTTErr == MQTT_CONNECT_BAD_PROTOCOL)
    Serial.print("Connect bad protocol");
  else if (MQTTErr == MQTT_CONNECT_BAD_CLIENT_ID)
    Serial.print("Connect bad Client-ID");
  else if (MQTTErr == MQTT_CONNECT_UNAVAILABLE)
    Serial.print("Connect unavailable");
  else if (MQTTErr == MQTT_CONNECT_BAD_CREDENTIALS)
    Serial.print("Connect bad credentials");
  else if (MQTTErr == MQTT_CONNECT_UNAUTHORIZED)
    Serial.print("Connect unauthorized");
}

void connectToMqtt(bool nonBlocking = false)
{
    Serial.print("MQTT connecting ");
    while (!client.connected())
    {
        if (pubSubClient.connect(THINGNAME))
        {
            Serial.println("connected!");

            if (!pubSubClient.subscribe(MQTT_SUB_TOPIC))
                pubSubErr(pubSubClient.state());
            }
        else
        {
            Serial.print("failed, reason -> ");
            pubSubErr(pubSubClient.state());
            int errCode = client.getLastSSLError();
            Serial.print("ERROR_CODE:");
            Serial.println(errCode);
            Serial.println(" < try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    delay(15000);

    Serial.begin(115200);
    Serial.println();

    pinMode(WIFI_INDICATOR, OUTPUT);
    //Not sure why the name isn't taking.
    WiFi.hostname(THINGNAME);

    Serial.printf("hostname set to: %s\n", WiFi.hostname().c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    connectToWiFi();

    servoMotor.attach(servoPin);
    pinMode(buttonPin, INPUT);

    setClock();

    //This is important to be called first
    client.setTrustAnchors(&serverTrustedCA);
    client.setClientRSACert(&clientCertList, &key);

    pubSubClient.setServer(MQTT_HOST, MQTT_PORT);
    pubSubClient.setCallback(messageReceived);

    connectToMqtt();
}



void loop(){
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == HIGH) {
    servoMotor.write(90);
  } else {
    servoMotor.write(0);
  }
}
