// 페이지 설정
function initChat() {
	const chatMessages = document.getElementById('chat-messages');
    const messageInput = document.getElementById('message-input');
    const sendButton = document.getElementById('send-button');
    const emojiButton = document.getElementById('emoji-button');
    const emojiPicker = document.getElementById('emoji-picker');
    const filePreview = document.getElementById('file-preview');
    
    // 선택된 파일 배열
    let selectedFiles = [];
    
    // 이모지 선택기 토글
    emojiButton.addEventListener('click', function() {
        emojiPicker.classList.toggle('active');
    });
    
    // 이모지 선택 이벤트
    document.querySelectorAll('.emoji').forEach(emoji => {
        emoji.addEventListener('click', function() {
            const emojiChar = this.getAttribute('data-emoji');
            insertEmoji(emojiChar);
        });
    });
    
    // 이모지 삽입 함수
    function insertEmoji(emoji) {
        const startPos = messageInput.selectionStart;
        const endPos = messageInput.selectionEnd;
        const textBefore = messageInput.value.substring(0, startPos);
        const textAfter = messageInput.value.substring(endPos);
        
        messageInput.value = textBefore + emoji + textAfter;
        messageInput.focus();
        messageInput.selectionStart = startPos + emoji.length;
        messageInput.selectionEnd = startPos + emoji.length;
    }
    
    /* 파일 업로드 이벤트
    const fileUpload = document.getElementById('file-upload');
    fileUpload.addEventListener('change', function() {
        const files = Array.from(this.files);
        
        if (files.length > 0) {
            files.forEach(file => {
                // 파일 크기 제한 (10MB)
                if (file.size > 10 * 1024 * 1024) {
                    alert(`파일 크기가 너무 큽니다: ${file.name}`);
                    return;
                }
                
                // 파일 추가
                selectedFiles.push(file);
                
                // 파일 미리보기 추가
                addFilePreview(file);
            });
            
            // 파일 입력 초기화
            this.value = '';
        }
    });
    */
	
    // 파일 미리보기 추가 함수
    function addFilePreview(file) {
        const fileItem = document.createElement('div');
        fileItem.className = 'file-item';
        
        // 파일 아이콘 결정
        let fileIcon = 'fa-file';
        if (file.type.startsWith('image/')) {
            fileIcon = 'fa-file-image';
        } else if (file.type.startsWith('video/')) {
            fileIcon = 'fa-file-video';
        } else if (file.type.startsWith('audio/')) {
            fileIcon = 'fa-file-audio';
        } else if (file.type.includes('pdf')) {
            fileIcon = 'fa-file-pdf';
        } else if (file.type.includes('word') || file.type.includes('doc')) {
            fileIcon = 'fa-file-word';
        } else if (file.type.includes('excel') || file.type.includes('sheet')) {
            fileIcon = 'fa-file-excel';
        } else if (file.type.includes('powerpoint') || file.type.includes('presentation')) {
            fileIcon = 'fa-file-powerpoint';
        }
        
        // 파일 아이템 HTML
        fileItem.innerHTML = `
            <i class="fas ${fileIcon} file-item-icon"></i>
            <span class="file-item-name">${file.name}</span>
            <i class="fas fa-times file-item-remove" data-filename="${file.name}"></i>
        `;
        
        filePreview.appendChild(fileItem);
        
        // 파일 제거 이벤트
        fileItem.querySelector('.file-item-remove').addEventListener('click', function() {
            const filename = this.getAttribute('data-filename');
            removeFile(filename);
            fileItem.remove();
        });
    }
    
    // 파일 제거 함수
    function removeFile(filename) {
        selectedFiles = selectedFiles.filter(file => file.name !== filename);
    }
    
    // 메시지 전송 이벤트
    sendButton.addEventListener('click', sendMessage);
    messageInput.addEventListener('keypress', function(e) {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
    
    // 메시지 전송 함수
    function sendMessage() {
        let messageText = messageInput.value.trim();
        
        // 메시지가 비어있고 파일도 없는 경우 전송하지 않음
        if (messageText === '' && selectedFiles.length === 0) {
            messageText = '내용을 입력하세요' 
            // return;
        }
        
        // 현재 시간
        const now = new Date();
        const timeString = now.getHours().toString().padStart(2, '0') + ':' + 
                           now.getMinutes().toString().padStart(2, '0');
        
        // 메시지 요소 생성
        const messageElement = document.createElement('div');
        messageElement.className = 'message sent';
        
        let messageContent = `<div class="message-content">`;
        
        // 텍스트 메시지 추가
        if (messageText !== '') {
            messageContent += `<p>${messageText}</p>`;
        }
        
        // 파일 메시지 추가
        if (selectedFiles.length > 0) {
            selectedFiles.forEach(file => {
                messageContent += `
                    <div class="file-message">
                        <i class="fas fa-paperclip file-icon"></i>
                        <span class="file-name">${file.name}</span>
                    </div>
                `;
            });
        }
        
        messageContent += `<span class="message-time">${timeString}</span></div>`;
        messageElement.innerHTML = messageContent;
        
        // 메시지 추가
        chatMessages.appendChild(messageElement);
        
        // 스크롤을 맨 아래로 이동
        chatMessages.scrollTop = chatMessages.scrollHeight;
        
        // 입력 필드 초기화
        messageInput.value = '';
        
        // 파일 초기화
        selectedFiles = [];
        filePreview.innerHTML = '';
        
        // 자동 응답 (데모용)
        setTimeout(autoReply, 1000);
    }
    
    // 자동 응답 함수 (데모용)
    function autoReply() {
        const now = new Date();
        const timeString = now.getHours().toString().padStart(2, '0') + ':' + 
                           now.getMinutes().toString().padStart(2, '0');
        
        const messageElement = document.createElement('div');
        messageElement.className = 'message received';
        
        const responses = [
            '메시지를 받았습니다. 확인해 주셔서 감사합니다.',
            '네, 알겠습니다. 도움이 필요하시면 언제든지 말씀해 주세요.',
            '좋은 질문이네요! 잠시만 기다려주세요.',
            '파일을 확인했습니다. 처리해 드리겠습니다.',
            '이모티콘이 정말 귀엽네요! 😊'
        ];
        
        const randomResponse = responses[Math.floor(Math.random() * responses.length)];
        
        messageElement.innerHTML = `
            <div class="message-content">
                <p>${randomResponse}</p>
                <span class="message-time">${timeString}</span>
            </div>
        `;
        
        chatMessages.appendChild(messageElement);
        chatMessages.scrollTop = chatMessages.scrollHeight;
    }
    
    // 텍스트 영역 자동 높이 조절
    messageInput.addEventListener('input', function() {
        this.style.height = 'auto';
        this.style.height = (this.scrollHeight) + 'px';
    });
    
    // 문서 클릭 시 이모지 선택기 닫기
    document.addEventListener('click', function(e) {
        if (!emojiButton.contains(e.target) && !emojiPicker.contains(e.target)) {
            emojiPicker.classList.remove('active');
        }
    });
    
    // 드래그 앤 드롭 파일 업로드
    chatMessages.addEventListener('dragover', function(e) {
        $('#uploadProgress').html('dragover ==>')
        e.preventDefault();
        e.stopPropagation();
        this.classList.add('drag-over');
    });
    
    chatMessages.addEventListener('dragleave', function(e) {
        e.preventDefault();
        e.stopPropagation();
        this.classList.remove('drag-over');
    });
    
    chatMessages.addEventListener('drop', function(e) {
        $('#uploadProgress').html('drop ==>')
        e.preventDefault();
        e.stopPropagation();
        this.classList.remove('drag-over');
        
        const files = Array.from(e.dataTransfer.files);
        
        if (files.length > 0) {
            files.forEach(file => {
                // 파일 크기 제한 (10MB)
                if (file.size > 10 * 1024 * 1024) {
                    alert(`파일 크기가 너무 큽니다: ${file.name}`);
                    return;
                }
                
                // 파일 추가
                selectedFiles.push(file);
                
                // 파일 미리보기 추가
                addFilePreview(file);
            });
        }
    });

    return { sendMessage, autoReply }	
}

function initUpload() {
	// DOM 요소
	const uploadArea = document.getElementById('uploadArea');
	const fileInput = document.getElementById('fileInput');
	const fileList = document.getElementById('fileList');
	const uploadProgress = document.getElementById('uploadProgress');

	// 파일 아이콘 매핑
	const fileIcons = {
		'image': '🖼️',
		'video': '🎥',
		'audio': '🎵',
		'document': '📄',
		'archive': '📦',
		'default': '📁'
	};

	// 파일 타입 매핑
	const fileTypes = {
		'image': ['.jpg', '.jpeg', '.png', '.gif', '.bmp', '.webp'],
		'video': ['.mp4', '.avi', '.mov', '.wmv', '.flv', '.mkv'],
		'audio': ['.mp3', '.wav', '.ogg', '.flac', '.m4a'],
		'document': ['.pdf', '.doc', '.docx', '.txt', '.rtf', '.odt'],
		'archive': ['.zip', '.rar', '.7z', '.tar', '.gz']
	};

	// 파일 크기 포맷팅
	function formatFileSize(bytes) {
		if (bytes === 0) return '0 Bytes';
		const k = 1024;
		const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
		const i = Math.floor(Math.log(bytes) / Math.log(k));
		return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
	}

	// 파일 타입 확인
	function getFileType(extension) {
		for (const [type, extensions] of Object.entries(fileTypes)) {
			if (extensions.includes(extension.toLowerCase())) {
				return type;
			}
		}
		return 'default';
	}

	// 파일 아이콘 가져오기
	function getFileIcon(filename) {
		const extension = '.' + filename.split('.').pop();
		const type = getFileType(extension);
		return fileIcons[type] || fileIcons.default;
	}

	// 파일 목록에 파일 추가
	function addFileToList(file) {
		const fileItem = document.createElement('div');
		fileItem.className = 'file-item';
		fileItem.dataset.filename = file.name;

		const fileIcon = document.createElement('span');
		fileIcon.className = 'file-icon';
		fileIcon.textContent = getFileIcon(file.name);

		const fileInfo = document.createElement('div');
		fileInfo.className = 'file-info';

		const fileName = document.createElement('div');
		fileName.className = 'file-name';
		fileName.textContent = file.name;

		const fileSize = document.createElement('div');
		fileSize.className = 'file-size';
		fileSize.textContent = formatFileSize(file.size);

		const removeButton = document.createElement('span');
		removeButton.className = 'remove-file';
		removeButton.innerHTML = '&times;';
		removeButton.onclick = () => removeFile(file.name);

		fileInfo.appendChild(fileName);
		fileInfo.appendChild(fileSize);
		fileItem.appendChild(fileIcon);
		fileItem.appendChild(fileInfo);
		fileItem.appendChild(removeButton);
		fileList.appendChild(fileItem);
	}

	// 파일 제거
	function removeFile(filename) {
		const fileItem = fileList.querySelector(`[data-filename="${filename}"]`);
		if (fileItem) {
			fileItem.remove();
		}
	}

	// 파일 업로드 진행 상황 표시
	function updateProgress(filename, progress) {
		const progressItem = uploadProgress.querySelector(`[data-filename="${filename}"]`);
		if (progressItem) {
			const progressFill = progressItem.querySelector('.progress-fill');
			progressFill.style.width = `${progress}%`;
		}
	}

	// 파일 업로드 시뮬레이션
	async function uploadFiles(files) {
		for (const file of files) {
			const progressItem = document.createElement('div');
			progressItem.className = 'progress-item';
			progressItem.dataset.filename = file.name;

			const progressBar = document.createElement('div');
			progressBar.className = 'progress-bar';

			const progressFill = document.createElement('div');
			progressFill.className = 'progress-fill';

			progressBar.appendChild(progressFill);
			progressItem.appendChild(progressBar);
			uploadProgress.appendChild(progressItem);

			// 업로드 시뮬레이션
			for (let i = 0; i <= 100; i += 10) {
				await new Promise(resolve => setTimeout(resolve, 200));
				updateProgress(file.name, i);
			}

			// 업로드 완료 후 파일 목록에서 제거
			setTimeout(() => {
				removeFile(file.name);
				progressItem.remove();
			}, 500);
		}
	}

	// 이벤트 리스너
	uploadArea.addEventListener('click', () => fileInput.click());

	uploadArea.addEventListener('dragover', (e) => {
		e.preventDefault();
		uploadArea.classList.add('dragover');
	});

	uploadArea.addEventListener('dragleave', () => {
		uploadArea.classList.remove('dragover');
	});

	uploadArea.addEventListener('drop', (e) => {
		e.preventDefault();
		uploadArea.classList.remove('dragover');
		pageInfo.files = Array.from(e.dataTransfer.files);
		pageInfo.files.forEach(addFileToList);
	});

	fileInput.addEventListener('change', (e) => {
		pageInfo.files = Array.from(e.target.files);
		pageInfo.files.forEach(addFileToList);
	});
	/*
	const uploadButton = document.getElementById('uploadButton');
	uploadButton.addEventListener('click', () => {
		const files = Array.from(fileList.children).map(item => ({
			name: item.dataset.filename,
			size: parseInt(item.querySelector('.file-size').textContent)
		}));
		if (files.length > 0) {
			uploadFiles(files);
		}
	});
	
	const clearButton = document.getElementById('clearButton');
	clearButton.addEventListener('click', () => {
		fileList.innerHTML = '';
		uploadProgress.innerHTML = '';
	}); 
	*/

}