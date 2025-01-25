
#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h> 

const char* ssid = "Vianet promotion MI -1";
const char* password = "Vianet@123";
const char* serverName = "http://<Flask_Server_IP>/sensor_data";  

#define ULTRASONIC_TRIG_PIN 19 
#define ULTRASONIC_ECHO_PIN 21 

DHT dht(4, DHT11);  

#define MQ2_SENSOR_PIN 34  
#define PIR_SENSOR_PIN 2 
#define LED_PIN 32   

int gasThreshold = 35;  
int waterLevelThreshold = 10;  // Add a threshold for water level (in cm)

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();

  lcd.begin(16,2);
  lcd.backlight(); 

  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int gasValue = analogRead(MQ2_SENSOR_PIN); 

  if (gasValue > gasThreshold || temperature > 30) { 
    sendAlert(gasValue, temperature);
  }

  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);  
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "temperature=" + String(temperature) + "&humidity=" + String(humidity);
    int httpResponseCode = http.POST(httpRequestData);

    if(httpResponseCode > 0) {
      Serial.println("Data Sent Successfully");
    } else {
      Serial.println("Error sending data");
    }

    http.end();
  }

  int motionDetected = digitalRead(PIR_SENSOR_PIN);
  if (motionDetected == HIGH) {
    digitalWrite(LED_PIN, HIGH); 
    Serial.println("Motion detected: LED ON");
  } else {
    digitalWrite(LED_PIN, LOW); 
    Serial.println("No motion: LED OFF");
  }

  delay(10000); 
}

void sendAlert(int gasLevel, float temp) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String alertMessage = "Alert! ";
    if (gasLevel > gasThreshold) {
      alertMessage += "Gas leak detected. ";
    }

    if (temp > 30) {
      alertMessage += "High temperature detected. ";
    }

    String serverAlertURL = "http://<Flask_Server_IP>/alert"; 
    http.begin(serverAlertURL);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "alert=" + alertMessage;
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.println("Alert sent successfully");
    } else {
      Serial.println("Error sending alert");
    }

    http.end();
  }
}

void checkWaterLevel() {
  long duration, distance;

  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);

  distance = (duration / 2) / 29.1;

  if (distance <= waterLevelThreshold) {
    sendWaterAlert();
  }
}

void sendWaterAlert() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String alertMessage = "Alert! Water level detected in the tank.";

    String serverAlertURL = "http://<Flask_Server_IP>/alert"; 
    http.begin(serverAlertURL);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "alert=" + alertMessage;
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.println("Water alert sent successfully");
    } else {
      Serial.println("Error sending water alert");
    }

    http.end();
  }
}
