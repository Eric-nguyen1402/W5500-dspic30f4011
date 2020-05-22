
#include "xc.h"
#include <p30f4011.h>
#include <uart.h>
#include <libpic30.h>
#include <stdio.h>
#include <spi.h>
#include "dns.h"
#include <stdbool.h>
#include "dhcp.h"
#include <string.h>

//------------------------------------------**--------------------------------------
_FOSC(CSW_FSCM_OFF & FRC_PLL16);  // Fosc=16x4MHz, i.e. 30 MIPS  Fcy=16MHz
_FWDT(WDT_OFF);                  // Watchdog timer off
_FBORPOR(MCLR_DIS);


//------------------------------------------**--------------------------------------

#define WIZCHIP_SPI_CLK       TRISFbits.TRISF6 =0;
#define WIZCHIP_SPI_MOSI      TRISFbits.TRISF3 =0;
#define WIZCHIP_SPI_MISO      TRISFbits.TRISF2 =1;
#define Wizchip_CS            LATBbits.LATB4
#define Wizchip_RS            LATCbits.LATC14
#define Wizchip_CS_Direction  TRISBbits.TRISB4
#define Wizchip_RS_Direction  TRISCbits.TRISC14

//------------------------------------------**--------------------------------------
                                         /*UART*/
//------------------------------------------**--------------------------------------

void Delay_x_mS( long N_mS ){

		long	Loop_mS,Del_1mS;
		
		for ( Loop_mS = 0 ; Loop_mS < N_mS ; Loop_mS++ ) 
		{
			for (Del_1mS = 0 ; Del_1mS < 60000 ; Del_1mS ++ );
					
		}
    }

void UART_PutChar(char c)
{
  while (U2STAbits.UTXBF);        // Wait for space in UART2 Tx buffer
  U2TXREG = c;                    // Write character to UART2
}

int    write(int handle, void *buffer, unsigned int len)
{
  int i;
   switch (handle)
  {
      case 0:        // handle 0 corresponds to stdout
      case 1:        // handle 1 corresponds to stdin
      case 2:        // handle 2 corresponds to stderr
      default:
          for (i=0; i<len; i++)
              UART_PutChar(*(char*)buffer++);
  }
  return(len);
}
void UART_init(void){
 U2BRG = 194;     // 9600Baud for 16MIP (See FRM Tables) 
//                // Change U1BRG to suit your clock frequency 
 U2MODE=0x8000; // Enable, 8data, no parity, 1 stop    
 U2STA =0x8400; // Enable TX       
 
 _TRISF5 = 0;           //Look for the exact TX/RX pins of your pic device
 _TRISF4 = 1;           //TX pin must be set as output port and RX input
 _LATF4 = 0;
 _LATF5 = 0;
}

//------------------------------------------**--------------------------------------
                                         /*SPI*/
//------------------------------------------**--------------------------------------

void InitSPI (void){
    printf("\r\n SPI Connect Successfully \r\n");
    unsigned int SPICONValue;
    unsigned int SPISTATValue;
    TRISFbits.TRISF6 =0;    //Output RF6/SCK1
    TRISFbits.TRISF2 =1;    //Input RF2/SDI1
    TRISFbits.TRISF3 =0;    //Output RF3/SDO1
    ADPCFGbits.PCFG2 =1;    //Pin RB2 analog in
    LATBbits.LATB2 = 1;     
    TRISBbits.TRISB2 = 0;   // Output RB2 (CS  RB2)
    SPICONValue = FRAME_ENABLE_OFF&          //FRMEN: 0= Framed SPI support disabled
                  FRAME_SYNC_OUTPUT&         //SPIIFSD: 0= Frame sync pulse output(master)
                  ENABLE_SDO_PIN&            //DISSDO: 0= SDOx pin is controlled by the module
                  SPI_MODE16_OFF&            //MODE16: 0= Communication is byte-wide (8 bits)
                  SPI_SMP_OFF&               //SMP: 0= Input data sample at middle of data output time
                  SPI_CKE_OFF&               //CKE: 0= Serial output data changes on transition from Idle clk to active clk state
                  SLAVE_ENABLE_OFF&          //SSEN: 0= SS pin not use by module. Pin controlled by port function   
                  CLK_POL_ACTIVE_HIGH&       //CKP: 1= Idle state for clock is high level. Active state is low level
                  MASTER_ENABLE_ON&          //MSTEN: 1= Master mode
                  SEC_PRESCAL_3_1&           //SPRE<2:0>: Secondary Prescale 3:1
                  PRI_PRESCAL_1_1;           //PPRE<1:0>: Primary Prescale 1:1
    SPISTATValue = SPI_ENABLE&               //SPIEN: 1= Enables module and configures SCKx, SDOx, SDIx and SSx as serial port pin
                   SPI_IDLE_CON&             //SPISIDL: 0= Continue module operation in Idle mode 
                   SPI_RX_OVFLOW_CLR;        //SPIROV: 0= No overflow has occurred. Clear receive overflow bit
    
    OpenSPI1(SPICONValue,SPISTATValue);
}

void SPI_Wri(uint8_t data){
 while(SPI1STATbits.SPITBF);
 WriteSPI1(data);

}

uint8_t SPI_Re(){
    uint8_t tmp = 0;
    while(!DataRdySPI1());
    tmp = ReadSPI1();
    return tmp;
}

//------------------------------------------**--------------------------------------
                                         /*W5500*/
//------------------------------------------**--------------------------------------

void WIZCHIP_Se(void)
{
    Wizchip_CS = 0;
}

// brief Call back function for WIZCHIP deselect.
void WIZCHIP_De(void)
{
    Wizchip_CS = 1;
}

void W5500_init(){
    
    uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
    Wizchip_CS_Direction = 0;
    Wizchip_RS_Direction = 0;
    
    Wizchip_RS = 0;
    Delay_x_mS(1);
    Wizchip_RS = 1;
    Delay_x_mS(1);
    
    wizchip_init(memsize,memsize);
//    ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
//    ctlwizchip(CW_SET_PHYCONF, (void*) &phyConf);
    
}
//------------------------------------------**--------------------------------------
                                         /*DHCP SETUP*/
//------------------------------------------**--------------------------------------

#define DHCP_HOSTNAME_MAX_LEN 32
#define IP_LEN 4
#define ETH_LEN 6

 char hostname[DHCP_HOSTNAME_MAX_LEN] = "dspic30f4011-w5500";   // Last two characters will be filled by last 2 MAC digits ;
 uint8_t dhcpState;
 uint8_t mymac[ETH_LEN];  ///< MAC address
 uint8_t myip[IP_LEN];    ///< IP address
 bool using_dhcp;   ///< True if using DHCP

    // EtherCard.cpp
enum {
    DHCP_STATE_INIT,
    DHCP_STATE_SELECTING,
    DHCP_STATE_REQUESTING,
    DHCP_STATE_BOUND,
    DHCP_STATE_RENEWING,
};

char toAsciiHex(uint8_t b) {
    char c = b & 0x0f;
    c += (c <= 9) ? '0' : 'A'-10;
    return c;
}

bool dhcpSetup (const char *hname, bool fromRam) {
    // Use during setup, as this discards all incoming requests until it returns.
    // That shouldn't be a problem, because we don't have an IPaddress yet.
    // Will try 60 secs to obtain DHCP-lease.

    if(hname != NULL) {
        if(fromRam) {
            strncpy(hostname, hname, DHCP_HOSTNAME_MAX_LEN);
        }
    }
    else {
        // Set a unique hostname, use Arduino-?? with last octet of mac address
        hostname[strlen(hostname) - 2] = toAsciiHex(mymac[5] >> 4);   // Appends mac to last 2 digits of the hostname
        hostname[strlen(hostname) - 1] = toAsciiHex(mymac[5]);   // Even if it's smaller than the maximum <thus, strlen(hostname)>
    }
    return dhcpState == DHCP_STATE_BOUND ;
}