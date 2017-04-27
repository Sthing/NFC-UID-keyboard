/*
 * Reads a card UID from an ISO14443 card and outputs the UID as keyboard input.
 * 
 * Author: Søren Thing <soeren@thing.dk>
 * Released under MIT license - please see the file LICENSE.
 * 
 * HW:
 *  - A 3.3V/8MHz Arduino Pro Micro clone
 *  - A RFID-RC522 card reader (Buy at eBay or AliExpress)
 *  - A buzzer
 *  - An LED and a resistor
 * 
 * Beware: The card reader is NOT 5V tolerant.
 * 
 * Connections:
 * Signal     Pin                    Pin
 *            Arduino Pro Micro      MFRC522 board
 * -----------------------------------------
 * Reset      10                     RST
 * SPI SS     18 (marked A0)         SDA
 * SPI MOSI   16                     MOSI
 * SPI MISO   14                     MISO
 * SPI SCK    15                     SCK
 * LED         5
 * Buzzer      3
 */
#include <SPI.h>
#include <MFRC522.h>
#include "Keyboard.h"

//#define USE_SERIAL 1

#define BUZZER_PIN 3
#define LED_PIN 5
#define SS_PIN 18
#define RST_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.
MFRC522::Uid lastUid;     // UID of last scanned card
unsigned long lastMillis; // value of millis() when last card was scanned.

void setup() {
  #ifdef USE_SERIAL
	Serial.begin(9600);	// Initialize serial communications with the PC
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Scan PICC to see UID and type...");
  #endif
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Keyboard.begin();
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();	// Init MFRC522 card
}

void loop() {
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //Serial.print("."); delay(500);
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
    #ifdef USE_SERIAL
    Serial.println("Not selected");
    #endif
		return;
	}

  // Check that it is not a bounce of the same card
  if (memcmp(&lastUid, &mfrc522.uid, mfrc522.uid.size) == 0 // Same UID
        && (millis() - lastMillis) < 1000) { // Scanned again within on second 
    // Halt the card so it does not respond again while in the field
    mfrc522.PICC_HaltA();
    return;
  }

  // Turn on buzzer and LED
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);

  // Remember the time
  lastMillis = millis();
  
	// Output the UID followed by Enter.
  #ifdef USE_SERIAL
  Serial.print("Card UID: ");
  #endif
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    #ifdef USE_SERIAL
    Serial.print(mfrc522.uid.uidByte[i] >> 4, HEX);
    Serial.print(mfrc522.uid.uidByte[i] & 0x0F, HEX);
    #endif
    Keyboard.print(mfrc522.uid.uidByte[i] >> 4, HEX);
    Keyboard.print(mfrc522.uid.uidByte[i] & 0x0F, HEX);
  } 
  #ifdef USE_SERIAL
  Serial.println();
  #endif
  Keyboard.write(KEY_RETURN);

  // Wait a short while before turning off buzzer
  while (millis() < lastMillis + 75) {
    // Burn, baby, burn!
  }
  digitalWrite(BUZZER_PIN, LOW);

  // Wait a little before turning off LED
  while (millis() < lastMillis + 200) {
    // Burn, baby, burn!
  }
  digitalWrite(LED_PIN, LOW);

  // Halt the card so it does not respond again while in the field
  mfrc522.PICC_HaltA();

  // Remember the UID
  memcpy(&lastUid, &mfrc522.uid, sizeof(lastUid));
}

