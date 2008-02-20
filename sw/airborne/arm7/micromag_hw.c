/* PNI micromag3 connected on SPI1 */
/* 
   Twisted Logic
   SS    on P0.20
   RESET on P0.29 
   DRDY  on P0.30 ( EINT3 )
*/

/* 
   IMU
   SS on P0.20
   RESET on P1.21
   DRDY on P0.15 ( EINT2 )
*/

#include "micromag.h"

volatile uint8_t micromag_cur_axe;

static void EXTINT_ISR(void) __attribute__((naked));

void micromag_hw_init( void ) {

  /* configure SS pin */
  SetBit(MM_SS_IODIR, MM_SS_PIN); /* pin is output  */
  MmUnselect();                   /* pin idles high */

  /* configure RESET pin */
  SetBit(MM_RESET_IODIR, MM_RESET_PIN); /* pin is output  */
  MmReset();                            /* pin idles low  */

  /* configure DRDY pin */
  /* connected pin to EXINT */ 
  MM_DRDY_PINSEL |= MM_DRDY_PINSEL_VAL << MM_DRDY_PINSEL_BIT;
  SetBit(EXTMODE, MM_DRDY_EINT); /* EINT is edge trigered */
  SetBit(EXTPOLAR,MM_DRDY_EINT); /* EINT is trigered on rising edge */
  SetBit(EXTINT,MM_DRDY_EINT);   /* clear pending EINT */
  
  /* initialize interrupt vector */
  VICIntSelect &= ~VIC_BIT( MM_DRDY_VIC_IT );  /* select EINT as IRQ source */
  VICIntEnable = VIC_BIT( MM_DRDY_VIC_IT );    /* enable it */
  VICVectCntl9 = VIC_ENABLE | MM_DRDY_VIC_IT;
  VICVectAddr9 = (uint32_t)EXTINT_ISR;         // address of the ISR 

}

#include "uart.h"
#include "messages.h"
#include "downlink.h"

void EXTINT_ISR(void) {
  ISR_ENTRY();

  //  DOWNLINK_SEND_BOOZ_DEBUG_FOO(&micromag_status);

  micromag_status = MM_GOT_EOC;
  /* clear EINT */
  SetBit(EXTINT,MM_DRDY_EINT);

  VICVectAddr = 0x00000000;    /* clear this interrupt from the VIC */
  ISR_EXIT();
}

