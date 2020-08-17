/*
* main.c    ATmega328P    F_CPU = 16000000 Hz
*
* Created on: 27.05.2019
*     Author: admin
*/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// TDA1543A CONNECTIONS
// CLOCK - PB5
// WS -    PB0
// DATA -  PB3
#define TDA_PORT DDRB
#define WS_PIN PB0
#define CLOCK_PIN PB5
#define DATA_PIN PB3
#define SS_PIN PB2	// not used, but should be config as output


volatile uint8_t probe_idx;

void tda1543a_write_sample(int16_t channel_l, int16_t channel_r);
int16_t read_sinus_sample(int s);
void spi_init(void);

uint16_t sin_tab[256] = {
	32768, 33572, 34375, 35178, 35979, 36779, 37575, 38369, 39160, 39947, 40729,
	41507, 42279, 43046, 43807, 44560, 45307, 46046, 46777, 47500, 48214, 48919,
	49613, 50298, 50972, 51635, 52287, 52927, 53555, 54170, 54773, 55362, 55938,
	56499, 57047, 57579, 58097, 58600, 59087, 59558, 60013, 60451, 60873, 61278,
	61666, 62036, 62389, 62724, 63041, 63339, 63620, 63881, 64124, 64348, 64553,
	64739, 64905, 65053, 65180, 65289, 65377, 65446, 65496, 65525, 65535, 65525,
	65496, 65446, 65377, 65289, 65180, 65053, 64905, 64739, 64553, 64348, 64124,
	63881, 63620, 63339, 63041, 62724, 62389, 62036, 61666, 61278, 60873, 60451,
	60013, 59558, 59087, 58600, 58097, 57579, 57047, 56499, 55938, 55362, 54773,
	54170, 53555, 52927, 52287, 51635, 50972, 50298, 49613, 48919, 48214, 47500,
	46777, 46046, 45307, 44560, 43807, 43046, 42279, 41507, 40729, 39947, 39160,
	38369, 37575, 36779, 35979, 35178, 34375, 33572, 32768, 31963, 31160, 30357,
	29556, 28756, 27960, 27166, 26375, 25588, 24806, 24028, 23256, 22489, 21728,
	20975, 20228, 19489, 18758, 18035, 17321, 16616, 15922, 15237, 14563, 13900,
	13248, 12608, 11980, 11365, 10762, 10173, 9597,  9036,  8488,  7956,  7438,
	6935,  6448,  5977,  5522,  5084,  4662,  4257,  3869,  3499,  3146,  2811,
	2494,  2196,  1915,  1654,  1411,  1187,  982,   796,   630,   482,   355,
	246,   158,   89,    39,    10,    0,     10,    39,    89,    158,   246,
	355,   482,   630,   796,   982,   1187,  1411,  1654,  1915,  2196,  2494,
	2811,  3146,  3499,  3869,  4257,  4662,  5084,  5522,  5977,  6448,  6935,
	7438,  7956,  8488,  9036,  9597,  10173, 10762, 11365, 11980, 12608, 13248,
	13900, 14563, 15237, 15922, 16616, 17321, 18035, 18758, 19489, 20228, 20975,
	21728, 22489, 23256, 24028, 24806, 25588, 26375, 27166, 27960, 28756, 29556,
30357, 31160, 31963};


int main(void) {
	spi_init();
	while (1) {
		int16_t  LeftSample_16Bit   = read_sinus_sample(probe_idx);
		int16_t  RightSample_16Bit  = read_sinus_sample(probe_idx);
		tda1543a_write_sample(LeftSample_16Bit,RightSample_16Bit);
		probe_idx++;
	}
}

void tda1543a_write_sample(int16_t channel_l, int16_t channel_r){
	TDA_PORT |= (1<<WS_PIN);	// select left channel
	SPDR = 0;	// align to 24 bit format
	while(!(SPSR & (1<<SPIF)));
	SPDR = (channel_l>>8);	// from MSB
	while(!(SPSR & (1<<SPIF)));
	SPDR = channel_l;	// to LSB
	while(!(SPSR & (1<<SPIF)));
	TDA_PORT &= ~(1<<WS_PIN);	// select right channel
	SPDR = 0;	// align to 24 bit format
	while(!(SPSR & (1<<SPIF)));
	SPDR = (channel_r>>8);	// from MSB
	while(!(SPSR & (1<<SPIF)));
	SPDR = channel_r;	// to LSB
	while(!(SPSR & (1<<SPIF)));
}

int16_t read_sinus_sample(int s){
	return (int16_t)(sin_tab[s]-32768);
}

void spi_init(void){
	// MOSI (DATA), SCK (BCK) and SS (WS) - outputs
	TDA_PORT |=(1<<WS_PIN)|(1<<CLOCK_PIN)|(1<<DATA_PIN)|(1<<SS_PIN);
	// enable, master, CLK/2, MSb first, sampling on rising clock edge, data change on falling edge, idle clk = 0
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
	SPSR = (1<<SPI2X);
}