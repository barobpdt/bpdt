#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
FastAPI 서버 종료 스크립트
"""

import sys
import os
import argparse
from server_control import ServerController, shutdown_server_by_port, find_all_uvicorn_servers

def main():
    """메인 함수"""
    parser = argparse.ArgumentParser(description='FastAPI 서버 종료 도구')
    parser.add_argument('--port', '-p', type=int, default=8000, help='종료할 서버 포트 (기본값: 8000)')
    parser.add_argument('--force', '-f', action='store_true', help='강제 종료')
    parser.add_argument('--list', '-l', action='store_true', help='실행 중인 서버 목록 조회')
    parser.add_argument('--all', '-a', action='store_true', help='모든 uvicorn 서버 종료')
    
    args = parser.parse_args()
    
    print("=" * 50)
    print("FastAPI 서버 종료 도구")
    print("=" * 50)
    
    if args.list:
        # 서버 목록 조회
        print("\n실행 중인 uvicorn 서버 목록:")
        print("-" * 30)
        servers = find_all_uvicorn_servers()
        
        if servers:
            for i, server in enumerate(servers, 1):
                print(f"{i}. PID: {server['pid']}, 포트: {server['port']}, 호스트: {server['host']}")
                print(f"   이름: {server['name']}, 상태: {server['status']}")
        else:
            print("실행 중인 uvicorn 서버가 없습니다.")
        return
    
    if args.all:
        # 모든 서버 종료
        print("\n모든 uvicorn 서버를 종료합니다...")
        servers = find_all_uvicorn_servers()
        
        if not servers:
            print("종료할 서버가 없습니다.")
            return
        
        for server in servers:
            port = server['port']
            print(f"포트 {port}의 서버를 종료합니다...")
            result = shutdown_server_by_port(port, force=args.force)
            
            if result['success']:
                print(f"✅ 포트 {port}: {result['message']}")
            else:
                print(f"❌ 포트 {port}: {result['message']}")
        return
    
    # 특정 포트 서버 종료
    port = args.port
    force = args.force
    
    print(f"\n포트 {port}의 서버를 {'강제 ' if force else ''}종료합니다...")
    
    # 서버 상태 확인
    controller = ServerController(port=port)
    status = controller.get_server_info()
    
    if not status['is_running']:
        print(f"포트 {port}에서 실행 중인 서버가 없습니다.")
        return
    
    # 서버 종료
    result = shutdown_server_by_port(port, force=force)
    
    if result['success']:
        print(f"✅ {result['message']}")
        print(f"방법: {result['method']}")
        if result['process_info']:
            print(f"PID: {result['process_info']['pid']}")
    else:
        print(f"❌ {result['message']}")
        sys.exit(1)

if __name__ == "__main__":
    main() 