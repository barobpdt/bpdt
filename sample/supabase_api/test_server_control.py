#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
FastAPI 서버 제어 기능 테스트 스크립트
"""

import requests
import json
import sys
import os
import time

# 현재 디렉토리를 Python 경로에 추가
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

def test_server_control():
    """서버 제어 기능 테스트"""
    
    base_url = "http://localhost:8000"
    
    print("=" * 60)
    print("FastAPI 서버 제어 기능 테스트")
    print("=" * 60)
    
    # 1. 서버 상태 확인
    print("\n1. 서버 상태 확인")
    print("-" * 30)
    try:
        response = requests.get(f"{base_url}/server/status")
        if response.status_code == 200:
            status = response.json()
            print(f"호스트: {status['host']}")
            print(f"포트: {status['port']}")
            print(f"실행 중: {status['is_running']}")
            print(f"URL: {status['base_url']}")
            
            if status['process_info']:
                print(f"프로세스 ID: {status['process_info']['pid']}")
                print(f"프로세스명: {status['process_info']['name']}")
                print(f"상태: {status['process_info']['status']}")
                print(f"CPU 사용률: {status['process_info']['cpu_percent']}%")
                print(f"메모리 사용률: {status['process_info']['memory_percent']}%")
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"서버 상태 확인 실패: {e}")
    
    # 2. 모든 서버 목록 조회
    print("\n2. 모든 uvicorn 서버 목록")
    print("-" * 30)
    try:
        response = requests.get(f"{base_url}/server/list")
        if response.status_code == 200:
            server_list = response.json()
            print(f"총 서버 수: {server_list['total_servers']}")
            
            if server_list['servers']:
                for i, server in enumerate(server_list['servers'], 1):
                    print(f"  {i}. PID: {server['pid']}, 포트: {server['port']}, 호스트: {server['host']}")
                    print(f"     이름: {server['name']}, 상태: {server['status']}")
            else:
                print("  실행 중인 uvicorn 서버가 없습니다.")
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"서버 목록 조회 실패: {e}")
    
    # 3. 특정 포트 서버 상태 확인
    print("\n3. 특정 포트 서버 상태 확인")
    print("-" * 30)
    try:
        response = requests.get(f"{base_url}/server/status/8000")
        if response.status_code == 200:
            status = response.json()
            print(f"포트 8000 서버 상태:")
            print(f"  실행 중: {status['is_running']}")
            if status['process_info']:
                print(f"  PID: {status['process_info']['pid']}")
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"특정 포트 상태 확인 실패: {e}")

def test_server_shutdown():
    """서버 종료 기능 테스트"""
    
    base_url = "http://localhost:8000"
    
    print("\n" + "=" * 60)
    print("서버 종료 기능 테스트")
    print("=" * 60)
    
    print("\n⚠️  주의: 이 테스트는 실제로 서버를 종료합니다!")
    print("테스트를 계속하시겠습니까? (y/N): ", end="")
    
    try:
        user_input = input().strip().lower()
        if user_input != 'y':
            print("테스트를 취소했습니다.")
            return
    except KeyboardInterrupt:
        print("\n테스트를 취소했습니다.")
        return
    
    # 1. 정상 종료 테스트
    print("\n1. 정상 종료 테스트")
    print("-" * 30)
    try:
        print("서버를 정상 종료합니다...")
        response = requests.post(f"{base_url}/server/shutdown")
        if response.status_code == 200:
            result = response.json()
            print(f"성공: {result['message']}")
            print(f"방법: {result['method']}")
            if result['process_info']:
                print(f"PID: {result['process_info']['pid']}")
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"정상 종료 실패: {e}")
    
    # 2. 서버 종료 확인
    print("\n2. 서버 종료 확인")
    print("-" * 30)
    try:
        time.sleep(2)  # 종료 대기
        response = requests.get(f"{base_url}/server/status")
        if response.status_code == 200:
            status = response.json()
            print(f"서버 실행 중: {status['is_running']}")
            if not status['is_running']:
                print("✅ 서버가 성공적으로 종료되었습니다.")
            else:
                print("❌ 서버가 여전히 실행 중입니다.")
        else:
            print("✅ 서버에 연결할 수 없습니다 (종료됨)")
    except Exception as e:
        print("✅ 서버에 연결할 수 없습니다 (종료됨)")

def test_force_shutdown():
    """강제 종료 기능 테스트"""
    
    print("\n" + "=" * 60)
    print("강제 종료 기능 테스트")
    print("=" * 60)
    
    print("이 테스트는 서버를 강제로 종료합니다.")
    print("테스트를 계속하시겠습니까? (y/N): ", end="")
    
    try:
        user_input = input().strip().lower()
        if user_input != 'y':
            print("테스트를 취소했습니다.")
            return
    except KeyboardInterrupt:
        print("\n테스트를 취소했습니다.")
        return
    
    # 특정 포트 강제 종료 테스트
    print("\n특정 포트 강제 종료 테스트")
    print("-" * 30)
    
    port = input("종료할 포트 번호를 입력하세요 (기본값: 8000): ").strip()
    if not port:
        port = 8000
    else:
        try:
            port = int(port)
        except ValueError:
            print("잘못된 포트 번호입니다.")
            return
    
    try:
        print(f"포트 {port}의 서버를 강제 종료합니다...")
        response = requests.post(f"http://localhost:8000/server/force-shutdown/{port}")
        if response.status_code == 200:
            result = response.json()
            print(f"성공: {result['message']}")
            print(f"방법: {result['method']}")
            print(f"포트: {result['port']}")
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"강제 종료 실패: {e}")

def main():
    """메인 함수"""
    print("FastAPI 서버 제어 기능 테스트")
    print("서버가 실행 중인지 확인하세요 (http://localhost:8000)")
    print()
    
    try:
        # 기본 서버 제어 기능 테스트
        test_server_control()
        
        # 종료 테스트는 사용자 선택
        print("\n" + "=" * 60)
        print("종료 테스트를 실행하시겠습니까?")
        print("1. 정상 종료 테스트")
        print("2. 강제 종료 테스트")
        print("3. 테스트 종료")
        print("선택 (1-3): ", end="")
        
        try:
            choice = input().strip()
            if choice == "1":
                test_server_shutdown()
            elif choice == "2":
                test_force_shutdown()
            else:
                print("테스트를 종료합니다.")
        except KeyboardInterrupt:
            print("\n테스트를 종료합니다.")
        
    except Exception as e:
        print(f"\n테스트 중 오류 발생: {e}")
    
    print("\n" + "=" * 60)
    print("테스트 완료!")
    print("=" * 60)

if __name__ == "__main__":
    main() 