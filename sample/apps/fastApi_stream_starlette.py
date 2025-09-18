from fastapi import FastAPI
from fastapi_socketio import SocketManager
from loguru import logger
from starlette.responses import FileResponse
from starlette.staticfiles import StaticFiles
'''
#https://github.com/Theofilusarifin/real-time-ai-using-socket
sio = socketio.AsyncServer(cors_allowed_origins="*", async_mode="aiohttp")
app = web.Application()
sio.attach(app)
@sio.event
async def connect(sid, auth):
    username = auth.get("username")
    if not username:
        raise ConnectionRefusedError("Missing username")
    await sio.save_session(sid, {"username": username})
    await sio.enter_room(sid, "default_room")
    print(f"[{sid}] : {username} connected")

@sio.event
async def chat_message(sid, data):
    session = await sio.get_session(sid)
    username = session.get("username", "Unknown")
    user_message = data.get("message", "").strip()
    
    # Just broadcast the user message to everyone
    await sio.emit(
        "broadcast_message",
        {"user": username, "message": user_message},
        room="default_room",
        skip_sid=sid,
    )

if "@gemini" in user_message.lower():
    try:
        question = user_message.split("@gemini", 1)[1].strip()
        response = model.generate_content(question, stream=True)
        for chunk in response:
            if hasattr(chunk, "text") and chunk.text:
                # Relay chunk to clients
                await sio.emit("gemini_stream", {"data": chunk.text}, room="default_room")

        # Signal clients that streaming is done
        await sio.emit("stream_finished", room="default_room")

    except Exception as e:
        # If anything goes wrong with the LLM call, broadcast error
        await sio.emit("broadcast_message", {
            "user": "Gemini",
            "message": f"Error: {str(e)}"
        }, room="default_room")    

@sio.event 
# 로컬에서  socket.emit('send_image'); 이렇게 요청하면 이게 실행됨
async def send_image(sid):
    IMAGE_PATH = "win.png"
    # 이미지 파일을 바이너리 모드로 읽음
    if os.path.exists(IMAGE_PATH):
        with open(IMAGE_PATH, "rb") as f:
            image_data = f.read()
        await sio.emit('receive_image', image_data, room=sid)
    else:
        await sio.emit('receive_image', {'error': 'Image not found'}, room=sid)


// 서버에서 이미지 수신
        socket.on('receive_image', function(data) {
            const arrayBufferView = new Uint8Array(data);  // 바이너리 데이터를 Uint8Array로 변환
            const blob = new Blob([arrayBufferView], { type: "image/jpeg" });  // Blob 객체 생성
            const urlCreator = window.URL || window.webkitURL;
            const imageUrl = urlCreator.createObjectURL(blob);  // Blob을 사용해 이미지 URL 생성

            // 이미지를 화면에 표시
            document.getElementById('image').src = imageUrl;
        });
'''
app = FastAPI()
app.mount("/static", StaticFiles(directory="../public"), name="static")
socket_manager = SocketManager(app=app, mount_location="/ws")


connected_users = []
@app.sio.event
async def join_room(sid, room_name):
    app.sio.enter_room(sid, room_name)
    print(f"Client {sid} joined room: {room_name}")
    await app.sio.emit("room_joined", {"room": room_name, "sid": sid}, room=room_name) 
    # Confirm to the joining client

@app.sio.event
async def send_message_to_room(sid, data):
    message = data.get("message")
    room = data.get("room")
    if message and room:
        print(f"Emitting '{message}' to room '{room}' from {sid}")
        await app.sio.emit("room_message", {"sender_sid": sid, "message": message}, room=room)

@app.sio.on("connect")
async def connect(sid, environ):
    """Handle initial connection of socket user."""
    connected_users.append(sid)
    logger.info(f"User {sid} connected")


@app.sio.on("disconnect")
async def disconnect(sid):
    """Handle disconnection."""
    connected_users.remove(sid)
    await app.sio.emit("update-user-list", {"userIds": connected_users})
    logger.info(f"User {sid} disconnected")


@app.sio.on("requestUserList")
async def request_user_list(sid):
    """Update list of users."""
    await app.sio.emit("update-user-list", {"userIds": connected_users})
    logger.info(f"{sid} requested user list update")


@app.sio.on("mediaOffer")
async def media_offer(sid, data):
    """Handle offer to communicate."""
    await app.sio.emit(
        "mediaOffer", {"from": data["from"], "offer": data["offer"]}, room=data["to"]
    )
    logger.info(f"Media Offer from {data['from']}")


@app.sio.on("mediaAnswer")
async def media_answer(sid, data):
    """Handle media answer."""
    await app.sio.emit(
        "mediaAnswer", {"from": data["from"], "answer": data["answer"]}, room=data["to"]
    )
    logger.info(f"Media Answer from {data['from']}")


@app.sio.on("iceCandidate")
async def ice_candidate(sid, data):
    """Handle Ice Candidate."""
    await app.sio.emit(
        "remotePeerIceCandidate", {"candidate": data["candidate"]}, room=data["to"]
    )
    logger.info(f"Ice candidate for  {data['to']}")


@app.get("/")
def read_root():
    return FileResponse("../public/index.html")
    
## index.html
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="/static/index.css"/>
    <title>Video calling</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bulma@0.9.3/css/bulma.min.css">
</head>
<body>
<section class="section">
    <div class="content">

        <img src="/static/webrtc.png">
        <div class="box">
            <p>Stack: JavaScript Socket.IO, Python FastAPI, Python aiohttp, Node.JS Express.</p>
            <p>You may test Peer to Peer video call by opening another tab or opening this website on mobile phone (Safari/Chrome).
            Source code can be found <a href="https://github.com/matacoder/p2p-video-calling-app">on GitHub</a>.</p>
        </div>
        <div id="userId"></div>
        <div class="box">
            <h2>Remote Camera:</h2>
            <video id="remoteVideo" playsinline autoplay></video>
        </div>
        <div class="box">
            <h2>My camera:</h2>
            <video id="localVideo" playsinline autoplay muted></video>
        </div>

        <div class="box">
            <h2>Connected users:</h2>

            <div id="usersList">
                No users connected
            </div>

            <div>
                <button id="call">Call</button>
            </div>
        </div>

    </div>
</section>
<script src="https://cdn.socket.io/4.5.0/socket.io.min.js" integrity="sha384-7EyYLQZgWBi67fBtVxw60/OWl1kjsfrPFcaU0pp0nAh+i8FD068QogUvg85Ewy1k" crossorigin="anonymous"></script>
<script>
// Creating the peer

let selectedUser='';
const peer = new RTCPeerConnection({
  iceServers: [
    {
      urls: "stun:stun.stunprotocol.org"
    }
  ]
});

// Connecting to socket (custom path for FastAPI
const socket = io({path: '/ws/socket.io'});

const onSocketConnected = async () => {
  const constraints = {
    audio: true,
    video: true
  };
  const stream = await navigator.mediaDevices.getUserMedia(constraints);
  document.querySelector('#localVideo').srcObject = stream;
  stream.getTracks().forEach(track => peer.addTrack(track, stream));
}

let callButton = document.querySelector('#call');

// Handle call button
callButton.addEventListener('click', async () => {
  const localPeerOffer = await peer.createOffer();
  await peer.setLocalDescription(new RTCSessionDescription(localPeerOffer));  
  socket.emit('mediaOffer', {offer: localPeerOffer, from: socket.id, to: selectedUser });
});

// Create media offer
socket.on('mediaOffer', async (data) => {
  await peer.setRemoteDescription(new RTCSessionDescription(data.offer));
  const peerAnswer = await peer.createAnswer();
  await peer.setLocalDescription(new RTCSessionDescription(peerAnswer));
  socket.emit('mediaAnswer', {answer: peerAnswer, from: socket.id, to: data.from })
});

// Create media answer
socket.on('mediaAnswer', async (data) => {
  await peer.setRemoteDescription(new RTCSessionDescription(data.answer));
});

// ICE layer
peer.onicecandidate = (event) => socket.emit('iceCandidate', { to: selectedUser, candidate: event.candidate});

socket.on('remotePeerIceCandidate', async (data) => {
  try {
    const candidate = new RTCIceCandidate(data.candidate);
    await peer.addIceCandidate(candidate);
  } catch (error) {
    // Handle error, this will be rejected very often
  }
})

peer.addEventListener('track', (event) => {
  const [stream] = event.streams;
  document.querySelector('#remoteVideo').srcObject = stream;
}) 

const onUpdateUserList = ({ userIds }) => {
  const usersList = document.querySelector('#usersList');
  const usersToDisplay = userIds.filter(id => id !== socket.id);
  usersList.innerHTML = '';
  usersToDisplay.forEach(user => {
    const userItem = document.createElement('div');
    userItem.innerHTML = user;
    userItem.className = 'user-item';
    userItem.addEventListener('click', () => {
      const userElements = document.querySelectorAll('.user-item');
      userElements.forEach((element) => {
        element.classList.remove('user-item--touched');
      })
      userItem.classList.add('user-item--touched');
      selectedUser = user;
    });
    usersList.appendChild(userItem);
  });
};
socket.on('update-user-list', onUpdateUserList);

socket.on('connect', async () => {
  onSocketConnected();
  socket.emit('requestUserList');
});
</script>
</body>
</html>    