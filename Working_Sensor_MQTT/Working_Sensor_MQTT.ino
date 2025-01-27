#include <WiFi.h>  
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Pin Definitions
const int MOISTURE_PIN = 7;
const int DHT_PIN = 42;
const int RAIN_PIN = 6;
const int GREEN_LED_PIN = 9;
const int RED_LED_PIN = 5;

// Soil Moisture Mapping
int MinMoistureValue = 4095;
int MaxMoistureValue = 2060;
int MinMoisture = 0;
int MaxMoisture = 100;

// WiFi and MQTT Configuration
const char* ssid = "Ryan";
const char* password = "1234567890";
const char* mqtt_server = "104.197.97.42";  // Your VM's external IP
const int mqtt_port = 1883;
const char* mqtt_user = "";     // If you have MQTT authentication
const char* mqtt_password = ""; // If you have MQTT authentication

// MQTT Topics
const char* TOPIC_MOISTURE = "sensors/moisture";
const char* TOPIC_TEMPERATURE = "sensors/temperature";
const char* TOPIC_HUMIDITY = "sensors/humidity";
const char* TOPIC_RAIN = "sensors/rain";
const char* TOPIC_STATUS = "sensors/status";

// DHT Setup
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// Sensor data structure
struct SensorData {
  int moistureRaw;
  int moisturePercentage;
  float temperature;
  float humidity;
  bool isRaining;
} sensorData;

// MQTT Client setup
WiFiClient wifiClient;
PubSubClient client(wifiClient);

unsigned long lastMsgTime = 0;
const long INTERVAL = 30000;  // 30 seconds interval

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);  // Added explicit WiFi mode
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ArduinoPlant-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Publish a status message upon connection
      client.publish(TOPIC_STATUS, "{\"status\":\"connected\"}");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) continue;  // Wait for Serial to be ready
  
  // Setup pins
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RAIN_PIN, INPUT);
  
  // Initial LED test
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  // Initialize sensors and connections
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.println("==========================================");
  Serial.println("Smart Plant Monitoring System");
  Serial.println("==========================================");
}

void readAllSensors() {
  // Read Soil Moisture
  sensorData.moistureRaw = analogRead(MOISTURE_PIN);
  Serial.print("Raw Moisture Value: ");
  Serial.println(sensorData.moistureRaw);
  
  sensorData.moistureRaw = constrain(sensorData.moistureRaw, MaxMoistureValue, MinMoistureValue);
  sensorData.moisturePercentage = map(sensorData.moistureRaw, MinMoistureValue, MaxMoistureValue, MinMoisture, MaxMoisture);
  
  // Read Temperature and Humidity
  sensorData.humidity = dht.readHumidity();
  sensorData.temperature = dht.readTemperature();
  
  // Read Rain Sensor
  sensorData.isRaining = !digitalRead(RAIN_PIN);  // Inverted because sensor gives LOW when wet
  
  updateLEDs();
}

void updateLEDs() {
  bool isAlert = false;
  
  // Check conditions
  if (sensorData.moisturePercentage <= 30 ||
      sensorData.temperature > 30 ||
      sensorData.humidity > 85 ||
      sensorData.isRaining) {
    isAlert = true;
  }
  
  digitalWrite(RED_LED_PIN, isAlert);
  digitalWrite(GREEN_LED_PIN, !isAlert);
}

void publishData() {
  // Create JSON document for all sensor data
  StaticJsonDocument<200> doc;
  doc["moisture"] = sensorData.moisturePercentage;
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["isRaining"] = sensorData.isRaining;
  doc["timestamp"] = millis();
  
  char buffer[200];
  serializeJson(doc, buffer);
  
  // Publish to individual topics
  client.publish(TOPIC_MOISTURE, String(sensorData.moisturePercentage).c_str());
  client.publish(TOPIC_TEMPERATURE, String(sensorData.temperature).c_str());
  client.publish(TOPIC_HUMIDITY, String(sensorData.humidity).c_str());
  client.publish(TOPIC_RAIN, sensorData.isRaining ? "1" : "0");
  
  // Publish complete JSON to status topic
  client.publish(TOPIC_STATUS, buffer);
}

void printReadings() {
  Serial.println("\n----------------------------------------");
  Serial.print("Soil Moisture: ");
  Serial.print(sensorData.moisturePercentage);
  Serial.println("%");
  
  Serial.print("Temperature: ");
  Serial.print(sensorData.temperature, 1);
  Serial.println("°C");
  
  Serial.print("Humidity: ");
  Serial.print(sensorData.humidity, 1);
  Serial.println("%");
  
  Serial.print("Rain Status: ");
  Serial.println(sensorData.isRaining ? "RAINING!" : "No Rain");
  
  Serial.print("Environment Status: ");
  if (sensorData.temperature > 30) {
    Serial.println("Warning: Temperature too high!");
  } else if (sensorData.humidity > 85) {
    Serial.println("Warning: Humidity too high!");
  } else if (sensorData.isRaining) {
    Serial.println("Warning: Rain detected!");
  } else {
    Serial.println("Optimal Growing Conditions");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsgTime > INTERVAL) {
    lastMsgTime = now;
    
    readAllSensors();
    printReadings();
    publishData();
  }
}
