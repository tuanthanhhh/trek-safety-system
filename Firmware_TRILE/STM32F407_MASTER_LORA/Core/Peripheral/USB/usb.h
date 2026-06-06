/*
 * usb.h
 *
 * Created on: May 28, 2026
 * Author: Lê Anh Trí Tri
 */

#ifndef USB_H
#define USB_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "LoRa_Frame.h"
#include <master_lora.h>

// Đổi tên các enum cho phù hợp với giao tiếp USB (tránh nhầm lẫn với bản cũ)
typedef enum
{
    CMD_USB_CONNECT_MASTER = 0,  // Gói USB App -> Master để thiết lập kết nối

    CMD_USB_GPS_UPDATE,          // Gói USB App -> Master để cập nhật vị trí GPS hiện tại của Master
    CMD_USB_HELP,                // Gói USB App -> Master để cập nhật vị trí GPS của Slave cần hỗ trợ

    CMD_USB_WARN_POINT,          // Gói USB App -> Master để cập nhật vị trí GPS điểm cảnh báo
    CMD_USB_MARK_POINT,          // Gói USB App -> Master để cập nhật vị trí GPS điểm cần đi đến

    CMD_USB_PING,                // Gói USB App -> Master để hỏi Slave còn sống không

    CMD_USB_TRACKING,            // Gói USB Master -> App để cập nhật gói tin tracking định kì từ Slave
    CMD_USB_PONG,                // Gói USB Master -> App để trả lời CMD_PING

	CMD_USB_NONE,

} cmd_usb_t;

typedef struct
{
    uint8_t cmd_usb;
    uint8_t id_slave;
    double lat;
    double lng;

} USB_Message_t;

extern USB_Message_t msg_usb;

/* Các hàm khởi tạo và xử lý USB */
void USB_ProcessReceivedData(void);

// Hàm này sẽ được gọi từ file usbd_cdc_if.c của hệ thống
void USB_Receive_Callback(uint8_t *Buf, uint32_t Len);

void Send_JSON_Message(lora_frame_t *p_frame);

#endif /* USB_H */
