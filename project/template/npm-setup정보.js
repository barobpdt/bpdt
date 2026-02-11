@baro.setNodeVersion() {
	@baro.cmdRun(c,'node -v',func(&s) {
		root=object('baro.services')
		s.findPos("\n")
		line=s.findPos("\n")
		root.set('@nodeVersion', line)
	})
}

##> cors 설정
res.setHeader('Access-Control-Allow-origin', '*');
res.setHeader('Access-Control-Allow-Credentials', 'true'); // 쿠키 주고받기 허용
res.end();

// 모든 허용
app.use(cors({
    origin: true, // 출처 허용 옵션
    credential: true // 사용자 인증이 필요한 리소스(쿠키 ..등) 접근
}));

// 특정주소 허용
const whitelist = ["http://localhost:8080", "http://localhost:8081"];
const corsOptions = {
  origin: function (origin, callback) { 
    if (whitelist.indexOf(origin) !== -1) { // 만일 whitelist 배열에 origin인자가 있을 경우
      callback(null, true); // cors 허용
    } else {
      callback(new Error("Not Allowed Origin!")); // cors 비허용
    }
  },
};
app.use(cors(corsOptions)); 

##> postgresql 설정
C:\Program Files\PostgreSQL\x.x\bin

psql postgres
create database nodedb;

##> 새로운 프로잭트 만들기
mkdir nodejs-api
cd nodejs-api
npm init -y
dotenv - npm i dotenv
npm install express --save
npm i nodemon
npm i pg-pool pg

 

/*
@baro.findBindPort(5173, func(port) {
	if(port) return print("front 데몬이 실행중입니다")
	print("xxxxxxx 프론트엔드 실행 시작 xxxxxxxxxxx")
	@baro.viteRunDev()
})
*/
@baro.viteCreate(projectPath, projectName) {
	cc=@baro.cmd('npm')
	not(projectPath) projectPath='c:/temp/vite'
	@baro.cmdRun(cc, 'cd c:/temp/vite')
	@baro.cmdRun(cc, 'npm create vite@latest sample-tailwind -- --template react')
	@baro.cmdRun(cc, 'cd sample-tailwind')
	@baro.cmdRun(cc, 'npm install react-router-dom')
	@baro.cmdRun(cc, 'npm install -D tailwindcss@3 postcss autoprefixer')
	@baro.cmdRun(cc, 'npx tailwindcss init -p')
	tailwinConfigFile = 'c:/temp/vite/tailwind.config.js'
	src=fileRead(tailwinConfigFile)
	saveConfig(src)
	saveConfig=func(&s) {
		ss=''
		content='"./index.html", "./src/**/*.{js,ts,jsx,tsx}"'
		left=s.findPos('content:') 
		c=s.ch()
		not(c)return print("@@ tailwinConfig 파일 수정오류 (경로:$tailwinConfigFile)")
		s.match()
		ss.add(left)
		ss.add("content: [$content]")
		ss.add(s)
		fileWrite(tailwinConfigFile,ss)
	};
	@baro.viteRunDev(pathJoin(projectPath,projectName) )
}
@baro.viteRunDev(projectPath, logPath) {
	not(projectPath) projectPath = 'C:/temp/vite/sample-baro1'
	not(logPath) logPath='log.txt'	
	npmCmd = 'npm run dev >> "$logPath" 2>&1'
	cc=@baro.cmd('frontend')
	@baro.cmdRun(cc, "cd $projectPath", @baro.frontendProc )
	@baro.cmdRun(cc, _s(npmCmd) )
	print("xxxxxxx 프론트엔드 실행중 xxxxxxxxxxx")
}


@baro.findBindPort(tcpPort, checkCallback) {
	cmd=@baro.cmd('userProc')
	funcParam(cmd,'type','findBindPort')
	funcParam(cmd,'tcpPort,checkCallback')
	@baro.cmdRun(cmd, "netstat -ano | findstr $tcpPort", @baro.userProc);

	addCmdWorker("port_kill=>netstat -ano | findstr $tcpPort")
	addCmdWorker("port_check=>netstat -ano | findstr $tcpPort")
}

@baro.userProc(&s) {
	// tcpPort close
	switch(funcParam('type')) {
	case findBindPort:	
		funcParam('tcpPort,checkCallback').inject(port,callback)
		print(">> findBindPort start ======= PORT:$port")
		while(s.valid()) {		
			s.findPos('TCP')
			not(s.ch()) break;
			line = s.findPos("\n")			
			line.findPos(":$port")
			c=line.ch(0)
			print(">> line:$line [$c]")
			not(typeof(c,'num')) {
				line.findPos('LISTENING')
				pid=line.trim()
				if(pid) {
					if(typeof(callback,'func')) {
						callback(pid)
					} else if(callback) {
						@baro.cmdRun(this, "taskkill /PID $pid /F" )
					}
					return;
				}
			}
		}
		print(">> findBindPort end $port 포트를 찾을수 없습니다")
		if(typeof(callback,'func')) callback()
	default:
	}
	// baro userProc end
}
