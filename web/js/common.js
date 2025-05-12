/* 
페이지 공통설정 
*/
const cf={}
const pageInfo = {
	editor: null
	/* 웹소켓 설정 */
    , websocket: null
    , wsUrl:'ws://localhost:8092/chat'
 	, wsCallback: null
	, wsStatus:''
    , wsType:''
    , wsMode: ''
    , wsVersion:'1.0'
	, currentRetry: 0             // 현재 재시도 횟수
	, maxRetries: 5 // 최대 재시도 횟수
	, retryInterval: 2000 // 재시도 간격 (밀리초)
	/* 기타 공통 설정 */
	, dumyDiv: document.createElement('div')
};

const impl = (obj, dest) => {
    for (var key in dest) {
        obj[key] = dest[key];
    }
    return obj;
};

const stringByteLength = (s,b,i,c) => {
    for(b=i=0;c=s.charCodeAt(i++);b+=c>>11?3:c>>7?2:1);
    return b
}
function qa(s) {
	const a = document.querySelectorAll(s)
	return a||[]
}
function qs(s) {
	const a = document.querySelector(s)
	return a||pageInfo.dumyDiv
}
function appendParam() {
	let s=''
	Array.from(arguments).map((c,n)=>s+=(n>0?'|':'')+c)
	return s
}

function websocketConnect() {
    // 웹소켓 연결 함수
    let ws = null
    const isConnect = () => ws!=null
    function sendData(type, header, param) {
        let message='', contentType=''
		if( param instanceof FileReader ) {
			const ab = param.result
			console.log('@@ websocket send readyState == ', param.readyState, ab)
			message = btoa(String.fromCharCode.apply(null, new Uint8Array(ab)));
			contentType='base64'
		} else if( param && typeof(param)=='object' ) {
            contentType= 'json'
            message = JSON.stringify(param)
        } else {
            contentType = 'text'
            message = param
        }
        if(ws==null) {
            updateConnectionStatus('전송오류 웹소켓 미정의', 'error')
            return
        }
        const size = stringByteLength(message)
        const data = '@'+type+'::'+header+'\r\n'+size+'::'+contentType+'::'+pageInfo.wsVersion+'\r\n\r\n'+message
        console.log('@@ send\r\n@'+type+'::'+header+'\r\n'+size+'::'+contentType+'::'+pageInfo.wsVersion)
        ws.send(data)
    }    
    function connect(url) {
		if( url ) {
			pageInfo.wsUrl = url
		} else {
			url = pageInfo.wsUrl
		}
        try {
            // 웹소켓 객체 생성
            pageInfo.websocket = new WebSocket(url);
            // 연결 성공 이벤트
            ws=pageInfo.websocket
            ws.onopen = function() {
                console.log('웹소켓 연결 성공!');
                pageInfo.currentRetry = 0; // 연결 성공 시 재시도 카운터 초기화
                
                // 연결 상태 표시
                updateConnectionStatus('연결됨', 'success');
                
                // 서버에 연결 성공 메시지 전송
                sendData('connection','check',{status: 'connected', message: '클라이언트가 연결되었습니다.'});
            };
            
            // 메시지 수신 이벤트
            ws.onmessage = function(event) {
                try {
                    const pos = event.data.indexOf('\r\n\r\n')
                    console.log('@@ 서버로부터 메시지 수신: pos == '+pos);
                    if(pos!=-1) {
                        const header = event.data.substr(0,pos)
                        const message = event.data.substr(pos+4)
                        // const node = JSON.parse(message);
                        // 메시지 타입에 따른 처리
                        if( typeof(pageInfo.wsCallback)=='function' ) pageInfo.wsCallback(header, message);
                    }
                } catch (error) {
                    console.error('메시지 파싱 오류:', error);
                }
            };
            
            // 연결 종료 이벤트
            ws.onclose = function(event) {
                console.log('웹소켓 연결 종료:', event.code, event.reason);
                ws = null
                updateConnectionStatus('연결 끊김', 'error');
				if( pageInfo.wsMode=='close' ) return
                // 정상 종료가 아닌 경우 재연결 시도
                if (!event.wasClean && pageInfo.maxRetries && pageInfo.currentRetry < pageInfo.maxRetries) {
                    pageInfo.currentRetry++;
                    console.log(`${pageInfo.retryInterval/1000} 초 후 재연결 시도...`);
                    setTimeout(connect, pageInfo.retryInterval);
                } else if (pageInfo.currentRetry >= pageInfo.maxRetries) {
                    console.error('최대 재시도 횟수에 도달했습니다. 연결을 포기합니다.');
                    updateConnectionStatus('연결 실패', 'error');
                }
            };
            
            // 오류 이벤트
            ws.onerror = function(error) {
                ws = null
                console.error('웹소켓 오류:', error);
                updateConnectionStatus('연결 오류', 'error');
            };
            
        } catch (error) {
            ws = null
            console.error('웹소켓 연결 중 오류 발생:', error);
            if( pageInfo.wsMode=='close' ) return
            // 오류 발생 시 재연결 시도
            if ( pageInfo.maxRetries && pageInfo.currentRetry < pageInfo.maxRetries) {
                pageInfo.currentRetry++;
                console.log(`${pageInfo.retryInterval/1000}초 후 재연결 시도...`);
                
                setTimeout(connect, pageInfo.retryInterval);
            } else {
                console.error('최대 재시도 횟수에 도달했습니다. 연결을 포기합니다.');
                updateConnectionStatus('연결 실패', 'error');
            }
        }
    }   
    
    // 연결 상태 표시 함수
    function updateConnectionStatus(status, type) {
        pageInfo.wsStataus = status
        pageInfo.wsType = type
    }
    
    return { connect, isConnect, sendData }
}


function wsChunkUploadFiles(ws) {
	const info={ws, uploadFiles:null, fileIndex:1, currentFile:null, chunkSize: 64 * 1024 }
	function start(files) {
		info.fileIndex=0
		info.currentFile
		info.uploadFiles=[]
		if( Array.isArray(files) && files.length>0 ) {
			files.map(c=>info.uploadFiles.push({
				file:c,
				name:c.name,
				size:c.size,
				type:c.type,
				lastModified:c.lastModified,
				fieldId:'',
				progress: 0,
				currentIndex: 0,
				currentChunk: 0,
				currentSendSize: 0,
				totalChunks:0
			}))
			startUpload()
		}
	}
	function startUpload() {
		if( info.uploadFiles.length==0 ) {
			return updateUploadStatus(`업로드 파일이 없습니다`);
		}
		uploadFile(info.uploadFiles.splice(0,1)[0]).then(e=>start).catch(e=>start)
	}
	function uploadFile(cur) {
		info.currentFile = cur
		return new Promise((resolve, reject) => {
			const reader = new FileReader();
			const file = cur.file
			cur.fieldId = generateFileId();
			cur.totalChunks = Math.ceil(cur.size / info.chunkSize);
			// 파일 정보 표시
			updateUploadStatus(`파일 업로드 시작: ${cur.name} (${formatFileSize(cur.size)})`);
			
			// 파일 청크 읽기
			reader.onload = function(e) {
				const {fieldId, name, size, currentIndex, currentChunk, totalChunks, lastModified, type } = cur
				console.log('reader onload ', reader, ws, appendParam(fieldId, name, size, currentIndex, currentChunk, totalChunks, lastModified, type))
				ws.sendData('req_chunkFileUpload', appendParam(fieldId, name, size, currentIndex, currentChunk, totalChunks, lastModified, type), reader)
				
				cur.currentChunk += cur.currentSendSize
				// 다음 청크 읽기 또는 완료
				if (cur.currentChunk < cur.size ) {
					cur.progress = Math.round((cur.currentIndex / cur.totalChunks) * 100);
					
					cur.currentIndex++
					updateUploadStatus(`파일 업로드 중: ${cur.name} (${cur.progress}%)`);
					readNextChunk();
				} else {
					resolve()
				} 
			};
			
			// 다음 청크 읽기 함수
			function readNextChunk() {
				const start = cur.currentChunk;
				const end = Math.min(start + info.chunkSize, cur.size);
				cur.currentSendSize = end - start
				console.log('@@ readNextChunk ', start, end, cur.currentSendSize)
				reader.readAsArrayBuffer(file.slice(start, end));
			}
			
			// 첫 번째 청크 읽기 시작
			readNextChunk();
		});
		return {}
	} 

	// 업로드 상태 표시 함수
	function updateUploadStatus(message, type = 'info') {
		console.log('@@ upload status >> '+ message)
	}
	return {info, start}
}

// 파일아이디 생성 
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


