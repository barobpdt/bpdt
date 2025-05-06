from flask import Flask, request, jsonify
import json
import random
from datetime import datetime
import threading
import time
import os
from dotenv import load_dotenv
from langchain.chat_models import ChatOpenAI
from langchain.memory import ConversationBufferMemory
from langchain.chains import ConversationChain
from langchain.prompts import PromptTemplate
from news_collector import NewsCollector
from youtube_searcher import YouTubeSearcher

# 환경 변수 로드
load_dotenv()

app = Flask(__name__)

# 대화 컨텍스트 저장소
conversations = {}

# 뉴스 수집기 초기화
news_collector = NewsCollector()

# YouTube 검색기 초기화
youtube_searcher = YouTubeSearcher()

# LangChain 설정
template = """당신은 친절하고 도움이 되는 AI 어시스턴트입니다. 
이전 대화를 고려하여 자연스럽게 대화를 이어가세요.
뉴스 정보와 YouTube 동영상 정보를 활용하여 사용자의 질문에 답변하세요.

이전 대화:
{history}

현재 대화:
Human: {input}

AI:"""

prompt = PromptTemplate(
    input_variables=["history", "input"],
    template=template
)

def get_conversation_chain(conversation_id):
    if conversation_id not in conversations:
        # 새로운 대화 체인 생성
        llm = ChatOpenAI(temperature=0.7)
        memory = ConversationBufferMemory(return_messages=True)
        conversation = ConversationChain(
            llm=llm,
            memory=memory,
            prompt=prompt,
            verbose=True
        )
        conversations[conversation_id] = {
            "chain": conversation,
            "last_active": datetime.now(),
            "messages": []
        }
    return conversations[conversation_id]

def clean_old_conversations():
    while True:
        current_time = datetime.now()
        to_delete = []
        for conv_id, conv in conversations.items():
            if (current_time - conv["last_active"]).total_seconds() > 3600:  # 1시간
                to_delete.append(conv_id)
        
        for conv_id in to_delete:
            del conversations[conv_id]
        
        time.sleep(300)  # 5분마다 실행

def update_news_periodically():
    while True:
        news_collector.update_news()
        time.sleep(3600)  # 1시간마다 업데이트

def get_news_context(message: str) -> str:
    """메시지와 관련된 뉴스 정보를 검색하고 컨텍스트로 반환"""
    try:
        articles = news_collector.search_articles(message, n_results=3)
        if not articles:
            return ""
        
        context = "관련 뉴스 정보:\n"
        for article in articles:
            context += f"- {article['title']}\n"
            context += f"  요약: {article['content'][:200]}...\n"
            context += f"  출처: {article['url']}\n\n"
        
        return context
    except Exception as e:
        print(f"Error getting news context: {str(e)}")
        return ""

def get_youtube_context(message: str) -> str:
    """메시지와 관련된 YouTube 동영상 정보를 검색하고 컨텍스트로 반환"""
    try:
        # 일반 검색
        videos = youtube_searcher.search_videos(message, n_results=3)
        
        # 인기 동영상 검색 (관련성이 높을 경우)
        trending_videos = youtube_searcher.get_trending_videos(n_results=3)
        
        context = "관련 YouTube 동영상 정보:\n"
        
        if videos:
            context += "\n검색 결과:\n"
            for video in videos:
                context += f"- {video['title']}\n"
                context += f"  채널: {video['channel_title']}\n"
                context += f"  조회수: {video['view_count']}\n"
                context += f"  URL: {video['url']}\n\n"
        
        if trending_videos:
            context += "\n인기 동영상:\n"
            for video in trending_videos:
                context += f"- {video['title']}\n"
                context += f"  채널: {video['channel_title']}\n"
                context += f"  조회수: {video['view_count']}\n"
                context += f"  URL: {video['url']}\n\n"
        
        return context
    except Exception as e:
        print(f"Error getting YouTube context: {str(e)}")
        return ""

@app.route('/chat', methods=['POST'])
def chat():
    data = request.json
    conversation_id = data.get('conversation_id', 'default')
    message = data.get('message', '')
    
    conversation = get_conversation_chain(conversation_id)
    conversation["last_active"] = datetime.now()
    
    try:
        # 뉴스 컨텍스트 가져오기
        news_context = get_news_context(message)
        
        # YouTube 컨텍스트 가져오기
        youtube_context = get_youtube_context(message)
        
        # LangChain을 사용하여 응답 생성
        response = conversation["chain"].predict(
            input=f"{news_context}\n{youtube_context}\n{message}"
        )
        
        # 대화 기록 저장
        conversation["messages"].append({
            "user": message,
            "bot": response,
            "timestamp": datetime.now().isoformat()
        })
        
        return jsonify({
            "response": response,
            "conversation_id": conversation_id
        })
    except Exception as e:
        print(f"Error: {str(e)}")
        return jsonify({
            "response": "죄송합니다. 응답을 생성하는 중에 오류가 발생했습니다.",
            "conversation_id": conversation_id
        }), 500

@app.route('/history/<conversation_id>', methods=['GET'])
def get_history(conversation_id):
    if conversation_id in conversations:
        return jsonify(conversations[conversation_id]["messages"])
    return jsonify([])

if __name__ == '__main__':
    # OpenAI API 키 확인
    if not os.getenv("OPENAI_API_KEY"):
        print("Error: OPENAI_API_KEY environment variable is not set")
        exit(1)
    
    # YouTube API 키 확인
    if not os.getenv("YOUTUBE_API_KEY"):
        print("Error: YOUTUBE_API_KEY environment variable is not set")
        exit(1)
    
    # 오래된 대화 정리 스레드 시작
    cleaner_thread = threading.Thread(target=clean_old_conversations, daemon=True)
    cleaner_thread.start()
    
    # 뉴스 업데이트 스레드 시작
    news_update_thread = threading.Thread(target=update_news_periodically, daemon=True)
    news_update_thread.start()
    
    app.run(debug=True, port=5001) 