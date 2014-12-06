//F_CPU has been defined in project configuration to 12Mhz

#include"at93c46.h"
//#include<avr/io.h> //included in above included header file
//#include<util/delay.h>
#include<avr/wdt.h>
#include<avr/interrupt.h>

#include"usbdrv/usbdrv.h"

#define USB_LED_OFF	 0
#define USB_LED_ON	 1

#define READ_WORD	2
#define WRITE_WORD	3

#define READ_BYTE 	4
#define WRITE_BYTE	5

static unsigned int buffer;
static uchar address,wordbuff[2],bytebuf,soft_clk_flag=0;

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (usbRequest_t*)data;

	switch(rq->bRequest)
	{
		case READ_WORD:	address = (uchar)rq->wIndex.word;

						soft_clk_flag = (uchar)rq->wValue.word;

						buffer = at93read_word(address,soft_clk_flag);

						wordbuff[0] = buffer>>8;
						wordbuff[1] = buffer;
						usbMsgPtr = wordbuff;
						return sizeof(wordbuff);
		break;

		case WRITE_WORD:write_enable();
						address = (uchar)rq->wIndex.word;
						buffer = rq->wValue.word;
						at93write_word(address,buffer);
		break;

		case READ_BYTE:	address = (uchar)rq->wIndex.word;
						soft_clk_flag = (uchar)rq->wValue.word;
						bytebuf = at93read_byte(address,soft_clk_flag);
						usbMsgPtr = &bytebuf;
						return 1;
		break;

		case WRITE_BYTE:write_enable();
						address = (uchar)rq->wIndex.word;
						bytebuf = (uchar)rq->wValue.bytes[0];
						at93write_byte(address,bytebuf);
		break;

		case USB_LED_ON:	DDRB |= (1<<PB0);
							PORTB |= (1<<PB0);
		break;

		case USB_LED_OFF:PORTB &= ~(1<<PB0);
	}
	return 0;
}


int main()
{

	spi_init();

	wdt_enable(WDTO_1S);

	usbInit();

	usbDeviceDisconnect();

	for(uint8_t i=0;i<255;i++)
	{
		wdt_reset();
		_delay_ms(2);
	}

	usbDeviceConnect();

	sei();

	while(1)
	{
		wdt_reset();
		usbPoll();
	}

	return 0;
}
