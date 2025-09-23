#Socket_io.py file
import uvicorn
from fastapi import FastAPI
from fastapi_socketio import SocketManager
from starlette.responses import FileResponse
from starlette.staticfiles import StaticFiles


app = FastAPI()
# app.mount("/static", StaticFiles(directory="../public"), name="static")
socket_manager = SocketManager(app=app, mount_location="/ws")

@app.get("/")
def read_root():
    return {"Hello": "World"}

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
    print(f"User {sid} connected")


@app.sio.on("disconnect")
async def disconnect(sid):
    """Handle disconnection."""
    connected_users.remove(sid)
    await app.sio.emit("update-user-list", {"userIds": connected_users})
    print(f"User {sid} disconnected")
	
if __name__=="__main__":
	uvicorn.run("server:app", host="0.0.0.0", port=7777, lifespan="on", reload=False)