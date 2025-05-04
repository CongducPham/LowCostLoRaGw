/* bcm2835.h
   
    C and C++ support for Broadcom BCM 2835 as used in Raspberry Pi
   
    Author: Mike McCauley
    Copyright (C) 2011-2013 Mike McCauley
    $Id: bcm2835.h,v 1.25 2019/07/22 23:04:13 mikem Exp $
 */
 
 /* Defines for BCM2835 */
 #ifndef BCM2835_H
 #define BCM2835_H
 
 #include <stdint.h>
 
 #define BCM2835_VERSION 10060 /* Version 1.60 */
 
 /* RPi 2 is ARM v7, and has DMB instruction for memory barriers.
    Older RPis are ARM v6 and don't, so a coprocessor instruction must be used instead.
    However, not all versions of gcc in all distros support the dmb assembler instruction even on conmpatible processors.
    This test is so any ARMv7 or higher processors with suitable GCC will use DMB.
 */
 #if __ARM_ARCH >= 7
 #define BCM2835_HAVE_DMB
 #endif
 
 #define HIGH2 0x1
 
 #define LOW2 0x0
 
 #ifndef MIN
 #define MIN(a, b) (a < b ? a : b)
 #endif
 
 #define BCM2835_CORE_CLK_HZ             250000000       
 #define BMC2835_RPI2_DT_FILENAME "/proc/device-tree/soc/ranges"
 
 #define BCM2835_PERI_BASE               0x20000000
 
 #define BCM2835_PERI_SIZE               0x01000000
 
 #define BCM2835_RPI2_PERI_BASE          0x3F000000
 
 #define BCM2835_RPI4_PERI_BASE          0xFE000000
 
 #define BCM2835_RPI4_PERI_SIZE          0x01800000
 
 #define BCM2835_ST_BASE                                 0x3000
 
 #define BCM2835_GPIO_PADS               0x100000
 
 #define BCM2835_CLOCK_BASE              0x101000
 
 #define BCM2835_GPIO_BASE               0x200000
 
 #define BCM2835_SPI0_BASE               0x204000
 
 #define BCM2835_BSC0_BASE                               0x205000
 
 #define BCM2835_GPIO_PWM                0x20C000
 
 #define BCM2835_AUX_BASE                                0x215000
 
 #define BCM2835_SPI1_BASE                               0x215080
 
 #define BCM2835_SPI2_BASE                               0x2150C0
 
 #define BCM2835_BSC1_BASE                               0x804000
 
 
 extern uint32_t *bcm2835_peripherals_base;
 extern uint32_t bcm2835_peripherals_size;
 
 extern uint32_t *bcm2835_peripherals;
 
 extern volatile uint32_t *bcm2835_st;
 
 extern volatile uint32_t *bcm2835_gpio;
 
 extern volatile uint32_t *bcm2835_pwm;
 
 extern volatile uint32_t *bcm2835_clk;
 
 extern volatile uint32_t *bcm2835_pads;
 
 extern volatile uint32_t *bcm2835_spi0;
 
 extern volatile uint32_t *bcm2835_bsc0;
 
 extern volatile uint32_t *bcm2835_bsc1;
 
 extern volatile uint32_t *bcm2835_aux;
 
 extern volatile uint32_t *bcm2835_spi1;
 
 
 typedef enum
 {
     BCM2835_REGBASE_ST   = 1, 
     BCM2835_REGBASE_GPIO = 2, 
     BCM2835_REGBASE_PWM  = 3, 
     BCM2835_REGBASE_CLK  = 4, 
     BCM2835_REGBASE_PADS = 5, 
     BCM2835_REGBASE_SPI0 = 6, 
     BCM2835_REGBASE_BSC0 = 7, 
     BCM2835_REGBASE_BSC1 = 8,  
         BCM2835_REGBASE_AUX  = 9,  
         BCM2835_REGBASE_SPI1 = 10  
 } bcm2835RegisterBase;
 
 #define BCM2835_PAGE_SIZE               (4*1024)
 
 #define BCM2835_BLOCK_SIZE              (4*1024)
 
 
 /* Defines for GPIO
    The BCM2835 has 54 GPIO pins.
    BCM2835 data sheet, Page 90 onwards.
 */
 #define BCM2835_GPFSEL0                      0x0000 
 #define BCM2835_GPFSEL1                      0x0004 
 #define BCM2835_GPFSEL2                      0x0008 
 #define BCM2835_GPFSEL3                      0x000c 
 #define BCM2835_GPFSEL4                      0x0010 
 #define BCM2835_GPFSEL5                      0x0014 
 #define BCM2835_GPSET0                       0x001c 
 #define BCM2835_GPSET1                       0x0020 
 #define BCM2835_GPCLR0                       0x0028 
 #define BCM2835_GPCLR1                       0x002c 
 #define BCM2835_GPLEV0                       0x0034 
 #define BCM2835_GPLEV1                       0x0038 
 #define BCM2835_GPEDS0                       0x0040 
 #define BCM2835_GPEDS1                       0x0044 
 #define BCM2835_GPREN0                       0x004c 
 #define BCM2835_GPREN1                       0x0050 
 #define BCM2835_GPFEN0                       0x0058 
 #define BCM2835_GPFEN1                       0x005c 
 #define BCM2835_GPHEN0                       0x0064 
 #define BCM2835_GPHEN1                       0x0068 
 #define BCM2835_GPLEN0                       0x0070 
 #define BCM2835_GPLEN1                       0x0074 
 #define BCM2835_GPAREN0                      0x007c 
 #define BCM2835_GPAREN1                      0x0080 
 #define BCM2835_GPAFEN0                      0x0088 
 #define BCM2835_GPAFEN1                      0x008c 
 #define BCM2835_GPPUD                        0x0094 
 #define BCM2835_GPPUDCLK0                    0x0098 
 #define BCM2835_GPPUDCLK1                    0x009c 
 /* 2711 has a different method for pin pull-up/down/enable  */
 #define BCM2835_GPPUPPDN0                    0x00e4 /* Pin pull-up/down for pins 15:0  */
 #define BCM2835_GPPUPPDN1                    0x00e8 /* Pin pull-up/down for pins 31:16 */
 #define BCM2835_GPPUPPDN2                    0x00ec /* Pin pull-up/down for pins 47:32 */
 #define BCM2835_GPPUPPDN3                    0x00f0 /* Pin pull-up/down for pins 57:48 */
 
 typedef enum
 {
     BCM2835_GPIO_FSEL_INPT  = 0x00,   
     BCM2835_GPIO_FSEL_OUTP  = 0x01,   
     BCM2835_GPIO_FSEL_ALT0  = 0x04,   
     BCM2835_GPIO_FSEL_ALT1  = 0x05,   
     BCM2835_GPIO_FSEL_ALT2  = 0x06,   
     BCM2835_GPIO_FSEL_ALT3  = 0x07,   
     BCM2835_GPIO_FSEL_ALT4  = 0x03,   
     BCM2835_GPIO_FSEL_ALT5  = 0x02,   
     BCM2835_GPIO_FSEL_MASK  = 0x07    
 } bcm2835FunctionSelect;
 
 typedef enum
 {
     BCM2835_GPIO_PUD_OFF     = 0x00,   
     BCM2835_GPIO_PUD_DOWN    = 0x01,   
     BCM2835_GPIO_PUD_UP      = 0x02    
 } bcm2835PUDControl;
 
 /* need a value for pud functions that can't work unless RPI 4 */
 #define BCM2835_GPIO_PUD_ERROR  0x08 
 
 #define BCM2835_PADS_GPIO_0_27               0x002c 
 #define BCM2835_PADS_GPIO_28_45              0x0030 
 #define BCM2835_PADS_GPIO_46_53              0x0034 
 #define BCM2835_PAD_PASSWRD                  (0x5A << 24)  
 #define BCM2835_PAD_SLEW_RATE_UNLIMITED      0x10 
 #define BCM2835_PAD_HYSTERESIS_ENABLED       0x08 
 #define BCM2835_PAD_DRIVE_2mA                0x00 
 #define BCM2835_PAD_DRIVE_4mA                0x01 
 #define BCM2835_PAD_DRIVE_6mA                0x02 
 #define BCM2835_PAD_DRIVE_8mA                0x03 
 #define BCM2835_PAD_DRIVE_10mA               0x04 
 #define BCM2835_PAD_DRIVE_12mA               0x05 
 #define BCM2835_PAD_DRIVE_14mA               0x06 
 #define BCM2835_PAD_DRIVE_16mA               0x07 
 typedef enum
 {
     BCM2835_PAD_GROUP_GPIO_0_27         = 0, 
     BCM2835_PAD_GROUP_GPIO_28_45        = 1, 
     BCM2835_PAD_GROUP_GPIO_46_53        = 2  
 } bcm2835PadGroup;
 
 typedef enum
 {
     RPI_GPIO_P1_03        =  0,  
     RPI_GPIO_P1_05        =  1,  
     RPI_GPIO_P1_07        =  4,  
     RPI_GPIO_P1_08        = 14,  
     RPI_GPIO_P1_10        = 15,  
     RPI_GPIO_P1_11        = 17,  
     RPI_GPIO_P1_12        = 18,  
     RPI_GPIO_P1_13        = 21,  
     RPI_GPIO_P1_15        = 22,  
     RPI_GPIO_P1_16        = 23,  
     RPI_GPIO_P1_18        = 24,  
     RPI_GPIO_P1_19        = 10,  
     RPI_GPIO_P1_21        =  9,  
     RPI_GPIO_P1_22        = 25,  
     RPI_GPIO_P1_23        = 11,  
     RPI_GPIO_P1_24        =  8,  
     RPI_GPIO_P1_26        =  7,  
     /* RPi Version 2 */
     RPI_V2_GPIO_P1_03     =  2,  
     RPI_V2_GPIO_P1_05     =  3,  
     RPI_V2_GPIO_P1_07     =  4,  
     RPI_V2_GPIO_P1_08     = 14,  
     RPI_V2_GPIO_P1_10     = 15,  
     RPI_V2_GPIO_P1_11     = 17,  
     RPI_V2_GPIO_P1_12     = 18,  
     RPI_V2_GPIO_P1_13     = 27,  
     RPI_V2_GPIO_P1_15     = 22,  
     RPI_V2_GPIO_P1_16     = 23,  
     RPI_V2_GPIO_P1_18     = 24,  
     RPI_V2_GPIO_P1_19     = 10,  
     RPI_V2_GPIO_P1_21     =  9,  
     RPI_V2_GPIO_P1_22     = 25,  
     RPI_V2_GPIO_P1_23     = 11,  
     RPI_V2_GPIO_P1_24     =  8,  
     RPI_V2_GPIO_P1_26     =  7,  
     RPI_V2_GPIO_P1_29     =  5,  
     RPI_V2_GPIO_P1_31     =  6,  
     RPI_V2_GPIO_P1_32     = 12,  
     RPI_V2_GPIO_P1_33     = 13,  
     RPI_V2_GPIO_P1_35     = 19,  
     RPI_V2_GPIO_P1_36     = 16,  
     RPI_V2_GPIO_P1_37     = 26,  
     RPI_V2_GPIO_P1_38     = 20,  
     RPI_V2_GPIO_P1_40     = 21,  
     /* RPi Version 2, new plug P5 */
     RPI_V2_GPIO_P5_03     = 28,  
     RPI_V2_GPIO_P5_04     = 29,  
     RPI_V2_GPIO_P5_05     = 30,  
     RPI_V2_GPIO_P5_06     = 31,  
     /* RPi B+ J8 header, also RPi 2 40 pin GPIO header */
     RPI_BPLUS_GPIO_J8_03     =  2,  
     RPI_BPLUS_GPIO_J8_05     =  3,  
     RPI_BPLUS_GPIO_J8_07     =  4,  
     RPI_BPLUS_GPIO_J8_08     = 14,  
     RPI_BPLUS_GPIO_J8_10     = 15,  
     RPI_BPLUS_GPIO_J8_11     = 17,  
     RPI_BPLUS_GPIO_J8_12     = 18,  
     RPI_BPLUS_GPIO_J8_13     = 27,  
     RPI_BPLUS_GPIO_J8_15     = 22,  
     RPI_BPLUS_GPIO_J8_16     = 23,  
     RPI_BPLUS_GPIO_J8_18     = 24,  
     RPI_BPLUS_GPIO_J8_19     = 10,  
     RPI_BPLUS_GPIO_J8_21     =  9,  
     RPI_BPLUS_GPIO_J8_22     = 25,  
     RPI_BPLUS_GPIO_J8_23     = 11,  
     RPI_BPLUS_GPIO_J8_24     =  8,  
     RPI_BPLUS_GPIO_J8_26     =  7,  
     RPI_BPLUS_GPIO_J8_29     =  5,  
     RPI_BPLUS_GPIO_J8_31     =  6,  
     RPI_BPLUS_GPIO_J8_32     = 12,  
     RPI_BPLUS_GPIO_J8_33     = 13,  
     RPI_BPLUS_GPIO_J8_35     = 19,  
     RPI_BPLUS_GPIO_J8_36     = 16,  
     RPI_BPLUS_GPIO_J8_37     = 26,  
     RPI_BPLUS_GPIO_J8_38     = 20,  
     RPI_BPLUS_GPIO_J8_40     = 21   
 } RPiGPIOPin;
 
 /* Defines for AUX
   GPIO register offsets from BCM2835_AUX_BASE.
 */
 #define BCM2835_AUX_IRQ                 0x0000  
 #define BCM2835_AUX_ENABLE              0x0004  
 #define BCM2835_AUX_ENABLE_UART1        0x01    
 #define BCM2835_AUX_ENABLE_SPI0         0x02    
 #define BCM2835_AUX_ENABLE_SPI1         0x04    
 #define BCM2835_AUX_SPI_CNTL0           0x0000  
 #define BCM2835_AUX_SPI_CNTL1           0x0004  
 #define BCM2835_AUX_SPI_STAT            0x0008  
 #define BCM2835_AUX_SPI_PEEK            0x000C  
 #define BCM2835_AUX_SPI_IO              0x0020  
 #define BCM2835_AUX_SPI_TXHOLD          0x0030  
 #define BCM2835_AUX_SPI_CLOCK_MIN       30500           
 #define BCM2835_AUX_SPI_CLOCK_MAX       125000000       
 #define BCM2835_AUX_SPI_CNTL0_SPEED     0xFFF00000  
 #define BCM2835_AUX_SPI_CNTL0_SPEED_MAX 0xFFF      
 #define BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT 20        
 #define BCM2835_AUX_SPI_CNTL0_CS0_N     0x000C0000 
 #define BCM2835_AUX_SPI_CNTL0_CS1_N     0x000A0000 
 #define BCM2835_AUX_SPI_CNTL0_CS2_N     0x00060000 
 #define BCM2835_AUX_SPI_CNTL0_POSTINPUT 0x00010000  
 #define BCM2835_AUX_SPI_CNTL0_VAR_CS    0x00008000  
 #define BCM2835_AUX_SPI_CNTL0_VAR_WIDTH 0x00004000  
 #define BCM2835_AUX_SPI_CNTL0_DOUTHOLD  0x00003000  
 #define BCM2835_AUX_SPI_CNTL0_ENABLE    0x00000800  
 #define BCM2835_AUX_SPI_CNTL0_CPHA_IN   0x00000400  
 #define BCM2835_AUX_SPI_CNTL0_CLEARFIFO 0x00000200  
 #define BCM2835_AUX_SPI_CNTL0_CPHA_OUT  0x00000100  
 #define BCM2835_AUX_SPI_CNTL0_CPOL      0x00000080  
 #define BCM2835_AUX_SPI_CNTL0_MSBF_OUT  0x00000040  
 #define BCM2835_AUX_SPI_CNTL0_SHIFTLEN  0x0000003F  
 #define BCM2835_AUX_SPI_CNTL1_CSHIGH    0x00000700  
 #define BCM2835_AUX_SPI_CNTL1_IDLE      0x00000080  
 #define BCM2835_AUX_SPI_CNTL1_TXEMPTY   0x00000040  
 #define BCM2835_AUX_SPI_CNTL1_MSBF_IN   0x00000002  
 #define BCM2835_AUX_SPI_CNTL1_KEEP_IN   0x00000001  
 #define BCM2835_AUX_SPI_STAT_TX_LVL     0xFF000000  
 #define BCM2835_AUX_SPI_STAT_RX_LVL     0x00FF0000  
 #define BCM2835_AUX_SPI_STAT_TX_FULL    0x00000400  
 #define BCM2835_AUX_SPI_STAT_TX_EMPTY   0x00000200  
 #define BCM2835_AUX_SPI_STAT_RX_FULL    0x00000100  
 #define BCM2835_AUX_SPI_STAT_RX_EMPTY   0x00000080  
 #define BCM2835_AUX_SPI_STAT_BUSY       0x00000040  
 #define BCM2835_AUX_SPI_STAT_BITCOUNT   0x0000003F  
 /* Defines for SPI
    GPIO register offsets from BCM2835_SPI0_BASE. 
    Offsets into the SPI Peripheral block in bytes per 10.5 SPI Register Map
 */
 #define BCM2835_SPI0_CS                      0x0000 
 #define BCM2835_SPI0_FIFO                    0x0004 
 #define BCM2835_SPI0_CLK                     0x0008 
 #define BCM2835_SPI0_DLEN                    0x000c 
 #define BCM2835_SPI0_LTOH                    0x0010 
 #define BCM2835_SPI0_DC                      0x0014 
 /* Register masks for SPI0_CS */
 #define BCM2835_SPI0_CS_LEN_LONG             0x02000000 
 #define BCM2835_SPI0_CS_DMA_LEN              0x01000000 
 #define BCM2835_SPI0_CS_CSPOL2               0x00800000 
 #define BCM2835_SPI0_CS_CSPOL1               0x00400000 
 #define BCM2835_SPI0_CS_CSPOL0               0x00200000 
 #define BCM2835_SPI0_CS_RXF                  0x00100000 
 #define BCM2835_SPI0_CS_RXR                  0x00080000 
 #define BCM2835_SPI0_CS_TXD                  0x00040000 
 #define BCM2835_SPI0_CS_RXD                  0x00020000 
 #define BCM2835_SPI0_CS_DONE                 0x00010000 
 #define BCM2835_SPI0_CS_TE_EN                0x00008000 
 #define BCM2835_SPI0_CS_LMONO                0x00004000 
 #define BCM2835_SPI0_CS_LEN                  0x00002000 
 #define BCM2835_SPI0_CS_REN                  0x00001000 
 #define BCM2835_SPI0_CS_ADCS                 0x00000800 
 #define BCM2835_SPI0_CS_INTR                 0x00000400 
 #define BCM2835_SPI0_CS_INTD                 0x00000200 
 #define BCM2835_SPI0_CS_DMAEN                0x00000100 
 #define BCM2835_SPI0_CS_TA                   0x00000080 
 #define BCM2835_SPI0_CS_CSPOL                0x00000040 
 #define BCM2835_SPI0_CS_CLEAR                0x00000030 
 #define BCM2835_SPI0_CS_CLEAR_RX             0x00000020 
 #define BCM2835_SPI0_CS_CLEAR_TX             0x00000010 
 #define BCM2835_SPI0_CS_CPOL                 0x00000008 
 #define BCM2835_SPI0_CS_CPHA                 0x00000004 
 #define BCM2835_SPI0_CS_CS                   0x00000003 
 typedef enum
 {
     BCM2835_SPI_BIT_ORDER_LSBFIRST = 0,  
     BCM2835_SPI_BIT_ORDER_MSBFIRST = 1   
 }bcm2835SPIBitOrder;
 
 typedef enum
 {
     BCM2835_SPI_MODE0 = 0,  
     BCM2835_SPI_MODE1 = 1,  
     BCM2835_SPI_MODE2 = 2,  
     BCM2835_SPI_MODE3 = 3   
 }bcm2835SPIMode;
 
 typedef enum
 {
     BCM2835_SPI_CS0 = 0,     
     BCM2835_SPI_CS1 = 1,     
     BCM2835_SPI_CS2 = 2,     
     BCM2835_SPI_CS_NONE = 3  
 } bcm2835SPIChipSelect;
 
 typedef enum
 {
     BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,       
     BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,   
     BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,   
     BCM2835_SPI_CLOCK_DIVIDER_8192  = 8192,    
     BCM2835_SPI_CLOCK_DIVIDER_4096  = 4096,    
     BCM2835_SPI_CLOCK_DIVIDER_2048  = 2048,    
     BCM2835_SPI_CLOCK_DIVIDER_1024  = 1024,    
     BCM2835_SPI_CLOCK_DIVIDER_512   = 512,     
     BCM2835_SPI_CLOCK_DIVIDER_256   = 256,     
     BCM2835_SPI_CLOCK_DIVIDER_128   = 128,     
     BCM2835_SPI_CLOCK_DIVIDER_64    = 64,      
     BCM2835_SPI_CLOCK_DIVIDER_32    = 32,      
     BCM2835_SPI_CLOCK_DIVIDER_16    = 16,      
     BCM2835_SPI_CLOCK_DIVIDER_8     = 8,       
     BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       
     BCM2835_SPI_CLOCK_DIVIDER_2     = 2,       
     BCM2835_SPI_CLOCK_DIVIDER_1     = 1        
 } bcm2835SPIClockDivider;
 
 /* Defines for I2C
    GPIO register offsets from BCM2835_BSC*_BASE.
    Offsets into the BSC Peripheral block in bytes per 3.1 BSC Register Map
 */
 #define BCM2835_BSC_C                                                   0x0000 
 #define BCM2835_BSC_S                   0x0004 
 #define BCM2835_BSC_DLEN                0x0008 
 #define BCM2835_BSC_A                   0x000c 
 #define BCM2835_BSC_FIFO                0x0010 
 #define BCM2835_BSC_DIV                 0x0014 
 #define BCM2835_BSC_DEL                 0x0018 
 #define BCM2835_BSC_CLKT                0x001c 
 /* Register masks for BSC_C */
 #define BCM2835_BSC_C_I2CEN             0x00008000 
 #define BCM2835_BSC_C_INTR              0x00000400 
 #define BCM2835_BSC_C_INTT              0x00000200 
 #define BCM2835_BSC_C_INTD              0x00000100 
 #define BCM2835_BSC_C_ST                0x00000080 
 #define BCM2835_BSC_C_CLEAR_1           0x00000020 
 #define BCM2835_BSC_C_CLEAR_2           0x00000010 
 #define BCM2835_BSC_C_READ              0x00000001 
 /* Register masks for BSC_S */
 #define BCM2835_BSC_S_CLKT              0x00000200 
 #define BCM2835_BSC_S_ERR               0x00000100 
 #define BCM2835_BSC_S_RXF               0x00000080 
 #define BCM2835_BSC_S_TXE               0x00000040 
 #define BCM2835_BSC_S_RXD               0x00000020 
 #define BCM2835_BSC_S_TXD               0x00000010 
 #define BCM2835_BSC_S_RXR               0x00000008 
 #define BCM2835_BSC_S_TXW               0x00000004 
 #define BCM2835_BSC_S_DONE              0x00000002 
 #define BCM2835_BSC_S_TA                0x00000001 
 #define BCM2835_BSC_FIFO_SIZE           16 
 typedef enum
 {
     BCM2835_I2C_CLOCK_DIVIDER_2500   = 2500,      
     BCM2835_I2C_CLOCK_DIVIDER_626    = 626,       
     BCM2835_I2C_CLOCK_DIVIDER_150    = 150,       
     BCM2835_I2C_CLOCK_DIVIDER_148    = 148        
 } bcm2835I2CClockDivider;
 
 typedef enum
 {
     BCM2835_I2C_REASON_OK            = 0x00,      
     BCM2835_I2C_REASON_ERROR_NACK    = 0x01,      
     BCM2835_I2C_REASON_ERROR_CLKT    = 0x02,      
     BCM2835_I2C_REASON_ERROR_DATA    = 0x04       
 } bcm2835I2CReasonCodes;
 
 /* Defines for ST
    GPIO register offsets from BCM2835_ST_BASE.
    Offsets into the ST Peripheral block in bytes per 12.1 System Timer Registers
    The System Timer peripheral provides four 32-bit timer channels and a single 64-bit free running counter.
    BCM2835_ST_CLO is the System Timer Counter Lower bits register.
    The system timer free-running counter lower register is a read-only register that returns the current value
    of the lower 32-bits of the free running counter.
    BCM2835_ST_CHI is the System Timer Counter Upper bits register.
    The system timer free-running counter upper register is a read-only register that returns the current value
    of the upper 32-bits of the free running counter.
 */
 #define BCM2835_ST_CS                   0x0000 
 #define BCM2835_ST_CLO                  0x0004 
 #define BCM2835_ST_CHI                  0x0008 
 /* Defines for PWM, word offsets (ie 4 byte multiples) */
 #define BCM2835_PWM_CONTROL 0
 #define BCM2835_PWM_STATUS  1
 #define BCM2835_PWM_DMAC    2
 #define BCM2835_PWM0_RANGE  4
 #define BCM2835_PWM0_DATA   5
 #define BCM2835_PWM_FIF1    6
 #define BCM2835_PWM1_RANGE  8
 #define BCM2835_PWM1_DATA   9
 
 /* Defines for PWM Clock, word offsets (ie 4 byte multiples) */
 #define BCM2835_PWMCLK_CNTL     40
 #define BCM2835_PWMCLK_DIV      41
 #define BCM2835_PWM_PASSWRD     (0x5A << 24)  
 #define BCM2835_PWM1_MS_MODE    0x8000  
 #define BCM2835_PWM1_USEFIFO    0x2000  
 #define BCM2835_PWM1_REVPOLAR   0x1000  
 #define BCM2835_PWM1_OFFSTATE   0x0800  
 #define BCM2835_PWM1_REPEATFF   0x0400  
 #define BCM2835_PWM1_SERIAL     0x0200  
 #define BCM2835_PWM1_ENABLE     0x0100  
 #define BCM2835_PWM0_MS_MODE    0x0080  
 #define BCM2835_PWM_CLEAR_FIFO  0x0040  
 #define BCM2835_PWM0_USEFIFO    0x0020  
 #define BCM2835_PWM0_REVPOLAR   0x0010  
 #define BCM2835_PWM0_OFFSTATE   0x0008  
 #define BCM2835_PWM0_REPEATFF   0x0004  
 #define BCM2835_PWM0_SERIAL     0x0002  
 #define BCM2835_PWM0_ENABLE     0x0001  
 typedef enum
 {
     BCM2835_PWM_CLOCK_DIVIDER_2048  = 2048,    
     BCM2835_PWM_CLOCK_DIVIDER_1024  = 1024,    
     BCM2835_PWM_CLOCK_DIVIDER_512   = 512,     
     BCM2835_PWM_CLOCK_DIVIDER_256   = 256,     
     BCM2835_PWM_CLOCK_DIVIDER_128   = 128,     
     BCM2835_PWM_CLOCK_DIVIDER_64    = 64,      
     BCM2835_PWM_CLOCK_DIVIDER_32    = 32,      
     BCM2835_PWM_CLOCK_DIVIDER_16    = 16,      
     BCM2835_PWM_CLOCK_DIVIDER_8     = 8,       
     BCM2835_PWM_CLOCK_DIVIDER_4     = 4,       
     BCM2835_PWM_CLOCK_DIVIDER_2     = 2,       
     BCM2835_PWM_CLOCK_DIVIDER_1     = 1        
 } bcm2835PWMClockDivider;
 
 /* Historical name compatibility */
 #ifndef BCM2835_NO_DELAY_COMPATIBILITY
 //#define delay(x) bcm2835_delay(x)
 //#define delayMicroseconds(x) bcm2835_delayMicroseconds(x)
 #endif
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
     extern int bcm2835_init(void);
 
     extern int bcm2835_close(void);
 
     extern void  bcm2835_set_debug(uint8_t debug);
 
     extern unsigned int bcm2835_version(void);
 
     extern uint32_t* bcm2835_regbase(uint8_t regbase);
 
     extern uint32_t bcm2835_peri_read(volatile uint32_t* paddr);
 
     extern uint32_t bcm2835_peri_read_nb(volatile uint32_t* paddr);
 
 
     extern void bcm2835_peri_write(volatile uint32_t* paddr, uint32_t value);
 
     extern void bcm2835_peri_write_nb(volatile uint32_t* paddr, uint32_t value);
 
     extern void bcm2835_peri_set_bits(volatile uint32_t* paddr, uint32_t value, uint32_t mask);
     extern void bcm2835_gpio_fsel(uint8_t pin, uint8_t mode);
 
     extern void bcm2835_gpio_set(uint8_t pin);
 
     extern void bcm2835_gpio_clr(uint8_t pin);
 
     extern void bcm2835_gpio_set_multi(uint32_t mask);
 
     extern void bcm2835_gpio_clr_multi(uint32_t mask);
 
     extern uint8_t bcm2835_gpio_lev(uint8_t pin);
 
     extern uint8_t bcm2835_gpio_eds(uint8_t pin);
 
     extern uint32_t bcm2835_gpio_eds_multi(uint32_t mask);
 
     extern void bcm2835_gpio_set_eds(uint8_t pin);
 
     extern void bcm2835_gpio_set_eds_multi(uint32_t mask);
     
     extern void bcm2835_gpio_ren(uint8_t pin);
 
     extern void bcm2835_gpio_clr_ren(uint8_t pin);
 
     extern void bcm2835_gpio_fen(uint8_t pin);
 
     extern void bcm2835_gpio_clr_fen(uint8_t pin);
 
     extern void bcm2835_gpio_hen(uint8_t pin);
 
     extern void bcm2835_gpio_clr_hen(uint8_t pin);
 
     extern void bcm2835_gpio_len(uint8_t pin);
 
     extern void bcm2835_gpio_clr_len(uint8_t pin);
 
     extern void bcm2835_gpio_aren(uint8_t pin);
 
     extern void bcm2835_gpio_clr_aren(uint8_t pin);
 
     extern void bcm2835_gpio_afen(uint8_t pin);
 
     extern void bcm2835_gpio_clr_afen(uint8_t pin);
 
     extern void bcm2835_gpio_pud(uint8_t pud);
 
     extern void bcm2835_gpio_pudclk(uint8_t pin, uint8_t on);
 
     extern uint32_t bcm2835_gpio_pad(uint8_t group);
 
     extern void bcm2835_gpio_set_pad(uint8_t group, uint32_t control);
 
     extern void bcm2835_delay (unsigned int millis);
 
     extern void bcm2835_delayMicroseconds (uint64_t micros);
 
     extern void bcm2835_gpio_write(uint8_t pin, uint8_t on);
 
     extern void bcm2835_gpio_write_multi(uint32_t mask, uint8_t on);
 
     extern void bcm2835_gpio_write_mask(uint32_t value, uint32_t mask);
 
     extern void bcm2835_gpio_set_pud(uint8_t pin, uint8_t pud);
 
     extern uint8_t bcm2835_gpio_get_pud(uint8_t pin);
 
     extern int bcm2835_spi_begin(void);
 
     extern void bcm2835_spi_end(void);
 
     extern void bcm2835_spi_setBitOrder(uint8_t order);
 
     extern void bcm2835_spi_setClockDivider(uint16_t divider);
 
    extern void bcm2835_spi_set_speed_hz(uint32_t speed_hz);
 
     extern void bcm2835_spi_setDataMode(uint8_t mode);
 
     extern void bcm2835_spi_chipSelect(uint8_t cs);
 
     extern void bcm2835_spi_setChipSelectPolarity(uint8_t cs, uint8_t active);
 
     extern uint8_t bcm2835_spi_transfer(uint8_t value);
     
     extern void bcm2835_spi_transfernb(char* tbuf, char* rbuf, uint32_t len);
 
     extern void bcm2835_spi_transfern(char* buf, uint32_t len);
 
     extern void bcm2835_spi_writenb(const char* buf, uint32_t len);
 
     extern void bcm2835_spi_write(uint16_t data);
 
     extern int bcm2835_aux_spi_begin(void);
 
     extern void bcm2835_aux_spi_end(void);
 
     extern void bcm2835_aux_spi_setClockDivider(uint16_t divider);
 
     extern uint16_t bcm2835_aux_spi_CalcClockDivider(uint32_t speed_hz);
 
     extern void bcm2835_aux_spi_write(uint16_t data);
 
     extern void bcm2835_aux_spi_writenb(const char *buf, uint32_t len);
 
     extern void bcm2835_aux_spi_transfern(char *buf, uint32_t len);
 
     extern void bcm2835_aux_spi_transfernb(const char *tbuf, char *rbuf, uint32_t len);
 
     extern int bcm2835_i2c_begin(void);
 
     extern void bcm2835_i2c_end(void);
 
     extern void bcm2835_i2c_setSlaveAddress(uint8_t addr);
 
     extern void bcm2835_i2c_setClockDivider(uint16_t divider);
 
     extern void bcm2835_i2c_set_baudrate(uint32_t baudrate);
 
     extern uint8_t bcm2835_i2c_write(const char * buf, uint32_t len);
 
     extern uint8_t bcm2835_i2c_read(char* buf, uint32_t len);
 
     extern uint8_t bcm2835_i2c_read_register_rs(char* regaddr, char* buf, uint32_t len);
 
     extern uint8_t bcm2835_i2c_write_read_rs(char* cmds, uint32_t cmds_len, char* buf, uint32_t buf_len);
 
     extern uint64_t bcm2835_st_read(void);
 
     extern void bcm2835_st_delay(uint64_t offset_micros, uint64_t micros);
 
     extern void bcm2835_pwm_set_clock(uint32_t divisor);
     
     extern void bcm2835_pwm_set_mode(uint8_t channel, uint8_t markspace, uint8_t enabled);
 
     extern void bcm2835_pwm_set_range(uint8_t channel, uint32_t range);
     
     extern void bcm2835_pwm_set_data(uint8_t channel, uint32_t data);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* BCM2835_H */