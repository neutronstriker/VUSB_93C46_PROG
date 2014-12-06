/*SPI port and pin defines. Have to define in at93c46.h 
#define SCK  5
#define MISO 4
#define MOSI 3
#define SS   2

#define SPIPORT PORTB
#define SPIDDR	DDRB		*/

#include"at93c46.h"
//At93c46 and all other 93xx series devices have active high ss/cs
#define ss_low() SPIPORT &= ~(1<<SS)
#define ss_high() SPIPORT |= (1<<SS) 

#define clk_ss() ss_low(); _delay_us(1); ss_high() //each instruction starts with rising edge of ss


/*

#define soft_clk() SPIPORT &= ~(1<<SCK); _delay_us(1); SPIPORT |=  (1<<SCK); _delay_us(1); SPIPORT &= ~(1<<SCK) //data is sent out at the rising edge of clk

void spi_init()
{
	SPIDDR |= (1<<MOSI) | (1<<SCK) | (1<<SS);
	
	SPIPORT |= (1<<SS);

	SPCR = (1<<MSTR) | (1<<SPE) | (1<<SPR0); //750Khz spi freq, mode 0
}

unsigned char spi_sr(unsigned char data)
{
	unsigned char read;
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	read = SPDR;
	SPSR |= (1<<SPIF);
	return read;
}


void hexbyte(unsigned char num)
{
	unsigned char a,b;
	a=num & 0x0f;
	b= ((num>>4) & 0x0f);
	if(a>=0 && a<=9)
		a+=0x30;
	else a+=(0x41-0xA);
	if(b>=0 && b<=9)
		b+=0x30;
	else b+=(0x41-0xA);
	uart_send(b);
	uart_send(a);
	uart_send(' ');
}

void hexword(unsigned int num)
{
	unsigned char a[4];
	a[0] = num & 0x000f;
	a[1] = (num>>4) & 0x000f;
	a[2] = (num>>8) & 0x000f;
	a[3] = (num>>12) & 0x000f;

	for(uint8_t i=0;i<4;i++)
	{
		if(a[i] >= 0 && a[i]<=9)
			a[i]+=0x30;
		else a[i]+=(0x41-0xA);
	
	}

	uart_send('0');
	uart_send('x');
	for(uint8_t i=0;i<4;i++)
		uart_send(a[3-i]);
		uart_send(' ');

}

*/


//i actually found out also that if we enable ext.clock mode 1 of USI 
//it was working in experiment but it didn't work here and gave me  a headache
//So it is better to stick with ext clk mode 0 and don't use soft_clk();


uchar usi_sr(uchar data)
{


	USISR |= (1<<USIOIF); //clear the flag

	USIDR = data;
		
	do{
		//spi(3 wire USI mode as master)
		//externally clocked(i.e. since the USITC is connected to USCK pin
		//toggling it from software will actually toggle that pin
		USICR = (1<<USIWM0) | (1<<USICS1) | (1<<USICLK) | (1<<USITC);
		//SPIPORT ^= (1<<SCK); //so either you can toggle the pin directly or
		//or write 1 to USITC in each cycle when USICLK is already 1 so that
		//it will toggle the USCK pin look at the USI block diagram in datasheet
		//to better understand.
		
		//To toggle the pin directly using USCK pin, first we should clear USICLK and USITC.

		//if you want mode 1 instead of mode 0 spi then set USICS0 both and USICS1 
					
		clk_period();
	
	}while(!(USISR & (1<<USIOIF)));//wait until the 4 bit counter overflows
	//since the 4 bit counter actually counts the number of edges during external clocking
	//it increments to 16 for 8 clk pulses
	
	
	return USIDR;
}

void usi_spi_init()
{
	SPIDDR = (1<<SCK) | (1<<DO) | (1<<SS);

}

void soft_clk()
{	
	USICR &= ~((1<<USICS1) | (1<<USICS0) | (1<<USICLK));
	
	for(uint8_t i=0;i<2;i++)
	{
		USICR |= (1<<USITC);
		clk_period();		//toggle the port pin twice
	//we don't need to re-enable all those bits we disabled in USICR because
	//anyhow we program all those bits in usi_spi_sr() every time
	}
}
/*
void at93read_block(unsigned char address,uint8_t organisation,unsigned char blocksize)
{

	unsigned char byte1,byte2;
//	byte1 = 0b00000110; //sb and opcode for read bit 2 is SB (i think start bit) next two bits are opcode
//	byte2 = 0b01111111 & address; //truncate address to 7 bits in 128 x 8 configuration
									//for 64 x 16 configuration truncate address to 6 bit

	//now adjustments part to adjust according to require programming pattern
//actually we don't need to do this because i know that always the address is 7bit and opcode is 10 and sb is 1
//so theres not point in checking this bit and again changing the bit etc
//	if(byte1 & (1<<0))
//		byte2 |= (1<<7);
//
//	byte1 = byte1>>1;

//here we are packing the total 10bits of command sequence into two bits leaving no gap between
//adjacent parameters or command words	

//for 93c series devices chip select is active high

	if(organisation) //here organisation refers to 16 bit or 8 bit organisation of data in chip
	{
		byte1 = 0b00000011;
		byte2 = 0b01111111 & address;
		
		ss_high();

		spi_sr(byte1);
		spi_sr(byte2);
		
	//	SPCR &= ~(1<<SPE);
		soft_clk();//soft clock removes the dummy 0 before the data stage.
	//	SPCR |= (1<<SPE);

		for(uint8_t i=0;i<blocksize;i++)
		{
	
			hexbyte(spi_sr(0x00));

		}

		ss_low();
	}
	else 
	{
		byte1 = 0b00000001;
		byte2 = 0b10000000 | address;
	
		ss_high();
			
		spi_sr(byte1);
		spi_sr(byte2);

		unsigned int data=0;

	//	SPCR &= ~(1<<SPE);
		soft_clk();//soft clock removes the dummy 0 before the data stage.
	//	SPCR |= (1<<SPE);
		
		for(uint8_t i=0;i<blocksize;i++)
		{
	
			data = spi_sr(0xff);
			data = data<<8;
			data = data | spi_sr(0xff);

			hexword(data);

		}

		ss_low();
	}

	//to discard dummy 0 ouput before that 93series chips output before real data in every new read instruction
	//we provide we soft_clk() which overrides the spi SCLK, however if i keep the ss pin HIGH and keep clocking the 
	//the clock, the address pointer increments automatically and gives me data until it reaches
	//the last address then it rolls over.

}											


*/
unsigned char at93read_byte(unsigned char address, unsigned char soft)
{

	unsigned char data;

	ss_high();
	spi_sr(0x03);
	spi_sr(0x7f & address);

	//remove dummy bit 0	

//	SPCR &= ~(1<<SPE); //not needed for usi soft clock function
//	soft_clk(); 
//	SPCR |= (1<<SPE);

//i don't know my but because of some reason it is giving 
 //wrong results if i give soft clock in USI mode
//but if i don't use soft clock at all then it is giving correct results
//so anyway may be it is because of the  USITC toggling effect which is
//quite unpredictable that during 1st toggle it will be in which state.

// i finally found out the reason if we change the USI ext. clock to mode 1
//i.e. USICS0 = 1 then we need soft_clk. however after reading the data
//sheets of both device i am a bit confused. Since tin2313 datasheet 
//says that ext.clock mode 0 samples DI at rising edge of SCK and at932c46
//ds says that DO is synchronised with rising edge of SCK.
//SO according to above statements data we should need soft_clk in mode 0
//not in mode 1.


//from testing i found out that on some chips the softclk is required and on
//some chips its not so i gave the softclk and select it as an optional feature

	if(soft)
		soft_clk();

	data = spi_sr(0xff);	
	
	ss_low();
	
	return data;

	
}


unsigned int at93read_word(unsigned char address, unsigned char soft)
{
	unsigned int data=0;
	ss_high();
	spi_sr(0b00000001);
	spi_sr(0b10000000 | address);


//	SPCR &= ~(1<<SPE); //the spi mode disables direct access of PORT pins so to generate a software
//not needed for usi soft clock						//clk by directly accessing SCK pin we need to first disable the SPI mode
//	soft_clk();
//	SPCR |= (1<<SPE); //after our work is done we should reenable it as we will need it to receive data
	
	//here also you may need to change format according to little endian or big endian requirement

//from testing i found out that on some chips the softclk is required and on
//some chips its not so i gave the softclk and select it as an optional feature

	if(soft)
		soft_clk();	

	data = spi_sr(0xff);
	data = data <<8;
	data = data | spi_sr(0xff);

	//before i used this mechanism to read 2 words and Left shift 1st word once and 
	//then assign the MSB of 2nd word to LSB of first word which is a very clumsy and inefficient technique
	
	/*dummy = spi_sr(0xff); 
	dummy = dummy<<8;
	dummy = dummy | spi_sr(0xff);

	data = data<<1;
	if(dummy & (1<<15))
		data |= (1<<0);
*/
	ss_low();

	return data;
}
	
void write_enable()
{
	unsigned char byte1,byte2;
	byte1 = 0b00000010;
	byte2 = 0b01100000;


	ss_low();
	ss_high();
	spi_sr(byte1);
	spi_sr(byte2);
	ss_low();
	_delay_ms(10);
}

void at93write_byte(unsigned char address,unsigned char data)
{
	
	unsigned char byte1,byte2;
	
	byte1 = 0b00000010; //part of opcode, sb = 1 and opcode is 01
	byte2 = 0x80 | address; //part of opcode and address

	ss_high();
	
	spi_sr(byte1);
	spi_sr(byte2);
	spi_sr(data);
	
	
	ss_low();//this has to brought low immediately after sending the last bit of data so that 
			//self timed programming cycle with in begins
	ss_high();
	

	while(!(SPIPIN & (1<<DI)));
	ss_low();
	_delay_ms(10);

}

void at93write_word(unsigned char address,unsigned int data)
{
	ss_high();
	spi_sr(0b00000001);
	spi_sr(0b01000000 | address);
	
	//change this for converting writing mechanism according to little endian or big endian
	
	spi_sr(data>>8);
	spi_sr(data);

	ss_low();
	ss_high();
	while(!(SPIPIN & (1<<DI)));
	ss_low();
	_delay_ms(10);
	
}

/*
int main()
{

	uart_init();
	spi_init();
	u_print("\r\n\r\n");
	u_print("hello i am initialised\r\n");

	u_print("please press a key to begin\r\n");
	uart_receive();//wait for a keyress
	u_print("now writing data 0xAA to locations 0 to 5\r\n");

	write_enable(); //by default when the 93s series is powered on the writing mode is disabled
					//by sending the write enable command it is enableed until a write disable command
					//is sent or vcc fall below to reset itself

					//for safety purposes a write disable command must be issued after writing 
					//is no longer necessary in the application, so that data doesn't get corrupt
	

	for(uint8_t i=0;i<5;i++)
	{

		at93write_byte(i,0x87);
		
	}
	
	_delay_ms(500);

	
	u_print("Now lets write few words of data\r\n");
	write_enable();

	for(uint8_t i=0;i<5;i++)
	at93write_word(0,0x87AA);
	
	//now lets read the chip
	u_print("now reading data from at93c46 in word mode\r\n");
	_delay_ms(100);

	for(uint8_t i=0;i<128;i++)
	hexword(at93read_word(i));
	
	u_print("now reading data from at93c46 in Block mode\r\n");
	at93read_block(0,0,5); //address 0, organisation 16bit and block size = 5
	
	while(1);
	return 0;

} */
