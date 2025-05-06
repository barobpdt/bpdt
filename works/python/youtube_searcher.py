from googleapiclient.discovery import build
from googleapiclient.errors import HttpError
import os
from typing import List, Dict
from datetime import datetime, timedelta

class YouTubeSearcher:
    def __init__(self):
        self.api_key = os.getenv("YOUTUBE_API_KEY")
        if not self.api_key:
            raise ValueError("YouTube API key not found in environment variables")
        
        self.youtube = build('youtube', 'v3', developerKey=self.api_key)
    
    def search_videos(self, query: str, max_results: int = 5) -> List[Dict]:
        try:
            # 검색 요청
            search_response = self.youtube.search().list(
                q=query,
                part='id,snippet',
                maxResults=max_results,
                order='relevance',  # 관련성 순으로 정렬
                type='video',
                relevanceLanguage='ko',  # 한국어 콘텐츠 우선
                regionCode='KR'  # 한국 지역
            ).execute()
            
            videos = []
            for item in search_response.get('items', []):
                video_id = item['id']['videoId']
                
                # 동영상 상세 정보 가져오기
                video_response = self.youtube.videos().list(
                    part='snippet,statistics',
                    id=video_id
                ).execute()
                
                video_info = video_response['items'][0]
                snippet = video_info['snippet']
                statistics = video_info['statistics']
                
                # 조회수, 좋아요 수 등이 없는 경우 처리
                view_count = statistics.get('viewCount', '0')
                like_count = statistics.get('likeCount', '0')
                comment_count = statistics.get('commentCount', '0')
                
                videos.append({
                    'title': snippet['title'],
                    'description': snippet['description'],
                    'url': f'https://www.youtube.com/watch?v={video_id}',
                    'thumbnail': snippet['thumbnails']['high']['url'],
                    'channel_title': snippet['channelTitle'],
                    'published_at': snippet['publishedAt'],
                    'view_count': view_count,
                    'like_count': like_count,
                    'comment_count': comment_count
                })
            
            return videos
            
        except HttpError as e:
            print(f"An HTTP error occurred: {e}")
            return []
        except Exception as e:
            print(f"An error occurred: {e}")
            return []
    
    def get_trending_videos(self, max_results: int = 5) -> List[Dict]:
        try:
            # 인기 동영상 검색
            search_response = self.youtube.videos().list(
                part='snippet,statistics',
                chart='mostPopular',
                regionCode='KR',
                maxResults=max_results
            ).execute()
            
            videos = []
            for item in search_response.get('items', []):
                snippet = item['snippet']
                statistics = item['statistics']
                
                videos.append({
                    'title': snippet['title'],
                    'description': snippet['description'],
                    'url': f'https://www.youtube.com/watch?v={item["id"]}',
                    'thumbnail': snippet['thumbnails']['high']['url'],
                    'channel_title': snippet['channelTitle'],
                    'published_at': snippet['publishedAt'],
                    'view_count': statistics.get('viewCount', '0'),
                    'like_count': statistics.get('likeCount', '0'),
                    'comment_count': statistics.get('commentCount', '0')
                })
            
            return videos
            
        except HttpError as e:
            print(f"An HTTP error occurred: {e}")
            return []
        except Exception as e:
            print(f"An error occurred: {e}")
            return []

if __name__ == "__main__":
    # 테스트 코드
    searcher = YouTubeSearcher()
    
    # 검색 테스트
    print("=== 검색 결과 ===")
    results = searcher.search_videos("파이썬 프로그래밍")
    for video in results:
        print(f"제목: {video['title']}")
        print(f"URL: {video['url']}")
        print(f"조회수: {video['view_count']}")
        print("---")
    
    # 인기 동영상 테스트
    print("\n=== 인기 동영상 ===")
    trending = searcher.get_trending_videos()
    for video in trending:
        print(f"제목: {video['title']}")
        print(f"URL: {video['url']}")
        print(f"조회수: {video['view_count']}")
        print("---") 