**This is version 1.0V, the next version (1.1V) will include a 3D model to arrange everything together and a circuit diagram.
This product will give you the ability to take any component that transmits at 13.56 MHz and write it onto a tag that operates at 125 kHz! This means you can adapt systems with, for example, iPhone NFC to a home system in a more cost-effective way.

System Uses:

1)Technology Compatibility: Allows systems using different RFID technologies to communicate with each other, providing greater flexibility in integrating components with different technologies.

2)Upgrade and Maintenance: Offers a cost-effective way to upgrade or maintain existing systems by using new tags with newer technology.

3)Extended Capabilities: Enables a low-frequency RFID system (125 kHz) to be supported by readers with higher frequencies (13.56 MHz), thereby expanding the usability of the tags.

4)Cost Savings: Adapts the use of different tags and readers, reducing the need to purchase only one type of technology, which results in cost savings.

# RFID Project

This project reads data from an RDM6300 RFID reader and writes it to an RC522 RFID writer. The data is then verified by reading it back from the RC522.

## Hardware Requirements

- Arduino UNO
- RDM6300 RFID Reader and Writer
- RC522 RFID Writer and Reader
- Connecting wires

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
