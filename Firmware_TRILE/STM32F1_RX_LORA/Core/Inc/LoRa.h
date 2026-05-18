#ifndef __LORA_H
#define __LORA_H

#include "stm32f1xx_hal.h"

#define LORA_OK 200
#define LORA_NOT_FOUND 404

#define SLEEP_MODE 0
#define STNBY_MODE 1
#define TRANSMIT_MODE 3
#define RXCONTIN_MODE 5

// Registers
#define RegFiFo 0x00
#define RegOpMode 0x01
#define RegFrMsb 0x06
#define RegFrMid 0x07
#define RegFrLsb 0x08
#define RegPaConfig 0x09
#define RegOcp 0x0B
#define RegLna 0x0C
#define RegFiFoAddPtr 0x0D
#define RegFiFoTxBaseAddr 0x0E
#define RegFiFoRxBaseAddr 0x0F
#define RegFiFoRxCurrentAddr 0x10
#define RegIrqFlags 0x12
#define RegRxNbBytes 0x13
#define RegPktRssiValue 0x1A
#define RegModemConfig1 0x1D
#define RegModemConfig2 0x1E
#define RegSymbTimeoutL 0x1F
#define RegPreambleMsb 0x20
#define RegPreambleLsb 0x21
#define RegPayloadLength 0x22
#define RegModemConfig3 0x26
#define RegDioMapping1 0x40
#define RegVersion 0x42

// Settings
#define BW_250KHz 8
#define BW_125KHz 7
#define SF_7 7
#define CR_4_5 1
#define POWER_20db 0xFF
#define POWER_17db 0xFC
#define POWER_14db 0xF9

typedef struct {
    int current_mode;
    int frequency;
    int spredingFactor;
    int bandWidth;
    int crcRate;
    int power;
    int overCurrentProtection;
    int preamble;
    
    SPI_HandleTypeDef* hSPIx;
    GPIO_TypeDef* CS_port;
    uint16_t CS_pin;
    GPIO_TypeDef* reset_port;
    uint16_t reset_pin;
    GPIO_TypeDef* DIO0_port;
    uint16_t DIO0_pin;
} LoRa;

LoRa newLoRa(void);
uint8_t LoRa_init(LoRa* _LoRa);
void LoRa_reset(LoRa* _LoRa);
void LoRa_readReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* output, uint16_t w_length);
void LoRa_writeReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* values, uint16_t w_length);
void LoRa_gotoMode(LoRa* _LoRa, int mode);
uint8_t LoRa_read(LoRa* _LoRa, uint8_t address);
void LoRa_write(LoRa* _LoRa, uint8_t address, uint8_t value);
void LoRa_BurstWrite(LoRa* _LoRa, uint8_t address, uint8_t *value, uint8_t length);
uint8_t LoRa_transmit(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout);
uint8_t LoRa_receive(LoRa* _LoRa, uint8_t* data, uint8_t length);
void LoRa_startReceiving(LoRa* _LoRa);
int LoRa_getRSSI(LoRa* _LoRa);
void LoRa_setSyncWord(LoRa* _LoRa, uint8_t syncWord);

#endif
