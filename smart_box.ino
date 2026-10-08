#include <ESP32Servo.h> 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

const int servoPin = 12; 

const int lidTrigPin = 26;  
const int lidEchoPin = 25;  

const int SERVO_CLOSED = 90; 
const int SERVO_OPEN = 10;   

const int levelTrigPin = 5;
const int levelEchoPin = 18;

#define EMPTY_CM 30.0   
#define FULL_CM 5.0     

const char* WIFI_SSID = "";
const char* WIFI_PASS = "";
const char* SERVER_URL = ""; 

const unsigned long SEND_INTERVAL = 5000;  
unsigned long lastSend = 0;
float currentPct = 0.0; 

Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200); 
  
  myServo.attach(servoPin);
  myServo.write(SERVO_CLOSED); 
  pinMode(lidTrigPin, OUTPUT);
  pinMode(lidEchoPin, INPUT);

  pinMode(levelTrigPin, OUTPUT);
  pinMode(levelEchoPin, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Level:");
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi ");

  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  lcd.setCursor(0, 1);
  lcd.print("WiFi Connected! ");
  delay(1000);
}

int getLidDistance() {
  digitalWrite(lidTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(lidTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(lidTrigPin, LOW);
  long duration = pulseIn(lidEchoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

float getLevelDistance() {
  digitalWrite(levelTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(levelTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(levelTrigPin, LOW);
  long duration = pulseIn(levelEchoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

float distanceToPercent(float d) {
  float pct = (EMPTY_CM - d) / (EMPTY_CM - FULL_CM) * 100.0;
  return constrain(pct, 0, 100); 
}

void sendToServer(float pct, String state) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  
  http.setTimeout(1000); 
  
  String body = "{\"device_id\":\"smart_box_01\",\"cover_status\":\"" + state + "\",\"fill_percentage\":" + String(pct, 1) + "}";  
  
  int code = http.POST(body); 
  
  Serial.print("HTTP POST Status: ");
  Serial.println(code); 
  http.end();
}

void loop() {
  float levelDist = getLevelDistance();
  lcd.setCursor(0, 1);
  
  if (levelDist < 0) {
    lcd.print("Out of range   ");
  } else {
    currentPct = distanceToPercent(levelDist); 
    lcd.print(currentPct, 0);
    lcd.print(" %      ");

    if (millis() - lastSend >= SEND_INTERVAL) {
      lastSend = millis();
      sendToServer(currentPct, "CLOSED"); 
    }
  }

  int lidDist = getLidDistance();
  
  if (lidDist > 0 && lidDist < 10) {
    Serial.println("Hand detected! Opening...");
    myServo.write(SERVO_OPEN); 
    
    sendToServer(currentPct, "OPEN"); 
    
    delay(3000); 
    
    int missedReadings = 0; 
    
    while (missedReadings < 3) {
      delay(150); 
      int currentLidDist = getLidDistance(); 
      
      if (currentLidDist > 0 && currentLidDist <= 12) {
        missedReadings = 0; 
      } else {
        missedReadings++; 
      }
    }
    
    Serial.println("Hand removed. Closing...");
    myServo.write(SERVO_CLOSED); 
    
    sendToServer(currentPct, "CLOSED");
    lastSend = millis(); 
    
    delay(500); 
  }
  
  delay(100); 
}
