## websocket app
from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from websockets.exceptions import ConnectionClosed
from fastapi.templating import Jinja2Templates
import uvicorn
import asyncio
import cv2
app = FastAPI()
camera = cv2.VideoCapture(0,cv2.CAP_DSHOW)
templates = Jinja2Templates(directory="templates")

@app.get('/')
async def index(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

@app.websocket("/ws")
async def get_stream(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            success, frame = camera.read()
            if not success:
                break
            else:
                ret, buffer = cv2.imencode('.jpg', frame)
                await websocket.send_bytes(buffer.tobytes()) 
            await asyncio.sleep(0.03)
    except (WebSocketDisconnect, ConnectionClosed):
        print("Client disconnected")   

if __name__ == '__main__':
    uvicorn.run(app, host='127.0.0.1', port=8000)

## pyclient.py
from websockets.exceptions import ConnectionClosed
import websockets
import numpy as np
import asyncio
import cv2
async def main():
    url = 'ws://127.0.0.1:8000/ws'    
    async for websocket in websockets.connect(url):
        try:
            #count = 1
            while True:
                contents = await websocket.recv()
                arr = np.frombuffer(contents, np.uint8)
                frame = cv2.imdecode(arr, cv2.IMREAD_UNCHANGED)
                cv2.imshow('frame', frame)
                cv2.waitKey(1)                
                #cv2.imwrite("frame%d.jpg" % count, frame)
                #count += 1
        except ConnectionClosed:
            continue  # attempt reconnecting to the server (otherwise, call `break` instead)

asyncio.run(main())

## index.html
<!DOCTYPE html>
<html>
    <head>
        <title>Live Streaming</title>
    </head>
    <body>
        <img id="frame" src="">
        <script>
            let ws = new WebSocket("ws://localhost:8000/ws");
            let image = document.getElementById("frame");
            image.onload = function(){
                URL.revokeObjectURL(this.src); // release the blob URL once the image is loaded
            } 
            ws.onmessage = function(event) {
                image.src = URL.createObjectURL(event.data);
            };
        </script>
    </body>
</html>



## 방식2 ================= read Image ===================
import cv2
import time
import uvicorn
from fastapi import FastAPI, Request
from fastapi.templating import Jinja2Templates
from fastapi.responses import StreamingResponse
app = FastAPI()
camera = cv2.VideoCapture(0, cv2.CAP_DSHOW)
templates = Jinja2Templates(directory="templates")
def gen_frames():
    while True:
        success, frame = camera.read()
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
        time.sleep(0.03)


@app.get('/')
async def index(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})


@app.get('/video_feed')
def video_feed():
    return StreamingResponse(gen_frames(), media_type='multipart/x-mixed-replace; boundary=frame')


if __name__ == '__main__':
    uvicorn.run(app, host='127.0.0.1', port=8000, debug=True)
    
## index.html
<!DOCTYPE html>
<html>
    <body>
        <div class="container">
            <h3> Live Streaming </h3>
            <img src="{{ url_for('video_feed') }}" width="50%">
        </div>
    </body>
</html>

