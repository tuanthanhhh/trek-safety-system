/* ============================================================
   map.js — Leaflet Map Management Module
   ============================================================
   Quản lý bản đồ Leaflet + OpenStreetMap (miễn phí, không cần API Key).
   
   MARKER TYPES:
   - Master (id "00"): Marker xanh lá đặc biệt, vị trí máy tính
   - Slave  (id "01"-"FE"): Marker màu xoay vòng, cập nhật từ LoRa
   - SOS    : Slave đang SOS → marker đỏ nhấp nháy
   - Gather : Marker vàng tạm khi gửi lệnh tập hợp
   
   TƯƠNG TÁC:
   - Click Marker Slave → hiển thị Node Detail Dialog
   - Click khoảng trống bản đồ → hỏi Gather (tập hợp)
   ============================================================ */

const MapManager = (() => {
    // --- Private State ---
    let map = null;
    let markers = {};           // { id_slave: L.marker }
    let masterMarker = null;
    let actionMarkers = [];     // Mảng chứa các marker đánh dấu tạm (Warn/Mark)
    let masterPosition = null;  // { lat, lng }
    let sosBlinkIntervals = {}; // { id_slave: intervalId } — cho hiệu ứng nhấp nháy SOS

    // Callbacks
    let onMapClick = null;      // (lat, lng) => void — khi click bản đồ
    let onMarkerClick = null;   // (id_slave) => void — khi click marker Slave

    // Màu sắc cho từng Node (xoay vòng)
    const NODE_COLORS = [
        '#6c8cff', '#a78bfa', '#f472b6', '#fb923c',
        '#34d399', '#38bdf8', '#facc15', '#c084fc'
    ];

    // ---------------------------------------------------------------
    // HELPER: Tạo custom icon SVG cho marker (Leaflet dùng L.divIcon)
    // @param {string} color  — màu nền
    // @param {string} label  — text hiển thị giữa marker
    // @param {boolean} isSOS — true → hiệu ứng pulse đỏ
    // @param {number} size   — kích thước (px)
    // ---------------------------------------------------------------
    function _createIcon(color, label, isSOS = false, size = 36) {
        const pulseClass = isSOS ? 'marker-sos-pulse' : '';
        const html = `
            <div class="custom-marker ${pulseClass}" style="
                width:${size}px; height:${size}px;
                background: ${color};
                border: 3px solid rgba(255,255,255,0.9);
                border-radius: 50%;
                display: flex; align-items: center; justify-content: center;
                color: #fff; font-weight: 800; font-size: ${size * 0.35}px;
                font-family: 'Inter', sans-serif;
                box-shadow: 0 2px 12px ${color}88, 0 0 0 4px ${color}33;
                transition: transform 0.2s;
            ">${label}</div>
        `;
        return L.divIcon({
            html: html,
            className: 'leaflet-marker-custom',
            iconSize: [size, size],
            iconAnchor: [size / 2, size / 2],
            popupAnchor: [0, -size / 2 - 4]
        });
    }

    // ---------------------------------------------------------------
    // PUBLIC: Khởi tạo Leaflet Map
    // Sử dụng Geolocation API lấy vị trí máy tính → đặt Master marker
    // ---------------------------------------------------------------
    function init() {
        // Vị trí mặc định (ĐH Bách Khoa TP.HCM) nếu không lấy được GPS
        const defaultPos = [10.7724, 106.6581];

        // Khởi tạo bản đồ Leaflet
        map = L.map('map', {
            center: defaultPos,
            zoom: 15,
            zoomControl: true
        });

        // Tile layer: OpenStreetMap standard (sáng, rõ ràng, dễ đọc)
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors',
            maxZoom: 19
        }).addTo(map);

        // Lắng nghe sự kiện click trên bản đồ → chức năng Gather
        map.on('click', (e) => {
            if (onMapClick) {
                onMapClick(e.latlng.lat, e.latlng.lng);
            }
        });

        // Lấy vị trí hiện tại bằng Geolocation API (liên tục cập nhật)
        let firstGPSFix = false;

        if (navigator.geolocation) {
            // Hiển thị trạng thái đang tìm GPS
            if (window.AppController && window.AppController.onMasterLocationReady) {
                window.AppController.onMasterLocationReady(null, 'searching');
            }

            // Dùng watchPosition để liên tục cập nhật vị trí GPS
            navigator.geolocation.watchPosition(
                (pos) => {
                    const location = {
                        lat: pos.coords.latitude,
                        lng: pos.coords.longitude
                    };
                    masterPosition = location;
                    _createMasterMarker(location);

                    // Chỉ pan bản đồ đến vị trí GPS lần đầu tiên
                    if (!firstGPSFix) {
                        map.setView([location.lat, location.lng], 15);
                        firstGPSFix = true;
                    }

                    // Thông báo cho app.js cập nhật tọa độ Master
                    if (window.AppController && window.AppController.onMasterLocationReady) {
                        window.AppController.onMasterLocationReady(location, 'ok');
                    }
                },
                (err) => {
                    console.warn('Không lấy được vị trí GPS:', err.message);

                    // Chỉ fallback về default nếu chưa có GPS fix nào
                    if (!firstGPSFix) {
                        masterPosition = { lat: defaultPos[0], lng: defaultPos[1] };
                        _createMasterMarker(masterPosition);

                        if (window.AppController && window.AppController.onMasterLocationReady) {
                            window.AppController.onMasterLocationReady(masterPosition, 'error');
                        }
                    }
                },
                { enableHighAccuracy: true, timeout: 30000, maximumAge: 5000 }
            );
        } else {
            masterPosition = { lat: defaultPos[0], lng: defaultPos[1] };
            _createMasterMarker(masterPosition);

            if (window.AppController && window.AppController.onMasterLocationReady) {
                window.AppController.onMasterLocationReady(masterPosition, 'error');
            }
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Đăng ký callbacks
    // ---------------------------------------------------------------
    function setCallbacks({ mapClick, markerClick }) {
        onMapClick = mapClick || null;
        onMarkerClick = markerClick || null;
    }

    // ---------------------------------------------------------------
    // PUBLIC: Cập nhật hoặc tạo mới Marker cho Slave Node
    // Throttled: mỗi marker chỉ cập nhật tối đa 1 lần / 2 giây
    //
    // @param {string} id      - ID của slave (ví dụ: "01", "02")
    // @param {number} lat     - Latitude
    // @param {number} lng     - Longitude
    // @param {Object} data    - { temp, humi, bat } (optional)
    // @param {boolean} isSOS  - true nếu đang ở trạng thái SOS
    // ---------------------------------------------------------------
    let markerThrottles = {};   // { id: timerId }
    let markerBuffers = {};     // { id: { lat, lng, data, isSOS } }

    function updateSlaveMarker(id, lat, lng, data = {}, isSOS = false) {
        if (!map) return;

        // Nếu marker chưa tồn tại → tạo ngay (không throttle)
        if (!markers[id]) {
            _doUpdateMarker(id, lat, lng, data, isSOS);
            return;
        }

        // Buffer dữ liệu mới nhất
        markerBuffers[id] = { lat, lng, data, isSOS };

        // Nếu đang throttle → chờ
        if (markerThrottles[id]) return;

        // Áp dụng ngay lần đầu + đặt throttle
        _doUpdateMarker(id, lat, lng, data, isSOS);
        markerThrottles[id] = setTimeout(() => {
            delete markerThrottles[id];
            // Áp dụng dữ liệu buffer mới nhất (nếu có)
            if (markerBuffers[id]) {
                const b = markerBuffers[id];
                _doUpdateMarker(id, b.lat, b.lng, b.data, b.isSOS);
                delete markerBuffers[id];
            }
        }, 2000);
    }

    function _doUpdateMarker(id, lat, lng, data, isSOS) {
        const colorIndex = (parseInt(id, 16) - 1) % NODE_COLORS.length;
        const color = isSOS ? '#f43f5e' : NODE_COLORS[colorIndex >= 0 ? colorIndex : 0];
        const icon = _createIcon(color, id, isSOS);

        if (markers[id]) {
            // Marker đã tồn tại → cập nhật vị trí (smooth animation)
            _animateMarker(markers[id], [lat, lng]);
            markers[id].setIcon(icon);
        } else {
            // Tạo Marker mới cho Slave
            const marker = L.marker([lat, lng], { icon: icon }).addTo(map);

            // Popup thông tin cơ bản
            marker.bindPopup(_buildPopupContent(id, data));

            // Click vào Marker → hiển thị Node Detail Dialog
            marker.on('click', () => {
                if (onMarkerClick) {
                    onMarkerClick(id);
                }
            });

            markers[id] = marker;
        }

        // Cập nhật popup content
        markers[id].setPopupContent(_buildPopupContent(id, data));

        // Nếu SOS → thêm hiệu ứng nhấp nháy
        if (isSOS) {
            _startSOSBlink(id);
        } else {
            _stopSOSBlink(id);
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Xóa trạng thái SOS trên Marker (sau khi xác nhận)
    // ---------------------------------------------------------------
    function clearSOSMarker(id) {
        _stopSOSBlink(id);
        if (markers[id]) {
            const colorIndex = (parseInt(id, 16) - 1) % NODE_COLORS.length;
            const color = NODE_COLORS[colorIndex >= 0 ? colorIndex : 0];
            markers[id].setIcon(_createIcon(color, id, false));
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Pan bản đồ đến vị trí cụ thể
    // Nếu khoảng cách lớn → dùng setView (tránh giật)
    // Nếu khoảng cách gần → dùng flyTo (mượt mà)
    // ---------------------------------------------------------------
    function panTo(lat, lng, zoom) {
        if (!map) return;

        const targetZoom = zoom || map.getZoom();
        const center = map.getCenter();
        const dist = Math.abs(center.lat - lat) + Math.abs(center.lng - lng);

        if (dist > 0.05) {
            // Khoảng cách xa (>~5km) → chuyển ngay, không animate
            map.setView([lat, lng], targetZoom);
        } else {
            // Khoảng cách gần → animate mượt
            map.flyTo([lat, lng], targetZoom, { duration: 0.8 });
        }
    }

    // ---------------------------------------------------------------
    // PUBLIC: Đặt Marker tạm thời (vàng cho điểm đến, đỏ cho nguy hiểm)
    // Hỗ trợ đánh dấu nhiều điểm cùng lúc
    // ---------------------------------------------------------------
    function setGatherMarker(lat, lng, type = 'mark') {
        let iconColor = '#fbbf24'; // default yellow for mark
        let iconLabel = '📍';
        let popupText = '<strong>📍 Điểm tập kết</strong><br><span style="font-size:10px;color:#888;">(Nhấn vào marker để xóa)</span>';

        if (type === 'warn') {
            iconColor = '#f43f5e'; // red for warn
            iconLabel = '⚠️';
            popupText = '<strong>⚠️ Cảnh báo Nguy hiểm</strong><br><span style="font-size:10px;color:#888;">(Nhấn vào marker để xóa)</span>';
        }

        const actionIcon = _createIcon(iconColor, iconLabel, type === 'warn', 42); // bounce if warn
        const newMarker = L.marker([lat, lng], { icon: actionIcon }).addTo(map);
        newMarker.bindPopup(popupText).openPopup();

        actionMarkers.push(newMarker);

        // Xóa đúng marker này khi click vào nó
        newMarker.on('click', () => {
            map.removeLayer(newMarker);
            actionMarkers = actionMarkers.filter(m => m !== newMarker);
        });
    }

    // ---------------------------------------------------------------
    // PUBLIC: Lấy vị trí Master hiện tại
    // ---------------------------------------------------------------
    function getMasterPosition() {
        return masterPosition;
    }

    // ---------------------------------------------------------------
    // PRIVATE: Tạo Marker cho Master (trạm trung tâm)
    // Marker đặc biệt: lớn hơn, viền trắng, màu xanh lá
    // ---------------------------------------------------------------
    function _createMasterMarker(position) {
        if (masterMarker) {
            masterMarker.setLatLng([position.lat, position.lng]);
            return;
        }

        const masterIcon = _createIcon('#34d399', 'M', false, 44);
        masterMarker = L.marker([position.lat, position.lng], { 
            icon: masterIcon,
            zIndexOffset: 1000 
        }).addTo(map);

        masterMarker.bindPopup(`
            <div style="font-family:Inter,sans-serif;text-align:center;">
                <strong>🏠 Trạm Master</strong><br/>
                <small style="color:#666;">Lat: ${position.lat.toFixed(5)}, Lng: ${position.lng.toFixed(5)}</small>
            </div>
        `);
    }

    // ---------------------------------------------------------------
    // PRIVATE: Tạo nội dung popup cho Slave marker
    // ---------------------------------------------------------------
    function _buildPopupContent(id, data) {
        const temp = data.temp !== undefined ? `${data.temp}°C` : '--';
        const humi = data.humi !== undefined ? `${data.humi}%` : '--';
        const bat = data.bat !== undefined ? `${data.bat}%` : '--';

        return `
            <div style="font-family:Inter,sans-serif;min-width:140px;">
                <strong style="font-size:14px;">📡 Node ${id}</strong><br/>
                <div style="margin-top:6px;font-size:12px;color:#555;">
                    🌡️ ${temp} &nbsp; 💧 ${humi} &nbsp; 🔋 ${bat}
                </div>
            </div>
        `;
    }

    // ---------------------------------------------------------------
    // PRIVATE: Animate marker di chuyển mượt đến vị trí mới
    // ---------------------------------------------------------------
    function _animateMarker(marker, newLatLng) {
        const startLatLng = marker.getLatLng();
        const deltaLat = newLatLng[0] - startLatLng.lat;
        const deltaLng = newLatLng[1] - startLatLng.lng;

        const duration = 500; // ms
        const startTime = performance.now();

        function step(currentTime) {
            const elapsed = currentTime - startTime;
            const progress = Math.min(elapsed / duration, 1);
            // Easing: ease-in-out quad
            const ease = progress < 0.5
                ? 2 * progress * progress
                : 1 - Math.pow(-2 * progress + 2, 2) / 2;

            marker.setLatLng([
                startLatLng.lat + deltaLat * ease,
                startLatLng.lng + deltaLng * ease
            ]);

            if (progress < 1) {
                requestAnimationFrame(step);
            }
        }
        requestAnimationFrame(step);
    }

    // ---------------------------------------------------------------
    // PRIVATE: Hiệu ứng nhấp nháy SOS cho marker
    // ---------------------------------------------------------------
    function _startSOSBlink(id) {
        if (sosBlinkIntervals[id]) return; // Đã nhấp nháy rồi

        let visible = true;
        sosBlinkIntervals[id] = setInterval(() => {
            if (markers[id]) {
                const el = markers[id].getElement();
                if (el) {
                    el.style.opacity = visible ? '0.3' : '1';
                    el.style.transform = visible 
                        ? 'scale(0.85) translate(-50%, -50%)' 
                        : 'scale(1.15) translate(-50%, -50%)';
                    visible = !visible;
                }
            }
        }, 500);
    }

    function _stopSOSBlink(id) {
        if (sosBlinkIntervals[id]) {
            clearInterval(sosBlinkIntervals[id]);
            delete sosBlinkIntervals[id];
            // Reset marker về trạng thái bình thường
            if (markers[id]) {
                const el = markers[id].getElement();
                if (el) {
                    el.style.opacity = '1';
                    el.style.transform = '';
                }
            }
        }
    }

    // --- Public API ---
    return {
        init,
        setCallbacks,
        updateSlaveMarker,
        clearSOSMarker,
        panTo,
        setGatherMarker,
        getMasterPosition
    };
})();

// Export cho global scope
window.MapManager = MapManager;
