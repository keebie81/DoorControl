
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

// Motor speed and direction definitions
const uint8_t MOTOR_SPEED = 200;
const uint8_t MOTOR_CW = 0;
const uint8_t MOTOR_CCW = 1;

//Lockitron limit switches
const uint8_t SW_1A_PIN = A0;
const uint8_t SW_1B_PIN = A1;
const uint8_t SW_2A_PIN = A2;
const uint8_t SW_2B_PIN = A3;

//TB6612 Wiring
const uint8_t BIN1_PIN = 2;
const uint8_t BIN2_PIN = 3;
const uint8_t PWMB_PIN = 4;

// Switch state variables
uint8_t sw_1a;
uint8_t sw_1b;
uint8_t sw_2a;
uint8_t sw_2b;

#define OPEN true
#define CLOSE false
int openlock = 5;
int openlockstatus = 0;
int closelock = 6;
int closelockstatus = 0;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

// Change the HEX values to the tags you scanned
byte tag1[4] = {0x12, 0x73, 0xE5, 0x56}; // RFID Tag 1
byte tag2[4] = {0x02, 0x30, 0xE4, 0x56}; // RFID Tag 2
byte tag3[4] = {0xB2, 0x0F, 0xE5, 0x56}; // RFID Tag 3

int tag1count = 0; // Tag 1 unlock status
int tag2count = 0; // Tag 2 unlock status
int tag3count = 0; // Tag 3 unlock status
int door_data = 0; // Door unlock staus


void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  // Set up motor pins
  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);
  pinMode(PWMB_PIN, OUTPUT);

  pinMode(openlock, INPUT);
  pinMode(closelock, INPUT);
  pinMode(SW_1A_PIN, INPUT_PULLUP);
  pinMode(SW_1B_PIN, INPUT_PULLUP);
  pinMode(SW_2A_PIN, INPUT_PULLUP);
  pinMode(SW_2B_PIN, INPUT_PULLUP);


  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

}

void loop() {


  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  // Checks if this was same as last tag read
  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println();
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 5; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];

    }

    // Print out the tag NUID
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();

    // Checks if it matches RFID Tag 1
    if (rfid.uid.uidByte[0] == tag1[0] &
        rfid.uid.uidByte[1] == tag1[1] &
        rfid.uid.uidByte[2] == tag1[2] &
        rfid.uid.uidByte[3] == tag1[3])
    {
      if (tag1count == 0 & tag2count == 0 & tag3count == 0)
      {
        tag1count = 1; // Sets Tag 1 to unlock
        Serial.println(F("unlock"));
      }
      else
      {
        resetall();
      }


    }

    // Check if it matches RFID Tag 2
    if (rfid.uid.uidByte[0] == tag2[0] &
        rfid.uid.uidByte[1] == tag2[1] &
        rfid.uid.uidByte[2] == tag2[2] &
        rfid.uid.uidByte[3] == tag2[3])
    {
      if (tag1count == 1 & tag2count == 0 & tag3count == 0) // Checks if Tag 1 has only been unlocked
      {
        tag2count = 1; // Unlocks Tag 2
        Serial.println(F("unlock"));
      }
      else
      {
        resetall();
      }


    }
    // Check if it matches RFID Tag 3
    if (rfid.uid.uidByte[0] == tag3[0] &
        rfid.uid.uidByte[1] == tag3[1] &
        rfid.uid.uidByte[2] == tag3[2] &
        rfid.uid.uidByte[3] == tag3[3])
    {
      if (tag1count == 1 & tag2count == 1 & tag3count == 0) // Checks if only Tag 1 and Tag 2 are unlocked
      {
        tag3count = 1; // Unlocks Tag 3
        door_data = 1; // Now that all tags have been unlocked in correct order sets door to unlock
        Serial.println(F("unlock"));
      }
      else
      {
        resetall();
      }


    }
  }

  else
  {
    Serial.println(F("Card read previously."));
    resetall();
  }

  openlockstatus = digitalRead(openlock); // Looks for a signal from the MKR1000
  closelockstatus = digitalRead(closelock); // Looks for a signal from the MKR1000

  // Prints out status of values for debugging
  Serial.println(tag1count);
  Serial.println(tag2count);
  Serial.println(tag3count);
  Serial.println(door_data);
  Serial.println(openlockstatus);
  Serial.println(closelockstatus);


  // If door was set to unlock it opens the lock
  if (door_data == 1) {
    unlock();
    resetall();
  }

  // If MKR1000 sent high on pin 6 will unlock door
  if (openlockstatus == HIGH) {
    unlock();
    resetall();
  }


  // If MKR1000 sent high on pin 5 will lock door
  if (closelockstatus == HIGH) {
    lock();
    resetall();
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}


// Resets all lock status values to low
void resetall() {
  tag1count = 0;
  tag2count = 0;
  tag3count = 0;
  door_data = 0;
  Serial.println("All values have been reset");

}

void lock()
{
  // Move motor to lock the deadbolt
  moveMotor(MOTOR_SPEED, MOTOR_CW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_1a == 0) && (sw_1b == 1) &&
            (sw_2a == 0) && (sw_2b == 1)) );
  stopMotor();
  delay(100);

  // Move motor back to starting position
  moveMotor(MOTOR_SPEED, MOTOR_CCW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_2a == 1) && (sw_2b == 1)) );
  stopMotor();
}

void unlock()
{
  // Move motor to lock the deadbolt
  moveMotor(MOTOR_SPEED, MOTOR_CCW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_1a == 1) && (sw_1b == 0) &&
            (sw_2a == 1) && (sw_2b == 0) ));
  stopMotor();
  delay(100);

  // Move motor back to starting position
  moveMotor(MOTOR_SPEED, MOTOR_CW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_2a == 1) && (sw_2b == 1)) );
  stopMotor();
}

void moveMotor(uint8_t spd, uint8_t dir)
{
  boolean bin1;
  boolean bin2;

  // Define direction pins
  if ( dir )
  {
    bin1 = HIGH;
    bin2 = LOW;
  }
  else
  {
    bin1 = LOW;
    bin2 = HIGH;
  }

  // Set motor to GO!
  analogWrite(PWMB_PIN, spd);
  digitalWrite(BIN1_PIN, bin1);
  digitalWrite(BIN2_PIN, bin2);
}

void stopMotor()
{
  analogWrite(PWMB_PIN, 0);
}

