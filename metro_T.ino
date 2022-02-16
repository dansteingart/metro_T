#include <Arduino.h>
#include <WiFiNINA.h>
#include <math.h> // (no semicolon)
#include "PubSubClient.h"
#include "secrets.h"
#include <ArduinoJson.h>

//setup mqtt
WiFiClient espClient;
PubSubClient client(espClient); //lib required for mqtt;

void setup()
{
    analogReadResolution(14); //set read to 14 bits

    Serial.begin(115200);
    Serial.println();

    WiFi.begin(SSID, WPA);

    while (WiFi.status() != WL_CONNECTED) {delay(500);Serial.print(".");}
    Serial.print("WiFi connected: "); Serial.println(WiFi.localIP());

    //start MQTT stuff
    client.setServer(MQTT, MQTT_PORT);//connecting to mqtt server
    client.setCallback(callback);

}

void callback(char* topic, byte* payload, unsigned int length) {   //callback includes topic and payload ( from which (topic) the payload is comming)
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  StaticJsonDocument<1024> doc;
  deserializeJson(doc, payload, length);
  //Serial.println(doc);
  serializeJsonPretty(doc, Serial);

}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("wifikit32")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("steingart_lab/status/wifikit32", "We're here!");
      // ... and resubscribe
      client.subscribe("steingart_lab/data/dan/#");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop()
{
    StaticJsonDocument<256> docout;

    if (!client.connected()){reconnect();}
    client.loop();
    delay(5000);

    float a0 = analogRead(0);

    float beta = 3950; //factor
    float T0 = 298; //K
    float R0 = 10000; //ohms

    // calculate R

    ///I = Vin/(RT + RasdF) = Vout/RF
    // a0 = analog input

    float R = ((16384/a0) - 1)*R0;
    float f1 = (1/T0)+(1/beta)*log(R/R0);
    float T = 1/f1;

    char buffer[256];
    docout["T"] = T;
    size_t n = serializeJson(docout, buffer);
    Serial.println(buffer);
    client.publish("steingart_lab/data/dan/metro_test", buffer, n);

}
