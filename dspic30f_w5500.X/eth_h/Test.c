
 
#include <p30F4011.h>
#include "Wizchip_Conf.h"
#include "w5500.h"
#include "socket.h"
#include <spi.h>


// Socket & Port number definition
#define SOCK_ID_TCP       0
#define SOCK_ID_UDP       1
#define PORT_TCP          5000
#define PORT_UDP          10001

#define DATA_BUF_SIZE     1024
uint8_t gDATABUF[DATA_BUF_SIZE];
#define  Wizchip_CS       LATBbits.LATB4   
#define Wizchip_CS_Direction TRISBbits.TRISB4
#define Wizchip_Rst           LATCbits.LATC14
#define Wizchip_Rst_Direction  TRISCbits.TRISC14

///////////////////////////////////////////////////////////////////////////////

volatile wiz_NetInfo gWIZNETINFO =
{
  {0x00, 0x14, 0xA3, 0x72, 0x17, 0x3f},    // Source Mac Address
  {10, 1,  1, 21},                      // Source IP Address
  {255, 255, 254, 0},                      // Subnet Mask
  {192, 168,  0, 1},                       // Gateway IP Address
  {192, 168,  14, 99},                      // DNS server IP Address
  NETINFO_STATIC
 };

volatile wiz_PhyConf phyConf =
{
  PHY_CONFBY_HW,       // PHY_CONFBY_SW
  PHY_MODE_MANUAL,     // PHY_MODE_AUTONEGO
  PHY_SPEED_10,        // PHY_SPEED_100
  PHY_DUPLEX_FULL,     // PHY_DUPLEX_HALF
};

wiz_NetInfo pnetinfo;

////////////////////////////////////////////////////////////////////////////////
void InitSPI (void);
void CB_ChipSelect(void);
void CB_ChipDeselect(void);
void SPIWrite(unsigned int data);
unsigned int SPIRead();
void TCP_Server(void);
void UDP_Server(void);
static void W5500_Init(void);
void Delay_x_mS( int N_mS );


////////////////////////////////////////////////////////////////////////////////
void InitSPI (void){
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
                  PRI_PRESCAL_1_1;           //PPRE<1:0>: Primary Presacle 1:1
    SPISTATValue = SPI_ENABLE&               //SPIEN: 1= Enables module and configures SCKx, SDOx, SDIx and SSx as serial port pin
                   SPI_IDLE_CON&             //SPISIDL: 0= Continue module operation in Idle mode 
                   SPI_RX_OVFLOW_CLR;        //SPIROV: 0= No overflow has occurred. Clear receive overflow bit
    
    OpenSPI1(SPICONValue,SPISTATValue);
}

// brief Call back function for WIZCHIP select.
void CB_ChipSelect(void)
{
    Wizchip_CS = 0;
}

// brief Call back function for WIZCHIP deselect.
void CB_ChipDeselect(void)
{
    Wizchip_CS = 1;
}

void SPIWrite(unsigned int data){
 while(SPI1STATbits.SPITBF);
 WriteSPI1(data);
}

// brief Callback function to read byte using SPI.
unsigned int SPIRead(){
    uint8_t tmp = 0;
    WriteSPI1(0);
    while(!DataRdySPI1());
    tmp = ReadSPI1();
    return tmp;
    
}
// brief Handle TCP socket state.
void TCP_Server(void)
{
    int32_t ret;
    uint16_t size = 0, sentsize = 0;

    // Get status of socket
    switch(getSn_SR(SOCK_ID_TCP))
    {
        // Connection established
        case SOCK_ESTABLISHED :
        {
            // Check interrupt: connection with peer is successful
            if(getSn_IR(SOCK_ID_TCP) & Sn_IR_CON)
            {
                // Clear corresponding bit
                setSn_IR(SOCK_ID_TCP,Sn_IR_CON);
            }

            // Get received data size
            if((size = getSn_RX_RSR(SOCK_ID_TCP)) > 0)
            {
                // Cut data to size of data buffer
                if(size > DATA_BUF_SIZE)
                {
                    size = DATA_BUF_SIZE;
                }

                // Get received data
                ret = recv(SOCK_ID_TCP, gDATABUF, size);

                // Check for error
                if(ret <= 0)
                {
                    return;
                }

                // Send echo to remote
                sentsize = 0;
                while(size != sentsize)
                {
                    ret = send(SOCK_ID_TCP, gDATABUF + sentsize, size - sentsize);
                    
                    // Check if remote close socket
                    if(ret < 0)
                    {
                        close(SOCK_ID_TCP);
                        return;
                    }

                    // Update number of sent bytes
                    sentsize += ret;
                }
            }
            break;
        }

        // Socket received the disconnect-request (FIN packet) from the connected peer
        case SOCK_CLOSE_WAIT :
        {
            // Disconnect socket
            if((ret = disconnect(SOCK_ID_TCP)) != SOCK_OK)
            {
                return;
            }

            break;
        }

        // Socket is opened with TCP mode
        case SOCK_INIT :
        {
            // Listen to connection request
            if( (ret = listen(SOCK_ID_TCP)) != SOCK_OK)
            {
                return;
            }

            break;
        }

        // Socket is released
        case SOCK_CLOSED:
        {
            // Open TCP socket
            if((ret = socket(SOCK_ID_TCP, Sn_MR_TCP, PORT_TCP, 0x00)) != SOCK_ID_TCP)
            {
                return;
            }

           break;
        }

        default:
        {
            break;
        }
    }
}

// brief Handle UDP socket state.
void UDP_Server(void)
{
    int32_t  ret;
    uint16_t size, sentsize;
    uint8_t  destip[4];
    uint16_t destport;

    // Get status of socket
    switch(getSn_SR(SOCK_ID_UDP))
    {
        // Socket is opened in UDP mode
        case SOCK_UDP:
        {
            // Get received data size
            if((size = getSn_RX_RSR(SOCK_ID_UDP)) > 0)
            {
                // Cut data to size of data buffer
                if(size > DATA_BUF_SIZE)
                {
                    size = DATA_BUF_SIZE;
                }

                // Get received data
                ret = recvfrom(SOCK_ID_UDP, gDATABUF, size, destip, (uint16_t*)&destport);

                // Check for error
                if(ret <= 0)
                {
                    return;
                }

                // Send echo to remote
                size = (uint16_t) ret;
                sentsize = 0;
                while(sentsize != size)
                {
                    ret = sendto(SOCK_ID_UDP, gDATABUF + sentsize, size - sentsize, destip, destport);
                    if(ret < 0)
                    {
                        return;
                    }
                    // Update number of sent bytes
                    sentsize += ret;
                }
            }
            break;
        }

        // Socket is not opened
        case SOCK_CLOSED:
        {
            // Open UDP socket
            if((ret=socket(SOCK_ID_UDP, Sn_MR_UDP, PORT_UDP, 0x00)) != SOCK_ID_UDP)
            {
                return;
            }

            break;
        }

        default :
        {
           break;
        }
    }
}

// brief Initialize modules
static void W5500_Init(void)
{
    // Set Tx and Rx buffer size to 2KB
    uint8_t buffsize[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };

    Wizchip_Rst_Direction = 0;                           // Set Rst pin to be output
    Wizchip_CS_Direction = 0;                         // Set CS pin to be output
    
    CB_ChipDeselect();                                                          // Deselect module

    // Reset module
    Wizchip_Rst = 0;
    Delay_x_mS(1);
    Wizchip_Rst = 1;
    Delay_x_mS(1);

  //  SPI1_Init_Advanced(_SPI_MASTER_OSC_DIV4, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_LOW, _SPI_LOW_2_HIGH);

    // Wizchip initialize
    //wizchip_init(buffsize, buffsize, 0, 0, CB_ChipSelect, CB_ChipDeselect, 0, 0, SPIRead, SPIWrite);
    wizchip_init(buffsize, buffsize);
    
    // Wizchip netconf
    ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
    ctlwizchip(CW_SET_PHYCONF, (void*) &phyConf);
}

void Delay_x_mS( int N_mS ) 
{
		int	Loop_mS,Del_1mS;
		
		for ( Loop_mS = 0 ; Loop_mS < N_mS ; Loop_mS++ ) 
		{
			for (Del_1mS = 0 ; Del_1mS < 324 ; Del_1mS ++ );
					
		} 
}

////////////////////////////////////////////////////////////////////////////////
int main(void) 
{
    ADPCFG = 0;      // Set PORT B as digital
    InitSPI();
    W5500_Init();
    wizchip_getnetinfo(&pnetinfo);

    while(1)
    {
        // TCP Server
        TCP_Server();
        // UDP Server
        UDP_Server();
    }
}
  
