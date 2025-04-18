function initUpload() {
	// DOM 요소
	const uploadArea = document.getElementById('uploadArea');
	const fileInput = document.getElementById('fileInput');
	const fileList = document.getElementById('fileList');
	const uploadButton = document.getElementById('uploadButton');
	const clearButton = document.getElementById('clearButton');
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
		const files = Array.from(e.dataTransfer.files);
		files.forEach(addFileToList);
		cf.files=files
	});

	fileInput.addEventListener('change', (e) => {
		const files = Array.from(e.target.files);
		files.forEach(addFileToList);
	});

	uploadButton.addEventListener('click', () => {
		const files = Array.from(fileList.children).map(item => ({
			name: item.dataset.filename,
			size: parseInt(item.querySelector('.file-size').textContent)
		}));
		if (files.length > 0) {
			uploadFiles(files);
		}
	});

	clearButton.addEventListener('click', () => {
		fileList.innerHTML = '';
		uploadProgress.innerHTML = '';
	}); 

}