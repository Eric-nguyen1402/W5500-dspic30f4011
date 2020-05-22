/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <p30f4011.h>
//------------------------------------------**--------------------------------------

//// Configuration settings
_FOSC(CSW_FSCM_OFF & FRC_PLL16); // Fosc=16x4MHz, Fcy=30MHz , Fpwm= 20kHz
_FWDT(WDT_OFF);                  // Watchdog timer off
_FBORPOR(MCLR_DIS);              // Disable reset pin
//------------------------------------------**--------------------------------------

// Debugging Message Printout enable //

#define _MAIN_DEBUG_

// DSPIC30F SPI PIN Definition //

#define WIZCHIP_SPI_CLK       TRISFbits.TRISF6 =0;
#define WIZCHIP_SPI_MOSI      TRISFbits.TRISF3 =0;
#define WIZCHIP_SPI_MISO      TRISFbits.TRISF2 =1;
#define Wizchip_CS            LATBbits.LATB2   
#define Wizchip_CS_Direction  TRISBbits.TRISB2
#define RD3                    _LATD3
 // SOCKET NUMBER DEFINION for Examples //
#define SOCK_DHCP			0
#define MY_MAX_DHCP_RETRY	3
#define DATA_BUF_SIZE   1024 

//------------------------------------------**--------------------------------------
          
uint8_t gDATABUF[DATA_BUF_SIZE];
volatile uint32_t msTicks; // counts 1ms timeTicks 
uint32_t prevTick;

//------------------------------------------**--------------------------------------
 // Default Network Inforamtion //

wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},
                            .ip = {192, 168, 0, 123},
                            .sn = {255,255,255,0},
                            .gw = {192, 168, 1, 1},
                            .dns = {0,0,0,0},
                            .dhcp = NETINFO_DHCP };

//------------------------------------------**--------------------------------------

// Call back function for W5500 SPI - Theses used to parameter or reg_wizchip_xxx_cbfunc()  //

void InitSPI(void);                     // Initialization of SPI
void SPI_Write(uint8_t data);       // dsPIC SPI Write function
uint8_t SPI_Read(void);             // dsPIC SPI Read function
void WIZCHIP_Select(void);
void WIZCHIP_Deselect(void);
void UART_init(void);

//------------------------------------------**--------------------------------------

// Initialization and Application functions for W5500	//

void network_init(void);
void SysTickIntHandler(void);
void my_ip_assign(void);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

