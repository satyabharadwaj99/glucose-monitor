#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Broker settings
const char* mqtt_broker = "broker.hivemq.com";
const char* mqtt_topic = "glucose_monitor/data";
const int mqtt_port = 1883;
const char* mqtt_client_id = "esp32_glucose_monitor";

// Alert thresholds
const float LOW_GLUCOSE = 80.0;
const float HIGH_GLUCOSE = 150.0;
const float CRITICAL_LOW = 70.0;
const float CRITICAL_HIGH = 180.0;

// LED Pins for alerts
const int RED_LED = 12;    // Critical alerts
const int YELLOW_LED = 14; // Warnings
const int GREEN_LED = 27;  // Normal

// NTP settings
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

WiFiClient espClient;
PubSubClient client(espClient);

// Moving average for glucose smoothing
const int WINDOW_SIZE = 5;
float readings[WINDOW_SIZE];
int readIndex = 0;
float total = 0;
float average = 0;

// Function to generate random glucose values with trends
float getRandomGlucose() {
  static float lastValue = 100.0; // Start at 100 mg/dL
  
  // Add some trend (slowly go up or down)
  static float trend = 0;
  if (random(100) < 20) { // 20% chance to change trend
    trend = random(-10, 11) / 10.0; // Trend between -1 and +1 mg/dL per reading
  }
  
  // Add noise and trend
  float noise = -2.0 + (random(0, 400) / 100.0);
  float newValue = lastValue + trend + noise;
  
  // Keep within realistic bounds
  newValue = constrain(newValue, CRITICAL_LOW, CRITICAL_HIGH);
  lastValue = newValue;
  
  // Update moving average
  total = total - readings[readIndex];
  readings[readIndex] = newValue;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % WINDOW_SIZE;
  
  average = total / WINDOW_SIZE;
  return average;
}

void updateLEDs(float glucose) {
  // Turn off all LEDs first
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  
  // Critical alerts
  if (glucose <= CRITICAL_LOW || glucose >= CRITICAL_HIGH) {
    digitalWrite(RED_LED, HIGH);
  }
  // Warnings
  else if (glucose <= LOW_GLUCOSE || glucose >= HIGH_GLUCOSE) {
    digitalWrite(YELLOW_LED, HIGH);
  }
  // Normal
  else {
    digitalWrite(GREEN_LED, HIGH);
  }
}

void setup_wifi() {
  delay(10);
  Serial.println("\nConnecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      
      // Subscribe to control topic
      client.subscribe("glucose_monitor/control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming messages
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);
}

void setup() {
  Serial.begin(115200);
  
  // Initialize LED pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Initialize moving average
  for (int i = 0; i < WINDOW_SIZE; i++) {
    readings[i] = 100.0; // Initialize with normal values
    total += readings[i];
  }
  
  randomSeed(analogRead(0));
  
  setup_wifi();
  timeClient.begin();
  timeClient.setTimeOffset(19800); // UTC+5:30 for India
  
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
  // Initial LED test
  digitalWrite(RED_LED, HIGH);
  delay(500);
  digitalWrite(YELLOW_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, HIGH);
  delay(500);
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  timeClient.update();

  // Generate and send data every 5 seconds
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Generate random glucose value
    float glucoseValue = getRandomGlucose();
    
    // Update LEDs based on glucose level
    updateLEDs(glucoseValue);
    
    // Create JSON document
    StaticJsonDocument<200> doc;
    doc["glucose"] = glucoseValue;
    doc["timestamp"] = timeClient.getEpochTime();
    doc["device_id"] = mqtt_client_id;
    
    // Add alert status
    if (glucoseValue <= CRITICAL_LOW) {
      doc["alert"] = "CRITICAL_LOW";
    } else if (glucoseValue >= CRITICAL_HIGH) {
      doc["alert"] = "CRITICAL_HIGH";
    } else if (glucoseValue <= LOW_GLUCOSE) {
      doc["alert"] = "LOW";
    } else if (glucoseValue >= HIGH_GLUCOSE) {
      doc["alert"] = "HIGH";
    } else {
      doc["alert"] = "NORMAL";
    }
    
    // Serialize to JSON string
    char jsonString[200];
    serializeJson(doc, jsonString);
    
    // Publish to MQTT topic
    client.publish(mqtt_topic, jsonString);
    
    // Print debug information
    Serial.print("Published glucose value: ");
    Serial.print(glucoseValue);
    Serial.print(" mg/dL at ");
    Serial.println(timeClient.getFormattedTime());
  }
}
