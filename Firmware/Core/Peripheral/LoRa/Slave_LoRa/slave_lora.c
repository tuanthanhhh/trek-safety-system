/*
 * slave.c
 *
 * Created on: May 23, 2026
 * Author: Lê Anh Trí Tri
 */

#include "slave_lora.h"

// Bộ nhớ đệm lưu trữ danh sách các gói tin (dựa trên packet_cnt) đã xử lý gần đây
static uint8_t cache_index[33] = {0};
static uint8_t seen_packet_cnt[33][CACHE_SIZE];

// Biến đếm packet_cnt nội bộ của chính Slave này (tăng dần mỗi lần tạo tin mới)
static uint8_t slave_packet_cnt = 0;
static uint16_t time_interval   = 1000;

static bool rx_flag = false;

// Hàm map ID từ 255 về slot 32
static uint8_t map_id_to_index(uint8_t id)
{
    if (id == 0xFF) return 32; // Master nằm ở slot cuối cùng
    return id; // Các Slave (0-31) nằm ở vị trí tương ứng
}

static bool is_Message_In_Cache(uint8_t origin_node_id, uint8_t packet_cnt)
{
    uint8_t mapped_id = map_id_to_index(origin_node_id);
    if (mapped_id >= 33) return true; // Vẫn giữ chốt an toàn

    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (seen_packet_cnt[mapped_id][i] == packet_cnt)
        {
            return true;
        }
    }
    return false;
}

static void save_Message_To_Cache(uint8_t origin_node_id, uint8_t packet_cnt)
{
    uint8_t mapped_id = map_id_to_index(origin_node_id);
    if (mapped_id >= 33) return;

    uint8_t idx = cache_index[mapped_id];
    seen_packet_cnt[mapped_id][idx] = packet_cnt;
    cache_index[mapped_id] = (idx + 1) % CACHE_SIZE;
}

/**
 * @brief Hàm chính gọi trong vòng lặp while(1) của Node Slave để xử lý dữ liệu đến
 */
void Slave_Process_LoRa_Frame(lora_frame_t *p_frame)
{
    if (p_frame == NULL) return;

    // 1. Kiểm tra xem gói tin này đã xử lý chưa?
    if (is_Message_In_Cache(p_frame->id_device, p_frame->packet_cnt))
    {
        return; // Nếu đã có trong Cache -> Lập tức thoát hàm
    }

    // 2. Nếu là tin mới -> Lưu vào Cache ngay lập tức để ghi nhớ cho lần sau
    save_Message_To_Cache(p_frame->id_device, p_frame->packet_cnt);

    rx_flag = true;

    // 3. THỰC THI LỆNH (Action) theo cmd_lora
    switch (p_frame->cmd_lora)
    {
        case CMD_LORA_HELP:
            break;

        case CMD_LORA_MARK_POINT:
            break;

        case CMD_LORA_WARN_POINT:
            break;

        case CMD_LORA_SOS:
            break;

        case CMD_LORA_TRACKING:

            break;

        case CMD_LORA_PING:
        	tx_lora_frame.cmd_lora = CMD_LORA_PONG;
            break;

        default:
            break;
    }

    // 4. CHUYỂN TIẾP (Relay / Hop)
    if (p_frame->hop_count > 0)
    {
    	// Tạo một khung tạm thời để chứa gói tin mượn đường
		lora_frame_t relay_frame;
		memcpy(&relay_frame, p_frame, sizeof(lora_frame_t));

		relay_frame.hop_count -= 1;
		relay_frame.id_relay = ID_DEVICE;

		// Truyền khung tạm này đi, giữ nguyên vẹn tx_lora_frame của bản thân
		LoRa_Send_Frame(&relay_frame, relay_frame.cmd_lora);
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

bool Slave_Send_New_Message()
{
	// 1. Kiểm tra trạng thái GPS để nháy LED báo hiệu (Sử dụng trực tiếp tham số truyền vào)
	if (tx_lora_frame.gps.lat != 0.0f && tx_lora_frame.gps.lng != 0.0f)
	{
		LED02_TOGGLE();
	}

	// 2. Cập nhật các thông số định danh và định tuyến
	tx_lora_frame.id_device  = ID_DEVICE; // Gốc phát là Node này
	tx_lora_frame.id_relay   = ID_DEVICE; // Bản tin mới tạo nên relay cũng là Node này

	tx_lora_frame.hop_count  = MAX_HOPS;
	slave_packet_cnt++;
	tx_lora_frame.packet_cnt = slave_packet_cnt;


	// 4. Gọi hàm gửi LoRa (Hàm này đã bao gồm việc tính lại mã CRC16)
	bool check_tx = LoRa_Send_Frame(&tx_lora_frame, tx_lora_frame.cmd_lora);

	LoRa_startReceiving(&myLoRa);

	// 5. Xử lý logic Reset lệnh sau khi gửi
	// Nếu vừa gửi lệnh PONG để phản hồi, đưa trạng thái hệ thống về lại TRACKING
	if (tx_lora_frame.cmd_lora == CMD_LORA_PONG)
	{
		tx_lora_frame.cmd_lora = CMD_LORA_TRACKING; // Cập nhật lại biến trạng thái toàn cục
	}

	return check_tx;
}

static void Lora_Frame_Test(void)
{
    // 1. Thông số định tuyến mạng (Mesh/Routing)
    tx_lora_frame.id_mesh    = ID_MESH;      // Lấy từ file .h (ví dụ 0x01)
    tx_lora_frame.id_device  = ID_DEVICE;    // Lấy từ file .h (ví dụ 0x02)
    tx_lora_frame.id_relay   = ID_DEVICE;    // Ban đầu người phát cũng là người chuyển tiếp

    tx_lora_frame.cmd_lora   = CMD_LORA_TRACKING; // Trạng thái mặc định
    tx_lora_frame.hop_count  = MAX_HOPS;     // Lấy từ file .h (ví dụ 3)
    tx_lora_frame.packet_cnt = 0;            // Bộ đếm bắt đầu từ 0

    // 2. Thông số ngoại vi (Sensor/Battery)
    tx_lora_frame.batt       = 100;          // Giả định pin đầy 100%
    tx_lora_frame.sensor.temp = 25;          // Nhiệt độ phòng 25 độ C
    tx_lora_frame.sensor.humi = 80;          // Độ ẩm 80%

    // 3. Tọa độ GPS (Vị trí mặc định - ví dụ: Bách Khoa TP.HCM)
    // Nếu chưa có sóng GPS, tọa độ mặc định giúp hệ thống không gửi về 0.0 (tọa độ rác)
    tx_lora_frame.gps.lat    = 10.7733f;
    tx_lora_frame.gps.lng    = 106.6597f;

    // 4. Reset mã kiểm tra lỗi
    tx_lora_frame.crc        = 0;
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

	Lora_Frame_Test();

	HAL_Delay(1000);
}

void Slave_Lora_Loop()
{
    #ifdef TRILE
		Slave_Process_Button();
		GPS_Process_Loop();
	#endif

	static uint32_t last_send_tick = 0;

	if (HAL_GetTick() - last_send_tick >= time_interval)
	{
		last_send_tick = HAL_GetTick();

		if(rx_flag)
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

	// Nếu có gói tin RF bay ngang qua
	if (LoRa_Read_Frame(&rx_lora_frame))
	{
		Slave_Process_LoRa_Frame(&rx_lora_frame);
	}
}
