#ifndef DEV_BOARD_H
#define DEV_BOARD_H

// olimex LPC-P2138:
// buttons on P0.15/P0.16 (active low)
#define BUT1PIN 	15
#define BUT2PIN 	16
// LEDs on P0.12/P0.13 (active low)
#define LED1PIN  	12
#define LED2PIN  	13

// define LED-Pins as outputs
#define LED_INIT() {  IODIR0 |= (1<<LED1PIN)|(1<<LED2PIN); }
#define YELLOW_LED_ON() { IOCLR0 = (1<<LED1PIN); } 
#define YELLOW_LED_OFF() { IOSET0 = (1<<LED1PIN); } // LEDs active low
#define GREEN_LED_ON() { IOCLR0 = (1<<LED2PIN); }
#define GREEN_LED_OFF() { IOSET0 = (1<<LED2PIN); }

// define Button-Pins as inputs
#define BUTTON_INIT() {  IODIR0 &= ~((1<<BUT1PIN)|(1<<BUT2PIN)); }
// true if button released (active low)
#define BUTTTON1_OFF() (IOPIN0 & (1<<BUT1PIN))
#define BUTTTON2_OFF() (IOPIN0 & (1<<BUT2PIN))

#endif /* DEV_BOARD_H */
