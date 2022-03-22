#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>

#define RST_PIN         0          // D3
#define SS_PIN          4         // D2
#define SPK_PIN         15       // D8
#define LED_PIN         2       // D4
#define AP_SSID         "Your SSID"
#define AP_PSK          "Your PSK"
#define AUTH_ALLOWED_INTERVAL 120000 //milliseconds; last authentication fact is remembered for 2 minutes by default

String allowedID = String("yourAllowedNfcId");
String targetMAC = String("YO:UR:MA:CA:DD:RE:SS");
static unsigned long lastAuthTime = 0;
enum signal_t {detected, allowed, denied};

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
WiFiUDP UDP;
WakeOnLan WOL(UDP);
WiFiServer socket(80);

unsigned long getID(){ //get nfc id
  unsigned long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}

bool canAuthenticate() {
  unsigned long curTime = millis();
  if (lastAuthTime == 0) {
    return false;
  }
  if (curTime - lastAuthTime <= AUTH_ALLOWED_INTERVAL) {
    return true;
  }
  else {
    return false;
  }
}

void emitSignal(signal_t signal) {
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPK_PIN, OUTPUT);
  if (signal == detected) {
    for (byte i = 0; i < 2; i++) {
      delay(50);
      digitalWrite(LED_PIN, HIGH);
      delay(50);
      digitalWrite(LED_PIN, LOW);
    }
  }
  else if (signal == allowed) {
    tone(SPK_PIN, 1000);
    delay(100);
    noTone(SPK_PIN);
  }
  else if (signal == denied) {
    for (byte i = 0; i < 2; i++) {
      delay(100);
      tone(SPK_PIN, 700);
      delay(100);
      noTone(SPK_PIN);
    }
  }

  pinMode(LED_PIN, INPUT);
  pinMode(SPK_PIN, INPUT);
}

void setup() {
  //Serial.begin(9600);		// Initialize serial communications with the PC
  //while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();			// Init SPI bus
 
		
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PSK);

  while (WiFi.status() != WL_CONNECTED) { //wait for the network to connect (or loop forever)   
  }

  mfrc522.PCD_Init();    // Init MFRC522
  delay(4); 
  socket.begin(); //Init the TCP socket

  //Serial.println("MFRC522 initialized");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());

}

void loop() {  
  delay(1000); //polling interval is 1 second
  //handle tcp requests on port 80
  if (WiFiClient con = socket.available()) {
    while (con.connected()) {
      char incoming[64];
      memset(incoming, 0x0, sizeof(incoming));
      con.read(incoming, sizeof(incoming));
      String inStr = String(incoming);
      inStr.trim();
      //Serial.print("Received: ");
      //Serial.println(inStr);
      if (inStr.equals("AUTHENTICATE")) {
        if (canAuthenticate()) {
          con.write("OK\n", 3);
        }
        else {
          con.write("FAIL\n", 5);
        }
      }
      else {
        con.write("UNKNOWN\n", 8);
      }
      con.stop();
    }
  }
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return; // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  }


  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  enum signal_t signal = detected;

  emitSignal(signal);

  String cardID = String(getID());
  //Serial.println(cardID);  
  if (cardID.equals(allowedID)) {
    signal = allowed;
    //Serial.println("allowed");
    WOL.sendMagicPacket(targetMAC);
    lastAuthTime = millis(); //remember authentication for some time
  }
  else {
    signal = denied;
  }

  emitSignal(signal);
}
