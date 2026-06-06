/* ============================================================
   serial.js — Web Serial API Communication Module
   ============================================================
   Quản lý toàn bộ giao tiếp Serial giữa Web App và Master MCU.
   
   LUỒNG GIAO TIẾP:
   1. Người dùng nhấn "Kết nối" → requestPort() → open()
   2. Gửi handshake: {"Connect":"Master"}
   3. Chờ phản hồi:  {"Master":"ACK"}
   4. Nếu OK → trạng thái "connected", bắt đầu nhận data
   
   CÁC GÓI TIN GỬI XUỐNG MASTER (Web → MCU):
   - CMD_UART_GPS_UPDATE  : Cập nhật tọa độ GPS cho Master
   - CMD_UART_HELP        : Yêu cầu hỗ trợ Slave đang gặp nạn
   - CMD_UART_WARN_POINT  : Đánh dấu điểm nguy hiểm
   - CMD_UART_MARK_POINT  : Đánh dấu điểm tập kết/đích đến
   - CMD_UART_PING        : Kiểm tra kết nối Slave
   
   CÁC GÓI TIN NHẬN TỪ MASTER (MCU → Web):
   - CMD_UART_TRACKING : Dữ liệu định kỳ từ Slave (lat,lng,temp,humi,bat,rssi)
   - CMD_UART_SOS      : Tín hiệu SOS từ Slave
   - CMD_UART_PONG     : Phản hồi Ping
   ============================================================ */

const SerialManager = (() => {
    // --- Private State ---
    let port = null;           // Web Serial port object
    let reader = null;         // ReadableStream reader
    let writer = null;         // WritableStream writer
    let readLoop = null;       // read loop reference
    let isConnected = false;
    let buffer = '';           // Buffer dữ liệu nhận (tích lũy cho đến khi có JSON hoàn chỉnh)

    // Callbacks - sẽ được gán từ app.js
    let onStatusChange = null;   // (status: 'disconnected'|'connecting'|'connected') => void
    let onDataReceived = null;   // (parsedJSON: Object) => void
    let onLog = null;            // (message: string, type: 'info'|'rx'|'tx'|'error') => void

    // --- Encoder/Decoder ---
    const encoder = new TextEncoder();
    const decoder = new TextDecoder();

    // ---------------------------------------------------------------
    // PUBLIC: Đăng ký callbacks
    // ---------------------------------------------------------------
    function setCallbacks({ statusChange, dataReceived, log }) {
        onStatusChange = statusChange || null;
        onDataReceived = dataReceived || null;
        onLog = log || null;
    }

    // ---------------------------------------------------------------
    // PUBLIC: Kết nối Serial Port
    // Bước 1: requestPort() → chọn cổng COM
    // Bước 2: open() với baudRate 115200
    // Bước 3: Gửi handshake {"Connect":"Master"}
    // Bước 4: Chờ phản hồi {"Master":"ACK"}
    // ---------------------------------------------------------------
    async function connect() {
        try {
            // Kiểm tra trình duyệt hỗ trợ Web Serial
            if (!('serial' in navigator)) {
                _log('Trình duyệt không hỗ trợ Web Serial API. Hãy dùng Chrome/Edge.', 'error');
                return false;
            }

            _setStatus('connecting');
            _log('Đang yêu cầu chọn cổng COM...', 'info');

            // Bước 1: Người dùng chọn cổng COM từ dialog trình duyệt
            port = await navigator.serial.requestPort();

            // Bước 2: Mở cổng Serial - baudRate 115200 (phù hợp với MCU)
            await port.open({ baudRate: 115200 });
            _log('Đã mở cổng Serial (115200 baud)', 'info');

            // Lấy writer để gửi dữ liệu
            writer = port.writable.getWriter();

            // Bước 3: Gửi handshake xuống Master
            const handshakeMsg = JSON.stringify({ Connect: "Master" });
            await _sendRaw(handshakeMsg);
            _log(`Gửi handshake: ${handshakeMsg}`, 'tx');

            // Bước 4: Bắt đầu đọc - chờ ACK
            _startReading();

            // Timeout cho handshake (5 giây)
            const ackTimeout = setTimeout(() => {
                if (!isConnected) {
                    _log('Handshake timeout - không nhận được ACK từ Master', 'error');
                    _setStatus('disconnected');
                }
            }, 5000);

            // Lưu timeout để có thể clear khi nhận ACK
            window._ackTimeout = ackTimeout;

            return true;
        } catch (err) {
            if (err.name === 'NotFoundError') {
                _log('Người dùng đã hủy chọn cổng COM', 'info');
            } else {
                _log(`Lỗi kết nối: ${err.message}`, 'error');
            }
            _setStatus('disconnected');
            return false;
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Ngắt kết nối
    // ---------------------------------------------------------------
    async function disconnect() {
        try {
            isConnected = false;
            if (reader) {
                await reader.cancel();
                reader.releaseLock();
                reader = null;
            }
            if (writer) {
                writer.releaseLock();
                writer = null;
            }
            if (port) {
                await port.close();
                port = null;
            }
            buffer = '';
            _setStatus('disconnected');
            _log('Đã ngắt kết nối Serial', 'info');
        } catch (err) {
            _log(`Lỗi ngắt kết nối: ${err.message}`, 'error');
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Gửi gói tin JSON xuống Master
    // @param {Object} jsonObj - Object JSON cần gửi
    // 
    // VÍ DỤ:
    //   sendJSON({ msg_type: "CMD_GPS_UPDATE", id_slave: "00",
    //              data: { lat: 12.23, lng: 109.19 } })
    // ---------------------------------------------------------------
    async function sendJSON(jsonObj) {
        if (!isConnected || !writer) {
            _log('Không thể gửi - chưa kết nối', 'error');
            return false;
        }
        try {
            const jsonStr = JSON.stringify(jsonObj);
            await _sendRaw(jsonStr);
            _log(`${jsonStr}`, 'tx');
            return true;
        } catch (err) {
            _log(`Lỗi gửi dữ liệu: ${err.message}`, 'error');
            return false;
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Kiểm tra trạng thái kết nối
    // ---------------------------------------------------------------
    function getConnectionStatus() {
        return isConnected;
    }

    // ---------------------------------------------------------------
    // PRIVATE: Gửi chuỗi raw qua Serial (thêm newline \n làm delimiter)
    // MCU sẽ đọc cho đến khi gặp \n để parse JSON
    // ---------------------------------------------------------------
    async function _sendRaw(str) {
        if (writer) {
            await writer.write(encoder.encode(str + '\n'));
        }
    }

    // ---------------------------------------------------------------
    // PRIVATE: Vòng lặp đọc dữ liệu liên tục từ Serial
    // Dữ liệu nhận được tích lũy vào buffer,
    // khi phát hiện ký tự newline → tách ra và parse JSON
    // ---------------------------------------------------------------
    async function _startReading() {
        reader = port.readable.getReader();
        try {
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    _log('Serial reader đã đóng', 'info');
                    break;
                }
                if (value) {
                    // Decode bytes nhận được thành chuỗi
                    const chunk = decoder.decode(value, { stream: true });
                    buffer += chunk;

                    // Tách buffer theo newline, mỗi dòng là 1 JSON message
                    const lines = buffer.split('\n');
                    // Giữ lại phần chưa hoàn chỉnh (dòng cuối không có \n)
                    buffer = lines.pop();

                    for (const line of lines) {
                        const trimmed = line.trim();
                        if (trimmed) {
                            _processIncoming(trimmed);
                        }
                    }
                }
            }
        } catch (err) {
            if (err.name !== 'NetworkError' && err.message !== 'The device has been lost.') {
                _log(`Lỗi đọc Serial: ${err.message}`, 'error');
            }
        } finally {
            if (reader) {
                reader.releaseLock();
                reader = null;
            }
            if (isConnected) {
                isConnected = false;
                _setStatus('disconnected');
                _log('Mất kết nối với thiết bị', 'error');
            }
        }
    }

    // ---------------------------------------------------------------
    // PRIVATE: Xử lý một dòng dữ liệu nhận được từ Serial
    // 
    // LUỒNG XỬ LÝ:
    //   1. Parse JSON
    //   2. Nếu là ACK handshake → xác nhận kết nối
    //   3. Nếu là data packet → forward lên app.js qua callback
    //
    // ĐỊNH DẠNG NHẬN TỪ MASTER:
    //   Handshake ACK:  {"Master":"ACK"}
    //   Data thường:    {"msg_type":"0x00","id_slave":"01","data":{...}}
    //   SOS:            {"msg_type":"CMD_SOS","id_slave":"02","data":{...}}
    // ---------------------------------------------------------------
    function _processIncoming(rawStr) {
        _log(`${rawStr}`, 'rx');

        try {
            const json = JSON.parse(rawStr);

            // --- Xử lý Handshake ACK ---
            // Khi Master phản hồi {"Master":"ACK"} → kết nối thành công
            if (json.Master === 'ACK') {
                isConnected = true;
                if (window._ackTimeout) {
                    clearTimeout(window._ackTimeout);
                }
                _setStatus('connected');
                _log('✓ Handshake thành công - Kết nối với Master!', 'info');
                return;
            }

            // --- Forward data lên tầng ứng dụng ---
            // Gồm: dữ liệu định kỳ (0x00) và SOS (CMD_SOS)
            if (json.msg_type && onDataReceived) {
                // Nếu nhận được data hợp lệ nhưng chưa connected
                // (ví dụ: Master không gửi ACK nhưng vẫn gửi data)
                // → tự động coi là đã kết nối
                if (!isConnected) {
                    isConnected = true;
                    if (window._ackTimeout) {
                        clearTimeout(window._ackTimeout);
                    }
                    _setStatus('connected');
                    _log('✓ Tự động kết nối - nhận được dữ liệu từ Master', 'info');
                }
                onDataReceived(json);
            }
        } catch (err) {
            // Dữ liệu không phải JSON hợp lệ - bỏ qua hoặc log
            _log(`Dữ liệu không hợp lệ: ${rawStr}`, 'error');
        }
    }

    // ---------------------------------------------------------------
    // PRIVATE: Cập nhật trạng thái kết nối
    // ---------------------------------------------------------------
    function _setStatus(status) {
        if (onStatusChange) {
            onStatusChange(status);
        }
    }

    // ---------------------------------------------------------------
    // PRIVATE: Ghi log
    // ---------------------------------------------------------------
    function _log(msg, type) {
        if (onLog) {
            onLog(msg, type);
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Gửi chuỗi raw text qua Serial (không qua JSON.stringify)
    // Dùng cho test/debug
    // ---------------------------------------------------------------
    async function sendRawText(text) {
        if (!writer) {
            _log('Không thể gửi - chưa có writer (chưa kết nối)', 'error');
            return false;
        }
        try {
            await _sendRaw(text);
            _log(`${text}`, 'tx');
            return true;
        } catch (err) {
            _log(`Lỗi gửi dữ liệu: ${err.message}`, 'error');
            return false;
        }
    }

    // --- Public API ---
    return {
        setCallbacks,
        connect,
        disconnect,
        sendJSON,
        sendRawText,
        getConnectionStatus
    };
})();
