/*
 * slave.c
 *
 * Created on: May 23, 2026
 * Author: Lê Anh Trí Tri
 */

#include "slave_lora.h"
#include "gps_gp02.h"
#include "../../../Function/Function_Help.h"
#include "../../../Function/Function_Gather.h"

// ----- CẤU HÌNH THÔNG SỐ MESH CHO SLAVE -----
#define CACHE_SIZE 10        // Số lượng gói tin gần nhất được lưu để chống lặp
#define MAX_HOPS   5         // Số bước nhảy tối đa cho một gói tin (Time-To-Live)

#define SLAVE_NUM  250
#define MAX_REPEAT 5

// Bảng lưu vị trí GPS của các Node khác
Other_Node_GPS_t other_nodes_gps[256] = {0};

// Bộ nhớ đệm lưu trữ danh sách các gói tin (dựa trên packet_cnt) đã xử lý gần đây
static uint8_t cache_index[SLAVE_NUM] = {0};
static uint8_t seen_packet_cnt[SLAVE_NUM][CACHE_SIZE];
static uint8_t repeat_packet_cnt[SLAVE_NUM][CACHE_SIZE];

// Biến đếm packet_cnt nội bộ của chính Slave này (tăng dần mỗi lần tạo tin mới)
#if (ID_DEVICE == 2)
	static uint16_t time_interval    = 900;
#else
	static uint16_t time_interval    = 1000;
#endif

static bool rx_flag       = false;
static bool response_flag = false;

// Phân loại trạng thái gói tin khi đi qua bộ đệm
typedef enum
{
    PACKET_NEW = 0,     // Gói tin mới hoàn toàn -> Cần thực thi lệnh & Chuyển tiếp
    PACKET_REPEAT,      // Gói tin cũ, chưa vượt MAX_REPEAT -> Bỏ qua lệnh, CHỈ Chuyển tiếp
    PACKET_DROP         // Gói tin cũ, đã vượt MAX_REPEAT -> Hủy hoàn toàn

} Packet_Status_t;

/**
 * @brief Kiểm tra và tự động cập nhật bộ đệm lịch sử gói tin
 */
static Packet_Status_t Check_And_Update_Cache(lora_frame_t *p_frame)
{
	if (p_frame == NULL|| p_frame->id_source >= SLAVE_NUM || p_frame->id_source == ID_DEVICE) return PACKET_DROP;

    // 1. Kiểm tra xem gói tin đã tồn tại trong lịch sử chưa
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (seen_packet_cnt[p_frame->id_source][i] == p_frame->packet_cnt)
        {
            if (repeat_packet_cnt[p_frame->id_source][i] < MAX_REPEAT)
            {
                repeat_packet_cnt[p_frame->id_source][i]++; // Tăng số lần lặp
                return PACKET_REPEAT;
            }
            return PACKET_DROP; // Vượt quá số lần cho phép
        }
    }

    // 2. Nếu quét hết mảng không thấy -> Đây là gói tin mới.
    // Tự động ghi vào bộ đệm vòng ngay tại đây.
    uint8_t idx = cache_index[p_frame->id_source];
    seen_packet_cnt[p_frame->id_source][idx] = p_frame->packet_cnt;
    repeat_packet_cnt[p_frame->id_source][idx] = 1; // Khởi tạo số lần lặp

    cache_index[p_frame->id_source] = (idx + 1) % CACHE_SIZE;

    return PACKET_NEW;
}

/**
 * @brief Hàm chính gọi trong vòng lặp while(1) của Node Slave để xử lý dữ liệu đến
 */
void Slave_Process_LoRa_Frame(lora_frame_t *p_frame)
{
    // Phân loại gói tin thông qua Cache
    Packet_Status_t status = Check_And_Update_Cache(p_frame);

    // NẾU LÀ GÓI TIN RÁC HOẶC ĐÃ LẶP QUÁ GIỚI HẠN -> LOẠI BỎ
    if (status == PACKET_DROP)
    {
        return;
    }

    rx_flag = true;

    // Lưu lại GPS của Node khác nếu gói tin chứa tọa độ hợp lệ
    if (p_frame->id_source != ID_DEVICE && p_frame->id_source < 256)
    {
        if (p_frame->gps.lat != 0.0f && p_frame->gps.lng != 0.0f)
        {
            other_nodes_gps[p_frame->id_source].lat = p_frame->gps.lat;
            other_nodes_gps[p_frame->id_source].lng = p_frame->gps.lng;
            other_nodes_gps[p_frame->id_source].last_seen_tick = HAL_GetTick();
            other_nodes_gps[p_frame->id_source].is_valid = true;
        }
    }

    switch (p_frame->cmd_lora)
    {
        case CMD_LORA_HELP:
            Function_Help_Trigger(p_frame);
            break;
        case CMD_LORA_MARK_POINT:
            Function_Gather_Trigger(p_frame);
            break;
        case CMD_LORA_WARN_POINT:
            break;
        case CMD_LORA_SOS:
            if (p_frame->id_source != ID_DEVICE)
            {
                Function_Help_Trigger(p_frame);
            }
            break;
        case CMD_LORA_TRACKING:
            Function_Help_Clear_Ignore(p_frame->id_source);
            break;
        case CMD_LORA_PING:
            if (p_frame->id_dest == ID_DEVICE)
            {
                response_flag = true;
            }
            break;
        default:
            break;
    }

    // CHUYỂN TIẾP (Relay / Hop)
    // Cả gói tin MỚI và gói tin LẶP (REPEAT) đều sẽ chạy vào khối này nếu hop_count > 0
    if (p_frame->hop_count > 0)
    {
        lora_frame_t relay_frame;
        memcpy(&relay_frame, p_frame, sizeof(lora_frame_t));

        relay_frame.hop_count -= 1;

        LoRa_Send_Frame(&relay_frame);
        LoRa_startReceiving(&myLoRa);
    }
}

#ifdef TRILE
	void Slave_Process_Button(void)
	{
		// Quét qua toàn bộ các nút trong mảng
		for (int i = 0; i < BTN_MAX_COUNT; i++)
		{
			// Nếu phát hiện có cờ được bật
			if (Buttons[i].flag)
			{
				// Xóa cờ ngay lập tức để chờ lần nhấn tiếp theo
				Buttons[i].flag = false;

				// Phân loại xử lý dựa trên ID của nút (chính là biến i)
				switch (i)
				{
					case BTN_SOS:
						tx_lora_frame.cmd_lora = CMD_LORA_SOS;
						time_interval = 200;
						break;

					case BTN_SELECT:
						tx_lora_frame.cmd_lora = CMD_LORA_TRACKING;
						time_interval = 1000;
						break;

					case BTN_UP:
						// Thêm code xử lý cho nút UP
						break;

					case BTN_DOWN:
						// Thêm code xử lý cho nút DOWN
						break;

					case BTN_LEFT:
						// Thêm code xử lý cho nút LEFT
						break;

					case BTN_RIGHT:
						// Thêm code xử lý cho nút RIGHT
						break;

					default:
						break;
				}
			}
		}
	}
#endif

bool Slave_Send_New_Message(void)
{
    // 1. Kiểm tra trạng thái GPS để nháy LED báo hiệu
    if (current_gps.is_valid)
    {
        LED02_TOGGLE();
    }
    else
    {
        LED02_OFF();
    }

    // 2. Cập nhật các thông số định danh và định tuyến
    tx_lora_frame.id_mesh    = ID_MESH;
    tx_lora_frame.id_source  = ID_DEVICE; // Gốc phát là Node này
    tx_lora_frame.id_dest    = 0;

    tx_lora_frame.hop_count  = MAX_HOPS;
    tx_lora_frame.packet_cnt++;

    if (tx_lora_frame.packet_cnt == 255)
    {
        tx_lora_frame.packet_cnt = 0;
    }

    if (response_flag)
    {
        response_flag = false;
        tx_lora_frame.cmd_lora = CMD_LORA_PONG;
    }

    // 3. Gọi hàm gửi LoRa (bao gồm tính CRC16)
    bool check_tx = LoRa_Send_Frame(&tx_lora_frame);

    LoRa_startReceiving(&myLoRa);

    // 4. Reset lệnh về trạng thái TRACKING sau khi gửi PONG phản hồi
    if (tx_lora_frame.cmd_lora == CMD_LORA_PONG)
    {
        tx_lora_frame.cmd_lora = CMD_LORA_TRACKING;
    }

    return check_tx;
}

void Slave_Lora_Init(SPI_HandleTypeDef *hspi,
        GPIO_TypeDef *cs_port, uint16_t cs_pin,
        GPIO_TypeDef *rst_port, uint16_t rst_pin,
        GPIO_TypeDef *dio0_port, uint16_t dio0_pin)
{
	LED01_ON();
	LED02_ON();
	LED03_OFF();

	memset(seen_packet_cnt, 0xFF, sizeof(seen_packet_cnt)); // 0xFF: Chưa có tin nào

    #ifdef TRILE
		GPS_Init();
    #endif

	Lora_Init(hspi,
			cs_port, cs_pin,
			rst_port, rst_pin,
			dio0_port, dio0_pin);

	HAL_Delay(1000);
}

void Slave_Lora_Loop(void)
{
#ifdef TRILE
    Slave_Process_Button();
    GPS_Process_Loop();
#endif

    static uint32_t last_send_tick = 0;

    if (HAL_GetTick() - last_send_tick >= time_interval)
    {
        last_send_tick = HAL_GetTick();

        if (rx_flag)
        {
            rx_flag = false;
            LED03_TOGGLE();
        }
        else
        {
            LED03_OFF();
        }

        if (Slave_Send_New_Message())
        {
            LED01_TOGGLE();
        }
    }

    // Nếu có gói tin RF nhận được
    if (LoRa_Read_Frame(&rx_lora_frame))
    {
        Slave_Process_LoRa_Frame(&rx_lora_frame);
    }
}
