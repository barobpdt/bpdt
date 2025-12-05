function WebsocketManager(serverUrl ) {
	if(!serverUrl) serverUrl = 'ws://localhost:8092/ws'
	const WS_READY = 0
	const WS_START = 1
	const WS_CONNECT = 3
	const WS_JOIN = 4
	const WS_DISCONNECT = 5
	const WS_ERROR = 9
	
	let ws = null;
	let reconnectTimeout = 0
	let wsStatus=WS_READY

	// 채팅 입장
	function getStatus() {
		return wsStatus;
	}

	// WebSocket 연결
	function connectWebSocket() {
		updateConnectionStatus(WS_START, false);
		
		// WebSocket 연결 (ws:// 프로토콜 사용)
		ws = new WebSocket(serverUrl);

		// 연결 성공
		ws.onopen = () => {
			console.log('✅ WebSocket 연결 성공');
			updateConnectionStatus(WS_CONNECT, true);
			// 방 입장
			sendWsMessage('join', { mode:'dev' });
		};

		// 메시지 수신
		ws.onmessage = (event) => {
			try {
				const { type, data } = JSON.parse(event.data);
				handleMessage(type, data);
			} catch (error) {
				console.error('메시지 파싱 오류:', error);
			}
		};

		// 연결 종료
		ws.onclose = () => {
			updateConnectionStatus(WS_DISCONNECT, false);
			reconnectTimeout = setTimeout(() => {
				console.log('🔄 재연결 시도...');
				connectWebSocket();
			}, 5000);
		};

		// 에러 처리
		ws.onerror = (error) => {
			console.error('WebSocket 오류:', error);
			updateConnectionStatus(WS_ERROR, false);
		};
	}
	function isConnect() {
		return ws && ws.readyState === WebSocket.OPEN
	}
	function isJoin() {
		return wsStatus === WS_JOIN
	}
	// WebSocket 메시지 전송
	function sendWsMessage(type, data) {
		if ( isConnect() ) {
			ws.send(JSON.stringify({ type, data }));
		}
	}

	// 메시지 핸들러
	function handleMessage(type, data) {
		switch (type) {
		case 'joined':
			updateConnectionStatus(WS_JOIN, true)
			break;	
		case 'error':
			clog('서버 오류:', data.message);
			break;
		}
	}

	// 연결 상태 업데이트
	function updateConnectionStatus(status, connected) {
		wsStatus = status;
	}
	 
	// HTML 이스케이프
	function escapeHtml(text) {
		const div = document.createElement('div');
		div.textContent = text;
		return div.innerHTML;
	}
	function closeSocket() {
		if (ws) {
			ws.close();
			wsStatus = WS_READY
			ws = null
		}
	}
	// 페이지 종료 시 WebSocket 닫기
	window.addEventListener('beforeunload', () => {
		if (ws) {
			ws.close();
		}
	});
	return {getStatus, isConnect, isJoin, connectWebSocket, handleMessage, sendWsMessage, closeSocket}
}