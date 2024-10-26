#include <ESP32Servo.h>   // Library untuk motor servo pada ESP32
#include <WiFi.h>         // Library untuk Wi-Fi pada ESP32
#include <PubSubClient.h> // Library untuk MQTT
#include <ArduinoJson.h>  // Library untuk JSON (kirim data dalam format JSON)

// Koneksi Wi-Fi dan ThingsBoard
const char* ssid = "Kos 16 Bawahaha";                   // Ganti dengan nama Wi-Fi Anda
const char* password = "apasamyang";          // Ganti dengan password Wi-Fi Anda
#define TOKEN "Q0LH1Lu0LSianLj5cbY1"           // Ganti dengan Access Token dari ThingsBoard
#define THINGSBOARD_SERVER "demo.thingsboard.io" // Server ThingsBoard
#define THINGSBOARD_PORT 1883                  // Port MQTT ThingsBoard

WiFiClient espClient;
PubSubClient client(espClient);

// Define pin untuk HC-SR04 dan Servo
const int trigPin = 7;   // Pin trig dihubungkan ke GPIO 7 pada ESP32-C6
const int echoPin = 8;   // Pin echo dihubungkan ke GPIO 8 pada ESP32-C6
const int servoPin = 23; // Pin untuk menghubungkan motor servo (GPIO 23)

// Variabel untuk menyimpan waktu dan jarak
long duration;
float distance;
int angle; // Variabel untuk menyimpan sudut servo

// Buat objek Servo
Servo myServo;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32_Client", TOKEN, NULL)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Inisialisasi pin servo dan sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myServo.attach(servoPin);
  myServo.write(0);  // Set servo ke posisi awal 0 derajat

  // Koneksi Wi-Fi
  setup_wifi();

  // Set MQTT server
  client.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca jarak dari sensor HC-SR04
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Logika kontrol servo berdasarkan jarak
  if (distance < 10) {
    angle = 90; // Gerakkan servo ke 90 derajat jika jarak < 10 cm
  } else if (distance > 20) {
    angle = 0;  // Kembalikan servo ke 0 derajat jika jarak > 30 cm
  } else {
    angle = 90; // Tetap di 90 derajat jika jarak di antara 10 dan 30 cm
  }

  // Set posisi servo berdasarkan sudut yang telah dihitung
  myServo.write(angle);

  // Buat JSON payload
  StaticJsonDocument<200> doc;
  doc["jarak"] = distance;
  doc["sudut_servo"] = angle;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  // Kirim data ke ThingsBoard
  client.publish("v1/devices/me/telemetry", jsonBuffer);

  // Delay sebelum pengukuran berikutnya
  delay(1000);
}
