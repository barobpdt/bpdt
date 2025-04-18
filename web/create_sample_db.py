import sqlite3
import os

def create_sample_database(db_path):
    """
    샘플 SQLite 데이터베이스를 생성하는 함수
    
    Args:
        db_path: 생성할 데이터베이스 파일 경로
    """
    # 기존 파일이 있으면 삭제
    if os.path.exists(db_path):
        os.remove(db_path)
        print(f"기존 데이터베이스 '{db_path}'를 삭제했습니다.")
    
    # 데이터베이스 연결
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # 사용자 테이블 생성
    cursor.execute('''
    CREATE TABLE users (
        user_id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT NOT NULL UNIQUE,
        email TEXT NOT NULL UNIQUE,
        password_hash TEXT NOT NULL,
        first_name TEXT,
        last_name TEXT,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )
    ''')
    
    # 사용자 프로필 테이블 생성
    cursor.execute('''
    CREATE TABLE user_profiles (
        profile_id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        bio TEXT,
        avatar_url TEXT,
        location TEXT,
        website TEXT,
        FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
    )
    ''')
    
    # 카테고리 테이블 생성
    cursor.execute('''
    CREATE TABLE categories (
        category_id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE,
        description TEXT,
        parent_id INTEGER,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (parent_id) REFERENCES categories(category_id) ON DELETE SET NULL
    )
    ''')
    
    # 게시물 테이블 생성
    cursor.execute('''
    CREATE TABLE posts (
        post_id INTEGER PRIMARY KEY AUTOINCREMENT,
        title TEXT NOT NULL,
        content TEXT NOT NULL,
        user_id INTEGER NOT NULL,
        category_id INTEGER,
        status TEXT DEFAULT 'draft' CHECK(status IN ('draft', 'published', 'archived')),
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        published_at TIMESTAMP,
        FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
        FOREIGN KEY (category_id) REFERENCES categories(category_id) ON DELETE SET NULL
    )
    ''')
    
    # 태그 테이블 생성
    cursor.execute('''
    CREATE TABLE tags (
        tag_id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )
    ''')
    
    # 게시물-태그 연결 테이블 생성
    cursor.execute('''
    CREATE TABLE post_tags (
        post_id INTEGER NOT NULL,
        tag_id INTEGER NOT NULL,
        PRIMARY KEY (post_id, tag_id),
        FOREIGN KEY (post_id) REFERENCES posts(post_id) ON DELETE CASCADE,
        FOREIGN KEY (tag_id) REFERENCES tags(tag_id) ON DELETE CASCADE
    )
    ''')
    
    # 댓글 테이블 생성
    cursor.execute('''
    CREATE TABLE comments (
        comment_id INTEGER PRIMARY KEY AUTOINCREMENT,
        post_id INTEGER NOT NULL,
        user_id INTEGER NOT NULL,
        parent_id INTEGER,
        content TEXT NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (post_id) REFERENCES posts(post_id) ON DELETE CASCADE,
        FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
        FOREIGN KEY (parent_id) REFERENCES comments(comment_id) ON DELETE CASCADE
    )
    ''')
    
    # 좋아요 테이블 생성
    cursor.execute('''
    CREATE TABLE likes (
        like_id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        post_id INTEGER NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        UNIQUE(user_id, post_id),
        FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
        FOREIGN KEY (post_id) REFERENCES posts(post_id) ON DELETE CASCADE
    )
    ''')
    
    # 인덱스 생성
    cursor.execute('CREATE INDEX idx_users_username ON users(username)')
    cursor.execute('CREATE INDEX idx_users_email ON users(email)')
    cursor.execute('CREATE INDEX idx_posts_user_id ON posts(user_id)')
    cursor.execute('CREATE INDEX idx_posts_category_id ON posts(category_id)')
    cursor.execute('CREATE INDEX idx_posts_status ON posts(status)')
    cursor.execute('CREATE INDEX idx_comments_post_id ON comments(post_id)')
    cursor.execute('CREATE INDEX idx_comments_user_id ON comments(user_id)')
    cursor.execute('CREATE INDEX idx_comments_parent_id ON comments(parent_id)')
    
    # 샘플 데이터 삽입
    # 사용자 데이터
    cursor.execute('''
    INSERT INTO users (username, email, password_hash, first_name, last_name)
    VALUES 
        ('admin', 'admin@example.com', 'hash1', 'Admin', 'User'),
        ('john_doe', 'john@example.com', 'hash2', 'John', 'Doe'),
        ('jane_smith', 'jane@example.com', 'hash3', 'Jane', 'Smith')
    ''')
    
    # 카테고리 데이터
    cursor.execute('''
    INSERT INTO categories (name, description)
    VALUES 
        ('Technology', 'Technology related posts'),
        ('Travel', 'Travel related posts'),
        ('Food', 'Food related posts')
    ''')
    
    # 태그 데이터
    cursor.execute('''
    INSERT INTO tags (name)
    VALUES 
        ('python'),
        ('sqlite'),
        ('web'),
        ('travel'),
        ('food')
    ''')
    
    # 변경사항 저장 및 연결 종료
    conn.commit()
    conn.close()
    
    print(f"샘플 데이터베이스 '{db_path}'가 성공적으로 생성되었습니다.")


if __name__ == "__main__":
    create_sample_database("sample.db") 