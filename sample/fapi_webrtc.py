from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import socketio
import uvicorn

app = FastAPI()
mgr = socketio.AsyncManager() 
sio = socketio.AsyncServer(
    async_mode="asgi", cors_allowed_origins="*", client_manager=mgr
)
sio_asgi_app = socketio.ASGIApp(socketio_server=sio, other_asgi_app=app)

app.add_route("/socket.io/", route=sio_asgi_app, methods=["GET", "POST"])
app.add_websocket_route("/socket.io/", sio_asgi_app)

rooms = {}

@sio.event
async def joinRoom(id,roomid,userid):
    sio.enter_room(id,roomid)

    message = userid + " has joined the room : "+roomid

    if roomid not in rooms:
        rooms[roomid] = {}
    
    rooms[roomid][userid] = id
    print(list(rooms[roomid].keys()))
    await sio.emit("userList",message,room=roomid)

@sio.event
async def signalingMessage(sid,message,roomid):
    if(roomid):
        await sio.emit('signalingMessage',[message,roomid],room=roomid,skip_sid=sid)

@sio.on('disconnect')
async def handle_disconnect(sid):
    print(f"disconnected {sid}")
    roomid = ""
    userid = ""
    for outerKey,outerDict in rooms.items():
        for innerKey, innervalue in outerDict.items():
            if innervalue == sid:
                del rooms[outerKey][innerKey]
                roomid = outerKey
                userid = innerKey
                print("deleted - ",innerKey)
                break
    
    message = userid + " has left the room."
    await sio.emit("userList",message,room=roomid)

@app.get('/')
def index():
    return "This is a server"

if __name__ == "__main__":
    uvicorn.run('app:app', host=ENV.HOST, port=8000,reload=True)


<script>
const socket = io(ENV.SERVER_URL,{path:'/api/socket.io/'});

let localStream=null;
let remoteStream=null;
let peerConnection;
let guserid;
let groomid;

const servers = {
    iceServers:[
        {
            urls:['stun:stun1.l.google.com:19302','stun:stun2.l.google.com:19302']
        },
    ],
}

async function joinRoom(){
    const local = document.getElementById('local');
    const roomid = document.getElementById('roomid').value;
    const startcall = document.getElementById('startcall');
    const userid = document.getElementById('userid').value;

    guserid = userid;
    groomid = roomid;

    if( roomid === '' || userid === ''){
        alert("Please enter all fields")
        return;
    }
    localStream = await navigator.mediaDevices.getUserMedia({video:{width:200,height:200},audio:true});

    local.srcObject = localStream;
    startcall.disabled = false;

    socket.emit('joinRoom',roomid,userid);
    document.getElementById('roomid').value = '';
    document.getElementById('userid').value = '';
}

function leaveRoom(){
    peerConnection.close();
    peerConnection = null;
    document.getElementById('remote').srcObject = null;
    socket.emit('signalingMessage',{type:'hangup'},groomid);
}

function startCall(){
    const remote = document.getElementById('remote');
    const userid = guserid;
    const roomid = groomid;

    peerConnection = new RTCPeerConnection(servers);

    localStream.getTracks().forEach(track=>peerConnection.addTrack(track,localStream));
    
    peerConnection.ontrack = event =>{
        remoteStream = event.streams[0];
        remote.srcObject = remoteStream;
        document.getElementById('hangup').disabled=false;
    }

    peerConnection.onicecandidate = event => {
        if(event.candidate){
            socket.emit('signalingMessage',{candidate:event.candidate},roomid)
        }
    }

    peerConnection.createOffer().then(offer=>{
        peerConnection.setLocalDescription(offer);
        socket.emit('signalingMessage',{offer},roomid);
    })
}

socket.on('signalingMessage',async ([message,roomid])=>{

    const remote = document.getElementById('remote');

    if(message.offer){
        peerConnection = new RTCPeerConnection(servers);
        localStream.getTracks().forEach(track=>peerConnection.addTrack(track,localStream));
        
        peerConnection.ontrack = event =>{
            remoteStream = event.streams[0];
            remote.srcObject = remoteStream;
            document.getElementById('hangup').disabled=false;
        }
        
        await peerConnection.setRemoteDescription(message.offer);
        const answer = await peerConnection.createAnswer();
        await peerConnection.setLocalDescription(answer);
        socket.emit('signalingMessage',{answer},roomid);

        peerConnection.onicecandidate = event=>{
            if(event.candidate){
                socket.emit('signalingMessage',{candidate:event.candidate},roomid);
            }
        }

    } else if(message.answer){
        await peerConnection.setRemoteDescription(message.answer);
    } else if(message.candidate){
        await peerConnection.addIceCandidate(message.candidate);
    } else if(message.type === 'hangup'){
        peerConnection.close();
        peerConnection = null;
        document.getElementById('remote').srcObject = null;
    }
})

socket.on("userList",msg=>{
    
    const cont = document.getElementById('cont');
    const child = document.createElement('div');
    child.textContent = msg;
    child.className = "username";
    cont.appendChild(child);

})

function appendMessage(msg){
    const cont = document.getElementById('cont');
    const child = document.createElement('div');
    child.textContent = msg;
    child.className = "username";
    cont.appendChild(child);
}
</script> 

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Video Calling App</title>
  <style>

    .box{
      margin-top: 100px;
      margin-left: 150px;
      display: flex;
      height: 500px;
    }
    .names{
      width: 200px;
      border: 1px solid black;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
    }
    .right{
      width: 60%;
      display: flex;
      flex-direction: column;
      border: 1px solid black;
      border-left: none;
    }
    .cont{
      flex:9;
    }
    .bottom{
      flex:1;
      height: 20%;
      border-top: 1px solid black;
      display: flex;
    }
    .username{
      border-bottom: 1px solid gray;
      padding: 10px;
    }
    #startcall{
      background-color: green;
      color: white;
      margin-left: auto;
    }
    #hangup{
      background-color: red;
      color: white;
      margin-left: 2px;
    }
    #local,#remote{
      background-color:rgb(190, 190, 190);
    }
    #local{
      border-top: 1px solid black;
    }
    #remote{
      border-bottom: 1px solid black;
    }

  </style>
</head>
<body>
  
  <script src="https://cdn.socket.io/4.6.0/socket.io.min.js"></script>
  <script src="script.js"></script>
  <div id="root"></div>

  <div class="box">
    
    <div id="names" class="names">
      <video id="remote" width="200" height="200" autoplay></video>
      <video id="local" width="200" height="200" autoplay muted></video>
    </div>
    
    <div class="right">
      
      <div id="cont" class="cont">
      </div>
      <div class="bottom">
        <input type="text" id="userid" placeholder="Enter your Name">
        <input type="text" id="roomid" placeholder="Enter Room ID">
        <button onclick="joinRoom()">Join Room</button>
        <!-- <button onclick="leaveRoom()">Leave Room</button> -->
        <button id="startcall" onclick="startCall()" disabled>Start Call</button>
        <button id="hangup" onclick="leaveRoom()" disabled>Leave</button> 
      </div>
    </div>
  </div>

</body>
</html>