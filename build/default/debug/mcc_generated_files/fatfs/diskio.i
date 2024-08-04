# 1 "mcc_generated_files/fatfs/diskio.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 288 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\language_support.h" 1 3
# 2 "<built-in>" 2
# 1 "mcc_generated_files/fatfs/diskio.c" 2
# 10 "mcc_generated_files/fatfs/diskio.c"
# 1 "mcc_generated_files/fatfs/diskio.h" 1
# 12 "mcc_generated_files/fatfs/diskio.h"
# 1 "mcc_generated_files/fatfs/integer.h" 1
# 16 "mcc_generated_files/fatfs/integer.h"
typedef int INT;
typedef unsigned int UINT;


typedef unsigned char BYTE;


typedef short SHORT;
typedef unsigned short WORD;
typedef unsigned short WCHAR;


typedef long LONG;
typedef unsigned long DWORD;


typedef unsigned long long QWORD;
# 12 "mcc_generated_files/fatfs/diskio.h" 2




typedef BYTE DSTATUS;


typedef enum {
 RES_OK = 0,
 RES_ERROR,
 RES_WRPRT,
 RES_NOTRDY,
 RES_PARERR
} DRESULT;






DSTATUS disk_initialize (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);
# 10 "mcc_generated_files/fatfs/diskio.c" 2

# 1 "mcc_generated_files/fatfs/../../mcc_generated_files/sd_spi/sd_spi.h" 1
# 25 "mcc_generated_files/fatfs/../../mcc_generated_files/sd_spi/sd_spi.h"
# 1 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\stdint.h" 1 3



# 1 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\musl_xc8.h" 1 3
# 5 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\stdint.h" 2 3
# 26 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\stdint.h" 3
# 1 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\bits/alltypes.h" 1 3
# 133 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\bits/alltypes.h" 3
typedef unsigned __int24 uintptr_t;
# 148 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\bits/alltypes.h" 3
typedef __int24 intptr_t;
# 164 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\bits/alltypes.h" 3
typedef signed char int8_t;




typedef short int16_t;




typedef __int24 int24_t;




typedef long int32_t;





typedef long long int64_t;
# 194 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\bits/alltypes.h" 3
typedef long long intmax_t;





typedef unsigned char uint8_t;




typedef unsigned short uint16_t;




typedef __uint24 uint24_t;




typedef unsigned long uint32_t;





typedef unsigned long long uint64_t;
# 235 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\bits/alltypes.h" 3
typedef unsigned long long uintmax_t;
# 27 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\stdint.h" 2 3

typedef int8_t int_fast8_t;

typedef int64_t int_fast64_t;


typedef int8_t int_least8_t;
typedef int16_t int_least16_t;

typedef int24_t int_least24_t;
typedef int24_t int_fast24_t;

typedef int32_t int_least32_t;

typedef int64_t int_least64_t;


typedef uint8_t uint_fast8_t;

typedef uint64_t uint_fast64_t;


typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;

typedef uint24_t uint_least24_t;
typedef uint24_t uint_fast24_t;

typedef uint32_t uint_least32_t;

typedef uint64_t uint_least64_t;
# 148 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\stdint.h" 3
# 1 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\bits/stdint.h" 1 3
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
# 149 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\stdint.h" 2 3
# 25 "mcc_generated_files/fatfs/../../mcc_generated_files/sd_spi/sd_spi.h" 2

# 1 "C:\\Program Files\\Microchip\\xc8\\v2.46\\pic\\include\\c99\\stdbool.h" 1 3
# 26 "mcc_generated_files/fatfs/../../mcc_generated_files/sd_spi/sd_spi.h" 2
# 86 "mcc_generated_files/fatfs/../../mcc_generated_files/sd_spi/sd_spi.h"
_Bool SD_SPI_IsMediaInitialized(void);
_Bool SD_SPI_MediaInitialize(void);
_Bool SD_SPI_IsMediaPresent(void);
_Bool SD_SPI_IsWriteProtected(void);
uint16_t SD_SPI_GetSectorSize(void);
uint32_t SD_SPI_GetSectorCount(void);
# 123 "mcc_generated_files/fatfs/../../mcc_generated_files/sd_spi/sd_spi.h"
_Bool SD_SPI_SectorRead(uint32_t sector_address, uint8_t* buffer, uint16_t sector_count);
# 154 "mcc_generated_files/fatfs/../../mcc_generated_files/sd_spi/sd_spi.h"
_Bool SD_SPI_SectorWrite(uint32_t sector_address, const uint8_t* buffer, uint16_t sector_count);
# 11 "mcc_generated_files/fatfs/diskio.c" 2




enum DRIVER_LIST{
    DRVA = 0,
};





DSTATUS disk_status (
    BYTE pdrv
)
{
    DSTATUS stat = 0x01;

    switch (pdrv) {

        case DRVA:
            if ( SD_SPI_IsMediaPresent() == 0)
            {
               stat = 0x02;
            }
            else if ( SD_SPI_IsMediaInitialized() == 1)
            {
                stat &= ~0x01;
            }

            if ( SD_SPI_IsWriteProtected() == 1)
            {
                stat |= 0x04;
            }

            break;

        default:
            break;
    }
    return stat;
}







DSTATUS disk_initialize (
    BYTE pdrv
)
{
    DSTATUS stat = 0x01;

    switch (pdrv) {
        case DRVA :
            if(SD_SPI_MediaInitialize() == 1)
            {
                stat = RES_OK;
            }
            else
            {
                stat = RES_ERROR;
            }
            break;
        default:
            break;
    }

    return stat;
}







DRESULT disk_read (
    BYTE pdrv,
    BYTE *buff,
    DWORD sector,
    UINT count
)
{
    DRESULT res = RES_PARERR;

    switch (pdrv) {
        case DRVA :
            if(SD_SPI_SectorRead(sector, buff, count) == 1)
            {
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;

        default:
            break;
    }

    return res;
}







DRESULT disk_write (
    BYTE pdrv,
    const BYTE *buff,
    DWORD sector,
    UINT count
)
{
    DRESULT res = RES_PARERR;

    switch (pdrv) {
        case DRVA :
            if(SD_SPI_SectorWrite(sector, buff, count) == 1)
            {
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;

        default:
            break;
    }

    return res;
}







DRESULT disk_ioctl (
    BYTE pdrv,
    BYTE cmd,
    void *buff
)
{
    DRESULT res = RES_OK;

    switch (pdrv) {
        case DRVA :
            return res;

        default:
            break;
    }

    return RES_PARERR;
}
