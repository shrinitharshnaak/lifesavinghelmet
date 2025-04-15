#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// GPS and GSM module communication pins
SoftwareSerial gpsSerial(4, 3);   // RX, TX for GPS
SoftwareSerial gsmSerial(7, 8);   // RX, TX for GSM

TinyGPSPlus gps;

const int vibrationPin = A0;
const int threshold = 600; // Adjust based on sensor testing
const char emergencyNumber[] = "+911234567890"; // Replace with your number

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  gsmSerial.begin(9600);
  
  pinMode(vibrationPin, INPUT);
  delay(1000);
  
  Serial.println("System Initialized.");
  sendSMS("Helmet system is active.");
}

void loop() {
  int vibrationValue = analogRead(vibrationPin);
  Serial.print("Vibration: ");
  Serial.println(vibrationValue);

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (vibrationValue > threshold) {
    Serial.println("Threshold Exceeded! Accident Detected!");

    // Wait for valid GPS signal
    unsigned long start = millis();
    while (!gps.location.isValid() && millis() - start < 5000) {
      if (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
      }
    }

    if (gps.location.isValid()) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();

      String message = "Accident Detected!\nLocation:\n";
      message += "https://maps.google.com/?q=";
      message += String(latitude, 6);
      message += ",";
      message += String(longitude, 6);

      sendSMS(message.c_str());
    } else {
      sendSMS("Accident detected! GPS location not available.");
    }

    delay(10000); // Avoid spamming
  }

  delay(500);
}

void sendSMS(const char* message) {
  gsmSerial.println("AT+CMGF=1"); // Set SMS mode
  delay(100);
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(emergencyNumber);
  gsmSerial.println("\"");
  delay(100);
  gsmSerial.print(message);
  delay(100);
  gsmSerial.write(26); // Ctrl+Z to send SMS
  delay(5000);
}

