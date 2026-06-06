/* ============================================================
   app.js — Application Controller
   ============================================================
   Điều phối chính: liên kết Serial module, Map module với UI.
   
   CHỨC NĂNG:
   1. Quản lý kết nối Serial (handshake, trạng thái)
   2. Xử lý dữ liệu nhận từ Master (0x00, CMD_SOS)
   3. Quản lý danh sách Node trên sidebar
   4. Điều khiển các dialog (Node Detail, Gather, SOS Alert)
   5. Gửi lệnh xuống Master (CMD_GPS_UPDATE, CMD_HELP, CMD_GATHER)
   ============================================================ */

const AppController = (() => {
    // --- State ---
    // Lưu trữ dữ liệu tất cả các Node Slave
    // Key: id_slave (string), Value: { lat, lng, temp, humi, bat, isSOS, lastUpdate }
    const nodes = {};
    let selectedNodeId = null;    // Node đang được chọn để xem chi tiết
    let currentSOSNodeId = null;  // Node đang SOS (hiển thị trên overlay)
    let masterLocation = null;    // Vị trí Master { lat, lng }
    const acknowledgedSOS = new Set(); // Các Node SOS đã được xác nhận (không popup lại)
    const pendingPings = new Set();  // nodeIds đang chờ PONG (1 PING → 1 PONG)

    // --- SOS Audio (Web Audio API) ---
    let audioCtx = null;
    let sosOscillator = null;

    // --- DOM Elements ---
    const DOM = {};

    // ---------------------------------------------------------------
    // Khởi tạo ứng dụng
    // ---------------------------------------------------------------
    function init() {
        _cacheDOMElements();
        _bindEvents();
        _setupSerial();
        _startClock();
        // Khởi tạo bản đồ Leaflet (không cần callback async như Google Maps)
        MapManager.init();
    }

    // ---------------------------------------------------------------
    // Cache các DOM element thường dùng
    // ---------------------------------------------------------------
    function _cacheDOMElements() {
        DOM.connectBtn = document.getElementById('connectBtn');
        DOM.connectionStatus = document.getElementById('connectionStatus');
        DOM.masterCoords = document.getElementById('masterCoords');
        DOM.nodeList = document.getElementById('nodeList');
        DOM.nodeCount = document.getElementById('nodeCount');
        DOM.serialLog = document.getElementById('serialLog');
        DOM.logClearBtn = document.getElementById('logClearBtn');

        // Map status bar
        DOM.mapStatusConn = document.getElementById('mapStatusConn');
        DOM.mapStatusNodes = document.getElementById('mapStatusNodes');
        DOM.mapStatusTime = document.getElementById('mapStatusTime');

        // SOS Overlay
        DOM.sosOverlay = document.getElementById('sosOverlay');
        DOM.sosAlertText = document.getElementById('sosAlertText');
        DOM.sosCoords = document.getElementById('sosCoords');
        DOM.sosLocateBtn = document.getElementById('sosLocateBtn');
        DOM.sosDismissBtn = document.getElementById('sosDismissBtn');

        // Map Action Dialog
        DOM.mapActionDialog = document.getElementById('mapActionDialog');
        DOM.mapActionCoords = document.getElementById('mapActionCoords');
        DOM.mapActionCloseBtn = document.getElementById('mapActionCloseBtn');
        DOM.btnMarkPoint = document.getElementById('btnMarkPoint');
        DOM.btnWarnPoint = document.getElementById('btnWarnPoint');

        // Node Detail Dialog
        DOM.nodeDetailDialog = document.getElementById('nodeDetailDialog');
        DOM.nodeDetailClose = document.getElementById('nodeDetailClose');
        DOM.nodeDetailAvatar = document.getElementById('nodeDetailAvatar');
        DOM.nodeDetailTitle = document.getElementById('nodeDetailTitle');
        DOM.nodeDetailStatus = document.getElementById('nodeDetailStatus');
        DOM.nodeDetailTemp = document.getElementById('nodeDetailTemp');
        DOM.nodeDetailHumi = document.getElementById('nodeDetailHumi');
        DOM.nodeDetailBat = document.getElementById('nodeDetailBat');
        DOM.nodeDetailCoords = document.getElementById('nodeDetailCoords');
        DOM.nodeDetailRssi = document.getElementById('nodeDetailRssi');
        DOM.nodeHelpBtn = document.getElementById('nodeHelpBtn');
        DOM.nodePingBtn = document.getElementById('nodePingBtn');
        DOM.nodeSosAckBtn = document.getElementById('nodeSosAckBtn');
    }

    // ---------------------------------------------------------------
    // Bind sự kiện UI
    // ---------------------------------------------------------------
    function _bindEvents() {
        // Nút kết nối Serial
        DOM.connectBtn.addEventListener('click', _handleConnect);

        // Xóa log
        DOM.logClearBtn.addEventListener('click', () => {
            DOM.serialLog.innerHTML = '<div class="log-entry log-info">Log đã xóa</div>';
        });

        // SOS Overlay
        DOM.sosLocateBtn.addEventListener('click', _handleSOSLocate);
        DOM.sosDismissBtn.addEventListener('click', _handleSOSDismiss);

        // Map Action Dialog
        DOM.mapActionCloseBtn.addEventListener('click', () => {
            DOM.mapActionDialog.classList.add('hidden');
        });

        // Node Detail Dialog
        DOM.nodeDetailClose.addEventListener('click', () => {
            DOM.nodeDetailDialog.classList.add('hidden');
        });
        DOM.nodeHelpBtn.addEventListener('click', _handleSendHelp);
        DOM.nodePingBtn.addEventListener('click', _handleSendPing);
        DOM.nodeSosAckBtn.addEventListener('click', _handleSOSAck);
    }

    // ---------------------------------------------------------------
    // Cấu hình Serial callbacks
    // ---------------------------------------------------------------
    function _setupSerial() {
        SerialManager.setCallbacks({
            statusChange: _onSerialStatusChange,
            dataReceived: _onSerialDataReceived,
            log: _addLog
        });

        // Cấu hình Map callbacks
        MapManager.setCallbacks({
            mapClick: _onMapClick,
            markerClick: _onMarkerClick
        });
    }

    // ---------------------------------------------------------------
    // CALLBACK: Khi vị trí Master (GPS máy tính) sẵn sàng / cập nhật
    // Được gọi từ map.js liên tục khi Geolocation API cập nhật
    // → Gửi CMD_GPS_UPDATE xuống Master MCU
    //
    // @param {Object|null} location  - { lat, lng } hoặc null khi đang tìm
    // @param {string} status         - 'searching' | 'ok' | 'error'
    // ---------------------------------------------------------------
    function onMasterLocationReady(location, status = 'ok') {
        if (status === 'searching') {
            DOM.masterCoords.textContent = 'Đang tìm vị trí GPS...';
            DOM.masterCoords.style.color = 'var(--orange)';
            return;
        }

        if (status === 'error') {
            DOM.masterCoords.style.color = 'var(--red)';
            if (location) {
                DOM.masterCoords.textContent = 
                    `⚠ Không lấy được GPS (mặc định: ${location.lat.toFixed(5)}, ${location.lng.toFixed(5)})`;
                masterLocation = location;
            } else {
                DOM.masterCoords.textContent = '⚠ Không lấy được GPS';
            }
            return;
        }

        // status === 'ok' — GPS lấy thành công
        masterLocation = location;
        DOM.masterCoords.textContent = 
            `${location.lat.toFixed(5)}, ${location.lng.toFixed(5)}`;
        DOM.masterCoords.style.color = '';

        // Nếu đã kết nối Serial → gửi ngay CMD_GPS_UPDATE
        if (SerialManager.getConnectionStatus()) {
            _sendGPSUpdate(location);
        }
    }

    // ===============================================================
    // XỬ LÝ KẾT NỐI SERIAL
    // ===============================================================

    // ---------------------------------------------------------------
    // Handler: Nhấn nút "Kết nối thiết bị"
    // Toggle giữa connect/disconnect
    // ---------------------------------------------------------------
    async function _handleConnect() {
        if (SerialManager.getConnectionStatus()) {
            await SerialManager.disconnect();
        } else {
            await SerialManager.connect();
        }
    }

    // ---------------------------------------------------------------
    // CALLBACK: Trạng thái Serial thay đổi
    // Cập nhật UI (nút, status dot, map status bar)
    // ---------------------------------------------------------------
    function _onSerialStatusChange(status) {
        const statusDot = DOM.connectionStatus.querySelector('.status-dot');
        const statusText = DOM.connectionStatus.querySelector('.status-text');
        const mapDot = DOM.mapStatusConn.querySelector('.status-dot');

        // Reset classes
        statusDot.className = 'status-dot';
        mapDot.className = 'status-dot';

        switch (status) {
            case 'connecting':
                statusDot.classList.add('connecting');
                mapDot.classList.add('connecting');
                statusText.textContent = 'Đang kết nối...';
                DOM.mapStatusConn.innerHTML = `<span class="status-dot connecting"></span>Serial: Đang kết nối`;
                DOM.connectBtn.disabled = true;
                break;

            case 'connected':
                statusDot.classList.add('connected');
                mapDot.classList.add('connected');
                statusText.textContent = 'Kết nối thành công';
                DOM.mapStatusConn.innerHTML = `<span class="status-dot connected"></span>Serial: Đã kết nối`;
                DOM.connectBtn.innerHTML = `
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
                    Ngắt kết nối`;
                DOM.connectBtn.classList.add('connected-state');
                DOM.connectBtn.disabled = false;

                // Gửi CMD_GPS_UPDATE ngay khi kết nối thành công
                if (masterLocation) {
                    _sendGPSUpdate(masterLocation);
                }
                break;

            case 'disconnected':
            default:
                statusDot.classList.add('disconnected');
                mapDot.classList.add('disconnected');
                statusText.textContent = 'Chưa kết nối';
                DOM.mapStatusConn.innerHTML = `<span class="status-dot disconnected"></span>Serial: Ngắt kết nối`;
                DOM.connectBtn.innerHTML = `
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="7" width="20" height="14" rx="2" ry="2"/><polyline points="16 3 12 7 8 3"/></svg>
                    Kết nối thiết bị`;
                DOM.connectBtn.classList.remove('connected-state');
                DOM.connectBtn.disabled = false;
                break;
        }
    }

    // ===============================================================
    // XỬ LÝ DỮ LIỆU NHẬN TỪ MASTER (MCU → Web)
    // ===============================================================

    // ---------------------------------------------------------------
    // CALLBACK: Nhận được gói tin JSON từ Master qua Serial
    //
    // ĐỊNH DẠNG GÓI TIN TỪ FIRMWARE (Master → App):
    //
    // 1) Dữ liệu tracking định kỳ:
    //    {
    //      "msg_type": "CMD_UART_TRACKING",
    //      "id_slave": 1,          ← INTEGER (không phải string!)
    //      "id_relay": 0,
    //      "data": { "lat": 12.238790, "lng": 109.196540,
    //                "temp": 37, "humi": 80, "bat": 80, "rssi": -65 }
    //    }
    //
    // 2) Tín hiệu SOS từ Slave:
    //    {
    //      "msg_type": "CMD_UART_SOS",
    //      "id_slave": 2,
    //      "id_relay": 0,
    //      "data": { "lat": ..., "lng": ..., "rssi": -70 }
    //    }
    //
    // 3) Phản hồi Ping:
    //    {
    //      "msg_type": "CMD_UART_PONG",
    //      "id_slave": 3,
    //      "id_relay": 0,
    //      "data": { ... }
    //    }
    // ---------------------------------------------------------------
    function _onSerialDataReceived(json) {
        const { msg_type, id_slave, id_relay, data } = json;

        // Firmware gửi id_slave là integer (1, 2, 3...) 
        // → Convert sang hex string 2 ký tự ("01", "02", "0A"...) cho hiển thị
        if (id_slave === undefined || id_slave === null || !data) return;
        const nodeId = String(id_slave).padStart(2, '0');

        switch (msg_type) {
            case 'CMD_UART_TRACKING':
                _handlePeriodicData(nodeId, data, id_relay);
                break;

            // --- Gói SOS từ Slave ---
            case 'CMD_UART_SOS':
                _handleSOSData(nodeId, data);
                break;

            // --- Phản hồi Ping ---
            case 'CMD_UART_PONG':
                // Chỉ xử lý nếu đang chờ PONG từ Node này (1 PING → 1 PONG)
                if (!pendingPings.has(nodeId)) break;
                pendingPings.delete(nodeId);

                _addLog(`✓ Node ${nodeId} phản hồi PONG (RSSI: ${data.rssi || '--'} dBm)`, 'rx');
                // Cập nhật trạng thái node nếu đã có
                if (nodes[nodeId]) {
                    nodes[nodeId].lastUpdate = Date.now();
                    nodes[nodeId].rssi = data.rssi || 0;
                    _renderNodeList();
                }
                // Hiển thị thông báo
                _showToast(
                    `✅ Node ${nodeId} PONG thành công! (RSSI: ${data.rssi || '--'} dBm)`,
                    'success',
                    `pong-${nodeId}`
                );
                break;

            default:
                _addLog(`Gói tin không xác định: msg_type=${msg_type}`, 'info');
                break;
        }
    }

    // ---------------------------------------------------------------
    // Xử lý gói tracking định kỳ từ Slave (msg_type = "CMD_UART_TRACKING")
    // Cập nhật dữ liệu Node, marker trên bản đồ, sidebar
    // ---------------------------------------------------------------
    function _handlePeriodicData(id, data, id_relay) {
        const { lat, lng, temp, humi, bat, rssi } = data;

        // Nếu Node đang SOS hoặc đang trong danh sách xác nhận mà nhận lại tracking → tự động xóa SOS
        const wasSOS = nodes[id] ? nodes[id].isSOS : false;
        if (wasSOS || acknowledgedSOS.has(id)) {
            _addLog(`Node ${id} đã gửi lại dữ liệu tracking → tự động xóa SOS`, 'info');
            MapManager.clearSOSMarker(id);
            acknowledgedSOS.delete(id); // Cho phép popup lại nếu SOS mới
        }

        // Cập nhật/tạo mới dữ liệu Node trong bộ nhớ
        nodes[id] = {
            lat: lat,
            lng: lng,
            temp: temp || 0,
            humi: humi || 0,
            bat: bat || 0,
            rssi: rssi || 0,
            idRelay: id_relay || 0,
            isSOS: false, // Tracking = không còn SOS
            lastUpdate: Date.now()
        };

        // Cập nhật Marker trên bản đồ Leaflet
        MapManager.updateSlaveMarker(id, lat, lng, data, false);

        // Cập nhật danh sách Node trên sidebar
        _renderNodeList();

        // Cập nhật số lượng Node trên map status
        DOM.mapStatusNodes.innerHTML = `
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/></svg>
            Nodes: ${Object.keys(nodes).length}`;
    }

    // ---------------------------------------------------------------
    // Xử lý gói SOS từ Slave (msg_type = "CMD_SOS")
    // Kích hoạt cảnh báo khẩn cấp toàn màn hình
    // ---------------------------------------------------------------
    function _handleSOSData(id, data) {
        const { lat, lng } = data;

        // Cập nhật dữ liệu Node với trạng thái SOS
        nodes[id] = {
            lat: lat,
            lng: lng,
            temp: nodes[id] ? nodes[id].temp : 0,
            humi: nodes[id] ? nodes[id].humi : 0,
            bat: nodes[id] ? nodes[id].bat : 0,
            isSOS: true,
            lastUpdate: Date.now()
        };

        currentSOSNodeId = id;

        // Cập nhật Marker SOS trên bản đồ (đỏ + bounce)
        MapManager.updateSlaveMarker(id, lat, lng, {}, true);

        // Chỉ hiển thị SOS Overlay nếu Node này CHƯA được xác nhận
        if (!acknowledgedSOS.has(id)) {
            DOM.sosAlertText.textContent = `Node ${id} đang phát tín hiệu cầu cứu!`;
            DOM.sosCoords.textContent = `Lat: ${lat.toFixed(5)}, Lng: ${lng.toFixed(5)}`;
            DOM.sosOverlay.classList.remove('hidden');

            // Phát âm thanh cảnh báo
            _playSOSAudio();
        }

        // Cập nhật sidebar
        _renderNodeList();
    }

    // ===============================================================
    // GỬI LỆNH XUỐNG MASTER (Web → MCU)
    // ===============================================================

    // ---------------------------------------------------------------
    // Gửi CMD_GPS_UPDATE — Cập nhật tọa độ GPS cho Master
    // Được gọi khi: kết nối thành công hoặc Geolocation cập nhật
    //
    // FRAME GỬI:
    // {
    //   "msg_type": "CMD_GPS_UPDATE",
    //   "id_slave": 0,             // 0 = Master
    //   "data": { "lat": xx.xxx, "lng": xxx.xxx }
    // }
    // ---------------------------------------------------------------
    function _sendGPSUpdate(location) {
        SerialManager.sendJSON({
            msg_type: "CMD_UART_GPS_UPDATE",
            id_slave: 0,
            data: {
                lat: location.lat,
                lng: location.lng
            }
        });
    }

    // ---------------------------------------------------------------
    // Gửi CMD_HELP — Yêu cầu hỗ trợ Slave đang gặp nạn
    // Gửi tọa độ của Node cần hỗ trợ để các Slave khác tìm kiếm
    //
    // FRAME GỬI:
    // {
    //   "msg_type": "CMD_HELP",
    //   "id_slave": 2,            // ID Slave cần hỗ trợ (integer)
    //   "data": { "lat": xx.xxx, "lng": xxx.xxx }
    // }
    // ---------------------------------------------------------------
    function _handleSendHelp() {
        if (!selectedNodeId || !nodes[selectedNodeId]) return;

        if (!SerialManager.getConnectionStatus()) {
            _showToast('⚠️ Chưa thực hiện kết nối với Master', 'error');
            return;
        }

        const node = nodes[selectedNodeId];
        SerialManager.sendJSON({
            msg_type: "CMD_UART_HELP",
            id_slave: parseInt(selectedNodeId, 10),
            data: {
                lat: node.lat,
                lng: node.lng
            }
        });

        _addLog(`Đã gửi lệnh HELP cho Node ${selectedNodeId}`, 'info');
        DOM.nodeDetailDialog.classList.add('hidden');
    }

    // ---------------------------------------------------------------
    // Gửi CMD_GATHER — Lệnh tập hợp toàn đội tại vị trí click
    // Broadcast đến tất cả Slave (id_slave = 255)
    //
    // FRAME GỬI:
    // {
    //   "msg_type": "CMD_GATHER",
    //   "id_slave": 255,          // 255 = broadcast tất cả
    //   "data": { "lat": xx.xxx, "lng": xxx.xxx }
    // }
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // Gửi CMD_UART_PING — Kiểm tra kết nối Slave
    // ---------------------------------------------------------------
    function _handleSendPing() {
        if (!selectedNodeId) return;

        if (!SerialManager.getConnectionStatus()) {
            _showToast('⚠️ Chưa thực hiện kết nối với Master', 'error');
            return;
        }

        // Đánh dấu đang chờ PONG từ Node này
        pendingPings.add(selectedNodeId);

        SerialManager.sendJSON({
            msg_type: "CMD_UART_PING",
            id_slave: parseInt(selectedNodeId, 10),
            data: { lat: 0, lng: 0 }
        });

        _addLog(`Đã gửi lệnh PING tới Node ${selectedNodeId}`, 'tx');
    }

    // ---------------------------------------------------------------
    // Hành động khi Click bản đồ: Chọn Mark Point hoặc Warn Point
    // ---------------------------------------------------------------
    let pendingActionCoords = null;

    function _onMapClick(lat, lng) {

        pendingActionCoords = { lat, lng };
        DOM.mapActionCoords.textContent = `Lat: ${lat.toFixed(5)}, Lng: ${lng.toFixed(5)}`;
        DOM.mapActionDialog.classList.remove('hidden');

        // Bind Mark Point button
        DOM.btnMarkPoint.onclick = () => {
            if (!SerialManager.getConnectionStatus()) {
                _showToast('⚠️ Chưa thực hiện kết nối với Master', 'error');
                DOM.mapActionDialog.classList.add('hidden');
                return;
            }
            if (pendingActionCoords) {
                SerialManager.sendJSON({
                    msg_type: "CMD_UART_MARK_POINT",
                    id_slave: 255, // Broadcast (0xFF)
                    data: { lat: pendingActionCoords.lat, lng: pendingActionCoords.lng }
                });
                MapManager.setGatherMarker(pendingActionCoords.lat, pendingActionCoords.lng, 'mark');
                _addLog(`Đã đánh dấu ĐIỂM ĐẾN tại (${pendingActionCoords.lat.toFixed(5)}, ${pendingActionCoords.lng.toFixed(5)})`, 'tx');
            }
            DOM.mapActionDialog.classList.add('hidden');
            pendingActionCoords = null;
        };

        // Bind Warn Point button
        DOM.btnWarnPoint.onclick = () => {
            if (!SerialManager.getConnectionStatus()) {
                _showToast('⚠️ Chưa thực hiện kết nối với Master', 'error');
                DOM.mapActionDialog.classList.add('hidden');
                return;
            }
            if (pendingActionCoords) {
                SerialManager.sendJSON({
                    msg_type: "CMD_UART_WARN_POINT",
                    id_slave: 255, // Broadcast (0xFF)
                    data: { lat: pendingActionCoords.lat, lng: pendingActionCoords.lng }
                });
                MapManager.setGatherMarker(pendingActionCoords.lat, pendingActionCoords.lng, 'warn');
                _addLog(`Đã đánh dấu ĐIỂM NGUY HIỂM tại (${pendingActionCoords.lat.toFixed(5)}, ${pendingActionCoords.lng.toFixed(5)})`, 'tx');
            }
            DOM.mapActionDialog.classList.add('hidden');
            pendingActionCoords = null;
        };
    }

    // ===============================================================
    // XỬ LÝ SOS UI
    // ===============================================================

    // Nút "Định vị trên bản đồ" trong SOS overlay
    function _handleSOSLocate() {
        if (currentSOSNodeId && nodes[currentSOSNodeId]) {
            const node = nodes[currentSOSNodeId];
            MapManager.panTo(node.lat, node.lng, 17);
            
            // Đánh dấu đã xác nhận để tránh bị popup lại liên tục khi đang xem bản đồ
            acknowledgedSOS.add(currentSOSNodeId);
        }
        DOM.sosOverlay.classList.add('hidden');
        _stopSOSAudio();
    }

    // Nút "Đã xác nhận" trong SOS overlay
    function _handleSOSDismiss() {
        // Đánh dấu Node SOS hiện tại đã được xác nhận → không popup lại
        if (currentSOSNodeId) {
            acknowledgedSOS.add(currentSOSNodeId);
        }
        DOM.sosOverlay.classList.add('hidden');
        _stopSOSAudio();
    }

    // Nút "Xác nhận SOS" trong Node Detail Dialog
    function _handleSOSAck() {
        if (selectedNodeId && nodes[selectedNodeId]) {
            nodes[selectedNodeId].isSOS = false;
            acknowledgedSOS.add(selectedNodeId); // Không popup lại
            MapManager.clearSOSMarker(selectedNodeId);
            _renderNodeList();
            _addLog(`Đã xác nhận SOS cho Node ${selectedNodeId}`, 'info');
        }
        DOM.nodeDetailDialog.classList.add('hidden');
    }

    // ===============================================================
    // UI: DANH SÁCH NODE SIDEBAR
    // ===============================================================

    // ---------------------------------------------------------------
    // Click vào Marker trên bản đồ → hiển thị Node Detail Dialog
    // ---------------------------------------------------------------
    function _onMarkerClick(id) {
        _showNodeDetail(id);
    }

    // ---------------------------------------------------------------
    // Render danh sách Node trên sidebar
    // Throttled: tối đa mỗi 2 giây để tránh giật UI khi data cập nhật nhanh
    // ---------------------------------------------------------------
    let _renderTimer = null;
    let _lastRenderedIds = '';  // Track cấu trúc list để biết khi nào cần full re-render

    function _renderNodeList() {
        // Throttle: gộp nhiều lần gọi liên tiếp thành 1 lần render
        if (_renderTimer) return;
        _renderTimer = setTimeout(() => {
            _renderTimer = null;
            _doRenderNodeList();
        }, 2000);
        
        // Lần gọi đầu tiên → render ngay
        if (_lastRenderedIds === '') {
            _doRenderNodeList();
        }
    }

    function _doRenderNodeList() {
        const ids = Object.keys(nodes);
        DOM.nodeCount.textContent = ids.length;

        if (ids.length === 0) {
            DOM.nodeList.innerHTML = `
                <div class="node-list-empty">
                    <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" opacity="0.3"><circle cx="12" cy="12" r="10"/><path d="M8 12h8"/></svg>
                    <p>Chưa có Node nào kết nối</p>
                </div>`;
            _lastRenderedIds = '';
            return;
        }

        const currentIds = ids.join(',');

        // Nếu danh sách Node không đổi → chỉ cập nhật text, không xóa DOM
        if (currentIds === _lastRenderedIds) {
            ids.forEach(id => {
                const card = DOM.nodeList.querySelector(`[data-node-id="${id}"]`);
                if (card && nodes[id]) {
                    const n = nodes[id];
                    const meta = card.querySelector('.node-meta');
                    if (meta) meta.textContent = `${n.lat.toFixed(5)}, ${n.lng.toFixed(5)}`;
                    const badges = card.querySelectorAll('.badge');
                    if (badges[0]) badges[0].textContent = `${n.temp}°`;
                    if (badges[1]) badges[1].textContent = `${n.bat}%`;
                    if (badges[2]) badges[2].textContent = `${n.rssi || '--'}dB`;
                    // Cập nhật trạng thái SOS
                    if (n.isSOS) {
                        card.classList.add('sos-active');
                    } else {
                        card.classList.remove('sos-active');
                    }
                }
            });
            return;
        }

        // Danh sách Node thay đổi (thêm/xóa) → full re-render
        const COLORS = ['#6c8cff', '#a78bfa', '#f472b6', '#fb923c', '#34d399', '#38bdf8', '#facc15', '#c084fc'];

        DOM.nodeList.innerHTML = ids.map(id => {
            const n = nodes[id];
            const colorIdx = (parseInt(id, 16) - 1) % COLORS.length;
            const color = COLORS[colorIdx >= 0 ? colorIdx : 0];
            const sosClass = n.isSOS ? ' sos-active' : '';

            return `
                <div class="node-card${sosClass}" data-node-id="${id}" onclick="AppController.showNodeDetail('${id}')">
                    <div class="node-avatar" style="background:linear-gradient(135deg,${color},${color}99)">
                        ${id}
                    </div>
                    <div class="node-info">
                        <div class="node-name">Node ${id} ${n.isSOS ? '🚨' : ''}</div>
                        <div class="node-meta">${n.lat.toFixed(5)}, ${n.lng.toFixed(5)}</div>
                    </div>
                    <div class="node-badges">
                        <span class="badge badge-temp">${n.temp}°</span>
                        <span class="badge badge-bat">${n.bat}%</span>
                        <span class="badge" style="background:rgba(108,140,255,0.1);color:#6c8cff;">${n.rssi || '--'}dB</span>
                        ${n.isSOS ? '<span class="badge badge-sos">SOS</span>' : ''}
                    </div>
                </div>`;
        }).join('');

        _lastRenderedIds = currentIds;
    }

    // ---------------------------------------------------------------
    // Hiển thị Node Detail Dialog
    // ---------------------------------------------------------------
    function _showNodeDetail(id) {
        selectedNodeId = id;
        const node = nodes[id];
        if (!node) return;

        const COLORS = ['#6c8cff', '#a78bfa', '#f472b6', '#fb923c', '#34d399', '#38bdf8', '#facc15', '#c084fc'];
        const colorIdx = (parseInt(id, 16) - 1) % COLORS.length;
        const color = COLORS[colorIdx >= 0 ? colorIdx : 0];

        DOM.nodeDetailAvatar.textContent = id;
        DOM.nodeDetailAvatar.style.background = `linear-gradient(135deg, ${color}, ${color}99)`;
        DOM.nodeDetailTitle.textContent = `Node ${id}`;

        if (node.isSOS) {
            DOM.nodeDetailStatus.textContent = '🚨 SOS — Cầu cứu';
            DOM.nodeDetailStatus.className = 'node-detail-status sos';
            DOM.nodeSosAckBtn.classList.remove('hidden');
        } else {
            DOM.nodeDetailStatus.textContent = 'Hoạt động';
            DOM.nodeDetailStatus.className = 'node-detail-status';
            DOM.nodeSosAckBtn.classList.add('hidden');
        }

        DOM.nodeDetailTemp.textContent = `${node.temp}°C`;
        DOM.nodeDetailHumi.textContent = `${node.humi}%`;
        DOM.nodeDetailBat.textContent = `${node.bat}%`;
        DOM.nodeDetailRssi.textContent = `${node.rssi || '--'} dBm`;
        DOM.nodeDetailCoords.textContent = `${node.lat.toFixed(5)}, ${node.lng.toFixed(5)}`;

        // Pan + zoom bản đồ đến vị trí Node (phóng to gần)
        MapManager.panTo(node.lat, node.lng, 18);

        DOM.nodeDetailDialog.classList.remove('hidden');
    }

    // ===============================================================
    // SERIAL LOG
    // ===============================================================
    function _addLog(msg, type = 'info') {
        const entry = document.createElement('div');
        entry.className = `log-entry log-${type}`;

        const time = new Date().toLocaleTimeString('vi-VN', { hour12: false });
        entry.textContent = `[${time}] ${msg}`;

        DOM.serialLog.appendChild(entry);
        // Auto-scroll xuống cuối
        DOM.serialLog.scrollTop = DOM.serialLog.scrollHeight;

        // Giới hạn 200 entries để tránh chiếm bộ nhớ
        while (DOM.serialLog.children.length > 200) {
            DOM.serialLog.removeChild(DOM.serialLog.firstChild);
        }
    }

    // ===============================================================
    // TOAST NOTIFICATION
    // Hỗ trợ 'key': nếu toast với key đã tồn tại → cập nhật nội dung
    // thay vì tạo mới (tránh spam toast trùng lặp)
    // ===============================================================
    const _activeToasts = {};  // { key: { element, timerId } }

    function _showToast(message, type = 'info', key = null) {
        // Tạo container nếu chưa có
        let container = document.getElementById('toastContainer');
        if (!container) {
            container = document.createElement('div');
            container.id = 'toastContainer';
            container.style.cssText = `
                position: fixed; top: 20px; right: 20px; z-index: 2000;
                display: flex; flex-direction: column; gap: 8px;
                pointer-events: none;
            `;
            document.body.appendChild(container);
        }

        // Nếu có key và toast cũ còn hiển thị → cập nhật nội dung
        if (key && _activeToasts[key] && _activeToasts[key].element.parentNode) {
            const existing = _activeToasts[key];
            existing.element.textContent = message;
            // Hiệu ứng nhấp nháy nhẹ khi cập nhật
            existing.element.style.animation = 'none';
            existing.element.offsetHeight; // force reflow
            existing.element.style.animation = 'toastPulse 0.3s ease';
            // Reset timer tự đóng
            clearTimeout(existing.timerId);
            existing.timerId = setTimeout(() => _dismissToast(existing.element, key), 4000);
            return;
        }

        const toast = document.createElement('div');
        toast.style.cssText = `
            padding: 14px 22px; border-radius: 10px;
            font-family: var(--font-main); font-size: 14px; font-weight: 600;
            color: #fff; pointer-events: auto; cursor: pointer;
            box-shadow: 0 8px 30px rgba(0,0,0,0.3);
            animation: toastSlideIn 0.35s ease;
            backdrop-filter: blur(10px);
            max-width: 380px;
        `;

        // Màu theo type
        if (type === 'success') {
            toast.style.background = 'linear-gradient(135deg, #059669, #34d399)';
        } else if (type === 'error') {
            toast.style.background = 'linear-gradient(135deg, #dc2626, #f43f5e)';
        } else {
            toast.style.background = 'linear-gradient(135deg, #4f46e5, #6c8cff)';
        }

        toast.textContent = message;
        container.appendChild(toast);

        // Click để đóng sớm
        toast.addEventListener('click', () => _dismissToast(toast, key));

        // Tự đóng sau 4 giây
        const timerId = setTimeout(() => _dismissToast(toast, key), 4000);

        // Lưu lại nếu có key
        if (key) {
            _activeToasts[key] = { element: toast, timerId };
        }
    }

    function _dismissToast(toast, key = null) {
        if (!toast.parentNode) return;
        toast.style.animation = 'toastSlideOut 0.3s ease forwards';
        setTimeout(() => {
            if (toast.parentNode) toast.parentNode.removeChild(toast);
        }, 300);
        // Xóa khỏi danh sách active
        if (key && _activeToasts[key]) {
            delete _activeToasts[key];
        }
    }

    // ===============================================================
    // SOS AUDIO (Web Audio API)
    // ===============================================================
    function _playSOSAudio() {
        try {
            if (!audioCtx) {
                audioCtx = new (window.AudioContext || window.webkitAudioContext)();
            }
            if (sosOscillator) return; // Đang phát rồi

            sosOscillator = audioCtx.createOscillator();
            const gainNode = audioCtx.createGain();
            sosOscillator.connect(gainNode);
            gainNode.connect(audioCtx.destination);

            sosOscillator.type = 'square';
            sosOscillator.frequency.setValueAtTime(800, audioCtx.currentTime);
            gainNode.gain.setValueAtTime(0.15, audioCtx.currentTime);

            // Beep pattern: tần số thay đổi tạo hiệu ứng SOS
            const now = audioCtx.currentTime;
            for (let i = 0; i < 50; i++) {
                sosOscillator.frequency.setValueAtTime(800, now + i * 0.6);
                sosOscillator.frequency.setValueAtTime(600, now + i * 0.6 + 0.3);
            }

            sosOscillator.start();
        } catch (e) {
            console.warn('Không thể phát âm thanh SOS:', e);
        }
    }

    function _stopSOSAudio() {
        if (sosOscillator) {
            try { sosOscillator.stop(); } catch (e) {}
            sosOscillator = null;
        }
    }

    // ===============================================================
    // CLOCK (Map Status Bar)
    // ===============================================================
    function _startClock() {
        setInterval(() => {
            const now = new Date().toLocaleTimeString('vi-VN', { hour12: false });
            DOM.mapStatusTime.innerHTML = `
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><polyline points="12 6 12 12 16 14"/></svg>
                ${now}`;
        }, 1000);
    }

    // --- Public API ---
    return {
        init,
        onMasterLocationReady,
        showNodeDetail: _showNodeDetail
    };
})();

// Export cho global scope
window.AppController = AppController;

// ============================================================
// Khởi tạo khi DOM ready
// ============================================================
document.addEventListener('DOMContentLoaded', () => {
    AppController.init();
});
