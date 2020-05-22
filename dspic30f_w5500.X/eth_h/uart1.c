/*
#define FCY 30000000
 
#include <xc.h>
#include <stdio.h>
#include <delay.h>
#include <libpic30.h>
#include <math.h>
#include <p30F4011.h>
 
 
_FOSC(CSW_FSCM_OFF & FRC_PLL16);  // Fosc=16x7.5MHz, i.e. 30 MIPS
_FWDT(WDT_OFF);                  // Watchdog timer off
_FBORPOR(MCLR_DIS);
 
void UART_Init ( void )
{
    _TRISF5 = 0;           //Look for the exact TX/RX pins of your pic device
    _TRISF4 = 1;           //TX pin must be set as output port and RX input
     
     
    U2BRG = 194;             // 9600 baud rate @ 30 MIPS
    U2MODEbits.UARTEN = 1; // Enable UART
}
 
int main( void )
{
    UART_Init();
 
    while( 1 )
    {
         //To print a variable
//         sprintf(array,"%d",variable);
//         printf(array);
//         printf("\r\n");
// 
         //To print text
         printf("Greetings from Pakistan\r\n");
 
         __delay_ms(1000);
    }
}
*/