/*
   Test 2114 memory chip tester

   Version 0.3

   Danjovic 2018

   Released under GPL V2.0


   Pin assignment:
   2114  AVR   Arduino
    A0   PC0   A0 (14)
    A1   PC1   A1 (15)
    A2   PC2   A2 (16)
    A3   PC3   A3 (17)
    A4   PC4   A4 (18)
    A5   PC5   A5 (19)
    A6   PB0    8
    A7   PB1    9
    A8   PB2   10
    A9   PB3   11
    /WE  PB4   12
    /CE  PB5   13
    IO1  PD4    4
    IO2  PD5    5
    IO3  PD6    6
    IO4  PD7    7
*/

#include<avr/io.h>

#define WE  (1<<4)
#define CS  (1<<5)

#define disable_2114()     PORTB |= ( CS + WE )

#define enable_CS_2114()   PORTB &= ~CS
#define disable_CS_2114()  PORTB |=  CS

#define enable_WE_2114()   PORTB &= ~WE
#define disable_WE_2114()  PORTB |=  WE

#define float_data_lines_2114() do { DDRD &= 0x0f; PORTD |= 0xf0; } while (0)

#define activate_data_lines_2114() DDRD |= 0xf0


inline void Set_address_2114 (uint16_t address) {
  // DDRB = 0x3F;  // All outputs
  // DDRC = 0x3F;  // All outputs

  disable_2114() ;
  PORTC = (uint8_t)(address & (0x3f));       //  Set low address pins (a0..a5)
  PORTB &= (0x30);                //  keep WE and CS as is
  PORTB |= (uint8_t)((address >> 6) & 0x0f); //  Set high address pins (a6..a9)
}

inline void Set_data_2114 ( uint8_t data) {
  PORTD &= 0x0f;                // clear upper bits
  PORTD |= (uint8_t)((data << 4) & 0xf0); // set data in upper four bits
}

inline uint8_t Read_data_2114(void ) {
  return ((PIND >> 4 ) & 0x0f );
}


inline void Write_2114 (uint16_t address, uint8_t data) {
  disable_2114();
  Set_address_2114 (address);
  Set_data_2114 ( data );
  enable_CS_2114();
  enable_WE_2114();

  activate_data_lines_2114();
  // Wait 4 cycles -> 250ns @ 16MHz
  asm ("NOP");
  asm ("NOP");
  asm ("NOP");
  asm ("NOP");
  disable_2114();

  float_data_lines_2114();

}

inline uint8_t Read_2114 (uint16_t address) {
  uint8_t data;
  disable_2114();
  float_data_lines_2114();
  Set_address_2114 (address);
  disable_WE_2114();
  enable_CS_2114();
  // Wait 4 cycles -> 250ns @ 16MHz
  asm ("NOP");
  asm ("NOP");
  asm ("NOP");
  asm ("NOP");

  data = Read_data_2114();
  disable_CS_2114();
  return data;
}


void   Perform_test_2114( uint8_t pattern) {
  uint16_t address;

  // Write test pattern
  for (address = 0; address < 1024; address++) {
    Write_2114 (address, pattern);
  }

  // Read back test pattern
  for (address = 0; address < 1024; address++) {
    if (Read_2114(address) != pattern) {
      Serial.print ("Fail at ");
      Serial.print (address, DEC);
      break;
    }
  }
  //
  if (address == 1024) Serial.print ("Ok");
}



void setup() {
  // Initialize I/O
  DDRB = 0x3F;  // All outputs
  DDRC = 0x3F;  // All outputs
  DDRD = 0x00;  // All inputs
  PORTB = (1 << 5) | (1 << 4); // signals /CE and /WE high
  PORTC = 0;
  PORTD = 0xF0; // Pullups on D4..D7

  // Initialize serial port
  Serial.begin(9600);
  Serial.print("\nTest 2114");
}



void loop() {
  for (;;) {
    Serial.print("\n\nStarting...");
    // Iterate through memory with different patterns
    Serial.print ("\nTest pattern 0000: ");
    Perform_test_2114(0x00);
    Serial.print ("\nTest pattern 0101: ");
    Perform_test_2114(0x5);
    Serial.print ("\nTest pattern 1010: ");
    Perform_test_2114(0xA);
    Serial.print ("\nTest pattern 1111: ");
    Perform_test_2114(0xF);
    Serial.print ("\nTest pattern 0001: ");
    Perform_test_2114(0x01);
    Serial.print ("\nTest pattern 0010: ");
    Perform_test_2114(0x2);
    Serial.print ("\nTest pattern 0100: ");
    Perform_test_2114(0x4);
    Serial.print ("\nTest pattern 1000: ");
    Perform_test_2114(0x8);
    delay(500);
  }
}

