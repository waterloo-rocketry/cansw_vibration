#include <xc.h>

void spi1_init (uint8_t baud_prescaler) {
/*Configure and enable SPI module 1*/
   
    // Clear enable before configuring other bits
    SPI1CON0bits.EN = 0; 
    
    // Set the baud rate
    SPI1CLKbits.CLKSEL = 0x04; //CLK clock reference
    SPI1BAUDbits.BAUD = baud_prescaler; // SCK toggle frequency = clock reference/(2*(baud_prescaler + 1))
            
    // Configure SPI Master mode settings
    SPI1CON0bits.LSBF = 0; //msb first as in traditional spi
    SPI1CON0bits.MST = 1; // set as master mode
    SPI1CON0bits.BMODE = 0; // send 8 bit packets 
    
    SPI1CON1bits.SMP = 0; //sample bits in the middle of a clock cycle
    SPI1CON1bits.CKE = 0; //use rising edge of clock
    SPI1CON1bits.CKP = 0; //active-high clock
    SPI1CON1bits.SSP = 0; //active-high SS
    SPI1CON1bits.SDIP = 0; //active-high SDI 
    SPI1CON1bits.SDOP = 0; //active-high SDO
    
    SPI1CON2bits.SSET = 0; // SS is driven to active state while the transmit counter is not zero
    
    //PPS remap RC5 as SDO pin, RC3 as SCK pin, RC4 as SDI pin, RA5 as SS pin
    RC5PPS &= ~(0b111111);
    RC5PPS |= 0b011111;
    
    SPI1SCKPPS &= ~(0b111111);
    SPI1SCKPPS |= 0b1 0011;
    
    SPI1SDIPPS &= ~(0b111111);
    SPI1SDIPPS|= 0b1 0100;
    
    SPI1SSPPS &= ~(0b111111);
    SPI1SSPPS |= 0b0 0101;
    
    // Set the enable bit after configuration is complete
    SPI1CON0bits.EN = 1; 
    
}

bool spi1_write();

static void spi1_send(uint8_t data);

static void spi1_send_buffer(uint8_t *data, uint16_t data_len);

static void spi1_read_buffer(uint8_t *data, uint16_t data_len);




#include "spi.h"



