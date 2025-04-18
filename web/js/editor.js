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
