<func>
	initPage() {
		include("app/javaRun.js");
		conf=object('ndid.conf');
		conf.screenCount=System.info("screenCount");
		conf.statusCheck=0;  
		while(idx=0, idx<conf.screenCount, idx++) {
			Cf.sourceApply(#[
				<widgets base="ndid">
					<page id="did_${idx}" margin="0">
						<context id="c">
					</page>
					<label id="loading_${idx}"></label> 
				</widgets>
			]); 
			rect=System.info("screenRect", idx);
			page=page("ndid:did_${idx}");
			if(page) {
				page.move(rect);
				page.open();
				didDrawFunction(page.get("c"), idx);
				/* 로딩 GIF 실행 
				if(idx==0) {
					label=object("label.ndid:loading_0");
					rcLoading=rect.center(480,270);
					widgetSub(page, label, rcLoading);
					label.playGif('data/images/etc/loading00.gif');
				}
				*/
			}
			not(conf.mainPage) {
				conf.mainPage=@app.initMainPage(page);
			}
		}
		@app.initDid();
	}
	didDrawFunction(context, index ) {
		// createParticleRain(context, 150);
		context.interval(80);
		context.set("displayIndex", index);
		context[
			onDraw(dc, rc) {
				dc.fill('#fff'); 
				idx=this.displayIndex;
				rcIcon=rc.center(800,600);
				idx=this[progressIndex++];
				if(idx>99) {
					idx=this[progressIndex=0];
				}
				dc.image(rcIcon,"loading1/loading1-${idx}.png");
				x=0, y=rcIcon.bottom()+32,w=rc.width();
				rcMessage=rc(x,y,w,350)
				dc.html(rcMessage, this.getDidMessage());
			}
			getDidMessage() {
				web=Baro.web('ndid'); 
				idx=this.get("displayIndex");
				errStyle='font-size:20px;color:#fa3322;font-weight:bold;';
				message=object('ndid.conf').get('downloadStatus');
				not(message) message=web.procStatus;
				return format(#[
					<table width="100%" height="100%" style="font-size:26px;color:#889;">
						<tr>
							<td align="center">
								화면번호 #{idx} 
								#{message,[<br><font style="font-size:16px;color:#aaa;">@</font>]}
							</td>
						</tr>
						<tr>
							<td align="center">
								#{apiError,[<br><font style="${errStyle}">@</font>]}
							</td>
						</tr>
					</table>
				], web);
			}
		];
	}	
	@app.initDid() {
		if(isFile('data/modify-info.json')) {
			fileDelete('data/modify-info.json');
		}
		web=Baro.web('ndid'); 
		path=System.path()
		driver=path.value(0,2);
		db=Baro.db('nrjlink');
		db.open("$driver/NRJ/Noorija/NrjLink/contents/database/nrjlink.db");
		not(db.open()) {
			return @app.apiError(web, "링크 DB오픈 실패");
		}
		conf=object('ndid.conf');
		db.fetch("select * from tb_affl_shop_info", conf);
		print(">> init did config => $conf");
		
		conf.inject(httpUrl,equipNo,afflShopNo ); 
		not(afflShopNo && equipNo) return @app.apiError(web, "가맹점 또는 장비정보가 없습니다");
		@app.apiConnectInEq(web)
	}
	@app.initMainPage(page) { 
		page.timer("idle",1000);
		page[
			onInit() {
				this.action([
				  {id:app.adminTool, text:DID 관리툴 , icon: vicon.package_green},
				  {id:app.didInfo, text:DID 정보 , icon: vicon.page_code},
				  {id:app.exit, text:프로그램 종료, icon: ficon.cross}
				]);
				tray=this.tray();
				tray.contextMenu([app.adminTool,app.didInfo,-,app.exit]);
				tray.show();
				tray.icon("vicon.package_green");
			}
			onAction(action) {
				print("xxxxxxxxxxx $action");
				switch(action.id) {
				case 'app.adminTool':
					System.run('http://localhost');
				case 'app.didInfo':
					include("dev/sourceEditor.js");
					@dev.sourceEditor().open();
				case 'app.exit':
					@app.closeDid();
				default:
				}
			}
			onTimer() {
				conf=object('ndid.conf');
				if(conf.closeTick) {
					dist=System.tick() - conf.closeTick;
					if(dist>5000) {
						if( conf.statusCheck < 3 ) {
							@app.closeDid();
						}
					}
					return;
				}
				not(conf.statusCheck) return;
				switch(conf.statusCheck ) {
				case 1:
					if(@app.downloadEndCheck()) {
						@app.startDid();
						@app.setCheckTick(2, "donwloadEnd");
					}
				case 2:
					dist=System.tick() - conf.checkTick;
					if(dist>5000) {
						@app.setCheckTick(3, "javaStart"); 
					}
				case 3:
					dist=System.tick() - conf.checkTick;
					if(dist>5000) {
						// DID로딩 화면을 닫아준다
						while(idx=0, idx<conf.screenCount, idx++) {
							page=page("ndid:did_${idx}");
							page.hide();
						}
						label=object("label.ndid:loading_0")
						if(label) label.hide();
						conf.javaCheckCount=0;
						@app.setCheckTick(4,"didRun");
					}
				case 4:
					dist=System.tick() - conf.checkTick;
					// 10초에 한번씩 실행여부 체크
					if(dist>10000) {
						@app.setCheckTick();
						@app.didRunCheck();
						not(conf.javaProcessId) {
							if(conf.javaCheckCount>5 ) {
								@app.setCheckTick(5,"javaRestart");
							} else {
								conf.javaCheckCount++;
							}
						}
					}
				case 5:
					// 자바 미실행시 다시실행
					dist=System.tick() - conf.checkTick;
					if(dist>25000) {
						conf.javaCheckCount=0;
						@app.javaStart();
						@app.setCheckTick(4);
					} 
				default:
				}
			}
		] return page;
	}
	@app.setCheckTick(status, msg) {
		conf=object('ndid.conf');
		conf.checkTick=System.tick();
		if(typeof(status,'num') && status ) {
			conf.statusCheck=status;
		}
		if(msg) conf.pageStatus=msg;
	}
	@app.startDid() {
		conf=object('ndid.conf');
		server=Baro.udp("ndid");
		server.start(33351, func(data) { @app.udpProc(data) });
		@app.javaRun();
	}
	@app.udpProc(&s) {
		print(">> udpProc start == $s");
		
		web=Baro.web('ndid'); 
		conf=object('ndid.conf');
		conf.inject(httpUrl,equipNo,afflShopNo ); 
		udpCheck=false;
		if(s.start('$#',true)) {
			line=s.findPos('#$');
			conf[udpCommand]=line.findPos(':').trim();
			afflShop=line.findPos(':').trim();
			equip=line.findPos(':').trim();
			conf[udpInfo]=line.trim();
			udpCheck = afflShop.eq(afflShopNo) && equip.eq(equipNo);
		}
		not(udpCheck) {
			return print("UDP 체크오류 가맹점 또는 장비번호 불일치 ==> ", udpCheck, afflShopNo, equipNo, afflShop, equip)
		}  
		cmd=conf[udpCommand];
		if(cmd.eq('30','90','91')) {
			message=format('@@> {"type":"udpCommand", "command":#{0}, "udpCommand":#{1} }', Cf.jsValue(s), Cf.jsValue(cmd));
			logWriter('didCommand').appendLog(message);
			/*
			udpCommand 
				30: 긴급명령 해제
				90: 시스템 정검시작
				91: 시스템 정검종료
			*/
			return;
		}
		header=web.addNode("@header")
		node=web.addNode("@connectInEq");
		header.set("Accept", "*/*");
		header.set("Connection", "keep-alive");
		header.set("Content-Type", "application/json");
		header.set("X-AUTH-TOKEN", node.connKey);
		web.set("notiData","");
		web[data]=#[{"afflShopId":"${afflShopNo}","equipNo":"${equipNo}"}];
		print("긴급알림 메시지 시작", web.data);
		web.call("${httpUrl}/api/link/noti-emer", "POST", func(type, data) {
			if(type.eq('finish')) return @app.apiNotifyResult(this);
			if(type.eq('error')) return @app.apiError(this, data); 
			if(type.eq('read')) {
				this.appendText("notiData",data);
			}
		});
	}
	 
	@app.apiNotifyResult(web) {
		conf=object('ndid.conf');
		node=object("data.notiInfo").removeAll(true);
		node.parseJson(web.notiData);
		node.udpCommand=conf.udpCommand;
		node.udpInfo=conf.udpInfo;
		if(node.downUrl) {
			node.displayFileNm=node.outImgFileNm;
			@app.downloadPush(node);
			@app.downloadStart();
		} else {
			result=@json.nodeStr(node);
			message=format('@@> {"type":"notifyMessage", "viewTypeCd":#{0}, "alarmNo":#{1}, "text":#{2}, "moveCd":#{3}, "textColor":#{4}, "bgColor":#{5}, "textSize":#{6}, "outLocCd":#{7}, "udpCommand":#{8} }', 
				Cf.jsValue(node.viewTypeCd),
				Cf.jsValue(node.alarmNo),
				Cf.jsValue(node.outText),
				Cf.jsValue(node.moveCd),
				Cf.jsValue(node.textColor),
				Cf.jsValue(node.bgColor),
				Cf.jsValue(node.textSize),
				Cf.jsValue(node.outLocCd),
				Cf.jsValue(node.udpCommand)
				
			);
			logWriter('didCommand').appendLog(message);
		}
	}
	@app.apiConnectInEq(web) { 		
		object('ndid.conf').inject(httpUrl,equipNo,afflShopNo ); 
		web.set("procStatus", "$equipNo 장비 API 호출시작");
		macInfo=System.networkInfo().get(3);
		header=web.addNode("@header")
		header.set("Accept", "*/*");
		header.set("Connection", "keep-alive");
		header.set("Content-Type", "application/json");
		web[apiError]='';
		web[data]=#[{"afflShopId":"${afflShopNo}","equipNo":"${equipNo}","macInfo":"${macInfo}"}];
		web.call("${httpUrl}/api/member/connect-in-eq", "POST", func(type, data) {
			switch(type) {
			case read:		this.appendText("readData",data); 
			case finish:	@app.apiDidResult(this); 
			case error:		@app.apiError(this, data); 
			} 
		});
	}
	@app.apiDidResult(web) { 
		if(web.apiError ) return;
		object('ndid.conf').inject(httpUrl,equipNo,afflShopNo ); 
		node=web.addNode("@connectInEq");
		node.parseJson(web.readData);
				
		header=web.addNode("@header");
		header.set("Accept", "*/*");
		header.set("Connection", "keep-alive");
		header.set("Content-Type", "application/json");
		header.set("X-AUTH-TOKEN", node.connKey);
		web[readData]='';
		web[data]=#[{"afflShopId":"${afflShopNo}","equipNo":"${equipNo}","jobCd":"1"}];
		web.call("${httpUrl}/api/link/v2-init-did-info", "POST", func(type, data) {
			switch(type) {
			case read:		this.appendText("readData",data); 
			case finish:	@app.downloadInit(this); 
			case error:		@app.apiError(this, data); 
			} 
		})
	}
	@app.apiDidAlarmTransChk() {
		web=Baro.web('ndid');
		object('ndid.conf').inject(httpUrl,equipNo,afflShopNo ); 
		web.set("procStatus", "$equipNo DID 데이타 변경시 알람 체크 여부 확인");
		node=web.addNode("@connectInEq");
		header=web.addNode("@header")
		header.set("Accept", "*/*");
		header.set("Connection", "keep-alive");
		header.set("Content-Type", "application/json");
		header.set("X-AUTH-TOKEN", node.connKey);
		web[apiError]='';
		web[data]=#[{"afflShopId":"${afflShopNo}","equipNo":"${equipNo}","jobCd":"1"}];
		print("프로그램 시작", web.data);
		web.call("${httpUrl}/api/link/did-alarm-trans-chk", "POST", func(type, data) {
			switch(type) {
			case read:		this.appendText("readData",data); 
			case finish:	print("apiDidAlarmTransChk finished ", this.readData);
			case error:		@app.apiError(this, data); 
			} 
		});
	}

	@app.apiError(web, data) {
		not(web) {
			web=Baro.web('ndid');
			return web[apiError];
		}
		conf=object('ndid.conf');
		message="${web.url} 호출오류 : $data";
		log=format('@@> {"type":"apiError", "message":#{0} }', Cf.jsValue(message));
		logWriter('didCommand').appendLog(log);
		web[apiError]=message;
		conf.closeTick=System.tick();
	}
</func>


<func note="컨텐츠 다운로드">
	@app.initDidInfo(&s) {
		data=object("data.didInfo").removeAll(true);
		save=true;
		not(s) {
			save=false;
			s=fileRead('data/did-info.json');
		}
		data.parseJson(s);
		if(save) {
			fileWrite("data/did-info.json", s);
		}
		locationId=@app.locationId(s);
		print("init did info [locationId==$locationId]");
		return data;
	}
	@app.downloadInit(web) { 
		web.set("procStatus", "컨텐츠 다운로드 시작");
		data=@app.initDidInfo(web.ref(readData));
		list=data.get("contentsFileList");
		@app.downloadAdd(list);
		if(@app.downloadEndCheck()) {
			return;
		}
		@app.downloadStart();
	}
	@app.downloadAdd(list) {
		while(cur, list, idx ) {
			n=idx%5;
			web=Baro.web("down-$n");
			web.addArray("@downList").add(cur);
		}
		conf=object('ndid.conf');
		conf.statusCheck=1;
	}
	@app.downloadEndCheck() {
		while(n=0, n<5,  n++ ) {
			web=Baro.web("down-$n")
			if(web.isRun()) return false;
			arr=web.get("@downList");
			not(typeof(arr,'array')) continue;
			if(arr.size()) return false;
		}
		object('ndid.conf').set('downloadStatus','');
		return true;
	}
	@app.downloadPush(cur) {
		conf=object('ndid.conf');
		idx=conf[downIndex++];
		n=idx%5;
		web=Baro.web("down-$n");
		web.addArray("@downList").add(cur);
	} 
	@app.downloadStart() {
		while(n=0, n<5,  n++ ) {
			web=Baro.web("down-$n")
			arr=web.get("@downList");
			if(arr.size()) { 
				@app.downloadFile(web);
			}
		}
	}
	@app.downloadProcess(type,data) {
		print("process type==$type");
		node=this.currentNode;
		if(type.eq('error')) {
			web=Baro.web('ndid'); 
			@app.apiError(web, "다운로드오류: ${node.displayFileNm}");
			return;
		}
		if(type.eq('finish')) {
			object('ndid.conf').set('downloadStatus',"완료 : ${node.displayFileNm}");
			if(node.viewTypeCd ) @app.notifyEndCheck(node);			
			
			@app.downloadFile(this);
		}
	}
	@app.downloadFile(web) {
		node=web.get("@downList").pop();
		not(node) {			
			return;
		}		
		node.inject(downUrl, displayFileNm) not(displayFileNm) return @app.downloadFile(web);
		ext=rightVal(displayFileNm).lower();
		if(ext.eq('png','jpg','jpeg','gif','bmp')) {
			node.fullPath="data/images/$displayFileNm";	
		} else {
			node.fullPath="data/movies/$displayFileNm";
		} 
		// 다운로드 파일이 있다면 무시한다.
		if( isFile(node.fullPath) ) {
			if(node.viewTypeCd ) @app.notifyEndCheck(node);
			return @app.downloadFile(web);
		}
		object('ndid.conf').set('downloadStatus', "다운로드시작 ${node.displayFileNm}");
		web.currentNode=node;
		web.download(downUrl, node.fullPath, "GET", @app.downloadProcess );
	}
	@app.closeDid() {
		@app.didKillStart(true);
	}
	@app.notifyEndCheck(node) {
		if(node.viewTypeCd ) {
			message=format('@@> {"type":"notifyMessage", "viewTypeCd":#{0}, "name":#{1}, "alarmNo":#{2} }', 
				Cf.jsValue(node.viewTypeCd),
				Cf.jsValue(node.outImgFileNm),
				Cf.jsValue(node.alarmNo)
			);
			logWriter('didCommand').appendLog(message);
		}
	}
	
</func>


<func>
	createParticleRain(canvas, count) {
		obj=Cf.getObject("class", "ParticleRain");
		canvas.size().inject(width, height);
		not(obj) {
			obj=Cf.getObject("class", "ParticleRain", true);
			obj[
				initClass() {
					this.member(canvas, canvas);
					this.canvasWidth=width;
					this.canvasHeight=height;
					this.x=rand(0,this.canvasWidth);
					this.y=0;
					this.color=randomColor();
				}
				update() {
					dy=rand(10, 30);
					this[y+=dy] if(this[y>canvasHeight]) {
						this.x=rand(0,this.canvasWidth);
						this.y=0;
					}
				}
				draw(dc) {
					rc=rc(this.x, this.y, 18, 18);
					dc.arcPath(rc, 0, 360, true, this.color );
				}
			];
		}
		idx=Cf.rootNode().incrNum('particles');
		root=object("obj.particle$idx");
		while(count) {
			node=root.addNode();
			node.class(obj);
			node.initClass();
		}
		canvas.particles=root;
		return root;
	}
	
	@app.test() {
		db=Baro.db('nrjlink');
		not(db.open('contents/database/nrjlink.db')) {
			db.open('C:/NRJ/Noorija/NrjLink/contents/database/nrjlink.db');
		}
		server=Baro.udp("ndid");
		server.start(33351, func(data) { @app.udpProc(data) });
		
		conf=object('ndid.conf');
		db.fetch("select * from tb_affl_shop_info", conf);
		print(">> init did config => $conf");
		web=Baro.web('ndid'); 
		conf.inject(httpUrl,equipNo,afflShopNo ); 
		not(afflShopNo && equipNo) return @app.apiError(web, "가맹점 또는 장비정보가 없습니다");
				object('ndid.conf').inject(httpUrl,equipNo,afflShopNo ); 
		web.set("procStatus", "$equipNo 장비 API 호출시작");		
		macInfo=System.networkInfo().get(3);
		header=web.addNode("@header")
		header.set("Accept", "*/*");
		header.set("Connection", "keep-alive");
		header.set("Content-Type", "application/json");
		web[apiError]='';
		web[readData]='';
		web[data]=#[{"afflShopId":"${afflShopNo}","equipNo":"${equipNo}","macInfo":"${macInfo}"}];
		web.call("${httpUrl}/api/member/connect-in-eq", "POST", func(type, data) {
			switch(type) {
			case read:		this.appendText("readData",data); 
			case finish:	
				node=this.addNode("@connectInEq");
				node.parseJson(this.readData);
			case error:		@app.apiError(this, data); 
			} 
		});		
	}
</func>