// 페이지 설정
const pageInfo = {
	url: 'ws://localhost:8080' // 웹소켓 서버 URL (필요에 따라 변경)
	, maxRetries: 3              // 최대 재시도 횟수
	, retryInterval: 2000        // 재시도 간격 (밀리초)
	, currentRetry: 0             // 현재 재시도 횟수
	, editor: null
	, chunkSize: 64 * 1024 // 64KB 청크 크기
	, serverUrl: 'http://localhost:8080/upload' // 파일 업로드 서버 URL
	, maxRetries: 3 // 최대 재시도 횟수
	, retryInterval: 2000 // 재시도 간격 (밀리초)
	
};

function websocketConnect() {
    
    // 웹소켓 연결 함수
    function connectWebSocket() {
        try {
            console.log(`웹소켓 연결 시도 중... (시도 ${pageInfo.currentRetry + 1}/${pageInfo.maxRetries})`);
            
            // 웹소켓 객체 생성
            const ws = new WebSocket(pageInfo.url);
            pageInfo.ws = ws;
            
            // 연결 성공 이벤트
            ws.onopen = function() {
                console.log('웹소켓 연결 성공!');
                pageInfo.currentRetry = 0; // 연결 성공 시 재시도 카운터 초기화
                
                // 연결 상태 표시
                updateConnectionStatus('연결됨', 'success');
                
                // 서버에 연결 성공 메시지 전송
                ws.send(JSON.stringify({
                    type: 'connection',
                    status: 'connected',
                    message: '클라이언트가 연결되었습니다.'
                }));
            };
            
            // 메시지 수신 이벤트
            ws.onmessage = function(event) {
                try {
                    const data = JSON.parse(event.data);
                    console.log('서버로부터 메시지 수신:', data);
                    
                    // 메시지 타입에 따른 처리
                    handleServerMessage(data);
                } catch (error) {
                    console.error('메시지 파싱 오류:', error);
                }
            };
            
            // 연결 종료 이벤트
            ws.onclose = function(event) {
                console.log('웹소켓 연결 종료:', event.code, event.reason);
                updateConnectionStatus('연결 끊김', 'error');
                
                // 정상 종료가 아닌 경우 재연결 시도
                if (!event.wasClean && pageInfo.currentRetry < pageInfo.maxRetries) {
                    pageInfo.currentRetry++;
                    console.log(`${pageInfo.retryInterval/1000}초 후 재연결 시도...`);
                    
                    setTimeout(connectWebSocket, pageInfo.retryInterval);
                } else if (pageInfo.currentRetry >= pageInfo.maxRetries) {
                    console.error('최대 재시도 횟수에 도달했습니다. 연결을 포기합니다.');
                    updateConnectionStatus('연결 실패', 'error');
                }
            };
            
            // 오류 이벤트
            ws.onerror = function(error) {
                console.error('웹소켓 오류:', error);
                updateConnectionStatus('연결 오류', 'error');
            };
            
        } catch (error) {
            console.error('웹소켓 연결 중 오류 발생:', error);
            
            // 오류 발생 시 재연결 시도
            if (pageInfo.currentRetry < pageInfo.maxRetries) {
                pageInfo.currentRetry++;
                console.log(`${pageInfo.retryInterval/1000}초 후 재연결 시도...`);
                
                setTimeout(connectWebSocket, pageInfo.retryInterval);
            } else {
                console.error('최대 재시도 횟수에 도달했습니다. 연결을 포기합니다.');
                updateConnectionStatus('연결 실패', 'error');
            }
        }
    }
    
    // 서버 메시지 처리 함수
    function handleServerMessage(data) {
        switch (data.type) {
            case 'code_update':
                // 서버에서 코드 업데이트를 받은 경우
                if (data.code !== undefined) {
                    editor.setValue(data.code);
                }
                break;
            case 'language_change':
                // 서버에서 언어 변경을 받은 경우
                if (data.language) {
                    document.getElementById('language-select').value = data.language;
                    editor.session.setMode(`ace/mode/${data.language}`);
                }
                break;
            case 'theme_change':
                // 서버에서 테마 변경을 받은 경우
                if (data.theme) {
                    document.getElementById('theme-select').value = data.theme;
                    editor.setTheme(`ace/theme/${data.theme}`);
                }
                break;
            default:
                console.log('알 수 없는 메시지 타입:', data.type);
        }
    }
    
    // 연결 상태 표시 함수
    function updateConnectionStatus(status, type) {
        // 상태 표시 요소가 없으면 생성
        if (!document.getElementById('connection-status')) {
            const statusElement = document.createElement('div');
            statusElement.id = 'connection-status';
            statusElement.className = 'connection-status';
            document.querySelector('.container').appendChild(statusElement);
        }
        
        const statusElement = document.getElementById('connection-status');
        statusElement.textContent = `서버 상태: ${status}`;
        statusElement.className = `connection-status ${type}`;
    }
    
    // 초기 웹소켓 연결
    connectWebSocket();
    
    // 수동 재연결 버튼 추가
    const reconnectButton = document.createElement('button');
    reconnectButton.id = 'reconnect-btn';
    reconnectButton.textContent = '서버 재연결';
    reconnectButton.className = 'reconnect-button';
    reconnectButton.addEventListener('click', function() {
        pageInfo.currentRetry = 0; // 재시도 카운터 초기화
        connectWebSocket();
    });
    document.querySelector('.buttons').appendChild(reconnectButton);
}
 

// 파일 업로드 함수
function uploadFile(file) {
	return new Promise((resolve, reject) => {
		const reader = new FileReader();
		const fileId = generateFileId();
		const totalChunks = Math.ceil(file.size / pageInfo.chunkSize);
		let currentChunk = 0;
		
		// 파일 정보 표시
		updateUploadStatus(`파일 업로드 시작: ${file.name} (${formatFileSize(file.size)})`);
		
		// 파일 청크 읽기
		reader.onload = function(e) {
			const chunk = e.target.result;
			sendChunk(file, fileId, chunk, currentChunk, totalChunks)
				.then(() => {
					currentChunk++;
					
					// 진행률 업데이트
					const progress = Math.round((currentChunk / totalChunks) * 100);
					updateUploadStatus(`파일 업로드 중: ${file.name} (${progress}%)`);
					
					// 다음 청크 읽기 또는 완료
					if (currentChunk < totalChunks) {
						readNextChunk();
					} else {
						// 모든 청크 전송 완료
						finalizeUpload(file, fileId, totalChunks)
							.then(() => {
								updateUploadStatus(`파일 업로드 완료: ${file.name}`, 'success');
								resolve();
							})
							.catch(error => {
								updateUploadStatus(`파일 업로드 실패: ${file.name}`, 'error');
								reject(error);
							});
					}
				})
				.catch(error => {
					updateUploadStatus(`청크 전송 실패: ${file.name}`, 'error');
					reject(error);
				});
		};
		
		// 다음 청크 읽기 함수
		function readNextChunk() {
			const start = currentChunk * pageInfo.chunkSize;
			const end = Math.min(start + pageInfo.chunkSize, file.size);
			reader.readAsArrayBuffer(file.slice(start, end));
		}
		
		// 첫 번째 청크 읽기 시작
		readNextChunk();
	});
}

// 청크 전송 함수
function sendChunk(file, fileId, chunk, chunkIndex, totalChunks) {
	return new Promise((resolve, reject) => {
		const formData = new FormData();
		formData.append('fileId', fileId);
		formData.append('fileName', file.name);
		formData.append('fileType', file.type);
		formData.append('chunkIndex', chunkIndex);
		formData.append('totalChunks', totalChunks);
		formData.append('chunk', new Blob([chunk]));
		
		let retryCount = 0;
		
		function attemptUpload() {
			fetch(pageInfo.serverUrl, {
				method: 'POST',
				body: formData
			})
			.then(response => {
				if (!response.ok) {
					throw new Error(`HTTP error! status: ${response.status}`);
				}
				return response.json();
			})
			.then(data => {
				resolve(data);
			})
			.catch(error => {
				console.error(`청크 ${chunkIndex + 1}/${totalChunks} 전송 실패:`, error);
				
				// 재시도 로직
				if (retryCount < pageInfo.maxRetries) {
					retryCount++;
					console.log(`${pageInfo.retryInterval/1000}초 후 재시도... (시도 ${retryCount}/${pageInfo.maxRetries})`);
					setTimeout(attemptUpload, pageInfo.retryInterval);
				} else {
					reject(new Error(`청크 ${chunkIndex + 1}/${totalChunks} 전송 실패: 최대 재시도 횟수 초과`));
				}
			});
		}
		
		attemptUpload();
	});
}

// 업로드 완료 처리 함수
function finalizeUpload(file, fileId, totalChunks) {
	return new Promise((resolve, reject) => {
		fetch(`${pageInfo.serverUrl}/complete`, {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({
				fileId: fileId,
				fileName: file.name,
				fileType: file.type,
				totalChunks: totalChunks,
				fileSize: file.size
			})
		})
		.then(response => {
			if (!response.ok) {
				throw new Error(`HTTP error! status: ${response.status}`);
			}
			return response.json();
		})
		.then(data => {
			resolve(data);
		})
		.catch(error => {
			console.error('업로드 완료 처리 실패:', error);
			reject(error);
		});
	});
}

// 파일 ID 생성 함수
function generateFileId() {
	return 'file_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
}

// 파일 크기 포맷 함수
function formatFileSize(bytes) {
	if (bytes === 0) return '0 Bytes';
	const k = 1024;
	const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
	const i = Math.floor(Math.log(bytes) / Math.log(k));
	return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

// 업로드 상태 표시 함수
function updateUploadStatus(message, type = 'info') {
	// 상태 표시 요소가 없으면 생성
	if (!document.getElementById('upload-status')) {
		const statusElement = document.createElement('div');
		statusElement.id = 'upload-status';
		statusElement.className = 'upload-status';
		document.querySelector('.container').appendChild(statusElement);
	}
	
	const statusElement = document.getElementById('upload-status');
	statusElement.textContent = message;
	statusElement.className = `upload-status ${type}`;
}

function setUploadFileEvent() {
	// 파일 업로드 버튼 추가
    const uploadButton = document.createElement('button');
    uploadButton.id = 'upload-btn';
    uploadButton.textContent = '파일 업로드';
    uploadButton.className = 'upload-button';
    
    // 파일 입력 요소 생성
    const fileInput = document.createElement('input');
    fileInput.type = 'file';
    fileInput.id = 'file-input';
    fileInput.style.display = 'none';
    document.body.appendChild(fileInput);
    
    // 파일 선택 이벤트
    fileInput.addEventListener('change', function(e) {
        const files = e.target.files;
        if (files.length > 0) {
            const file = files[0];
            uploadFile(file)
                .then(() => {
                    console.log('파일 업로드 성공:', file.name);
                })
                .catch(error => {
                    console.error('파일 업로드 실패:', error);
                });
        }
    });
    
    // 업로드 버튼 클릭 이벤트
    uploadButton.addEventListener('click', function() {
        fileInput.click();
    });
    
    // 버튼 추가
    document.querySelector('.buttons').appendChild(uploadButton);
}

function addPythonCompletion() {
	   // Python 자동완성 워커 설정
    const pythonWorker = editor.session.getUseWorker();
    editor.session.setUseWorker(true);
    
    // Python 자동완성 설정
    const langTools = ace.require("ace/ext/language_tools");
    editor.completers = [langTools.snippetCompleter, langTools.textCompleter];
    
    // Python 스니펫 추가
    const pythonSnippets = [
        {
            name: "def",
            content: "def ${1:function_name}(${2:parameters}):\n\t${3:pass}",
            tabTrigger: "def"
        },
        {
            name: "class",
            content: "class ${1:ClassName}:\n\tdef __init__(self):\n\t\t${2:pass}",
            tabTrigger: "class"
        },
        {
            name: "if",
            content: "if ${1:condition}:\n\t${2:pass}",
            tabTrigger: "if"
        },
        {
            name: "for",
            content: "for ${1:item} in ${2:items}:\n\t${3:pass}",
            tabTrigger: "for"
        },
        {
            name: "while",
            content: "while ${1:condition}:\n\t${2:pass}",
            tabTrigger: "while"
        }
    ];
    
    // 스니펫 등록
    pythonSnippets.forEach(snippet => {
        langTools.addCompleter({
            getCompletions: function(editor, session, pos, prefix, callback) {
                callback(null, [snippet]);
            }
        });
    });
}

document.addEventListener('DOMContentLoaded', function() {
    // Ace 에디터 초기화
    const editor = ace.edit("editor");
    editor.setTheme("ace/theme/monokai");
    editor.session.setMode("ace/mode/python");
    editor.setFontSize(14);
    editor.setOptions({
        enableBasicAutocompletion: true,
        enableLiveAutocompletion: true,
        enableSnippets: true,
        showPrintMargin: false,
        showGutter: true,
        highlightActiveLine: true,
        wrap: true,
        useSoftTabs: true,
        tabSize: 4
    });

    // 기본 코드 템플릿
    const defaultCode = {
        python: `# Python 코드 예제
def hello_world():
    print("Hello, World!")

hello_world()`,
        javascript: `// JavaScript 코드 예제
function helloWorld() {
    console.log("Hello, World!");
}

helloWorld();`,
        html: `<!DOCTYPE html>
<html>
<head>
    <title>HTML 예제</title>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>이것은 HTML 예제입니다.</p>
</body>
</html>`,
        css: `/* CSS 예제 */
body {
    font-family: Arial, sans-serif;
    margin: 0;
    padding: 20px;
    background-color: #f0f0f0;
}

h1 {
    color: #333;
    text-align: center;
}

p {
    color: #666;
    line-height: 1.6;
}`
    };

    // 초기 코드 설정
    editor.setValue(defaultCode.python);

    // 언어 선택 이벤트
    document.getElementById('language-select').addEventListener('change', function() {
        const language = this.value;
        editor.session.setMode(`ace/mode/${language}`);
        editor.setValue(defaultCode[language]);
    });

    // 테마 선택 이벤트
    document.getElementById('theme-select').addEventListener('change', function() {
        const theme = this.value;
        editor.setTheme(`ace/theme/${theme}`);
    });

    // 글꼴 크기 선택 이벤트
    document.getElementById('font-size-select').addEventListener('change', function() {
        const fontSize = this.value;
        editor.setFontSize(fontSize);
    });

    // 들여쓰기 타입 선택 이벤트
    document.getElementById('indent-type').addEventListener('change', function() {
        const useSoftTabs = this.value === 'spaces';
        editor.setOptions({
            useSoftTabs: useSoftTabs
        });
    });

    // 들여쓰기 크기 선택 이벤트
    document.getElementById('indent-size').addEventListener('change', function() {
        const tabSize = parseInt(this.value);
        editor.setOptions({
            tabSize: tabSize
        });
    });

    // 저장 버튼 이벤트
    document.getElementById('save-btn').addEventListener('click', function() {
        const code = editor.getValue();
        const language = document.getElementById('language-select').value;
        const extension = getFileExtension(language);
        
        // 파일 다운로드
        downloadFile(code, `code.${extension}`);
    });

    // 불러오기 버튼 이벤트
    document.getElementById('load-btn').addEventListener('click', function() {
        // 파일 입력 요소 생성
        const fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.accept = '.txt,.py,.js,.html,.css';
        
        fileInput.addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (file) {
                const reader = new FileReader();
                reader.onload = function(e) {
                    const content = e.target.result;
                    editor.setValue(content);
                    
                    // 파일 확장자에 따라 언어 모드 설정
                    const extension = file.name.split('.').pop().toLowerCase();
                    const language = getLanguageFromExtension(extension);
                    if (language) {
                        document.getElementById('language-select').value = language;
                        editor.session.setMode(`ace/mode/${language}`);
                    }
                };
                reader.readAsText(file);
            }
        });
        
        fileInput.click();
    });

    // 지우기 버튼 이벤트
    document.getElementById('clear-btn').addEventListener('click', function() {
        if (confirm('에디터의 모든 내용을 지우시겠습니까?')) {
            editor.setValue('');
        }
    });

    // 파일 확장자 가져오기 함수
    function getFileExtension(language) {
        const extensions = {
            'python': 'py',
            'javascript': 'js',
            'html': 'html',
            'css': 'css'
        };
        return extensions[language] || 'txt';
    }

    // 확장자로부터 언어 가져오기 함수
    function getLanguageFromExtension(extension) {
        const languages = {
            'py': 'python',
            'js': 'javascript',
            'html': 'html',
            'css': 'css'
        };
        return languages[extension] || null;
    }

    // 파일 다운로드 함수
    function downloadFile(content, filename) {
        const blob = new Blob([content], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }
}); 


function setDragDropArea() {
	// 드래그 앤 드롭 영역 요소
    const dragDropArea = document.getElementById("drag-drop-area");

    // 드래그 앤 드롭 이벤트 처리
    ["dragenter", "dragover", "dragleave", "drop"].forEach(eventName => {
        dragDropArea.addEventListener(eventName, preventDefaults, false);
    });

    function preventDefaults(e) {
        e.preventDefault();
        e.stopPropagation();
    }

    ["dragenter", "dragover"].forEach(eventName => {
        dragDropArea.addEventListener(eventName, highlight, false);
    });

    ["dragleave", "drop"].forEach(eventName => {
        dragDropArea.addEventListener(eventName, unhighlight, false);
    });

    function highlight(e) {
        dragDropArea.classList.add("drag-over");
    }

    function unhighlight(e) {
        dragDropArea.classList.remove("drag-over");
    }

    // 파일 드롭 이벤트
    dragDropArea.addEventListener("drop", handleDrop, false);

    function handleDrop(e) {
        const dt = e.dataTransfer;
        const files = dt.files;
        handleFiles(files);
    }

    // 파일 처리 함수
    function handleFiles(files) {
        if (files.length > 0) {
            const file = files[0];
            loadFile(file);
            dragDropArea.classList.remove("active");
        }
    }

    // 클릭으로 파일 업로드
    dragDropArea.addEventListener("click", () => {
        const input = document.createElement("input");
        input.type = "file";
        input.accept = ".txt,.py,.js,.html,.css";
        input.onchange = (e) => {
            if (e.target.files.length > 0) {
                handleFiles(e.target.files);
            }
        };
        input.click();
    });

    // 에디터 영역에 마우스 진입 시 드래그 앤 드롭 영역 표시
    document.querySelector("main").addEventListener("mouseenter", () => {
        dragDropArea.classList.add("active");
    });

    // 에디터 영역에서 마우스 이탈 시 드래그 앤 드롭 영역 숨김
    document.querySelector("main").addEventListener("mouseleave", () => {
        dragDropArea.classList.remove("active");
    });
}