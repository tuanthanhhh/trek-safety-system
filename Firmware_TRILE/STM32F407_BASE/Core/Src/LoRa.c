#include "LoRa.h"
#define TRANSMIT_TIMEOUT 2000
#define RECEIVE_TIMEOUT  2000

LoRa newLoRa(void) {
    LoRa _LoRa;
    _LoRa.frequency = 433;
    _LoRa.spredingFactor = SF_7;
    _LoRa.bandWidth = BW_250KHz;
    _LoRa.crcRate = CR_4_5;
    _LoRa.power = POWER_20db;
    _LoRa.overCurrentProtection = 120;
    _LoRa.preamble = 10;
    return _LoRa;
}

void LoRa_reset(LoRa* _LoRa) {
    HAL_GPIO_WritePin(_LoRa->reset_port, _LoRa->reset_pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(_LoRa->reset_port, _LoRa->reset_pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

void LoRa_readReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* output, uint16_t w_length) {
    HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(_LoRa->hSPIx, address, r_length, TRANSMIT_TIMEOUT);
    while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
    HAL_SPI_Receive(_LoRa->hSPIx, output, w_length, RECEIVE_TIMEOUT);
    while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
}

void LoRa_writeReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* values, uint16_t w_length) {
    HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(_LoRa->hSPIx, address, r_length, TRANSMIT_TIMEOUT);
    while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
    HAL_SPI_Transmit(_LoRa->hSPIx, values, w_length, TRANSMIT_TIMEOUT);
    while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
}

void LoRa_gotoMode(LoRa* _LoRa, int mode) {
    uint8_t read;
    uint8_t data;
    read = LoRa_read(_LoRa, RegOpMode);
    
    if (mode == SLEEP_MODE) {
        data = (read & 0xF8) | 0x00;
        _LoRa->current_mode = SLEEP_MODE;
    } else if (mode == STNBY_MODE) {
        data = (read & 0xF8) | 0x01;
        _LoRa->current_mode = STNBY_MODE;
    } else if (mode == TRANSMIT_MODE) {
        data = (read & 0xF8) | 0x03;
        _LoRa->current_mode = TRANSMIT_MODE;
    } else if (mode == RXCONTIN_MODE) {
        data = (read & 0xF8) | 0x05;
        _LoRa->current_mode = RXCONTIN_MODE;
    }
    
    LoRa_write(_LoRa, RegOpMode, data);
}

uint8_t LoRa_read(LoRa* _LoRa, uint8_t address) {
    uint8_t read_data;
    uint8_t addr = address & 0x7F;
    LoRa_readReg(_LoRa, &addr, 1, &read_data, 1);
    return read_data;
}

void LoRa_write(LoRa* _LoRa, uint8_t address, uint8_t value) {
    uint8_t addr = address | 0x80;
    LoRa_writeReg(_LoRa, &addr, 1, &value, 1);
}

void LoRa_BurstWrite(LoRa* _LoRa, uint8_t address, uint8_t *value, uint8_t length) {
    uint8_t addr = address | 0x80;
    HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(_LoRa->hSPIx, &addr, 1, TRANSMIT_TIMEOUT);
    while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
    HAL_SPI_Transmit(_LoRa->hSPIx, value, length, TRANSMIT_TIMEOUT);
    while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
}

void LoRa_setFrequency(LoRa* _LoRa, int freq) {
    uint64_t frf = ((uint64_t)freq * 524288) / 32;
    LoRa_write(_LoRa, RegFrMsb, (uint8_t)(frf >> 16));
    LoRa_write(_LoRa, RegFrMid, (uint8_t)(frf >> 8));
    LoRa_write(_LoRa, RegFrLsb, (uint8_t)(frf >> 0));
}

void LoRa_setSpreadingFactor(LoRa* _LoRa, int SP) {
    uint8_t read = LoRa_read(_LoRa, RegModemConfig2);
    uint8_t data = (SP << 4) | (read & 0x0F);
    LoRa_write(_LoRa, RegModemConfig2, data);
}

void LoRa_setPower(LoRa* _LoRa, uint8_t power) {
    LoRa_write(_LoRa, RegPaConfig, power);
}

void LoRa_setOCP(LoRa* _LoRa, int current) {
    uint8_t OcpTrim = 0;
    if (current <= 120) {
        OcpTrim = (current - 45) / 5;
    } else if (current <= 240) {
        OcpTrim = (current + 30) / 10;
    }
    LoRa_write(_LoRa, RegOcp, 0x20 | (0x1F & OcpTrim));
}

void LoRa_setSyncWord(LoRa* _LoRa, uint8_t syncWord) {
    LoRa_write(_LoRa, 0x39, syncWord);
}

uint8_t LoRa_init(LoRa* _LoRa) {
    uint8_t read;
    uint8_t data;
    
    LoRa_reset(_LoRa);
    
    read = LoRa_read(_LoRa, RegVersion);
    if (read != 0x12) {
        return LORA_NOT_FOUND;
    }
    
    LoRa_gotoMode(_LoRa, SLEEP_MODE);
    LoRa_write(_LoRa, RegOpMode, 0x80); // LORA mode
    
    LoRa_setFrequency(_LoRa, _LoRa->frequency);
    LoRa_setPower(_LoRa, _LoRa->power);
    LoRa_setOCP(_LoRa, _LoRa->overCurrentProtection);
    LoRa_write(_LoRa, RegLna, 0x23);

    // Set Spreading factor & CRC on
    read = LoRa_read(_LoRa, RegModemConfig2);
    data = read | 0x04; // CRC on
    LoRa_write(_LoRa, RegModemConfig2, data);
    LoRa_setSpreadingFactor(_LoRa, _LoRa->spredingFactor);

    // Set Timeout Lsb
    LoRa_write(_LoRa, RegSymbTimeoutL, 0xFF);

    // Bandwidth, coding rate, explicit header
    data = (_LoRa->bandWidth << 4) | (_LoRa->crcRate << 1);
    LoRa_write(_LoRa, RegModemConfig1, data);

    // Set Preamble
    LoRa_write(_LoRa, RegPreambleMsb, _LoRa->preamble >> 8);
    LoRa_write(_LoRa, RegPreambleLsb, _LoRa->preamble & 0xFF);

    // DIO mapping RxDone -> DIO0
    LoRa_write(_LoRa, RegDioMapping1, 0x00);
    
    LoRa_gotoMode(_LoRa, STNBY_MODE);
    return LORA_OK;
}

uint8_t LoRa_transmit(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout) {
    int mode = _LoRa->current_mode;
    LoRa_gotoMode(_LoRa, STNBY_MODE);
    
    LoRa_write(_LoRa, RegIrqFlags, 0xFF);
    
    uint8_t read = LoRa_read(_LoRa, RegFiFoTxBaseAddr);
    LoRa_write(_LoRa, RegFiFoAddPtr, read);
    LoRa_write(_LoRa, RegPayloadLength, length);
    LoRa_BurstWrite(_LoRa, RegFiFo, data, length);
    
    LoRa_gotoMode(_LoRa, TRANSMIT_MODE);
    
    uint32_t start = HAL_GetTick();
    while (1) {
        read = LoRa_read(_LoRa, RegIrqFlags);
        if ((read & 0x08) != 0) { // TxDone
            LoRa_write(_LoRa, RegIrqFlags, 0xFF);
            LoRa_gotoMode(_LoRa, mode);
            return 1;
        }
        if ((HAL_GetTick() - start) > timeout) {
            LoRa_write(_LoRa, RegIrqFlags, 0xFF);
            LoRa_gotoMode(_LoRa, mode);
            return 0;
        }
    }
}

uint8_t LoRa_receive(LoRa* _LoRa, uint8_t* data, uint8_t length) {
    uint8_t read = LoRa_read(_LoRa, RegIrqFlags);
    if ((read & 0x40) != 0) { // RxDone
        if ((read & 0x20) != 0) {
            LoRa_write(_LoRa, RegIrqFlags, 0xFF);
            return 0;
        }
        
        LoRa_gotoMode(_LoRa, STNBY_MODE);
        LoRa_write(_LoRa, RegIrqFlags, 0xFF);
        
        uint8_t bytes = LoRa_read(_LoRa, RegRxNbBytes);
        read = LoRa_read(_LoRa, RegFiFoRxCurrentAddr);
        LoRa_write(_LoRa, RegFiFoAddPtr, read);
        
        uint8_t min = length >= bytes ? bytes : length;
        for (int i = 0; i < min; i++) {
            data[i] = LoRa_read(_LoRa, RegFiFo);
        }
        
        LoRa_gotoMode(_LoRa, RXCONTIN_MODE);
        return min;
    }
    return 0;
}

void LoRa_startReceiving(LoRa* _LoRa) {
    LoRa_gotoMode(_LoRa, RXCONTIN_MODE);
}

int LoRa_getRSSI(LoRa* _LoRa) {
    uint8_t read = LoRa_read(_LoRa, RegPktRssiValue);
    return -164 + read;
}
