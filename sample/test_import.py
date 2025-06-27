#!/usr/bin/env python3
"""
jkj_model.py import 테스트
"""
import io
import os
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

print("🔍 jkj_model.py import 테스트 시작")
print(f"📁 현재 작업 디렉토리: {os.getcwd()}")
print(f"📁 현재 파일 위치: {os.path.dirname(os.path.abspath(__file__))}")

# 현재 디렉토리를 sys.path에 추가
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.insert(0, current_dir)
    print(f"✅ 현재 디렉토리를 sys.path에 추가: {current_dir}")

print(f"📋 sys.path: {sys.path}")

# jkj_model.py 파일 존재 확인
jkj_model_path = os.path.join(current_dir, "jkj_model.py")
if os.path.exists(jkj_model_path):
    print(f"✅ jkj_model.py 파일 존재: {jkj_model_path}")
else:
    print(f"❌ jkj_model.py 파일 없음: {jkj_model_path}")
    exit(1)

# import 테스트
try:
    print("\n🔍 jkj_model.py import 시도...")
    from jkj_model import SgMemb, SgMembQr, SgQrInfo, SgMembCall
    print("✅ jkj_model.py import 성공!")
    
    # 모델 확인
    print(f"📊 SgMemb: {SgMemb}")
    print(f"📊 SgMembQr: {SgMembQr}")
    print(f"📊 SgQrInfo: {SgQrInfo}")
    print(f"📊 SgMembCall: {SgMembCall}")
    
except ImportError as e:
    print(f"❌ jkj_model.py import 실패: {e}")
    print(f"❌ 오류 타입: {type(e)}")
    import traceback
    traceback.print_exc()
except Exception as e:
    print(f"❌ 예상치 못한 오류: {e}")
    print(f"❌ 오류 타입: {type(e)}")
    import traceback
    traceback.print_exc()

print("\n🎉 테스트 완료!") 