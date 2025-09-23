import uvicorn
from fastapi import FastAPI
from starlette.responses import FileResponse
from fastapi.staticfiles import StaticFiles  
from fastapi.middleware.cors import CORSMiddleware
import socketio 
import os

app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
    expose_headers=["Content-Disposition"],
)
pid = os.getpid()

print(f"###### server start PID:{pid} ######")

'''
@app.middleware("http")
async def cors_middleware(request call_next):
    if request.url.path == "/ws":
        # Exclude the /ws route from CORS headers
        response = await call_next(request)
    else:
        # Apply CORS headers for other routes
        response = await call_next(request)
        response.headers["Access-Control-Allow-Origin"] = "*"
        response.headers["Access-Control-Allow-Headers"] = "*"
        response.headers["Access-Control-Allow-Methods"] = "*"
    return response
'''
sio = socketio.AsyncServer(async_mode='asgi', cors_allowed_origins='*')
sio_app = socketio.ASGIApp(sio, socketio_path='/ws/socket.io') 
app.mount("/ws", sio_app)

@app.get("/")
def read_root():
    return FileResponse('./templates/xx.html')


@app.get("/test")
def read_root():
    return {"Hello": "World"}

@sio.event
async def connect(sid, environ, auth):
    print(f'connected auth={auth} sid={sid}')
    # active_connections.append(sid)
    await sio.emit('chat', {'data': 'Connected', 'sid': sid}, room=sid)


@sio.event
def disconnect(sid):
    print('disconnected', sid)
    # active_connections.remove(sid)

@sio.on('query')
async def test_message(sid, message):
    print(message)
    await sio.emit('chat', {'data': message + " -Interaction Engine"}, room=sid)

if __name__=="__main__":
	uvicorn.run("server-file:app", host="0.0.0.0", port=7777, lifespan="on", reload=False)
     

