// *** =====================================================================
// *** 23LC1024 Seqential test
// *** CDF - 26/07/2017 with alot of help from GitHUB spiRAM extended
// *** =====================================================================
// ***
// *** Initilisation 

#include <SPI.h>
#include <Wire.h>

// SRAM opcodes
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2

// SRAM Hold line disabled when HOLD == 1
#define HOLD 0 // HOLD 1 FOR 1MBit, and 0 for 2MBit

// SRAM modes
#define PAGE_MODE (0x80 | HOLD)
#define SEQ_MODE  (0x40 | HOLD)

//Pin Clicker - this would be pins 10 and 9 on an Arduino Uno
byte ss_Pin[8] =  { 254, 253, 251, 247, 239, 223, 191, 127 };
byte led_Pin[8] = { 38,37,39,40,38,37,39,40 };
uint32_t number = 8;

// 1Mbit 23LC1024 Chip & Checkerboard pattern as test
//uint32_t maxram = 131072; // 1MBit
uint32_t maxram = 262144; //  2MBit
uint32_t errlevel = 500;

//uint32_t errlevel = 300000; //For test purposes - i.e. allows us to see high errors for diagnosis
uint8_t test_val = 0xAA;

int incomingByte = 0;

// *** Set up the various subroutines based on the spiRAM Extender in Github
// *** =====================================================================
// *** 
// *** Enable and Disable

// *** Enable CS cia I2C for tranfer of data 
void enable(uint8_t cs_pin) {
  Wire.beginTransmission(0x20);
  Wire.write(0x12);
  Wire.write(cs_pin);
  Wire.endTransmission();
//  digitalWrite(cs_pin, LOW);
}

// *** Diable CS cia I2C for tranfer of data 
void disable(uint8_t cs_pin) {
  Wire.beginTransmission(0x20);
  Wire.write(0x12);
  Wire.write(255);
  Wire.endTransmission();
//  digitalWrite(cs_pin, HIGH);
}

// *** Prepare SRAM for tranfer of data 
void prepare(char mode, char action, long address, uint8_t cs_pin) {
  set_mode(mode,cs_pin);
  enable(cs_pin);
  SPI.transfer(action);
  SPI.transfer((char)(address >> 16));
  SPI.transfer((char)(address >> 8));
  SPI.transfer((char)address);
}

// *** Set the mode 
void set_mode(char mode, uint8_t cs_pin) {
    enable(cs_pin);
    SPI.transfer(WRSR);
    SPI.transfer(mode);
    disable(cs_pin);
}

// *** Write data in seq mode
void write_seq(uint32_t ramsize, uint8_t value, uint8_t cs_pin) {
  int i;
  prepare(SEQ_MODE,WRITE,0x00,cs_pin);
  for (i = 0; i < ramsize; i++) {
    SPI.transfer(value);
  }
  disable(cs_pin);
}


// *** Read data in seq mode
uint32_t read_seq(uint32_t ramsize, uint8_t value, uint8_t cs_pin) {
  int i;
  uint8_t read_byte;
  uint32_t num = 0;
  prepare(SEQ_MODE,READ,0x00,cs_pin);
  for (i = 0; i < ramsize; i++) {
    read_byte = SPI.transfer(0xFF);
  if (read_byte != value) {
        num++;
    } 
  }
  return num;
  disable(cs_pin);
}

// *** Initial routine; this includes the writing of the data
// *** =====================================================================
// ***
void setup() { 
  uint32_t k;
  Serial.begin(9600);
  Serial2.begin(9600);
  SPI.begin();
  
  //**  Setup the IC2 bus to control the CS lines via an EXPAND 2
  Wire.begin(); 
  Wire.beginTransmission(0x20);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
  
  SPI.setClockDivider(SPI_CLOCK_DIV4);

  for (k=0; k<number; k++) {
    pinMode(led_Pin[k], OUTPUT);
  }  
}


// *** Loop routine; this reads and checks the data
// *** ===================================================================== 
// ***
void loop() {
uint32_t numerr,totalerr, numsram;
int k,j;

if (Serial.available() > 0) {
  // read the incoming byte:
  incomingByte = Serial.read();
}

switch (incomingByte)

{
    case 'w': // *** Write using SEQ mode and display errors (+ count)
      Serial.println("Begin WRITE");
      for (k=0; k<number; k++) {
       Serial2.print(":");
       Serial2.println(k);
       digitalWrite(led_Pin[k],HIGH);
       write_seq(maxram, test_val, ss_Pin[k]);
       digitalWrite(led_Pin[k],LOW);
       disable(ss_Pin[k]); 
      }
      Serial.println("Finish WRITE");
      incomingByte = '*';
      break;
      
    case 'r':
       Serial.println("Begin READ");
       totalerr = 0;
       numsram = 0;
       for (k=0; k<number; k++) {  
         digitalWrite(led_Pin[k],HIGH);
         numerr = read_seq(maxram,test_val,ss_Pin[k]);
         digitalWrite(led_Pin[k],LOW);
         if (numerr <= errlevel) {
           totalerr = totalerr + numerr;
           numsram++;
         } 
         Serial.print(":");
         Serial.print(k);
         Serial.print(":");
         Serial.println(numerr);
       }
       Serial.println("Finish READ");
       Serial.print("Errors: ");
       Serial.println(totalerr);
       incomingByte = '*';
       break;

      case 's': // *** Check this is SRAMs
      Serial.println("SRAM");
      incomingByte = '*';
      break;
}
}
