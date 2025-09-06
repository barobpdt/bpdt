from flask import Flask, render_template
from flask import request
from flask_socketio import SocketIO, emit
import argparse
import time 
import os
import signal

class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))

global app, socketio, log, fpOut
# 인자값을 받을 수 있는 인스턴스 생성
parser = argparse.ArgumentParser(description='WebRtc Server')

# 입력받을 인자값 등록
parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()

fpOut=open(args.out, 'a', encoding='utf8')
def log (msg):
	fpOut.write(f"##> {msg}\n")
	fpOut.flush()

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your_secret_key' # Replace with a strong secret key
socketio = SocketIO(app)
PID = os.getpid()

def shutdown_server():
	'''	
	func = request.environ.get('werkzeug.server.shutdown')
	if func is None:
		raise RuntimeError('Not running with the Werkzeug Server')
	func()
	'''
	pid = os.getpid()
	log(f"print: Server PID: {pid} == {PID}")
	if fpOut:
		fpOut.close()
	os.kill(pid, signal.SIGINT)
	return 'Server shutting down...'

@app.route('/')
def index():
	log(f"print: web root file: index.html")
	return render_template('index.html')

@app.route('/shutdown', methods=['GET'])
def shutdown():
	log(f"print: Server shutting down...")
	shutdown_server()
	return 'Server shutting down...'

@socketio.on('join_room')
def handle_join_room(data):
	room = data['room']
	sid = data['sid'] # Client's SocketIO session ID
	log(f"print: Client {sid} attempting to join room: {room}")
	socketio.join_room(room, sid=sid)
	emit('joined_room', {'room': room, 'sid': sid}, room=room, include_self=True)
	log(f"print: Client {sid} joined room: {room}")

@socketio.on('offer')
def handle_offer(data):
	room = data['room']
	offer = data['offer']
	target_sid = data.get('target_sid') # Optional: specific target for offer
	log(f"print: Offer received for room {room}. Target: {target_sid}")
	if target_sid:
		emit('offer', {'offer': offer}, room=target_sid)
	else:
		emit('offer', {'offer': offer}, room=room, include_self=False)

@socketio.on('answer')
def handle_answer(data):
	room = data['room']
	answer = data['answer']
	target_sid = data.get('target_sid')
	log(f"print: Answer received for room {room}. Target: {target_sid}")
	if target_sid:
		emit('answer', {'answer': answer}, room=target_sid)
	else:
		emit('answer', {'answer': answer}, room=room, include_self=False)

@socketio.on('ice_candidate')
def handle_ice_candidate(data):
	room = data['room']
	candidate = data['candidate']
	target_sid = data.get('target_sid')
	log(f"print: ICE candidate received for room {room}. Target: {target_sid}")
	if target_sid:
		emit('ice_candidate', {'candidate': candidate}, room=target_sid)
	else:
		emit('ice_candidate', {'candidate': candidate}, room=room, include_self=False)

if __name__ == '__main__':
	socketio.run(app, debug=False, host='0.0.0.0', port=8090, allow_unsafe_werkzeug=True)