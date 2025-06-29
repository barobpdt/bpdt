#!/usr/bin/env python3
"""
로그 파일 관리 스크립트
"""

import os
import glob
import shutil
from datetime import datetime, timedelta

def list_log_files():
    """로그 파일 목록을 조회합니다."""
    log_dir = "logs"
    if not os.path.exists(log_dir):
        print("📁 로그 디렉토리가 존재하지 않습니다.")
        return
    
    print("📋 로그 파일 목록:")
    log_files = glob.glob(os.path.join(log_dir, "*.log*"))
    
    if not log_files:
        print("  📄 로그 파일이 없습니다.")
        return
    
    for log_file in sorted(log_files):
        file_size = os.path.getsize(log_file)
        file_time = datetime.fromtimestamp(os.path.getmtime(log_file))
        print(f"  📄 {os.path.basename(log_file)}")
        print(f"     크기: {file_size:,} bytes")
        print(f"     수정: {file_time.strftime('%Y-%m-%d %H:%M:%S')}")

def show_latest_logs(lines=50):
    """최신 로그를 조회합니다."""
    log_file = "logs/fastapi_mysql_crud.log"
    
    if not os.path.exists(log_file):
        print("❌ 로그 파일이 존재하지 않습니다.")
        return
    
    print(f"📋 최신 로그 (마지막 {lines}줄):")
    print("=" * 80)
    
    try:
        with open(log_file, 'r', encoding='utf-8') as f:
            all_lines = f.readlines()
            latest_lines = all_lines[-lines:] if len(all_lines) > lines else all_lines
            
            for line in latest_lines:
                print(line.rstrip())
    except Exception as e:
        print(f"❌ 로그 파일 읽기 오류: {e}")

def clean_old_logs(days=30):
    """오래된 로그 파일을 삭제합니다."""
    log_dir = "logs"
    if not os.path.exists(log_dir):
        print("📁 로그 디렉토리가 존재하지 않습니다.")
        return
    
    cutoff_date = datetime.now() - timedelta(days=days)
    deleted_count = 0
    
    print(f"🧹 {days}일 이상 된 로그 파일 정리 중...")
    
    for log_file in glob.glob(os.path.join(log_dir, "*.log*")):
        file_time = datetime.fromtimestamp(os.path.getmtime(log_file))
        if file_time < cutoff_date:
            try:
                os.remove(log_file)
                print(f"🗑️ 삭제됨: {os.path.basename(log_file)}")
                deleted_count += 1
            except Exception as e:
                print(f"❌ 삭제 실패: {os.path.basename(log_file)} - {e}")
    
    print(f"✅ 정리 완료: {deleted_count}개 파일 삭제됨")

def backup_logs():
    """로그 파일을 백업합니다."""
    log_dir = "logs"
    if not os.path.exists(log_dir):
        print("📁 로그 디렉토리가 존재하지 않습니다.")
        return
    
    backup_dir = f"logs/backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    
    try:
        os.makedirs(backup_dir, exist_ok=True)
        
        log_files = glob.glob(os.path.join(log_dir, "*.log*"))
        copied_count = 0
        
        for log_file in log_files:
            if not log_file.startswith(backup_dir):  # 백업 디렉토리 자체는 제외
                filename = os.path.basename(log_file)
                backup_file = os.path.join(backup_dir, filename)
                shutil.copy2(log_file, backup_file)
                print(f"📋 백업됨: {filename}")
                copied_count += 1
        
        print(f"✅ 백업 완료: {copied_count}개 파일을 {backup_dir}에 저장")
        
    except Exception as e:
        print(f"❌ 백업 실패: {e}")

def main():
    """메인 함수"""
    import sys
    
    print("🚀 로그 파일 관리 도구")
    print("=" * 50)
    
    if len(sys.argv) < 2:
        print("사용법:")
        print("  python log_manager.py list          - 로그 파일 목록 조회")
        print("  python log_manager.py show [lines]  - 최신 로그 조회 (기본 50줄)")
        print("  python log_manager.py clean [days]  - 오래된 로그 정리 (기본 30일)")
        print("  python log_manager.py backup        - 로그 파일 백업")
        return
    
    command = sys.argv[1].lower()
    
    if command == "list":
        list_log_files()
    elif command == "show":
        lines = int(sys.argv[2]) if len(sys.argv) > 2 else 50
        show_latest_logs(lines)
    elif command == "clean":
        days = int(sys.argv[2]) if len(sys.argv) > 2 else 30
        clean_old_logs(days)
    elif command == "backup":
        backup_logs()
    else:
        print(f"❌ 알 수 없는 명령어: {command}")

if __name__ == "__main__":
    main() 