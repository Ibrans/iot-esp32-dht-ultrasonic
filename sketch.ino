// ==============================
// IMPORT LIBRARY
// ==============================

// Library untuk koneksi WiFi ESP32
#include <WiFi.h>

// Library untuk HTTP request (kirim data ke internet / ThingSpeak)
#include <HTTPClient.h>

// Library untuk sensor DHT (suhu & kelembaban)
#include <DHT.h>


// ==============================
// DEFINISI PIN
// ==============================

// Pin DHT22 (sensor suhu)
#define PIN_DHT 4

// Pin Ultrasonik
#define PIN_TRIG 5   // untuk mengirim sinyal
#define PIN_ECHO 18  // untuk menerima pantulan

// Pin LED indikator
#define PIN_LED 2


// ==============================
// SETUP SENSOR DHT
// ==============================

// Tipe sensor DHT (DHT22)
#define DHTTYPE DHT22

// Inisialisasi objek DHT
DHT dht(PIN_DHT, DHTTYPE);


// ==============================
// KONFIGURASI WIFI & THINGSPEAK
// ==============================

// SSID WiFi (di Wokwi wajib pakai ini)
const char* ssid = "Wokwi-GUEST";

// Password WiFi (Wokwi tidak pakai password)
const char* password = "";

// API Key dari ThingSpeak (untuk kirim data)
String apiKey = "G5GQDB7OEHDWRKMK";


// ==============================
// SETUP (DIJALANKAN SEKALI)
// ==============================

void setup() {
  // Memulai komunikasi serial (untuk debugging)
  Serial.begin(115200);

  // Mengatur mode pin
  pinMode(PIN_TRIG, OUTPUT); // ESP32 kirim sinyal
  pinMode(PIN_ECHO, INPUT);  // ESP32 menerima sinyal
  pinMode(PIN_LED, OUTPUT);  // LED sebagai output

  // Inisialisasi sensor DHT
  dht.begin();

  Serial.println("Sistem Monitoring ESP32 Ready!");

  // ==============================
  // KONEKSI KE WIFI
  // ==============================

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  // Tunggu sampai terhubung
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
}


// ==============================
// FUNGSI MEMBACA JARAK (ULTRASONIK)
// ==============================

float readDistance() {
  // 1. Reset TRIG
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  // 2. Kirim sinyal ultrasonik selama 10 mikrodetik
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // 3. Baca durasi sinyal pantulan
  long durasi = pulseIn(PIN_ECHO, HIGH);

  // 4. Konversi ke jarak (cm)
  // 0.034 cm/us = kecepatan suara
  float jarak = (durasi * 0.034) / 2;

  return jarak;
}


// ==============================
// LOOP (DIJALANKAN TERUS MENERUS)
// ==============================

void loop() {

  // ==============================
  // 1. BACA SENSOR
  // ==============================

  // Baca suhu dari DHT22
  float suhu = dht.readTemperature();

  // Cek apakah pembacaan gagal
  if (isnan(suhu)) {
    Serial.println("Gagal baca DHT!");
    return; // keluar dari loop sementara
  }

  // Baca jarak dari ultrasonik
  float jarak = readDistance();


  // ==============================
  // 2. TAMPILKAN KE SERIAL
  // ==============================

  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C | Jarak: ");
  Serial.print(jarak);
  Serial.println(" cm");


  // ==============================
  // 3. LOGIKA LED
  // ==============================

  // Jika suhu tinggi DAN jarak dekat → bahaya
  if (suhu > 28 && jarak < 15) {
    digitalWrite(PIN_LED, HIGH); // LED nyala
    Serial.println("STATUS: BAHAYA!");
  } else {
    digitalWrite(PIN_LED, LOW);  // LED mati
    Serial.println("STATUS: AMAN");
  }


  // ==============================
  // 4. KIRIM DATA KE THINGSPEAK
  // ==============================

  // Pastikan masih terhubung ke WiFi
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    // URL API ThingSpeak
    String url = "http://api.thingspeak.com/update?api_key=" + apiKey +
                 "&field1=" + String(suhu) +
                 "&field2=" + String(jarak);

    // Mulai koneksi HTTP
    http.begin(url);

    // Kirim request GET
    int httpCode = http.GET();

    // Tampilkan response dari server
    Serial.print("Response ThingSpeak: ");
    Serial.println(httpCode);

    // Tutup koneksi
    http.end();
  }


  // ==============================
  // 5. DELAY (WAJIB 15 DETIK)
  // ==============================

  // ThingSpeak hanya menerima data tiap 15 detik
  delay(15000);
}