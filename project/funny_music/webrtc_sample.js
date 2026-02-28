/*
발신자(Caller)                        수신자(Callee)
─────────────────────────────────────────────────────
1. pc.createOffer()  →  SDP offer 생성
2. pc.setLocalDescription(offer)
3. offer를 서버 DB에 저장 ──────────────→ DB에서 offer 조회
                                          4. pc.setRemoteDescription(offer)
                                          5. pc.createAnswer() → SDP answer 생성
                                          6. pc.setLocalDescription(answer)
                          ←────────────── 7. answer를 서버 DB에 저장
8. DB에서 answer 조회
9. pc.setRemoteDescription(answer)

*/
const pc = new RTCPeerConnection();

// ① Caller: offer 생성 → DB 저장
const offer = await pc.createOffer();
await pc.setLocalDescription(offer);

await fetch("/api/peers", {
  method: "POST",
  headers: { "Content-Type": "application/json" },
  body: JSON.stringify({
    roomId: "room-001",
    peerId: "peer-abc",
    type: "offer",
    sdp: offer.sdp,   // ← 브라우저가 생성한 SDP 문자열
  }),
});

// ② Callee: DB에서 offer 조회 후 answer 생성 → DB 저장
const { sdp } = await fetch("/api/peers/peer-abc").then(r => r.json());
await pc.setRemoteDescription({ type: "offer", sdp });

const answer = await pc.createAnswer();
await pc.setLocalDescription(answer);

await fetch("/api/peers", {
  method: "POST",
  headers: { "Content-Type": "application/json" },
  body: JSON.stringify({
    roomId: "room-001",
    peerId: "peer-xyz",
    type: "answer",
    sdp: answer.sdp,  // ← 브라우저가 생성한 SDP 문자열
  }),
});
