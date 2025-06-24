#include <SPI.h>
#include <Wire.h>

// SRAM codes (for 1Mb and 2Mb)
#define RDSR        5
#define WRSR        1
#define READ        3
#define WRITE       2
#define WREN        6
#define WRDI        8

// SRAM code 
#define SEQ_MODE  (0x40 | HOLD)

// SRAM Hold line disabled when HOLD == 1
#define HOLD 0 // HOLD 1 FOR 1MBit, and 0 for 2MBit

//Settings

SPISettings settingsA(20000000, MSBFIRST, SPI_MODE0);
uint32_t maxram = 262144;
long     startAddr = 0x00;

// Set maxram for 1Mbit - 131072 bytes
// Set maxram for 2Mbit - 262144 bytes
// Set maxram for 4Mbit - 524288 bytes

// Other parameters
uint8_t  set_value = 170;
uint32_t number = 8;
byte     cs_Pins[8] =  { 2,3,4,5,6,7,8,9 };

void Spi_SRAM_WriteSeq(uint32_t maxram, uint8_t data_byte, uint8_t cs_pin) {
  uint32_t i;
  SPI.beginTransaction(settingsA);
  digitalWrite(cs_pin, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((uint8_t)(startAddr >> 16) & 0xff);
  SPI.transfer((uint8_t)(startAddr >> 8) & 0xff);
  SPI.transfer((uint8_t)(startAddr & 0xff));
  for (i = 0; i < maxram; i++) {
    SPI.transfer(data_byte);
  }
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
}

uint32_t Spi_SRAM_ReadSeq(uint32_t maxram, uint8_t check_value, uint8_t cs_pin) {
  uint8_t read_byte;
  uint32_t num = 0;
  uint32_t i;
  SPI.beginTransaction(settingsA);
  digitalWrite(cs_pin, LOW);
  SPI.transfer(READ);
  SPI.transfer((uint8_t)(startAddr >> 16) & 0xff);
  SPI.transfer((uint8_t)(startAddr >> 8) & 0xff);
  SPI.transfer((uint8_t)(startAddr & 0xff));
  for (i = 0; i < maxram; i++) {
   read_byte = SPI.transfer(0x00);
   if (read_byte != check_value) {
       num++;
 //      Serial2.print(read_byte);
 //      Serial.print(" at: ");
 //      Serial.print(i);
 //      Serial.print(" : ");
 //      Serial.println(num);
   } 
  }
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
  return num;
}

void setup()
{
    int k;  

    Serial1.begin(9600);
    Serial.begin(9600);
    SPI.begin();

//    while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//    }

// Set up CS_Pins - digital and initially HIGH
  for (k=0; k<number; k++) {
    pinMode(cs_Pins[k], OUTPUT);
    digitalWrite(cs_Pins[k], HIGH);
  }  

// Set each SRAM chip into SEQ mode
    for (k=0; k<number; k++) {  
     digitalWrite(cs_Pins[k], LOW);
     SPI.beginTransaction(settingsA);
     SPI.transfer(WRSR);
     SPI.transfer(SEQ_MODE);
     digitalWrite(cs_Pins[k], HIGH); 
     SPI.endTransaction();

    } 
}

void loop() {
uint32_t numerr,totalerr;
int k,j;
byte incomingByte;

if (Serial1.available() > 0) {
  // read the incoming byte:
  incomingByte = Serial1.read();
  Serial.println(incomingByte);
}

switch (incomingByte)

{
    case 'w': // *** Write using SEQ mode and display errors (+ count)
      Serial1.println("Begin WRITE");
      for (k=0; k<number; k++) {
       Spi_SRAM_WriteSeq(maxram, set_value, cs_Pins[k]);
      }
      Serial1.println("Finish WRITE");
      incomingByte = '*';
      break;
      
    case 'r':
       Serial1.println("Begin READ");
       totalerr = 0;
       for (k=0; k<number; k++) {  
         numerr = Spi_SRAM_ReadSeq(maxram,set_value,cs_Pins[k]);
         totalerr = totalerr + numerr;
         Serial1.print(":");
         Serial1.print(k);
         Serial1.print(":");
         Serial1.println(numerr);
       }
       Serial1.println("Finish READ");
       Serial1.print("Errors: ");
       Serial1.println(totalerr);
       incomingByte = '*';
       break;

      case 's': // *** Check this is SRAMs
      Serial1.println("SRAM");
      incomingByte = '*';
      break;
}



}
