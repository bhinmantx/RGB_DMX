#include "Conceptinetics.h"
#include "Rdm_Defines.h"
#include "Rdm_Uid.h"

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

unsigned char commandVals[24] = {0x8,
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
0xF0};
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
  //
  // EXAMPLE DESCRIPTION
  //
  // If the first channel comes above 50% the led will switch on
  // and below 50% the led will be turned off
 /* 
  // NOTE:
  // getChannelValue is relative to the configured startaddress
  if ( dmx_slave.getChannelValue (1) > 127 )
    digitalWrite ( ledPin, HIGH );
  else
    digitalWrite ( ledPin, LOW );
    */
    int prevCommand = 0;
int currCommand = 0;
while (true) {
currCommand = dmx_slave.getChannelValue (1);

if (currCommand != prevCommand) {
  prevCommand = currCommand;
digitalWrite ( ledPin, HIGH );
    if ( currCommand < 20 ){
        dmx_ir(currCommand);  
    }else{
      digitalWrite ( ledPin, LOW );
    }
}
else {
  digitalWrite ( ledPin, LOW );
}
}
  delay(1000);
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


