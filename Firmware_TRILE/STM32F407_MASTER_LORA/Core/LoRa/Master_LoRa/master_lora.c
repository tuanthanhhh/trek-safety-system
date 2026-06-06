/*
 * master.c
 *
 * Created on: May 22, 2026
 * Author: Lê Anh Trí Tri
 */

#include <master_lora.h>

#define MAX_QUEUE_SIZE 5 // Giới hạn hàng đợi 5 lệnh

static USB_Message_t cached_usb;
static uint8_t send_count = 0;
static uint32_t last_send_tick = 0;
static bool is_transmitting = false;

#define MAX_SLAVES_DISPLAY 3

// Danh sách ID của 3 Slave cần theo dõi
static const uint8_t track_slave_ids[MAX_SLAVES_DISPLAY] = {0x01, 0x02, 0x03};
static uint8_t active_packet_cnt = 0;

// Bộ nhớ lưu dữ liệu, cờ kết nối và chỉ số trang hiện tại
static lora_frame_t  slave_display_data[MAX_SLAVES_DISPLAY];
static bool          slave_connected[MAX_SLAVES_DISPLAY] = {false, false, false};
static uint32_t      slave_last_recv_tick[MAX_SLAVES_DISPLAY] = {0, 0, 0};
static int8_t        current_lcd_page = 0; // Trang 0, 1 hoặc 2

static void Master_Display_LCD_Page(void)
{
    // Kiểm tra timeout mất kết nối của toàn bộ các Slave (quá 5 giây)
    for (int i = 0; i < MAX_SLAVES_DISPLAY; i++)
    {
        if (slave_connected[i] && (HAL_GetTick() - slave_last_recv_tick[i] > 5000))
        {
            slave_connected[i] = false;
        }
    }

    char lcd_line[50];
    uint8_t target_id = track_slave_ids[current_lcd_page];

    // Kiểm tra cờ kết nối của Slave ở trang hiện tại
    if (slave_connected[current_lcd_page] == true)
    {
        // Lấy dữ liệu từ bộ nhớ đệm ra in
        lora_frame_t *p_frame = &slave_display_data[current_lcd_page];

        // Dòng 0: Hiển thị ID và CMD (Căn đủ 20 ký tự)
        lcd_goto_XY(0, 0);
        snprintf(lcd_line, sizeof(lcd_line), "ID:%02X        CMD:%02X", target_id, p_frame->cmd_lora);
        lcd_send_string(lcd_line);

        lcd_goto_XY(1, 0);
        snprintf(lcd_line, sizeof(lcd_line), "LA:%-9.4f R:%-4d ", p_frame->gps.lat, rssi_lora);
        lcd_send_string(lcd_line);

        lcd_goto_XY(2, 0);
        snprintf(lcd_line, sizeof(lcd_line), "LN:%-9.4f PK:%-3d", p_frame->gps.lng, p_frame->packet_cnt);
        lcd_send_string(lcd_line);

        lcd_goto_XY(3, 0);
        snprintf(lcd_line, sizeof(lcd_line), "T:%2dC H:%2d%%  B:%3d%%", p_frame->sensor.temp, p_frame->sensor.humi, p_frame->batt);
        lcd_send_string(lcd_line);
    }
    else
    {
        // Dòng 0: Hiển thị ID, CMD để trống do chưa có kết nối
        lcd_goto_XY(0, 0);
        snprintf(lcd_line, sizeof(lcd_line), "ID:%02X                 ", target_id);
        lcd_send_string(lcd_line);

        // Xóa trắng dòng 1
        lcd_goto_XY(1, 0);
        lcd_send_string("                    ");

        // Dòng 2: Hiển thị trạng thái chờ kết nối
        lcd_goto_XY(2, 0);
        snprintf(lcd_line, sizeof(lcd_line), "  ID %02X Connecting ", target_id);
        lcd_send_string(lcd_line);

        // Xóa trắng dòng 3
        lcd_goto_XY(3, 0);
        lcd_send_string("                    ");
    }
}

static void Master_Process_LoRa_Frame(lora_frame_t *p_frame)
{
    if (p_frame == NULL) return;

    if (p_frame->cmd_lora == CMD_LORA_TRACKING || p_frame->cmd_lora == CMD_LORA_SOS || p_frame->cmd_lora == CMD_LORA_PONG)
    {
        Send_JSON_Message(p_frame);

        for (int i = 0; i < MAX_SLAVES_DISPLAY; i++)
        {
            if (p_frame->id_source == track_slave_ids[i])
            {
                // Lưu nguyên bản cấu trúc vào bộ nhớ đệm của trang tương ứng
                memcpy(&slave_display_data[i], p_frame, sizeof(lora_frame_t));

                // Bật cờ trạng thái đã kết nối và lưu thời gian nhận
                slave_connected[i] = true;
                slave_last_recv_tick[i] = HAL_GetTick();
                break;
            }
        }

        // Nếu nhận được phản hồi PONG -> lập tức hủy tiến trình gửi PING/truyền lại
        if (p_frame->cmd_lora == CMD_LORA_PONG)
        {
            is_transmitting = false;           // Tắt cờ tiến trình
            send_count = 0;                    // Reset bộ đếm truyền lại
            LED02_OFF();                       // Tắt LED báo hiệu đang truyền
            cached_usb.cmd_usb = CMD_USB_NONE; // Dọn dẹp lệnh cũ trong cache USB
        }
    }
}

void Master_Process_Buttons(void)
{
    // Biến static lưu thời gian để tạo chu kỳ quét
    static uint32_t last_btn_tick = 0;

    // Sử dụng thuật toán kiểm tra chu kỳ 50ms (Debounce bằng phần mềm)
    if (HAL_GetTick() - last_btn_tick >= 50)
    {
        last_btn_tick = HAL_GetTick();

        // Duyệt qua toàn bộ các nút nhấn đã định nghĩa trong mảng Buttons
        for (int i = 0; i < BTN_MAX_COUNT; i++)
        {
            // Đọc trạng thái vật lý hiện tại của chân GPIO
            GPIO_PinState current_state = HAL_GPIO_ReadPin(Buttons[i].port, Buttons[i].pin);

            // Điều kiện: Trạng thái hiện tại là LOW (đang nhấn) và trước đó (ref) là HIGH (vừa mới nhấn xuống)
            if (current_state == GPIO_PIN_RESET && Buttons[i].ref == GPIO_PIN_SET)
            {
                // Xử lý logic tương ứng với từng nút
                switch (i)
                {
                    case BTN_LEFT:
                        current_lcd_page--;
                        if (current_lcd_page < 0)
                        {
                            current_lcd_page = MAX_SLAVES_DISPLAY - 1; // Cuốn vòng về trang cuối
                        }
                        Master_Display_LCD_Page();
                        break;

                    case BTN_RIGHT:
                        current_lcd_page++;
                        if (current_lcd_page >= MAX_SLAVES_DISPLAY)
                        {
                            current_lcd_page = 0; // Cuốn vòng về trang đầu
                        }
                        Master_Display_LCD_Page();
                        break;

                    case BTN_SELECT:
                        // Đặt code thực thi cho nút Select vào đây
                        break;

                    default:
                        break;
                }
            }

            // Lưu lại trạng thái của chu kỳ này để làm tham chiếu cho chu kỳ 50ms tiếp theo
            Buttons[i].ref = current_state;
        }
    }
}

void Master_Lora_Init(SPI_HandleTypeDef *hspi,
                      GPIO_TypeDef *cs_port, uint16_t cs_pin,
                      GPIO_TypeDef *rst_port, uint16_t rst_pin,
                      GPIO_TypeDef *dio0_port, uint16_t dio0_pin)
{
    lcd_init();
    lcd_goto_XY(0, 0);
    lcd_send_string("=== MASTER_LORA ===");

    LED01_ON();
    LED02_OFF();
    LED03_OFF();

    Lora_Init(hspi, cs_port, cs_pin, rst_port, rst_pin, dio0_port, dio0_pin);

    lcd_goto_XY(1, 0);
    lcd_send_string("[LORA]: INIT_OK");

    HAL_Delay(1000);
}

bool Master_Send_USB_Message(USB_Message_t *p_frame)
{
    if (p_frame == NULL) return false;

    lora_frame_t tx_frame;

    // 1. Dọn sạch rác RAM trong biến cục bộ trước khi nạp dữ liệu
    memset(&tx_frame, 0, sizeof(lora_frame_t));

    // 2. Nạp thông số mạng bắt buộc
    tx_frame.id_mesh   = ID_MESH;  // Để Slave nhận diện đúng mạng
    tx_frame.id_source = ID_DEVICE;
    tx_frame.id_dest   = p_frame->id_slave;
    tx_frame.hop_count = 5;

    active_packet_cnt++;
    if (active_packet_cnt >= 254) active_packet_cnt = 1;
    tx_frame.packet_cnt = active_packet_cnt;

    switch (p_frame->cmd_usb)
    {
        case CMD_USB_HELP:
            tx_frame.cmd_lora = CMD_LORA_HELP;
            break;
        case CMD_USB_WARN_POINT:
            tx_frame.cmd_lora = CMD_LORA_WARN_POINT;
            break;
        case CMD_USB_MARK_POINT:
            tx_frame.cmd_lora = CMD_LORA_MARK_POINT;
            break;
        case CMD_USB_PING:
            tx_frame.cmd_lora = CMD_LORA_PING;
            break;
        default:
            return false;
    }

    tx_frame.gps.lat = p_frame->lat;
    tx_frame.gps.lng = p_frame->lng;

    tx_frame.sensor.temp = 0;
    tx_frame.sensor.humi = 0;
    tx_frame.batt = 0;

    bool tx_result = LoRa_Send_Frame(&tx_frame);

    LoRa_startReceiving(&myLoRa);

    return tx_result;
}


static void Master_Process_USB_Frame(USB_Message_t *p_frame)
{
    // ========================================================
    // BIẾN TĨNH CHO HÀNG ĐỢI
    // ========================================================
    static USB_Message_t cmd_queue[MAX_QUEUE_SIZE];
    static uint8_t q_head = 0;
    static uint8_t q_tail = 0;
    static uint8_t q_count = 0;

    // --------------------------------------------------------
    // KHỐI 1: NHẬN LỆNH MỚI VÀ XẾP VÀO HÀNG ĐỢI
    // --------------------------------------------------------
    if (p_frame->cmd_usb == CMD_USB_HELP || p_frame->cmd_usb == CMD_USB_WARN_POINT ||
        p_frame->cmd_usb == CMD_USB_MARK_POINT || p_frame->cmd_usb == CMD_USB_PING)
    {
        if (q_count < MAX_QUEUE_SIZE)
        {
            // Copy vào cuối hàng đợi
            memcpy(&cmd_queue[q_tail], p_frame, sizeof(USB_Message_t));
            q_tail = (q_tail + 1) % MAX_QUEUE_SIZE;
            q_count++;
        }

        // Xóa lệnh gốc
        p_frame->cmd_usb = CMD_USB_NONE;
    }
    else if (p_frame->cmd_usb != CMD_USB_NONE)
    {
        LED02_OFF();
        p_frame->cmd_usb = CMD_USB_NONE; // Dọn dẹp lệnh rác
    }


    // --------------------------------------------------------
    // KHỐI 2: LẤY LỆNH TỪ HÀNG ĐỢI RA CHẠY
    // --------------------------------------------------------
    if (!is_transmitting && q_count > 0)
    {
        // Lấy lệnh đầu hàng nạp vào cached_usb
        memcpy(&cached_usb, &cmd_queue[q_head], sizeof(USB_Message_t));
        q_head = (q_head + 1) % MAX_QUEUE_SIZE;
        q_count--;

        is_transmitting = true;
        send_count = 0;
        last_send_tick = HAL_GetTick() - 1000;
    }

    // --------------------------------------------------------
    // KHỐI 3: TRUYỀN QUA LORA
    // --------------------------------------------------------
    if (is_transmitting)
    {
        static int time_send = 500;

        if (HAL_GetTick() - last_send_tick >= time_send)
        {
            last_send_tick = HAL_GetTick();
            send_count++;

            // Sử dụng cached_usb đang chứa lệnh hiện tại
            if (Master_Send_USB_Message(&cached_usb))
            {
                LED02_TOGGLE();
            }

            if (send_count >= (10000 / time_send))
            {
                is_transmitting = false;
                send_count = 0;
                LED02_OFF();
                cached_usb.cmd_usb = CMD_USB_NONE;
            }
        }
    }
}

void Master_Lora_Loop()
{
    USB_ProcessReceivedData();
    Master_Process_Buttons();
    Master_Display_LCD_Page();

    if (LoRa_Read_Frame(&rx_lora_frame))
    {
        Master_Process_LoRa_Frame(&rx_lora_frame);
        LED01_TOGGLE();
    }

    Master_Process_USB_Frame(&msg_usb);
}


