#Socket_io.py file
import uvicorn
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import socketio
# from starlette.responses import FileResponse
# from starlette.staticfiles import StaticFiles

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # This allows all origins, you can specify certain domains here
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

sio = socketio.AsyncServer(async_mode='asgi')
sio_app = socketio.ASGIApp(sio) # , socketio_path='/ws/socket.io'
app.mount("/ws/socket.io", sio_app)


@app.get("/test")
def read_root():
    return {"Hello": "World"}

@sio.on("connect")
async def connect(sid, env):
    print("New Client Connected to This id :"+" "+str(sid))

@sio.on("disconnect")
async def disconnect(sid):
    print("Client Disconnected: "+" "+str(sid))


@sio.event
async def message(sid, data):
    print(f"Message from {sid}: {data}")
    await sio.emit('response', {'data': data})
	
if __name__=="__main__":
	uvicorn.run("server:app", host="0.0.0.0", port=7777, lifespan="on", reload=False)