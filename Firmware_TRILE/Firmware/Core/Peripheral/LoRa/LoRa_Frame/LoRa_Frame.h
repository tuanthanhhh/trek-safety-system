/*
 * LoRa_Frame.h
 *
 *  Created on: May 17, 2026
 *      Author: ACER
 */

#ifndef INC_LORA_FRAME_H_
#define INC_LORA_FRAME_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "LoRa.h"
#include "main.h"

#define ID_MESH        14
#define ID_DEVICE      2
#define RX_BUFFER_SIZE 256

extern uint8_t cmd_lora;
extern uint8_t rssi_lora;
extern uint8_t rx_lora[RX_BUFFER_SIZE];

extern LoRa myLoRa;

typedef enum
{
    CMD_LORA_TRACKING = 0, // Gói tin LORA định vị định kì
	CMD_LORA_SOS,          // Gói tin LORA thông báo node đang gặp nạn
	CMD_LORA_HELP,         // Gói tin LORA yêu cầu giúp đỡ

	CMD_LORA_WARN_POINT,   // Gói tin LORA gửi cảnh báo các vị trí nguy hiểm
	CMD_LORA_MARK_POINT,   // Gói tin LORA gửi vị trí các điểm cần đến

	CMD_LORA_PING,         // Gói tin LORA Master -> Slave để kiểm tra
	CMD_LORA_PONG,         // Gói tin LORA Salve -> Master để trả lời gói tin CMD_PING

} cmd_lora_t;

typedef struct
{
	uint8_t id_mesh;   // 1 Byte

    uint8_t id_source; // 1 Byte
    uint8_t id_dest;   // 1 Byte

    uint8_t cmd_lora; // 1 Byte
    uint8_t batt;      // 1 Byte

    // Hai trường mới bổ sung cho thuật toán Flooding Mesh
	uint8_t packet_cnt; // 1 Byte (Packet định danh gói tin tăng dần)
	uint8_t hop_count;  // 1 Byte (Số lần được phép chuyển tiếp)

    uint8_t reserved;   // 1 Byte (Padding tường minh để float căn chỉnh 4-byte)

    struct
	{
    	float lat; // 4 Bytes
    	float lng; // 4 Bytes

    } gps; // 4+4 = 8 Bytes

    struct
	{
    	uint8_t temp; // 1 Byte
    	uint8_t humi; // 1 Byte

    } sensor; // 1+1 = 2 Bytes

    uint16_t crc; // 2 Bytes

} lora_frame_t; // Kích thước cố định là 20 Bytes, căn chỉnh tự nhiên cực kì an toàn

extern lora_frame_t tx_lora_frame;
extern lora_frame_t rx_lora_frame;

uint16_t calculate_crc16(const uint8_t *data, uint16_t length);

void Lora_Init(SPI_HandleTypeDef *hspi,
               GPIO_TypeDef *cs_port, uint16_t cs_pin,
               GPIO_TypeDef *rst_port, uint16_t rst_pin,
               GPIO_TypeDef *dio0_port, uint16_t dio0_pin);

bool LoRa_Read_Frame(lora_frame_t *p_frame);
bool LoRa_Send_Frame(lora_frame_t *p_frame);
void Lora_Update_Data(float* latitude, float* longitude, uint8_t batt, uint8_t temp, uint8_t humi);

#endif /* INC_LORA_FRAME_H_ */
