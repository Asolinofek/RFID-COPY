#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

// RDM6300 Settings
#define RDM6300_RX_PIN 6
#define RDM6300_TX_PIN 8

const int BUFFER_SIZE = 14; // RFID DATA FRAME FORMAT: 1byte head (value: 2), 10byte data (2byte version + 8byte tag), 2byte checksum, 1byte tail (value: 3)
const int DATA_SIZE = 10; // 10byte data (2byte version + 8byte tag)
const int DATA_VERSION_SIZE = 2; // 2byte version (actual meaning of these two bytes may vary)
const int DATA_TAG_SIZE = 8; // 8byte tag
const int CHECKSUM_SIZE = 2; // 2byte checksum

SoftwareSerial ssrfid(RDM6300_RX_PIN, RDM6300_TX_PIN);

uint8_t buffer[BUFFER_SIZE]; // used to store an incoming data frame 
int buffer_index = 0;
long extractedTag = 0;

// RC522 Settings
#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

MFRC522::MIFARE_Key key; // Define a MIFARE_Key object

void setup() {
  Serial.begin(9600); 

  // Initialize RDM6300
  ssrfid.begin(9600);
  ssrfid.listen(); 

  // Initialize RC522
  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522 card

  // Set the key (default key for new MIFARE cards)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("INIT DONE");
}

void loop() {
  if (ssrfid.available() > 0) {
    bool call_extract_tag = false;
    
    int ssvalue = ssrfid.read(); // read 
    if (ssvalue == -1) { // no data was read
      return;
    }

    if (ssvalue == 2) { // RDM630/RDM6300 found a tag => tag incoming 
      buffer_index = 0;
    } else if (ssvalue == 3) { // tag has been fully transmitted       
      call_extract_tag = true; // extract tag at the end of the function call
    }

    if (buffer_index >= BUFFER_SIZE) { // checking for a buffer overflow (It's very unlikely that an buffer overflow comes up!)
      Serial.println("Error: Buffer overflow detected!");
      return;
    }
    
    buffer[buffer_index++] = ssvalue; // everything is alright => copy current value to buffer

    if (call_extract_tag == true) {
      if (buffer_index == BUFFER_SIZE) {
        extractedTag = extract_tag();
        writeTagToRC522(extractedTag);
        readTagFromRC522(); // Read back the data to verify
      } else { // something is wrong... start again looking for preamble (value: 2)
        buffer_index = 0;
        return;
      }
    }    
  }
}

unsigned extract_tag() {
    uint8_t msg_head = buffer[0];
    uint8_t *msg_data = buffer + 1; // 10 byte => data contains 2byte version + 8byte tag
    uint8_t *msg_data_version = msg_data;
    uint8_t *msg_data_tag = msg_data + 2;
    uint8_t *msg_checksum = buffer + 11; // 2 byte
    uint8_t msg_tail = buffer[13];

    // print message that was sent from RDM630/RDM6300
    Serial.println("--------");

    Serial.print("Message-Head: ");
    Serial.println(msg_head);

    Serial.println("Message-Data (HEX): ");
    for (int i = 0; i < DATA_VERSION_SIZE; ++i) {
      Serial.print(char(msg_data_version[i]));
    }
    Serial.println(" (version)");
    for (int i = 0; i < DATA_TAG_SIZE; ++i) {
      Serial.print(char(msg_data_tag[i]));
    }
    Serial.println(" (tag)");

    Serial.print("Message-Checksum (HEX): ");
    for (int i = 0; i < CHECKSUM_SIZE; ++i) {
      Serial.print(char(msg_checksum[i]));
    }
    Serial.println("");

    Serial.print("Message-Tail: ");
    Serial.println(msg_tail);

    Serial.println("--");

    long tag = hexstr_to_value((char*)msg_data_tag, DATA_TAG_SIZE);
    Serial.print("Extracted Tag: ");
    Serial.println(tag);

    long checksum = 0;
    for (int i = 0; i < DATA_SIZE; i+= CHECKSUM_SIZE) {
      long val = hexstr_to_value((char*)msg_data + i, CHECKSUM_SIZE);
      checksum ^= val;
    }
    Serial.print("Extracted Checksum (HEX): ");
    Serial.print(checksum, HEX);
    if (checksum == hexstr_to_value((char*)msg_checksum, CHECKSUM_SIZE)) { // compare calculated checksum to retrieved checksum
      Serial.print(" (OK)"); // calculated checksum corresponds to transmitted checksum!
    } else {
      Serial.print(" (NOT OK)"); // checksums do not match
    }

    Serial.println("");
    Serial.println("--------");

    return tag;
}

long hexstr_to_value(char *str, unsigned int length) { // converts a hexadecimal value (encoded as ASCII string) to a numeric value
  char* copy = (char*)malloc((sizeof(char) * length) + 1); 
  memcpy(copy, str, sizeof(char) * length);
  copy[length] = '\0'; 
  // the variable "copy" is a copy of the parameter "str". "copy" has an additional '\0' element to make sure that "str" is null-terminated.
  long value = strtol(copy, NULL, 16);  // strtol converts a null-terminated string to a long value
  free(copy); // clean up 
  return value;
}

void writeTagToRC522(long tag) {
  byte dataBlock[] = {
    (byte)((tag >> 24) & 0xFF), (byte)((tag >> 16) & 0xFF),
    (byte)((tag >> 8) & 0xFF), (byte)(tag & 0xFF),
    0x00, 0x00, 0x00, 0x00, // Padding or additional data
    0x00, 0x00, 0x00, 0x00, // Padding or additional data
    0x00, 0x00, 0x00, 0x00  // Padding or additional data
  };

  MFRC522::StatusCode status;
  byte block = 1; // Choose the block to write, usually 1 for data

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("No card present");
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Failed to read card");
    return;
  }

  // Authenticate with the tag
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write the data to the tag
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(block, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("Data was written to the tag successfully!");

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

void readTagFromRC522() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
  
  byte block = 1; // Read block 1 where data was written
  byte buffer[18]; // Buffer to store read data
  byte size = sizeof(buffer);

  // Authenticate with the tag
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read the block
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.print("Data in block: ");
  for (byte i = 0; i < size; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}
