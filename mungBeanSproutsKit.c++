#include <ESP8266WiFi.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <RtcDS3231.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define AUTH "your_blynk_auth_token" // ใส่ Blynk Auth Token ของคุณ
#define DHTPIN D4     // กำหนดขาของ DHT11
#define DHTTYPE DHT11 // ใช้ DHT11
#define LED1_PIN D9
#define LED2_PIN D10
#define LED3_PIN D11
#define LED4_PIN D12
#define LED5_PIN D13
#define BUZZER_PIN D5
#define PUMP1_PIN D6
#define PUMP2_PIN D7
#define PUMP3_PIN D8
#define PUMP4_PIN D3

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

RtcDS3231<TwoWire> Rtc(Wire);
DHT dht(DHTPIN, DHTTYPE);

uint8_t currentDay = 1;
bool buzzerEnabled = true;
float tempMax = 30.0;  // กำหนดอุณหภูมิสูงสุด
float humMin = 40.0;   // กำหนดความชื้นต่ำสุด

BlynkTimer timer;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUMP1_PIN, OUTPUT);
  pinMode(PUMP2_PIN, OUTPUT);
  pinMode(PUMP3_PIN, OUTPUT);
  pinMode(PUMP4_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);
  
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize RTC
  Rtc.Begin();
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
  Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz);
  timeClient.begin();

  // Initialize DHT
  dht.begin();

  // Connect to Blynk
  Blynk.begin(AUTH, WiFi, "blynk-cloud.com", 80);
  
  // Set up Blynk timer to send DHT data
  timer.setInterval(60000L, sendDHTData); // Send data every 60 seconds
}

void loop() {
  timeClient.update();
  RtcDateTime now = Rtc.GetDateTime();

  // Check if it's time to increment the day
  if (now.Hour() == 0 && now.Minute() == 0 && now.Second() == 1) {
    currentDay++;
    if (currentDay == 6) {
      currentDay = 1;
      buzzerEnabled = true;
    }
  }

  // Check if it's time to activate the buzzer
  if (buzzerEnabled && now.Minute() % 2 == 0 && now.Second() < 30) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Control pumps based on the specified times and days
  if (currentDay == 2 && now.Hour() == 3 && now.Minute() == 0 && now.Second() == 0) {
    digitalWrite(PUMP1_PIN, HIGH);
    delay(20000); // Run pump1 for 20 seconds
    digitalWrite(PUMP1_PIN, LOW);
  }

  if (currentDay == 2 && now.Hour() == 15 && now.Minute() == 0 && now.Second() == 0) {
    digitalWrite(PUMP2_PIN, HIGH);
    delay(20000); // Run pump2 for 20 seconds
    digitalWrite(PUMP2_PIN, LOW);
  }

  if (currentDay == 3 && now.Hour() == 15 && now.Minute() == 0 && now.Second() == 0) {
    digitalWrite(PUMP3_PIN, HIGH);
    delay(20000); // Run pump3 for 20 seconds
    digitalWrite(PUMP3_PIN, LOW);
  }

  if (currentDay == 4 && now.Hour() == 15 && now.Minute() == 0 && now.Second() == 0) {
    digitalWrite(PUMP4_PIN, HIGH);
    delay(20000); // Run pump4 for 20 seconds
    digitalWrite(PUMP4_PIN, LOW);
  }

  // Run pump4 at specified times except on day 2 at 03:00:00 and 01:00:00
  if (now.Hour() % 2 == 1 && now.Minute() == 0 && now.Second() == 0 && !(currentDay == 2 && (now.Hour() == 3 || now.Hour() == 1))) {
    digitalWrite(PUMP4_PIN, HIGH);
    delay(20000); // Run pump4 for 20 seconds
    digitalWrite(PUMP4_PIN, LOW);
  }

  Blynk.run();
  timer.run();
  sendDHTData(); // Send
}

void sendDHTData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  
  if (temperature > tempMax || humidity < humMin) {
    digitalWrite(PUMP4_PIN, HIGH);
    delay(10000); // Run pump4 for 10 seconds
    digitalWrite(PUMP4_PIN, LOW);
  }
}
