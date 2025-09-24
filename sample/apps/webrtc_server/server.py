import asyncio
from aiortc import RTCPeerConnection, RTCSessionDescription, MediaStreamTrack
from aiortc.contrib.media import MediaRecorder, MediaBlackhole
from flask import Flask, render_template, request, jsonify
import cv2
import uuid
import logging
import os
import signal

from fastapi.responses import StreamingResponse
import subprocess

from av import VideoFrame
import argparse

## pip install aiortc==1.5.0
## pip install Flask
## pip install opencv-python

global fpOut, log

fpOut = None
def log(msg):
	if fpOut:
		fpOut.write(f"##> {msg}\n")
		fpOut.flush()
	else:
		print(f"##> {msg}")

app = Flask(__name__)
logger = logging.getLogger("pc")
pcs = set()
# relay = MediaRelay()

'''
# 비디오 파일 경로 및 코덱 설정
fourcc = cv2.VideoWriter_fourcc(*'mp4v') # MP4 코덱 지정
fps = 20.0 # 초당 프레임 수
frame_size = (640, 480) # 프레임 해상도 (너비, 높이)
output_file = 'output.mp4' # 저장할 파일 이름

# VideoWriter 객체 생성
out = cv2.VideoWriter(output_file, fourcc, fps, frame_size)

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break
    # 원하는 이미지 처리 (예: 흑백 변환)
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    # 흑백 프레임을 컬러로 다시 변환하여 저장
    gray_frame_color = cv2.cvtColor(gray_frame, cv2.COLOR_GRAY2BGR)
    # 처리된 프레임 저장
    out.write(gray_frame_color)
	
cap.release()
out.release()	
'''

@app.get("/video_stream")
async def video_stream():
	def generate_video_stream():
		# FFmpeg command to process and stream the video
		command = [
			"ffmpeg",
			"-i", "input.mp4",  # Replace with your input source (file, camera, etc.)
			"-f", "mp4",       # Output format (e.g., mp4, webm)
			"-movflags", "frag_keyframe+empty_moov", # Essential for fragmented MP4 streaming
			"-preset", "ultrafast", # Adjust for performance vs quality
			"-c:v", "libx264", # Video codec
			"-crf", "23",      # Constant Rate Factor for quality (lower is better)
			"-r", "30",        # Frame rate
			"-s", "640x480",   # Resolution
			"pipe:1"           # Output to stdout
		]
		process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

		try:
			while True:
				chunk = process.stdout.read(4096)  # Read in chunks
				if not chunk:
					break
				yield chunk
		finally:
			process.terminate()
			process.wait()

	return StreamingResponse(generate_video_stream(), media_type="video/mp4")

# add transformation filters on video track
class VideoTransformTrack(MediaStreamTrack):
	kind = "video"
	def __init__(self, track, transform):
		super().__init__()
		self.track = track
		self.transform = transform

	async def recv(self):
		frame = await self.track.recv()

		if self.transform == "cartoon":
			img = frame.to_ndarray(format="bgr24")

			# prepare color
			img_color = cv2.pyrDown(cv2.pyrDown(img))
			for _ in range(6):
				img_color = cv2.bilateralFilter(img_color, 9, 9, 7)
			img_color = cv2.pyrUp(cv2.pyrUp(img_color))

			# prepare edges
			img_edges = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
			img_edges = cv2.adaptiveThreshold(
				cv2.medianBlur(img_edges, 7),
				255,
				cv2.ADAPTIVE_THRESH_MEAN_C,
				cv2.THRESH_BINARY,
				9,
				2,
			)
			img_edges = cv2.cvtColor(img_edges, cv2.COLOR_GRAY2RGB)

			# combine color and edges
			img = cv2.bitwise_and(img_color, img_edges)

			# rebuild a VideoFrame, preserving timing information
			new_frame = VideoFrame.from_ndarray(img, format="bgr24")
			new_frame.pts = frame.pts
			new_frame.time_base = frame.time_base
			return new_frame
		elif self.transform == "edges":
			# perform edge detection
			img = frame.to_ndarray(format="bgr24")
			img = cv2.cvtColor(cv2.Canny(img, 100, 200), cv2.COLOR_GRAY2BGR)

			# rebuild a VideoFrame, preserving timing information
			new_frame = VideoFrame.from_ndarray(img, format="bgr24")
			new_frame.pts = frame.pts
			new_frame.time_base = frame.time_base
			return new_frame
		elif self.transform == "rotate":
			# rotate image
			img = frame.to_ndarray(format="bgr24")
			rows, cols, _ = img.shape
			M = cv2.getRotationMatrix2D((cols / 2, rows / 2), frame.time * 45, 1)
			img = cv2.warpAffine(img, M, (cols, rows))

			# rebuild a VideoFrame, preserving timing information
			new_frame = VideoFrame.from_ndarray(img, format="bgr24")
			new_frame.pts = frame.pts
			new_frame.time_base = frame.time_base
			return new_frame
		else:
			return frame

class OpenCVMediaStreamTrack(MediaStreamTrack):
	kind = "video"	
	def __init__(self, device_index=0):
		super().__init__()
		self.cam = cv2.VideoCapture(device_index)

	async def recv(self):
		success, frame = self.cam.read()
		if not success:
			raise Exception("Failed to capture frames")
		
		# Convert the image from OpenCV format to AV format
		frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
		frame = VideoFrame.from_ndarray(frame, format="rgb24")
		frame.pts = frame.time_base = None
		return frame

@app.route('/')
def index():
	log(f"page index start")
	return render_template('index.html')

@app.route('/offer', methods=['POST'])
async def offer():
	log(f'Offer start')
	params = request.get_json() # synchronous
	log(f"Offer received params: {params}")
	if not params:
		return jsonify({"error": "Invalid JSON data"}), 400
	offer = RTCSessionDescription(sdp=params["sdp"], type=params["type"])
	pc = RTCPeerConnection()
	pc_id = "PeerConnection(%s)" % uuid.uuid4()
	pcs.add(pc)

	log(f"Offer Created pc:{pc_id} for {request.remote_addr}")

	# player = MediaPlayer('video=Integrated Camera', format='dshow', options={'frame_rate': "60", 'video_size': '640x480'})
	if args.record_to:
		recorder = MediaRecorder(args.record_to)
	else:
		recorder = MediaBlackhole()

	@pc.on("connectionstatechange")
	async def on_connectionstatechange():
		log(f"Connection state is {pc.connectionState}")
		if pc.connectionState == "failed":
			await pc.close()
			pcs.discard(pc)

	@pc.on("track")
	async def on_track(track):
		log(f"Track {track.kind} received")
		
		if track.kind == "audio":
			pc.addTrack(track)
			recorder.addTrack(track)
		elif track.kind == "video":
			# pc.addTrack(VideoTransformTrack(relay.subscribe(track)), transform=params["video_transform"])
			video_track = OpenCVMediaStreamTrack(device_index=0)
			pc.addTrack(VideoTransformTrack(video_track, transform=params.get("video_transform", "")))
			if args.record_to:
				recorder.addTrack(video_track)
		
		@track.on("ended")
		async def on_ended():
			log(f"Track {track.kind} ended")
			await recorder.stop()

	# handle offer
	await pc.setRemoteDescription(offer)
	await recorder.start()

	# send answer
	answer = await pc.createAnswer()
	await pc.setLocalDescription(answer)

	# Handle offer
	# loop = asyncio.new_event_loop()
	# asyncio.set_event_loop(loop)
	# loop.run_until_complete(pc.setRemoteDescription(offer))
	# loop.run_until_complete(recorder.start())

	# # Send answer
	# answer = loop.run_until_complete(pc.createAnswer())
	# loop.run_until_complete(pc.setLocalDescription(answer))
	
	return jsonify({"sdp": pc.localDescription.sdp, "type": pc.localDescription.type})

'''
@app.teardown_appcontext
def on_shutdown(exc):
	log("Server on_shutdown...")
	loop = asyncio.new_event_loop()
	asyncio.set_event_loop(loop)
	# close peer connections
	coros = [pc.close() for pc in pcs]
	asyncio.get_event_loop().run_until_complete(asyncio.gather(*coros))
	pcs.clear()
	loop.close()
'''

@app.route('/echo', methods=['POST'])
def echo():
	data = request.get_json()
	return jsonify(data)
	
@app.route('/test')
def test():
	return "Test successful"

@app.route('/shutdown', methods=['GET'])
def shutdown():
	log("Server shutting down...")
	if fpOut:
		fpOut.close()
	os.kill(os.getpid(), signal.SIGINT)
	return "Server shutting down..."

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="WebRTC+Flask Live Streaming Application")
	parser.add_argument("--host", default="0.0.0.0", help="Host for HTTP server")
	parser.add_argument("--port", type=int, default=8090, help="Port for HTTP server")
	parser.add_argument("--record_to", help="Write received media to a file")
	parser.add_argument("--verbose", "-v", action="count")
	parser.add_argument('--out', help='로그 출력파일')

	args = parser.parse_args()

	if args.out:
		fpOut=open(args.out, 'a', encoding='utf8')
	else:
		fpOut=None
	
	app.run(host=args.host, port=args.port)
