/*
 * File:   main.c
 * Author: DELL
 *
 * Created on May 14, 2020, 1:44 PM
 */


#include "xc.h"
#include "new.h"

int main(void)
{
   uint8_t tmp;
   uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
   uint8_t my_dhcp_retry = 0;
   uint32_t led_msTick = 1000;
   
   TRISDbits.TRISD3 =0;
   
   InitSPI();

   // Chip selection call back 
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(WIZCHIP_Select, WIZCHIP_Deselect);
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(WIZCHIP_Select, WIZCHIP_Select);  // CS must be tried with LOW.
#else
   #if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
      #error "Unknown _WIZCHIP_IO_MODE_"
   #else
      reg_wizchip_cs_cbfunc(WIZCHIP_Select, WIZCHIP_Deselect);
   #endif
#endif
    // SPI Read & Write callback function 
    reg_wizchip_spi_cbfunc(SPI_Read, SPI_Write);
   
    // wizchip initialize
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
    {
        printf("WZICHIP Initialized fail. \r\n");
        while(1);
    }

    // PHY link status check 
    do
    {
       if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
           printf("Unknown PHY Link status. \r\n");
    }while(tmp == PHY_LINK_OFF);
    
     // must be set the default mac before DHCP started.
	setSHAR(gWIZNETINFO.mac);

	DHCP_init(SOCK_DHCP, gDATABUF);
	// if you want defiffent action instead default ip assign,update, conflict,
	// if cbfunc == 0, act as default.
    	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);

    prevTick = msTicks;
    
    // Main Loop 
    while(1)
    {
		switch(DHCP_run())
		{
			case DHCP_IP_ASSIGN:
			case DHCP_IP_CHANGED:
				// If this block empty, act with default_ip_assign & default_ip_update 
				//
				// This example calls my_ip_assign in the two case.
				//
				// Add to ...
				//
				break;
			case DHCP_IP_LEASED:
				//
				// TO DO YOUR NETWORK APPs.
				//
				break;
			case DHCP_FAILED:
				// ===== Example pseudo code =====  
				// The below code can be replaced your code or omitted.
				// if omitted, retry to process DHCP
				my_dhcp_retry++;
				if(my_dhcp_retry > MY_MAX_DHCP_RETRY)
				{
#ifdef _MAIN_DEBUG_
                    printf(">> DHCP %d Failed \r\n", my_dhcp_retry);
#endif              
					my_dhcp_retry = 0;
					DHCP_stop();      // if restart, recall DHCP_init()
					network_init();   // apply the default static network and print out netinfo to serial
				}
				break;
			default:
				break;
		}

    	// LED Toggle every 1sec 
    	if((msTicks - prevTick) > led_msTick)
    	{
    		tmp = ~tmp;
    		RD3 = ~RD3;
    		
    		prevTick = msTicks;
    	}
    } // end of Main loop
} // end of main()

