#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
FastAPI 서버 제어 모듈
"""

import os
import signal
import psutil
import requests
import time
from typing import Optional, Dict, Any
from log import logger

class ServerController:
    """FastAPI 서버 제어 클래스"""
    
    def __init__(self, host: str = "0.0.0.0", port: int = 8000):
        self.host = host
        self.port = port
        self.base_url = f"http://{host}:{port}"
    
    def find_server_process(self) -> Optional[psutil.Process]:
        """현재 포트에서 실행 중인 서버 프로세스 찾기"""
        try:
            for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
                try:
                    # uvicorn 프로세스 찾기
                    if proc.info['name'] and 'uvicorn' in proc.info['name'].lower():
                        # 포트 확인
                        connections = proc.connections()
                        for conn in connections:
                            if conn.laddr.port == self.port:
                                return proc
                    
                    # Python 프로세스에서 uvicorn 실행 확인
                    if proc.info['cmdline']:
                        cmdline = ' '.join(proc.info['cmdline'])
                        if 'uvicorn' in cmdline and str(self.port) in cmdline:
                            return proc
                            
                except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                    continue
                    
        except Exception as e:
            logger.error(f"프로세스 검색 중 오류: {e}")
        
        return None
    
    def is_server_running(self) -> bool:
        """서버가 실행 중인지 확인"""
        try:
            response = requests.get(f"{self.base_url}/", timeout=5)
            return response.status_code == 200
        except requests.exceptions.RequestException:
            return False
    
    def get_server_info(self) -> Dict[str, Any]:
        """서버 정보 조회"""
        info = {
            "host": self.host,
            "port": self.port,
            "base_url": self.base_url,
            "is_running": False,
            "process_info": None
        }
        
        # 서버 실행 상태 확인
        info["is_running"] = self.is_server_running()
        
        # 프로세스 정보 확인
        process = self.find_server_process()
        if process:
            try:
                info["process_info"] = {
                    "pid": process.pid,
                    "name": process.name(),
                    "status": process.status(),
                    "cpu_percent": process.cpu_percent(),
                    "memory_percent": process.memory_percent(),
                    "create_time": process.create_time()
                }
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass
        
        return info
    
    def shutdown_server(self, force: bool = False) -> Dict[str, Any]:
        """서버 종료"""
        result = {
            "success": False,
            "message": "",
            "method": "",
            "process_info": None
        }
        
        # 1. API를 통한 정상 종료 시도
        if not force:
            try:
                response = requests.post(f"{self.base_url}/shutdown", timeout=10)
                if response.status_code == 200:
                    result["success"] = True
                    result["message"] = "서버가 정상적으로 종료되었습니다."
                    result["method"] = "API"
                    return result
            except requests.exceptions.RequestException:
                logger.info("API를 통한 종료 실패, 프로세스 직접 종료 시도")
        
        # 2. 프로세스 직접 종료
        process = self.find_server_process()
        if process:
            try:
                result["process_info"] = {
                    "pid": process.pid,
                    "name": process.name()
                }
                
                if force:
                    # 강제 종료
                    process.kill()
                    result["method"] = "force_kill"
                else:
                    # 정상 종료 시도
                    process.terminate()
                    result["method"] = "terminate"
                
                # 종료 대기
                try:
                    process.wait(timeout=10)
                    result["success"] = True
                    result["message"] = f"서버가 종료되었습니다. (PID: {process.pid})"
                except psutil.TimeoutExpired:
                    # 강제 종료
                    process.kill()
                    result["success"] = True
                    result["message"] = f"서버가 강제 종료되었습니다. (PID: {process.pid})"
                    result["method"] = "force_kill"
                    
            except (psutil.NoSuchProcess, psutil.AccessDenied) as e:
                result["message"] = f"프로세스 종료 실패: {e}"
        else:
            result["message"] = f"포트 {self.port}에서 실행 중인 서버를 찾을 수 없습니다."
        
        return result
    
    def start_server(self, script_path: str = "fastapi_supabase.py") -> Dict[str, Any]:
        """서버 시작"""
        result = {
            "success": False,
            "message": "",
            "pid": None
        }
        
        # 이미 실행 중인지 확인
        if self.is_server_running():
            result["message"] = f"서버가 이미 포트 {self.port}에서 실행 중입니다."
            return result
        
        try:
            import subprocess
            import sys
            
            # 서버 시작
            cmd = [sys.executable, script_path]
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=os.path.dirname(os.path.abspath(script_path))
            )
            
            result["pid"] = process.pid
            
            # 서버 시작 대기
            for _ in range(30):  # 30초 대기
                time.sleep(1)
                if self.is_server_running():
                    result["success"] = True
                    result["message"] = f"서버가 성공적으로 시작되었습니다. (PID: {process.pid})"
                    return result
            
            # 시작 실패
            process.terminate()
            result["message"] = "서버 시작 시간 초과"
            
        except Exception as e:
            result["message"] = f"서버 시작 실패: {e}"
        
        return result

def shutdown_server_by_port(port: int, force: bool = False) -> Dict[str, Any]:
    """포트 번호로 서버 종료"""
    controller = ServerController(port=port)
    return controller.shutdown_server(force=force)

def get_server_status_by_port(port: int) -> Dict[str, Any]:
    """포트 번호로 서버 상태 확인"""
    controller = ServerController(port=port)
    return controller.get_server_info()

def find_all_uvicorn_servers() -> list:
    """모든 uvicorn 서버 찾기"""
    servers = []
    
    try:
        for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
            try:
                if proc.info['name'] and 'uvicorn' in proc.info['name'].lower():
                    connections = proc.connections()
                    for conn in connections:
                        if conn.laddr.port:
                            servers.append({
                                "pid": proc.pid,
                                "port": conn.laddr.port,
                                "host": conn.laddr.ip,
                                "name": proc.name(),
                                "status": proc.status()
                            })
                
                # Python 프로세스에서 uvicorn 실행 확인
                if proc.info['cmdline']:
                    cmdline = ' '.join(proc.info['cmdline'])
                    if 'uvicorn' in cmdline:
                        # 포트 추출 시도
                        import re
                        port_match = re.search(r'--port\s+(\d+)', cmdline)
                        if port_match:
                            port = int(port_match.group(1))
                            servers.append({
                                "pid": proc.pid,
                                "port": port,
                                "host": "0.0.0.0",  # 기본값
                                "name": proc.name(),
                                "status": proc.status()
                            })
                            
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                continue
                
    except Exception as e:
        logger.error(f"서버 검색 중 오류: {e}")
    
    return servers 