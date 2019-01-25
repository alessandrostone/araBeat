/*******************************************************************************
 *    @brief    MAX30003 header
 *
 ********************************************************************************/
#ifndef _MAX_H_
#define _MAX_H_

/*******************************************************************************
 * Includes
 ********************************************************************************/
#include <Arduino.h>
#include <SPI.h>

/*******************************************************************************
 * Definitions
 ********************************************************************************/
// pin
#define MAX30003_CS_PIN 7

// registers
#define WREG 0x00
#define RREG 0x01

#define NO_OP_ 0x00
#define STATUS 0x01
#define EN_INT 0x02
#define EN_INT2 0x03
#define MNGR_INT 0x04
#define MNGR_DYN 0x05
#define SW_RST 0x08
#define SYNCH 0x09
#define FIFO_RST 0x0A
#define INFO 0x0F
#define CNFG_GEN 0x10
#define CNFG_CAL 0x12
#define CNFG_EMUX 0x14
#define CNFG_ECG 0x15
#define CNFG_RTOR1 0x1D
#define CNFG_RTOR2 0x1E
#define ECG_FIFO_BURST 0x20
#define ECG_FIFO 0x21
#define RTOR 0x25
#define NO_OP 0x7F

class MAX30003
{
public:
    // constructor
    MAX30003();

    // public functions
    void MAX30003_begin();
    void MAX30003_Reg_Read(uint8_t Reg_address);
    void MAX30003_Reg_Write(unsigned char WRITE_ADDRESS, unsigned long data);
};

#endif /*_MAX_H_*/
