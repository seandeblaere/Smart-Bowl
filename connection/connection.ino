#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <HX711_ADC.h>
#include "arduino_secrets.h"

// WiFi and MQTT settings
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "192.168.50.74";
int port = 1883;
const char topicWeight[] = "arduino/smartbowl/weight";
const char topicFeed[] = "arduino/smartbowl/feed";
const char topicCooldown[] = "arduino/smartbowl/cooldown";
float targetWeight = 500; // Default target weight
bool feedDog = false;

// Cooldown settings
unsigned long cooldownTime = 0; // in milliseconds
unsigned long lastFeedTime = 0;

// Pins for RFID
#define SS_PIN 12
#define RST_PIN 11

// Pins for Load Cell
const int HX711_dout = 2; // MCU > HX711 DOUT pin
const int HX711_sck = 3;  // MCU > HX711 SCK pin

// Calibration factor for load cell
const float calibration_factor = -103.219; // Known calibration factor

// RFID and Servo objects
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
Servo myservo; // Create servo object to control a servo

// Load Cell object
HX711_ADC LoadCell(HX711_dout, HX711_sck);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize SPI and RFID
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  // Attach and initialize the servo
  myservo.attach(9);
  myservo.write(93); // Stop the motor initially

  // Initialize Load Cell
  LoadCell.begin();
  unsigned long stabilizingtime = 2000; // Stabilizing time after power-up
  bool _tare = true; // Perform tare operation during startup
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  } else {
    LoadCell.setCalFactor(calibration_factor); // Set the known calibration factor
    Serial.println("HX711 initialization completed");
    float startWeight = LoadCell.getData();
    Serial.print("Starting Weight: ");
    Serial.print(startWeight);
    Serial.println("g");
  }

  // Initialize WiFi
  Serial.print("Poging om verbinding te maken met WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Verbinding met het netwerk is geslaagd");
  Serial.println();

  // Initialize MQTT
  Serial.print("Poging om verbinding te maken met de MQTT broker: ");
  Serial.println(broker);
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT verbinding mislukt! Foutcode = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }
  Serial.println("Je bent verbonden met de MQTT broker!");
  Serial.println();
  mqttClient.onMessage(onMessageReceived);
  mqttClient.subscribe(topicWeight);
  mqttClient.subscribe(topicFeed);
  mqttClient.subscribe(topicCooldown);
  LoadCell.setSamplesInUse(4);
}

void loop() {
  // Poll MQTT messages occasionally
  static unsigned long lastMqttPoll = 0;
  if (millis() - lastMqttPoll > 1000) { // Poll every second
    mqttClient.poll();
    lastMqttPoll = millis();
  }

  LoadCell.update();

  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (millis() - lastFeedTime >= cooldownTime) {
      // Show UID on serial monitor
      Serial.print("UID tag :");
      String content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println();
      Serial.print("Message : ");
      content.toUpperCase();

      // Check for authorized UID
      if (content.substring(1) == "53 A0 49 34") { // Change here the UID of the card/cards that you want to give access
        Serial.println("Authorized access");
        feed();
        lastFeedTime = millis(); // Update the last feed time
      } else {
        Serial.println("Access denied");
      }
    } else {
      unsigned long remainingTime = cooldownTime - (millis() - lastFeedTime);
      unsigned long remainingMinutes = (remainingTime / 1000) / 60;
      unsigned long remainingSeconds = (remainingTime / 1000) % 60;
      Serial.print("Cooldown time active. Try again in ");
      Serial.print(remainingMinutes);
      Serial.print(" minutes and ");
      Serial.print(remainingSeconds);
      Serial.println(" seconds.");
    }
    delay(500); // Wait for a while before scanning for the next card
  }

  // Check if the dog should be fed without RFID
  if (feedDog) {
    feedDog = false; // Reset the flag
    feed();
    lastFeedTime = millis(); // Update the last feed time
  }
}

void onMessageReceived(int messageSize) {
  String message = "";
  while (mqttClient.available()) {
    char c = (char)mqttClient.read();
    message += c;
  }
  Serial.print("Ontvangen bericht: ");
  Serial.println(message);

  // Check if the message is the "feed" command
  if (message == "feed") {
    feedDog = true;
    Serial.println("Feed dog command received");
  }
  // Check if the message can be converted to a float (for weight setting)
  else if (message.indexOf(":") != -1) {
    int separator = message.indexOf(":");
    int hours = message.substring(0, separator).toInt();
    int minutes = message.substring(separator + 1).toInt();
    cooldownTime = (hours * 3600UL + minutes * 60UL) * 1000UL; // Convert to milliseconds
    Serial.print("New cooldown time: ");
    Serial.print(hours);
    Serial.print(" hours and ");
    Serial.print(minutes);
    Serial.println(" minutes.");
  } else {
    float weight = message.toFloat();
    if (weight != 0.0 || message == "0" || message == "0.0") {
      targetWeight = weight;
      Serial.print("Nieuwe target weight: ");
      Serial.println(targetWeight);
    } else {
      Serial.println("Geen geldig bericht ontvangen.");
    }
  }
}

void feed() {
  myservo.write(87); // Rotate the motor counter-clockwise

  // Keep rotating until weight reaches target weight
  while (true) {
    LoadCell.update(); // Update load cell data
    float weight = LoadCell.getData();
    Serial.print("Current weight: ");
    Serial.print(weight);
    Serial.println("g");

    if (weight >= targetWeight - 35) {
      break;
    }

    delay(10); // Small delay to prevent overload
  }

  myservo.write(93); // Stop the motor
}












