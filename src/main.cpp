#include <Arduino.h>
#include <HX711_ADC.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- KONFIGURACJA WiFi ---
const char* ssid = "Redmi Note 9 Pro";
const char* password = "12345678";
String apiKey = "ESUGMCKYJXTKFVTZ";

// --- KONFIGURACJA PINÓW ---
const int HX711_DT_PIN = 22;  // D22
const int HX711_SCK_PIN = 21; // D21
const int BIA_DAC_PIN = 25;   // D25
const int BIA_ADC_PIN = 34;   // D34

// --- OBIEKTY I ZMIENNE GLOBALNE ---
HX711_ADC LoadCell(HX711_DT_PIN, HX711_SCK_PIN);

float kalibracjaWagi = -21.93; 

// Konfiguracja pomiaru BIA
const int SINE_TABLE_SIZE = 64;
byte sine_wave_table[SINE_TABLE_SIZE];
int ostatniaAmplitudaBIA = 0;
float ostatniaWagaKg = 0;

// Timer do nieblokującego wysyłania danych
unsigned long ostatnieWyslanieDanych = 0;
const long interwalWysylania = 20000; // Wysyłaj dane co 20 sekund (ThingSpeak limit)

void connectToWiFi() {
  Serial.print("Łączenie z WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Połączono! IP: ");
  Serial.println(WiFi.localIP());
}

void wyslijDoThingSpeak(float waga) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://api.thingspeak.com/update");
    
    String postData = "api_key=" + apiKey + "&field1=" + String(waga, 3);
    
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(postData);
    
    if (httpResponseCode > 0) {
      Serial.print("ThingSpeak: Wysłano wagę ");
      Serial.print(waga, 3);
      Serial.print(" kg - Status: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Błąd wysyłania: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Brak połączenia WiFi!");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n--- Start programu wagi BIA v2.0 z ThingSpeak ---");

  // KROK 1: POŁĄCZENIE Z WiFi
  connectToWiFi();

  // KROK 2: INICJALIZACJA WAGI
  LoadCell.begin();
  Serial.println("INFO: Uruchamianie i tarowanie wagi...");
  LoadCell.start(2000, true);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("BŁĄD KRYTYCZNY: Nie można skomunikować się z modułem HX711.");
    while (1); // Zatrzymaj program
  }
  LoadCell.setCalFactor(kalibracjaWagi);
  Serial.println("INFO: Inicjalizacja wagi zakończona.");

  // KROK 3: INICJALIZACJA UKŁADU BIA
  for (int i = 0; i < SINE_TABLE_SIZE; i++) {
    sine_wave_table[i] = (byte)((sin(i * 2.0 * PI / SINE_TABLE_SIZE) + 1.0) * 127.5);
  }
  Serial.println("INFO: Inicjalizacja BIA zakończona.");
  Serial.println("----------------------------------------------------");
  Serial.println("System gotowy. Wyślij 't' aby wytarować wagę.");
  Serial.println("----------------------------------------------------");
}

void loop() {
  // --- CIĄGŁY POMIAR WAGI W TLE ---
  if (LoadCell.update()) {
    ostatniaWagaKg = LoadCell.getData() / 1000.0;
  }

  // --- CIĄGŁY POMIAR BIA W TLE ---
  int min_reading = 4095;
  int max_reading = 0;
  for (int i = 0; i < SINE_TABLE_SIZE; i++) {
    dacWrite(BIA_DAC_PIN, sine_wave_table[i]);
    delayMicroseconds(20); 
    int reading = analogRead(BIA_ADC_PIN);
    if (reading < min_reading) min_reading = reading;
    if (reading > max_reading) max_reading = reading;
  }
  ostatniaAmplitudaBIA = max_reading - min_reading;

  // --- OBSŁUGA KOMEND Z KOMPUTERA ---
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 't' || command == 'T') {
      LoadCell.tare();
      Serial.println("--> WAGA WYTAROWANA <--");
    }
  }

  // --- WYSYŁANIE DANYCH CO 20 SEKUND ---
  if (millis() - ostatnieWyslanieDanych >= interwalWysylania) {
    ostatnieWyslanieDanych = millis();
    
    // Wyświetl lokalnie
    Serial.print("Waga: ");
    Serial.print(ostatniaWagaKg, 3);
    Serial.print(" kg");
    Serial.print("  |  Amplituda BIA: ");
    Serial.println(ostatniaAmplitudaBIA);
    
    // Wyślij do ThingSpeak
    wyslijDoThingSpeak(ostatniaWagaKg);
  }
}