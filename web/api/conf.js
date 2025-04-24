<api>
	test(req, param, &uri) {
		System.sleep(10000)
		param.message='sleep 10s test'
		return param
	}
	confVal(req, param, &uri) {
		name=uri.findPos('/').trim()
		db=Baro.db('config')
		db.fetchAll("select data from conf_info where grp='$name'", param, true)
		return param;
	}
	confList(req, param, &uri) {
		name=uri.findPos('/').trim()
		not(name) name='sql'
		param.name=name
		sql=V[
			select grp, cd 
			from conf_info 
			where 1=1 #[name? and grp=#{name}]
			order by grp
		]
		db=Baro.db('config')
		db.fetchAll(sql, param, true)
		return param;
	}
	confGroupList(req, param, &uri) {
		db=Baro.db('config') 
		sql=#[
			select grp, cnt from (
				select grp, count(1) as cnt
				from conf_info 
				where 1=1
				group by grp
			) order by cnt
		]
		db.fetchAll(sql, param, true)
		return param;
	}
	confGroupRemove(req, param, &uri) {
		grp=uri.findPos('/').trim()
		db=Baro.db('config') 
		db.exec("delete from conf_info where 1=1 and grp='${grp}'", param, true)
		return param;
	}
	refresh(req, param, &uri) {
		@router.makeUrlMap();
		return param;
	}
	saveTestSrc(req, param, &uri) {
		db=Baro.db('config')
		param.inject(saveName, saveDesc, data)
		param.data=''
		cnt=db.value("select count(1) from conf_info where grp='test#src' and cd='$saveName'")
		conf("test#src.$saveName", data.escape(), true)
		if(cnt==0 ) 
			param.message="$saveName 테스트 소스가 등록 되었습니다"
		else 
			param.message="$saveName 테스트 소스가 수정 되었습니다"
		if(saveDesc) conf("test#desc.$saveName", saveDesc, true)
		return param
	}
	saveNameCount(req, param, &uri) {
		db=Baro.db('config')
		name=uri.trim()
		param.count=db.value("select count(1) from conf_info where grp='test#src' and cd='$name'")
		return param;
	}
	readTestSrc(req, param, &uri ) {
		db=Baro.db('config')
		name=uri.trim()
		cnt=db.value("select count(1) from conf_info where grp='test#src' and cd like '%$name%'")
		if(cnt==0 ) {
			return "$name 으로 검색되는 테스트 소스가 없습니다";
		}
		if(cnt==1 ) {
			cnt=db.value("select count(1) from conf_info where grp='test#src' and cd ='$name'")
			if(cnt==1 ) {
				node=db.fetch("select data from conf_info where grp='test#src' and cd='$name'")
				desc=conf("test#desc.$name");
				return "[${desc}]${node.data}";
			}
		}
		db.fetchAll("select grp, cd, regdt, tm from conf_info where grp='test#src' and cd like '%$name%'", param)
		ss="$name 으로 등록된 테스트 소스";
		while(cur, param) {
			cur.inject(grp, cd, regdt, tm)
			modifyDate=System.date(tm,'yyyy-MM-dd hh:mm')
			regDate=regdt.value(0,16)
			desc=conf("test#desc.$cd");
			ss.add("\n- $cd $desc 등록일:$regDate 수정일:$modifyDate");
		}
		print(">>>", cnt, ss)
		return ss;
	}
	routePage(req, param, &uri) {
		// ex) localhost/api/conf/routePage
		db=Baro.db('config')
		aa=uri.findPos('/').trim()
		if( aa=='save' ) {
			savePath=uri.findPos('/').trim()
			if(savePath) {
				savePath=Cf.val(System.path(),'/',savePath)
			} else {
				savePath=conf('path.sourceSavePath')
				not(savePath) {
					savePath=System.path()
					savePath.add('/data/temp')
				}
			}
			cnt=0
			while(cur, db.fetchAll("select grp, cd, data from conf_info where grp like 'urlMap#%'")) {
				if(cur.cd=='/') cur.cd='/main'
				fileWrite("${savePath}${cur.cd}.html", cur.data);
				cnt++;
			}
			param.message = "$savePath 폴더에 $cnt 건 저장했습니다"
		} 
		if(aa=='list') {
			db.fetchAll("select cd as page_code, tm, regdt from conf_info where grp like 'urlMap#%' order by regdt desc", param)
			return param;
		}
		// 소스 문자열 리턴
		group=aa
		bb=uri.findPos('/').trim()
		if(bb) {
			cd="/$bb"
		} else {
			if(group) {
				if(group=='main') cd='/' else cd="/$group"
			} else {
				group='main'
				cd='/'
			}
		}
		node=db.fetch("select data from conf_info where grp='urlMap#$group' and cd='$cd'")
		if(node) return node.data
		param.error = "GROUP:$group CODE:$cd 정보를 불러올수 없습니다"
		return param;
	}
	subPage(req, param, &uri) {
		// ex) localhost/api/conf/subPage/[url,list,save,delete]
		db=Baro.db('config')
		aa=uri.findPos('/').trim()
		if( aa=='save' ) {
			savePath=
			if(savePath) {
				savePath=Cf.val(System.path(),'/',savePath)
			} else {
				savePath=conf('path.sourceSavePath')
				not(savePath) {
					savePath=System.path()
					savePath.add('/data/temp')
				}
			}
			cnt=0
			while(cur, db.fetchAll("select grp, cd, data from conf_info where grp = 'src#pages'")) {
				if(cur.cd=='/') cur.cd='/main'
				fileWrite("${savePath}${cur.cd}.html", cur.data);
				cnt++;
			}
			param.message = "$savePath 폴더에 $cnt 건 저장했습니다"
			return param;
		} 
		if(aa=='list') {
			// ex) http://localhost/api/conf/subPage/list
			db.fetchAll("select cd as page_code, tm, regdt from conf_info where grp='src#pages' order by regdt desc", param)
			return param;
		}
		if(aa=='delete') {
			db.fetchAll("select cd as page_code, tm, regdt from conf_info where grp='src#pages' order by regdt desc", param)
			return param;
		}
		// 소스 문자열 리턴
		cd=aa;
		bb=uri.trim()
		if(bb) cd.add('/',bb)
		node=db.fetch("select data from conf_info where grp='src#pages' and cd='$cd'")
		if(node) {
			if(node.data) return node.data
			node.error ="URI $cd 페이지 소스 내용이 없습니다"
		}
		param.error = "URI $cd 페이지 정보를 찾을 수 없습니다"
		return param;
	}
	data(req, param, &uri) {
		db=Baro.db('config')
		a=uri.findPos('/').trim()
		b=uri.trim()
		node=db.fetch("select data from conf_info where grp='$a' and cd='$b'")
		print("node=>$node", a,b)
		return when(node,node.data,"nodata")
	}
	makePathTree(req, param, &uri) {
		type=uri.findPos('/').trim()
		switch(type) {
		case js: 
			path=System.path();
			path.add("/web/js");
			return make(path, object('path.js').removeAll());
		default:
		}		
		make=func(path, root, baseLen ) {
			not(baseLen) baseLen=path.size()+1;
			fo=Baro.file()
			fo.var(sort,'type,name')
			fo.list(path, func(info) {
				while(info.next()) {
					info.inject(type, fullPath, name, modifyDt, createDt)
					if(type=='folder') {
						relative=fullPath.value(baseLen)
						print(type, name, relative)
						cur=root.addNode().with(type, name, relative, modifyDt, createDt)
						make(fullPath, cur, baseLen)
					}
				}
			})
			return root;
		};
	}
	subPageSave(req, param, &uri) {
		param.inject(apiUri, src)
		apiUri.ref()
		print("xxxxxxxxxxx", param, apiUri )
		if(apiUri.ch('/')) apiUri.incr();
		a=apiUri.findPos('/').trim();
		b=apiUri.trim() 
		if(service && subPageCode ) {
			conf("src#${a}.${b}", src.escape(), true);
		}
		param.src=''
		return param
	}
	// ace 경로 xui.define 설정
	aceReplace() {
		fo=Baro.file()
		parse=func(&s) {
			rst=''
			while(s.valid(), n) {
				left=s.findPos('ace.define')
				rst.add(left)
				not(s.ch()) break
				rst.add('xui.define')
			}
			return rst
		};
		parseRequire=func(&s) {
			rst=''
			while(s.valid(), n) {
				left=s.findPos('ace.require')
				rst.add(left)
				not(s.ch()) break
				rst.add('xui.require')
			}
			return rst
		};
		fo.list('C:\BARO\web\js\ace\worker', func(info) {
			while(info.next() ) {
				info.inject(type, fullPath)
				if(type=='file') {
					src=parse(fileRead(fullPath))
					if(src.find('ace.require')) {
						src=parseRequire(src)
					}
					fileWrite(fullPath, src)
				}
			}
		})
	}
	
	driveList(req,param) {
		param.driveList=System.driveList()
		return param
	}
	folderChild(req,param,&uri) {
		drive=uri.findPos('/').trim()
		parentName=uri.trim()
		path="$drive:/$parentName";
		print("xxxxx", drive, parentName)
		fo=Baro.file('folders')
		fo.var(filter,'folders')
		fo.var(sort, 'name')
		fo.list(path,func(info) {
			while(info.next()) {
				info.inject(type, name, fullPath, createDt)
				if(type=='file') continue
				if(name.eq('.','..')) continue
				if(chkRoot) {
					if(name.start('Program Files')) continue;
					if(name.eq('Windows')) continue;
				}
				relative="$parentName/$name"
				cur=param.addNode()
				cur.with(type, name, relative, createDt)
				cur.childFolderCount=@fs.getChildCount(fullPath)
				parse(fullPath, cur, depth, pathLen)
			}
		})
		return param;
	}
	
	folderList(req,param,&uri) {
		Cf.include('data/libs/common/fs.js')
		maxDepth=uri.findPos('/').trim()
		drive=uri.findPos('/').trim()
		relative=uri.trim()
		path="$drive:/$relative";
		fo=Baro.file('folders')
		fo.var(filter,'folders')
		fo.var(sort, 'name')
		if( maxDepth>4 ) maxDepth=4;
		return parse(fo, path, param, maxDepth)
		parse = func(fo, path, parent, depth, pathLen) {
			if(depth<=0 ) return parent;
			chkRoot=false;
			not(pathLen ) {
				rs=relative.size()
				pathLen=path.length()
				if(rs>0) pathLen-=rs;
				print("xxxx folderList xxx", rs, pathLen)
				chkRoot=true
			}
			depth-=1;
			fo.list(path,func(info) {
				while(info.next()) {
					info.inject(type, name, fullPath, createDt)
					if(type=='file') continue
					if(name.eq('.','..')) continue
					if(chkRoot) {
						if(name.start('Program Files')) continue;
						if(name.eq('Windows')) continue;
					}
					relative=fullPath.value(pathLen)
					cur=parent.addNode()
					cur.with(type, name, relative, createDt)
					not(depth) cur.childFolderCount=@fs.getChildCount(fullPath)
					parse(fo, fullPath, cur, depth, pathLen)
				}
			})
		};
	}
	// 위젯템플릿 저장
	applyXTemplate(req, param, &uri) {
		param.inject(src, tag)		
		conf("xtemplate.${tag}", src.escape(), true)
		param.src='ok'
		return param;
	}
	// 위젯템플릿 처리
	parseXTemplate(req, param, &uri) {
		source=param.src.escape()
		if(param.name) {
			conf("test#origin.$name", source, true)
		}
		return @template.parse(source, '', param)
	}

	// 파일 업로드 처리
	uploadFile(req,param,&uri) {
		param.inject(uploadKey, uploadPath, uploadFileName, uploadLast);
		if(uploadLast=='Y') {
			root=webRoot()
			savePath="$root/upload";
			if(uploadPath) {
				not(uploadPath.ch('/')) savePath.add('/');
				savePath.add(uploadPath);
			}
			isFolder(savePath, true)
			@api.uploadFileCopy(uploadKey, savePath, uploadFileName);
		}
		return param;
	}
	// cmd 실행 결과정보
	cmdRun(req, param, &uri ) {
		command=uri.trim()
		logClass('cmd').timeout()
		cmd=Baro.process('cmd')
		args=cmd.args
		not(cmd.is()) {
			Cf.postEvent('cmdStart', cmd)
			param.message="cmd 프로세스를 다시 실행하였습니다. 다시 요청 하세요"
			return param;
		}
		not(typeof(args,'node')) {
			param.message="cmd 프로세스를 매개변수 객체오류"
			return param;
		}
		args.with(req,param,command)
		param.var(checkSend, true)
		Cf.postEvent('cmdCommand', cmd)
		return param;
	}
	// jshint 실행 결과정보 
	jshintUse(req, param, &uri ) {
		src=param.src.escape()
		cmd=Baro.process("cmd");
		commandArray=cmd.addArray("commandArray").reuse()
		args=cmd.args;
		args.type="jshint";
		parse(src)
		args.with(req,param,command,commandArray)
		param.var(checkSend, true)
		return param;
		
		parse=func(&s) {
			n=0, idx=1, cntTot=0;
			while(s.valid()) {
				left=s.findPos('<script',0,1)
				cntA=lineCount(left)
				cntTot+=cntA;
				not(s.ch()) break;
				ss=s.match('<script', '</script>')
				if(typeof(ss,'bool')) break;
				cntB=lineCount(ss)
				ss.findPos('>')
				name="${idx}_${cntTot}"
				fileWrite("data/temp/${name}.js", ss)
				cntTot+=cntB;
				commandArray.add("jshint ${name}.js")
				idx++;
			}
		};
		lineCount=func(&s) {
			n=0
			while(s.valid()) {
				s.findPos("\n")
				n++;
			}
			return n;
		};
		
	}
	srcList(req, param, &uri ) {
		db=Baro.db('config')
		db.fetchAll("select grp, cd, tm, regdt from conf_info where grp='test#src' order by regdt desc", param)
		while(cur, param) {
			moddt=System.date(cur.tm,'yyyy-MM-dd hh:mm')
			note=conf("test#desc.${cur.cd}")
			cur.with(moddt, note)
		}
		return param
	}
	testSource(req, param, &uri ) {
		name=uri.trim();
		src=conf("test#src.$name")
		not(src) src=conf("test#origin.$name")
		not(src) src="$cd 페이지코드 조회 오류"
		return src
	} 
	
	// vicon 정보
	viconInfo(req, param) {
		arr=Cf.rootNode().addArray(vicons);
		not(arr.size()) parse(fileRead('web/css/vicon.css'));
		parse=func(&s) { 
			while(s.valid()) {
				s.findPos('.xui-icon.');
				name=s.findPos('{').trim();
				not(name) break;
				arr.add(name);
			}
		};
		param.type='viconList';
		param.viconInfo=arr;
		return param;
	}

	// fa 아이콘 정보 출력
	iconInfo(req, param) {	
		arr=Cf.rootNode().addArray(fontawesomeIcons);
		if(arr.size()) arr=null;
		parse(fileRead('web/css/fontawesome.css'));
		parse=func(&s) {
			s.findPos('.fa-inverse');
			while(s.valid()) {
				s.findPos('.fa-');
				name=s.findPos('::').trim();
				if(name.find("\n")) continue;
				s.findPos('{',1,1);
				body=s.match();
				body.findPos('"\');
				value=body.find('"').trim();
				icon="fa-$name";
				if(arr) arr.add(icon);
				param.addNode().with(icon,name, value);
			}
		};
		return param;
	}
	// 이미지 리사이즈
	resizeImage(req, param, &vars) {
		include('app/etc.js');
		node=object('obj.images').removeAll(true);
		fo=Baro.file('convert');
		sub=vars.trim();
		path=System.path();
		targetPath="$path/data/images";
		if(sub) targetPath.add("/$sub");
		print("target path == $targetPath", node);
		fo.list(targetPath, func(info) {
			while(info.next()) {
				info.inject(type, name, ext);
				ext=ext.lower();
				if(ext.eq('jpg','jpeg','png','bmp','gif')) {
					node.addNode().with(targetPath, name, useConvert:true);
				}
			}
		});
		proc=objectFunc(Baro.process('magick'), V[	
			resize() {
				proc=this;
				if(proc.run()) proc.stop();
				list=proc.workList;
				cur=list.get(0) not(cur) return;
				cur.useConvert=true;
				cur.inject(targetPath, name);
				path=System.path();
				flag=2;
				proc.run("magick '${targetPath}/${name}' -resize 100 '${targetPath}/thumb-${name}'", 
					this.resizeProc, flag, "$path/programs/magick" 
				);
				return cur;
			};
			resizeProc(type,data) {
				proc=this;
				list=proc.workList;
				print("xxxx $type>> $data", list);
				if(type=='finish') { 
					cur=list.get(0);
					if(cur && cur.useConvert) {
						list.pop();
						print("finsh ========== $cur", list);
						proc.resize();
					} 
				}
			}
		], true);
		proc.workList=tempArray(node);
		not(proc.resize(list)) {
			node.error='변환할 이미지가 없습니다';
		} 
		return node;
	}
	// 비디오 쎔네일 만들기
	videoThumb(req, param, &vars) {
		include('app/etc.js');
		node=object('obj.videoThumb');
		fo=Baro.file('mediaConvert');
		sub=vars.trim();
		path=System.path();
		targetPath="$path/data/movies";
		if(sub) targetPath.add("/$sub");
		fo.list(targetPath, func(info) {
			while(info.next()) {
				info.inject(type, name, ext);
				ext=ext.lower();
				if(ext.eq('mp4')) {
					node.addNode().with(targetPath, name, useConvert:true);
				}
			}
		});
		Cf.postEvent('videoThumb', node);
		return node;
	}
	
</api>
