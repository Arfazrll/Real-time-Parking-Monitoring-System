#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

#define SS_PIN 21       // Pin SDA RFID RC522
#define RST_PIN 22      // Pin RST RFID RC522
#define BUZZER_PIN 15   // Pin Buzzer
#define SERVO_PIN 14    // Pin Servo  
#define IR_SENSOR_PIN 13 // Sensor palang parkir
#define IR_LED 2        // IR LED untuk deteksi slot parkir
#define IR_RECEIVER 4   // IR Receiver untuk deteksi slot parkir

const char* ssid = "iPhone";        
const char* password = "bismillah";             
const char* serverUrl = "http://172.20.10.3/sistem_parkir"; 

MFRC522 rfid(SS_PIN, RST_PIN);
Servo myServo;

String vehicleID[] = {"676F749F", "B9CFF66E", "A2FA2958"};
String lastDetectedUID = "UNKNOWN"; // Default jika belum ada kartu terdeteksi

bool gateOpen = false;
bool mobilTerparkir = false;
bool lastMobilTerparkir = false; // Variabel untuk melacak perubahan status
unsigned long lastdetectionTime = 0;
unsigned long lastSendTime = 0; // Waktu terakhir mengirim data ke server
unsigned long lastIRCheckTime = 0; // Untuk debouncing sensor IR

int totalSlot = 1;  // Jumlah total slot parkir
int irDetectionCount = 0; // Untuk filtering noise sensor IR

void setup() {
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(IR_SENSOR_PIN, INPUT);
    pinMode(IR_RECEIVER, INPUT);
    pinMode(IR_LED, OUTPUT);

    digitalWrite(IR_LED, HIGH); // Aktifkan IR LED secara permanen
    digitalWrite(BUZZER_PIN, LOW);
    myServo.attach(SERVO_PIN);
    myServo.write(0); // Posisi awal palang (tertutup)

    Serial.println("Menghubungkan ke WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nTerhubung ke WiFi!");
    Serial.println("IP Address: " + WiFi.localIP().toString());
    Serial.println("Sistem Siap! Menunggu kartu RFID...");
    
    // Cek kondisi awal sensor IR
    bool initialIRState = digitalRead(IR_RECEIVER);
    mobilTerparkir = (initialIRState == LOW);
    lastMobilTerparkir = mobilTerparkir;
    Serial.println("Status awal sensor IR: " + String(initialIRState == LOW ? "Terdeteksi (Slot Terisi)" : "Tidak Terdeteksi (Slot Kosong)"));
    
    // Kirim status awal ke server
    if (mobilTerparkir) {
        sendStatusToServer("Masuk");
        totalSlot = 0;
    } else {
        totalSlot = 1;
    }
}

void loop() {
    // Kontrol palang parkir dengan sensor IR
    bool irState = digitalRead(IR_SENSOR_PIN) == LOW;
    if (gateOpen) { 
        if (irState) {
            lastdetectionTime = millis();
        } else if (millis() - lastdetectionTime > 2000) {
            Serial.println("Tidak ada objek. Menutup palang...");
            myServo.write(0);
            gateOpen = false;
        }
    }

    // Pemindai kartu RFID untuk akses
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        String uid = getUID();
        Serial.print("UID kartu: ");
        Serial.println(uid);
        lastDetectedUID = uid; // Simpan UID kartu yang terdeteksi
        bool isAllowed = false;

        for (String id : vehicleID) {
            if (uid == id) {
                isAllowed = true;
                break;
            }
        }

        if (isAllowed) {
            Serial.println("Akses Diterima!");
            buzzerBeep(1, 200);
            if (!gateOpen) {
                Serial.println("Membuka palang...");
                myServo.write(90);
                gateOpen = true;
                lastdetectionTime = millis();
            }
        } else {
            Serial.println("Akses Ditolak!");
            buzzerBeep(3, 200);
        }
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }

    // Deteksi mobil di slot parkir dengan sensor IR
    // Hanya periksa sensor IR setiap 250ms untuk mengurangi noise
    if (millis() - lastIRCheckTime >= 250) {
        lastIRCheckTime = millis();
        
        digitalWrite(IR_LED, HIGH); // Pastikan IR LED aktif
        bool irParkirState = digitalRead(IR_RECEIVER);
        
        // Debug status sensor IR
        Serial.println("IR Sensor State: " + String(irParkirState == LOW ? "Terdeteksi (Mobil Ada)" : "Tidak Terdeteksi (Kosong)"));
        
        // Filtering untuk mengurangi noise
        if (irParkirState == LOW) { // Mobil terdeteksi
            irDetectionCount++;
            if (irDetectionCount >= 3 && !mobilTerparkir) { // Tiga bacaan konsekutif
                mobilTerparkir = true;
                totalSlot = 0; // Slot terisi, jadi tersedia = 0
                Serial.println("Mobil masuk. Slot tersedia: " + String(totalSlot));
                
                // Kirim status ke server (hanya jika terjadi perubahan)
                if (lastMobilTerparkir != mobilTerparkir) {
                    sendStatusToServer("Masuk");
                    lastMobilTerparkir = mobilTerparkir;
                }
            }
        } else { // Tidak ada mobil
            irDetectionCount = 0;
            if (mobilTerparkir) { // Jika sebelumnya terdeteksi mobil
                mobilTerparkir = false;
                totalSlot = 1; // Slot kosong, jadi tersedia = 1
                Serial.println("Mobil keluar. Slot tersedia: " + String(totalSlot));
                
                // Kirim status ke server (hanya jika terjadi perubahan)
                if (lastMobilTerparkir != mobilTerparkir) {
                    sendStatusToServer("Keluar");
                    lastMobilTerparkir = mobilTerparkir;
                }
            }
        }
    }
    
    delay(100); // Jeda singkat untuk stabilitas
}

String getUID() {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();  // Ubah menjadi huruf besar
    return uid;    
}

void buzzerBeep(int times, int duration) {
    for (int i = 0; i < times; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(duration);
        digitalWrite(BUZZER_PIN, LOW);
        delay(duration);
    }
}

void sendStatusToServer(String status) {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        
        // URL endpoint yang benar
        http.begin(client, String(serverUrl) + "/api/update_parking_status.php");
        http.addHeader("Content-Type", "application/json");
        
        // Format JSON sesuai dengan yang diharapkan API
        String jsonPayload = "{"
            "\"rfid_number\":\"" + lastDetectedUID + "\","
            "\"status\":\"" + status + "\""
        "}";
        
        Serial.println("Mengirim data: " + jsonPayload);
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("HTTP Response code: " + String(httpResponseCode));
            Serial.println("Response: " + response);
            
            // Jika berhasil, simpan status saat ini
            if (httpResponseCode == 200) {
                lastSendTime = millis();
                Serial.println("Data berhasil disimpan ke server!");
            }
        } else {
            Serial.println("Error pada HTTP request: " + String(httpResponseCode));
        }
        http.end();
    } else {
        Serial.println("WiFi tidak terhubung! Mencoba menghubungkan kembali...");
        WiFi.begin(ssid, password);
        
        // Tunggu maksimal 10 detik untuk koneksi
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
            Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nTerhubung kembali ke WiFi!");
        } else {
            Serial.println("\nGagal terhubung kembali ke WiFi!");
        }
    }
}