#include <xc.h>

#include "spi.h"

static void pull_ss_active(){LATAbits.LA5 = 0;};
static void pull_ss_inactive(){LATAbits.LA5 = 1;};

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
    //SPI1CON1bits.SSP = 1; //active-low SS
    SPI1CON1bits.SDIP = 0; //active-high SDI 
    SPI1CON1bits.SDOP = 0; //active-high SDO
    
    //SPI1CON2bits.SSET = 0; // SS is driven to active state while the transmit counter is not zero
    
    SPI1CON2bits.TXR = 0; // Initially disable TX
    SPI1CON2bits.RXR = 0; // Initially disable RX
    
    SPI1INTE = 0x00; // Disable all SPI interrupts. 
    
    //Critical todo: need to modify TRIS registers to configure below as input or output appropriately???
    
    //PPS remap RC5 as SDO pin, RC3 as SCK pin, RC4 as SDI pin, RA5 as SS pin
    TRISCbits.TRISC5 = 0; // RC5 is an output pin
    RC5PPS &= ~(0b111111);
    RC5PPS |= 0b011111;
    
    TRISCbits.TRISC3 = 0; // RC3 is an output pin
    RC3PPS &= ~(0b000000);
    RC5PPS |= 0b011110;
    
    TRISCbits.TRISC4 = 1; // RC4 is an input pin
    SPI1SDIPPS &= ~(0b111111);
    SPI1SDIPPS|= 0b1 0100;
    
    TRISAbits.TRISA5 = 0; // RC4 is an output pin
    RA5PPS &= 0b000000; //Set SS pin to manual control via LATA5 register
    
    // Set the enable bit after configuration is complete
    SPI1CON0bits.EN = 1; 
    
}

void spi1_send(uint8_t data)
{
    // ToDo: error/timeout handling. Then get rid of inefficient/unnecessary "Wait until TX buffer is empty" poll.
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // ToDO: maybe incorportate SRMTIF & TCZIF to determine end of transmission (interrupt stuff). Otherwise, just rely on polling of SPI1CON2bits.BUSY == 1
    
    //while (SPI1CON2bits.BUSY){}; // Wait until no data exchange in progress
    SPI1STATUSbits.TXWE = 0; // Clear any past TX buffer write error
    pull_ss_active();
    SPI1CON2bits.TXR = 1; // Enable TX
    
    SPI1TCNT = 1; // set the transfer count to 1
    //while (!SPI1STATUS.TXBE){}; // Wait until TX buffer is empty. !! this line is inefficient, we shouldnt have to wait
    SPI1TXB = data; // load the TX buffer
    
    unsigned int timeout = 0;
    while (SPI1CON2bits.BUSY && timeout < SPI_POLL_TIMEOUT){ timeout++; }; // Wait until no data exchange in progress
    SPI1CON2bits.TXR = 0; // Disable TX
    pull_ss_inactive();
    
}

void spi1_send_buffer(uint8_t *data, uint16_t data_len, bool force_ss_inactive)
{
    // ToDo: error/timeout handling. Then get rid of inefficient/unnecessary "Wait until TX buffer is empty" poll.
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // ToDO: maybe incorporate SRMTIF & TCZIF to determine end of transmission (interrupt stuff). Otherwise, just rely on polling of SPI1CON2bits.BUSY == 1
    
    //while (SPI1CON2bits.BUSY){}; // Wait until no data exchange in progress
    SPI1STATUSbits.TXWE = 0; // Clear any past TX buffer write error
    if(force_ss_inactive){
        pull_ss_inactive();
    } else {
        pull_ss_active();
    }
    SPI1CON2bits.TXR = 1; // Enable TX
    
    SPI1TCNT = data_len; // set the transfer count
    
    
    unsigned int txwe_count = 0;
    while (data_len && txwe_count < SPI_MAX_TXWE_CNT) {
        //while (SPI1STATUS.TXBE == 0){}; // Wait until TX buffer is empty. !! this line is inefficient, we shouldn't have to wait
        
        SPI1TXB = *data; // load the TX buffer
        
        // if loading the buffer fails due to full FIFO, clear the error and try again in the next iteration 
        if(!SPI1STATUSbits.TXWE) 
        {
            txwe_count = 0;
            data++; 
            data_len--;
        }
        else 
        {
           txwe_count++;
           SPI1STATUSbits.TXWE = 0; 
        }
    }
    
    unsigned int timeout = 0;
    while (SPI1CON2bits.BUSY && timeout < SPI_POLL_TIMEOUT){ timeout++; }; // Wait until no data exchange in progress
    SPI1CON2bits.TXR = 0; // Disable TX
    pull_ss_inactive();
}

uint8_t spi1_read(void)
{
    // ToDo: error/timeout handling improvements
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // Todo: consider interrupts involving spixrif, SRMTIF, and tczif
    // Todo: consider CLRBF necessity
    
    //while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1STATUSbits.CLRBF = 1; //Clear the RX (and TX) buffers
    SPI1STATUSbits.RXRE = 0; // Clear any past RX buffer read error
    pull_ss_active();
    SPI1CON2bits.RXR = 1; // Enable RX
    
    SPI1TCNT = 1; // set the transfer count to 1
    
    unsigned int timeout = 0;
    while (SPI1CON2bits.BUSY && timeout < SPI_POLL_TIMEOUT){ timeout++; }; // Wait until no data exchange in progress
    SPI1CON2bits.RXR = 0; // Disable RX
    pull_ss_inactive();
    
    return SPI1RXB; // Value from RX buffer
}


void spi1_read_buffer(uint8_t *data, uint16_t data_len)
{
    // ToDo: error/timeout handling improvements
    // Todo: need to consider full vs half duplex. Currently half duplex.
    // Todo: consider interrupts involving spixrif, SRMTIF, and tczif
    // Todo: consider CLRBF necessity
    
    //while (SPI1CON2bits.BUSY == 1){}; // Wait until no data exchange in progress
    SPI1STATUSbits.CLRBF = 1; // Clear the RX (and TX) buffers
    SPI1STATUSbits.RXRE = 0; // Clear any past RX buffer read error
    pull_ss_active();
    SPI1CON2bits.RXR = 1; // Enable RX
    
    SPI1TCNT = data_len; // set the transfer count
    
    unsigned int rxre_count = 0;
    while (data_len && rxre_count < SPI_MAX_RXRE_CNT) {
        *data = SPI1RXB; // Value from RX buffer
        // If reading the buffer fails due to empty FIFO, clear the error and try again in the next iteration 
        if(!SPI1STATUSbits.RXRE) 
        {
            rxre_count = 0;
            data++; 
            data_len--;
        }
        else 
        {
           rxre_count++;
           SPI1STATUSbits.RXRE = 0; 
        }
    }
    
    unsigned int timeout = 0;
    while (SPI1CON2bits.BUSY && timeout < SPI_POLL_TIMEOUT){ timeout++; }; // Wait until no data exchange in progress
    SPI1CON2bits.RXR = 0; // Disable RX
    pull_ss_inactive();
}

