## WebRTC 간단한 예제
	<!doctype html>
	<html>
	  <head>
		<title>WebRTC WebCam</title>
		<link rel="stylesheet" href="webcam.css"/>
		<script>
		(function(){
		  var mediaOptions = { audio: false, video: true };

		  if (!navigator.getUserMedia) {
			  navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;
		  }

		  if (!navigator.getUserMedia){
			return alert('getUserMedia not supported in this browser.');
		  }

		  navigator.getUserMedia(mediaOptions, success, function(e) {
			console.log(e);
		  });

		  function success(stream){
			var video = document.querySelector("#player");
			video.src = window.URL.createObjectURL(stream);
		  }
		})();
		</script>
	  </head>
	  <video id="player" autoplay="true"></video>
	</html>


	from flask import Flask, request, jsonify
	app = Flask(__name__)

	@app.route('/api/offer', methods=['POST'])
	def receive_offer():
		offer = request.json['offer']
		print("Received offer:", offer)
		# Process offer and generate answer
		return jsonify(answer="Here is the answer")
		
	@app.route('/offer', methods=['POST'])
	def handle_offer():
		offer = request.json['sdp']
		pc = RTCPeerConnection()
		pc.setRemoteDescription(offer)
		# Create an answer
		answer = pc.createAnswer()
		pc.setLocalDescription(answer)
		return jsonify({'sdp': pc.localDescription.sdp})


	if __name__ == '__main__':
		app.run(debug=True, port=5000)

## react 구현
	<script react>
	import React, { useRef } from 'react';

	function App() {
		const localVideoRef = useRef();
		const remoteVideoRef = useRef();

		// More React code to handle video streams
		const pc = new RTCPeerConnection();

		pc.ontrack = (event) => {
			if (event.streams && event.streams[0]) {
				remoteVideoRef.current.srcObject = event.streams[0];
			}
		};

		// Function to start the connection
		function startConnection() {
			navigator.mediaDevices.getUserMedia({ video: true, audio: true })
				.then(stream => {
					localVideoRef.current.srcObject = stream;
					stream.getTracks().forEach(track => pc.addTrack(track, stream));
				});
			// Signaling code to exchange offers/answers and candidates
		}
		return (
			<div>
				<video ref={localVideoRef} autoPlay playsInline />
				<video ref={remoteVideoRef} autoPlay playsInline />
			</div>
		);
	}

	export default App;
	</script>

## 파이션 WebRTC 구현
	<html>
	<head>
		<title>WebRTC Video Chat</title>
		<style>
			video {
				width: 48%;
				height: auto;
				border: 1px solid black;
				margin: 5px;
			}
		</style>
	</head>
	<body>
		<h1>WebRTC Video Chat</h1>
		<input type="text" id="roomInput" placeholder="Enter Room ID">
		<button onclick="joinRoom()">Join Room</button>
		<br>
		<video id="localVideo" autoplay muted playsinline></video>
		<video id="remoteVideo" autoplay playsinline></video>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/socket.io/4.0.0/socket.io.js"></script>
		<script>
			const socket = io();
			socket.on('joined_room', async (data) => {
				console.log('Joined room:', data.room);
				if (data.sid !== socket.id) { // If another user joined the room
					await createPeerConnection(data.sid);
					await createOffer();
				}
			});
			socket.on('offer', async (data) => {
				await createPeerConnection(data.offer.sdp.split('a=candidate')[0]); // Extract remote SID from offer (simplified)
				await peerConnection.setRemoteDescription(new RTCSessionDescription(data.offer));
				const answer = await peerConnection.createAnswer();
				await peerConnection.setLocalDescription(answer);
				socket.emit('answer', { room: currentRoom, answer: answer, target_sid: data.offer.sdp.split('a=candidate')[0] }); // Send answer to specific peer
			});

			socket.on('answer', async (data) => {
				await peerConnection.setRemoteDescription(new RTCSessionDescription(data.answer));
			});

			socket.on('ice_candidate', async (data) => {
				await peerConnection.addIceCandidate(new RTCIceCandidate(data.candidate));
			});

			const localVideo = document.getElementById('localVideo');
			const remoteVideo = document.getElementById('remoteVideo');
			const roomInput = document.getElementById('roomInput');
			let localStream;
			let peerConnection;
			let currentRoom;

			const configuration = {
				iceServers: [{ urls: 'stun:stun.l.google.com:19302' }]
			};
			async function joinRoom() {
				currentRoom = roomInput.value;
				if (!currentRoom) {
					alert('Please enter a room ID.');
					return;
				}
				try {
					localStream = await navigator.mediaDevices.getUserMedia({ video: true, audio: true });
					localVideo.srcObject = localStream;
					socket.emit('join_room', { room:currentRoom, sid:socket.id });
				} catch (error) {
					console.error('Error accessing media devices:', error);
				}
			} 
			async function createPeerConnection(remoteSocketId) {
				peerConnection = new RTCPeerConnection(configuration);
				localStream.getTracks().forEach(track => {
					peerConnection.addTrack(track, localStream);
				});
				peerConnection.ontrack = (event) => {
					remoteVideo.srcObject = event.streams[0];
				};
				peerConnection.onicecandidate = (event) => {
					if (event.candidate) {
						socket.emit('ice_candidate', {
							room: currentRoom,
							candidate: event.candidate,
							target_sid: remoteSocketId // Send candidate to specific peer
						});
					}
				};
			}

			async function createOffer() {
				const offer = await peerConnection.createOffer();
				await peerConnection.setLocalDescription(offer);
				socket.emit('offer', { room: currentRoom, offer: offer, target_sid: getOtherPeerId() });
			}
			
			function getOtherPeerId() {
				// In a simple two-person chat, you'd need a way to know the other peer's ID.
				// This is a placeholder for a more robust signaling mechanism.
				// For example, the server could send the other peer's ID in 'joined_room'.
				return null; // This needs to be implemented to target the correct peer
			}
		</script>
	</body>
	</html>
	
	
	from flask import Flask, render_template
	from flask_socketio import SocketIO, emit
	import logging

	app = Flask(__name__)
	app.config['SECRET_KEY'] = 'your_secret_key' # Replace with a strong secret key
	socketio = SocketIO(app)

	# Configure logging
	logging.basicConfig(level=logging.INFO)

	@app.route('/')
	def index():
		return render_template('index.html')

	@socketio.on('join_room')
	def handle_join_room(data):
		room = data['room']
		sid = data['sid'] # Client's SocketIO session ID
		logging.info(f"Client {sid} attempting to join room: {room}")
		socketio.join_room(room, sid=sid)
		emit('joined_room', {'room': room, 'sid': sid}, room=room, include_self=True)
		logging.info(f"Client {sid} joined room: {room}")

	@socketio.on('offer')
	def handle_offer(data):
		room = data['room']
		offer = data['offer']
		target_sid = data.get('target_sid') # Optional: specific target for offer
		logging.info(f"Offer received for room {room}. Target: {target_sid}")
		if target_sid:
			emit('offer', {'offer': offer}, room=target_sid)
		else:
			emit('offer', {'offer': offer}, room=room, include_self=False)

	@socketio.on('answer')
	def handle_answer(data):
		room = data['room']
		answer = data['answer']
		target_sid = data.get('target_sid')
		logging.info(f"Answer received for room {room}. Target: {target_sid}")
		if target_sid:
			emit('answer', {'answer': answer}, room=target_sid)
		else:
			emit('answer', {'answer': answer}, room=room, include_self=False)

	@socketio.on('ice_candidate')
	def handle_ice_candidate(data):
		room = data['room']
		candidate = data['candidate']
		target_sid = data.get('target_sid')
		logging.info(f"ICE candidate received for room {room}. Target: {target_sid}")
		if target_sid:
			emit('ice_candidate', {'candidate': candidate}, room=target_sid)
		else:
			emit('ice_candidate', {'candidate': candidate}, room=room, include_self=False)

	if __name__ == '__main__':
		socketio.run(app, debug=True, port=5000)

## Python SocketIO 간단한 서버 만들기
	import socketio

	# Socket.IO 서버 생성
	sio = socketio.Server()

	# 연결 이벤트 핸들러
	@sio.event
	def connect(sid, environ):
		print('클라이언트가 연결되었습니다:', sid)

	# 메시지 이벤트 핸들러
	@sio.event
	def message(sid, data):
		print('클라이언트로부터 메시지 수신:', data)

	# 실행할 때 웹 소켓 서버 시작
	if __name__ == '__main__':
		app = socketio.WSGIApp(sio)
		socketio.server.SocketIOServer(('localhost', 8000), app).serve_forever()
