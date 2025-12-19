import { useEffect, useState } from "react";
import AceEditor from "react-ace";

// Ace Editor 모드와 테마 import
import "ace-builds/src-noconflict/mode-javascript";
import "ace-builds/src-noconflict/theme-monokai";
import "ace-builds/src-noconflict/theme-github";
import "ace-builds/src-noconflict/ext-language_tools";

import { useProductStore } from "./store/useProductStore.js"

const clog = console.log

// Custom Hook: 초기 라우트 설정
function useInitialRoute(store, route) {
	useEffect(() => {
		store.routePage(route)
	}, [])
}

// Custom Hook: 라우트 변경 로깅
function useRouteLogger(store) {
	useEffect(() => {
		clog('Route changed:', store.currentPagePath)
	}, [store.currentPagePath])
}

// 기본 JavaScript 코드
const defaultCode = `// JavaScript Editor
function greet(name) {
  console.log('Hello, ' + name + '!');
  return 'Welcome to Ace Editor';
}

// 함수 실행
const message = greet('Developer');
console.log(message);

// 배열 예제
const numbers = [1, 2, 3, 4, 5];
const doubled = numbers.map(n => n * 2);
console.log('Doubled:', doubled);

// 객체 예제
const user = {
  name: 'John Doe',
  age: 30,
  skills: ['JavaScript', 'React', 'Node.js']
};

console.log('User:', user);
`;

function App() {
	const store = useProductStore()
	const [code, setCode] = useState(defaultCode)
	const [theme, setTheme] = useState('monokai')
	const [fontSize, setFontSize] = useState(14)

	// useEffect 대신 의미있는 이름의 custom hooks 사용
	useInitialRoute(store, 'main')
	useRouteLogger(store)

	// 코드 변경 핸들러
	const handleCodeChange = (newCode) => {
		setCode(newCode)
		clog('Code updated:', newCode.length, 'characters')
	}

	// 코드 실행 핸들러
	const runCode = () => {
		try {
			// 콘솔 출력을 캡처하기 위한 임시 로그 배열
			const logs = []
			const originalLog = console.log

			// console.log 오버라이드
			console.log = (...args) => {
				logs.push(args.join(' '))
				originalLog(...args)
			}

			// 코드 실행
			eval(code)

			// console.log 복원
			console.log = originalLog

			alert('코드 실행 완료!\n\n출력:\n' + logs.join('\n'))
		} catch (error) {
			alert('오류 발생:\n' + error.message)
		}
	}

	// 테마 토글
	const toggleTheme = () => {
		setTheme(prev => prev === 'monokai' ? 'github' : 'monokai')
	}

	clog('>>', store.currentPagePath)

	return (
		<div style={styles.container}>
			<div style={styles.header}>
				<h1 style={styles.title}>JavaScript Editor</h1>
				<div style={styles.controls}>
					<button onClick={toggleTheme} style={styles.button}>
						테마: {theme === 'monokai' ? '🌙 Dark' : '☀️ Light'}
					</button>
					<select
						value={fontSize}
						onChange={(e) => setFontSize(Number(e.target.value))}
						style={styles.select}
					>
						<option value={12}>12px</option>
						<option value={14}>14px</option>
						<option value={16}>16px</option>
						<option value={18}>18px</option>
						<option value={20}>20px</option>
					</select>
					<button onClick={runCode} style={styles.runButton}>
						▶ 실행
					</button>
				</div>
			</div>

			<div style={{ flex: 1, display: 'flex', flexDirection: 'column', minHeight: 0 }}>
				<AceEditor
					mode="javascript"
					theme={theme}
					value={code}
					onChange={handleCodeChange}
					name="javascript-editor"
					editorProps={{ $blockScrolling: true }}
					setOptions={{
						enableBasicAutocompletion: true,
						enableLiveAutocompletion: true,
						enableSnippets: true,
						showLineNumbers: true,
						tabSize: 2,
						fontSize: fontSize,
						showPrintMargin: false,
						highlightActiveLine: true,
						enableMultiselect: true,
					}}
					style={{
						width: '100%',
						height: '100%',
						borderRadius: '8px',
						boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)',
					}}
				/>
			</div>

			<div style={styles.footer}>
				<span style={styles.footerText}>
					줄 수: {code.split('\n').length} | 문자 수: {code.length}
				</span>
			</div>
		</div>
	);
}

// 스타일 정의
const styles = {
	container: {
		display: 'flex',
		flexDirection: 'column',
		height: '100vh',
		padding: '20px',
		backgroundColor: '#1e1e1e',
		fontFamily: 'Arial, sans-serif',
		overflow: 'hidden', // 페이지 스크롤 방지
		boxSizing: 'border-box',
	},
	header: {
		display: 'flex',
		justifyContent: 'space-between',
		alignItems: 'center',
		marginBottom: '15px',
		flexWrap: 'wrap',
		gap: '10px',
		flexShrink: 0, // 헤더 크기 고정
	},
	title: {
		color: '#fff',
		margin: 0,
		fontSize: '24px',
	},
	controls: {
		display: 'flex',
		gap: '10px',
		alignItems: 'center',
	},
	button: {
		padding: '8px 16px',
		backgroundColor: '#4a4a4a',
		color: '#fff',
		border: 'none',
		borderRadius: '4px',
		cursor: 'pointer',
		fontSize: '14px',
		transition: 'background-color 0.2s',
	},
	runButton: {
		padding: '8px 20px',
		backgroundColor: '#4CAF50',
		color: '#fff',
		border: 'none',
		borderRadius: '4px',
		cursor: 'pointer',
		fontSize: '14px',
		fontWeight: 'bold',
		transition: 'background-color 0.2s',
	},
	select: {
		padding: '8px 12px',
		backgroundColor: '#4a4a4a',
		color: '#fff',
		border: 'none',
		borderRadius: '4px',
		cursor: 'pointer',
		fontSize: '14px',
	},
	footer: {
		marginTop: '10px',
		padding: '10px',
		backgroundColor: '#2a2a2a',
		borderRadius: '4px',
		textAlign: 'center',
		flexShrink: 0, // 푸터 크기 고정
	},
	footerText: {
		color: '#aaa',
		fontSize: '12px',
	},
};

export default App;