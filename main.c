// draw audio waveform and spectrum
// 96x65 NOKIA LCD
// ATmega328P 16MHz
// avr-gcc


#include <stdint.h>
//#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "display.h" //display lib

#include "ffft.h" //chan fft lib


#define clear_bit(reg, bit)  reg &= (~(1<<(bit)))
#define set_bit(reg, bit)  reg |= (1<<(bit))
#define inv_bit(reg, bit)  reg ^= (1<<bit)


int16_t capture[FFT_N]; // ADC buffer //FFT_N = 64
complex_t bfly_buff[FFT_N]; //FFT buffer
uint16_t spektrum[FFT_N/2]; //spectrum output buffer

char strbuff[16];
uint16_t wavebuff[96];

uint16_t cnt1;
volatile uint8_t f=1;


///////////////////////////////////////////////////////////////////////////////////////////////////
void mcu_init(void)
{
ACSR=0b10000000; //выключить компаратор
ADMUX=0b01000000; //опорный источник Vdd //канал 0 АЦП
ADCSRA=0b10000100; //включить АЦП //предделитель
ADCSRB=0b10000000;

sei();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(void)
{
mcu_init();
lcd_init();
_delay_ms(100);

lcd_print("foxy-spectrum");
_delay_ms(800);

while(1)
	{
	for(uint8_t i=0; i<FFT_N; i++)
		{
		set_bit(ADCSRA,ADSC); //запускаем АЦП
		while(bit_is_clear(ADCSRA,ADIF)); //ждем окончания АЦП
		capture[i]=(int16_t)ADC-32768; //записываем полученное значение в буфер
		_delay_us(100); //частота выборок - меняет частный (отображение по x) диапазон спектра
		}

	fft_input(capture, bfly_buff); //fft input ADC array
	fft_execute(bfly_buff); // magic :3
	fft_output(bfly_buff, spektrum); //line spectrum

	lcd_clear();

	for(uint8_t i=1; i<FFT_N; i++) // draw waveform
		{
		lcd_line(i-1+32, 67-capture[i-1]*10/152, i+32, 67-capture[i]*10/152, PIXEL_ON); //related dots
		//lcd_pixel(i+32, 67-capture[i]*10/152, PIXEL_ON); //single dots
		}

	for(uint8_t i=0; i<=FFT_N/2-3; i++) // draw spectrum
		{
		int16_t temp = 67-spektrum[i+2];//4); //делитель меняет диапазон отображения спектра по y
		if(temp>67) temp=67;
		if(temp<0) temp=0;
		lcd_line( i, 67, i, temp, PIXEL_ON);
		}

	_delay_ms(100); //зависит частота обновления экрана

	}
}
