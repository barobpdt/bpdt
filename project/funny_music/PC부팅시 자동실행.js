Node.js 앱을 Windows 서비스로 등록해주는 라이브러리입니다.

bash
npm install -g node-windows
npm link node-windows
javascript
// install-service.js
import { Service } from 'node-windows';
const svc = new Service({
    name: 'Express Server',
    description: 'Karaoke Express 서버',
    script: 'C:\\bpdt\\project\\express-sample\\server.js',
    nodeOptions: [],
});
svc.on('install', () => svc.start());
svc.install();
bash
node install-service.js   # 서비스 등록
등록 후 Windows 서비스 목록(services.msc)에서 확인 가능:

서비스명: Express Server
상태: 실행 중
시작 유형: 자동 (부팅 시 자동 시작)
실제 사용 흐름
PC 부팅
  ↓
Windows 서비스 자동 시작 (node-windows)
  → server.js 실행
  ↓
시작 프로그램 (autostart.bat)
  → Chrome --kiosk http://localhost:8081/video-player.html



powershell -Command "Start-Process cmd -ArgumentList '/c cd /d C:\bpdt\project\express-sample && node scripts/install-service.cjs' -Verb RunAs -Wait"

2. package.json에 서비스 스크립트 명령 추가 후 서비스 등록 실행
powershell -Command "Get-Service -Name 'ExpressSampleServer' -ErrorAction SilentlyContinue | Select-Object Name, Status, StartType"

3. 서비스 시작
powershell -Command "Start-Service -Name 'expresssampleserver.exe'"
powershell -Command "Start-Process powershell -ArgumentList 'Start-Service -Name expresssampleserver.exe' -Verb RunAs -Wait"
powershell -Command "Get-Service -Name 'expresssampleserver.exe' | Select-Object Name, Status, StartType"

# 등록 (최초 1회, 관리자 권한 터미널)
npm run service:install
# 제거 (관리자 권한 터미널)
npm run service:uninstall

또는 
Win + R → services.msc
ExpressSampleServer 찾아서 시작/중지/재시작

## 부팅후 처리순서
PC 부팅
  ↓
ExpressSampleServer 서비스 자동 시작 (server.js)
  ↓
시작 프로그램 배치 파일 (autostart.bat)
  → Chrome --kiosk http://localhost:8081/video-player.html


##########

방법 1 — 시작 프로그램 폴더 (가장 간단 ⭐)
① 배치 파일 생성 (autostart.bat)

@echo off
REM 서버 먼저 시작 (이미 서비스로 돌고 있으면 생략)
cd /d C:\bpdt\project\express-sample
start "" node server.js
REM 서버 기동 대기
timeout /t 3 /nobreak > nul
REM Chrome 키오스크 모드 전체화면 시작
start "" "C:\Program Files\Google\Chrome\Application\chrome.exe" ^
    --kiosk ^
    --no-first-run ^
    --disable-infobars ^
    http://localhost:8081/video-player.html

② 시작 프로그램 폴더에 등록
Win + R → shell:startup → Enter
→ 열리는 폴더에 autostart.bat 복사 (또는 바로 가기)

방법 2 — 작업 스케줄러 (로그인 없이 시작 필요할 때)
작업 스케줄러 → 기본 작업 만들기
→ 트리거: "컴퓨터 시작 시"
→ 동작: 프로그램 시작 → autostart.bat 경로
→ "가장 높은 권한으로 실행" 체크


브라우저별 전체화면 옵션
브라우저				키오스크 모드				전체화면만
Chrome				--kiosk					--start-fullscreen
Edge					--kiosk					--start-fullscreen

--kiosk는 주소창·탭 완전히 숨김, ESC로 빠져나올 수 없음 (키오스크 전용)
--start-fullscreen은 F11 전체화면 (주소창 숨길 수 있고 ESC로 빠져나옴)

