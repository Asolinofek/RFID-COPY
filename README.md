# RFID Project

This project reads data from an RDM6300 RFID reader and writes it to an RC522 RFID writer. The data is then verified by reading it back from the RC522.

## Hardware Requirements

- Arduino UNO
- RDM6300 RFID Reader
- RC522 RFID Writer
- Breadboard and connecting wires

## Setup

1. Connect the RDM6300 to the Arduino:
   - VCC to 5V
   - GND to GND
   - TX to pin 6
   - RX to pin 8

2. Connect the RC522 to the Arduino:
   - SDA to pin 10
   - SCK to pin 13
   - MOSI to pin 11
   - MISO to pin 12
   - RST to pin 9
   - 3.3V to 3.3V
   - GND to GND

## Code

The main code file is `RFID_Project.ino`. Upload this code to your Arduino.

## Usage

1. Upload the code to your Arduino.
2. Open the Serial Monitor.
3. Scan a tag with the RDM6300 to read its data.
4. Present a writable tag to the RC522 to write the read data to it.
5. The data will be verified by reading it back from the writable tag.
