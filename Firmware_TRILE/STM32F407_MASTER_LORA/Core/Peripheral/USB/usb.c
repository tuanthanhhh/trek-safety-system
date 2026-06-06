/*
 * usb.c
 *
 * Created on: May 28, 2026
 * Author: Lê Anh Trí Tri
 */

#include "usb.h"
#include "usbd_cdc_if.h" // Thư viện lõi của USB CDC do CubeMX sinh ra

#define RX_USB_BUFFER_SIZE 512

/* Các biến toàn cục phục vụ nhận USB */
uint16_t rx_index = 0;
bool is_message_ready = false;
char rx_buffer[RX_USB_BUFFER_SIZE];

USB_Message_t msg_usb;

/**
 * @brief Hàm Callback xử lý dữ liệu USB nhận được
 * YÊU CẦU: Phải gọi hàm này bên trong CDC_Receive_FS của file usbd_cdc_if.c
 */
void USB_Receive_Callback(uint8_t *Buf, uint32_t Len)
{
    if (is_message_ready) return; // Chờ vòng lặp chính xử lý xong tin nhắn trước

    for (uint32_t i = 0; i < Len; i++)
    {
        if (Buf[i] == '\n' || Buf[i] == '\r')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';
                is_message_ready = true;
                break; // Dừng nhận tiếp để bảo toàn gói tin
            }
        }
        else
        {
            if (rx_index < RX_USB_BUFFER_SIZE - 1)
            {
                rx_buffer[rx_index++] = (char)Buf[i];
            }
            else
            {
                rx_index = 0; // Reset khi tràn bộ đệm
            }
        }
    }
}

/**
 * @brief Tách các trường cần thiết từ chuỗi JSON
 */
static bool Parse_JSON_Data(const char* json_str, USB_Message_t* parsed_msg)
{
    if (json_str == NULL || parsed_msg == NULL) return false;

    // Reset dữ liệu mặc định
    parsed_msg->cmd_usb  = CMD_USB_NONE;
    parsed_msg->id_slave = 0;
    parsed_msg->lat      = 0.0;
    parsed_msg->lng      = 0.0;

    // 1. Tách msg_type
    if (strstr(json_str, "\"Connect\":\"Master\"") || strstr(json_str, "\"CMD_UART_CONNECT_MASTER\""))
    {
        parsed_msg->cmd_usb = CMD_USB_CONNECT_MASTER;

        char response[] = "{\"Master\":\"ACK\"}\n";
        CDC_Transmit_FS((uint8_t*)response, strlen(response));
    }
    else if (strstr(json_str, "\"CMD_UART_GPS_UPDATE\""))
    {
        parsed_msg->cmd_usb = CMD_USB_GPS_UPDATE;
    }
    else if (strstr(json_str, "\"CMD_UART_HELP\""))
    {
        parsed_msg->cmd_usb = CMD_USB_HELP;
    }
    else if (strstr(json_str, "\"CMD_UART_WARN_POINT\""))
    {
        parsed_msg->cmd_usb = CMD_USB_WARN_POINT;
    }
    else if (strstr(json_str, "\"CMD_UART_MARK_POINT\""))
    {
        parsed_msg->cmd_usb = CMD_USB_MARK_POINT;
    }
    else if (strstr(json_str, "\"CMD_UART_PING\""))
    {
        parsed_msg->cmd_usb = CMD_USB_PING;
    }
    else
    {
        return false;
    }

    // 2. Tách id_slave
    char* ptr = strstr(json_str, "\"id_slave\"");
	if (ptr) {
		ptr = strchr(ptr, ':');
		if (ptr) {
			sscanf(ptr + 1, " %hhu", &parsed_msg->id_slave);
		}
	}

    // 3. Tách lat
    ptr = strstr(json_str, "\"lat\"");
    if (ptr) {
        ptr = strchr(ptr, ':');
        if (ptr) {
            parsed_msg->lat = atof(ptr + 1);
        }
    }

    // 4. Tách lng
    ptr = strstr(json_str, "\"lng\"");
    if (ptr) {
        ptr = strchr(ptr, ':');
        if (ptr) {
            parsed_msg->lng = atof(ptr + 1);
        }
    }

    return true;
}

/**
 * @brief Xử lý chuỗi JSON sau khi nhận hoàn tất
 */
void USB_ProcessReceivedData(void)
{
    if (is_message_ready)
    {
		// 1. Tạo buffer tạm
		char temp_buffer[RX_USB_BUFFER_SIZE];

		// 2. Tạm tắt ngắt USB hoặc copy thật nhanh để tránh tranh chấp
		__disable_irq();
		strcpy(temp_buffer, rx_buffer);
		rx_index = 0;
		memset(rx_buffer, 0, RX_USB_BUFFER_SIZE); // Xóa buffer gốc luôn
		is_message_ready = false;
		__enable_irq();

        if (Parse_JSON_Data(temp_buffer, &msg_usb))
        {

        }
    }
}

/* ---------------- CÁC HÀM ĐÓNG GÓI & GỬI (MASTER -> APP) ---------------- */

void Send_JSON_Message(lora_frame_t *p_frame)
{
    char tx_buffer[256];
    const char* cmd_str;

    switch (p_frame->cmd_lora)
    {
        case CMD_LORA_TRACKING: cmd_str = "CMD_UART_TRACKING"; break;
        case CMD_LORA_SOS:      cmd_str = "CMD_UART_SOS"; break;
        case CMD_LORA_PONG:     cmd_str = "CMD_UART_PONG"; break;
        default:                cmd_str = "CMD_UART_UNKNOWN"; break;
    }

    if(strcmp(cmd_str, "CMD_UART_UNKNOWN") != 0)
    {
        snprintf(tx_buffer, sizeof(tx_buffer),
                 "\n{"
                 "\"msg_type\":\"%s\","
                 "\"id_slave\":%d,"
                 "\"data\":"
                 "{"
                 "\"lat\":%.6f,"
                 "\"lng\":%.6f,"
                 "\"temp\":%d,"
                 "\"humi\":%d,"
                 "\"bat\":%d,"
                 "\"rssi\":%d"
                 "}"
                 "}",
                 cmd_str, p_frame->id_source, p_frame->gps.lat, p_frame->gps.lng, p_frame->sensor.temp, p_frame->sensor.humi, p_frame->batt, rssi_lora);

        CDC_Transmit_FS((uint8_t*)tx_buffer, strlen(tx_buffer));
    }
}
