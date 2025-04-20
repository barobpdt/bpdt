document.addEventListener('DOMContentLoaded', function() {
	// Ace Editor 모듈화 설정
	const editor = ace.require("ace/editor").Editor;
	const EditSession = ace.require("ace/edit_session").EditSession;
	const UndoManager = ace.require("ace/undomanager").UndoManager;
	const Mode = ace.require("ace/mode/javascript").Mode;
	const PythonMode = ace.require("ace/mode/python").Mode;
	const Theme = ace.require("ace/theme/monokai").Theme;

	// 에디터 초기화
	const editorInstance = ace.edit("editor");

	// 기본 설정
	editorInstance.setTheme("ace/theme/monokai");
	editorInstance.session.setMode("ace/mode/javascript");
	editorInstance.setOptions({
		enableBasicAutocompletion: true,
		enableLiveAutocompletion: true,
		enableSnippets: true,
		fontSize: "14px",
		showPrintMargin: false,
		showGutter: true,
		highlightActiveLine: true,
		wrap: true,
		useSoftTabs: true,
		tabSize: 4
	});

	// 붙여넣기 이벤트 처리 및 자동 선택 기능
	let pasteStartPosition = null;
	
	// 붙여넣기 시작 위치 저장
	editorInstance.on('beforepaste', function(e) {
		pasteStartPosition = editorInstance.getCursorPosition();
	});
	
	// 붙여넣기 후 자동 선택 적용
	editorInstance.on('paste', function(e) {
		if (pasteStartPosition) {
			setTimeout(function() {
				// 현재 커서 위치 (붙여넣기 후 위치)
				const currentPosition = editorInstance.getCursorPosition();
				
				// 선택 범위 설정 (붙여넣기 시작 위치에서 현재 위치까지)
				editorInstance.selection.setRange({
					start: pasteStartPosition,
					end: currentPosition
				});
				
				// 위치 초기화
				pasteStartPosition = null;
			}, 0);
		}
	});

	// 자동 완성 설정
	const langTools = ace.require("ace/ext/language_tools");
	editorInstance.completers = [langTools.textCompleter];

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

helloWorld();`
	};

	// 초기 코드 설정
	editorInstance.setValue(defaultCode.javascript); 

	// 언어 모드 전환 함수
	function switchLanguageMode(language) {
		if (language === 'python') {
			editorInstance.session.setMode("ace/mode/python");
			editorInstance.setValue(defaultCode.python);
			setupPythonCompletions();
		} else if (language === 'javascript') {
			editorInstance.session.setMode("ace/mode/javascript");
			editorInstance.setValue(defaultCode.javascript);
			setupJavaScriptCompletions();
		}
	}

	// Python 자동완성 설정
	function setupPythonCompletions() {
		const pythonCompleter = {
			getCompletions: function(editor, session, pos, prefix, callback) {
				const wordList = [
					"def", "class", "if", "else", "elif", "for", "while", "try", "except", "finally",
					"import", "from", "as", "return", "break", "continue", "pass", "raise", "with",
					"True", "False", "None", "and", "or", "not", "is", "in", "lambda", "global",
					"print", "len", "range", "list", "dict", "set", "tuple", "str", "int", "float"
				];
				
				const completions = wordList.map(word => ({
					value: word,
					meta: "python"
				}));
				
				callback(null, completions);
			}
		};
		
		editorInstance.completers = [pythonCompleter, langTools.textCompleter];
	}

	// JavaScript 자동완성 설정
	function setupJavaScriptCompletions() {
		editorInstance.completers = [langTools.textCompleter];
	}

	// 언어 전환 버튼 추가
	const languageButtons = document.createElement('div');
	languageButtons.style.position = 'absolute';
	languageButtons.style.top = '10px';
	languageButtons.style.right = '10px';
	languageButtons.style.zIndex = '1000';

	const jsButton = document.createElement('button');
	jsButton.textContent = 'JavaScript';
	jsButton.onclick = () => switchLanguageMode('javascript');
	jsButton.style.marginRight = '5px';

	const pythonButton = document.createElement('button');
	pythonButton.textContent = 'Python';
	pythonButton.onclick = () => switchLanguageMode('python');

	languageButtons.appendChild(jsButton);
	languageButtons.appendChild(pythonButton);
	document.body.appendChild(languageButtons);

	// 자동완성 항목 추가 함수
	window.appendAutocomplete = function(items, language = null) {
		// 현재 언어 모드 확인
		const currentMode = editorInstance.session.getMode().$id;
		const currentLanguage = currentMode.split('/').pop();
		
		// 언어가 지정되지 않았거나 현재 언어와 일치하는 경우에만 추가
		if (!language || language === currentLanguage) {
			// 현재 자동완성 설정 가져오기
			const currentCompleters = editorInstance.completers || [];
			
			// 새로운 자동완성 설정 생성
			const customCompleter = {
				getCompletions: function(editor, session, pos, prefix, callback) {
					// 기존 자동완성 항목 가져오기
					const existingCompletions = [];
					
					// 기존 자동완성 설정에서 항목 가져오기
					currentCompleters.forEach(completer => {
						if (completer.getCompletions) {
							completer.getCompletions(editor, session, pos, prefix, (err, results) => {
								if (!err && results) {
									existingCompletions.push(...results);
								}
							});
						}
					});
					
					// 새로운 자동완성 항목 추가
					const newCompletions = items.map(item => {
						// 문자열인 경우 객체로 변환
						if (typeof item === 'string') {
							return {
								value: item,
								meta: currentLanguage
							};
						}
						return item;
					});
					
					// 모든 자동완성 항목 합치기
					const allCompletions = [...existingCompletions, ...newCompletions];
					
					// 중복 제거 (value 기준)
					const uniqueCompletions = [];
					const seen = new Set();
					
					allCompletions.forEach(item => {
						if (!seen.has(item.value)) {
							seen.add(item.value);
							uniqueCompletions.push(item);
						}
					});
					
					callback(null, uniqueCompletions);
				}
			};
			
			// 자동완성 설정 업데이트
			editorInstance.completers = [customCompleter];
			
			console.log(`Added ${items.length} autocomplete items for ${currentLanguage}`);
		}
	};
	
	// 예시: 자동완성 항목 추가 버튼
	const addAutocompleteButton = document.createElement('button');
	addAutocompleteButton.textContent = 'Add Autocomplete';
	addAutocompleteButton.onclick = function() {
		// 예시 자동완성 항목 추가
		const customItems = [
			{ value: 'customFunction', meta: 'function', doc: 'A custom function' },
			{ value: 'customVariable', meta: 'variable', doc: 'A custom variable' },
			'customString', // 문자열로도 추가 가능
			{ value: 'customObject', meta: 'object', doc: 'A custom object' }
		];
		
		appendAutocomplete(customItems);
		alert('Custom autocomplete items added!');
	};
	addAutocompleteButton.style.marginRight = '5px';
	languageButtons.appendChild(addAutocompleteButton);

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
