/*
 * Lora_Frame.c
 *
 *  Created on: May 17, 2026
 *      Author: ACER
 */

#include "LoRa_Frame.h"

LoRa myLoRa;

uint8_t rssi_lora = 0;
uint8_t rx_lora[RX_BUFFER_SIZE];
uint8_t cmd_lora = CMD_LORA_TRACKING;

lora_frame_t tx_lora_frame =
{
		.id_mesh   = ID_MESH,
		.id_device = ID_DEVICE,
		.id_relay  = 0,

		.cmd_lora  = CMD_LORA_TRACKING,
		.batt      = 0,

		.packet_cnt = 0,
		.hop_count  = 0,

		.gps =
		{
				.lat = 0,
				.lng = 0,
		},

		.sensor =
		{
				.temp = 0,
				.humi = 0,
		},

		.crc = 0,
};

lora_frame_t rx_lora_frame;

uint16_t calculate_crc16(const uint8_t *data, uint16_t length)
{
    if (data == NULL || length == 0) { return 0; }

    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i];
        for (uint16_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief  Initializes the LoRa module with hardware mappings and RF configurations.
 * @param  None
 * @retval true  : Initialization successful, module is in continuous RX mode.
 * false : Initialization failed (module not found or SPI error).
 */
void Lora_Init(SPI_HandleTypeDef *hspi,
               GPIO_TypeDef *cs_port, uint16_t cs_pin,
               GPIO_TypeDef *rst_port, uint16_t rst_pin,
               GPIO_TypeDef *dio0_port, uint16_t dio0_pin)
{
    // Load default configurations into the LoRa structure
    myLoRa = newLoRa();

    // Hardware Peripheral Mapping
    myLoRa.hSPIx       = hspi;
	myLoRa.CS_port     = cs_port;
	myLoRa.CS_pin      = cs_pin;
	myLoRa.reset_port  = rst_port;
	myLoRa.reset_pin   = rst_pin;
	myLoRa.DIO0_port   = dio0_port;
	myLoRa.DIO0_pin    = dio0_pin;

    // RF Parameter Configurations
    myLoRa.frequency              = 433;         // Operating frequency in MHz
    myLoRa.spredingFactor         = SF_7;        // Spreading Factor 7
    myLoRa.bandWidth              = BW_125KHz;   // Signal bandwidth 125 kHz
    myLoRa.crcRate                = CR_4_5;      // Coding Rate 4/5
    myLoRa.power                  = POWER_20db;  // Output power 20 dBm
    myLoRa.overCurrentProtection  = 120;         // OCP threshold in mA
    myLoRa.preamble               = 10;          // Preamble length in symbols

    // Perform physical hardware reset via RST pin
    LoRa_reset(&myLoRa);

    // Initialize registers and verify chip version (checks for internal ID 0x12)
    while (LoRa_init(&myLoRa) != LORA_OK)
    {
    	HAL_Delay(1000);
    }

    // Set custom network Sync Word to isolate network traffic
    LoRa_setSyncWord(&myLoRa, 0x12);

    // Configure RegModemConfig3 (0x26) to 0x04 to enable AgcAutoOn (Automatic Gain Control)
    LoRa_write(&myLoRa, 0x26, 0x04);

    // Enter continuous receive mode to listen for incoming payloads
    LoRa_startReceiving(&myLoRa);
}


/**
 * @brief  Reads a data frame from the LoRa module, parses it, and verifies data integrity.
 * @param  p_frame: Pointer to the lora_frame_t structure to store data upon successful validation.
 * @retval 1: Successfully received a valid frame (correct size and matching CRC).
 * 0: No data available or frame corruption detected.
 */
bool LoRa_Read_Frame(lora_frame_t *p_frame)
{
    if (p_frame == NULL) return 0;

    // Read data from the LoRa module into the receive buffer
    uint8_t rx_len = LoRa_receive(&myLoRa, rx_lora, RX_BUFFER_SIZE - 1);

    // Verify if the received packet size matches the expected struct size
    if (rx_len == sizeof(lora_frame_t))
    {
        // Copy data directly from the buffer to the destination struct to optimize resources
        memcpy(p_frame, rx_lora, sizeof(lora_frame_t));

        // Calculate CRC16 over the payload (excluding the CRC field at the end)
        uint16_t crc_payload_len = sizeof(lora_frame_t) - sizeof(p_frame->crc);
        uint16_t calc_crc = calculate_crc16((uint8_t*)p_frame, crc_payload_len);

        // Verify data integrity by comparing calculated CRC with the received CRC
        if (calc_crc == p_frame->crc && p_frame->id_mesh == ID_MESH && p_frame->id_device != ID_DEVICE)
        {
        	rssi_lora = LoRa_getRSSI(&myLoRa);
            return true; // Data validation successful
        }
    }

    return false; // No data available or frame error
}

/**
 * @brief  Calculates the CRC16 checksum for the frame payload and transmits it via LoRa.
 * @param  p_frame: Pointer to the lora_frame_t structure containing data to be transmitted.
 * @retval 1: Transmission successful.
 * 0: Transmission failed or invalid pointer.
 */
bool LoRa_Send_Frame(lora_frame_t *p_frame, int cmd_data)
{
    if (p_frame == NULL) return 0;

    // Calculate CRC16 over the payload (excluding the 2-byte CRC field at the end)
    uint16_t crc_payload_len = sizeof(lora_frame_t) - sizeof(p_frame->crc);
    p_frame->crc = calculate_crc16((uint8_t*)p_frame, crc_payload_len);

    // Transmit the entire structure (including data, padding, and CRC)
    // Timeout is set to 2000ms based on driver configuration
    uint8_t tx_result = LoRa_transmit(&myLoRa, (uint8_t*)p_frame, sizeof(lora_frame_t), 2000);

    if (tx_result == 1)
    {
        return true; // Transmission successful
    }

    return false; // Transmission failed
}
