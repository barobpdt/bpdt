# React Native 푸시 알림 예제

이 프로젝트는 React Native에서 Firebase Cloud Messaging을 사용하여 백그라운드 푸시 알림을 처리하는 예제입니다.

## 기능

- 백그라운드 푸시 알림 수신
- 앱이 백그라운드에서 푸시 알림으로 열릴 때 데이터 처리
- 푸시 알림 권한 요청
- FCM 토큰 관리

## 설치 방법

1. 프로젝트 의존성 설치:
```bash
npm install
```

2. iOS의 경우 추가 설치:
```bash
cd ios
pod install
cd ..
```

3. Firebase 프로젝트 설정:
   - Firebase 콘솔에서 새 프로젝트 생성
   - iOS/Android 앱 등록
   - google-services.json (Android) 및 GoogleService-Info.plist (iOS) 파일 다운로드
   - Android: android/app/ 디렉토리에 google-services.json 파일 복사
   - iOS: Xcode에서 GoogleService-Info.plist 파일 추가

## 푸시 알림 테스트

1. Firebase 콘솔에서 테스트 메시지 전송:
```json
{
  "data": {
    "type": "order",
    "orderId": "12345"
  },
  "notification": {
    "title": "새 주문",
    "body": "새로운 주문이 들어왔습니다!"
  }
}
```

2. 또는 Cloud Functions를 사용하여 프로그래밍 방식으로 전송:
```javascript
admin.messaging().send({
  token: 'FCM_TOKEN',
  data: {
    type: 'message',
    content: '안녕하세요!'
  },
  notification: {
    title: '새 메시지',
    body: '새로운 메시지가 도착했습니다.'
  }
});
```

## 주의사항

- iOS에서는 백그라운드 푸시 알림을 위해 추가 설정이 필요할 수 있습니다.
- Android에서는 백그라운드 메시지 처리를 위해 별도의 서비스가 필요하지 않습니다.
- 실제 프로덕션 환경에서는 보안을 위해 FCM 토큰을 서버에 등록해야 합니다.

## 라이선스

MIT 

# 압축 파일 미리보기 프로그램

이 프로그램은 ZIP, RAR, 7Z 등의 압축 파일 내부의 내용을 미리 보고 선택적으로 추출할 수 있는 GUI 애플리케이션입니다.

## 주요 기능

- ZIP, RAR, 7Z 형식의 압축 파일 지원
- 압축 파일 내부의 파일 목록 표시
- 파일 크기 및 수정 날짜 정보 표시
- 선택한 파일만 추출 가능
- 직관적인 GUI 인터페이스

## 설치 방법

1. 필요한 라이브러리 설치:
```
pip install -r requirements.txt
```

2. RAR 파일 지원을 위해 UnRAR 유틸리티 설치:
   - Windows: [WinRAR](https://www.win-rar.com/) 설치
   - Linux: `sudo apt-get install unrar` (Ubuntu/Debian) 또는 `sudo yum install unrar` (CentOS/RHEL)
   - macOS: `brew install unrar` (Homebrew 사용)

## 사용 방법

1. 프로그램 실행:
```
python zip_preview.py
```

2. "압축 파일 선택" 버튼을 클릭하여 압축 파일을 선택합니다.
3. 압축 파일 내부의 파일 목록이 표시됩니다.
4. 원하는 파일을 선택하고 "선택 항목 추출" 버튼을 클릭하여 추출할 위치를 지정합니다.

## 주의사항

- 7Z 파일 지원을 위해서는 py7zr 라이브러리가 필요합니다.
- RAR 파일 지원을 위해서는 UnRAR 유틸리티가 시스템에 설치되어 있어야 합니다.
- 대용량 압축 파일의 경우 로딩 시간이 길어질 수 있습니다.

# 클립보드 모니터링 프로그램 (Tkinter)

이 프로그램은 클립보드의 내용을 실시간으로 모니터링하고 변경 사항을 기록하는 GUI 애플리케이션입니다.

## 주요 기능

- 클립보드 내용 실시간 모니터링
- 클립보드 히스토리 저장 및 불러오기
- 정규식 기반 필터링 기능
- 클립보드 내용 상세 보기
- JSON 형식으로 히스토리 저장

## 설치 방법

1. 필요한 라이브러리 설치:
```
pip install -r requirements.txt
```

## 사용 방법

1. 프로그램 실행:
```
python clipboard_monitor.py
```

2. "모니터링 시작" 버튼을 클릭하여 클립보드 모니터링을 시작합니다.
3. 클립보드에 복사된 내용이 자동으로 히스토리에 추가됩니다.
4. 필터 입력란에 정규식을 입력하고 "적용" 버튼을 클릭하여 특정 패턴의 내용만 필터링할 수 있습니다.
5. 히스토리 항목을 선택하면 해당 내용이 상세하게 표시됩니다.
6. "저장" 버튼을 클릭하여 현재 히스토리를 JSON 파일로 저장할 수 있습니다.
7. "불러오기" 버튼을 클릭하여 이전에 저장한 히스토리를 불러올 수 있습니다.

## 주의사항

- 클립보드 모니터링은 백그라운드 스레드에서 실행되므로 프로그램이 실행 중일 때만 작동합니다.
- 대용량 텍스트나 이미지가 클립보드에 복사될 경우 메모리 사용량이 증가할 수 있습니다.
- 민감한 정보가 클립보드에 복사될 수 있으므로 주의가 필요합니다.

# 클립보드 모니터링 프로그램 (PyQt)

이 프로그램은 PyQt5를 사용하여 구현된 클립보드 모니터링 애플리케이션으로, 더 현대적이고 세련된 UI를 제공합니다.

## 주요 기능

- 클립보드 내용 실시간 모니터링
- 클립보드 히스토리 저장 및 불러오기
- 정규식 기반 필터링 기능
- 클립보드 내용 상세 보기
- JSON 형식으로 히스토리 저장
- 스플리터를 사용한 직관적인 UI
- 모던한 Fusion 스타일 적용

## 설치 방법

1. 필요한 라이브러리 설치:
```
pip install -r requirements.txt
```

## 사용 방법

1. 프로그램 실행:
```
python clipboard_monitor_qt.py
```

2. "모니터링 시작" 버튼을 클릭하여 클립보드 모니터링을 시작합니다.
3. 클립보드에 복사된 내용이 자동으로 히스토리에 추가됩니다.
4. 필터 입력란에 정규식을 입력하고 "적용" 버튼을 클릭하여 특정 패턴의 내용만 필터링할 수 있습니다.
5. 히스토리 항목을 선택하면 해당 내용이 상세하게 표시됩니다.
6. "저장" 버튼을 클릭하여 현재 히스토리를 JSON 파일로 저장할 수 있습니다.
7. "불러오기" 버튼을 클릭하여 이전에 저장한 히스토리를 불러올 수 있습니다.
8. 스플리터를 드래그하여 클립보드 내용과 히스토리 영역의 크기를 조절할 수 있습니다.

## Tkinter 버전과의 차이점

- 더 현대적이고 세련된 UI (Fusion 스타일)
- 스플리터를 사용한 직관적인 레이아웃
- 그룹박스를 사용한 명확한 영역 구분
- 더 나은 파일 저장 대화상자
- 더 세련된 메시지 대화상자

## 주의사항

- 클립보드 모니터링은 백그라운드 스레드에서 실행되므로 프로그램이 실행 중일 때만 작동합니다.
- 대용량 텍스트나 이미지가 클립보드에 복사될 경우 메모리 사용량이 증가할 수 있습니다.
- 민감한 정보가 클립보드에 복사될 수 있으므로 주의가 필요합니다.

# 다중 파일 업로드 애플리케이션

이 프로젝트는 웹소켓을 사용하여 파일을 효율적으로 업로드할 수 있는 웹 애플리케이션입니다.

## 주요 기능

- 드래그 앤 드롭으로 파일 업로드
- 다중 파일 선택 및 업로드
- 파일 타입별 아이콘 표시
- 실시간 업로드 진행 상황 표시
- 웹소켓을 통한 효율적인 바이너리 데이터 전송
- 청크 단위 파일 전송으로 대용량 파일 지원

## 기술 스택

- **프론트엔드**: HTML, CSS, JavaScript
- **백엔드**: Node.js, WebSocket (ws 라이브러리)

## 설치 및 실행 방법

### 서버 설정

1. 필요한 패키지 설치:
   ```
   npm install
   ```

2. 서버 실행:
   ```
   npm start
   ```

3. 서버는 기본적으로 `http://localhost:8080`에서 실행됩니다.

### 클라이언트 실행

1. `multi_file_upload.html` 파일을 웹 브라우저에서 열거나 웹 서버를 통해 제공합니다.

## 웹소켓 파일 업로드 프로세스

1. **연결 설정**: 클라이언트가 웹소켓 서버에 연결합니다.
2. **업로드 시작**: 클라이언트가 파일 메타데이터(이름, 크기, 타입)를 서버에 전송합니다.
3. **청크 전송**: 파일을 작은 청크(64KB)로 나누어 순차적으로 전송합니다.
4. **진행 상황 업데이트**: 서버가 각 청크를 수신할 때마다 진행 상황을 클라이언트에 보고합니다.
5. **업로드 완료**: 모든 청크가 전송되면 클라이언트가 완료 메시지를 서버에 전송합니다.

## 파일 지원 형식

- **이미지**: .jpg, .jpeg, .png, .gif, .bmp, .webp
- **비디오**: .mp4, .avi, .mov, .wmv, .flv, .mkv
- **오디오**: .mp3, .wav, .ogg, .flac, .m4a
- **문서**: .pdf, .doc, .docx, .txt, .rtf, .odt
- **압축파일**: .zip, .rar, .7z, .tar, .gz

## 주의사항

- 대용량 파일 업로드 시 브라우저 메모리 사용량에 주의하세요.
- 서버 측에서 업로드된 파일은 `uploads` 디렉토리에 저장됩니다.
- 웹소켓 연결이 끊어지면 자동으로 재연결을 시도합니다.

## Markdown to HTML 변환 기능

이 프로젝트는 Markdown 파일을 HTML로 변환하는 기능을 제공합니다.

### 주요 기능

- Markdown 파일을 HTML로 변환
- 다양한 Markdown 확장 기능 지원 (코드 블록, 테이블, 구문 강조 등)
- 반응형 디자인이 적용된 HTML 출력
- 명령줄 인터페이스 제공

### 사용 방법

1. 명령줄에서 직접 사용:
```bash
python markdown_to_html.py input.md -o output.html
```

2. 대화형 모드:
```bash
python markdown_to_html.py
```
그런 다음 Markdown 텍스트를 입력하고 Ctrl+D를 눌러 입력을 완료한 후, 출력 파일 경로를 입력하면 됩니다.

### 지원하는 Markdown 확장 기능

- `fenced_code`: 코드 블록 지원
- `tables`: 테이블 지원
- `codehilite`: 구문 강조
- `toc`: 목차 생성
- `meta`: 메타데이터 지원
- `nl2br`: 줄바꿈을 `<br>`로 변환
- `sane_lists`: 목록 처리 개선
- `smarty`: 스마트 따옴표 및 대시 변환

### 확장 기능 지정 방법

```bash
python markdown_to_html.py input.md -o output.html -e fenced_code tables codehilite
```

## Markdown 이미지 처리 및 Pandas 데이터 처리

이 프로젝트는 Markdown 파일의 이미지를 처리하고 Pandas를 사용하여 데이터를 처리하는 기능을 제공합니다.

### 주요 기능

- Markdown 이미지 처리
  - 상대 경로 이미지를 절대 경로로 변환
  - 이미지 크기 자동 조정 (너무 큰 이미지 리사이징)
  - 이미지를 base64로 인코딩하여 HTML에 내장
  - 이미지 파일 존재 여부 확인 및 경고 메시지 출력

- Pandas 데이터 처리
  - 다양한 형식의 데이터 파일 지원 (CSV, Excel, JSON, HTML, SQL)
  - DataFrame을 Markdown 테이블로 변환
  - 데이터 크기 제한 (행 수, 열 수)
  - 데이터 테이블을 Markdown 문서에 자동 추가

### 사용 방법

1. 기본 사용법:
```bash
python markdown_pandas_processor.py input.md -o output.html
```

2. 이미지 처리와 함께 사용:
```bash
python markdown_pandas_processor.py input.md -o output.html -b /path/to/images
```

3. 데이터 파일 처리와 함께 사용:
```bash
python markdown_pandas_processor.py input.md -o output.html -d data.csv
```

4. 모든 기능 함께 사용:
```bash
python markdown_pandas_processor.py input.md -o output.html -b /path/to/images -d data.csv
```

### 이미지 처리 옵션

- `-b, --base-dir`: 이미지 파일의 기본 디렉토리 지정
- 이미지 크기 조정: 너무 큰 이미지(800px 초과)는 자동으로 리사이징
- 이미지 최적화: 이미지 품질 조정 및 최적화

### 데이터 처리 옵션

- `-d, --data`: 처리할 데이터 파일 경로
- 지원하는 파일 형식:
  - CSV (.csv)
  - Excel (.xlsx, .xls)
  - JSON (.json)
  - HTML (.html)
  - SQL (.sql) - 쿼리 텍스트만 반환
- 데이터 크기 제한:
  - 기본적으로 최대 100행으로 제한
  - 열 수 제한 가능

### 예제

1. 이미지가 포함된 Markdown 파일 변환:
```bash
python markdown_pandas_processor.py blog_post.md -o blog_post.html -b ./images
```

2. 데이터 테이블이 포함된 보고서 생성:
```bash
python markdown_pandas_processor.py report_template.md -o report.html -d sales_data.xlsx
```

3. 이미지와 데이터 테이블이 모두 포함된 문서 생성:
```bash
python markdown_pandas_processor.py presentation.md -o presentation.html -b ./images -d survey_results.csv
``` 

# bpdt
baro project developer tool

[diff] tool = winmerge

 

[merge] tool = winmerge

 

[difftool "WinMerge"]

    cmd = \"C:\\Program Files\\WinMerge\\WinMergeU.exe\" -e -u -dl \"Old $BASE\" -dr \"New $BASE\" \"$LOCAL\" \"$REMOTE\"

    trustExitCode = true

 

[mergetool "WinMerge"]

    cmd = \"C:\\Program Files\\WinMerge\\WinMergeU.exe\" -e -u -dl \"Local\" -dm \"Base\" -dr \"Remote\" \"$LOCAL\" \"$BASE\" \"$REMOTE\" -o \"$MERGED\"

    trustExitCode = true

    keepBackup = false

[노트패드 설정]
    git config --global core.editor "'C:/Program Files/Notepad++/notepad++.exe' -multiInst -nosession"


[설정 확인]
    git config --list
    git config user.name 

[저장소 생성]  
    git remote add origin [repository 주소]
    git remote -v
    git push -u orign master
    

[챠트 참조사이트]
    https://echarts.apache.org/examples/en/editor.html?c=line-race
    
[주식 참조사이트]
 https://github.com/hyunyulhenry/quant_py/blob/main/data_korea.ipynb
	
	
