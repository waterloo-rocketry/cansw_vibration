/**
 * Generated Driver File
 * 
 * @file pins.c
 * 
 * @ingroup  pinsdriver
 * 
 * @brief This is generated driver implementation for pins. 
 *        This file provides implementations for pin APIs for all pins selected in the GUI.
 *
 * @version Driver Version 3.1.0
*/

/*
� [2024] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

#include "../pins.h"


void PIN_MANAGER_Initialize(void)
{
	/**
    LATx registers
    */
    LATA = 0x00;
    LATB = 0x06;
    LATC = 0x00;

    /**
    TRISx registers
    */
    TRISA = 0xDF;
    TRISB = 0xF9;
    TRISC = 0xD7;

    /**
    ANSELx registers
    */
    ANSELC = 0xC7;
    ANSELB = 0xF9;
    ANSELA = 0xDF;

    /**
    WPUx registers
    */
    WPUE = 0x00;
    WPUB = 0x00;
    WPUA = 0x00;
    WPUC = 0x00;

    /**
    ODx registers
    */
    ODCONA = 0x00;
    ODCONB = 0x06;
    ODCONC = 0x00;

    /**
    SLRCONx registers
    */
    SLRCONA = 0xFF;
    SLRCONB = 0xFF;
    SLRCONC = 0xFF;

    /**
    INLVLx registers
    */
    INLVLA = 0xFF;
    INLVLB = 0xFF;
    INLVLC = 0xFF;
    INLVLE = 0x08;

   /**
    RxyI2C | RxyFEAT registers   
    */
    /**
    PPS registers
    */
    I2C1SCLPPS = 0x9;  //RB1->I2C1:SCL1;
    RB1PPS = 0x21;  //RB1->I2C1:SCL1;
    I2C1SDAPPS = 0xA;  //RB2->I2C1:SDA1;
    RB2PPS = 0x22;  //RB2->I2C1:SDA1;

    SPI1SCKPPS = 0x13;   //RC3->SPI1:SCK1;    
    RC3PPS = 0x1E;   //RC3->SPI1:SCK1;    
    RC5PPS = 0x1F;   //RC5->SPI1:SDO1;    
    SPI1SDIPPS = 0x14;   //RC4->SPI1:SDI1; 

   /**
    IOCx registers 
    */
    IOCAP = 0x0;
    IOCAN = 0x0;
    IOCAF = 0x0;
    IOCBP = 0x0;
    IOCBN = 0x0;
    IOCBF = 0x0;
    IOCCP = 0x0;
    IOCCN = 0x0;
    IOCCF = 0x0;
    IOCEP = 0x0;
    IOCEN = 0x0;
    IOCEF = 0x0;


}
  
void PIN_MANAGER_IOC(void)
{
}
/**
 End of File
*/
