#ifndef __at93c46_h__
#define __at93c46_h__

#include<avr/io.h>
#include<util/delay.h>

/*
#define SCK  7
#define MISO 6
#define MOSI 5
#define SS   4

#define SPIPORT PORTB
#define SPIDDR	DDRB
#define SPIPIN	PINB

void spi_init();

*/

/* Old functions where designed for SPI module of Atmega series. But the new functions below 
	are defined for USI as SPI master found in Tiny Series */

#define SCK 	7 //USCK
#define DO	 	6
#define DI		5
#define SS		4

#define SPIPORT PORTB
#define SPIDDR	DDRB
#define SPIPIN	PINB

#define spi_sr	usi_sr //conversion from old library to new
#define spi_init() usi_spi_init()

void usi_spi_init();

#define clk_period() _delay_us(1)//delay to be used for software clock pin
//toggling. 100us is approx 100khz
	 
typedef unsigned char uchar;

//normal functions 
unsigned char at93read_byte(unsigned char address,unsigned char soft); //here soft is parameter which enables or disables soft clk during read operation 
unsigned int at93read_word(unsigned char address,unsigned char soft);//if soft =0 soft_clk is disabled if soft = 1 soft_clk() is enabled.

void write_enable();//this must be called once before any write operation. However if you don't send a write disable
//command then as long as the chip is powered on after first write_enable() we don't need to send it again until
//reset.

void at93write_byte(unsigned char address, unsigned char data);
void at93write_word(unsigned char address, unsigned int data);



#endif
