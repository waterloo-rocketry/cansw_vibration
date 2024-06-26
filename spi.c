#include <xc.h>

#include "spi.h"

void spi1_init (uint8_t baud_prescaler) {
/*Configure and enable SPI module 1*/
    
    //Todo: should we check SPI1CON2bits.BUSY before initialization?
    //Todo: consider whether to use SPI interrupt registers.
    
    // Clear enable before configuring other bits
    SPI1CON0bits.EN = 0; 
    //SPI1STATUSbits.CLRBF = 1;// clear the RX and TX FIFOs
    
    // Set the baud rate
    SPI1CLKbits.CLKSEL = 0x04; //CLK clock reference
    SPI1BAUDbits.BAUD = baud_prescaler; // SCK toggle frequency = clock reference/(2*(baud_prescaler + 1))
            
    // Configure SPI Master mode settings
    SPI1CON0bits.LSBF = 0; // MSB first as in traditional SPI
    SPI1CON0bits.MST = 1; // set as master mode
      
    SPI1CON0bits.BMODE = 0; //For BMODE = 0,  total bits sent is SPIxTWIDTH + (SPIxTCNT*8)  
    SPI1TWIDTHbits.TWIDTH = 0b000; 
    
    SPI1CON1bits.SMP = 0; //sample bits in the middle of a clock cycle
    SPI1CON1bits.CKE = 0; //use rising edge of clock
    SPI1CON1bits.CKP = 0; //active-high clock
    SPI1CON1bits.SSP = 1; //active-low SS
    SPI1CON1bits.SDIP = 0; //active-high SDI 
    SPI1CON1bits.SDOP = 0; //active-high SDO
    
    SPI1CON2bits.SSET = 0; // SS is driven to active state while the transmit counter is not zero
    
    SPI1CON2bits.TXR = 0; // Initially disable Tx
    SPI1CON2bits.RXR = 0; // Initially disable Rx
    
    SPI1INTE = 0x00; // Disable all SPI interrupts. 
    
    //PPS remap RC5 as SDO pin, RC3 as SCK pin, RC4 as SDI pin, RA5 as SS pin
    RC5PPS &= ~(0b111111);
    RC5PPS |= 0b011111;
    
    SPI1SCKPPS &= ~(0b111111);
    SPI1SCKPPS |= 0b1 0011;
    
    SPI1SDIPPS &= ~(0b111111);
    SPI1SDIPPS|= 0b1 0100;
    
    SPI1SSPPS &= ~(0b111111); 
    SPI1SSPPS |= 0b0 0101; // Todo: Remove this line if it interferes with auto SS control
    
    // Set the enable bit after configuration is complete
    SPI1CON0bits.EN = 1; 
    
}

void spi1_send(uint8_t data)
{
    // ToDo: error/timeout handling. Then get rid of inefficient/unnecessary "Wait until TX buffer is empty" poll.
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // ToDO: maybe incorportate SRMTIF & TCZIF to determine end of transmission (interrupt stuff). Otherwise, just rely on polling of SPI1CON2bits.BUSY == 1
    
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    
    SPI1CON2bits.TXR = 1; // Enable Tx
    
    SPI1TCNT = 1; // set the transfer count to 1
    while (SPI1STATUS.TXBE == 0) // Wait until TX buffer is empty. !! this line is inefficient, we shouldnt have to wait
    SPI1TXB = data; // load the TX buffer
    
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1CON2bits.TXR = 0; // Disable Tx
    
}

void spi1_send_buffer(uint8_t *data, uint16_t data_len)
{
    // ToDo: error/timeout handling. Then get rid of inefficient/unnecessary "Wait until TX buffer is empty" poll.
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // ToDO: maybe incorportate SRMTIF & TCZIF to determine end of transmission (interrupt stuff). Otherwise, just rely on polling of SPI1CON2bits.BUSY == 1
    
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1CON2bits.TXR = 1; // Enable Tx
    
    SPI1TCNT = data_len; // set the transfer count
    
    while (data_len) {
        while (SPI1STATUS.TXBE == 0) // Wait until TX buffer is empty. !! this line is inefficient, we shouldnt have to wait
        SPI1TXB = *data; // load the TX buffer
        
        
        data++; 
        data_len--;
    }
    
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1CON2bits.TXR = 0; // Disable Tx

}

uint8_t spi1_read(void)
{
    // ToDo: error/timeout handling. Then get rid of inefficient/unnecessary "Wait until Rx buffer is full" poll.
    // Todo: need to use spixtct and SPIxTWIDTH registers, and TXR/RXR for this operation?
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // Todo: consider interrupts involving spixrif, SRMTIF, and tczif
    // Todo: consider CLRBF necessity
    
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1STATUSbits.CLRBF = 1; //Clear the Rx (and Tx) buffers
    SPI1CON2bits.RXR = 1; // Enable Rx
    
    SPI1TCNT = 1; // set the transfer count to 1
    
    //while (SPI1STATUS.RXBF == 0){}; // Wait until RX buffer is full. !! This code line is wrong since we dont expect/neeed the buffer to become full
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1CON2bits.RXR = 0; // Disable Rx
    
    //Critical Todo: we have to make sure the RX buffer is not empty before reading. How?
    
    return SPI1RXB; // Value from Rx buffer
}


void spi1_read_buffer(uint8_t *data, uint16_t data_len)
{
    // ToDo: error/timeout handling. Then get rid of inefficient/unnecessary "Wait until Rx buffer is full" poll.
    // Todo: need to use spixtct and SPIxTWIDTH registers, and TXR/RXR for this operation?
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // Todo: consider interrupts involving spixrif, SRMTIF, and tczif
    // Todo: consider CLRBF necessity
    
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1STATUSbits.CLRBF = 1; // Clear the Rx (and Tx) buffers
    SPI1CON2bits.RXR = 1; // Enable Rx
    
    SPI1TCNT = data_len; // set the transfer count
    
    while (data_len) {
        //while (SPI1STATUS.RXBF == 0){}; // Wait until RX buffer is full. !! This code line is wrong since we dont expect/neeed the buffer to become full
        
        //Todo: add handling when less than expected data is recieved.
        //Critical Todo: we have to make sure the RX buffer is not empty before reading. How?
        
        
        *data = SPI1RXB; // Value from Rx buffer
        data++;
        data_len--;
    }
    
    while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1CON2bits.RXR = 0; // Disable Rx
}

