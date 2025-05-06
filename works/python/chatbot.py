from flask import Flask, render_template, request, jsonify
import requests
import uuid

app = Flask(__name__)

# MCP 서버 URL
MCP_SERVER_URL = "http://localhost:5001"

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/chat', methods=['POST'])
def chat():
    user_message = request.json.get('message', '')
    conversation_id = request.json.get('conversation_id', str(uuid.uuid4()))
    
    try:
        # MCP 서버로 요청 전송
        response = requests.post(
            f"{MCP_SERVER_URL}/chat",
            json={
                "message": user_message,
                "conversation_id": conversation_id
            }
        )
        
        if response.status_code == 200:
            data = response.json()
            return jsonify({
                "response": data["response"],
                "conversation_id": data["conversation_id"]
            })
        else:
            return jsonify({"response": "서버 오류가 발생했습니다."}), 500
            
    except requests.exceptions.RequestException:
        return jsonify({"response": "서버 연결에 실패했습니다."}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5000) 