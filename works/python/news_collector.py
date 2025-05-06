import chromadb
from newspaper import Article
import json
from datetime import datetime
import os
from typing import List, Dict
import time

class NewsCollector:
    def __init__(self, db_path: str = "news_db"):
        self.client = chromadb.PersistentClient(path=db_path)
        self.collection = self.client.get_or_create_collection(
            name="news_articles",
            metadata={"hnsw:space": "cosine"}
        )
        
        # 뉴스 소스 설정
        self.news_sources = {
            "naver": "https://news.naver.com/main/main.naver?mode=LSD&mid=shm&sid1=100",
            "daum": "https://news.daum.net/breakingnews/digital",
            "khan": "https://www.khan.co.kr/technology/"
        }
    
    def fetch_article(self, url: str) -> Dict:
        try:
            article = Article(url)
            article.download()
            article.parse()
            
            return {
                "title": article.title,
                "text": article.text,
                "url": url,
                "published_date": article.publish_date.isoformat() if article.publish_date else datetime.now().isoformat(),
                "keywords": article.keywords,
                "summary": article.summary
            }
        except Exception as e:
            print(f"Error fetching article {url}: {str(e)}")
            return None
    
    def store_article(self, article: Dict):
        if not article:
            return
        
        # 문서 ID 생성 (URL의 해시값 사용)
        doc_id = str(hash(article["url"]))
        
        # 문서 메타데이터
        metadata = {
            "title": article["title"],
            "url": article["url"],
            "published_date": article["published_date"],
            "keywords": json.dumps(article["keywords"], ensure_ascii=False)
        }
        
        # 문서 저장
        self.collection.add(
            documents=[article["text"]],
            metadatas=[metadata],
            ids=[doc_id]
        )
    
    def search_articles(self, query: str, n_results: int = 5) -> List[Dict]:
        results = self.collection.query(
            query_texts=[query],
            n_results=n_results
        )
        
        articles = []
        for i in range(len(results["ids"][0])):
            articles.append({
                "id": results["ids"][0][i],
                "title": results["metadatas"][0][i]["title"],
                "url": results["metadatas"][0][i]["url"],
                "published_date": results["metadatas"][0][i]["published_date"],
                "keywords": json.loads(results["metadatas"][0][i]["keywords"]),
                "content": results["documents"][0][i]
            })
        
        return articles
    
    def update_news(self):
        """주기적으로 뉴스를 업데이트하는 메서드"""
        print("Updating news database...")
        for source_name, source_url in self.news_sources.items():
            try:
                # 뉴스 기사 URL 수집 (실제 구현에서는 각 뉴스 사이트의 구조에 맞게 파싱 필요)
                # 여기서는 예시로 단순화
                article_urls = [source_url]  # 실제로는 여러 기사 URL을 수집해야 함
                
                for url in article_urls:
                    article = self.fetch_article(url)
                    if article:
                        self.store_article(article)
                        print(f"Stored article: {article['title']}")
                        time.sleep(1)  # 서버 부하 방지
                
            except Exception as e:
                print(f"Error updating news from {source_name}: {str(e)}")

if __name__ == "__main__":
    collector = NewsCollector()
    collector.update_news() 