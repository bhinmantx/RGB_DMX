#include "Conceptinetics.h"
#include "Rdm_Defines.h"
#include "Rdm_Uid.h"

//BASED ON: 
/*
  DMX_Slave.ino - Example code for using the Conceptinetics DMX library
  Copyright (c) 2013 W.A. van der Meeren <danny@illogic.nl>.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/





//
// CTC-DRA-13-1 ISOLATED DMX-RDM SHIELD JUMPER INSTRUCTIONS
//
// If you are using the above mentioned shield you should 
// place the RXEN jumper towards G (Ground), This will turn
// the shield into read mode without using up an IO pin
//
// The !EN Jumper should be either placed in the G (GROUND) 
// position to enable the shield circuitry 
//   OR
// if one of the pins is selected the selected pin should be
// set to OUTPUT mode and set to LOGIC LOW in order for the 
// shield to work
//

//
// The slave device will use a block of 10 channels counting from
// its start address.
//
// If the start address is for example 56, then the channels kept
// by the dmx_slave object is channel 56-66
//
#define DMX_SLAVE_CHANNELS   10 

//
// Pin number to change read or write mode on the shield
// Uncomment the following line if you choose to control 
// read and write via a pin
//
// On the CTC-DRA-13-1 shield this will always be pin 2,
// if you are using other shields you should look it up 
// yourself
//
///// #define RXEN_PIN                2


// Configure a DMX slave controller
DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS );

// If you are using an IO pin to control the shields RXEN
// the use the following line instead
///// DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS , RXEN_PIN );

const int ledPin = 13;
#define IR_COMMAND_DIMMER  25
#define IR_COMMAND_BRIGHTER 27
#define IR_COMMAND_ON 26
#define IR_COMMAND_OFF 23
//older bulb
/*
unsigned char commandVals[24] =  {
  0x8,
  0x10,
  0x18,
  0x20,
  0x28,
  0x30,
  0x48,
  0x50,
  0x58,
  //0x60,
  //0x68,
  0x70,
  0x88,
  0x90,
  0x98,
  0xA0,
  0xA8,
  0xC8,
  0xD0,
  0xD8,
  0xE8,
  0xF0
};
*/

unsigned char commandVals[29] =  {
  0x1,  //swapping pattern?   0
  0x2,  //orange (yellow?)1
  0x1F, //purple  2
  0x20, //purple maybe  3
  0x28, //teal  4
  0x30, //quick fade pattern  5
  0x32, //teal-ish again  6
  0x38, //yello 7
  0x48, //pale blue?  8
  0x50, //sickly yellow 9
  0x58, //pausing mix 10
  0x68, //Blue/purple 11
  0x70, //light purple  12
  0x78, //light blue13
  0x88, //dark blue (true blue) 14
  0x90, //(full blue?)15
  0x98, //some red  16
  0xA8, //white 17
  0xB0, //Another fade  18
  0xB2, //fast multi color strobe 19
  0xD8, //green 20
  0xE8, //orange  21
  0xF0, //Purple  22
  0xF8, //OFF?  23
  0xB8, //DOWN  24
  0xB8, //DOWN  25
  0xB0, //ON  26
  0x90, //UP MABYE?27
  0x90 //UP MABYE?28
};

// the setup routine runs once when you press reset:
void setup() {             
  
  // Enable DMX slave interface and start recording
  // DMX data
  dmx_slave.enable ();  
  
  // Set start address to 1, this is also the default setting
  // You can change this address at any time during the program
  dmx_slave.setStartAddress (1);
  
  // Set led pin as output pin
  pinMode ( ledPin, OUTPUT );

  TCCR0A = (0x01<<COM0A0) | (0x01<<WGM01); // CTC mode, toglle OC0A on compare match
  TCCR0B = (0x01<<CS00); // cpu clock, no prescaler
  OCR0A = 209; // Toggle/clear at 209 to give 38khz at 16mhz clock speed

  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);


  
}

// the loop routine runs over and over again forever:
void loop() 
{
  int prevCommand = -1; // tracking history
  int currCommand = 0;

  int currBrightnessCommand = 0; //hilariously this might be thought of more like howDimmed
  int prevBrightnessCommand = -1; 
  int currBrightnessInt = 0; //Since it's not a 1 to 1, we have to remap, and know if it's changed
  bool hasBrightnessReset = false;
  
  while (true) {
    currCommand = dmx_slave.getChannelValue (1);
    currBrightnessCommand = dmx_slave.getChannelValue (2);
    
    if (currCommand != prevCommand) {
      hasBrightnessReset =  true;
      prevCommand = currCommand;
      digitalWrite ( ledPin, HIGH ); //indicate we're sending
    if ( currCommand < 28 ){
        dmx_ir(currCommand);  
    }else{
      digitalWrite ( ledPin, LOW );
    }
    if (hasBrightnessReset) {
      //We need to get back to the correct brightness level. 
      //If it's reset, previous brightness is 0 (full)
      //we want to get the currently ordered brightness
      int targetBrightnessInt = remapBrightToInt(currBrightnessCommand);

      for(int i = 0; i <= targetBrightnessInt; i++){
        dmx_ir(IR_COMMAND_DIMMER); 
      }
    hasBrightnessReset = false;
    }
  }
  else {
    digitalWrite ( ledPin, LOW );
  }
}

}



void dmx_ir (int command){
  send_command(commandVals[command]);
  return;
}


void send_command(unsigned char data)
{
 unsigned char i;
  
 command_init();
 
 unsigned char address = 0x00;
 
 send_ir_byte(address);
 send_ir_byte(address ^ 0xff);
 
 send_ir_byte(data);
 send_ir_byte(data ^ 0xff);
 
 send_bit_low(); // stop-bit
}

void send_ir_byte(unsigned char data)
{
  unsigned char i;

  for (i=0;i<8;i++) // MSB first
  {
    if ((data<<i) & 0x80)
    {
      send_bit_high();
    } else
    {
      send_bit_low();
    }
  }  
}

void send_bit_high()
{
  // 1.65ms
  ir_on();
  delayMicroseconds(560);
  ir_off();
  delayMicroseconds(1650);
}

void send_bit_low()
{
  // 0.55ms
  ir_on();
  delayMicroseconds(560);
  ir_off();
  delayMicroseconds(560);
}

void command_init()
{
  ir_on();
  delayMicroseconds(9000);
  ir_off();
  delayMicroseconds(4500);
}

void ir_on()
{
  PORTD |= 0x80; // debug output whithout carrier frequency
  
  TIFR0 = (0x01<<TOV0);
  TCNT0 = 0;
  TCCR0A = (0x01<<COM0A0) | (0x01<<WGM01); 
  TCCR0B = (0x01<<CS00); // start the timer, no prescaler
}

void ir_off()
{
  PORTD &= 0b01111111; // debug output whithout carrier frequency
  
  TCCR0B = 0x00; // stop the timer
  TCCR0A = (0x01<<WGM01);
  PORTD &= 0b10111111;
}


int remapBrightToInt (int bright) {
  if (bright >= 0 && bright <=  25 ) 
  { 
    return  0; 
  }   // 0
if (bright >= 26 && bright <=  50) 
  { 
    return  1; 
  }   // 1
if (bright >= 51 && bright <=  75) 
  { 
    return  2 ; 
  }   // 2
if (bright >= 76 && bright <=  100) 
  { 
    return  3 ; 
  }   // 3
if (bright >= 101 && bright <=  125) 
  { 
    return  4 ; 
  }   // 4
if (bright >= 126 && bright <=  150) 
  {
    return  5 ;
  }   // 5
if (bright >= 151 && bright <=  175) 
  { 
    return  6 ; 
  }   // 6
if (bright >= 176 && bright <=  200) 
  { 
    return  7 ; 
  }   // 7
if (bright >= 201 && bright <=  225) 
  { 
    return  8 ; 
  }   // 8
if (bright >= 225 && bright <=  255) 
  { 
    return  9 ; 
  }   // 9

}


