## 파일전송 프로세스
http://localhost:8081/file-transfer2.html

[Sender]   POST   /api/peers         → offer SDP 저장
           GET    /api/peers/room/:roomId/type/answer → answer 폴링 (2초)
[Receiver] GET    /api/peers/room/:roomId/type/offer  → offer 폴링 감지
           POST   /api/peers         → answer SDP 저장

순서	행동
1	송신자 — 같은 Room ID 입력 → 입장 → ICE 수집 후 DB에 Offer 자동 저장
2	수신자 — 같은 Room ID, Receiver 선택 → 입장 → Offer 감지 시 자동 Answer
3	DataChannel 연결되면 파일 선택 후 파일 전송 클릭

Method	Path	용도
PATCH	/api/peers/:roomId/:peerId/sdp	SDP 업데이트
GET	/api/peers/room/:roomId/type/:type	타입별 피어 조회
DELETE	/api/peers/room/:roomId	룸 전체 초기화

## table order 프로세스
고객 착석 → tables.status = 'occupied'
    ↓
주문 → orders 생성 (pending)
    ↓
항목 추가 → order_items 삽입
    ↓
주방 확인 → orders.status = 'confirmed'
    ↓
서빙 완료 → orders.status = 'served'
    ↓
결제 → orders.status = 'paid' + tables.status = 'available'

Method	Endpoint	설명
GET	/api/order/tables	테이블 목록
POST	/api/order/tables	테이블 추가
PATCH	/api/order/tables/:id/status	테이블 상태 변경
DELETE	/api/order/tables/:id	테이블 삭제
GET	/api/order/categories	카테고리 목록
POST	/api/order/categories	카테고리 추가
GET	/api/order/menu	메뉴 목록 (?categoryId= 필터)
POST	/api/order/menu	메뉴 추가
PATCH	/api/order/menu/:id	메뉴 수정
GET	/api/order/orders	주문 목록 (?status= 필터)
POST	/api/order/orders	주문 생성 (가격 자동 계산)
GET	/api/order/orders/:id	주문 상세 (항목 포함)
POST	/api/order/orders/:id/items	주문에 메뉴 추가
PATCH	/api/order/orders/:id/status	주문 상태 변경
자동 처리
주문 생성 → 테이블 status occupied로 자동 변경
결제 완료(paid) → 테이블 status available로 자동 복구
총액(total_amount) 자동 계산


// 현재 코드 순서 ← 문제!
pc = new RTCPeerConnection(ICE_CFG);
pc.ondatachannel = e => { ... };           // ① 등록
await pc.setRemoteDescription(offer);      // ② Offer 적용 → 이 때 datachannel 이벤트 발생 가능
// 647번줄: answer 받은 후
await pc.setRemoteDescription({ type: 'answer', sdp: answers[0].sdp });
log('setRemoteDescription(answer) 완료', 'ok');
setStatus('협상 완료 — DataChannel 대기 중', 'connecting');
// ← dc.onopen이 트리거되지 않는 상황

##
relay 강제 모드에서 결과 예측:

케이스 A — ICE 수집이 여전히 오래 걸리고 failed
→ TURN 서버 자체가 막혀있음 (회사 방화벽 등). 다른 TURN 서버 필요.

케이스 B — ICE 수집이 빨라지고 연결됨 ✅
→ TURN 서버는 작동하는데 all 모드에서 host/srflx 후보가 relay보다 먼저 시도되다 실패한 것. relay 모드가 정답.

케이스 C — ICE 수집 완료가 훨씬 빨라졌는데도 failed
→ 두 기기 중 한쪽(수신자)의 TURN 후보가 없거나, 수신자 쪽에서 페이지를 새로고침 안 해서 구버전 ICE_CFG 사용 중.