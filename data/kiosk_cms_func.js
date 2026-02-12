function.setGlobalValue(code, val) {
	cf = get('global');
	cf[$code] = val;
	return val;
}
function.include(code, overwrite) {
	not( Cf[includeNode] ) Cf[includeNode]= {};
	root = Cf[includeNode];
	root.removeAll();
	db = instance('pages.model');
	
	if( code.find('.') ) {
		groupName=code.find('.').trim();
		className=code.find('.').right().trim();
		if( className.eq('*') ) {
			className=null;
		}
		
		if( className ) {		
			file=instance('my.file');
			fileName="data/classes/${groupName}/${className}.class";
			if( file.isFile(fileName) ) {
				root.put( className, groupName);
				tm=file.modifyDate(fileName);
				modifyTm=db.value("select tm from class_mst where class_grp=#{groupName} and class_nm=#{className}", root);		
				
				not( tm.eq(modifyTm) ) {
					overwrite=true;
					if( modifyTm ) {
						db.exec("update class_mst set tm='$tm' where class_grp=#{groupName} and class_nm=#{className}", root);
					} else {
						db.exec("insert into class_mst(class_grp, class_nm, status, useyn, tm) values(#{groupName}, #{className}, '1', 'N', '$tm')", root );
					}
				}			
				if( root[$code], not(overwrite) ) {
					return true;
				}
				root[$code]=true;
				src=file.readAll(fileName);
				arr=class('util').arr();
				saveClassFile( db, src.ref(), groupName, arr, tm);
				print("load classes arr: $arr");
				while( classNm, arr ) {
					classCode="${groupName}.${classNm}";
					Cf.reloadClass(classCode);
					loadClassByDb(classCode);
				}			
				print("modifyTm=$modifyTm == $tm");
			} else {
				cls=Cf.class(code);
				if( cls, not(overwrite) ) return true;
				loadClassByDb(code);		
			}		
		} else {
			root.put( groupName);
			db.fetchAll("select class_nm from class_info where class_grp=#{groupName} group by class_nm", root );
			not( root.childCount() ) return false;
			while( node, root ) {
				classCode="${groupName}.${node[class_nm]}";
				cls=Cf.class(classCode);
				if( cls, not(overwrite) ) continue;
				Cf.reloadClass(classCode);
				loadClassByDb(classCode);
			} 
		}	
	} else {
		root[cmsCode]=code;
		db.fetchAll("select funcName, funcParam, funcData, type from cmsFunc where useyn='Y' and cmsCode=#{cmsCode} ", root); 
		while( cur, root ) {
			static=when( cur[type].eq('S'), true);
			Cf.func("${cur[funcName]}($cur[funcParam]) {$cur[funcData]}", static);
		}	
	}
	return true;
}

function.pageReload(dbcode ) {
	args(1, path, pageGroup, pageName, pageSrc, reload);
	print( pageGroup, pageName );
	not(dbcode )	dbcode='pages';
	not(path ) 		path='data/pages';
	node=_node();
	db=Class.db(dbcode);
	result = '';	
	savePageGroup=func(fileName, modifyDate) {
		file=Class.file();
		fullPath="$path/$fileName";
		not( file.isFile(fullPath) ) {
			_err("common_func", "pageReload fail: $fullPath not found!!!");
		}
		cmscode =fileName.findLast('.').trim();
		not( modifyDate ) modifyDate=file.modifyDate(fullPath);
		node[func] = file.readAll(fullPath);
		node[modifyDate] = modifyDate;
		not( db.count("select count(1) from page_info where type_code='$dbcode' and cms_code='$cmscode'") ) {
			result.add("[new page=$cmscode], ");
			db.exec("insert into page_info(type_code, cms_code, func, type, status, useyn, modifyDate) values ('$dbcode','$cmscode',#{func}, 'P', '0', 'Y',#{modifyDate})", node);
			continue;
		}
		not( db.count("select count(1) as cnt from page_info where type_code='$dbcode' and cms_code='$cmscode' and modifyDate=#{modifyDate}", node) ) {
			result.add("[modify page=$cmscode],");
			if( reload ) {
				db.exec("delete from pageFunc where cmsCode='$cmscode'");
			}
			db.exec("update page_info set func=#{func}, src=null, modifyDate=#{modifyDate} where type_code='$dbcode' and cms_code='$cmscode'", node);
		}	
	};
	savePage=func(&s, cmscode) {
		not( cmscode ) return false;
		file=Class.file();
		node[modifyDate]=file.modifyDate(fullPath);
		if( db.count("select count(1) as cnt from page_info where type_code='$dbcode' and cms_code='$cmscode' and modifyDate=#{modifyDate}", node) ) {
			return true;
		}
		while( s.valid() ) {
			s.findPos('[##');
			pageDesc=s.findPos('##]').trim();
			key=s.move();
			not( pageName.eq(key) ) {
				continue;
			}
			not( s.ch().eq(':') ) {
				break;
			}
			s.incr();
			body=s.match(1);
			node[func]="${pageName} : { $body }";
			if( db.count("select count(1) from page_info where type_code='$dbcode' and cms_code='$cmscode'") ) {
				result.add("[update page=$cmscode], $pageName");
				if( reload ) {
					db.exec("delete from pageFunc where cmsCode='$cmscode'");
				}
				db.exec("update page_info set func=#{func}, src=null, modifyDate=#{modifyDate} where type_code='$dbcode' and cms_code='$cmscode'", node);
			} else {
				result.add("[new page=$cmscode], $pageName");
				db.exec("insert into page_info(type_code, cms_code, func, type, status, useyn, modifyDate) values ('$dbcode','$cmscode',#{func}, 'P', '0', 'Y',#{modifyDate})", node);			
			}
			break;
		}
	}
	if( typeof(pageGroup,'bool') ) {
		pageGroup=null;
		reload=true;
	} else if( typeof(pageName,'bool') ) {
		pageName=null;
		reload=true; 
	} else if( typeof(pageSrc,'bool') ) {
		pageSrc=null;
		reload=true; 
	}

	if( pageGroup ) {
		if( pageName ) {
			if( pageSrc ) {
				print("xxxxxxxxxxxxxxxxxxxxxxxxx pageReload => $pageGroup, $pageName  xxxxxxxxxxxxxxxxx");
				node[modifyDate]=System.localtime();
				node[func]="$pageName : { $pageSrc }";
				not( db.exec("update page_info set func=#{func}, src=null, modifyDate=#{modifyDate} where type_code='$dbcode' and cms_code='$pageGroup'", node) ) {
					db.exec("insert into page_info(type_code, cms_code, func, type, status, useyn, modifyDate) values ('$dbcode','$pageGroup',#{func}, 'P', '0', 'Y',#{modifyDate})", node);	
				}
				if( db.error() ) {
					return false;
				}
			} else {
				fullPath="$path/${pageGroup}.pages";
				src=fileRead(fullPath);
				savePage( src.ref(), pageGroup );
			}
		} else {
			savePageGroup("${pageGroup}.pages");
		}
		Cf.makeCmsTable(dbcode);
		return true;
	} else {
		ff=Class.filefind();
		print(dbcode, path, db, ff);
		node=_node().removeAll();
		while( file , ff.fetchAll(path,'*.pages') ) {
			print("$dbcode > $cmscode file===$file[fileName]");
			savePageGroup(file[fileName], file[modifyDate]);
		}
		Cf.makeCmsTable(dbcode);
	}
	
	return result;
}

util_common.localModel() {
	not( Cf[modelIndex] ) Cf[modelIndex] = 0;
	idx = Cf[modelIndex++];
	return instance("local${idx}.model"); 
}
util_common.ynNode() {
	not( Cf[useyn] ) {
		root = {};
		root.addNode({code:Y, value:예});
		root.addNode({code:N, value:아니요});
		Cf[useyn] = root;
	}
	return Cf[useyn];
}
util_common.lpad( num, size, def ) {
	s="$num";
	return s.lpad(size, def);
}
util_common.saveIcons(path) {
	db=get('global.icons'); 
	bindBlob = callback(ty,field) { return when( field.eq('data'), 'blob', 'bind'); };
	path = "data/webpages/images/icons";
	node = inst('test.filefind').fetchAll(path,'*.png');
	while( file , node, n, 0 ) {
		a = file[fileName].findLast('.').value(); 		
		not( a.find('_') ) {
			a.add('_default');
		}
		fullpath = "$path/$file[fileName]";
		node[data] = inst('vrs.file').readAll(fullpath);
		node[id] = a;
		node[type] = 'vicon';
		node[tm] = System.localtime();
		not( db.exec("update icons set data=#{data}, tm=#{tm} where id=#{id} and type=#{type}",	node, bindBlob) ) {
			db.exec("insert into icons (id,type,data,prop,use,tm) values (#{id},#{type},#{data},'clipboard','Y',#{tm})", node, bindBlob);
		}
	}
	node.delete();
}
util_common.replaceText(src, sep, rep, flag) {
	str = '';
	while( n, 32 ) {
		left = src.findPos(sep, flag);
		not( src.valid() ) {
			str.add( left );
			break;
		}
		str.add( left, rep );
	}
	return str;
}
util_common.findNodeByCode(node, code) {
	while( cur, node) {
		if( cur[code].eq(code) ) return cur;
	}
	return null;
}
util_common.useYnNode() {
	not( Cf[useyn] ) {
		root = {};
		root.addNode({code:Y, value:사용});
		root.addNode({code:N, value:미사용});
		Cf[useyn] = root;
	}
	return Cf[useyn];
}
util_common.findCodeValue(node, code) {
	while( cur, node) {
		if( cur[code].eq(code) ) return cur[value];
	}
	return null;
}
util_draw.rateArray(info, tot, arr) {
	if( typeof(info,'array') ) {
		rates=info;
	} else if( typeof(info,'number') ) {
		rates=class('util').arr();
		while( n, info ) rate.add(4);
	} else {
		rates=info.split();
	}

	tr = rates.sum();
	not( arr ) arr = class('util').arr();
	arr.reuse();
	while( cur, rates ) {
		if( typeof(cur,'array') ) {
			while( num, cur ) {
				arr.add( expr( (num/tr.0)*tot) );
			}
		} else {
			arr.add( expr( (cur/tr.0)*tot) );
		}
	}
	arr.recalc(tot);
	return arr;
}
util_draw.drawSpin( rc, canvas, val ) { 
	Cf[gridSpinParent] = canvas;
	not( Cf[gridSpin] ) {
		Cf[gridSpin]=Cf.widget(template() { 
			tag:spin , 
			onKeyDown() {
				if( @key.eq(KEY.Escape) ) {
					this.hide();
				} else if( @key.eq(KEY.Tab, KEY.Return, KEY.Enter) ) {
					Cf[gridSpinParent].send('result','spin',this.value());
					this.hide();
				}
			} 
		});
		Cf[gridSpin].flags('tooltip');	
	}
	input = Cf[gridSpin];
	input.value(val);
	input.select();
	input.geo(rc);
	input.move(canvas.mapGlobal(rc.lt()) );
	input.open();
	return input;
}
util_etc.checkTickCount(node, var) {
	not( node[$var@delay] ) 
		return true;
	dist = System.tick() - node[$var@tick];
	if( dist > node[$var@delay] ) 
		return false;
	return true;
}
util_widget.gridAddRow(grid, cur, col) {
	cur.state(NODE.add, true);
	grid.update();
	grid.current(cur);
	grid.scroll(cur);
	if( typeof(col,'number') ) {
		grid.edit(cur,col);
	} else if( col ) {
		grid.edit(cur, grid.field(col));
	}
}
util_widget.openCenter( w, p ) {
	w.open();
	pos = p.mapGlobal();
	rc = p.rect().move(pos).center(w.size());
 	print("rc====>$rc");
	w.move(rc.lt());
}
util_draw.topTitleDraw(d,title) {
	rc=d.rect();
	d.image(rc.width(1024).sp(r1), Image.gss.top);
	d.image(rc.move(r1.rt()), Image.gss.topbg, 'fill');
	d.pen('#e0e0e0').font(14,'bold').text(rc.move(12,6), title);	
	d.pen('#303030').text(rc.move(10,4), title);	
}
util_common.nodeArray(node, val, ty ) {
	arr=[];
	not( ty ) ty='start';
	while( k, node.keys() ) {
		switch( ty ) {
		case start:	if( k.start(val) ) arr.add(node[$k]);
		case eq:		if( k.eq(val) ) arr.add(node[$k]);
		case in:		if( k.find(val) ) arr.add(node[$k]);
		case notin:	not( k.find(val) ) arr.add(node[$k]);
		default: break;;
		}
	}
	return arr;
}
util_draw.drawTreeIcon(d, rc, node ) {
	if( d.state(STYLE.Selected) ) {
		d.fill( rc.x(0,true), '#f0f0f0' );
	} else {
		d.fill( rc.x(0,true), '#ffffff' );
	}
	rcIcon = rc.width(16); 
	if( node.childCount() ) {
		if( d.state(STYLE.Open) ) {
			d.image( rcIcon.center(14,16).incrY(2), Image.tree.plus );
		} else {
			d.image( rcIcon.center(14,16).incrY(2), Image.tree.minus );			
		}
	} else if( node[depth]<depth ) {
		d.image( rcIcon.center(14,16).incrY(2), Image.vicon.bullet_black );			
	}
	return rcIcon;
}
util_draw.drawTree(d, node ) {
	rc = d.rect().incrX(-20);
	rcIcon = drawTreeIcon(d,rc,node);
	r = rc.move(rcIcon.rt()).width(16); 
	switch( node[depth]) {
	case 0:
		d.icon( r.center(14,14), node[icon] );
		d.text( rc.move(r.rt()), node[value] );
	case 1:
		d.icon( r.center(14,14), Icon.vicon.folder_database );
		d.text( rc.move(r.rt()), node[value] );
	case 2:
		d.icon( r.center(14,14), Icon.vicon.database_table );
		d.text( rc.move(r.rt()), node[value] );
	default:
		d.text( rc.move(rcIcon.rt()), node[value] );
	} 
}
util_draw.drawGridModify(d, node, rc) {
	if( node.state(NODE.modify) ) {
		d.fill( rc, '#d0e0ff50' );
		return true;
	} 
	if( node.state(NODE.add) ) {
		d.fill( rc, '#fff0c0' );
		return true;
	}
	return false;
}
util_draw.drawGrid(d, node, grid) {
	rc=d.rect();
	if( d.state(STYLE.Selected) ) {	
		d.fill( rc, '#f0f0f0' );
	} else {
		d.fill();
	}
	field=grid.field(d.index());
	switch( field ) {
	case check:		
		if( node[checked] ) 
			d.icon(rc.center(16.16), Icon.func.check);
		else
			d.icon(rc.center(16.16), Icon.func.add);
	default:
		d.text( rc.incrX(2), node.value(field) );	
	} 
	d.rectLine(rc,4,'#d0d0d0');
}
util_common.makeMenuText( sub, depth ) {
	data = '';
	not( depth ) 
		depth=0;
	if( sub[text].eq('-') ) {
		data.add('-,');
		return data;
	}
	if( depth.eq(0) ) {
		data.add("{id: ROOT, ");
	} else {
		data.add("{ id: $sub[id], text: $sub[text], icon:ICON.vicon.$sub[icon], ");
	}
	if( sub.size() ) {
		data.add("type:menu, actions:[");
		while(node, sub ) {
			data.add( makeMenuText(node, depth+1) );
		}
		data.add("]}");
	} else {
		data.add("}");
	}
	if( depth ) data.add(",");
	return data;	
}
util_common.tableColumnArray(db, table) {
	root={table:$table};
	arr=[];
	while( cur, db.fetchAll("select column_name from INFORMATION_SCHEMA.COLUMNS where table_name =#{table}", root) ) {
		arr.add(cur[column_name]);
	}
	root.delete();
	return arr;
}
util_widget.gridPopup( popup, grid, node, index ) {
	rc = grid.nodeRect(node, index);
	openPopup( popup, grid.mapGlobal(rc) );
}
util_widget.commTreeDraw( d, node, startDepth) {
	rc = d.rect().incrX(-20);
	if( d.state(STYLE.Selected) ) {
		d.fill( rc.x(0,true), '#f0f0f0' );
	}
	rcIcon = rc.width(16); 
	if( node.childCount() ) {
		if( d.state(STYLE.Open) ) {
			d.image( rcIcon.center(14,16).incrY(2), Image.tree.plus );
		} else {
			d.image( rcIcon.center(14,16).incrY(2), Image.tree.minus );			
		}
	} else if( node[depth]<4 ) {
		d.image( rcIcon.center(14,16).incrY(2), Image.vicon.bullet_black );			
	}
	
	r = rc.move(rcIcon.rt()).width(16); 
	if( node[depth].eq(startDepth) ) {
 		d.icon( r.center(14,14), "vicon.$node[icon]");
		d.text( rc.move(r.rt()), node[value] );
 	} else {
		if( node[checked] )
			d.icon( r.center(14,14), Icon.func.check );
		else
			d.icon( r.center(14,14), Icon.func.add );
		d.text( rc.move(r.rt()), node[value] ); 
	} 
}
util_widget.commTreePopup(parent, title, code, icon ) {
	not( icon ) icon = "vicon.user_edit";
	cf = {
		title: $title,
		icon: $icon,
		layout: <page><tree id=tree><hbox><button id=ok text=확인><button id=cancel text=취소></hbox></page>
		onInit() {
			db = inst('p1.model');
			this[tree].check('treeMode',true);
			this[tree].model(localModel(), 'value' );
		}
		initData(node, parent) {
			root = this[tree].rootNode();
			root.initNode(node);
			loadChild = callback(node) {
				not(node ) return;
				db.fetchAll("select code, pcode, depth, value, icon from commtree where ref='CC' and pcode=#{code} order by sort, seq", node );
				while( cur, node ) {
					loadChild(cur);
				}
			}
			db.fetchAll("select code, pcode, depth, value, icon from commtree where ref='CC' and code=#{code} order by sort, seq", root );
			cur = root.child(0);
			this[startDepth] = cur[depth];
			loadChild(cur);
			this[tree].update();
			this[tree].expand(cur);
			this[opener] = parent;
		}
		ok.onClick() {
			this[opener].commTreePopupOk(this);
			this.hide();
		}
		cancel.onClick() {
			this.hide();
		}
		tree.onDraw() {
			commTreeDraw(@draw, @node, this[startDepth]);
		}
		tree.onMouseDown() {
			node = @me.at(@pos);
			if( node[depth]<2 ) return;
			checkChild = callback(node) {
				chk = node[checked];
				while( cur, node ) {
					cur[checked] = chk;
					checkChild(cur);
				}
			}
			rc = this[tree].nodeRect(node);
			rcIcon = rc.width(16).center(14,14);
			if( rcIcon.contains(@pos) ) {
				node.toggle('checked');
				checkChild(node);
				@me.update();
			}
		}
 		getCheck() {
			arr = [];
			root = this[tree].rootNode().child(0);
			getCheckArray = callback(node) {
				while( cur, node ) {
					if( cur[checked] ) {
						not( cur.size() ) arr.add(cur);
					}
					if( cur.size() ) getCheckArray( cur );
				}
			}
			getCheckArray( root );
			return arr;
		}
		setCheck(fetchNode) {
			tree = this[tree];
			find = callback(code) {
				while( cur, fetchNode ) {
					if( cur[code].eq(code) ) return true; 
				}
				return false;
			}
			setCheckArray = callback(node) {
				while( cur, node ) {
					not( fetchNode ) {
						cur[checked] = false;
						continue;
					}
					cur[checked] = when( find(cur[code]), true ); 
					if( cur[checked] ) {
						tree.expand(cur.prent());
					}
					if( cur.size() ) setCheckArray(cur);
				}
			}
			root = tree.rootNode().child(0);
			setCheckArray( root );
			this[tree].update();
		}
	};
	popup = parent.widget(cf);
	node = {code: $code};
	if( node[code] ) {
		popup.initData(node, parent);
	}
	Cf[currentPopup] = popup;
	return popup;
}
util_widget.gridEditFinish(grid, node, index) {
	field=grid.field(index);  
	not( node[$field].eq(data) ) {
		not( node.state(NODE.add) )
			node.state(NODE.modify, true);
		node[$field] = data;
	}
	grid.update();
	grid.check('sortEnable',true);
}
util_widget.gridCombo(node, val, def) {
	combo=this.widget('tag:combo',true);
	combo.addItem(node, 'code,value', when(def,'==선택=='));
	combo.value(val);
	return combo;
}
util_widget.gridCheck(grid, node, button) {
	node.toggle('checked');
	grid.update();
	bchk = false;
	while( cur, grid.rootNode() ) {
		if( cur[checked] ) {
			bchk=true;
			break;
		}
	}
	not( button ) return;
	if( bchk ) {
		button.show();
	} else {
		button.hide();
	}
}
util_common.getRateArray(rates, tot, arr) {	tr = rates.sum();
	not( tot ) return null;
	not( tr ) return null;
	if( arr ) 
		arr.reuse();
	else 
		arr = [];
	
	_child = callback(parr) { 
		while( a, parr ) {
			if( typeof(a,'array') ) {
				_child(a);
			} else {
				arr.add( expr( (a/tr.0)*tot) );
			}
		}
		return sum;
	}
	while( cur, rates ) {
		if( typeof(cur,'array') ) { 
			_child(cur);
		} else {
			arr.add( expr( (cur/tr.0)*tot) );
		}
	}
	arr.recalc(tot);
	return arr;
}
util_common.mergeRect() {
	arr=args(), var=arr[0];
	rc=Class.rect();
	if( typeof( var,'node') ) {
		if( arr.size().eq(3) ) {
			arr.inject(node, startIndex, lastIndex);
		} else {
			arr.inject(node, lastIndex);
			startIndex=0;
			not( typeof(lastIndex,'number') ) lastIndex=-1;
		}
		fst=node.child(startIndex), last=node.child(lastIndex);
		node[rect]=rc.merge(fst[rect], last[rect]);		
		return node[rect];
	} else if( typeof(var,'rect') ) {
		arr.inject(r1, r2); 
		return rc.merge(r1,r2);
	}
	return rc;


}
util_class.stripComment(&src ) {
	rst = '';
	while( src.valid() ) {
		left = src.findPos("/*",1,1);
		rst.add(left);
		not( src.valid() ) break;
		not( src.match(1) ) {
			print("########### comment match error ############");
			break;
		}
	}
	return rst;
}
util_etc.pow(mlvl) {
	while( n, mlvl ) {
		if( n.eq(0) ) {
			mr=1;
		} else {
			mr*=2;
		}
	}
	return mr;
}
util_etc.getNodeArray(node, code) {
	a=node[$code];
	if( a ) {
		a.reuse();
	} else { 
		a=[];
		node[$code] = a;
	}
	return a;
}
util_etc.arrayEqual( a, b ) {
	not( typeof(a,'array') && typeof(b,'array') ) return false;
	size = a.size();
	if( size.ne( b.size()) ) return false; 
	while( v, a, n, 0 ) {
		not( v.eq(a[$n]) ) return false;
	}
	return true;
}
util_class.makeClass(&src ) {
	while( src.valid() ) { 
		ch = src.ch();
		not( ch )
			break;
		classNm = src.move().trim();
		classParam = '';
		c = src.ch();
		param = null;
		if( c.eq('(') ) {
			param = src.match(1);
			if( param.ch() ) {
				classParam = param.value();
			}
		}
		blib = when( classParam.eq('class'), true );
		c = src.ch();
		not( c.eq('{') ) {
			print("not func body");
			return false;
		}
		in = src.match(1);
		cls='';
		while( in.valid() ) {
			c = in.ch();
			if( c.eq('#') ) {
				in.incr();
				w = in.move();
			} else {
				w = in.move();
				c = in.ch();
			}
			not( w ) break;
			if( c.eq("(") ) {
				param = in.match(1);
				c=in.ch();
				not( c.eq("{") ) {
					print("$w func not valid");
					return false;
				}
				body = in.match(1);
				if( w.eq(classNm) ) { 
					if( param.ch() ) {
						not( classParam ) classParam=param.value();
					} else {
						param = classParam;
					}
					cls.add("\r\n\tclass[$w] = func($param) {\r\n$body\r\n\t}");
				} else {
					cls.add("\r\n\tnot(class[$w]) class[$w] = func($param) {\r\n$body\r\n\t}");
				}
			} else if( c.eq("#") ) {
				c=in.ch();
				if( c.eq('(') ) {
					in.match(1);
					c=in.ch();
				}
				not( c.eq("{") ) {
					print("$w func not valid");
					return false;
				}
				body = in.match(1);
				cls.add("\r\n\tnot(class[$w]) class[$w] = func() {\r\n$body\r\n\t}");
			} else {
				print("$w $c not valid char");
				return false;
			}
		}
		not( blib ) {
			cp = '';
			if( classParam ) cp = ", $classParam";
			cls.add("\r\n\tCf.setClass(class, '$classNm' $cp);");
		}
		cls.add("\r\n\treturn class;");
		print("cls ############ $cls");
		Cf.func("${classNm}($classParam) {\r\n\tnot(class) class={};\r\n$cls \r\n}\r\n");
	}
	return true;
}
util_class.stripLineComment(&src ) {
	rst = '';
	while( src.valid() ) {
		left = src.findPos("//",1);
		rst.add(left,"\r\n");
		src.findPos("\n");
	}
	return rst;
}
util_common.loadPage(path, overwrite) {
	fileName = path.findLast('/').right().value();
	fileNm = fileName.findLast('.').value();
	not( overwrite) {
		while( page, Cf.pageList() ) {
			if( page.eq(fileNm) ) return;
		}
	}
		
	node={}, file=instance('p1.file');
	node[func] = file.readAll(path);
	node[modifyDate] = file.modifyDate(path);
	
	db = instance('pages.model');
	if( db.fetch("select count(1) as cnt from page_info where type_code='pages' and cms_code='$fileNm'").eq('cnt',0) ) {
		db.exec("insert into page_info(type_code, cms_code, func, type, status, useyn, modifyDate) values ('pages','$fileNm',#{func}, 'P', '0', 'Y',#{modifyDate})", node);
	} else if( db.fetch("select count(1) as cnt from page_info where type_code='pages' and cms_code='$fileNm' and modifyDate=#{modifyDate}", node).eq('cnt',0) ) {
		db.exec("update page_info set func=#{func}, src=null, modifyDate=#{modifyDate} where type_code='pages' and cms_code='$fileNm'", node);
	} 
	node.delete();
	Cf.makeCmsTable('pages');
}
util_class.makeSrc(&src) {
	if( src.find('/*',1) ) {
		src = stripComment(src.ref());
	}
	if( src.find('// ',1) ) {
		src = stripLineComment(src.ref());
	}
	return src;
}
util_etc.tickCheck() {

}
util_etc.setArray( arr, idx, cur ) {
	not( typeof(arr,'array') ) return;
	if( idx< arr.size() ) {
		arr[$idx] = cur;
	} else {
		arr.add(cur);
	}
	return arr;
}
util_etc.splitTrim(&str, sep) {
	arr = [];
	while( str.valid() ) {
		arr.add( str.findPos(sep).trim() );
	}
	return arr;
}
util_etc.indentText(&s) {
	sp=s.cur();
	s.ch();
	ep=s.cur();
	return s.value(sp,ep,true);}
util_etc.openPopup(page, parent, rc) {
	rcScreen = System.info('screenRect', rc);
	if( rc.bottom() > rcScreen.bottom() ) {
		dist = rcScreen.bottom() - rc.bottom();
		rc.incrY(dist,true);
	}
	if( rc.right() > rcScreen.right() ) {
		dist = rcScreen.right() - rc.right();
		rc.incrX(dist,true);
	}
	page.flags('popup');
	if( parent ) {
		page.open(parent, rc); 
	} else {
		page.open(rc); 
	}
}
util_etc.allSourceConfig() {
	return {
		title: 전체소스 보기,
		icon: ICON.vicon.table_edit,
		layout: 
		<page>
			<label id=funcInfo text="전체소스 정보">
			<editor id=src>
			<hbox>
				<button id=ok text=적용 width=60>
				<space>
				<button id=cancel text=취소 width=60>
			</hbox>
		</page>
		onInit() {
			editorInit(this[src]); 
		}
		cancel.onClick() {
			this.close();
		}
		ok.onClick() { 
			parentWindow.allSourceConfigOk(this);
		}
		initSource(data, info) {
			this[src].clear();
			this[src].insert(data,true); 
			this[funcInfo].value(info);
		}
	}
}
util_etc.cursorRect(w,h) {
	pt = System.info('cursor');
	return Class.rect(pt,w,h);
}
util_etc.blankText(&s) {
	sp=s.cur();
	s.ch();
	ep=s.cur();
	return s.value(sp,ep,true);
}
util_etc.makeIndent( &str, tabs ) {
	rst='', n=0;
	while( str.valid() ) {
		line = str.findPos("\n");
		indent = blankText(line.ref);
		ch = line.ch();
		not( ch ) {
			rst.add("\n");
			continue;
		}
		val ='';
		if( n.eq(0) ) {
			fst = indent.size();
		} else if( fst<indent.size() ) {
			val = indent.value(fst);
		print("$n =>$val##");
		}
		rst.add("\n$tabs$val$line");
		n++;
	}
	return rst;
}
util_etc.divideColumn(rc, num, incr, arr) {
	not( arr ) arr=[];
	not( typeof(rc,'rect') ) return arr;
	x=rc.x(), y=rc.y(), h=rc.height();
	getRateArray(num, rc.width(), arr);
	while( w, arr, n, 0 ) {
		rc = Class.rect(x, y, w, h);
		x+=w;
		arr[$n] = when(incr, rc.incr(incr), rc);
	}
	return arr;
}
util_etc.divideRow(rc, num, incr, arr) {
	not( arr ) arr=[];
	not( typeof(rc,'rect') ) return arr;
	x=rc.x(), y=rc.y(), w=rc.width();
	getRateArray(num, rc.height(), arr );
	while( h, arr, n, 0 ) {
		rc = Class.rect(x, y, w, h);
		y+=h;
		a[$n] = when(incr, rc.incr(incr), rc);
	}
	return arr;
}
util_etc.parseData( &data, root ) {
	not( root ) root = {};
	row = 0;
	while( data.valid() ) {
		bchk = true;
		if( data.ch().eq('<') ) {
			sp = data.cur();
			tag = data.incr().move();
			data.pos(sp);
			if( tag.eq("text") ) {
				idx = 0;
				while( data.valid() ) {
					val = data.match("<text>","</text>");
					not( val )
						 return false;
					root[$row@$idx] = val.trim();
					idx++;
					not( data.ch().eq(',') ) {
						row++;
						break; 
					} 
						
					data.incr();
					if( data.ch().eq('<') ) {
						sp = data.cur();
						tag = data.incr().move();
						data.pos(sp);
						not( tag.eq("text") ) break;				
					} else break;
				}
				data.ch();
				bchk = false;
			}
		} 
		if( bchk ) {
			line = data.findPos("\n");
			print("line=>$line");
			not( line.ch() ) continue;
			idx =0;
			while( line.valid() ) {
				val = line.findPos(",");
				root[$row@$idx] = val.trim();
				idx++;
			}
			row++;
		}
	}
	return root;
}
util_etc.getStateType(rc) {
	state = rc.state();
	return state & 0xF;
}
util_etc.findArrayCode(arr, code) {
	while(key,arr) {
		if( key.eq(code) ) return true;
	}
	return false;
}
util_etc.getEpsPage(page, ref, db) {
	not( Cf[pageData] ) Cf[pageData] = {};
	root = Cf[pageData];
	root.removeAll();
	not( db ) db = instance('config.model');
	sql ="select b.data as data
		from (select idx as tree_idx, sort from epsTreeList where pidx in (select idx from epsTreeList where type='mission' and ref='$ref') ) A, epsTreeStep B
		where 
			  A.tree_idx = B.tree_idx
		order by sort";
	rst='';
	while( cur, db.fetchAll(sql,root) ) {
		rst.add(cur[data]);
	}
	return page.widget(rst);
}
util_etc.getTypePath(ty) {
	path=conf('project.workspace'), typePath=conf("project.$ty");
	return "$path\\$typePath";
}
util_etc.exploreFiles(path, root, modify ) {
	ff = instance('eps.filefind');
	not( root ) root = {type:explore};
	not( modify ) modify = 0;
	_child = callback(path, parent) {
		ff.fetch(path,  parent, NULL, modify);
		while( c, parent) {
			if( c.state(FF.folder) ) { 
				_child("$path/$c[name]", c);
			}
		}
	}
	_child(path, root);
	return root;
}
util_class.loadClassByDb(code) {not( Cf[classNode] ) Cf[classNode]= {};
root = Cf[classNode];
root.removeAll();
db = instance('pages.model');

if( code.find('.') ) {
	groupName=code.find('.').trim();
	className=code.find('.').right().trim();
} else {
	className=code, groupName='';
}

root.put( className, groupName);
instance("pages.model").fetchAll("select 
	class_func, 
	class_param, 
	case when length(class_src)=0 then class_data else class_src end as class_src, 
	type 
from 
	class_info 
where 
	class_nm=#{className} 
	#[groupName ? and class_grp=#{groupName}] 
order by type", root );

not( root.childCount() ) return false;

rst="", classParam="";
while( node, root ) {
	fnm = node[class_func], fparam = node[class_param];
	if( node[type].eq('A' ) ) {
		classParam=fparam;
	}
	if( node[type].eq('S') ) {
		rst.add("\r\nclass[$fnm]= callback($fparam) {\r\n${node[class_src]}\r\n}" );
	} else {
		rst.add("\r\nclass[$fnm]= func($fparam) {\r\n${node[class_src]}\r\n}" );
	}
}

cp = '';
if( classParam ) cp = ", $classParam";
rst.add("\r\n\tCf.setClass(class, '$className' $cp);");
rst.add("\r\n\treturn class;");

Cf.func("${className}($classParam) {\r\n\tnot(class) class={};\r\n$rst \r\n}", false, groupName);
print("loadClassByDb ############## ${className}($classParam)");
return true;}
util_class.makePages(db, dbcode, path) {not( db ) 			db = get('global.config');
not( dbcode ) 	dbcode = 'pages';
not( path ) 		path = 'data/pages';

not( Cf[classNode] ) Cf[classNode]= {};
node = Cf[classNode];

result = '';
while( file , instance('eps.filefind').fetchAll(path,'*.pages') ) { 
		cmscode = file[fileName].findLast('.').trim();
 		node[func] = instance('eps.file').readAll("$path/$file[fileName]");
		node[modifyDate] = file[modifyDate];
		if( db.fetch("select count(1) as cnt from page_info where type_code='$dbcode' and cms_code='$cmscode'").eq('cnt',0) ) {
			result.add("[new page=$cmscode], ");
			db.exec("insert into page_info(type_code, cms_code, func, type, status, useyn, modifyDate) values ('$dbcode','$cmscode',#{func}, 'P', '0', 'Y',#{modifyDate})", node);
		} else if( db.value("select count(1) as cnt from page_info where type_code='$dbcode' and cms_code='$cmscode' and modifyDate=#{modifyDate}", node).eq('0') ) {
			result.add("[modify page=$cmscode],");
			db.exec("update page_info set func=#{func}, src=null, modifyDate=#{modifyDate} where type_code='$dbcode' and cms_code='$cmscode'", node);
		}
}
Cf.makeCmsTable(dbcode);
return result;
}
util_etc.getPage(page, ref) {
	not( Cf[pageData] ) Cf[pageData] = {};
	root = Cf[pageData];
	cur = root[$ref];
	if( cur ) return cur;
	root.removeAll();
	db = get('global.config');
	sql ="select b.data as data
		from (select idx as tree_idx, sort from epsTreeList where pidx in (select idx from epsTreeList where type='mission' and ref='$ref') ) A, epsTreeStep B
		where 
			  A.tree_idx = B.tree_idx
		order by sort";
	rst='';
	while( cur, db.fetchAll(sql,root) ) {
		rst.add(cur[data]);
	}
	cur = page.widget(rst);
	
	print("getPage ################# ref=$ref, $cur");
	root[$ref] = cur;
	return cur;
}
util_class.saveClassFile(pageDb, &src, group, arr, tm) {dataNode=_node('classNode');

not( tm ) tm = System.localtime();

dataNode[lastModify] = lastModify;
while( src.valid() ) { 
	comment = '';
	ch = src.ch();
	if( ch.eq('/') ) {
		while( ch.eq('/') ) {
			ch = src.ch(1);
			if( ch.eq('/') ) {
				src.incr(2);
				comment.add( src.findPos("\n") );
			} else if( ch.eq('*') ) {
				comment.add( src.match('/*', '*/',1) );
			}
			ch = src.ch();
		}
	}
	not( ch ) break;
	classNm = src.move().trim();
	not( classNm ) break;
	if( arr ) {
		arr.add(classNm);
	}
	classParam = ''; 
	param = null;
	dataNode.put(group, classNm, comment );
	if( src.ch().eq('(') ) {
		param = src.match(1).trim();
		if( param.ch() ) {
			classParam = param;
		}
	}
	blib = when( classParam.eq('class'), true );
	dataNode[type] = when( blib, 'L' );
	not( pageDb.exec("update class_mst set class_desc=#{comment}, type=#{type} where class_grp=#{group} and class_nm=#{classNm}", dataNode) ) {
			pageDb.exec("insert into class_mst(class_grp, class_nm, class_desc, type, tm) values (#{group}, #{classNm}, #{comment}, #{type}, '$tm')", dataNode);
	}
	pageDb.exec("delete from class_info where class_grp=#{group} and class_nm=#{classNm}", dataNode);

	not( src.ch().eq('{') ) {
		dataNode[note] = "not func body";
		pageDb.exec("update class_mst set note=#{note} where class_grp=#{group} and class_nm=#{classNm}", dataNode);
		return false;
	}
	in = src.match(1);
	cls='';
	while( in.valid() ) { 
		comment = '';
		ch = in.ch();

		if( ch.eq('/') ) {
			while( ch.eq('/') ) {
				ch = in.ch(1);
				if( ch.eq('/') ) {
					in.incr(2);
					comment.add( in.findPos("\n") );
				} else if( ch.eq('*') ) {
					comment.add( in.match('/*', '*/',1) );
				}
				ch = in.ch();
			}
		}
		w = in.move();
		not( w ) break;
		funcType='F';
		print("w=$w");
		if( w.eq('public', 'private', 'persist','static', 'interface') ) {
			if( w.eq('public') ) {
				funcType='P';
			} else if( w.eq('private') ) {
				funcType='Z';
			} else if( w.eq('interface') ) {
				funcType='I';
			} else {
				funcType='S';
			}
			w = in.move();
		} 
		not( in.ch().eq("(") ) {
			dataNode[note] = "function:$w char:$ch is not valid : must be input func param";
			pageDb.exec("update class_mst set note=#{note} where class_grp=#{group} and class_nm=#{classNm}", dataNode);
			return false;				
		}
		param = in.match(1).trim(); 
		not( in.ch().eq("{") ) {
			dataNode[note] = "function:$w char:$ch is not valid : must be input func body";
			pageDb.exec("update class_mst set note=#{note} where class_grp=#{group} and class_nm=#{classNm}", dataNode);
			return false;
		}
		body = in.match(1);
		if( w.eq(classNm) ) { 
			if( param.ch() ) {
				not( classParam ) classParam=param;
			} else {
				param = classParam;
			}
			dataNode[type] = 'A';
		} else {
			dataNode[type] = funcType;
		}
		if( body.finds('/*','//') ) {
			fsrc = body;
			if( fsrc.find('/*',1) ) {
				fsrc = stripComment(fsrc.ref());
			}
			if( fsrc.find('//',1) ) {
				fsrc = stripLineComment(fsrc.ref());
			}
		} else {
			fsrc=null;
		}
		dataNode.put(w, param, body, fsrc, comment);
		/*
		모두 지우고 시작하므로 업데이트할 내용이 없다
		not( pageDb.exec("update class_info set 
			class_param=#{param}, class_data=#{body}, class_src=#{fsrc}, 
			note=#{comment}, type=#{type}, tm='$tm' 
			where class_grp='$group' and class_nm=#{classNm} and class_func=#{w}", dataNode) ) {
		} 
		*/
		pageDb.exec("insert into class_info(class_grp, class_nm, class_func, class_param, class_data, class_src, note, type, tm) values (#{group}, #{classNm}, #{w}, #{param}, #{body}, #{fsrc}, #{comment}, #{type}, '$tm')", dataNode);
	}
}

return true;

}
util_widget.formCheck(page, id, msg) {el = page[$id];
val = el.value();
not( val ) {
	page.alert(msg);
	el.focus();
	return true;
}
return false;}
util_class.loadClass(fullpath) {
not( fullpath.find('.') ) {
	className = fullpath;
	if( loadClassByDb(className) ) {
		print("load By DB : $className");
		return true;
	}
	fullpath = "data/classes/${className}.class";	
} 

print("loadClass fullpath===$fullpath");
data = inst('p1.file').readAll(fullpath);

if( data.find('/*',1) ) {
	data = stripComment(data.ref());
}
if( data.find('//',1) ) {
	data = stripLineComment(data.ref());
}
return makeClass( data.ref() );}
util_etc.makeFormData(db, node, &data) {&param={};
page_idx=node[page_idx], sql_cd=node[sql_cd];
db.exec("delete from project_page_form where page_idx=#{page_idx} and sql_cd=#{sql_cd}", node);
data.findPos("#form");
row=0, col=0;
print();
while( data.valid() ) {
	line = data.findPos("\n");
	not( line.ch() ) continue; 
	prev = line.prevChar();
	while( line.valid() ) {
		field = line.move().trim();
		ch= line.ch();
		if( ch.eq(":") ) {
			 line.incr();
			 ch=line.ch();
		}
		if( ch.eq("{") ) {
			sp = line.cur();
			line.match();
			ep = line.cur();
			param.reuse().parseJson( line.value(sp,ep,true) ); 
		}
		not( field ) break;
		ch=line.ch();
		param.put(field, row, col, page_idx, sql_cd);
		db.exec("insert into project_page_form (  
			page_idx, sql_cd, field, label, row_num, col_num, tag, style, prop, alert , 
			next_focus, class_nm, data, width, info, note
		) values (
			#{page_idx}, #{sql_cd}, #{field}, #{label},#{row}, #{col}, #{tag}, #{style}, #{prop}, #{alert},
			#{next}, #{class}, #{data}, #{width}, #{info}, #{note}
		)", param);
		not( ch.eq(",", "\\") ) break;
		line.incr();
		col++;
	}
	if( prev.eq("\\") ) continue;
	row++;
	col=0, sep=0;
} }
util_widget.codeCombo(page, pcode, val, idx) {combo=page.widget(ginfo('inlineCombo'));
combo.addItem( class('code').ccRefNode(pcode), 'ref1,value', '=선택=');
combo.init(page, idx);
if( val ) combo.value(val);
return combo;
}
util_etc.getLocalPath(&path) {not( path ) return System.path();
if( path.start('/') ) {
	a=System.path();
	a.add(path);
	return getLocalPath(a);
}
s='';
while( path.valid() ) {
	left = path.findPos('/');
	s.add(left);
	if( path.valid() ) s.add("\\");
}
return s;}
util_etc.jobTrigger(target, name, stat) {
root = Cf[jobRoot];
not(root ) {
	root = {};
	Cf[jobRoot] = root;
}
root[target] = target.utf8();
root[name] = name.utf8();

db = get('global.config');
last = db.value("select modify_dtm from comm_job_file where target=#{target} and name=#{name}", root);
tm = System.localtime();

if( last ) {
	dist = tm-last;
	print("dist====$dist");
	if( stat.eq(3) ) {
		if( dist<100 ) {
			print("xxxxxx may be dup xxxxxxx");
			return;
		}
	}
}

type='folder';
if( instance('my.file').isFile("$root[target]/$root[name]") ) {
	type='file';
}

cnt = db.exec("update comm_job_file set stat='$stat', modify_dtm=$tm where  target=#{target} and name=#{name}",root);
if( db.error() ) {
	print("db error : $db.error()");
	return;
}
not( cnt ) {
	db.exec("insert into comm_job_file ( target, name, type, stat, modify_dtm) values ( #{target}, #{name}, '$type', '$stat', $tm)", root);
}

print("$last, $cnt, $targetNm/$fileNm");}
util_class.class(code, reload, setClass) {	not( Cf[classData] ) Cf[classData]={};
	cf = Cf[classData]; 
	if( reload ) {
		cf[$code]=null;
		if( setClass ) setStaticClass(code);
	}
	if( cf[$code] ) return cf[$code];
	switch(code) {
	case code:
		not( typeof(codeClass,'class'), or(reload) ) loadClass('codeClass');
		db = get('global.config');
		cf[$code] = codeClass( db );			
	case db: 
		not( typeof(dbClass,'class'), or(reload) ) loadClass('dbClass');
		cf[$code] = dbClass();
	case parse:
		not( typeof(parseClass,'class'), or(reload) ) loadClass('parseClass');
		cf[$code] = parseClass();
	case file:
		not( typeof(fileClass,'class'), or(reload) ) loadClass('fileClass');
		cf[$code] = fileClass();
	default:
		classCd="static.$code";
		fc=Cf.class(classCd);
		not( typeof(fc,'class'), or(reload) ) {
			loadClassByDb(classCd);
		}
		cf[$code] = Cf.class(classCd,true);
	}
	return cf[$code];
	}
util_draw.drawMatchTwo(d, match, rc) {match.inject(maxLevel, centerRect, dataNode);
match.conf(rc);		
halfLevel = maxLevel/2;
info = dataNode[info.0];

_setValue = callback(rc, node, row) {
	if( node[code] && row.eq(0) ) {
		d.icon(rc.width(24).center(20,20), "sido.${node[code]}_icon");
		rc.move(28);
	} else {
		rc.move(4);
	}
	d.text(rc, "$node[city] $node[name]");
}

while( row, halfLevel) { 
	halfSize = match[r$row].size() / 2; 
	data = dataNode[info.$row];
	while( rc, match[r$row], n, 0 ) {
		hh = rc.height()/2;
		if( row==0 ) {
			a = n*2;
			if( info[${a}.city].eq('#') ) {
				if( n%2 ) {
					r1 = rc.move('bottom',28);
					d.rectLine(rc, 4, '#a0a0a0', 2);
				} else {
					r1 = rc.incrH(-1*hh).height(20);
					d.rectLine(rc, 2, '#a0a0a0', 2);
				}
				a++;
				_setValue(r1, data[$a], row );
				continue;
			}
		} else if( row==1 ) {
			a=n*4, b=a+2, ar=n*2, br=ar+1, r=null, h=0;
			if( n<halfSize ) {
				if( info[${a}.city].eq('#') ) {
					r = match[r0.$ar];
					x=r.right(), y=r.y(), h=r.height()/2;
				}
				if( info[${b}.city].eq('#') ) {
					r = match[r0.$br];
					not( h ) {
						x = rc.x(), y=rc.y();
					}
					h+=r.height()/2;
				}
			} else {
				if( info[${a}.city].eq('#') ) {
					r = match[r0.$ar];
					x=rc.x(), y=r.y();
					h=r.height()/2;
				}
				if( info[${b}.city].eq('#') ) {
					r = match[r0.$br];
					not( h ) {
						x=rc.x(), y=rc.y();
					}
					h+=r.height()/2;
					h++;
				}				
			}
			if( r ) {
				h+=rc.height(), w=rc.width();
				rc = Class.rect(x,y,w,h);
			}
		}
		
		a = n*2; 
		_setValue(rc.incrH(-1*hh).height(20), data[$a], row); a++;
		_setValue(rc.move('bottom',28), data[$a], row);
		
		if( n<halfSize ) {
			d.rectLine(rc, 234, '#a0a0a0', 2);
		} else {
			d.rectLine(rc, 214, '#a0a0a0', 2);	
		}
	}
}
if( centerRect ) d.fill( centerRect, '#a0a0a0');	}
util_common.webpageReload() {result = pageReload('webpages',	'data\webpages');
d = result.str();
while( d.valid() ) {
	if( d.ch().eq('[') ) {
		c = d.match();
		ty =c.move();
		code = c.findPos('=').right();
		print("ty->$ty, code->$code");
		if( ty.eq('modify') ) {
			print("code=>$code");
			instance('test.was').reload(code);
		}
	} else break;

		if( d.ch().eq(',') ) d.incr();
}

}
util_widget.gridInputCreate(page, index) {input  = {
	tag: input, 
	onKeyDown() {
		not( @key.eq( KEY.Enter, KEY.Return) ) return; 
		w = this[mainWindow];
		this[mainWindow].nextFocus(this, @mode&KEY.ctrl);
	}
	init( main, index) {
		this[mainWindow] = main;
		this[index] = index;
	}
}
input.init( page, index);
return input;
}
util_common.runPageWorker(node) {switch( node[type] ) {
case addr:
 	instance('addr.web').call( node, callback(type, data) {
		switch(type) {
		case read: 
			root = node[root]; 
			not( root ) {
				root = {};
				node[root] = root;
			}
			parseZipData(data.ref(), root);
		case finish: 
			node[run] = false;
			node[page].post(1, node); 
			print("address call finished!!! $node[page]");
		case error:
			node[error] = data;
		}
	});	
case upload:
	print("# upload node = $node");
	instance('upload.web').call(node, callback(type, data) {
	case finish:		node[page].post(11, node); 
	case error:  	node[error] = data;
	});
case download:
	print("# download node = $node");
	instance('download.web').call(node, callback(type, data) {
	case finish:		node[page].post(21, node); 
	case error:  	node[error] = data;
	});
default: break;
}
}
util_class.getLastSync(code) {return instance('config.model').value("select data from conf_info where grp='lastSync' and cd='$code'");}
util_common.upgradeCheck(prev) {
ver=null, etc=null;
Cf.webConnection('http://1.215.224.202:8089/@common.app.version', null, 'get', callback(type, data) {
	switch(type) {
	case read:
		print("data===$data");
		ver = data.findPos('/').trim();
		etc = data.findPos('/').right().trim();
		print("version: $ver, $etc, $prev");
	case finish:
		if( ver ) {
			if( ver.eq(prev) ) {
				if( etc.eq('copyFuncs') ) {
					src = get('global.config');
					dest = instance('pages.model');
					copyFuncs( src, dest, getLastSync('func') );
				} 
				Cf.loadPage('partic.login').open('center');
			} else {
				Cf.loadPage('system.upgrade').open('center');
			}
		} else {
			Cf.loadPage('partic.login').open('center');
		}
	case error:
		print("version check error: $data");
	}
});

return true;}
util_etc._setVal(cf, key, val) {if( typeof(cf,'array') ) {
	while( cur, cf ) {
		if( key.eq(cur) ) return;
	} 
	cf.add(key);
} else if( typeof(cf,'node') ) {
	cf[$key]=val;
}
}
util_etc._getVal(cf, key, val) {not( cf ) return;
data = cf[$key];
if( val ) {
	not( data ) {
		cf[$key] = val;
		return val;
	}
}
return data;}
util_etc._val(&code, data) {_obj=func(&s) {
	obj=null;
	while( n,16 ) {
		key=s.move().trim();
		print("xxxxxxx $key xxxxxxxxxx");
		not( key ) break;
		if( n.eq(0) ) {
			obj=get(key);
		} else {
			not( obj ) {
				print("xxxxxxx $key not define xxxxxxxxxx");
				return null;
			}
			v=obj[$key];
			obj=v;		
		}
		c=s.ch();
		if( c.eq('.') ) {
			s.incr();
		} else {
			break;
		}
	}
	return obj;
}

if( code.find('#') ) {
	key=code.findPos('#');
	obj=get(key);
	not( obj ) return null;
	
	key=code.move();
	ctrl=obj[$key];
	c=code.ch();
	if( c.eq('.') ) {
		code.incr();
		fc=code.move();
		switch(fc) {
		case val: 
			if( data ) ctrl.value(data);
			return ctrl.value();
		default:
			return ctrl[$fc];
		}
		return null;
	} else {
		return ctrl;
	}
}
return _obj(code);
}
util_etc._map(node, &data, flag) {reset=false;
if( flag ) {
	switch( flag) {
	case in:
		not( Cf[mapArray] ) Cf[mapArray]=[];
		arr = Cf[mapArray].reuse();
		field = data.trim();
		while(cur, node) {
			val = cur[$field];
			if( val ) arr.add(val);
		}
		return arr;
	case reset: 		reset=true;
	default: return null;
	}
}

ch  = data.ch();
print("ch====$ch");
if( ch.eq('#') ) {
	
	data.incr();
	code = data.move();
	if( data.ch().eq('(') ) {
		in = data.match();
		not( in ) return null;
		field = in.move().trim();
		op = in.ch();
		print("xxx====$code, $field, $op $data");
		if( op.eq('=') ) {
			in.incr();
			key = in.trim();
			cur = node.findOne(field, key);
		print("xxx====$code, $field, $op, $key, $cur");
			if( cur ) {
				return cur[$code];
			}
		}
	}
	return null;
}
while( data.valid() ) {
	left = data.findPos(',');
	not( left.ch() ) break;
	k=left.findPos('=').trim();
	v=left.trim();
	node[$k] = node[$v];
	if( reset ) node[$v]=null;
	print("_map ========= $k, $v");
}
return node;

}
util_etc._kindCd(db, city) {node = Cf[kindNode];
not( node ) {
	node={};
	Cf[kindNode] = node;
	db.fetchAll("select A.ref1, A.ref3 from comm_tree A, (select code from comm_tree where ref='CC' and ref1='group_cd' ) B where A.pcode=B.code and use_yn='Y'", node);
}
val = _map(node, "#ref3(ref1=$city)");
not( val ) return null;
return val.value(0,1);}
util_draw.nextRect(rects, col) {size = rects.size();
while(n,size,col) {
	if( rects[$n] ) return rects[$n];
}
return null;

}
util_etc._sum(obj, field, sp, ep) {sum=0;
not( sp ) sp=0;
if( typeof(obj,'node') ) {
	not( ep ) ep=obj.childCount();
	while(n, ep, sp ) {
	 	cur=obj.child(n);
	 	sum+=cur[$field];
	}
} else if( typeof(obj,'array') ) {
	while( cur, obj ) {
		if( typeof(cur,'node') ) {
			sum+=cur[$field];
		} else if( typeof(cur,'number') ) {
			sum+=cur;
		}
	}
}

return sum;}
util_common.makeQuery(path) {}
util_etc._query(key) {	not( Cf[queryData] ) Cf[queryData] = {};
	node = Cf[queryData];
	not( key ) {
		return node;
	}
	id=key;
	id.add("Query");
	print("##### Query ID: $id #####");
 	return node[$id];}
util_common.makeQueryFile(path) {node = {}, arr=[];
not( path.find('.') ) {
	fileName = path;
	path = "data/info/${fileName}.src";	
}
line=null;
d=instance('test.file').readAll(path).str();
while( d.valid() ) {
	left = d.findPos('## ',1);
	if( left && line ) {
		line.str();
		code = line.move().trim();
		ch = line.ch();
		if( ch.eq(':','/') ) {
			while( 8 ) {
				sub = line.incr().move();
				code.add(ch,sub);
				ch = line.ch();
				not( ch.eq(':','/') ) break;
			}
		} 
		not( code ) break;
		sql= left.trim();
		node[$code] =sql.encode();
		arr.add(code);
	}
	line = d.findPos("\n").trim();
}
sql='';
while( code, arr) {
	if( sql ) sql.add("\r\n");
	sql.add("## $code\r\n$node[$code]"); 
}
instance('test.file').writeAll("data/info/${fileName}.sql", sql);
loadQuery(path);
node.delete(), arr.delete();}
util_common.trimTab(&str) {val='';
while(str.valid() ) {
	line=str.findPos("\n"); 
	not( line.ch() ) continue; 
 	if( val ) val.add("\n");
 	val.add(line.trim());
}
return val;}
util_draw.randomColor(a) {r=System.rand(256);
g=System.rand(256);
b=System.rand(256);
not( isset(a) ) a=256;
return Class.color(r,g,b,a);}
util_etc._replace(&data, src, dest) {str='', n=0;
while( data.valid() ) {
	if( n ) str.add(dest);
	str.add( data.findPos(src).trim() );
	n++;
}
return str;}
util_etc._find(obj, val, sub) {if( typeof(obj,'array') ) {
	not(val ) return -1;
	while( v, obj, n, 0) {
		if( val.eq(v) ) return n;
	}
	return -1;
} else if( typeof(obj,'node') ) {
	if( sub ) {
		while( c, obj, n, 0 ) {
			if( c[$val].eq(sub) ) return n;
		}
	} 
	return -1;
}
}
util_draw._treeIcon(d,rc,node,over) {	r=rc.x(0,true);
	if( d.state(STYLE.Selected) ) {
		d.fill(r, '#f0f0f0' );
		if( over ) d.rectLine(r,24,'#c0c0c0');
	} else if( over ) {
		d.fill(r, '#dfeffa' );
		d.rectLine(r,24,'#afbfef');
	}
	not(node ) return;
	rcIcon = rc.width(16); 
	cnt = node.childCount();
	ok = when(cnt, true);
	
	if( node[type].eq('gender') ) {
		if( cnt.eq(1) ) {
			d.image( rcIcon.center(14,16).incrY(2), Image.vicon.bullet_black );
			ok= false;
		} else if( cnt.eq(0) ) {
			ok= false;
		}
	}
	if( ok ) {
		if( d.state(STYLE.Open) ) {
			d.image( rcIcon.center(14,16).incrY(2), Image.tree.plus );
		} else {
			d.image( rcIcon.center(14,16).incrY(2), Image.tree.minus );			
		}		
	}
	return rcIcon.width();
}
util_draw._modifyMark(d,rc, arr) {not( arr ) return;
arr.reuse();
x=rc.right()-8, y=rc.y();
sp=Class.point(x,y);
arr.add(sp);
arr.add(rc.rt()); y+=8;
arr.add(Class.point(rc.right(),y));
arr.add(sp);
d.polygon(arr,'fill','red');
}
util_common._parentNode(node, code, type) {not( code ) code='type';
not( type ) type='root';
p=node;
while( p ) {
	if( p[$code].eq(type) ) return p;
	p=p.parent();
}
return null;
 }
util_widget.treeExpandAll(tree, root, expand) {not( root ) root=tree.rootNode();
while( cur, root ) {
	if( cur.childCount() ) {
		tree.expand(cur,expand);
		treeExpandAll(tree, cur, expand);
	}
}}
util_class.newClass(code, arr) {fc=Cf.class(code);
not( typeof(fc,'class') ) {
	include(code);
}
not( typeof(arr,'array') ) arr=args();
return Cf.class(code, arr); 
}
util_etc._err(type, msg) {conf("lastError.${type}", msg, true);

print("##### lastError.${type} => $msg");}
util_class._runClass(class, &s, node) {note='';
print("_runClass : $class");
while(s.valid() ) {
	ch=s.ch();
	not( ch ) break;
	if( ch.eq('/') ) {
		c1=s.ch(1);
		if( c1.eq('/') ) {
			s.incr(2);
			note.add(s.findPos("\n"));
		} else if( c1.eq('*') ) {
			note.add(s.match());
		}
		continue;
	}
	s.ch();
	funcNm=s.move();
	ch=s.ch();
	not( ch.eq('(') ) {
		_err("ClassSaveAll", "$funcNm 함수 매개변수 매치에러");
		break;
	}
	param=s.match();
	ch=s.ch();
	not( ch.eq('{') ) {
		_err("ClassSaveAll", "$funcNm 함수 본문 시작에러");
		break;
	}
	body=s.match(1);
	not( body ) {
		_err("ClassSaveAll", "$funcNm 함수 본문 내용없음");
	} 
	note='';
	print("class.function('$funcNm', callback($param){ $body })");	 
	Cf.call("class.function('$funcNm', callback($param){ $body })");	 
}	}
util_etc._nodeVal(node, &data) {		while( data.valid() ) {
			k=data.move();
			not( data.ch().eq(':') ) {
				break;
			}
			data.incr();
			data.ch();
			if( data.start('<s>', '<text>') ) {
				if( data.start('<s>') ) {
					v=data.match('<s>','</s>');
				} else {
					v=data.match('<text>','</text>');
				}
				if( data.ch().eq(',') ) data.incr();
			} else if( data.ch().eq() ) {
				v=data.match();
				if( data.ch().eq(',') ) data.incr();
			} else {
				v=data.findPos(',').trim();
			}
			node[$k]=v;
		}		
}
util_draw.commDrawTreeIcon(field) {	r=rc.x(0,true);
	if( draw.state(STYLE.Selected) ) {
		draw.fill(r, '#f0f0f0' );
		if( over ) draw.rectLine(r,24,'#c0c0c0');
	} else if( over ) {
		draw.fill(r, '#dfeffa' );
		draw.rectLine(r,24,'#afbfef');
	}
	
	if( node[type].eq(field) ) {
		@rc.incrX(12);
	} else {
		rcIcon = rc.width(16); 
		if( tree.is('child', node) ) {
			if( draw.state(STYLE.Open) ) {
				draw.image( rcIcon.center(15,16).incrY(2), "tree.plus" );
			} else {
				draw.image( rcIcon.center(15,16).incrY(2), "tree.minus" );			
			}
		} else {
			draw.image( rcIcon.center(15,16).incrY(2), "ficon.ui-panel");
		}
		@rc.incrX(18);
	} }
util_common.commArr(code, reuse) {not( code ) {
	code='commArr';
}
arr=Cf[$code];
not( arr ) {
	arr=[];
	Cf[$code]=arr;
}
if( reuse ) {
	arr.reuse();
}
return arr;}
util_common.commNode(code, reuse) {not( code ) {
	code='commNode';
}
node=Cf[$code];
not( node ) {
	node={};
	Cf[$code]=arr;
}
if( reuse ) {
	node.initNode();
}
return node;}
util_common.util_wonComma(won) { 
	if( typeof(won,'number') ) {
		num=won;
		won="$num";
	} else {
		not( won.isNum() ) return won;
	}
	s='', sign='';
	ch=won.ch();
	if( ch.eq('-','+') ) {
		sign=ch;
		w=won.value(1);
	} else {
		w=won;
	}
	size=w.size();
	sp= size % 3;
	if( sp ) {
		s.add( w.value(0,sp) );
		size-=sp;
	}
	while( 8 ) {
		if( size<=0 ) break;
		if( sp ) s.add(',');	
		ep=sp+3;
		s.add( w.value(sp,ep) );
		sp=ep;
		size-=3;
	}
	return "${sign}${s}";	 }
util_class.setStaticClass(code) {classNm = "static_${code}";
if( code.eq('code','file','db','parse') ) classNm.add('Class');

print("setStaticClass => $classNm");
buf=instance('my.file').readAll("data/classes/${classNm}.class");

not( buf ) {
	print("## setStaticClass error : file not found => data/classes/${classNm}.class");
	return;
}

db=instance('pages.model');
saveClassFile(db, buf.ref(), 'static');

print("setStaticClass ok!!!");
}
util_class.commonClass(code, reload, setClass) {not( Cf[classData] ) Cf[classData]={};
cf = Cf[classData]; 
if( reload ) {
	cf[$code]=null;
	if( setClass ) setStaticClass(code);
	loadClassByDb(code);
}
if( cf[$code] ) return cf[$code];
fc=Cf.class(code);
not( typeof(fc,'class'), or(reload) ) {
	loadClassByDb(classCd);
	fc=Cf.class(classCd);
}
cf[$code] = fc();
return cf[$code];
}
util_class.classLoad(code, setClass) {if( setClass ) {
	if( code.find('.') ) {
		group=code.find('.').trim();
		name=code.find('.').right().trim();
		print("setClass -> data/classes/${group}_${name}.class");
		src=class('file').readAll("data/classes/${group}_${name}.class");
		if( src ) {
			arr=class('util').arr();
			saveClassFile( instance('pages.model'), src.ref(), group, arr);
			print("load classes arr: $arr");
			while( classNm, arr ) {
				classCode="${group}.${classNm}";
				Cf.reloadClass(classCode);
				loadClassByDb(classCode);
			}		
		} else {
			print("class load error !!!");
		}
	}
} else {
	loadClassByDb(code);
	print("load class code: $code");
}
}
util_class.pageLoad(pcode, reload) {
	db = instance('pages.model');
	root=_node('pageLoadData');

	if( typeof(pcode,"page") ) {
		args(1,pageCode, reload);
		pageGroup=pcode[@cms.code];
		print("@@@@@ ${pageGroup}.${pageCode} @@@@@@@@@");
		return Cf.loadPage("${pageGroup}.${pageCode}", reload);
	} else {
		root[pageGroup]	=pcode.find('.').trim();
		root[pageCode]	=pcode.find('.').right().trim();
		args(1,reload);
	}

	tm=db.value("select max(tm) as tm from epstreestep where tree_idx in (
	  select idx from epstreelist where pidx = (
	  select idx from epstreelist where ref='$pcode')
	)");

	not( tm ) {
		return Cf.loadPage(pcode, reload);
	}

	not( Cf[tm#$pcode] ) {
		pageTm=db.value("select max(tm) from pageFunc where cmsCode=#{pageGroup} and pageCode=#{pageCode}", root);
		if( pageTm<tm ) {
			Cf[tm#$pcode]=pageTm;
		}
	}

	if( Cf[tm#$pcode], not(reload) ) {
		if( Cf[tm#$pcode].eq(tm) ) {
			print("$pageCode page reload already load !!!");
		} else {
			prevTm=Cf[tm#$pcode];
			root.removeAll();
			root[ref]=pcode;
			print("$pageCode page reload [prev=$prevTm, tm=$tm]" );
			db.fetch("select idx from epstreelist where ref=#{ref}",root);
			db.fetchAll("select idx from epstreelist where pidx=#{idx}",root);
			all=true, layout=false;
			src="";
			while( node, root, n, 0 ) {
				db.fetch("select case when length(data)=0 then src else data end as data, tm from epstreestep where tree_idx=#{idx}", node);
				if( node[tm]>prevTm ) {		
					src.add(node[data],"\n");
					if( n.eq(0) ) layout=true;
				} else {
					all=false;
				}
			}
			node[modifyDate] 	=tm; 
			if( all ) {
				root[func]="${root[pageCode]}: { $src }";	
				savePageInfo(root, true);
			} else {
				if( layout) {
					root[func]="${root[pageCode]}: { $src }";	
				} else {
					root[func]="${root[pageCode]}: { layout: skip, $src }";
				}
				savePageInfo(root, false);
			}
			reload=true;
		}
	} 
	Cf[tm#$pcode]=tm;

	print("pageLoad : $pcode");
	return Cf.loadPage(pcode, reload);
}
util_common.nodeVal(&str) {rst='';

while( n,20 ) {
	key=str.move();
	ch=str.ch();
	print("#1. key=$key");
	if( ch.eq(',') ) {
		rst.add("node[$key]=",key,";\n");
		str.incr();
		continue;
	}
	not( ch ) {
		rst.add("node[$key]=",key,";\n");
		break;
	}
	not( ch.eq(':') ) {
		break;
	}
	ch=str.incr().ch();
	if( ch.eq() ) {
		rst.add("node[$key]=",ch,str.match(),ch,";\n");
		if( str.ch().eq(',') ) {
			str.incr();
			continue;
		} else {
			break;
		}
	} 
	
	if( ch.eq('<') )  {
		sp=str.cur();
		tag=str.incr().move();
		str.pos(sp);
		in=str.match("<$tag","</tag>");
		in.findPos(">");
		rst.add("node[$key]=template($tag){",in,"};\n");
	} else {
		sp=str.cur();
		var=str.move();
		ch=str.ch();
		if( ch.eq(',') || not(ch) ) {
			rst.add("node[$key]=",var,";\n");
			str.incr();
			continue;
		}
		if( ch.eq('[') ) {
			str.match();	
			ch=str.ch();
		} else if( ch.eq('(') ) {
			while( ch.eq('(') ) {
				str.match(1);
				ch=str.ch();
				not( ch.eq('.') ) break;
				str.incr().move();
				ch=str.ch();
			}
		}
		if( ch.eq('.') ) {
			while( ch.eq('.') ) {
				x=str.incr().move();
				ch=str.ch();
			}
			if( ch.eq('(') ) {
				while( ch.eq('(') ) {
					str.match(1);
					ch=str.ch();
					not( ch.eq('.') ) break;
					x=str.incr().move();
					ch=str.ch();
				}
				ch=str.ch();
			}
		} 
		if( ch && ch.isOper() ) {
			str.findPos(s2,1,1);
			ep=findSrcEndPos(str);
			rst.add("__v=", str.value(sp,ep,true),";\nnode[$key]=__v;\n");
			str.pos(ep);
		} else {
			ep=str.cur();
			rst.add("node[$key]=", str.value(sp,ep,true),";\n"); 
		}
		not( str.ch().eq(',') ) break;
		str.incr();
	}
}
print("rst=======>\n$rst");
return rst;
}
util_common.str_format(&str) {	arr=args();
	rst='';
	while( str.valid() ) {
		rst.add( str.findPos('@') );
		ch=str.ch();
		not( ch ) break;
		if( ch.eq('{') ) {
			in=str.match().trim();
			idx=in+1;
			rst.add( arr[$idx] );
		} else {
			rst.add('@');
		}
	}
	return rst;
}
util_common.findSrcEndPos(str) {	while( str.valid() ) {
		ch=str.ch();
		if( ch.eq(',') || not(ch) ) break;
		if( ch.eq('(') ) {
			str.match(1);
			continue;
		}
		if( ch.isOper() ) {
			str.incr();
			continue;
		}
		str.move();
		ch=str.ch();
		if( ch.eq('[') ) {
			str.match();	
			ch=str.ch();
		} else if( ch.eq('(') ) {
			while( ch.eq('(') ) {
				str.match(1);
				not( str.ch().eq('.') ) break;
				str.incr().move();
				ch=str.ch();
			}
		} 
		if( ch.eq('.') ) {
			while( ch.eq('.') ) {
				str.incr().move();
				ch=str.ch();
			}
			if( ch.eq('(') ) {
				while( ch.eq('(') ) {
					str.match(1);
					not( str.ch().eq('.') ) break;
					x=str.incr().move();
					ch=str.ch();
				}
			}
		} 		
	}
	return str.cur();}
util_common.makeStepData(&src) {ok=false;
if( src.find('>>',1) ) {
	ok=true;
	src = src.findPos('>>',1);
}
if( src.find('/*',1) ) {
	ok=true;
	src = stripComment(src.ref());
}
if( src.find('// ',1) ) {
	ok=true;
	src = stripLineComment(src.ref());
}

return when( ok, src );
}
util_common.incrTab(&str, tabs, trim) {	rst='', fst=0;
	while( str.valid(), n, 0 ) {
		line = str.findPos("\n");
		indent = indentText(line.ref());
		if( trim ) line.ch();
		sz=indent.size();
		not( rst ) {
			not( line ) continue;
			fst = sz;
		} else {
			rst.add("\n");
		}
		val ='';
		if( fst<sz ) {
			val = indent.value(fst);
		}
		rst.add("$tabs$val$line");
	}
	return rst;	
}
util_common.str_trimRight(&s) { 
	x=-1;
	while( n, 256 ) {
		b=s.ch(x);
		not( b.isBlank() ) break;
		x--;
	}
	x++;
	if( x<0 ) {
		s.incr(x);
	}
	return s; 
}
util_class.setPageClass(classCode, page) {param=args();
if( page.pageImpl ) {
	page.alert("페이지 구현 클래스가 이미 등록되었습니다,  $classCode 등록 실패");
}
include(classCode);
print("xxxxxxxxxxx $classCode xxxxxxxxxx");
cls=newClass(classCode, param );
page.pageImpl=cls;
return cls;}
util_class.makeClassEventFunc(code) {	err='';
	not( code.find('.') ) {
		err="$code is not valid class code";
		return err;
	}
	groupName=code.find('.').trim();
	className=code.find('.').right().trim();
	file=instance('my.file');
	fileName="data/classes/${groupName}_${className}.class";
	not( file.isFile(fileName) ) {
		err="$code not found class file";
		return err;
	}
	node=class('util').node();
	parseClassInit=func(&s, classNm) {
		print("parseClassInit : $classNm");
		c=s.ch();
		not( c.eq('{') ) {
			@err="$code not match class braket";
			return false;
		}
		s.incr();
		fnm=s.move().trim();
		not( classNm.eq(fnm) ) {
			@err="$classNm start func errror [start function: $fnm]";
			return false;
		}	
		not( s.ch().eq('(') ) {
			@err="$classNm is not function";
			return false;
		}
		s.match();
		not( s.ch().eq('{') ) {
			@err="$classNm braket start error";
			return false;
		}
		body=s.match();
		sp=s.cur();
		while( true ) {
			left=body.findPos(".eventMap(");
			not( body.valid() ) break;
			
			line=left.findLast("\n").right();
			print("line======> $line");
			not( line ) line=left;
			var=line.trim();
			/* event map 매개변수 처리해서 노드추가*/	
			args=body.findPos(")");	
			event=args.move().trim();
			not( args.ch().eq(',') ) {
				@err.append("$var $event comma error\n");
				continue;
			}
			args.incr();
			fc=args.findPos(',').trim();
			param=when( args.valid(), args.trim() );
			print("args=$args ($fc, $param )");
			cur=node.addNode().val( varNm:var, eventNm:event, eventFunc: fc, eventParam: param );
			print("@@ cur===> $cur");
		}
		return sp;		
	};
	makeClassSrc=func(&s) { 
		while( s.valid() ) {
			c=s.ch();
			if( c.eq('/') ) {
				if( s.ch(1).eq('/') ) {
					s.findPos("\n");
				} else if( s.ch(1).eq('*') ) {
					s.match();
				} else {
					break;
				}
				continue;
			}
		}
		classNm=s.move().trim();
		sp=parseClassInit(s, classNm);
		not( sp ) break;
		not( node.childCount() ) {
			@err="$className 에 추가할 이벤트 함수가 없습니다";
			return;
		}
		
		src=s.value(0,sp,true);
		s.pos(sp);
		/* event map 함수 추가 */
		ok=false;
		date=System.date('yyyy-MM-dd');
		src.add("\r\n\r\n/* ================= $classNm 이벤트 함수 시작 =  생성일[$date] ================= */\r\n");
		findFunc=func(&s, func) {
			while(s.valid() ) {
				s.findPos(func);
				if( s.ch().eq('(') ) {
					s.match();
					if( s.ch().eq('{') ) return true;
				}
			}
			return false;
		};
		while( cur, node) {
			var=cur[eventFunc].move();
			if( var.eq('this') ) {
				eventFunc= cur[eventFunc].find('.').right().trim(); // var.right(1).trim();
				param=when( cur[eventParam], cur[eventParam].match() );
				not( findFunc(s, eventFunc) ) {
					ok=true;
					src.add("\r\n/* [이벤트맵] ${cur[varNm]}.$cur[eventNm] */");
					src.add("\r\n\tprivate ${eventFunc}($param) {\r\n\t\t\r\n\t}");
				}
			}
		}
		src.add("\r\n/* ================= $classNm 이벤트 함수 종료 ================= */\r\n");
		not(ok ) {
			@err="$className 이벤트가 이미 추가되었습니다";
			return;
		}
		src.add(s);
		/* 이전파일 백업 */
		name=fileName.findLast('.');
		ext=name.right();
		while(n,100) {
			backupFile="name ($n).$ext";
			if( file.isFile(backupFile) ) {
				file.copy(fileName, backupFile);
				break;
			}
		}
		file.writeAll(fileName, src);		
	};
	makeClassSrc( file.readAll(fileName) );
	return err;}
util_common.firstCommentSkip(&s) {	while(s.valid() ) {
		if( s.ch().eq('/') ) {
			if( s.ch(1).eq('/*') ) {
				s.match();
			} else if( s.ch(1).eq('/') ) {
				s.findPos("\n");
			} else {
				break;
			}
			continue;
		} 
		break;
	}
	return s.cur();}
util_common.tr(&s) {
	not(s) return '';
	not( s.find('[#]') ) {
		s=conf(s);
		not( s ) return '';
		not( s.find('[#]') ) return s;
		s.str();		
	}
	
	rst='';
	arr=args();
	idx=1;
	while( s.valid() ) {
		rst.add( s.findPos('[#]') );
		rst.add( arr[$idx] );
		idx++;
	}
	return rst;

}
util_common.@tr(s, def) {/*
	arr=args();
	rst='';
	idx=1;
	while( s.valid() ) {
		rst.add( s.findPos('[#]') );
		rst.add( arr[$idx] );
		idx++;
	}
	return rst;
*/
	if( def ) return def;
	return s;}
util_widget.gridHeaderWidth(grid) {
	fields=grid.fields();
	if( fields[rate], fields[widths] ) {
		wid=grid.rect().width();
		if( grid.is('vScrollVisible') ) {
			wid-=40; 
		} else {
			wid-=8;
		}
		totalWidth=wid;
		arr=[];
		wa=fields[widths].reuse();
		wideNum=0;
		while( val, fields[rate] ) {
			if( val.eq('*') || not(val) ) {
				wideNum++;
				wa.add(9999);
				continue;
			}
			if( val.find('px') ) {
				w=val.find('px').trim();
				wid-=w;
				wa.add(w);
			} else if( val.find('%') ) {
				a=val.find('%').trim();
				w=totalWidth * a;
				w/=100;
				wid-=w;
				wa.add(w);
			} else {
				arr.add(val);
				wa.add(0);
			}
		}
		remain= totalWidth - wid;
		ww=50;
		if( wideNum ) {
			if( remain>0 ) {
				ww=remain/wideNum;
			}
		}
		arr.recalc(wid);
		widx=0;
		while( w, wa, n, 0 ) {
			switch( w ) {
			case 0: 
				w=arr[$widx];
				wa[$n]=when(w, w, 50);
				widx++;
			case 9999: 
				wa[$n]=ww;
			} 
		}
		wa.recalc(totalWidth);
		grid.headerWidth( wa );
		return true;
	}
	return false;


}
function.pageDbCreate(node, pageGroup) {db=instance('pages.model');
pageType='pages';
not( pageGroup ) pageGroup=node[pageGroup];
node[modifyDate]=System.localtime();

if( db.count("select count(1) as cnt from page_info where type_code='$pageType' and cms_code='$pageGroup'") ) {
	result.add("[modify page=$pageGroup],");
	db.exec("update page_info set func=#{pageSrc}, src=null, modifyDate=#{modifyDate} where type_code='$pageType' and cms_code='$pageGroup'", node);
} else {
	result.add("[new page=$pageGroup], ");
	db.exec("insert into page_info(type_code, cms_code, func, type, status, useyn, modifyDate) values ('$pageType','$pageGroup',#{pageSrc}, 'P', '0', 'Y',#{modifyDate})", node);
}
Cf.makeCmsTable(dbcode);

 }
util_etc.fileRead(path, flag) {return instance('my.file').readAll(path);}
util_web.help_contentTitle(title, subTitle) {not( subTitle ) return template() {
<div class="whitebg right">
  <p>Free and open source<br><img align="top" border=0 src="images/gplv3.png"></p>
</div>
<h1 style="border-bottom: none;">$title<br><br><br><br></h1>
};

return template() {
<div class="whitebg right">
  <p>Free and open source<br><img align="top" border=0 src="images/gplv3.png"></p>
</div>
<h1 style="border-bottom: none;">[$title] $subTitle<br><br><br><br></h1>
};}
util_web.ux_makeCombo(db,sql,root,field,title) {root.removeAll();
if( sql ) db.fetchAll(sql, root );
s="<select id='select_$field'>";
if( title ) s.add("<option value=''>$title</option>");
while( cur, root ) {
	s.add("<option>$cur[$field]</option>");
}
s.add("</select>");
return s;}
util_web.web_pageNow(curPage, listTotal, rowCount ) {

	not( curPage ) curPage=1;
	pageNum = listTotal/rowCount, mod=listTotal%rowCount;
	if( mod ) pageNum++;
	s="<span class='pageNow'> 페이지 $curPage/$pageNum (전체 $listTotal 건) &nbsp;&nbsp;&nbsp;&nbsp;";
	while( n, pageNum ) {
		page=n+1;
		if( page.eq(curPage) ) {
			s.add("&nbsp;<a style='color:#909090' href='#'>$page</a>");	
		} else {
			href="'javascript:search($page)'";
			s.add("&nbsp;<a href=$href>$page</a>");
		}
	}
	s.add("</span>");
	return s; 

}
util_web.ux_makeRadio(title, name) {return template() {
<label class="cursor mR10">
	<input type="radio" name="$name"/>
	<span class="inBlock vm">$title</span>
</label>
};}
util_web.ux_makeButton(name, href, className) {
	return template() {
<button type="button" onClick="$href">$name</button>
	};
 

}
util_web.web_tableField(&s) {fields=class('util').node();
while( s.valid() ) {
	line = s.findPos(";");
	field=line.move().trim();
	if( line.ch().eq('=') ) {
		line.incr();
		cur=fields.addNode();
		fields[$field]=cur;
		line.split().inject(name, width, align);
		cur.put(field, name, width, align);
	}
}
return fields;}
util_web.ux_makeTable() {


	arr=args();
	bBody=false;
	switch( arr.size() ) {
	case 2:
		arr.inject(root, fieldInfo);
	case 3:
		arr.inject(root, fieldInfo, bBody);
	case 4:
		arr.inject(db, sql, root, fieldInfo);
		if( db ) db.fetchAll(sql, root.removeAll() );		
	}
	
	s='';
	
	if( bBody ) {
		arr=fieldInfo.split();
		while( row, root ) {
			s.add('<tr>');
			while( field, arr ) { 
				align='al';
				if( field.find('_type','_cd','_btn') ) {
					align='ac';
				}
				s.add("<td class='$align'>$row[$field]</td>");
				
			}
			s.add('</tr>');
		}
		not( root.childCount() ) {
			cols=arr.size();
			s.add("<tr><td colspan=$cols style='height:50px;'>검색된 결과가 없습니다</td></tr>");
		}	
		return s;
	}
	
	
	fs=web_tableField(fieldInfo.ref());
	s='';
	while( cur, fs) {
		s.add("<col style='width:$cur[width];'/>");
	}
	colGroup=tr('<colgroup>[#]</colgroup>',s);
	
	s='';
	while( cur, fs) {
		s.add("<th scope='col'>$cur[name]</th>");
	}
	thead=tr('<thead><tr>[#]</tr></thead>',s);
	
	s='';
	while( row, root ) {
		s.add('<tr>');
		while( cur, fs) {
			field=cur[field];
			s.add("<td class='$cur[align]'>$row[$field]</td>");
		}
		s.add('</tr>');
	}
	not( root.childCount() ) {
		cols=fs.childCount();
		s.add("<tr><td colspan=$cols style='height:50px;'>검색된 결과가 없습니다</td></tr>");
	}
	
	return tr('<table class="tb collapse">[#][#]<tbody>[#]</tbody></table>', colGroup, thead, s);


}
util_common.findTag(tag, root, all) {
	not( root ) return null;
	
	_find=func(tag, node) {
		while( cur, node ) {
			if( cur[tag].eq(tag) ) return cur;
			find=_find(tag, cur);
			if( find ) return find;
		}		
	};
	if( tag.find('.') ) {
		node=root;
		while( key, tag.split('.') ) {
			node=_find(key, node);
		}
		return node;
	} else {
		return _find(tag, root);
	}

	return null;

}
util_common.util_priceComma(won) { 
	if( typeof(won,'number') ) {
		num=won;
		won="$num";
	}
	not( won.isNum() ) return won;
	
	s='', sign='';
	ch=won.ch();
	if( ch.eq('-','+') ) {
		sign=ch;
		w=won.value(1);
	} else {
		w=won;
	}
	size=w.size();
	sp= size % 3;
	if( sp ) {
		s.add( w.value(0,sp) );
		size-=sp;
	}
	while( 8 ) {
		if( size<=0 ) break;
		if( sp ) s.add(',');	
		ep=sp+3;
		s.add( w.value(sp,ep) );
		sp=ep;
		size-=3;
	}
	return "${sign}${s}";	 }
function.pageFuncCheck(&src, page) {	while(src.valid() ) {
		not( src.ch() ) break;
		sp=src.cur();
		fc=src.move();
		ch=src.ch();
		print(fc, ch);
		if( ch.eq(':') ) {
			src.incr();
			ch=src.ch();
			if( ch.eq() ) {
				src.match();
			} else if( ch.eq('<') ) {
				sp=src.cur();
				src.incr();
				tag=src.move();
				src.pos(sp);
				src.match("<$tag", "</$tag>");
			} else {
				src.findPos(",",1,1);
			}
			if( src.ch().eq(',',';') ) src.incr();
			continue;
		}
		if( ch=='.') {
			src.incr();
			src.move();
			ep=src.cur();
			fc=src.value(sp,ep,true);
			ch=src.ch();
		}
		not( ch.eq('(') ) {
			print("xxxxxxxxxx param $ch xxxxxxxxx");
			break;
		}
		param=src.match();
		ch=src.ch();
		not( ch.eq('{') ) {
			print("xxxxxxxxxx func $ch xxxxxxxxx");
			break;
		}
		body= src.match(1);
		print("${fc} ($param);");
		if( src.ch().eq(',',';') ) src.incr();		
	} 

}
util_widget.comboValue(combo, newVal) {
	val=combo.value(), root=combo.rootNode();
	key=combo[@key];	
	if( newVal ) {
		cur=root.findOne(key, newVal);
		if( cur ) {
			combo.value(newVal);
		} else {
			cur=root.addNode();
			cur[$key]=newVal;
			cur.state(NODE.add, true);
			combo.update();
			combo.value(newVal);				
		}
		return newVal;
	}
		print("1 val==========$val, $root");
	not( val ) {
		val=combo.text();
		print("2 val==========$combo, $val");
		if( val ) {
			if( val.eq(combo[@comboVal]) ) {
				return null;
			}
			cur=root.addNode();
			cur[$key]=val;
			cur.state(NODE.add, true);
		}
	}
	combo[@comboVal]=val;
	return val;
}
util_widget.getParentWidget(page, wid) {
	p= page.parentPage;
	print(p, wid);
	not( p.tag.eq('page','dialog','main') ) {
		while( p ) {
	print(p, wid);
			if( p.tag.eq('page','dialog','main') ) {
				not( wid ) return p;
				widget=p[$wid];
				if( widget ) return widget;
			}
			p=p.parent();
		}		
	}
	not( wid ) return p;
	return p[$wid]; 
}
util_kiosk.findNodeKind(root, tag, kind) {	
	while( cur, root ) {
		if( cur[tag].eq(tag) ) {
			not( kind ) return cur;
			if( cur[kind].eq(kind) ) return cur;
		}
		find=findNodeKind(cur,tag,kind);
		if( find ) return find;
	} 
	return null;}
util_kiosk.findNodeId(root, id) {while( cur, root ) {
	if( cur[id].eq(id) ) return cur;
	find=findNodeId(cur, id);
	if( find ) return find;
}
return null;}
util_kiosk.confStyle(root, style) {
	
	rc=root[rect];
	switch( style ) {
	case vc: 
		args(2, hh, space, leftMargin);
		total=0;
		while( cur, root ) {
			total+=cur[rect].width();
		}
		rc.width(total);
		rcCur=rc.center(total,hh);
		rcCur.incrX(leftMargin);
		sx=rcCur.x(), sy=rcCur.y();
		while( cur, root ) {
			w=cur[rect].width();
			sw=w-space;
			cur[rect]=Class.rect(sx,sy,sw,hh);
			sx+=w;
		}
	default: break;
	}
}
util_kiosk.findPageNode(node) {

	p=node;
	while( p ) {
		if( p[tag].eq('Page', 'Window') ) return p;
		p=p.parent();
	}
	return null;}
util_kiosk.saveClass(class_grp, class_nm, &s, page) {

	root={};
	db=Class.db('pages');
	tm=System.localtime();
	root.put(class_grp, class_nm, tm);
	note='', err='';
	while( s.valid() ) {
		c=s.ch();
		not( c ) break;
		if( c.eq('/') ) {
			if( s.ch(1).eq('/') ) note.add( s.findPos("\n") );
			else if( s.ch(1).eq('*') ) note.add( s.match() );
			continue;
		}
		func=s.move();
		c=s.ch();
		not( c.eq('(') ) {
			err.add("함수 시작오류 : 함수명 : $func");
			break;
		}
		param=s.match().trim();
		c=s.ch();
		not( c.eq('{') ) {
			err.add("함수 매개변수 오류: $param");
			break;
		}
		body=s.match(1);
		root.varMap('class_func: func, class_param: param, note');
		if( body.find('//') || body.find('/*') ) {
			root[class_src]=body;
			root[class_data]=makeSrc(body);			
		} else {
			root[class_src]='';
			root[class_data]=body;			
		}
		if( func.eq(class_nm) ) {
			root[type]='A';
		} else {
			root[type]='F';
		}
		num=db.exec("update class_info set class_src=#{class_src}, class_data=#{class_data}, class_param=#{class_param}, type=#{type}, note=#{note}, tm=#{tm} where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", root);
		not( num ) {
			db.exec("insert into class_info( class_grp, class_nm, class_func, class_param, class_src, class_data, note, type, tm ) values (#{class_grp}, #{class_nm}, #{class_func}, #{class_param}, #{class_src}, #{class_data}, #{note}, #{type}, #{tm})", root);
		}
		note='';
	}
	cnt=db.count("select count(1) as cnt from class_mst where class_grp=#{class_grp} and class_nm=#{class_nm}", root);
	not( cnt ) {
		db.exec("insert into class_mst (class_grp, class_nm, type, tm) values (#{class_grp}, #{class_nm}, 'A', #{tm})", root);
	}
	if( page ) {
		if( err ) page.alert(err);
	} else {
		not( include("${class_grp}.${class_nm}",true) ) {
			err.add("클래스 로딩중 오류가 발생했습니다");
		}	
	}
	root.delete();
	return err.trim();

}
util_kiosk.getRectArray(cur, info) { 
	margin=0;
	aid=null;
	if( typeof(cur,'node') ) {
		rc=cur[rect];
		args(2, margin, aid, type);
		not( type ) type=cur[type];
	} else {
		rc=cur;
		args(2, margin, type, arr);
		not( type ) type='vbox';
	}
	
	tot=0;
	if( typeof(info,'array') ) {
		rate=info;
	} else if( typeof(info,'number') ) {
		rate=class('util').arr();
		while( n, info ) rate.add(4);
	} else {
		rate=info.split();
	}
	if( arr ) {
		arr.reuse();
	} else {
		if( aid ) {
			arr=_arr(cur,aid).reuse();
		} else {
			arr=class('util').arr();
		}	
	}
	
	switch(type) {
	case vbox: getRateArray(rate, rc.height(), arr);
	case hbox: getRateArray(rate, rc.width(), arr);
	}
	if( margin ) {
		margin/=2;
	}

	rc.inject(sx, sy, w, h);
	last=arr.size()-1;
	while( a, arr, n, 0 ) {
		switch(type) {
		case vbox: 
			if( margin ) {
				cy=sy, ca=a;
				if( n ) cy+=margin;
				not( n.eq(last) ) {
					if( n ) {
						ca-=margin * 2;
					} else {
						ca-=margin;
					}
				}
				arr[$n]=Class.rect(sx,cy,w,ca);
			} else {
				arr[$n]=Class.rect(sx,sy,w,a);
			}
			sy+=a;
		case hbox: 
			if( margin ) {
				cx=sx, ca=a;
				not( n.eq(last) ) {
					if( n ) {
						ca-=margin * 2;
					} else {
						ca-=margin;
					}					
				}
				if( n ) cx+=margin;
				arr[$n]=Class.rect(cx,sy,ca,h);
			} else {
				arr[$n]=Class.rect(sx,sy,a,h);
			}
			sx+=a;
		}	
	}
	return arr;



}
util_kiosk.drawNodeButton(draw, btn, sty, page ) {


	type='n';
	if( btn[disable] ) {
		type='d';
	}
	_img=func(btn) {
		img=btn[src$type];
		not( img ) {
			if( btn[src].find('[#]') ) {
				path= tr(btn[src],type);		
				img=Cf.imageLoad(path, true );
				btn[src$type]=img;
			} else {
				path=btn[src];
				img=Cf.imageLoad(path, true );
				btn[src$type]=img;
			}
		}
		return img;
	};
 
	rcButton=btn[rcButton];
	if( rcButton ) {
		if( page && rcButton.eq(page[mouseDownRect]) ) {
			type='p';
		}
		img=_img(btn);
	} else {
		rc=btn[rect];
		img=_img(btn);
		rcButton=img.center(rc);
		switch( btn[style] ) {
		case top:		rcButton.y(rc.y());
		case left:		rcButton.x(rc.x());
		case fixed:	
			img.imageSize().inject(w,h);
		case right:	
			w=rc.width();
			x=rc.right()-rc.width();
			rcButton.x(x);
		case bottom:
			h=rc.height();
			y=rc.bottom()-rc.height();
			rcButton.y(y);
		}
		if( page && rcButton.eq(page[mouseDownRect]) ) {
			type='p';
			img=_img(btn);
		}
		btn[rcButton]=rcButton;
	}

	not( rcButton ) {
		print("drawNodeButton image rect not define : $btn, $sty");
		return;
	} 
	draw.drawImage(rcButton, img);
	
	if( btn[text] ) {
		if( btn[TextMargin] ) {
			rcText=setMarginRect(btn[rect], btn, 'TextMargin');
		} else {
			rcText=btn[rect];
		}
		fonts=btn[Font];
		not(fonts ) {
			if( typeof(sty,'node') ) {
				fonts=sty[Font];
			}
		}
		if( typeof(fonts,'array') ) {
			drawNodeText(draw, rcText, btn[text], "center", fonts, page);		
		} else {
			drawNodeText(draw, rcText, btn[text], "center", sty);		
		}
	}






}
util_common.abs(n) {
	if( n<0 ) return -1 * n;
	return n;
}
util_kiosk.drawNodeStyle(draw, cur) {
	if( cur[Background] ) {
		draw.fill(cur[rect], cur[Background]);
	}
	if( cur[BackgroundImage] ) {
		img=cur[background];
		not( img ) {
			img=Cf.imageLoad(cur[BackgroundImage], true);
			cur[background]=img;
		}
		not( img ) return;
		draw.drawImage(cur[rect], img );
	}
}
util_kiosk.drawNodeImage(draw, var ) {


	if( typeof(var,'rect') ) {
		rcImage=var;
		args(2, root, key, type, mode);
		val=root[$key];
		if( val.find('[#]') ) {
			not( type ) type='n';
			img=root[$key$type];
			not( img ) {
				if( type.eq("default") ) {
					path=tr( root[$key], '');
				} else {
					path=tr( root[$key], type);
				}
				img=Cf.imageLoad(path, true );
				root[$key$type]=img; 
			}
		} else {
			if( type ) {
				path=type;
				img=root[@$key];
				not( img ) {
					img=Cf.imageLoad(path, true );
					root[@$key]=img;
					
				}				
			} else {
				img=root[@$key];
				not( img ) {
					path=root[$key];
					img=Cf.imageLoad(path, true );
					root[@$key]=img;
					
				}
			}
		}
		if( mode ) {
			rc=img.center(rcImage);
			draw.drawImage(rc, img);
			return rc;
		} else {
			draw.drawImage(rcImage, img);
		}
		
	} else if( typeof(var,'node') ) {
		args(2, box, page);
		btn=var;
		_img=func(btn) {
			img=btn[img$type];
			not( img ) {
				path= tr(btn[src],type);
				img=Cf.imageLoad(path, true );
				btn[img$type]=img;
			}
			return img;
		};
		type='n';
		if( btn[disable] ) {
			type='d';
		} 
		rcImage=btn[imageRect];
		not( rcImage ) {
			rc=btn[rect];
			size=btn[size];
			not( size ) {
				size=box[ButtonSize];
			}
			if( size ) {
				size.inject(a,b);
				w=page.rate(a), h=page.rate(b);
				rcImage=rc.center(w,h);
			} else {
				rcImage=_img(btn).center(rc);
			}
			btn[imageRect]=rcImage;
		}
		not( rcImage ) {
			print("drawNodeImage rect not define : $key, $type");
			return;
		}
		if( rcImage.eq(page[mouseDownRect]) ) {
			type='p';
		}
		draw.drawImage(rcImage, _img(btn));	
	} 
	return null;






}
util_kiosk.drawButtons(draw, cf, names, rates, totalWidth) {
		arr=class('util').split(names);
		not( cf.buttonRect ) {
			not( totalWidth ) {
				totalWidth=90*arr.size();
			}
			rcBtn = rc.move('end', totalWidth);
			totalWidth-=14;
			cf.buttonRect = rcBtn.move('bottom',40).center(totalWidth, 30);
		}
		not( rates ) {
			if( cf.buttonRate ) {
				rates=cf.buttonRate;
			} else {
				a=_arr(cf,'buttonRate').reuse();
				while( n, arr.size() ) {
					a.add(4);
				}
				rates=cf.buttonRate;
			}
		}
		rects=getRectArray(cf.buttonRect,rates,5,'hbox');
		while( rc, rects, n, 0 ) { 
			draw.ctrl('btn', rc, arr[$n]); 
		}
	}
util_kiosk.clickButtons(pos, cf) {
		while( rc, getRectArray(cf.buttonRect,cf.buttonRate,5,'hbox'), n, 0 ) {
			if( rc.contains(pos) ) {
				return n;
			}
		}
		return -1;
	}
util_kiosk.timelineCallCheck(tag) {	if( tag.timelineTick ) {
		dist=System.tick() - tag.timelineTick;
		if( dist<100 ) {
			return false;
		} 
	} 
	tag.timelineTick= System.tick();
	return true;}
util_kiosk.setMarginRect(rc, node, attr) {
	not( attr ) attr='Margin';
	not( node[$attr] ) return rc;
	
	rc.inject(x,y,w,h);
	if( typeof(node[$attr],'string') ) {
		arr=[];
		str=node[$attr].ref();
		while( str.valid(), n, 0 ) {
			val=str.findPos(',').trim();
			arr.add(val.toNumber());
		}
		node[$attr]=arr;
	} else {
		arr=node[$attr];
	}
	while( v, arr, n, 0 ) {
		not( v ) continue;
		switch(n) {
		case 0: x+=v, w-=v;
		case 1: y+=v, h-=v;
		case 2: w-=v;
		case 3: h-=v;
		}
	}
	return Class.rect(x,y,w,h);
}
util_kiosk.getDrawObject(node, var, a, b) {
	do= node[$var];
	if( do ) return do;
	not( a ) return null;
 	if( typeof(a,'rect') ) {
		do=Class.draw(a.size());
	} else {
		do=Class.draw(a,b);
	}
	node[$var]=do;
	return do;
}
util_kiosk.popupFadeIn(draw, timeline, style) {if( timeline ) {
	tid=timeline[tid];
	if( tid.eq('FadeInPopup') ) {
		frame=Cf.timeLine("FadeInPopup.current");
		if( Cf.timeLine("FadeInPopup.running") ) {
			opa=30;
			opa+=frame*7;
			
		} else {
			opa=100;
		}
		draw.opacity(opa);
	}
}

/*
draw.mode();
draw.effect(
	DRAW.RoundBox, tag[rect].incr(2), 15, '#cacaca', '#ffffff', 2
);
*/
}
util_kiosk.maxDealNo(db, order) {	order[key_type]='DealNo';
	not( order[key_date] ) {
		order[key_date]=System.date('yyyyMMdd');
	}
	db.fetch( "select max(seq) as seq from tb_key_gen where key_type=#{key_type} and key_date=#{key_date}", order);
	order[seq++];
	db.exec("insert into tb_key_gen (seq, key_type, key_date) values (#{seq}, #{key_type}, #{key_date})", order);
	order[DealNo]=lpad(order[seq],4,'0');
	return order[DealNo];}
util_kiosk.order_completeProcess(db, order, main, items) {
	not( order ) return false;
	not( order[InputCashOk] ) {
		order[error]="입금이 완료되지 않았씁니다. 관리자에 문의하세요");
		return false;
	}
	
	not( items ) {
		cf=main.cf;
		items=cf[ShoppingCart].getOrderList();
	}
	
	main.closePopup();
	main.openPopup('Loading');
	order[error]='';
	
	today			= System.date('yyyyMMdd');
	totalPrice	= cf[ShoppingCart.OrderTotalPrice]; 
	totalQty		= cf[ShoppingCart.OrderTotalQty];
	vat			= totalPrice/10;
	datetime	= System.date('yyyy-MM-dd HH:mm:dd');
	not( cf[OpenDate] ) {
		cf[OpenDate]=today;
	}
	
	order[StoreNo]			= cf[storeCd];
	order[PosNo]				= cf[PosNo];
	order[SaleTmnl] 		= 'K';
	order[SaleType]			= 'O';
	order[SaleDate]			= cf[OpenDate];
	order[DealDate]			= today;
	order[PmtCnt]			= totalQty; 
	order[MenuCnt]			= items.childCount();
	order[NormalAmt]		= totalPrice;
	order[SaleAmt]			= totalPrice; 
	order[TakeOutYn]		= when( cf[OrderSelectType].eq('Hall'), 'N', 'Y' );
	order[VatAmt]			= vat;
 	
	
	order_dealNo(db, order);
	
	
	remain=0;
	price= order[OrderTotalPrice];
	price-=order[InputCashPrice];
	if( price<0 ) {
		remain=abs(price);
		main.qtMonSendData("04,1,$remain");
 		while( n,10 ) {
 			print(" 잔돈배출 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx $n");
 			System.sleep(500);
 			if( kioskDeviceNode[recvData] ) break;
 		}
	}
		
	
	db.fetch("select store_nm, biz_no, tel, addr1 as addr from tb_store_mst where store_no=#{StoreNo}", order);
	order.inject(store_nm, biz_no, tel, addr);
	
	printStr = "$store_nm,$addr,$biz_no,$tel,$datetime,$order[PosNo],$order[DealNo],"; 
	while( sub, items ) {
		sub[sum_price]=sub[qty*sale_price];
		printStr.add("$sub[menu_nm]^$sub[sale_price]^$sub[qty]^$sub[sum_price]\t");
	}
	val=totalPrice-vat;
	printStr.add(",$totalPrice,$val,$vat,$totalPrice,$order[InputCashPrice],$remain");
	s="21,03,14,$printStr";
	
	print("영수증 인쇄 : $s");
	main.qtMonSendData(s.kr() );
 	while( n,10 ) {
 		print(" 영수증 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx $n");
 		System.sleep(500);
 		if( kioskDeviceNode[recvData] ) break;
 	}
 	
 	order_kitchenPrint(order, items, main);
	
	
	
	sql=order_makeQuery('tb_sale_header', tr('sql#kiosk.InsertSale#Header') );
	db.exec(sql, order);
	
	
	order[Seq]=1;
	while( sub, items ) {
		order[MenuCd]		= sub[menu_cd];
		order[Qty]				= sub[qty];
		order[SalePrice]		= sub[sale_price];
		sql=order_makeQuery('tb_sale_detail', tr('sql#kiosk.InsertSale#Detail') );
		db.exec(sql, order);
		order[Seq++];
	}

	
	order[Trdata1]=order[InputCashPrice];
	order[Trdata2]=remain;
	
	sql=order_makeQuery('tb_sale_payment', tr('sql#kiosk.InsertSale#Payment') );
	db.exec(sql, order);
	

	
	if( order[error] ) {
		order[error]="주문처리중 오류가 발생했습니다. 관리자에 문의하세요");
		return false;
	} 
	main.openPopup('CompleteOrder');
	return true;
	



}
util_kiosk.order_makeQuery(table, &s, updateWhere) {
	a='', b='';
	while( s.valid() ) {
		left=s.findPos(',');
		rst='';
		field=left.trim();
		while( left.valid() ) {
			fa=left.findPos('_').trim().lower(); 
			rst.add(fa.upper(1));
		}
		if( updateWhere ) {
			if( updateWhere.find(rst) ) {
				if( b ) b.add(" and ");
				b.add("$field=#{$rst}");
			} else {
				if( a ) a.add(', ');
				a.add("$field=#{$rst}");
			}
		} else {
			if( a ) a.add(",");
			if( b ) b.add(",");
			a.add(field);
			b.add("#{$rst}");
		}
	}
	if( updateWhere ) {
		sql="update $table set $a where $b";
	} else {
		sql="insert into $table ($a) values( $b)";
	}
	return sql;
}
util_web.ux_makeCodeCombo(id, code, title) { 
	s="<select id='$id'>";
	if( title ) s.add("<option value=''>$title</option>");
	while( cur, class('code').getCodeNode(code) ) {
		s.add("<option value='$cur[code]'>$cur[value]</option>");
	}
	s.add("</select>");
	return s;	
}
util_kiosk.order_cashOutError(db, totalOut, currentOut, order, cf, main ) {

 	date = System.date('yyyy-MM-dd HH:mm:ss');
	not( order[DealNo] ) {
		
		order[StoreNo]			= cf[storeCd];
		order[PosNo]				= cf[PosNo];
 		order[SaleDate]			= date;
		
		
		db.fetch( "select max(seq) as seq from tb_key_gen where key_type='DealNo' and key_date=#{SaleDate}", order);
		order[seq++];
		db.exec("insert into tb_key_gen (seq, key_type, key_date) values (#{seq}, 'DealNo', #{SaleDate})", order);
		order[DealNo]=lpad(order[seq],4,'0');
		
		
		seq=order[DealNo].value(2);
		order[ChangeNo]="05$seq";		
	}
	order[StoreNo]			= cf[storeCd];
	order[PosNo]				= cf[PosNo];
	db.fetch("select store_nm, owner_nm, biz_no, tel, addr1 as addr from tb_store_mst where store_no=#{StoreNo}", order);
	order.inject(store_nm, owner_nm, biz_no, tel, addr);
	remain=totalOut - currentOut;
	s='21,09,12,';
	s.add("$order[ChangeNo],$order[DealNo],$store_nm,$owner_nm,$biz_no,$addr,$date,");
	s.add("$totalOut,$currentOut,$remain,$order[PosNo],$tel");
	print("cash out : $order,  $s");
	main.qtMonSendData(s.kr());
 	while( n,20 ) {
 		print(" 현금배출 오류(잔액교환권 출력) xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx $n");
 		System.sleep(500);
 		if( kioskDeviceNode[recvData] ) break;
 	}	

}
util_kiosk.sql_makeQuery(db, table, root, &fieldInfo, updateWhere) {



 	fa='', fb='';
	arr=null;
	if( updateWhere ) {
		arr=updateWhere.split();
	}
	while( fieldInfo.valid() ) {
		left=fieldInfo.findPos(',');
		field=left.trim();
		if( field.find('_yn') ) {
			if( root[$field].eq('true') ) {
				root[$field]='Y';
			} else {
				root[$field]='N';
			}
		}
		if( arr ) {
			find=false;
			while( cur, arr ) {
				if( cur.eq(field) ) {
					find=true;
					break;
				}
			}
			if( find ) {
				if( fb ) fb.add(" and ");
				fb.add("$field=#{$field}");
			} else {
				if( fa ) fa.add(', ');
				fa.add("$field=#{$field}");
			}
		} else {
			if( fa ) fa.add(",");
			if( fb ) fb.add(",");
			fa.add(field);
			fb.add("#{$field}");
		}
	}
	if( updateWhere ) {
		sql="update $table set $fa where $fb";
	} else {
		sql="insert into $table ($fa) values( $fb)";
	}
	print("sql=$sql");
	db.exec(sql, root);
	
	err=db.error();
	if( err ) {
		return "({success:false,error:'$err'})";
	}
	return "({success:true})";




}
util_kiosk.order_kitchenPrintMobile( items, main) {


	s='';
	
	
	_sendPrint=func(s) {
		not(s ) return;
		s.add(",Y,N,N,N,N");
		main.qtMonSendData(s.kr());
	};
	
	time=System.date('HH시 mm분 ss초');
	dealNo='';
	while( cur, items ) {
		not( dealNo.eq(cur[deal_no]) ) {
			if( s ) {
				
				_sendPrint(s);
				s='';
				break;
			}
			dealNo=cur[deal_no];
			orderNo=cf[storeCd];
			orderNo.add(dealNo);
			s='21,10,8,';
			s.add("$orderNo,$time,");
		print("s= $s");
		}
		s.add("$cur[menu_nm]^$cur[qty]\t");
	}
	_sendPrint(s);
	items[DEAL_NO++];
	order_mobildSale(items, main);



}
util_kiosk.drawSubText(draw, rc, text, style, fonts) {
	switch( style ) {
	case MenuSub:
		not( text.ch().eq('(') ) {
			drawNodeText( draw, rc, text, 'center', fonts );
			return;
		}
		sub=text.match().trim();
		name=text.find(')').right().trim();
		print("xxx $name xxx");
		fonts.inject(fontSize, fontColor, fontWeight, fontName);
		not( fontName ) {
			fontName='맑은 고딕';
		}
		draw.save().font(fontSize, fontWeight, fontColor, fontName);
		w=draw.textWidth(name), w+=4;
		rcText=rc.width(w);
		draw.text(rcText, name, 'left');
		draw.font(12, 'normal', '#404090', fontName);
		draw.text(rc.incrX(w), sub, 'left');
		draw.restore();
	default:
		drawNodeText( draw, rc, text, 'center', fonts );
	}

}
util_kiosk.order_mobildSale(items, main) {

	db=Class.db('namzatang_local');
	dbBo=Class.db('tros_server');

	dbBo.fetchAll("select store_no, sale_date, pos_no, deal_no, deal_date, sale_tmnl, sale_type, pmt_cnt, menu_qty, normal_amt, sale_amt
	from TB_SALE_HEADER
	where sale_date=#{SALE_DATE} and deal_no=#{DEAL_NO} ", items.removeAll() );
	while( cur, items ) {
		db.exec("insert into TB_SALE_HEADER( 
			store_no, sale_date, pos_no, deal_no, deal_date, sale_tmnl, sale_type, pmt_cnt, menu_qty, normal_amt, sale_amt
		) values  (
			#{store_no}, #{sale_date}, #{pos_no}, #{deal_no}, #{deal_date}, #{sale_tmnl}, 'M', #{pmt_cnt}, #{menu_qty}, #{normal_amt}, #{sale_amt}
		) ", cur);	
	}
		
	dbBo.fetchAll("select store_no, sale_date, pos_no, deal_no, sale_tmnl, sale_type, 
 	seq, menu_cd, menu_type,  class_cd, tax_yn, sale_price, qty, normal_amt, sale_amt
	from TB_SALE_DETAIL
	where sale_date=#{SALE_DATE} and deal_no=#{DEAL_NO} ", items.removeAll());
	
	while( cur, items ) {
		db.exec("insert into TB_SALE_DETAIL ( 
			store_no, sale_date, pos_no, deal_no, sale_tmnl, sale_type, 
			seq, menu_cd, menu_type,  class_cd, tax_yn, sale_price, qty, normal_amt, sale_amt	
		) values  (
			#{store_no}, #{sale_date}, #{pos_no}, #{deal_no}, #{sale_tmnl}, 'M', 
			#{seq}, #{menu_cd}, #{menu_type}, #{class_cd}, #{tax_yn}, #{sale_price}, #{qty}, #{normal_amt}, #{sale_amt}
		) ", cur);	
	}
	
	dbBo.fetchAll("select store_no, sale_date, pos_no, deal_no, pmt_type, sale_type, arv_dt, pmt_amt, trdata1
		from TB_SALE_PAYMENT
	where sale_date=#{SALE_DATE} and deal_no=#{DEAL_NO} ", items.removeAll());
	
 	while( cur, items ) {
		db.exec("insert into TB_SALE_PAYMENT ( 
			store_no, sale_date, pos_no, deal_no, seq, sale_type, arv_dt, pmt_amt, trdata1
		) values  (
			#{store_no}, #{sale_date}, #{pos_no}, #{deal_no}, '1', 'M', #{arv_dt}, #{pmt_amt}, #{trdata1}
		) ", cur);	
	}

}
my.gridOver(draw, node, over) {
	rc=draw.rect(); 
	if( node.state(NODE.add) ) {
		draw.fill( rc, '#e0e0fa' );
		draw.rectLine(rc, 24, '#f0c0a0');
	} else if( draw.state(STYLE.Selected) ) {
		draw.fill( rc, '#f0f0f0' );
		if( over ) draw.rectLine(rc, 24, '#f0c0a0');
	} else {
		if( over ) draw.fill( rc, '#f0d0f0' );
		else draw.fill();
	} 
	return rc;

}
my.gridModifyMark(draw, rc, color) {
	not( color ) color='#c05060';
	arr=class('util').arr();
	x=rc.right()-8, y=rc.y();
	sp=Class.point(x,y);
	arr.add(sp);
	arr.add(rc.rt()); y+=8;
	arr.add(Class.point(rc.right(),y));
	arr.add(sp);
	draw.polygon(arr,'fill',color);
}
my.getMainPage(page) {
	_getPage=func(widget) {
		p=widget;
		while( p ) {
			if( p.tag.eq('page','dialog','main') ) return p;
			p=p.parent();
		}
		return null;
	}
	pp=when( page.tag.eq('page','dialog','main'), page.parentPage, _getPage(page) );
	not( pp ) return page;
	return getMainPage(pp);
}
my.setFormValue(page, node ) {
	while( w, page.widgets() ) {
		if( w[tag].eq('button', 'label', 'toolbutton') ) continue;
		id=w.id;
		if( node ) {
			w.value( node[$id] );
		} else {
			w.value('');
		}
	}

}
my.getFormValue(page, node ) {
	while( w, page.widgets() ) {
		if( w[tag].eq('button', 'label', 'toolbutton') ) continue;
		id=w.id;
		node[$id]=w.value();
	}

}
my.formValid(page, node, field) {
	while( c, field.split() ) {
		not( node[$c] ) {
			page.alert("$c 는 필수 입력항목입니다");
			page[$c].focus();
			return false;
		}
	}
	return true;
}
my.getQuery(table, &s, updateWhere) {
	a='', b='';
	arr=null;
	if( updateWhere ) {
		arr=updateWhere.split();
	}
	while( s.valid() ) {
		left=s.findPos(',');
		field=left.trim();
		if( arr ) {
			find=false;
			while( cur, arr ) {
				if( cur.eq(field) ) {
					find=true;
					break;
				}
			}
			if( find ) {
				if( b ) b.add(" and ");
				b.add("$field=#{$field}");
			} else {
				if( a ) a.add(', ');
				a.add("$field=#{$field}");
			}
		} else {
			if( a ) a.add(",");
			if( b ) b.add(",");
			a.add(field);
			b.add("#{$field}");
		}
	}
	if( updateWhere ) {
		sql="update $table set $a where $b";
	} else {
		sql="insert into $table ($a) values( $b)";
	}
	return sql;
}
my.getClassInfo(&s, type, root ) {

	not( s ) {
		cls=null;
		if( typeof(type,'node') ) {
			page=type;
			s=Cf.info('funcVar', page, 'init');
			s.str();
			while( s.valid() ) {
				line=s.findPos("\n");
				key=line.findPos("=");
				not( key ) break;
				if( key.ch().eq('@') ) continue;
				type=line.move().trim();
				if( type.eq('class') ) {
					cls=page[$key];
					break;
				}
			}
		}
		return cls;
	}
	_classMap=func() {
		root.removeAll();
		while( s.valid() ) {
			line=s.findPos("\n");
			key=line.findPos("=");
			if( key.ch().eq('@') ) continue;
			type=line.move().trim();
			if( type.eq('class') ) {
				in=line.match();
				cur=root.addNode();
				cur[key]=key;
				cur[value]="$key = $in";
			}
		}
		return root;
	};
	_classString=func( ) {
		s.findPos('class(');
		v=s.findPos(')');
		v.split('.').inject(group, name);
		db=Class.db('pages');
		root={};
		root.put(group, name);
		db.fetchAll("select class_nm, class_func, class_param, case when length(class_src)>0 then class_src else class_data end src 
			from class_info 
			where class_grp=#{group} and class_nm=#{name} 
			order by type", root);	
		rst='';
		while( a, root ) {
			if( a[note] ) {
				rst.add("\r\n/* $a[note] */");
			}
			rst.add("\r\n${a[class_func]}($a[class_param]) {$a[src]}\r\n");
		}	
		root.delete();
		return rst;
	};
	switch( type ) {
	case classMap: 	return _classMap();
	default: 				return _classString();
	}




}
my.getNodeFuncInfo(page, root, parent) {


	root.initNode();
	keys=_arr(root, 'pageKeys');
	arrEvent=_arr(root, 'pageEvent');
	arrCtrlEvent=_arr(root, 'pageCtrlEvent');
	arrUserFunc=_arr(root, 'pageUserFunc');
	while( key, page.keys(keys) ) {
		if( key.ch().eq('@') ) continue;
		a=page[$key];
		not( typeof(a,"function") ) continue;
		param=Cf.funcParam(a);
		if( key.eq('onInit') ) {
			root.addNode({funcName:$key, sort:1});
		} else if( key.start('on') ) {
			arrEvent.add({funcName:$key, funcParam:$param, sort:2}); 
		} else if( key.find('.') ) {
			arrCtrlEvent.add({funcName:$key, funcParam:$param, sort:3}); 
		} else {
			arrUserFunc.add({funcName:$key, funcParam:$param, sort:4});
		} 
	}
	arrEvent.sort('funcName');
	arrCtrlEvent.sort('funcName');
	arrUserFunc.sort('funcName');
	while( cur, arrEvent ) root.addNode(cur);
	while( cur, arrCtrlEvent ) root.addNode(cur);
	while( cur, arrUserFunc ) root.addNode(cur);
	return root;


}
my.treeIcon( tree, draw, node, over, cancel , iconUse) {
	rc = draw.rect().incrX(-20);
	r=rc.x(0,true);
	if( draw.state(STYLE.Selected) ) {
		draw.fill(r, '#f0f0f0' );
		if( over ) draw.rectLine(r,24,'#c0c0c0');
	} else if( over ) {
		draw.fill(r, '#dfeffa' );
		draw.rectLine(r,24,'#afbfef');
	} else {
		draw.fill();
	}
	if( cancel ) {
		rc.incrX(8);
	} else {
		rcIcon = rc.width(16); 
		if( tree.is('child', node) ) {
			if( draw.state(STYLE.Open) ) {
				draw.image( rcIcon.center(15,16).incrY(2), "tree.minus" );
			} else {
				draw.image( rcIcon.center(15,16).incrY(2), "tree.plus" );			
			}
		} else if( iconUse ) {
			draw.image( rcIcon.center(15,16).incrY(2), "ficon.ui-panel");
		}
		rc.incrX(18);
	}
	return rc;
}
my.classReload(&src, classGroup, dataNode, page ) {

	db=Class.db('pages');
	dataNode[tm]=System.localtime();
	dataNode[classGroup]=classGroup;
	fst =true;
	in = src;
	while( in.valid() ) { 
		comment = '';
		ch = in.ch();
		if( ch.eq('/') ) {
			while( ch.eq('/') ) {
				ch = in.ch(1);
				if( ch.eq('/') ) {
					in.incr(2);
					comment.add( in.findPos("\n") );
				} else if( ch.eq('*') ) {
					comment.add( in.match('/*', '*/',1) );
				}
				ch = in.ch();
			}
		}
		w = in.move();
		not( w ) break;
		funcType='F';
		if( w.eq('public', 'private', 'persist','static', 'interface') ) {
			if( w.eq('public') ) {
				funcType='P';
			} else if( w.eq('private') ) {
				funcType='Z';
			} else if( w.eq('interface') ) {
				funcType='I';
			} else {
				funcType='S';
			}
			w = in.move();
		} 
		not( in.ch().eq("(") ) {
 			dataNode[error]="$w 함수 매개변수 매칭오류\n함수 괄호를 확인하세요";
			return false;				
		}
		param = in.match(1).trim(); 
		not( in.ch().eq("{") ) {
  			dataNode[error]="$w 함수 매칭오류\n함수 괄호를 확인하세요";
			return false;
		}
		body = in.match(1);
		if( fst ) { 
			dataNode[classNm]=w;
			dataNode[type] = 'A';
			not( db.exec("update class_mst set tm=#{tm} where class_grp=#{classGroup} and class_nm=#{classNm}", dataNode) ) {
				db.exec("insert into class_mst(class_grp, class_nm, type, tm) values (#{classGroup}, #{classNm}, #{type}, #{tm})", dataNode);
			}
			db.exec("delete from class_info where class_grp=#{classGroup} and class_nm=#{classNm}", dataNode);
			fst=false;
		} else {
			dataNode[type] = funcType;
		}
		if( body.finds('/*','//') ) {
			fsrc = body;
			if( fsrc.find('/*',1) ) {
				fsrc = stripComment(fsrc.ref());
			}
			if( fsrc.find('//',1) ) {
				fsrc = stripLineComment(fsrc.ref());
			}
		} else {
			fsrc=null;
		}
		dataNode.put(w, param, body, fsrc, comment);
		db.exec("insert into class_info(class_grp, class_nm, class_func, class_param, class_data, class_src, note, type, tm) values (#{classGroup}, #{classNm}, #{w}, #{param}, #{body}, #{fsrc}, #{comment}, #{type}, #{tm})", dataNode);
	}
	dataNode.inject(classGroup, classNm);
	not( include("${classGroup}.${classNm}", true) ) {
  		dataNode[error]="클래스 로딩중 오류가 발생했습니다.";
		return false;
	}
	return true;


}
my.getParentFunc(page, funcName) {
	_getPage=func(widget) {
		p=widget;
		while( p ) {
			if( p.tag.eq('page','dialog','main') ) return p;
			p=p.parent();
		}
		return null;
	}
	pp=when( page.tag.eq('page','dialog','main'), page.parentPage, _getPage(page) );
	if( pp ) {
		fc=pp[$funcName];
		if( fc ) {
			return fc;
		}
		return getParentFunc(pp, funcName);
	}
	return null;

}
my.makeSourceIndentText(&s, indent) {
	rst='', fst='';
	left=s.find("\n");
	last=s.ch(-1);
	
	
	sp=left.cur();
	if( left.ch() ) {
		ep=left.cur();
		rst.add("\r\n", indent, left);
		if( sp<ep ) {
			fst=left.value(sp,ep,true);
		}
	} else {
		while( s.valid() ) {
			left=s.findPos("\n");
			sp=left.cur();
			not( left.ch() ) continue;
			ep=left.cur();
			rst.add("\r\n", indent, left);
			if( sp<ep ) {
				fst=left.value(sp,ep,true);
			}
			break;
		}	
	}
	sp=fst.size();
	while( s.valid() ) {
		line=s.findPos("\n");
		not( s.valid() ) {
			val=line;
			not( val.ch() ) {
				break;
			}
		}
		rst.add("\r\n", indent);
		ep=indentText(line).size();
		if( sp<ep ) {
			val=line.value(sp, ep );
			rst.add(val );
		}		
		rst.add( line.trim() );
	}
	rst.add("\r\n");
	return rst;
}
my.getParentObject(page, objectName) {
	_getPage=func(widget) {
		p=widget;
		while( p ) {
			if( p.tag.eq('page','dialog','main') ) return p;
			p=p.parent();
		}
		return null;
	}
	pp=when( page.tag.eq('page','dialog','main'), page.parentPage, _getPage(page) );
	if( pp ) {
		obj=pp[$objectName];
		if( obj ) {
			return obj;
		}
		return getParentObject(pp, objectName);
	}
	return null;

}
my.findQuery(root, &query) {
	arr=args().reuse();
	while( query.valid() ) {
		left=query.findPos(',');
		left.split('=').inject(k,v);
		arr.add( Class.pair(k,v) );
	}
	while( cur, root ) {
		chk=true;
		while( p, arr ) {
			p.inject(k,v);
			if( cur[$k].eq(v) ) continue;
			chk=false;
			break;
		}
		if( chk ) return cur;
	}
	return null;	
}
my.getCommCodeValue(code, key) {
	node=getCommCodeNode(code);
	if( node ) {
		cur=node.findOne('code', key);
		return cur[value];
	}
	return null;
}
my.canvasDraw(fc, draw, cf, timeline ) {
	while( tm, timeline ) {
		not( tm.state(NODE.start) ) continue;
		tid=tm[tid];
		if( Cf.timeLine("${tid}.running") ) { 
			fc(draw, tm);
		} else {
			tm.state(NODE.start, false);
			fc(draw, tm);
		}
	}
	if( cf[stackPage] ) {
		cf[stackPage].draw(draw);
	} else {
		fc(draw, tm);
	}
}
my.canvasMouseDown(fc, pos, cf ) {
	if( cf[stackPage] ) {
		cf[stackPage].mouseDown(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseDown(pos);
		return;
	}
	fc(pos);

}
my.canvasMouseUp(fc, pos, cf) {
	if( cf[stackPage] ) {
		cf[stackPage].mouseUp(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseUp(pos);
		return;
	}
	fc(pos);
}
my.canvasTimelineStart(canvas, timeline, tid, duration, range, mode, target ) {

	tm=timeline.findOne('tid',tid);
	not( tm ) {
		tm=timeline.addNode();
	}
	tm[tid]=tid;
	tm[startTick]=System.tick();
	tm[target]=target;
	tm.put(duration, range, mode);
	tm.state(NODE.start, true);
	Cf.timeLine("${tid}.start", canvas, duration, range, mode);

}
my.canvasTimelineStop(timeline, remove) {
	while( tm, timeline ) {
		key=tm[tid];
		if( Cf.timeLine("${key}.running") ) {
			Cf.timeLine("${key}.stop");
			tm.state(NODE.start, false);
		}
	}
	if( remove ) timeline.removeAll();
}
my.canvasMouseMove(fc, pos, cf) {
	if( cf.mouseDownAction ) {
		_arr(cf,'mouseActionPoints').add(pos);
		return true;
	}
	if( fc ) fc(pos);

}
my.setLogDb(code) {
	db=Class.db(code);
	not( db.open() ) {
		db.open("data/${code}.db");
	}
	not( db.count("select count(1) from sqlite_master where name='error_log'") ) {
		sql="CREATE TABLE error_logs (
  seq integer primary key autoincrement,  
  log_type char(1) default 'A',
  log_message text,
  status INTEGER DEFAULT 0,
  tm INTEGER DEFAULT 0,
  reg_dt datetime default (datetime('now','localtime'))
)";
		db.exec(sql);
	}
}
my.pageObjectDelete(page) {
	print("pageObjectDelete ================= $page ");
}
my.setSetupConfig(cf, projectId, pageCode ) {
	cf.debug=true;
	cf.pageMode='stretch';
	
	cf.projectId 			=projectId;
	cf.pageCode			=pageCode;
	cf.imagePath			="project/${projectId}/images";

}
my.getProjectDb(cf, projectId) {
	not( projectId ) projectId=cf[projectId];
	db=Class.db(projectId);
	not( db.open() ) {
		cf.inject(projectId, storeCode);
		path="project/$projectId/data/${storeCode}.db";
		db.open(path);
	}
	return db;
}
my.test(cf, rc, text) {
	dc=cf[mdc];
	not( d ) {
		dc=Class.draw(500,50);
		cf[mdc]=dc;
	}
	d=cf[test];
	not( d ) {
		d=Class.draw(rc);		
	}
	d.fill();
	d.font(16, 'bold', '#fafac0').text(rc.incrXY(2,2), text, 'left');
	
	dc.fill(rc,'#606060');
	dc.drawImage(rc, d, 'blur', 1);
	dc.font(16, 'bold', '#050505').text(rc, text, 'left');
	return dc;
}
my.drawTextEffect(draw, cur, var, text, style) {


	d=null;
	dc=Cf[DrawDC];
	not( dc ) {
		dc=Class.draw(500,500);
		Cf[DrawDC]=dc;
	}
	
	_draw=func(rc, text, align, fontSize, c1, c2, style, rate) {
		not( style ) style='bold';
		not( rate ) rate=1.5;
		dc.fill();
		rect=Class.rect(0,0,rc.size());
		d=Class.draw(rc);
		cur[draw $var]=d;
		d.fill();
		d.font(fontSize, style, c1).text(rect, text, align);
		dc.drawImage(rect.incrXY(2,2), d, 'blur', rate);
		d.fill().drawImage(rect, dc, 0, 0);
		d.font(fontSize, style, c2).text(rect, text, align);
		return d;
	};
	switch( style) {
	case GridHeader:
		d=cur[draw $var], rc=cur[rect $var];
		not( d ) {
			d=_draw(rc, text, 'center', 16, '#20204a', '#f0f0f0');
		}
 		draw.fill(rc, "#424243");
		if( col.eq(0) ) {
			draw.rectLine(rc,0, '#606062', 4);
		} else {
			draw.rectLine(rc, 234, '#606062', 4);
		}
		draw.drawImage(rc, d);
	case TitleText:
		d=cur[draw $var], rc=cur[rect $var];
		not( d ) {
			d=_draw(rc, text, 'left', 24, '#20204a', '#f0f0f0');
		}
		draw.drawImage(rc, d);
	case ButtonText:
		d=cur[draw $var], rc=cur[rect $var];
		not( d ) {
			d=_draw(rc, text, 'center', 18, '#20204a', '#f0f0f0');
		}
		draw.drawImage(rc, d);
	case FormLabel:
		d=cur[draw $var], rc=cur[rect $var]; 
		not( d ) {
			label="$text :";
			d=_draw(rc, label, 'left', 14, '#909090', '#20202a', 'normal');
		} 		 
 		draw.drawImage(rc, d);
	default:
	}
	return d;




}
my.textWidth(fontSize, text, style, margin) {
	not( style ) style='normal';
	dc=Cf[DrawDC];
	not( dc ) {
		dc=Class.draw(500,500);
		Cf[DrawDC]=dc;
	}
	not( margin ) margin=5;
	dc.font(fontSize, style);
	return dc.textWidth(text)+5;

}
my.makeFormRect( formArray, rect, style ) {
	w=rect.width() - 30;
	rc = rect.center( w, 40);
	rc.inject(x,y,w,h);
	while( sub, formArray ) {
		sub.inject( text, widget, width);
		tw=textWidth(14, text);
		if( widget.eq('check', 'radio') ) {
			not( width ) width=40;
			sub[rect widget]=Class.rect(x,y,width,h), x+=width+5;
			sub[rect label]=Class.rect(x,y,tw,h), x+=tw;
		} else {
			tw+=15;
			not( width ) width=80;
			sub[rect label]=Class.rect(x,y,tw,h), x+=tw;
			sub[rect widget]=Class.rect(x,y,width,h), x+=width;
		}
		x+=45;		
	}
}
my.drawFormArray(draw, formArray, node) {

	if( node[currentWidgetRect] ) {
		draw.rectLine(node[currentWidgetRect], 0, '#d0c0ca90', 4);	
	}
	while( sub, formArray ) {
		sub.inject( text, widget, width);
		if( widget.eq('check', 'radio') ) {
			drawTextEffect(draw, sub, 'label', text, 'FormText');
		} else {
			drawTextEffect(draw, sub, 'label', text, 'FormLabel');
		}
		rc=sub[rect widget].incr(2);
		switch( widget) {
		case input:
			draw.fill(rc,'#f0f0f0');
			draw.rectLine(rc, 12, '#a0a0a0', 2);
			draw.rectLine(rc.incrXY(1,2), 34, '#c0c0c0', 2);
			if( sub[value] ) {
				draw.font(16).text(rc.incrX(4), sub[value]);
			}
		case combo:
			draw.fill(rc,'#f0f0f0');
			draw.rectLine(rc, 12, '#a0a0a0', 2);
			draw.rectLine(rc.incrXY(1,2), 34, '#c0c0c0', 2);
			draw.font(10).text(rc.move('end',20), "▼");
			val=sub[value];
			not( val ) {
				val=sub[title];
			}
			if( val ) {
				draw.font(16).text(rc.incrX(4), val);
			}
		case check:
			draw.rectLine(rc.center(32,32), 0, '#909090');
		case radio:
			draw.pen('#909090').circle(rc.center(32,32) );
		}
	}

}
my.setDrawOpacity(draw, tm) {
	not( tm ) return 0;
	tid=tm[tid];
	frame=Cf.timeLine("${tid}.current");
	if( Cf.timeLine("${tid}.running") ) {
		opa=30;
		opa+=frame*8;
		
	} else {
		opa=100;
	}
	draw.opacity(opa);
}
my.textJoin(arr, sep) {
	not( sep ) sep=',';
	s='';
	while( a, arr ) {
		if( s ) s.add(sep);
		s.add(a);
	}
	return s;
}
my.tagClearRect(root, check, once) {

	p=root.parent();
	if( check &&  p ) {
		if( root[Height#set] ) {
			root[Height]=p[Height];
		}
		if( root[Width#set] ) {
			root[Width]=p[Width];
		}
		w=root[Width], h=root[Height];
		p[rect].inject(x,y);
		root[rect]=Class.rect(x,y,w,h);
	}

	while( cur, root ) {
		if( cur[tag].eq('Popup', 'Content') ) {
			if( cur[tag].eq('Content') ) {
				cur[rect]=null;
				if( cur[Width#set] ) 	cur[Width]=0;
				if( cur[Height#set] )	cur[Height]=0;
			}
			continue;
		}
		if( cur[rect] ) {
			cur[rect]=null;
		}
		if( cur[Width#set] ) cur[Width]=0;
		if( cur[Height#set] ) cur[Height]=0;
		if( once ) break;
		tagClearRect(cur);
	}

}
my.drawButton(draw, cur, text, type, imgId, over) { 
	rc=cur[rect $type];
	if( typeof( over,'rect') ) {
		hover=when(rc.eq(over) , 'p', 'n');
	} else {
		hover=when(over, 'p', 'n');
	}
	if( cur[$imgId] ) {
		img=imageLoad(cur, imgId, hover);
	} else {
		img=commonImage(imgId, hover);
	}
	if( img ) {
		draw.drawImage( img.center(rc), img );	
	}
	drawTextEffect(draw, cur, type, text, 'ButtonText');
}
my._arr(cf, key, reuse) {
	not( cf ) {
		n=Cf[arrIndex++];
		if( n.eq(16) ) {
			Cf[arrIndex]=0;
			n=Cf[arrIndex++];
		}
		arr=Cf[gArrray $n];
		not( arr ) {
			arr=[];
			Cf[gArrray $n]=arr;
		}
		return arr.reuse();
	}
	
	arr = cf[$key];
	not( arr ) {
		arr=[];
		cf[$key]=arr;
	}
	if( reuse ) arr.reuse();
	return arr;
}
my.makeCommConf(id, root, field) {
	field.split().inject(key, val);
	s='';
	not( key) return;
	idx=1;
	while( cur, root ) {
		if( s ) s.add(',');
		a=cur[$key];
		if( val ) {
			b=cur[$val];
			s.add("$a:$b");	
		} else {
			s.add("#$idx:$a"), idx++;
		} 
	}
	conf("cc.${id}", s, true);
}
my.textImage(draw, cur, rc, text, style ) {
	not( rc ) return;
	not( style ) style='bold';
	d=cur[drawObject $style];
	
	not( d ) {
		d=Class.draw(rc);
		cur[drawObject $style]=d;
	}
	
	dc=Cf[DrawDC];
	not( dc ) {
		dc=Class.draw(500,500);
		Cf[DrawDC]=dc;
	}
	
	dc.fill(), d.fill();
	rect=Class.rect(0,0,rc.size());
	
	_draw=func(fontSize, fontStyle, align, c1, c2, rcText, rate) {
		not( rate ) rate=1.5;
		not( rcText ) rcText=rect;
		d.font(fontSize, fontStyle, c1).text(rcText, text, align), dc.drawImage(rect.incrXY(1,2), d, 'blur', rate);
		d.fill().drawImage(rect, dc, 0, 0);
		d.font(fontSize, fontStyle, c2).text(rcText, text, align);
	};
	switch( style ) {
	case menuNormal:
		_draw(12, 'normal', 'center', '#a0a0ca', '#f0f0f0da');	
	case menuSelect:
		_draw(12, 'bold', 'center', '#c0c0ca', '#404046');	
	case submenu: 
		_draw(10, 'normal', 'left', '#d0d0da', '#202020', rect.incrX(26) );	
		if( cur[icon] ) {
			rcIcon =rect.width(20).center(16,16);			
			d.icon(rcIcon, cur[icon]);
		} 
		d.rectLine(rect.incrW(-6), 4, '#a0a0a0', 1, 'dash' ); 
	default:
		_draw(10, 'normal', '#d0d0da', '#202020' );	
	}
	draw.drawImage(rc, d,0,0);
}
my.drawForm(draw, node) {
	if( node[currentWidgetRect] ) {
		draw.rectLine(node[currentWidgetRect], 0, '#d0c0ca90', 4);	
	}
	while( sub, node ) {
		sub.inject( text, widget, width);
		if( widget.eq('check', 'radio') ) {
			drawTextEffect(draw, sub, 'label', text, 'FormText');
		} else {
			drawTextEffect(draw, sub, 'label', text, 'FormLabel');
		}
		rc=sub[rect widget].incr(2);
		switch( widget) {
		case input:
			draw.fill(rc,'#f0f0f0');
			draw.rectLine(rc, 12, '#a0a0a0', 2);
			draw.rectLine(rc.incrXY(1,2), 34, '#c0c0c0', 2);
			if( sub[value] ) {
				draw.font(16).text(rc.incrX(4), sub[value]);
			}
		case combo:
			draw.fill(rc,'#f0f0f0');
			draw.rectLine(rc, 12, '#a0a0a0', 2);
			draw.rectLine(rc.incrXY(1,2), 34, '#c0c0c0', 2);
			draw.font(10).text(rc.move('end',20), "▼");
			val=sub[value];
			not( val ) {
				val=sub[title];
			}
			if( val ) {
				draw.font(16).text(rc.incrX(4), val);
			}
		case check:
			draw.rectLine(rc.center(32,32), 0, '#909090');
		case radio:
			draw.pen('#909090').circle(rc.center(32,32) );
		}
	}


}
my.setNodeSize(root, sizeSet, reload) {

	total=0, arr=_arr();
	p=root.parent();
	not( root[Width] ) {
		root[Width]=nvl( p[rect].width(), p[Width], true);
	}
	not( root[Height] ) {
		root[Height]=nvl( p[rect].height(), p[Height], true);
	}
	if( reload ) {
		root[rect]=null;
	}
	not( root[rect] ) {
		x=0, y=0;
		if( p[rect] ) p[rect].inject(x,y);		
		w=nvl(root[Width]), 64), h=nvl(root[Height]), 32 );
		if( root[Margin] ) {
			str=root[Margin].ref();
			while( str.valid(), n, 0 ) {
				val=str.findPos(',').trim();
				v=_rate(val,0);
				switch(n) {
				case 0: x+=v; 
				case 1: y+=v; 
				case 2: w-=v;
				case 3: h-=v;
				}
			}
		}
		root[rect]=Class.rect(x, y, w, h);
	}
	
	root[rect].inject(sx,sy,sw,sh);
	_proc=func(attr) {

		while( cur, root ) {
			if( cur[tag].eq('Popup') ) continue;
			if( cur[class].eq('layer') ) {
				continue;
			}
			if( sizeSet ) {
				switch( attr ) {
				case Width:
					if( reload && cur[Height#set] ) {
						cur[Height]=0;
					}
					not( cur[Height] ) {
						cur[Height]=sh;
						cur[Height#set]=true;
					}
				case Height:
					if( reload && cur[Width#set] ) {
						cur[Height]=0;
					}
					not( cur[Width] ) {
						cur[Width]=sw;
						cur[Width#set]=true;
					}
				}
			}
			if( reload ) {
				cur[rect]=null;
				if( cur[$attr#set] ) cur[$attr]=0;
			}
			not( cur[$attr] ) {
				arr.add(cur);
				cur[$attr#set]=true;
				continue;
			}
			total+=cur[$attr];
		}

		size=arr.size();
		if( attr.eq('Width') ) {
			remain=sw-total;
		} else {
			remain=sh-total;
		}
		if( remain<0 ) {
			return;
		}
		if( size.eq(1) ) {
			cur=arr[0];
			cur[$attr]=remain;
		} else {
			ca=class('util').arr(), hh=remain/size;
			while(n, size) ca.add(hh);
			ca.recalc(remain);
			while( cur, arr, n, 0 ) cur[$attr]=ca[$n];
		}
	};
	
	type=root[type];
	not( type ) type='vbox';
	switch( type ) {
	case hbox: 
		_proc('Width');
	case vbox:	
		_proc('Height');
	case box: 
		_proc('Width'); 
		_proc('Height');
	case stack: 
		while( cur, root ) {
			not( cur[width] ) 	cur[Width]=root[Width];
			not( cur[Height] ) 	cur[Height]=root[Height];
		}
	}

}
my.getPageXml(projectId, pageId ) {
	id="${projectId}.${pageId}";
	node=_node('pageXmlNode');
	node[id]=id;
	node[page_group]=projectId;
	node[page_code]=pageId;
	
	
	Class.db('config').fetch("select page_info, page_kind from page_info where page_group=#{page_group} and page_code=#{page_code}", node);
	return node[page_info];
}
my.getPageString(root, pageId) {
	not( root ) {
		not( pageId ) return null;
		root=_node('@PageNode');	
		pageId.split('.').inject(page_group, page_code);
		root.put(page_group, page_code);
	}
	db=Class.db('pages');
	db.fetch("select layout from pageLayout where cmsCode=#{page_group} and pageCode=#{page_code} ", root);
	not( root[layout] ) return '';
	
	s='';
	s.add("layout: ", $root[layout]);

	db.fetchAll("select funcName, funcParam, funcData, note
		from pageFunc 
		where cmsCode=#{page_group} and pageCode=#{page_code} 
		order by sort", root.removeAll() );
		
	_body=func(s) {
		d='';
		s.ch();
		while(s.valid(), n, 0 ) {
			line=s.findPos("\n");
			if( n ) 
				d.add("\r\n" );
			else
				d.add("\r\n\t" );
			d.add(line);
		}
		return d;
	}
	while( a, root ) {
		if( a[note] ) {	
			s.add("\r\n/* $a[note] */");
		}
		body=_body(a[funcData].ref());
		s.add("\r\n${a[funcName]}($a[funcParam]) {$body\r\n}\r\n");
	}
	return s;
}
my.getPageControl(page) {
	p=page.parent();
	while( p  ) {
		if( p[@control] ) return p[@control];
		p=p.parent();
	}
	return null;
}
my.setGrid(cur, type ) {
	_drawStatus=func(rc) {
		draw.fill( rc, '#606062');
		draw.rectLine( rc, 134, '#606062', 4);
		node[rect form]=rc.move('down', 20);
		
		total=node.childCount();
		idx=startRow+1;
		
		draw.font(14, 'normal', '#f0f0f0');
		draw.text( rc.incr(10), "$idx/$total" );
		
		not( node[rect up] ) {
			divideRect( node, rc.move('end', 130), '55,10,55,*', 'up,#,down,#');	
		}
		if( startRow>0 ) {
			var='n';
		} else {
			var='d';
		}
		imgUp=commonImage('btn_up',var);
	
		nextRow=startRow+listNum;
		if( nextRow<total ) {
			var='n';
		} else {
			var='d';
		}
		imgDown=commonImage('btn_down',var);
		draw.drawImage( imgUp.center(node[rect up]), imgUp);
		draw.drawImage( imgDown.center(node[rect down]), imgDown);	
	};
	
	switch(type) {
	case header:
		args(2, rc, rate, fields, names);
		cur[rect header]=rc, arr=_arr(cur, 'HeaderWidths', true), farr=_arr(cur, 'HeaderFields', true); 
		divideRect(cur, rc, rate, fields );
		while( key, fields.split() ) {
			farr.add(key);
			rc=cur[rect $key];
			arr.add( rc.width() );
		}
		if( names ) {
			narr=_arr(cur, 'HeaderNames', true); 
			while(name, names.split() ) {
				narr.add(name);
			}
		}
	case list:
		args(2, node, startRow, listNum);
		not( startRow ) 		startRow=0;
		not( listNum )		listNum=10;
		node[rect]=cur[rect header];
		node[rect up]=null;
		node[rect].inject(sx, sy, sw, sh), sy+=sh;
		node[currentRow]=null;
		sp=startRow, ep=sp+listNum, rh=32;
		while( row, ep, sp ) {
			sub=node.child(row);
			not( sub ) return;
			cx=sx;
			sub[rect]=Class.rect(cx, sy, sw, rh);
			while( w, cur[HeaderWidths], col, 0 ) {
				sub[rect $col]=Class.rect(cx, sy, w, rh), cx+=w;
			}
			sy+=rh;
		}
	case draw:
		args(2, draw, node, startRow, listNum, drawFunc );
		arr=nvl(cur[HeaderNames], cur[HeaderFields]);
		fields=cur[HeaderFields];
		draw.save().font(14, 'bold', '#f0f0f0');
		while( field, arr, col, 0) {
			key=fields[$col];
			rc=cur[rect $key];
			draw.fill( rc , '#424243');
			draw.text(rc, field, 'center');
			if( col )
				draw.rectLine( rc, 234, '#606062', 4);
			else
				draw.rectLine( rc, 0, '#606062', 4);			
		}
		draw.font(11, 'normal', '#ffffff');
		sp=startRow, ep=sp+listNum, rcRow=null;
		while( row, ep, sp ) {
			sub=node.child(row);
			not( sub ) {
				break;
			}
			rcRow=sub[rect];
			while( field, fields, col, 0) {
				rc=sub[rect $col];
				if( drawFunc ) {
					drawFunc(draw, rc, sub, field, col );
				} else {
					draw.fill( rc, '#4f4f50');
					draw.text( rc.incrX(5), sub[$field], "left");
					if( col )
						draw.rectLine( rc, 34, '#606062', 4);
					else
						draw.rectLine( rc, 134, '#606062', 4);
				}
			}
		}
		draw.restore();
		not( rcRow ) return;
		_drawStatus( rcRow.move('down').height(65) );	
		if( node[currentRow] ) {
			draw.fill( node[currentRow.rect], '#dac0a040');
		}
	default:
	}



}
my.childAttrArray(root, attr, arr, type) {

	not( arr ) arr=[];
	while(cur, root ) {
		if( cur.childCount() ) {
			if( type.eq('sum') ) {
				arr.add( childAttrSum(cur, attr) );
			} else {
				childAttrArray(cur, attr, arr);
			}
		} else {
			arr.add( cur[$attr] );
		}
	}
	return arr;

}
my.childAttrSum(root, attr) {

	sum=0;
	while( cur, root ) {
		sum+=cur[$attr];
		if( cur.childCount() ) sum+=childAttrSum(cur, attr);
	}
	return sum;

}
my.childNodeDepth(root, depth) {
	not( depth ) depth=0;
	maxDepth=depth;
	while( cur, root ) {
		not( cur.childCount() ) continue;
		curDepth=childNodeDepth(cur, depth+1);
		if( maxDepth<curDepth ) maxDepth=curDepth;
	}
	return maxDepth;
}
my.childWidthArray(root, width ) {
	childAttrArray(root, 'rate', _arr(root,'rates',true), 'sum' );
	return _arr(root, 'WidthArray').recalc(width, root[rates]);
}
my.tagRateRect(root, rate) {
	while( cur, root ) {
		if( cur[tag].eq('Popup', 'Content') ) {
			continue;
		}
		rc=cur[rect];
		if( rc ) cur[rect]=rc.rate(rate);
		if( cur.childCount() ) tagRateRect(cur, rate);
	}
}
my.setCommCombo(combo, code, title) {
	node=getCommCodeNode(code);
	combo.removeAll();
	combo.addItem(node, 'code,value', title);
}
my.makeTreeNode(root, data, group) {
	not( group ) return;
	not( root ) root={};
	farr=_split(group);
	arr=[], parr=[];
	arr[0]=root;
	
	_depth=func(cur) {
		while( field, farr, n, 0 ) {
			prev=parr[$n];
			if( cur[$field].eq(prev) ) {
				continue;
			}
			parr[$n]=cur[$field];
			return n;
		}
	};
	while( cur, data, r, 0 ) {
		d=_depth(cur);
		p=arr[$d];
		if( p ) {
			field=farr[$d];
			c=p.addNode().initNode(cur), d++;
			c[@field]=field;
			c[depth]=d;
			c[tag]="node$d";
			c[value]=cur[$field];
			arr[$d]=c;
		}
	}
	arr.delete(), parr.delete();
	printNode(root);

}
my._split(&str, sep) {
	not( sep ) sep=',';
	arr = _arr();
	while( str.valid() ) {
		arr.add( str.findPos(sep).trim() );
	}
	return arr;
}
my.makeCommCode(code, data, field) {
	if( typeof(data,'node') ) {
		k='code', v='value';
		if( typeof(field,'array') ) {
			field.inject(k, v);
			not( k ) v=k;
		}
		s='';
		while( cur, data, n, 0 ) {
			if( n ) s.add(",\n");
			s.add("$cur[$k]: $cur[$v]");
		}
		print("makeCommCode=$s");
		conf("cc.${code}", s, true);
	}
}
my._node(val, key, reuse) {
	not( val ) {
		n=Cf[nodeIndex++];
		if( n.eq(16) ) {
			Cf[nodeIndex]=0;
			n=Cf[nodeIndex++];
		}
		node=Cf[gNode $n];
		not( node ) {
			node={};
			Cf[gNode $n]=node;
		}
		return node.reuse();
	}

	if( typeof(val,'node') ) {
		node=val;
		if( key ) {
			sub=node[$key];
			if( sub ) {
				if( reuse ) sub.removeAll();
			} else {
				sub={};
				node[$key] = sub;
			}
			return sub;
		}
		return node;
	}
	
	node = Cf[$val];
	not( node ) {
		node={};
		Cf[$val] = node;	
	}
	if( key ) node.removeAll();
	return node;

}
my.sortNode(root, attr, type) {
	not( attr ) attr='sort';
	node=_node(), arr=_arr();
	while( cur, root ) {
		arr.add(cur);
	}
	arr.sort(attr, type);
	root.reuse();
	while( cur, arr ) {
		root.addNode(cur);
	}	
}
my.childNodeSum(root, attr) {
	sum=0;
	while( cur, root ) {
		sum+=cur[$attr];
		if( cur.childCount() ) sum+=childNodeSum(cur,attr);
	}
	return sum;
}
my.gridMaxFiledWidth(root, field, fontSize, num) {

	not( fontSize ) fontSize=11;
	not( num ) num=20;
	mw=0;
	while( n, num ) {
		cur=root.child(n);
		val=cur[$field];
		if( val ) {
			w=textWidth(fontSize, val);
			if( mw<w ) mw=w;
		}
		not( cur ) break;
	}
	
	fw=textWidth(fontSize, field);
	
	if( mw>0 )  {
		mw+=10;
		return max(mw, fw);
	}  
	return fw+10;

}
my.ReportNode(classNode, parentCtrl) {
	classNode[ReportNode] = func(tag, parentCtrl) {
		parentCtrl.inject(db, cf);
		this.addClass('common/control.PageBase');
		_rate=func(x) {
			not( x ) return 0;
			x*=cf.pageRate;
			return x;
		}; 
	};
	classNode[@classBase]='ReportNode';
	Cf.setClass(classNode, 'ReportNode' , classNode, parentCtrl);
	
	not( classNode[tag].eq('report') ) return classNode;

	classNode[@className]= "ReportNode";
	copyClassFunc(classNode);
	return classNode;
}
my.ReportNodeBase( reload ) {


	classImpl=_node('ClassImpl');
	classNode=classImpl[ReportNodeBase];
	if( classNode ) {
		not( reload ) return classNode;
		classNode.initNode();
	} else {
		classNode={};
	}
	
	classNode.getMaxY = func(p) {
		not( p ) p=this.parent();
		maxY=p[rect].y();
		while( a, p ) {
			not( a[rectEnd] ) break;
			b=a[rectEnd].bottom();
			if( maxY< b ) maxY=b;
		}
		return maxY;
	}
	
	classNode.getMaxX = func(p) {
		not( p ) p=this.parent();
		maxX=p[rect].x();
		while( a, p ) {
			not( a[rectEnd] ) break;;
			b=a[rectEnd].right();
			if( maxX< b ) maxX=b;
		}
		return maxX;
	}
	classNode.setMargin = func(rc, end) {
		not( this[margin] ) return rc;
		
		this.inject(margin);
		if( typeof(margin,'num') ) {
			if( end ) {
				margin*=-1;
			}
			return rc.move(margin, margin);
		}
		if( typeof(margin,'array') ) {
			arr=margin;
		} else {
			arr=[];
			str=margin.ref();
			while( str.valid(), n, 0 ) {
				val=str.findPos(',').trim();
				arr.add(val.toNumber());
			}
			this[margin]=arr;
		}
		sz=arr.size(), chk=false;
		if( sz<3 ) chk=true;
		rc.inject(x,y,w,h);
		while( v, arr, n, 0 ) {
			switch(n) {
			case 0:
				x+=_rate(v);
				if( end ) {
					if(chk) {
						w+=2*v;
					} else if( arr[2].eq(0) || arr[2].eq(v) ) {
						w+=v;
					} 
				} else {
					if(chk) {
						w-=2*v;
					} else if( arr[2].eq(0) || arr[2].eq(v) ) {
						w-=v;
					}
				}
			case 1:
				if( end ) {
					y-=v; 
					if(chk) {
						h+=2*v;
					} else if( arr[3].eq(0) || arr[3].eq(v) ) {
						h+=v;
					}
				} else {
					y+=v; 
					if(chk) {
						h-=2*v;
					} else if( arr[3].eq(0) || arr[3].eq(v) ) {
						h-=v;
					}
				}
			case 2: 
				if( end ){
					w+=v;
				} else {
					w-=v;
				}
			case 3: 
				if( end ) {
					h+=v;
				} else {
					h-=v;
				}
			}
		}
		return Class.rect(x,y,w,h);
	}
	classNode.conf = func() {
		this.confStart();
		while( node, this ) {
			not( node[ReportNode]  ) {
				ReportNode( node, parentCtrl );
			}
			tag=node[tag].upper(1), func="conf$tag";
			not( node[$func] ) {
				node.copy( ReportNodeConf() );
				appendNodeText(node,"@classFuncs", "ReportNodeConf.conf$tag");
			}
			node.confStart();
			node.call(func);
			node.confEnd();
		}
		this.confEnd();
	}
	classNode.confStart = func() {
		this[rectEnd]=null;
		not( this[type] ) this[type]='vbox';
		not( this[rect] ) {
			p=this.parent();
			prc=p[rect];
			switch( nvl(p[type], 'vbox') ) {
			case vbox:
				x=prc.x(), y=this.getMaxY(p);
				w=when( this[width], this.rate(this[width]), prc.width() );
				if( this[height] ) {
					h=this.rate(this[height]);
				} else{
					remain=prc.bottom() - y;
					if( remain>0 ) {
						cnt=p.childCount(), cnt-=this.index();			
						h=remain, h/=cnt;			
					} else {
						h=100;
					}
				}
			case hbox:
				x=this.getMaxX(p), y=prc.y();
				h=when( this[height], this.rate(this[height]), prc.height() );
				if( this[width] ) {
					w=this.rate(this[width]);
				} else{
					remain=prc.right() - x;
					if( remain>0 ) {
						cnt=p.childCount(), cnt-=this.index();			
						w=remain, w/=cnt;			
					} else {
						w=100;
					}
				}
			default: 
			}
			rc=Class.rect(x, y, w, h);
			this[rect]=this.setMargin(rc);
		}
		this[rect].inject(x,y,w,h);
		this[posX]=x, this[posY]=y, this[rowW]=w, this[rowH]=h;
		return this[rect];
	}
	classNode.confEnd = func() {
		rc=this[rect];
		switch( this[type] ) {
		case vbox:
			not( this[posY].eq(rc.y() ) ) {
				this[rect]=rc.bottom(this[posY]);
			}
		case hbox:
			not( this[posX].eq(rc.x() ) ) {
				this[rect]=rc.right(this[posX]);
			}
		default:
		}
		this[rectEnd]=this.setMargin(this[rect],true);
	}
		
	classNode.rowStart = func(h) {
		this[posX]=this[rect].x();
		if( h ) {
			this[rowH]=_rate(h);
		} 
		return this[rowH];
	}
	classNode.rowRect = func(w, rate, space) {
		this.inject(posX, posY, rowH);
		cw=when( rate, _rate(w), w);
		rc=Class.rect(posX, posY, cw, rowH);
		posX+=cw;
		if( space ) posX+=space;
		this[posX]=posX;
		return rc;
	}
	classNode.rowEnd = func(space) {
		this.inject(posX, posY, rowH);
		if( posX > this[rect].right() ) {
			this[rect].right(posX);
		}
		posY+=rowH;
		if( space ) posY+=space;
		this[posY]=posY;
	}
	
	classNode.draw = func(draw, timeline) {
		while( node, this ) {
			tag=node[tag].upper(1), func="draw$tag";
			not( node[$func]) {
				node.copy( ReportNodeDraw() );
				appendNodeText(node,"@classFuncs", "ReportNodeDraw.draw$tag");
			}
			node.call(func, draw, timeline);
		}
	}
	classNode[@classRef]= true;
	
	classImpl[ReportNodeBase]=classNode;
	return classNode;
 

}
my.ReportNodeConf( reload ) {


	
classImpl=_node('ClassImpl');
classNode=classImpl[ReportNodeConf];
if( classNode ) {
	not( reload ) return classNode;
	classNode.initNode();
} else {
	classNode={};
} 
	
	
classNode.confTitle = func() {
	tw=textWidth(24, this[data])+50;
	this[rect text]=this[rect].center(tw, 40);
};
 	
classNode.confBoard = func() {
	_parse=func(s, stat) {
		if( row[cells].size() ) return;
		arr=_arr(row,'cells',true);
		while( s.valid() ) {
			c=s.ch();
			if( c.eq('<') ) {
				sp=s.cur(), s.incr();
				tag=s.move();
				if( tag.eq("blank") ) {
					arr.add("#blank");
				} else {
					s.pos(sp);
					in=s.match("<$tag", "</$tag>");
					prop= in.findPos('>').trim(), data=in.trim();
					arr.add("#${tag}[$prop]#$data");
				}
				s.findPos(",");
			} else {
				val=s.findPos(",").trim();
				if( stat  ) {
					if( val )
						arr.add( val );
					else
						arr.add("#blank");
				} else {
					arr.add( val );
				}
			}
		}
		print("arr=$arr");
	};
	
	_h=func( r, c ) {
		h=ha.get(r), r+=1;
		while( n,  ha.size(), r ) {
			row=tag.child(r);
			val=row[cells].get(c);
			not( val.eq('X') ) {
				break;
			}
			h+=ha.get(n);
		}
		return h;
	};
	_w=func(arr, c, size) {
		w=wa.get(c), c++;
		while( n, size, c) {
			not( arr[$n].eq('#blank') ) break;
			w+=wa.get(n);
		}
		return w;
	}
	
	tag[rect].inject( sx, sy, sw, sh);
	rowCount=tag.childCount();
	ha		=_arr().recalc( sh, rowCount );
	wa	=_arr().recalc( sw, tag[widthRate] );
	
	stat=tag[type].eq('statistics');
	while(row, tag ) {
		_parse( row[data].ref(), stat );
	}
	
	while( h, ha, r, 0 ) {
		row=tag.child(r), arr=row[cells];
		size=arr.size();
		cx=sx, lastCell=0;
		row[rect]=Class.rect(cx, sy, sw, h);
		while( c, size, 0 ) {
			val=arr[$c];
			if( val.eq('X','#blank') ) {
				if( val.eq('X') )  cx+=wa.get(c);
				continue;
			}
			ch=_h(r,c), cw=_w(arr,c, size);
			row[rect $c]=Class.rect(cx, sy, cw, ch);
			cx+=cw;
			lastCell=c;
		}
		row[lastCell]=lastCell;
		sy+=h;
	}		
};

classNode.confGrid = func() {
	header=tag.child(0);
	tag[rect].inject(sx,sy,sw,sh);
	
	hh=nvl(header[height],45);
	not( header[fields] ) {
		childAttrArray(header, 'field', _arr(header,'fields',true) );
	}
	hr=nvl(header[heightRate], childNodeDepth(header) );
	ha=_arr(header,'HeightArray').recalc(hh,hr);
	
	_confHeader=func(root, cx, cy, wa, depth) {
		while( th, root, r, 0 ) {
			w=wa.get(r);
			if( th.childCount() ) {
				h=ha.get(n);
				th[rect]=Class.rect(cx, cy, w, h);
				ty=cy+h;
				_confHeader(th, cx, ty,childWidthArray(th,w), depth+1  );
			} else {
				h=ha.sum(n);
				th[rect]=Class.rect(cx, cy, w, h);
			}
			cx+=w;
		}
	};
	header[rect]=Class.rect(sx,sy,sw,hh);
	_confHeader(header, sx, sy, childWidthArray(header, sw), 0);
	
	printNode(header);
};

classNode.confText = func() {
	print("confText=> $tag[rect]");
};

classImpl[ReportNodeConf]=classNode;
return classNode;



}
my.ReportNodeDraw( reload ) {



classImpl=_node('ClassImpl');
classNode=classImpl[ReportNodeDraw];
if( classNode ) {
	not( reload ) return classNode;
	classNode.initNode();
} else {
	classNode={};
} 
classNode.drawTitle = func(draw, timeline) {
	draw.rectLine( this[rect], 0, '#f09090');
	draw.text(this[rect text], this[data], 'center');
	print("##drawTitle $this[data]");
};

classNode.drawBoard = func(draw, timeline) {
	draw.font(11,'normal');
	lastRow=tag.childCount()-1;
	while( row, tag, r, 0 ) {
		last=row[lastCell];
		while( val, row[cells], c, 0 ) {
			if( val.eq('X','#blank') ) continue;
			rc=row[rect $c];
			if( c.eq(last) ) {
				draw.rectLine(rc, 4, '#909090');
			} else {
				draw.rectLine(rc, 34, '#909090');
			}
			if( val ) draw.text(rc, val, 'center');
		}
		if( r.eq(0) ) {
			draw.rectLine(row[rect], 2, '#909090', 2);
		} else if( r.eq(lastRow) ) {
			draw.rectLine(row[rect], 4, '#909090', 2);
		}
	}
};
classNode.drawGrid = func(draw, timeline) {
	/* 그리드 헤더 */
	_drawHeader=func(root) {
		while( th, root ) {
			if( th.childCount() ) {
				_drawHeader(th);
				draw.rectLine(th[rect], 4, '#c0c0c0', 2);				
				if( getNextNode(root, th, true) ) {
					draw.rectLine(th[rect], 3, '#c0c0c0');
				}
				draw.text(th[rect], getReportText(th[text]), 'center');
			} else {
				lineType= when( getNextNode(root, th, true), 34, 4 );
				draw.rectLine(th[rect], lineType, '#c0c0c0');
				draw.text(th[rect], getReportText(th[data]), 'center');
			}
		}
	};
	/* 그리드 리스트 */
	_drawList=func(root) {
		sp=0, ep=15, last=tag[fields].size() - 1;
		while( n, ep, sp ) {
			row=root.child(n);
			while( field, tag[fields], c, 0 ) {
				rc=row[rect $c];
				lineType=when( last.eq(c), 4, 34);
				draw.rectLine(rc, lineType, '#c0c0c0');
				switch(field) {
				case num:
					draw.text(rc, row[$field], 'center');
				default:
					if( row[$field] ) draw.text(rc, row[$field]);
				}
			}
		}
	};
	/* ## 그리드 그리기 ## */
	headerNode=null;
	while( cur, tag ) {
		switch(cur[tag]) {
		case header:
			headerNode=cur;
			draw.rectLine(cur[rect], 24, '#c0c0c0', 3);				
			_drawHeader(cur);
		case list:
			_drawList(cur);
		default:
		}
	}
};
classImpl[ReportNodeDraw]=classNode;
return classNode;




}
my.appendNodeText(node, key, text) {
	if( node[$key] ) {
		s=node[$key].ref();
		while( s.valid() ) {
			k=s.findPos(',').trim();
			if( k.eq(text) ) return;
		}
		node[$key].add(",");
	} else {
		node[$key]='';
	}
	node[$key].add(text);
}
my.getReportText(&s) {
	d='';
	while( s.valid() ) {
		left=s.findPos("<br>");
		d.add(left);
		if( s.valid() ) {
			d.add("\r\n");
		}
	}
	return d;
}
my.textBoundRect(rc, text, fontSize, fontStyle, type) {
	dc=Cf[DrawDC];
	not( dc ) {
		dc=Class.draw(500,500);
		Cf[DrawDC]=dc;
	}
	not( type ) type="wrap";
	not( fontSize ) fontSize=11;
	not( fontStyle ) fontStyle='normal';
	dc.font( fontSize, fontStyle );
	return dc.bound(rc, text, type);
}
my.getNextNode(root, cur, pchk) {
	tag=cur[tag];
	idx=cur.index() + 1;
	next=root.child(idx);
	if( next ) return next;
	
	if( pchk ) {
		if( root[tag].eq(tag) ) {
			pp=root.parent();
			return getNextNode(pp, root, true);
		}
	}
	return null;
}
my.copyClassFunc(node, reload) {

	db=Class.db('pages');
	root = Cf[includeNode];
	classImpl=_node('ClassImpl');
	
	
	baseName="${node[@classBase]}Base";
	root[class_grp]=node[@classBase];
	root[class_nm]=baseName;
	root.removeAll();
	
	classNode=classImpl[$baseName];
	not( classNode ) {
		classNode={};
	}
	if( classNode && reload ) {
		classNode.initNode();
	} 
	not( classNode[classSet] ) {
		db.fetchAll("select class_nm, class_func, class_param, case when length(class_src)>0 then class_src else class_data end src
			from class_func
			where class_grp=#{class_grp} and class_nm=#{class_nm}
			order by type", root);
		src='';
		while( cur, root ) {
			src.add("classNode[${cur[class_func]}] = func($cur[class_param]) { $cur[src] };");
		}		
		Cf.call(src);
		classNode[classSet] =true;
	}
	
	node.copy( classNode, reload );
	classImpl[$baseName]=classNode;
	
	
	className=node[@className];
	root[class_grp]=node[@classBase];
	root[class_nm]=className;
	
	classNode=classImpl[$className];	
	not( classNode ) {
		classNode={};
	}
	if( classNode && reload ) {
		classNode.initNode();
	} 
	not( classNode[classSet] ) {
 		db.fetchAll("select class_nm, class_func, class_param, case when length(class_src)>0 then class_src else class_data end src
			from class_func
			where class_grp=#{class_grp} and class_nm=#{class_nm}
			order by type", root);
		src='';
		while( cur, root ) {
			src.add("classNode[${cur[class_func]}] = func($cur[class_param]) { $cur[src] };");
		}
		Cf.call(src);
		classNode[classSet] =true;
	}
	node.copy( classNode, reload );
	classImpl[$className]=classNode;

}
my.saveClassBase(&src, classGroup, className ) {
	root = Cf[includeNode];
	root.removeAll();
	db=Class.db('pages');
	note='', tm=System.localtime();
	root[class_grp]=classGroup;
	root[class_nm]=className;
	db.exec("delete from class_func where class_grp=#{class_grp} and class_nm=#{class_nm}", root); 
	while( src.valid() ) {
		c=src.ch();
		not( c ) break;
		
		if( c.eq('/') ) {
			if( src.ch(1).eq('/') ) note.add( src.findPos("\n") );
			else if( src.ch(1).eq('*') ) note.add( src.match() );
			continue;
		}
		src.move();
		not( src.ch().eq('.') ) {
			break;
		}
		src.incr();
		funcName=src.move().trim();
		not( src.ch().eq('=') ) {
			break;
		}
		src.incr(), src.move();
		not( src.ch().eq('(') ) {
			break;
		}
		param=src.match().trim();
		not( src.ch().eq('{') ) {
			break;
		}
		root.varMap('class_func: funcName, class_param: param, note');
		body=src.match();
		if( body.find('//') || body.find('/*') ) {
			root[class_src]=body;
			root[class_data]=makeSrc(body);			
		} else {
			root[class_src]='';
			root[class_data]=body;			
		}
		num=db.exec("update class_func set class_src=#{class_src}, class_data=#{class_data}, class_param=#{class_param}, note=#{note}, tm='$tm' where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", root);
		not( num ) {
			db.exec("insert into class_func( class_grp, class_nm, class_func, class_param, class_src, class_data, note, tm ) values (#{class_grp}, #{class_nm}, #{class_func}, #{class_param}, #{class_src}, #{class_data}, #{note}, '$tm')", root);
		}
		note='';
		if( src.ch().eq(';') ) src.incr();
	}

}
KioskHiTec.getRectArr(var) {

	if( typeof(var,'node') ) {
		args(1, rect, rate, mode, rid, space);
		not( mode ) mode='hbox';
		arr= when( rid, _arr(var,rid,true), _arr() );
		
		rect.inject(x,y,w,h);
		if( mode.eq('vbox') ) {
			arr.recalc(rect.height(), rate, true);
			while( hh, arr, n, 0 ) {
				rc=Class.rect(x,y,w,hh);
				arr[$n]=rc;
				y+=hh;
				if( space ) y+=space;
			}
		} else {
			arr.recalc(rect.width(), rate, true);
			while( ww, arr, n, 0 ) {
				rc=Class.rect(x,y,ww,h);
				arr[$n]=rc;
				x+=ww;
				if( space ) x+=space;
			}
		}
		return arr;
	}
	return null;

}
KioskHiTec.setFont(draw,style) {
	ok=false, fontDef='맑은 고딕';
	if( typeof(style,'number') ) {
		fontSize=style;
		args(2, fontColor, fontWeight, fontName);
		ok=true;
	} else if( typeof(style,'array') ) {
		style.inject(fontSize, fontColor, fontWeight, fontName);
		ok=true;
	} 
	if( ok ) {
		not( fontName ) {
			fontName=fontDef;
		}
		draw.font(fontSize, fontWeight, fontColor, fontName);
		return draw;
	}
	switch( style ) {
	case TableHeader:
		draw.font(24, 'bold', "#eacaa0", fontDef );
	case PopupTitle:
		colorBk	=Class.color('#708a80');
		draw.font(28, 'bold', colorBk, fontDef );
	case PopupButton:
		colorBk	=Class.color('#4a4a4a');
		draw.font(28, 'bold', colorBk, fontDef );
	case SubTitle:
		draw.font(24, 'normal', "#505050", fontDef );
	case TableList:
		draw.font(24, 'normal', "#505050", fontDef );
	case OrderInfo:
		draw.font(16, 'normal', "#505050", fontDef );
	case OrderPrice:
		draw.font(14, 'normal', "#808080", fontDef );
	case OrderHeader:
		colorBk		=Class.color('#FF9900');
		draw.font(18, 'bold', colorBk.darkColor(130), fontDef );
	default:
		draw.font(11, 'normal');
	}
	return draw;
}
KioskHiTec.divideRect(cur, rc, rate, keys, vbox) {
	size=when( vbox, rc.height(), rc.width() );
	rc.inject(x,y,w,h);
	arr=_arr();
	if( rate.find('*') ) {
		while( a, rate.split() ) {
			if( a.eq('*') ) {
				arr.add(0);
			} else {
				arr.add(a);
			}
		}
		sum=arr.sum();
		remain=size-sum;
		if( remain>0 ) {
			while( a, arr, n, 0 ) {
				not( a ) {
					arr[$n]=remain;
					break;
				}
			}
		}		
	} else {
		arr.recalc(size, rate);	
	}
	if( keys ) {
		karr=when( typeof(keys,'array'), keys, keys.split() );
		while( key, keys.split(), n, 0 ) {
			if( key.eq('#') ) {
				if( vbox) {
					y+=arr[$n];
				} else {
					x+=arr[$n];
				}
				continue;
			}
			if( vbox) {
				ch=arr[$n];
				cur[rect $key]=Class.rect(x,y,w,ch), y+=ch;
			} else {
				cw=arr[$n];
				cur[rect $key]=Class.rect(x,y,cw,h), x+=cw;
			}
		}
	} else {
		while( a, arr, n, 0 ) {
			sub=cur.child(n);
			not( sub ) break;
			if( vbox) {
				sub[rect]=Class.rect(x,y,w,a), y+=a;
			} else {
				sub[rect]=Class.rect(x,y,a,h), x+=a;
			}
		}
	}
}
KioskHiTec.loadCommonImage(var) {




	imagePath=when( typeof(var,'node'), var.imagePath, var);
	commonImage('button60',		"$imagePath/main/common/button60_[#].png");
	commonImage('btn_down',		"$imagePath/main/common/btn_down_[#].png");
	commonImage('btn_up',			"$imagePath/main/common/btn_up_[#].png");
	
	commonImage('navi_on',		"$imagePath/main/common/navi_on.png");
	commonImage('navi_off',		"$imagePath/main/common/navi_off.png");
	
	commonImage('radio_on',		"$imagePath/main/common/radio_on.png");
	commonImage('radio_off',		"$imagePath/main/common/radio_off.png");
	commonImage('check_on',		"$imagePath/main/common/checkbox_on.png");
	commonImage('check_off',		"$imagePath/main/common/checkbox_off.png");
	
	commonImage('submenu_box11',	"$imagePath/main/common/submenu_box_11.png");
	commonImage('submenu_box13',	"$imagePath/main/common/submenu_box_13.png");
	commonImage('submenu_box21',	"$imagePath/main/common/submenu_box_21.png");
	commonImage('submenu_box22',	"$imagePath/main/common/submenu_box_22.png");
	commonImage('submenu_box23',	"$imagePath/main/common/submenu_box_23.png");
	
	commonImage('popup_box11',		"$imagePath/main/common/popup_top_left.png");
	commonImage('popup_box12',		"$imagePath/main/common/popup_top_center.png");
	commonImage('popup_box13',		"$imagePath/main/common/popup_top_right.png");
 	commonImage('popup_box21',		"$imagePath/main/common/popup_body_left.png");
	commonImage('popup_box22',		"$imagePath/main/common/popup_body_center.png");
	commonImage('popup_box23',		"$imagePath/main/common/popup_body_right.png");
 	commonImage('popup_box31',		"$imagePath/main/common/popup_status_left.png");
	commonImage('popup_box32',		"$imagePath/main/common/popup_status_center.png");
	commonImage('popup_box33',		"$imagePath/main/common/popup_status_right.png");
	
	commonImage('menu_bg',			"$imagePath/main/common/menu_bg.png");
	commonImage('menu_tab1',		"$imagePath/main/common/menu_tab1.png");
	commonImage('menu_tab2',		"$imagePath/main/common/menu_tab2.png");
	commonImage('menu_tab3',		"$imagePath/main/common/menu_tab3.png");
	commonImage('menu_down',		"$imagePath/main/common/menu_down.png");

	commonImage('calendar_bg',		"$imagePath/main/dialog/calendar_bg.png");
	commonImage('btn_next',			"$imagePath/main/common/c_next_[#].png");
	commonImage('btn_prev',			"$imagePath/main/common/c_prev_[#].png");
	
	commonImage('btn_cancel',		"${imagePath}/Common/kr/pop_cancel_[#].png");
	commonImage('btn_confirm',		"${imagePath}/Common/kr/pop_confirm_[#].png");
	commonImage('btn_ok',				"${imagePath}/Common/kr/pop_ok_[#].png");
	commonImage('btn_before',		"${imagePath}/Common/kr/pop_before_[#].png");
	commonImage('btn_bg',				"${imagePath}/admin/button_bg_[#].png");

	commonImage('page_close',		"${imagePath}/admin/page_close_[#].png");
	commonImage('popup_cancel',	"${imagePath}/type/btn/pop_cancel_[#].png");
	commonImage('popup_confirm',	"${imagePath}/type/btn/pop_confirm_[#].png");

	commonImage('lang01',				"${imagePath}/type/lang01.png"); 
	commonImage('lang02',				"${imagePath}/type/lang02.png"); 
	commonImage('lang03',				"${imagePath}/type/lang03.png"); 
	
	commonImage('langKor',			"${imagePath}/type/korea.png"); 
	commonImage('langEng',			"${imagePath}/type/english.png"); 
	commonImage('langJpn',			"${imagePath}/type/japan.png"); 
	commonImage('langCha',			"${imagePath}/type/china.png"); 
	

	commonImage('icon_enter',		"${imagePath}/tool/keyboard/icon_enter.png"); 
	commonImage('icon_etc',			"${imagePath}/tool/keyboard/icon_etc.png"); 
	commonImage('icon_bs',			"${imagePath}/tool/keyboard/icon_bs.png");  

	commonImage('menu_box',		"${imagePath}/type/menu_box.png"); 


	commonImage('admin_title',		"${imagePath}/admin/admin_page_title.png");
	commonImage('admin_bg',		"${imagePath}/admin/bg_pattern.jpg");
	commonImage('admin_bg1',		"${imagePath}/main/common/view_bg.png");
	commonImage('no_img',			"${imagePath}/Common/no_images4.png");


}
KioskHiTec.saveClassNode(var, &s, page) {
	classInfo=when( typeof(var,'class'),  var[@className], var );
	not( classInfo ) return;
	
 	classInfo.split('.').inject(class_grp, class_nm);
	root=_node();
	db=Class.db('pages');
	tm=System.localtime();
	root.put(class_grp, class_nm, tm);
	note='', err='';
	while( s.valid() ) {
		c=s.ch();
		not( c ) break;
		if( c.eq('/') ) {
			if( s.ch(1).eq('/') ) note.add( s.findPos("\n") );
			else if( s.ch(1).eq('*') ) note.add( s.match() );
			continue;
		}
		func=s.move();
		c=s.ch();
		not( c.eq('(') ) {
			err.add("함수 시작오류 : 함수명 : $func");
			break;
		}
		param=s.match().trim();
		c=s.ch();
		not( c.eq('{') ) {
			err.add("함수 매개변수 오류: $param");
			break;
		}
		body=s.match(1);
		root.varMap('class_func: func, class_param: param, note');
		if( body.find('//') || body.find('/*') ) {
			root[class_src]=body;
			root[class_data]=makeSrc(body);			
		} else {
			root[class_src]='';
			root[class_data]=body;			
		}
		if( func.eq(class_nm) ) {
			root[type]='A';
		} else {
			root[type]='F';
		}
		num=db.exec("update class_info set class_src=#{class_src}, class_data=#{class_data}, class_param=#{class_param}, type=#{type}, note=#{note}, tm=#{tm} where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", root);
		not( num ) {
			db.exec("insert into class_info( class_grp, class_nm, class_func, class_param, class_src, class_data, note, type, tm ) values (#{class_grp}, #{class_nm}, #{class_func}, #{class_param}, #{class_src}, #{class_data}, #{note}, #{type}, #{tm})", root);
		}
		note='';
	}
	cnt=db.count("select count(1) as cnt from class_mst where class_grp=#{class_grp} and class_nm=#{class_nm}", root);
	not( cnt ) {
		db.exec("insert into class_mst (class_grp, class_nm, type, tm) values (#{class_grp}, #{class_nm}, 'A', #{tm})", root);
	}
	if( page ) {
		if( err ) page.alert(err);
	} else {
		not( include("${class_grp}.${class_nm}",true) ) {
			err.add("클래스 로딩중 오류가 발생했습니다");
		}	
	}
	return err.trim();

}
KioskHiTec.getClassSrc(var) {
	classInfo=when( typeof(var,'class'),  var[@className], var );
		classInfo.split('.').inject(group, name);
		if( group, name ) {
			db=Class.db('pages');
		root=_node();
			root.put(group, name);
			db.fetchAll("select class_nm, class_func, class_param, case when length(class_src)>0 then class_src else class_data end src
				from class_info
				where class_grp=#{group} and class_nm=#{name}
				order by type, class_func", root);
			rst='';
			while( a, root ) {
				if( a[note] ) {
					rst.add("\r\n/* $a[note] */");
				}
				rst.add("\r\n${a[class_func]}($a[class_param]) {$a[src]}\r\n");
			}
			return rst;
		}
		return null;
}
KioskHiTec.commonImage(id, var, path) {

	node=Cf[commonImage];
	not( node ) {
		node={};
		Cf[commonImage]=node;
	}
 	not( node[$id] ) {
 		not( var ) return null;
 		not( path ) {
 			if( var.eq('n','p','d') ) return null;
			path=var;
			var=null;
		}
		if( path ) {
			print(id, path);
			node[$id]=path;
		}
	}
	return imageLoad(node, id, var);

}
KioskHiTec.imageLoad(node, type, var ) {

	path=node[$type];
	if( path.find('[#]') ) {
		not( var ) var='n';
		image=node[$type$var];
		if( image ) return image;
		imagePath=tr(path, var);	
		image=Cf.imageLoad( imagePath, true);
		node[$type$var]=image;
		return image;
	}
	image=node[@$type];
	if( image ) return image;
	
	if( var, not(path) ) {
		path=var;
	}
	if( Class.file().isFile(path) ) {
		image=Cf.imageLoad( path, true);
		node[@$type]=image;	
	}
	return image;

}
KioskHiTec.printNode(root, depth, all) {

	not( depth ) {
		print("== print node start ${root[tag]} ($root[rect]) ==");
		if( all ) print("root=> $root");
	}
	not( root ) {
		return null;
	}
	not( depth ) depth=0;
	indent='';
	while( n, depth ) indent.add(" ");
	while( cur, root ) {
		s="$indent ${cur[tag]} ($cur[rect])";
		if( all ) s.add("## $cur");
		print(s);
		printNode(cur, depth+1, all);
	}
	not( depth ) {
		print("== print node end ==");
	}



}
KioskHiTec.findTagId(tag, id, root, all) {

	_findId=func(id, node) {
		while( cur, node ) {
			if( cur[id].eq(id) ) {
				return cur;
			}
			if( all ) {
				find=_findId(id, cur);
				if( find ) return find;
			}
		}
		return null;
	};
 	_find=func(tag, id, node) {
		while( cur, node ) {
			if( cur[tag].eq(tag)  ) {
				if( cur[id].eq(id) ) {
					return cur;
				}
				find=_findId(id, cur);
				if( find ) return find;
			}
			if( all ) {
				find=_find(tag, id, cur);
				if( find ) return find;
			}
		}		
	};
	return _find(tag, id, root);

}
KioskHiTec.copyClassSrc(srcDbPath, classes) {
	not( srcDbPath ) return;
	
	src=Class.db('src');
	dest=Class.db('pages');
	src.close();
	src.open(srcDbPath);
	
	field='class_grp, class_nm, class_func, class_param, class_data, class_src, tm, type, note';
	upt=getQuery('class_info', field, 'class_grp, class_nm, class_func');
	ins=getQuery('class_info', field);
	
	in='';
 	while( c, classes.split(), n, 0 ) {
		if( n ) in.add("',''");
		in.add(c);
 	}
 	tm=System.localtime();
	while( cur, src.fetchAll("select $field from class_info where  class_nm in (''$in')  ") ) {
		print("## $cur[class_nm] > $cur[class_func] ## $cur[tm]" );
		cur[tm]=tm;
		not( dest.exec(upt,cur) ) {
			dest.exec(ins, cur);
		}
	}	
}
KioskHiTec.getCommCodeNode(code, def, reload) {
	x=Cf[CommCodeNode];
	not( x ) {
		x={};
		Cf[CommCodeNode]=x;
	}
	
	if( code.eq('corner') ) {
		node=_node(x,code);
		cnt=node.childCount();
		if( reload ) cnt=0;
		if( cnt ) return node;
		node.removeAll();
		if( def ) {
			sub=node.addNode();
			sub[code]="";
			sub[value]=def;
		} 
		Class.db('kiosk_hitec').fetchAll(conf("sql#cc.$code"), node);
	}


	node=_node(x,code);
	if( node.childCount() ) {
		not( reload ) return node;
		node.removeAll();
	}
	if( def ) {
		sub=node.addNode();
		sub[code]="";
		sub[value]=def;
	}
	_parse=func(&s) {
		while( s.valid() ) {
			k=s.move().trim();
			not( k ) break;
			sub=node.addNode();
			if( s.ch().eq(':') ) {
				sub[code]=k;
				c=s.incr().ch();
				if( c.eq() ) {
					sub[value]=s.match();
				} else {
					if( s.find('@') ) {
						left=s.findPos('@');
						sub[value]=left.findPos(',').trim();
						sub[data]=s.findPos("\n").trim();
					} else {
						sub[value]=s.findPos(',').trim();
					}
				}
			} else {
				sub[code]=k;
			}
		}
	}
	_parse(conf("cc.$code") );
	return node;


}
KioskHiTec.getEventTypeName(type) {
	while( str, Cf.define('KIOSK') ) {
		in=str.match();
		left=in.findPos(':');
		k=in.trim();
		if( k.eq(type) ) {
			return left.trim();
		}
	}
	return null;
}
KioskHiTec.rateArr(size, var) {
	return _arr().recalc(size, var);
}
KioskHiTec.loadMenu(cf, menu) {
	node=_node('MenuImages');
	menuCd=menu[menu_cd];
	img=node[@$menuCd];
	if( img ) return img;
	
	fileName=menu[goods_img];
	not( fileName ) {
		return null;
	}
	node[$menuCd]="$cf[imagePath]/menus/$fileName";
	return imageLoad(node, menuCd);
}
KioskHiTec.getDrawTimeline(node) {
	while( tm, node ) {
		not( tm.state(NODE.start) ) continue;
		tid=tm[tid];
		if( Cf.timeLine("${tid}.running") ) { 
			return tm;
		} else {
			tm.state(NODE.start, false);	
			return tm;
		}
	}
	return null;
}
KioskHiTec.pageActionAdd(page, actionCode) {
	switch( args().size() ) {
	case 3: args(2,text);
		page.action(actionCode).text(text);
	case 4: args(2,text,icon);
		page.action(actionCode, text, icon);
	case 5: args(2,text,icon,trigger);
		page.action(actionCode, text, icon);
		page.action(actionCode).trigger(trigger);
	default: return;
	} 
}
KioskHiTec.gridMakeField(&s, grid, fields) {
	not( fields ) {
		fields=grid.fields();
		not( fields ) fields={};
	}
	fields.removeAll();
	if( grid ) {
		cur=fields.addNode({code:check, text: *} );
		cur.width = '40px'; 
	}
	while( s.valid() ) {
		line=s.findPos(',');
		not( line.ch() ) continue;
		cur = fields.addNode();
		cur[code]=line.findPos(':').trim();
		cur[text]=line.findPos('#').trim();
		cur[width]=line.trim();
	}
	rate=_arr(fields,'rate').reuse();
	while( cur, fields ) {
		rate.add(cur[width]);
	}
	_arr(fields,'widths').reuse();
	return fields;


}
KioskHiTec.confNodeLayout(root, offset, space) {
	layout=root[type];
	not( layout ) layout="vbox";
	if( offset ) {
		offset.inject(ox, oy);
	}
	_rect=func(node, x, y, w, h, layer ) {
		if( node[Margin] ) {
			if( typeof(node[Margin],'string') ) {
				arr=[];
				str=node[Margin].ref();
				while( str.valid(), n, 0 ) {
					val=str.findPos(',').trim();
					arr.add(val.toNumber());
				}
				node[Margin]=arr;
			} else {
				arr=node[Margin];
			}
			sz=arr.size(), chk=false;
			if( sz<3 ) chk=true;
			while( v, arr, n, 0 ) {
				switch(n) {
				case 0: 
					x+=v; 
					not( layer ) {
						if(chk) {
							w-=2*v;
						} else if( arr[2].eq(0) || arr[2].eq(v) ) {
							w-=v;	
						}
					}
				case 1: 
					y+=v; 
					not( layer ) {
						if(chk) {
							h-=2*v;
						} else if( arr[3].eq(0) || arr[3].eq(v) ) {
							h-=v;
						}
					}
				case 2: w-=v;
				case 3: h-=v;
				}
			}
		}
		if( layer && offset ) {
			x+=ox;
			y+=oy;
		}
		node[rect]=Class.rect(x,y,w,h);
	};
	
	root[rect].inject(sx,sy,sw,sh);	
	while( cur, root ) {
		if( cur[tag].eq('Popup') ) continue;
		w=cur[Width];
		h=cur[Height];
		not( w ) w=sw;
		not( h ) h=sh;
		if( cur[class].eq('layer') ) {
			_rect(cur, 0, 0, w, h, true );
			continue;
		}
		
		_rect(cur, sx, sy, w, h );
		if( layout.eq('vbox') ) {
			sy+=h;
			if( space ) sy+=space;
		} else if( layout.eq('hbox') ) {
			sx+=w;
			if( space ) sx+=space;
		}
	}

}
KioskHiTec.drawCommButton(draw, rc, imgId, textId, page, lang) {


	ty=when( rc.eq(page.mouseDownRect), 'p', 'n');
	draw.drawImage( rc, commonImage(imgId,ty) );
	if( lang.eq('kor') ) {
		switch( textId ) {
		case ok:			text="확   인";
		case cancel: 	text="취   소";
		case order: 		text="카드결제";	
		}		
	} else { 
		switch( textId ) {
		case ok:			text="Ok";
		case cancel: 	text="Cancel";
		case order: 		text="Card Payment";	
		}
	}
	drawNodeText(draw, rc, text, "center", 'PopupButton');


}
KioskHiTec.drawFormInput(draw, el) {
	rc=el[rect];
	not( rc ) return;
	print( el, rc);
	draw.fill( rc, '#ffffff').rectLine( rc, 0, '#a0a0a0');
	draw.font(12,'normal','#30303a').text( rc.incrX(5), el[text]);
}
KioskHiTec.addFormElement(form, id, tag, rect, w, h, label) {
	cur=getFormElement(form, id);
	if( cur ) {
		return cur;
	}
	cur = form.addNode();
	cur.put(id, tag);
	if( w ) {
		if( label ) {
			rcForm=rect.center(rect.width(), h);
			tw= textWidth(14,label)+5;
			divideRect( form, rcForm, "$tw,$w,*", 'label, input,#');
			cur[label text]	= label;
			cur[label rect]	= form[rect label];
			cur[rect]			= form[rect input];
		} else {
			cur[rect]			= rect.width(w);
		}
	} else {
		cur[rect]= rect.inject(5);
	}
	return cur;
}
KioskHiTec.getFormElement(form, id, tag ) {
	cur=form.findOne('id', id);
	if( cur ) {
		return cur;
	}
	cur = form.addNode();
	cur.put(id, tag);	
	return cur;
}
KioskHiTec.drawFormElement(draw, form, id) {

	not( form ) return;
	cur=getFormElement(form,id);
	not( cur ) {
		return;
	}
	if( cur[label rect] ) {
		rc=cur[label rect];
		draw.font( 14, 'normal', '#f0f0f0').text( rc, cur[label text] );	
	}
	
	rc=cur[rect];
 	not( rc ) {
 		return;	
 	}
	draw.fill( rc, '#ffffff').rectLine( rc, 0, '#a0a0a0');
	draw.font(16,'bold','#30303a').text( rc.incrX(5), cur[text]);

}
KioskHiTec.drawNodeText(draw, rc, text, align, style, tag) {


	not( rc ) {
		print("drawNodeText rect not define: $text, $style");
		return;
	} 
	fontDef='나눔바른고딕';
	ok=false;
	if( typeof(style,'number') ) {
		fontSize=style;
		args(5, fontColor, fontWeight, fontName);
		not( fontName ) {
			fontName=fontDef;
		}
		draw.save().font(fontSize, fontWeight, fontColor, fontName);
		ok=true;
	} else if( typeof(style,'array') ) {
		args(5, ctrl);
		style.inject(fontSize, fontColor, fontWeight, fontName);
		not( fontName ) {
			fontName=fontDef;
		}
		if( ctrl ) {
			fontSize=ctrl.rate(fontSize);
		}
		draw.save().font(fontSize, fontWeight, fontColor, fontName);
		ok=true;
	} else if( style ) {
		draw.save();
		switch( style ) {
		case MenuName:
			draw.font(18, 'normal', "#e0e0ead0", fontDef );
			rw=rc.width(), tw=draw.textWidth(text)+16;
			if( rw< tw ) {
				draw.text(rc, text );
			} else {
				draw.text(rc, text, 'center');
			}
		case MenuPrice:
			draw.font(22, 'bold', "#f0f0ff", fontDef ).text(rc, text, 'center');
		case TabSelect:
			draw.font(24, 'bold', "#fafafa", fontDef );
			tw=draw.textWidth(text) + 15;
			if( text.find("\n") ) {
				draw.text(rc, text, 'center'); 
			} else {
				if( tw>rc.width() ) {
					draw.text(rc.incr(10), text, 'wrap');
				} else {
					draw.text(rc, text, 'center'); 
				}
			}
		case TabNormal:
			draw.font(24, 'bold', "#fbde9b", fontDef );
			tw=draw.textWidth(text) + 15;
			tw=draw.textWidth(text) + 15;
			if( text.find("\n") ) {
				draw.text(rc, text, 'center'); 
			} else {
				if( tw>rc.width() ) {
					draw.text(rc.incr(10), text, 'wrap');
				} else {
					draw.text(rc, text, 'center'); 
				}
			}
		case TableHeader:
			draw.font(24, 'bold', "#ECE6E0", fontDef ).text( rc.incrXY(1,2,true), text, align);
			draw.pen('#675550').text( rc, text, align);
		case OrderHeader:
			colorBk		=Class.color('#E8E5E0');
			draw.fill(rc.incr(1), '#E8E5E0');
			draw.font(18, 'bold', '675550', fontDef );
			if( align.eq('left') ) rc.incrX(40);
			draw.text( rc, text, align);
		case OrderInfo:
			draw.font(20, 'normal', "#505050", fontDef, 1.8).text( rc.incrX(15), text, align);
		case OrderPrice:
			draw.font(20, 'bold', "#C00D12", fontDef, 1.8).text( rc, text, align);
		case TableList:
			draw.font(22, 'normal', "#666666", fontDef ).text( rc, text, align);
		case MainButton:
			draw.font(26, 'bold', "#fafaf0", fontDef ).text(rc, text, 'center');		
		case PopupTitle:
			rc.incrX(45);
			colorBk	=Class.color('#606a6a');
			draw.font(28, 'bold', colorBk, fontDef ).text( rc.incrXY(1,2,true), text, align);
			draw.pen(colorBk.lightColor(230) ).text( rc, text, align);
		case PopupButton:
			colorBk	=Class.color('#606a6a');
			draw.font(28, 'bold', colorBk, fontDef ).text( rc.incrXY(1,2,true), text, align);
			draw.pen(colorBk.lightColor(225) ).text( rc, text, align);
		case SubTitle:
			draw.font(24, 'normal', "#505050", fontDef ).text( rc, text, align);
		default:
			draw.text(rc, text, align);
		}
		draw.restore();
		return;
	}
	
	draw.text(rc, text, align);
	if( ok ) {
		draw.restore();
	}


}
KioskHiTec.canvasMouseAction(ctrl ) {
	ctrl.inject( cf, page  );
	ok=false;
	
	if( cf[mouseActionPoints].size() > 10 ) {
		arr=Cf.direction(cf[mouseActionPoints] );
		print("canvasMouseAction=$arr");
		if( arr.size() > 1  ) {
			str=arr.join();
			switch(str) {
			case LeftDownRight:
				ok=true;
				ctrl.closeKiosk();
				Cf.exit();
			case LeftDown:
				cf[errorOpen]=false;
				ctrl.popupClose();
				Class.db('kiosk_hitec').exec("update kiosk_error set error_status='S' where error_status='R'");
			case RightLeft:
				cf[mouseDownAction]=false;
				Class.db('kiosk_hitec').exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status, tm) values( 'notify', 'admin', '점검', '잠시 정검중입니다. 조금만 기다려 주세요', 'R', 0 )" );
				ctrl.alert("잠시 정검중입니다. 조금만 기다려 주세요","정검", true);
			case RightDownLeft:
				cf[mouseDownAction]=false;
				if( System.processCheck('KioskWatcher.exe') ) {
					Class.web('admin').call('http://localhost:8089/@kiosk.Common.WatcherOpen');
				} else {
					System.run("KioskWatcher.exe");
				}
			default:
			}
		}
	}
	return ok;
}
KioskHiTec.order_completeCardProcess( main, node, order, cart, posInfo) {

	main.popupClose();
	main.popupOpen('Loading');
	System.timeout(250);

	not( order ) {
		return false;
	}
	
	main.inject(cf);
	cf[orderMessage]	= null;
	cf[orderStartTick]	= System.tick();
	
	setup			=cf[SetupInfo];
	cornerInfo	=cf[CornerInfo];
	printInfo		=cf[PrintInfo];
	order[error]='';

	items			= cart.getOrderList();
	totalPrice	= cart[OrderTotalPrice]; 
	totalQty		= cart[OrderTotalQty];
	today			= System.date('yyyyMMdd');
	datetime	= System.date('MM/dd HH:mm:dd');
	dtm = datetime;
	
	not( items ) {
		items=cf[orderItemList];
	}
	
	headerSql="insert into tb_sale_header (ms_no,open_date,pos_no,bill_no,sale_date,deal_no,detail_cnt,slip_cnt,total_qty,total_amt,sale_amt,vat_amt, send_yn) values( #{ms_no},#{open_date},#{pos_no},#{bill_no},#{sale_date},#{deal_no},#{detail_cnt},'1',#{total_qty},#{total_amt},#{sale_amt},#{vat_amt},'N')";
	detailSql="insert into tb_sale_detail (sale_seq,ms_no,open_date,pos_no,bill_no,detail_index,corner_cd,menu_cd,price,qty,total_amt,sale_amt,vat_amt,dc_amt,cancle_yn) values( #{sale_seq},#{ms_no},#{open_date},#{pos_no},#{bill_no},#{detail_index},#{corner_cd},#{menu_cd},#{price},#{qty},#{total_amt},#{sale_amt},#{vat_amt},'0','N')";
	
	not( setup[ms_no] ) {
		db.fetch(conf("sql#hitec.selectKioskSetup"),  setup);
	}
	
	
	order_dealNo(db, order, setup); 
	print("# 카드주문 시작 : $setup");
	
	
	order[pos_no]	= setup[pos_no];
	order[ms_no]		= setup[ms_no];
		
	
	order[sale_date]			= today;
	order[total_qty]      		= totalQty;
	order[total_amt]			= totalPrice;
	order[card_sale_tot]	= totalPrice;

	vat = totalPrice;
	vat /=1.1;
	order[sale_amt]		= vat.round();
	order[vat_amt]		= totalPrice - order[sale_amt];
	
	
	
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"), order);
	order.inject(ms_nm, biz_no, tel_no, bill_addr, master_nm, head_msg);
	
	order_pm_no = "$order[pos_no] - $order[bill_no]";
	
	
	card_str="21,04,21,";
	card_str.add("$ms_nm,$bill_addr,$biz_no,$tel_no ${master_nm},$dtm,${order_pm_no},$order[deal_no],"); 
	
	
	corner=_node('OrderCornerNode');
	not( corner.childCount() ) { 
		while( cur,  main.findTag('#CornerTab') ) {
			corner.addNode().varMap(cur, 'corner_cd, corner_nm, class_seq, clplu_nm2'); 
		}
	}
	
	while( cur, corner ) {
		cur.removeAll();
	}
	sum = 0;
	
	while( cur, items ) {
		sub=corner.findOne('corner_cd', cur[corner_cd]);
		not( sub ) {
			print("주문 내역를 코너별로 분리 오류: 노드=$cur");
			if( cur[corner_cd] ) {
				sub=corner.child(1);
			}
		}
		sub.addNode().varMap( cur, 'corner_cd, corner_nm,menu_cd, menu_nm, sale_price,qty');
	}	
	
	while(cur, corner ) {
		not( cur.childCount() ) continue;
		total_coner = 0;
		while(sub,cur){
			total_coner += sub[qty*sale_price];
		}	
		cur[total_coner] = total_coner;
	}
	
	detail_cnt = 0;
	while(cur, corner ) {
		not( cur.childCount() ) continue;
		while(sub,cur,n,0){
			sub[sum_price]=sub[qty*sale_price];
			if( sub[menu_nm] ) {
				name = sub[menu_nm].substr(0,8);
				menu_nm=name.replace(',','');			
			} else {
				menu_nm="상품명";
			}
			card_str.add("$menu_nm^$sub[sale_price]^$sub[qty]^$sub[sum_price]^$cur[total_coner]^${cur[clplu_nm2]}^${order_pm_no}\t");
			detail_cnt++;
		}
	}
	
	card_str.add(",$totalPrice,$order[sale_amt],$order[vat_amt],$totalPrice,$totalPrice,0");
	
	
	card_str.add(",$node[RS12],$node[RS07],$totalPrice,$node[RQ09],$node[RS09],$node[RS13],카드승인,${head_msg}");
	

	order[detail_cnt]	= detail_cnt;
	order[datetime]	= System.date('yyyyMMddHHmmdd');
 
		
	xmlStr = '<?xml version="1.0" encoding="euc-kr" ?><DRIM-RH2><TELEX-HD TELEX_ID="A10R" MSG_CD="0000" />';	
	src_h = '<HEADER SALE_DATE="${open_date}" MS_NO="${ms_no}" POS_NO="${pos_no}" BILL_NO="${bill_no}" REST_CD="${rest_cd}" OPER_CD="${oper_cd}" BR_CD="${br_cd}" SALE_FG="0" DATETIME="${datetime}" SALE_TOT="${total_amt}" SALE_AMT="${sale_amt}" CASH_AMT="0" CARD_AMT="${total_amt}" ETC_AMT="0" DC_AMT="0" DETAIL_CNT="${detail_cnt}" TABLE_NO="${deal_no}" SLIP_CNT="1" ORG_BILL_NO="">';

	_header = fmt(src_h, order);
	xmlStr.add(_header);
	print("# 전문 header 정보 == $_header");
 
 
	
	db.exec(headerSql, order);
	db.fetch("SELECT CURRVAL(pg_get_serial_sequence('tb_sale_header','sale_seq')) as sale_seq", order);
	error=db.error();
	if( error ) {
		order[error_data]="매출 헤더저장 오류: $error, 매출정보: $node";
		db.exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status) values( 'db', 'db', 'order', #{error_data}, 'R')", order);
	}
	
	
	src_d = '<DETAIL LINE_NO="${detail_index}" CLASS_CD="${corner_cd}" GOODS_CD="${menu_cd}" UPRICE="${price}" SALE_QTY="${qty}" SALE_TOT="${total_amt}" SALE_AMT="${sale_amt}" DC_AMT="0" />';
	order[detail_index]=0;
	while( sub, items ) {
		order[corner_cd]	= sub[corner_cd];
		order[menu_cd]		= sub[menu_cd];
		order[qty]				= sub[qty];
		order[price]			= sub[sale_price];
		order[total_amt]     = sub[qty*sale_price];
		
		_sale_vat = sub[qty*sale_price];
		_sale_vat /=1.1;
		order[sale_amt]			= _sale_vat.round();
		
		order[vat_amt]			= order[total_amt-sale_amt];
		order[detail_index++];
		db.exec(detailSql , order);
		
		_detail = fmt(src_d, order);
		xmlStr.add("\n$_detail");
		print("# 전문 detail 정보 == $_detail");
	}
	
	arv_dt=node[RS07];
	
   
	order[arv_dt]=arv_dt;
   
	order[trdata1]=node[RQ04];
	
	order[pur_card_no]=node[RS05];
	
	order[appr_no]=node[RS09];
	
	order[pur_card_nm]=node[RS12];	
 	
 	paymentSql="insert into tb_sale_payment (sale_seq,ms_no,open_date,pos_no,bill_no,arv_dt,trdata1,trdata5,trdata9,trdata11) values( #{sale_seq},#{ms_no},#{open_date},#{pos_no},#{bill_no},#{arv_dt},#{trdata1},#{pur_card_no},#{appr_no},#{pur_card_nm})";
	db.exec(paymentSql, order);
		
	error=db.error();
	if( error ) {
		order[error_data]="매출 payment 오류: $error, $node";
		db.exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status) values( 'db', 'db', 'order', #{error_data}, 'R')", order); 
	}
	
	
	if( order[trdata1] ) {
		cardNo=order[trdata1].value(0,6);
		order[card_no] = "${cardNo}******";
	} else {
		order[card_no] = "";
	}
	
	
	apprDate=arv_dt.value(0,6);
	order[appr_date] ="20${apprDate}";
	
	order[appr_time] =arv_dt.value(6,12);
			
	
	db.fetch(conf("sql#kiosk.hitec#VanCdSelect"), order);
	not( order[van_cd] ) {
		order[van_cd]='06'; 
	}
	
	
	db.fetch("select
		std_card_cd as std_card_cdrint_no, van_card_cd
		FROM hitec_m23s 
		where van_cd = #{van_cd}
    		and van_card_cd = #{pur_card_no}
    		and use_yn = 'Y'
		  order by log_seq desc
		   limit 1 offset 0", order);
	
	print("# 주문 정보 == $order");

	src_c =  '<SLIP><CARD SEQ="01" CARD_NO="${card_no}" INPUT_FG="0" APPR_AMT="${card_sale_tot}" APPR_NO="${appr_no}" APPR_DATE="${appr_date}" APPR_TIME="${appr_time}" VALID_TERM="202108" INST_MCNT="00" CARD_CD="${std_card_cdrint_no}" VAN_CD="${van_cd}" PUR_CARD_CD="${pur_card_no}" PUR_CARD_NM="${pur_card_nm}" /></SLIP></HEADER></DRIM-RH2>';	
	_card = fmt(src_c, order);
    print("# 전문 card 정보 == $_card");
    
	xmlStr.add("\n$_card") ;
	
    
    req=Cf[hitecReqNode];
    not( req ) {
    	req={  method:'POST', header: {} };
    	Cf[hitecReqNode]=req;
    }

	web=Class.web('kiosk');
	web[data] = xmlStr.kr();
	req[url]='http://61.78.39.134/telex_rh2/A10_Rcv.php';
	web.call( req, callback(type,data) {
		switch(type) {
			case read:
				if( data.find("MSG_CD='0000'") ) {
					if( xmlStr.find('="" ') ) {
						print("# 재전송 예약(항목없음) => $xmlStr");
					} else {
						db.exec("update tb_sale_header set send_yn='Y' where sale_seq=#{sale_seq} ", order);
					}
				}
				print("# 카드결제 전문 응답 =>$data ");
			case error:
				print("## 카드결제 전문 응답오류 =>$data",true);
		}
	});
	
	
	print("영수증 프린트 => $card_str");	
	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	main.qtMonSendData(card_str.kr() );
	while(n,10 ) {
		if( qrmonNode[data] ) {
			print("영수증 프린트 응답 : ($n)");
			break;
		}
		System.sleep(500);
	}
	
	dist=System.tick() - cf[orderStartTick];
	main._log("주문 처리시간 : ${dist} ms");
	
	
	if(cf[noSetupType].eq('1')) {
		order_kitchenPrint(order, corner, main, cornerInfo, dtm);	
	} else if(cf[noSetupType].eq('2')) {
		order_kitchenPrintNew(order, corner, main, cornerInfo, printInfo, dtm);
	}
	
	cf[orderStartTick]=0;
	System.timeout(250);
	main.popupClose();
	main.popupOpen('CompleteOrder');
	return true; 
	
}
KioskHiTec.order_kitchenPrint(order, corner, main, cornerInfo, time ) {

	
	sale_date	= order[open_date];
	ms_no		= order[ms_no];
	pos_no		= order[pos_no];
	bill_no		= order[bill_no];
	deal_no		= order[deal_no].toNumber();
	detail_no	= 1;
	
	orderMessage='';
	sendCnt=0, errorCnt=0;
	
	while(cur, corner, row, 0 ) {
		not( cur.childCount() ) continue;
		
		ipNode=cornerInfo.findOne('clplu_cd', cur[corner_cd] );
		not( ipNode ) {
			errorCnt++;
			continue;
		}
 		socket=ipNode[screen_socket];
 		not( socket ) {
 			if( ipNode[screen_ip] ) {
 				errorCnt++;
 			}
 			continue;
 		}
 		
 		not( socket.isConnect() ) {
 			if( orderMessage ) orderMessage.add(", ");
 			orderMessage.add(ipNode[clplu_nm]);
 			continue;
 		}
 		ipNode[data]='';
		data='';
		num=1, print_no=row+1;
		while( sub, cur, n ,0 ) {
			not(sub[menu_nm]){
				continue;
			}
			corner_cd	=sub[corner_cd];
			menu_cd		=sub[menu_cd];
			menu_nm		=sub[menu_nm];
			qty				=sub[qty];
			d1=lpad(detail_no,2), d2=lpad(num,2);
			data.add( "${ms_no}^${sale_date}^${pos_no}^${bill_no}^${d1}^${d2}^Y^${deal_no}^${print_no}^${corner_cd}${menu_cd}^${menu_nm}^${qty}^#" );
			num++, detail_no++;
		}		
		socket.send( data.kr() );
		System.sleep(250);
		not( ipNode[data] ) {
			System.sleep(250);
		}
		sendCnt++;
		print("주문 스크린 $ipNode[clplu_nm] 전송=>$data 응답: $ipNode[data]");
	}

	qrmonNode = _node('QtMonNode');

	
	
	not( time ) time=System.date('MM/dd HH:mm:ss');
		
	idx=1;
	while(cur, corner ) {
		not( cur.childCount() ) continue;
		
		send = "22,5,$order[pos_no],";
		send.add("$order[deal_no],$time,");
		
		while(sub,cur){
			not(sub[menu_nm]){
				continue;
			}
			send.add("$sub[menu_nm]^$sub[qty]\t");
		}
		
		ipNode=cornerInfo.findOne('clplu_cd', cur[corner_cd] );
		
		print("코너 정보 ipNode=$ipNode");
			
 		ip=ipNode[kitchen_ip1];
  		not( ip ) {
			if( orderMessage ) orderMessage.add(', ');
 			orderMessage.add(ipNode[clplu_nm]);
 			continue;
 		}
 		send.add(",$ip");

		print("주방프린터 출력 ip=$ip $send");
		qrmonNode[data] = null;
	
		main.qtMonSendData(send.kr());
	 	while(n, 6 ) {
  			System.sleep(500);
			if( qrmonNode[data]) break;
 		}
 		not( qrmonNode[data] ) {
 			print("# 주방프린터 출력중 오류가 발생했습니다 : $ipNode" );
 		}
		idx++;
	}
		
		
	if( orderMessage ) {
		order[orderMessage]=orderMessage;
	} else {
		if( sendCnt ) {
			while(cur, corner  ) {
				not( cur.childCount() ) continue;
				ipNode=cornerInfo.findOne('clplu_cd', cur[corner_cd] ); 
				not( ipNode[screen_socket] ) {
					continue;
				}
				not( ipNode[data] ) {
					if( orderMessage ) orderMessage.add(", ");
					orderMessage.add(ipNode[clplu_nm]);
				}
			}
		}
		if( orderMessage ) {
			orderMessage.add("\n주문스크린 응답 오류가 발생했습니다");
		} else if( errorCnt ) {
			orderMessage="주문스크리 출력중 오류가 발생했습니다.";
		}
		if( orderMessage ) {
			orderMessage.add("\n\n상품교환권을 가지고 관리자에게 문의하세요");
			order[orderMessage]=orderMessage;
		}
	}	
	print("############################ order_kitchenPrint END ##################################");

}
KioskHiTec.order_screenSend(order, corner, main) {

	return;

	
	serverInfo=getCommCodeNode('kiosk#serverInfo');
	time=System.date('MM/dd HH:mm:ss');
	
	socket=Class.socket('hitec');
	serverInfo=getCommCodeNode('kiosk#serverInfo');
	idx=1;
	ms_no		= order[ms_no];
	pos_no		= order[pos_no];
	bill_no		= order[deal_no];
	deal_no		= bill_no.toNumber();
	sale_date	=System.date('yyyyMMdd');
	print_no		='01';

	
	idx=1;
	while(cur, corner ) {
		not( cur.childCount() ) continue;
		
		si=serverInfo.findOne('code', "order_screen$idx");
		ip=si[data];
		not( socket.connect( ip, 2018) ) {
			main._log("#  order screen connect fail : $ip");
			continue;
		}
		data='';
		num=1;
		while( sub, cur, n ,0 ) {
			not(sub[menu_nm]){
				continue;
			}
			corner_cd	=sub[corner_cd];
			menu_cd	=sub[menu_cd];
			menu_nm	=sub[menu_nm];
			qty			=sub[qty];
			data.add( "${ms_no}^${sale_date}^${pos_no}^${bill_no}^0${num}^0$idx^Y^${deal_no}^${print_no}^${corner_cd}${menu_cd}^${menu_nm}^${qty}^#" );
			num++;
		}
		if( socket.isConnect() ) { 
			socket.sendBuffer( data.kr() );
			recv=socket.readBuffer();
			main._log("주문 스크린 $ip send=>$data\trecv=>$recv");
			socket.close();
		}
		idx++;
	}


}
KioskHiTec.order_dealNo(db, order, setup ) {

	
	
	db.fetch("SELECT max(open_date) as open_date FROM kiosk_open_close where close_date is null", order);
	not( order[open_date] ) { 
		order[open_date]=System.date('yyyyMMdd');
		db.exec("insert into kiosk_open_close(open_type,  open_date,  open_time,  reg_open_dtm, status) values( 'A', #{open_date}, '0000', now(), 'R')", order );
	}
	db.fetch( "select max(seq) as seq from tb_key_gen where key_type='DealNo' and key_date=#{open_date}", order);
	db.fetch( "select max(seq) as bseq from tb_key_gen where key_type='BillNo' and key_date=#{open_date}", order);
	
	print("[order_dealNo] 1) seq = $order[seq]");
	
	order[seq++];
	order[bseq++];
	
	print("[order_dealNo] 2) seq = $order[seq]");
	
	end=setup[order_end_no];
	if( end>0 ) {
		if( order[seq] > end ) {
			order[seq]=setup[order_start_no];
			
			print("[order_dealNo] 3) seq = $order[seq]");
		}
	} 
	order[bill_no]=lpad(order[bseq],4);
	order[deal_no]=lpad(order[seq],4);
	
	print("[order_dealNo] 4) deal_no = $order[deal_no]");
	
	not( db.exec("update tb_key_gen set seq=#{seq} where key_type='DealNo' and key_date=#{open_date} ", order) ) {
		if( setup[order_start_no] ) {
			order[seq]=setup[order_start_no];
			order[deal_no]=lpad(order[seq],4);
		}
		db.exec("insert into tb_key_gen (seq, key_type, key_date) values (#{seq}, 'DealNo', #{open_date})", order);
	}
	not( db.exec("update tb_key_gen set seq=#{bseq} where key_type='BillNo' and key_date=#{open_date} ", order) ) {
		db.exec("insert into tb_key_gen (seq, key_type, key_date) values (#{bseq}, 'BillNo', #{open_date})", order);
	}
	
	
	print("######## order_dealNo : $order ##########");

}
KioskHiTec.makeHeaderWidth(header, totalWidth ) {

	arr=_arr(), wa=_arr();
	wideNum=0;
	tw=totalWidth;
	while( cur, header ) {
		wid=cur[width];
		if( not(wid) ) {
			wideNum++;
			wa.add(9999);
			continue;
		}
		if( wid.find('px') ) {
			w=wid.find('px').trim();
			tw-=w;
			wa.add(w);
		} else if( wid.find('%') ) {
			w=wid.find('%').trim();
			w*=totalWidth;
			w/=100;
			tw-=w;
			wa.add(w);
		} else {
			arr.add(wid);
			wa.add(0);
		}
	}
	remain= totalWidth - tw;
	ww=50;
	if( wideNum ) {
		if( remain>0 ) {
			ww=remain/wideNum;
		}
	}
	arr.recalc(tw);
	widx=0;
	while( w, wa, n, 0 ) {
		switch( w ) {
		case 0:
			w=arr[$widx];
			wa[$n]=nvl(w, 50);
			widx++;
		case 9999:
			wa[$n]=ww;
		}
	}
	wa.recalc(totalWidth);
	while( cur, header, n, 0 ) {
		cur[width]=wa[$n];
	}
	return wa;

}
KioskHiTec.rectRateArray(rect, rate, vbox) {

	arr=_arr(), rst=_arr();
	tw=when( vbox, rect.height(), rect.width() );
	if( rate.find('*') ) {
		wideNum=0;
		while( a, rate.split() ) {
			if( a.eq('*') ) {
				wideNum++;
				arr.add(0);
			} else {
				arr.add(a);
			}
		}
		if( wideNum ) {
			sum=arr.sum();
			remain=tw-sum;
			remain/=wideNum;
			if( remain>0 ) {
				while( a, arr, n, 0 ) {
					not( a ) {
						arr[$n]=remain;
						break;
					}
				}
			}
		}
	} else {
		arr.recalc(tw, rate);
	}	
	rect.inject( sx, sy, sw, sh);
	if( vbox ) {
		while( h, arr ) {
			rst.add( Class.rect(sx, sy, sw, h) ), sy+=h;
		}
	} else {
		while( w, arr ) {
			rst.add( Class.rect(sx, sy, w, sh) ), sx+=w;
		}
	}
	return rst;

}
KioskHiTec.tagRect( tag, clear ) {
	if( clear ) {
		tagClearRect(tag);
	}
	setNodeSize(tag, true);
	confNodeLayout(tag);
	return tag;
}
KioskHiTec.rectVCenter(rc, h, gab) {
	w=rc.width();
	if( gab ) w-=gab;
	return rc.center(w,h);
}
KioskHiTec.kioskImage(code) {
 
	node=_node('MainImages'); 
	img=node[@$code];
	if( img ) return img;
	return imageLoad(node, code); 

}
KioskHiTec.watcherBatch(page, db) {
	page.inject(cf);
	not( cf[ms_no] ) {
		cf[lastUpdateTick] = System.tick();
		
		db.exec("delete from kiosk_error");
		db.exec("update watcher_ping_error set status='S' where status='R' ");
		db.fetch("select ms_no, pos_no, emp_id, emp_pw from kiosk_setup where use_yn='Y'", cf); 
		return;
	}
	index=cf[batchIndex++];
	errorNode=_node(page, 'kioskErrorNode');
	
	
	not( System.isConnect() ) {
		not( errorNode[connectFail] ) {
			errorNode[connectFail]=true;
			errorNode[error_data]='네트워크에 연결되지 않았습니다. 네트워크 연결을 확인하세요';
		}
		db.exec( conf("sql#hitec.networkError"),errorNode );
		return;
	}
	
	if( index.eq(10) ) {
		page.updateStart(true);
		cf[lastUpdateTick] = System.tick();
		return;
	}
	
	if( errorNode[connectFail] ) {
		errorNode[connectFail]=false;
		db.exec("delete from kiosk_error where error_kind='not connect' and error_nm='internet'");
	}
		
	
	dist=System.tick() - cf[lastUpdateTick];
	if( dist > 50000 ) {
		hh=System.date('hh').toNumber();
		if( hh<7 ) {
			if( dist<350000 ) {
				return;
			}
		} else if( hh>21 ) {
			if( dist<180000 ) {
				return;
			}			
		}
		file=Class.file();
		if( file.isFile('data/func.c') ) {
			src=file.readAll('data/func.c');
			print("사용자 함수 호출 : $src");
			file.delete('data/func.c');
			Cf.call(src);
			cf[lastUpdateTick] = System.tick();
			return;
		}
		page.updateStart(true);
		cf[lastUpdateTick] = System.tick();
	}
}
KioskHiTec.setupProcess(db, cf) {

	setup=cf[SetupInfo];
	db.fetch("
	   SELECT A.ms_no, A.pos_no, A.service_start_time, A.service_end_time, A.refresh_time, A.order_start_no, A.order_end_no,
		 			A.qt_mon_ip, A.qt_mon_port, A.emp_id, A.emp_pw, A.kiosk_id, A.kiosk_pw, 
		 			B.van_cd, B.ms_cat_id, C.kiosk_ver
  		  FROM kiosk_setup A
  		   LEFT JOIN hitec_m60s B ON A.ms_no = B.ms_no AND A.pos_no = B.pos_no AND B.use_yn = 'Y'
  		   LEFT JOIN tb_kiosk_version C ON A.ms_no = C.ms_no AND A.pos_no = C.pos_no
 		WHERE A.use_yn='Y'
 		  LIMIT 1",  setup);
	not( setup[ms_no ) {
		return;
	}
 	if( setup[qt_mon_ip] ) {
 		not( setup[qt_mon_ip].eq(cf.qtMonHost) ) {
			conf('setup#kiosk.qtMonHost', setup[qt_mon_ip], true);
		}
		cf.qtMonHost		= setup[qt_mon_ip];
 	}
 	if( setup[qt_mon_port] ) {
 		not( setup[qt_mon_port].eq(cf.qtMonPort) ) {
			conf('setup#kiosk.qtMonPort', setup[qt_mon_port], true);
		}
		cf.qtMonPort		= setup[qt_mon_port];
 	}
 	info=cf[CornerInfo];
	db.fetchAll(conf("sql#hitec.CornerInfo"), info.removeAll() ); 

}
KioskHiTec.kiosk_SendError(main, msg, code, cf, db) {






	if( cf[errorSendTick] ) {
		dist= System.tick() - cf[errorSendTick];
		if( dist<1000 ) {
			return;
		}
	}
	cf[errorSendTick] = System.tick();
	
	not( db ) {
		db=Class.db('kiosk_hitec');
	}
	setup=cf[SetupInfo];
	setup[sale_date]=System.date('yyyyMMdd');
	setup[reg_dtm]=System.date('yyyyMMddHHmmss');
	setup[info_gb]='1';
	not( setup[rest_cd] ) {
		db.fetch("select rest_cd FROM hitec_m40s where ms_no=#{ms_no} ", setup);
	}
	
	if( code.eq('01','06') ) {
		setup[error_flag]="1";
	} else {
		setup[error_flag]="2";
	}
	setup[error_code]="RP0$code";	
	setup[error_msg]=msg;
	
	main.alert("<font size=18>$msg</font>",'오류', true);
	print("# 에러전송 == $msg", true);
	
	
	db.exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status) values( 'hitec', 'hitec', 'error', '$msg', 'R')");

	req=Cf[HitecReqNode];
	xml=conf("page#xml.hitec#SendError");
	send=fmt(xml, setup);
	
	not( req ) {
		req={  method:'POST', header: {} };
		Cf[HitecReqNode]=req;
	}
	req[url]='http://61.78.39.134/telex_rh2/K10_Rcv.php';
	
	web=Class.web('hitecError');
	web[data] = send.kr();
	web.call( req, callback(type,data) {
		switch(type) {
			case read:
				data = data.utf8();
				print("# 에러전문 응답 ==> ################################  $data ####################################");	
			case error:
				print("## 에러전문 전문 응답오류 => $data",true);
		}
	});
	print("xxxxxxxxxxxxx kiosk_SendError : $send xxxxxxxxxxxx");






}
KioskHiTec.kiosk_SaleClose(main, setup, db) {

	db.fetch("select to_char(reg_open_dtm,'yyyymmddHH24mi') as open_dtm , to_char(reg_close_dtm,'yyyymmddHH24mi') as close_dtm from kiosk_open_close where open_date=#{open_date}", setup);


	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	s = '21,11,29,';
	
	
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"),  setup);
	
	s.add("$setup[ms_nm],$setup[biz_no],$setup[open_date],$setup[pos_no],$setup[master_nm],$setup[open_dtm],$setup[close_dtm],");  
	
	db.fetch("select sum(total_amt) as sale_total , count(*) as sale_count  from tb_sale_header where open_date = #{open_date}", setup);
	s.add("$setup[sale_total],0,$setup[sale_total],0,");
	
	db.fetch("select sum(total_amt) as cancel_total, count(*) as cancel_count  from tb_sale_header where cancle_yn = 'Y' and open_date = #{open_date}", setup);
	s.add("$setup[cancel_total],0,$setup[cancel_total],");
	print("setup ------------------------------------===> $setup");
	
	db.fetch(conf("sql#kiosk.hitec#saleAmt"), setup);
	
	s.add("$setup[sale_real_total],0,$setup[sale_real_total],$setup[kong_sum_amt],$setup[bu_sum_amt],$setup[sale_count],$setup[cancel_count],");
	
	s.add('0^0^0^0^0	,0^0^0^0^0	,0^0^0^0	,0^0^0^0^0	,0^0^0^0	,0,0,0');

	main._log("# 마감 전표 == $s");

	main.qtMonSendData(s.kr() );
		
	while(n,10){
		System.sleep(1000);
		if(qrmonNode[data]) break;
	}
	
	not( db ) {
		db=Class.db('kiosk_hitec');
	}
	db.fetch(conf("sql#kiosk.hitec#maxBillNo"), setup);
	not( setup[ms_no] ) {
		main.inject(cf);
		setup[ms_no]=cf[ms_no];
		setup[pos_no]=cf[pos_no];
	}
	not( setup[rest_cd] ) {
		db.fetch("select rest_cd FROM hitec_m40s where ms_no=#{ms_no} ", setup);
	}
	
	
	setup[sale_date] = setup[open_date];	
	setup[tot_amt_cnt] = setup[sale_count] + setup[cancel_count];
	setup[sale_tot_cnt] = setup[sale_count];
	not( setup[bill_no] ) {
		setup[bill_no]='0000';
	}

	xml=conf("page#xml.hitec#SaleClose");
	send=fmt(xml, setup);
	print("# 마감 전문: $send");
	
	req=Cf[HitecReqNode];
	not( req ) {
		req={  method:'POST', header: {} };
		Cf[HitecReqNode]=req;
	}
	req[url]='http://61.78.39.134/telex_rh2/T40_Rcv.php';
	
	web=Class.web('hitecSend');
	web[data] = send;
	web.call( req, callback(type,data) {
		switch(type) {
			case read:
				data = data.utf8();
				main._log("# 정산전문 응답 ==> ################################  $data ####################################");	
			case error:
				file=Class.file('test');
				file.writeAll("data/saleCloseResend.txt", send);
				file.copy("data/src/saleCloseResend.c","data/func.c");
				main._log("## 정산전문 전문 응답오류 => $data",true);
		}
	});
	print("xxxxxxxxxxx kiosk_SaleClose ok xxxxxxxxxxxxxxxxxxxx");


}
KioskHiTec.cancle_completeCardProcess(main, record) {
	
	db=Class.db('kiosk_hitec');
	
	
	db.fetch("select open_date as new_open_date from kiosk_open_close where close_date is null", record);
	db.fetch("select seq as bseq from tb_key_gen where key_type='BillNo' and key_date=#{new_open_date}", record);	

	record[bseq++];
	record[cancle_bill_no] = lpad(record[bseq],4);
  	
 	not( db.exec("update tb_key_gen set seq=#{bseq} where key_type='BillNo' and key_date=#{new_open_date} ", record) ) {
		db.exec("insert into tb_key_gen (seq, key_type, key_date) values (#{bseq}, 'BillNo',#{new_open_date})", record);
	}
  	cancle_bill_no = record[cancle_bill_no];

	db.exec("update tb_sale_header set cancle_bill_no=#{cancle_bill_no}, cancle_yn='Y' , cancle_date = now() where bill_no=#{bill_no} and open_date=#{open_date}", record) ;
	
	main._log("record ===> $record");

	s = "21,07,21,";
	
	datetime	= System.date('MM/dd HH:mm:dd');
	
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"), record);
	record.inject(ms_nm, biz_no, tel_no, bill_addr, master_nm, head_msg);
	
	s.add("$ms_nm,$bill_addr,$biz_no,$tel_no ${master_nm},$datetime,");
	
	model=Class.model('CardCancle');
	root=model.rootNode();
	
	root[bill_no]	= record[bill_no];
	root[open_date]	= record[open_date];
	
	xmlStr = '<?xml version="1.0" encoding="euc-kr" ?><DRIM-RH2><TELEX-HD TELEX_ID="A10R" MSG_CD="0000" />';
	datetime     = System.date('yyyyMMddHHmmdd');
	
	sale_fg      = 1;
	
	rest_cd = record[rest_cd];
	br_cd = record[br_cd];
	oper_cd = record[oper_cd];
	
	
	src_h ='<HEADER SALE_DATE="${today}"  MS_NO="${ms_no}"  POS_NO="${pos_no}"  BILL_NO="${cancle_bill_no}" REST_CD="${rest_cd}" OPER_CD="${oper_cd}"  BR_CD="${br_cd}" SALE_FG="${sale_fg}" DATETIME="${datetime}" SALE_TOT="${sale_tot}"  SALE_AMT="${sale_amt}" CASH_AMT="0"  CARD_AMT="${card_amt}"  ETC_AMT="0"  DC_AMT="0" DETAIL_CNT="${detail_cnt}" TABLE_NO="${deal_no}" SLIP_CNT="1" ORG_BILL_NO="${org_bill_no}">';

	db.fetchAll( conf("sql#hitec.CardCancle#header"), root.removeAll() );
	while(cur,root) {
		today     	= cur[open_date];
		ms_no    	= cur[ms_no];
		pos_no		= cur[pos_no];
		
		record[pos_no] = pos_no;
		
		bill_no		= cur[bill_no];
		
		sale_amt   	= cur[total_amt];
		sale_amt /=1.1;
		sale_amt   	= sale_amt.round();		
		
		card_amt   = cur[total_amt];
		sale_tot     	= cur[total_amt];
		detail_cnt  	= cur[detail_cnt];
		deal_no		= cur[deal_no];
		pm_no = "$cur[pos_no]-${bill_no}";
		
		s.add("${pm_no},$cur[deal_no],");
		org_bill_no =  "${today}${ms_no}${pos_no}${bill_no}";
		
		_header = fmt(src_h);
		xmlStr.add("$_header");
		
		main._log("cur ===>  $cur");
		main._log("header ==> $_header");
	}
	
	db.fetchAll( conf("sql#hitec.CardCancle#detail"), root.removeAll() );
	src_d =  conf('page#xml.kiosk#detail');
	line_no = 0;
	
	_model=Class.model('GoodsCancle');
	goods=_model.rootNode();

	while(cur,root){
		line_no++;
		goods_cd = cur[menu_cd];
		class_no = cur[corner_cd];
		
		uprice = cur[price];
		sale_qty = cur[qty];
		total_amt = uprice*sale_qty;
		
		cancle_amt = total_amt;
		cancle_amt /=1.1;
		cancle_amt = cancle_amt.round();
		
		goods[goods_cd] = cur[menu_cd];
		db.fetch( conf("sql#hitec.corner#goods"), goods.removeAll() );
	
		goods_nm = goods[goods_nm];
		corner_nm = goods[clplu_nm];
		goods_nm = goods_nm.substr(0,8);
		s.add("${goods_nm}^${uprice}^${sale_qty}^${total_amt}^${total_amt}^${cur[clplu_nm2]}^$cur[bill_no]\t");
		src_d.inject(line_no, class_no,goods_cd,uprice,sale_qty,total_amt,cancle_amt);
		_detail = fmt(src_d);
		
		xmlStr.add("$_detail");
		main._log("detail ==> $_detail");
	}

	getCardNo=func(s) {
		if(s.eq('')){
			return "";
		}else{
			cn=s.value(0,6);
			return "${cn}******";
		}
	};
	
	getApDate=func(s) {
		yy=s.value(0,2), mm=s.value(2,4), dd=s.value(4,6);
		return "20${yy}${mm}${dd}"; 
	};
	
	getApTime=func(s) {
		hh=s.value(6,8), mm=s.value(8,10), ss=s.value(10,12);
		return "${hh}${mm}${ss}";
	};
	
	db.fetchAll( conf("sql#hitec.CardCancle#card"), root.removeAll() );
	src_c =  conf('page#xml.kiosk#card');
	
	while(cur,root){
		card_no = getCardNo(cur[trdata1]);
		
		card_total_amt = cur[total_amt];
		
		card_sale_amt    = cur[total_amt];
		card_sale_amt /=1.1;
		card_sale_amt    = card_sale_amt.round();
		
		vat = card_total_amt - card_sale_amt;
	   
	   appr_date = getApDate(cur[sale_date]);
	   appr_time = getApTime(cur[sale_date]);
	   
	   appr_no = cur[trdata9];
	   pur_card_no = cur[trdata5];
	   pur_card_nm =cur[trdata11];
	   res_no = cur[trdata13];
	   
	   record[pur_card_no] = pur_card_no;
	
		main._log("1) record =====> $record");
		
		
		db.fetch(conf("sql#kiosk.hitec#VanCdSelect"), record);
		van_cd = record[van_cd];
		
		db.fetch("select
			std_card_cd as std_card_cdrint_no, van_card_cd
			FROM hitec_m23s 
			where van_cd = #{van_cd}
    			and van_card_cd = #{pur_card_no}
    			and use_yn = 'Y'
			  order by log_seq desc
			 limit 1 offset 0", record);
		std_card_cdrint_no = record[std_card_cdrint_no];
		van_card_cd = record[van_card_cd]; 
	
		main._log("2) record =====> $record");
	   
	   s.add(",$card_total_amt,$card_sale_amt,$vat,$card_total_amt,$card_total_amt,0");
	   
	   s.add(",$pur_card_nm,$appr_date,$card_total_amt,일시불,$appr_no,$res_no,카드승인,${head_msg}");
	   
		src_c.inject(card_no,card_total_amt, appr_no,appr_date,appr_time,std_card_cdrint_no,van_cd,van_card_cd,pur_card_nm);
		_card = fmt(src_c);
		xmlStr.add("$_card") ;
		
		main._log("card ==> $_card");
	}
	
	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	
	main._log("# 카드영수증 정보 == $s");
	
	main.qtMonSendData(s.kr() );
	
	 while(n,10){
			System.sleep(1000);
			if(qrmonNode[data]) break;
		}
	
	req=this[HitecWebNode];
	not( req ) {
		req={  method:'POST', header: {} };
		this[HitecWebNode]=req;
	}
	web=Class.web('kiosk');
	web[data] = xmlStr.kr();
	req[url]='http://61.78.39.134/telex_rh2/A10_Rcv.php';
	me=this;
	web.call( req, callback(type,data) {
		switch(type) {
			case read:
				data = data.utf8();
				main._log("data ==> ################################  $data ####################################");
			case finish:
	
			case error:
				main._log("## 카드결제 data 정보 error == $data",true);
		}
	});
	
   	if( record[cancle_yn].eq('Y') ) {
   		main.alert("카드취소 영수증 방출", "알림");
   	} else {
   		
   		if(cf[noSetupType].eq('1')) {
   			
   			cancle_kitchenPrint(main, record);		
   			
   		} else if(cf[noSetupType].eq('2')) {
   			
   			cancle_kitchenPrintNew(main, record);
   			
   		}
   		main.alert("카드취소 결제 성공", "알림");
   	}
	
	main.findControl('Content#AdminSaleStatus').search();

}
KioskHiTec.cancle_kitchenPrint(main, record) {

	print("# 주문취소 주방 프린터 $record ");

	node=_node(main, 'CancleOrderNode');
	
	db=Class.db('kiosk_hitec');
	
	info=_node(main, 'CornerInfo');
	not( info.childCount() ) {
		db.fetchAll(conf("sql#hitec.CornerInfo"), info.removeAll() );
	}
 	while( ipNode, info ) {
 		ipNode.removeAll();
 	}
 
	time=System.date('MM/dd HH:mm:ss');	

	
	qtmonNode = _node('QtMonNode');
	
	node.initNode(record);
	
	print("# 주문취소 상품정보: $node ");
	db.fetchAll( conf('sql#hitec.CardCancle#kitchenPrint'), node );
	not( node.childCount() ) {
		main[page].alert("주문 상품정보가 없습니다");
		print("주방 취소 주문 상품정보가 없습니다");
		return;
	}
	origin_no		= record[bill_no]; 
 	 
	prev='', str='';
	while( cur, node ) {
		ipNode=info.findOne('clplu_cd', cur[corner_cd]);
	print("node=ipNode=$ipNode cur=$cur============");
		ipNode.addNode().varMap( cur, 'corner_cd, menu_cd, goods_nm, price,qty');
	}
		
		
	print("맵핑 루트정보 ====$info");
	while( ipNode, info ) {
		
		not( ipNode.childCount() ) continue;
		ip=ipNode[kitchen_ip1];
		not( ip ) continue;
		str="23,5,$record[pos_no],";
		str.add("$record[deal_no],$time,");
		while( sub, ipNode ) {
			str.add("$sub[goods_nm]^$sub[qty]\t");
		}
		str.add(",$ip");		
		main._log("# 취소 주방프린터 출력 ip=$ip $str");
		qtmonNode[data] = null;

		main.qtMonSendData(str.kr());
		while(n,10){
			System.sleep(1000);
			if( qtmonNode[data]) break;
		}
		not( qtmonNode[data] ) {
			main._log("# 취소 주방프린터 출력중 오류가 발생했습니다");
		}		
	}
	
	
	main._log("# 취소 주방스크린 출력 시작");
		
	socket = Class.socket('hitec1');
	ms_no		= record[ms_no];
	open_date	= record[open_date];
	new_open_date = record[new_open_date];
	pos_no		= record[pos_no];
	bill_no		= record[cancle_bill_no];
	deal_no		= record[deal_no].toNumber(); 
	
	detail_no=1;
	
	while( ipNode, info ) {
		not( ipNode.childCount() ) continue; 
		ip=ipNode[screen_ip];		
		not( ip ){
			 continue;
		}		
		data='';
		num=1, print_no=row+1;
		while( sub, ipNode, n ,0 ) {
 			corner_cd		=sub[corner_cd];
			menu_cd		=sub[menu_cd];
			menu_nm		=sub[goods_nm];
			qty				=-1*sub[qty];
			d1=lpad(detail_no,2), d2=lpad(num,2);
			data.add("${ms_no}^${new_open_date}^${pos_no}^${bill_no}^${d1}^${d2}^N^${deal_no}^${print_no}^${corner_cd}${menu_cd}^${menu_nm}^${qty}^${ms_no}${open_date}${pos_no}${origin_no}#");
			num++, detail_no++;
		} 
		print("# 주문스크린 아이피 : $ip, $data");
		
		not( socket.connect( ip, 2018, 10) ) {
			main._log("## 주문스크린 연결오류 : $ip");
			continue;
		}
		if( socket.isConnect() ) { 
			socket.sendBuffer( data.kr() );
			main._log("# 취소 주문 스크린 $ip 보내기=>$data");
			recv=socket.readBuffer();
			main._log("# 취소 주문 스크린 결과 =>$recv");
			socket.close();
		}		
	}

	print("############################ cancle_kitchenPrint END ##################################");

}
KioskHiTec.util_formatDate(s) {

	if( s.size().eq(8) ) {
		yy=s.value(0,4), mm=s.value(4,6), dd=s.value(6);
		return "$yy-$mm-$dd";
	}
	return '';

}
KioskHiTec.updateDidCheck(db, cf, dist) {

	if( dist<180000 ) {
		return;
	}	
	cf.inject( imagePath );	
	
	adNode=_node(cf, 'AdImages');
	db.fetchAll( conf("sql#hitec.adImage"), adNode.removeAll() );
	while( cur, adNode ) {
		code=cur[set_cd];
		if( cur[set_val] ) {
			
			imgSrc="$imagePath/menus/kiosk/$cur[set_val]";
			not( imgSrc.eq(menuNode[$code]) ) {
				_log("# 광고이미지 추가 src=$imgSrc");
				if( menuNode[@$code] ) {
					menuNode[@$code]=null;
				}
				menuNode[$code]=imgSrc;
			}
		}
	}
	print("############## updateDidCheck #################");




}
KioskHiTec.kiosk_saleClosePrint( main, setup, db ) {
	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	s = '21,11,29,';
	
	
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"),  setup);
	
	s.add("$setup[ms_nm],$setup[biz_no],$setup[open_date],$setup[pos_no],$setup[master_nm],$setup[open_date]$setup[open_time],$setup[close_date]$setup[close_time],");  
	
	db.fetch("select sum(total_amt) as sale_total , count(*) as sale_count  from tb_sale_header where open_date = #{open_date}", setup);
	s.add("$setup[sale_total],0,$setup[sale_total],0,");
	
	db.fetch("select sum(total_amt) as cancel_total, count(*) as cancel_count  from tb_sale_header where cancle_yn = 'Y' and open_date = #{open_date}", setup);
	s.add("$setup[cancel_total],0,$setup[cancel_total],");
	print("setup ------------------------------------===> $setup");
	
	db.fetch(conf("sql#kiosk.hitec#saleAmt"), setup);
	s.add("$setup[sale_real_total],0,$setup[sale_real_total],$setup[kong_sum_amt],$setup[bu_sum_amt],$setup[sale_count],$setup[cancel_count],");
	
	s.add('0^0^0^0^0	,0^0^0^0^0	,0^0^0^0	,0^0^0^0^0	,0^0^0^0	,0,0,0');

	main._log("# 마감 정보 == $s");

	main.qtMonSendData(s.kr() );
		
	while(n,10){
		System.sleep(1000);
		if(qrmonNode[data]) break;
	}
}
KioskHiTec.util_pageFuncSave( pageGroup, pageCode, &s, note, root ) { 
	not( root ) root=_node();
	root[tm]=System.localtime();
	err='';
	db=Class.db('pages');
	root[pageGroup]=pageGroup, root[pageCode]=pageCode;
	while( s.valid() ) {
		c=s.ch();
		not( c ) break;
		if( c.eq('/') ) {
			if( s.ch(1).eq('/') ) note.add( s.findPos("\n") );
			else if( s.ch(1).eq('*') ) note.add( s.match() );
			continue;
		}
		sp=s.cur(), s.move();
		c=s.ch();
		if( c.eq('.') ) {
			s.incr(), s.move();
			c=s.ch();
		}
		ep=s.cur();
		funcName=s.value(sp,ep,true);
		not( c.eq('(') ) {
			err.add("함수 시작오류 : 함수명 : $funcName\n");
			break;
		}
		funcParam=s.match().trim();
		c=s.ch();
		not( c.eq('{') ) {
			err.add("함수 매개변수 오류: $funcName, $funcParam\n");
			break;
		}
		body=s.match(1);
		root.put( funcName, funcParam, note );
		if( body.find('/*') || body.find('// ') ) {
			root[funcSrc]=body;
			root[funcData]=makeSrc(body);
		} else {
			root[funcSrc]='';
			root[funcData]=body;
		}
		num=db.exec("update pageFunc set funcSrc=#{funcSrc}, funcData=#{funcData}, funcParam=#{funcParam}, note=#{note}, tm=#{tm} where cmsCode=#{pageGroup} and pageCode=#{pageCode} and funcName=#{funcName}", root);
		not( num ) {
			if( funcName.eq('onInit') ) {
				root[sort]=1;
			} else if( funcName.start('on') ) {
				root[sort]=2;
			} else if( funcName.find('.') ) {
				root[sort]=3;
			} else {
				root[sort]=4;
			}
			db.exec("insert into pageFunc( cmsCode, pageCode, funcName, funcParam, funcSrc, funcData, note, tm, sort ) values (#{pageGroup}, #{pageCode}, #{funcName}, #{funcParam}, #{funcSrc}, #{funcData}, #{note}, #{tm}, #{sort} )", root);
		}
		note=''; 
	}
	return err;
}
KioskHiTec.util_classFuncSave( classGroup, className, &s, note, root ) { 
	tm=System.localtime();
	not( root ) root=_node();
	err='';
	db=Class.db('pages');
	root[class_grp]=classGroup, root[class_name]=className;
	while( s.valid() ) {
		c=s.ch();
		not( c ) break;
		
		if( c.eq('/') ) {
			if( s.ch(1).eq('/') ) note.add( s.findPos("\n") );
			else if( s.ch(1).eq('*') ) note.add( s.match() );
			continue;
		}
		func=s.move();
		c=s.ch();
		not( c.eq('(') ) {
			err.add("함수 시작오류 : 함수명 : $func");
			break;
		} 
		param=s.match().trim();
		c=s.ch();
		not( c.eq('{') ) {
			err.add("함수 매개변수 오류: $param");
			break;
		}
		body=s.match(1);
		root.varMap('class_func: func, class_param: param, note');
		if( body.find('//') || body.find('/*') ) {
			root[class_src]=body;
			root[class_data]=makeSrc(body);
		} else {
			root[class_src]='';
			root[class_data]=body;
		}
		table="class_info", ok=false;
		not( ok ) {
			num=db.exec("update ${table} set class_src=#{class_src}, class_data=#{class_data}, class_param=#{class_param}, note=#{note}, tm='$tm' where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", root);
			not( num ) {
				db.exec("insert into ${table} ( class_grp, class_nm, class_func, class_param, class_src, class_data, note, tm ) values (#{class_grp}, #{class_nm}, #{class_func}, #{class_param}, #{class_src}, #{class_data}, #{note}, '$tm')", root);
			}
		}
		note='';
	}
	return err;
}
KioskHiTec.util_userFuncSave( funcGroup, &s  ) {
 
	node=_node();
	err='', note='';
	db=Class.db('pages');
	
	node[func_grp]=funcGroup, node[tm]=System.localtime();

	while( s.valid() ) {
		ok=false;
		c=s.ch();
		if( c.eq('/') ) {
			if( s.ch(1).eq('/') ) {
				node[note]=s.findPos("\n").trim();
			} else if( s.ch(1).eq('*') ) {
				node[note]=s.match().trim();
			}
			c=s.ch();
		}
		node[func_nm]=s.move().trim();
		if( s.ch().eq('(') ) {
			param=s.match();
			if( s.ch().eq('{') ) {
				src=s.match(1);
				if( src.find('/*') || src.find('//') ) {
					node[funcSrc]=makeSrc(src);
				}
				node[src]=src;
				node[func_param]=param;
				ok=true;
			}
		} else {
			error="함수 시작오류";
			break;
		}
		if( ok ) {
			not( db.exec( conf('sql#dev.funcUpdate'), node) ) {
				db.exec( conf('sql#dev.funcInsert'), node);
			}
		}
		note='';
	}
	return err;

}
KioskHiTec.kiosk_saleConerPrint( main, setup, db ) {


	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	s = '21,11,29,';
	
	
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"),  setup);
	getOpenDate=func(s) {
		yyyy=s.value(0,4), mm=s.value(4,6), dd=s.value(6,8);
		return "${yyyy} / ${mm} / ${dd}"; 
	};

	str = '21,13,3,';
	db=Class.db('kiosk_hitec');
	node=_node();
   node[open_date] = setup[open_date];
   opd = getOpenDate(node[open_date]);
   print(opd);
	db.fetchAll( conf("sql#hitec.SaleCloseCorner"), node);
	prev='', root={}, corner=null;
	arr=_arr();
	not( node.childCount() ) {
		main.alert('영업일자에 해당하는 매출이 없습니다.');
		return;
	}
	
	while( cur, node ) {
		not( prev.eq(cur[corner_cd]) ) {
			corner=root.addNode({tag:corner});
			corner.varMap(cur, 'corner_cd, clplu_nm');
			prev=cur[corner_cd];
		}
		menu=corner.addNode(cur);
		menu[tag]='menu';
	}
	
	while( cur, root ) {
		totalPrice = 0;
		totalQty=0;
		while( sub , cur ) {
			totalPrice += sub[price];
			totalQty += sub[qty];
		}
		str.add("$cur[clplu_nm]^${opd}^${totalQty}^${totalPrice}^");
		while( _sub , cur ) {
			str.add("$_sub[goods_nm]!$_sub[qty]!$_sub[price]@");
		}
		str.add("\t");
	}

	db.fetch( conf("sql#kiosk.hitec#saleAmt"), setup);
	main._log("====> $setup");
	str.add(",$setup[sale_real_qty],$setup[sale_real_total]");
	
	main._log("# 마감 정보 == $str");

	main.qtMonSendData(str.kr() );
		
	while(n,10){
		System.sleep(1000);
		if(qrmonNode[data]) break;
	}
	
	

}
KioskHiTec.cancle_cardPrint(main,record) {


	print("##########################record ===>  $record ##########################################");
	db=Class.db('kiosk_hitec');
	s = "21,07,21,";
	
	datetime	= System.date('MM/dd HH:mm:dd');
	
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"), record);
	record.inject(ms_nm, biz_no, tel_no, bill_addr, master_nm, head_msg);
	
	s.add("$ms_nm,$bill_addr,$biz_no,$tel_no ${master_nm},$datetime,");
	
	model=Class.model('CardCancle');
	root=model.rootNode();
	
	root[bill_no]	= record[bill_no];
	root[open_date]	= record[open_date];
	
	datetime     = System.date('yyyyMMddHHmmdd');
	
	sale_fg      = 1;
	
	rest_cd = record[rest_cd];
	br_cd = record[br_cd];
	oper_cd = record[oper_cd];
	
	db.fetchAll( conf("sql#hitec.CardCancle#header"), root.removeAll() );
	while(cur,root){
		today     = cur[open_date];
		ms_no    = cur[ms_no];
		pos_no	= cur[pos_no];
		
		record[pos_no] = pos_no;
		
		bill_no			=cur[bill_no];
		sale_amt    = cur[total_amt];
		sale_amt /=1.1;
		sale_amt    = sale_amt.round();
		card_amt    = cur[total_amt];
		sale_tot     = cur[total_amt];
		detail_cnt   = cur[detail_cnt];
		pm_no = "$cur[pos_no]-${bill_no}";
		
		s.add("${pm_no},$cur[deal_no],");
		org_bill_no =  "${today}${ms_no}${pos_no}${bill_no}";

		main._log("cur ===>  $_cur");
	}
	
	db.fetchAll( conf("sql#hitec.CardCancle#detail"), root.removeAll() );
	line_no = 0;
	
	_model=Class.model('GoodsCancle');
	goods=_model.rootNode();

	while(cur,root){
		line_no++;
		goods_cd = cur[menu_cd];
		class_no = cur[corner_cd];
		
		uprice = cur[price];
		sale_qty = cur[qty];
		total_amt = uprice*sale_qty;
		
		cancle_amt = total_amt;
		cancle_amt /=1.1;
		cancle_amt = cancle_amt.round();
		
		goods[goods_cd] = cur[menu_cd];
		db.fetch( conf("sql#hitec.corner#goods"), goods.removeAll() );
	
		goods_nm = goods[goods_nm];
		corner_nm = goods[clplu_nm];
		goods_nm = goods_nm.substr(0,8);
		s.add("${goods_nm}^${uprice}^${sale_qty}^${total_amt}^${total_amt}^${cur[clplu_nm2]}^$cur[bill_no]\t");
	}

	getCardNo=func(s) {
		if(s.eq('')){
			return "";
		}else{
			cn=s.value(0,6);
			return "${cn}******";
		}
	};
	
	getApDate=func(s) {
		yy=s.value(0,2), mm=s.value(2,4), dd=s.value(4,6);
		return "20${yy}${mm}${dd}"; 
	};
	
	getApTime=func(s) {
		hh=s.value(6,8), mm=s.value(8,10), ss=s.value(10,12);
		return "${hh}${mm}${ss}";
	};
	
	db.fetchAll( conf("sql#hitec.CardCancle#card"), root.removeAll() );
	
	while(cur,root){
		card_no = getCardNo(cur[trdata1]);
		
		card_total_amt = cur[total_amt];
		
		card_sale_amt    = cur[total_amt];
		card_sale_amt /=1.1;
		card_sale_amt    = card_sale_amt.round();
		
		vat = card_total_amt - card_sale_amt;
	   
	   appr_date = getApDate(cur[sale_date]);
	   appr_time = getApTime(cur[sale_date]);
	   
	   appr_no = cur[trdata9];
	   pur_card_no = cur[trdata10];
	   pur_card_nm =cur[trdata11];
	   res_no = cur[trdata13];
	   
	   record[pur_card_no] = pur_card_no;
	
		main._log("1111111111111111111111111111 =====> $record");
		
		
		db.fetch(conf("sql#kiosk.hitec#VanCdSelect"), record);
		van_cd = record[van_cd];
		
		db.fetch(conf("sql#kiosk.hitec#CardCdSelect"), record);
		std_card_cdrint_no = record[std_card_cdrint_no];
		van_card_cd = record[van_card_cd]; 
	
		main._log("2222222222222222222222222222 =====> $record");
	   
	   s.add(",$card_total_amt,$card_sale_amt,$vat,$card_total_amt,$card_total_amt,0");
	   
	   s.add(",$pur_card_nm,$appr_date,$card_total_amt,일시불,$appr_no,$res_no,카드승인,${head_msg}");
	}
	
	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	
	main._log("# 카드영수증 정보 == $s");
	
	main.qtMonSendData(s.kr() );
	
	 while(n,10){
		System.sleep(1000);
		if(qrmonNode[data]) break;
	}



}
KioskHiTec.order_realPrint(main,record) {

	
	db=Class.db('kiosk_hitec');
	s = "21,14,21,";
	
	info=_node();
	db.fetchAll(conf("sql#hitec.CornerInfo"), info.removeAll() );
	
	
	datetime	= System.date('MM/dd HH:mm:dd');
	
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"), record);
	record.inject(ms_nm, biz_no, tel_no, bill_addr, master_nm, head_msg);
	
	s.add("$ms_nm,$bill_addr,$biz_no,$tel_no ${master_nm},$datetime,");
	
	model=Class.model('CardCancle');
	root=model.rootNode();
	
	root[bill_no]	= record[bill_no];
	root[open_date]	= record[open_date];
	
	db.fetchAll( conf("sql#hitec.CardCancle#header"), root.removeAll() );
	
	while(cur,root){
		s.add("$cur[pos_no] - $record[bill_no],$cur[deal_no],");
	}
	while( cur, record ) {
		total_amt = cur[price*qty];
		corner=info.findOne('clplu_cd',cur[corner_cd]);
		corner_nm = corner[clplu_nm];
		class_seq = corner[class_seq];
		menu_nm = cur[menu_nm];
		menu_nm = menu_nm.substr(0,8);
		
		pm_no = "$cur[pos_no] - $record[bill_no]";
		
		s.add("${menu_nm}^$cur[price]^$cur[qty]^${total_amt}^${total_amt}^${class_seq}_${corner_nm}^${pm_no}\t");
	}
	getApDate=func(s) {
		yy=s.value(0,2), mm=s.value(2,4), dd=s.value(4,6);
		return "20${yy}${mm}${dd}";
	};
	db.fetchAll( conf("sql#hitec.CardCancle#card"), root.removeAll() );
	while(cur,root){
		card_no = getCardNo(cur[trdata1]);
		
		card_total_amt = cur[total_amt];
		
		card_sale_amt    = cur[total_amt];
		card_sale_amt /=1.1;
		card_sale_amt    = card_sale_amt.round();
		
		vat = card_total_amt - card_sale_amt;
	   
	   appr_date = getApDate(cur[sale_date]);
	   appr_no = cur[trdata9];
	   pur_card_no = cur[trdata10];
	   pur_card_nm =cur[trdata11];
	   res_no = cur[trdata13];
	   van_cd = '02';
	   
	   s.add(",$card_total_amt,$card_sale_amt,$vat,$card_total_amt,$card_total_amt,0");
	   
	   s.add(",$pur_card_nm,$appr_date,$card_total_amt,일시불,$appr_no,$res_no,카드승인,${head_msg}");
	}
	
	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	
	main._log("# 카드영수증 정보 == $s");
	
	main.qtMonSendData(s.kr() );
	
	 while(n,10){
		System.sleep(1000);
		if(qrmonNode[data]) break;
	}



}
KioskHiTec.findSubTag(&s, root) {
	tag=s.findPos('#').trim();
 	while( cur, root ) {
		not( cur[tag].eq(tag)  ) continue;
		id=s.trim();
		if( cur[id].eq(id) ) {
			return cur;
		}
		while( sub, cur ) {
			if( sub[id].eq(id) ) {
				return sub;
			}
		}
	}
	return null;
}
my.loadNewFuncs(path, reload) {

	db=Class.db('pages');
	tm=System.localtime();

	사용자_함수저장  = func(&s ) {
		root=_node();

		funcName=s.move().trim();
		not( s.ch().eq('(') ) return;
		funcParam=s.match().trim();
		not( s.ch().eq('{') ) return;

		body=s.match(1);
		root.varMap('cmsCode:funcGrp, funcName, funcParam, note, tm');
		if( body.find('/*') || body.find('// ') ) {
			root[funcSrc]=body;
			root[funcData]=makeSrc(body);
		} else {
			root[funcSrc]='';
			root[funcData]=body;
		}
		insert="insert into cmsFunc(cmsCode, funcName, funcParam, funcSrc, funcData, note, type, status, tm ) values( #{cmsCode}, #{funcName}, #{funcParam}, #{funcSrc}, #{funcData}, #{note}, 'A', 0, #{tm} )";
		if( reload ) {
			update="update cmsFunc set funcParam=#{funcParam}, funcData=#{funcData}, funcSrc=#{funcSrc}, note=#{note}, tm=#{tm} where cmsCode=#{cmsCode} and funcName=#{funcName}";
			not( db.exec(update, root) ) {
				db.exec(insert, root);
			}
		} else {
			not( db.count("select count(1) from cmsFunc where cmsCode=#{cmsCode} and funcName=#{funcName}", root) ) { 
				db.exec(insert, root);
			}
		}
		Cf.func("${root[funcName]}($root[funcParam]) {$root[funcData]}" );
	};
	클래스_함수저장 = func(&s) {
		root=_node();
		class_func=s.move().trim();
		not( s.ch().eq('(') ) return;
		class_param=s.match().trim();
		not( s.ch().eq('{') ) return;

		body=s.match(1);
		root.varMap('class_grp:classGrp, class_nm:classNm, class_func, class_param, note, tm');

		if( body.find('//') || body.find('/*') ) {
			root[class_src]=body;
			root[class_data]=makeSrc(body);
		} else {
			root[class_src]='';
			root[class_data]=body;
		}
		root[type] = when( class_func.eq(classNm), 'A', 'F');
		execNum=db.exec("update class_info set class_src=#{class_src}, class_data=#{class_data}, class_param=#{class_param}, note=#{note}, type=#{type}, tm='$tm' where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", root);
		not( execNum ) {
			db.exec("insert into class_info ( class_grp, class_nm, class_func, class_param, class_src, class_data, note, type, tm ) values (#{class_grp}, #{class_nm}, #{class_func}, #{class_param}, #{class_src}, #{class_data}, #{note}, #{type}, #{tm})", root);
		}
	};
	페이지_함수저장 = func(&s) {
		root=_node();
		s.ch();
		sp=s.cur();
		funcName=s.move(), c=s.ch();
		if( c.eq('.') ) {
			s.incr();
			s.move(), ep=s.cur(), c=s.ch();
			funcName=s.value(sp, ep, true);
		}
		print( funcName, c );
		not( c.eq('(') ) return;
		funcParam=s.match().trim(), c=s.ch();
		not( c.eq('{') ) return;

		body=s.match(1);
		root.varMap('pageGroup:pageGrp, pageCode:pageNm, funcName, funcParam, note, tm');
		if( body.find('/*') || body.find('// ') ) {
			root[funcSrc]=body;
			root[funcData]=makeSrc(body);
		} else {
			root[funcSrc]='';
			root[funcData]=body;
		}
		execNum=db.exec("update pageFunc set funcSrc=#{funcSrc}, funcData=#{funcData}, funcParam=#{funcParam}, note=#{note}, tm=#{tm} where cmsCode=#{pageGroup} and pageCode=#{pageCode} and funcName=#{funcName}", root);
		not( execNum ) {
			if( funcName.eq('onInit') ) {
				root[sort]=1;
			} else if( funcName.start('on') ) {
				root[sort]=2;
			} else if( funcName.find('.') ) {
				root[sort]=3;
			} else {
				root[sort]=4;
			}
			db.exec("insert into pageFunc( cmsCode, pageCode, funcName, funcParam, funcSrc, funcData, note, tm, sort ) values (#{pageGroup}, #{pageCode}, #{funcName}, #{funcParam}, #{funcSrc}, #{funcData}, #{note}, #{tm}, #{sort} )", root);
		}
	};
	설정_저장=func(data) {
		dbConfig=Class.db('config');
		root=_node();
		root.varMap('grp:confGrp, cd:confCode, note, data');
		execNum=dbConfig.exec("update conf_info set data=#{data}, note=#{note} where grp=#{grp} and cd=#{cd}", root);
		not( execNum ) {
			dbConfig.exec( getQuery('conf_info', 'grp, cd, data, note'), root);
		}
	};
	신규_함수저장 = func(&s) {
		if( s.find('<##',1) ) {
			left=s.findPos('<##',1);
			s=left;
		}
		while( s.valid(), n, 0 ) {
			data=s.findPos('## ',1);
			line=s.findPos("\n").trim();
			if( n>0 ) {
				c=data.ch(), comment='';
				if( c.eq('/') ) {
					if( data.ch(1).eq('/') ) comment.add( data.findPos("\n") );
					else if( data.ch(1).eq('*') ) comment.add( data.match() );
				}
				note=comment.trim();
				if( funcGrp ) {
					사용자_함수저장(data );
				} else if( classGrp ) {
					클래스_함수저장(data );
				} else if( pageGrp ) {
					페이지_함수저장(data );
				} else if( confGrp ) {
					설정_저장(data );
				}
				funcGrp=null, pageGrp=null, classGrp=null, confGrp=null;
			}
			if( line.start('config') ) {
				right=line.findPos('(',0,1).right();
				param=right.match();
				param.split('.').inject(confGrp, confCode);
			} else if( line.find('.') ) {
				line.split('.').inject(classGrp, classNm);
			} else if( line.find('#') ) {
				line.split('#').inject(pageGrp, pageNm);
			} else {
				funcGrp=line;
			}
			data=left;
		}
	};
	file=Class.file();
	not( path ) path="data/update/new_func.src";
	if( file.isFile(path) ) {
		신규_함수저장( file.readAll(path) );
	}

}
KioskHiTec.order_kitchenPrintNew(order, corner, main, cornerInfo, printInfo, time) {
	
	print("#####################################################");
	print(printInfo);
	
	print("주문 정보 order=$order");	
 	db.fetchAll( "SELECT X.print_no, CONCAT(STRING_AGG(CONCAT(B.goods_nm,'^',A.qty),'\t'),'\t') AS menu_info, 	  
  		 		MAX(C.kitchen_ip) AS kitchen_ip, 
  		 		MAX(D.clplu_nm) AS clplu_nm
  		FROM tb_sale_detail A
 		INNER JOIN hitec_m10s B ON A.corner_cd = B.clplu_cd AND A.menu_cd = B.goods_cd 
  		INNER JOIN hitec_m21s X ON A.menu_cd = X.goods_cd AND X.print_use_yn = '1'
  		LEFT JOIN kiosk_print_setup C ON X.print_no = C.print_no
 		INNER JOIN hitec_m03s D ON A.corner_cd = D.clplu_cd  
 		WHERE A.sale_seq = #{sale_seq}   
			AND A.ms_no = #{ms_no}
			AND A.open_date = #{open_date} 
			AND A.pos_no = #{pos_no}	 	 	 	
			AND A.bill_no = #{bill_no}
 		GROUP BY X.print_no  	
 		ORDER BY X.print_no", order.removeAll() );
	
	
	sale_date	= order[open_date];
	ms_no		= order[ms_no];
	pos_no		= order[pos_no];
	bill_no		= order[bill_no];
	deal_no		= order[deal_no].toNumber();
	detail_no	= 1;
	
	orderMessage='';
	sendCnt=0, errorCnt=0;
	
	while(cur, order, row, 0 ) {
		
		cur[sale_seq] 	= order[sale_seq]; 
		cur[ms_no] 		= order[ms_no]; 
		cur[open_date] 	= order[open_date]; 
		cur[pos_no] 		= order[pos_no]; 
		cur[bill_no] 		= order[bill_no]; 
		
		
		ipNode=printInfo.findOne('print_no', cur[print_no] );
		
		not( ipNode ) {
			errorCnt++;
			continue;
		}
 		socket=ipNode[screen_socket];
 		not( socket ) {
 			if( ipNode[screen_ip] ) {
 				errorCnt++;
 			}
 			continue;
 		}
 		
 		not( socket.isConnect() ) {
 			if( orderMessage ) orderMessage.add(", ");
 			orderMessage.add(cur[clplu_nm]);
 			continue;
 		}
 		 		
 		db.fetchAll( "SELECT X.print_no, CONCAT(X.print_no,'^',A.corner_cd,A.menu_cd,'^',B.goods_nm,'^',A.qty,'^') AS menu_info
  			FROM tb_sale_detail A
 			INNER JOIN hitec_m10s B ON A.corner_cd = B.clplu_cd AND A.menu_cd = B.goods_cd
 			INNER JOIN hitec_m21s X ON A.menu_cd = X.goods_cd AND X.print_use_yn = '1'
 			WHERE A.sale_seq = #{sale_seq} 
    			AND A.ms_no = #{ms_no}
				AND A.open_date = #{open_date}
				AND A.pos_no = #{pos_no}	 	
				AND A.bill_no = #{bill_no} 
				AND X.print_no = #{print_no}", cur.removeAll() );
 		
 		ipNode[data]='';
		data='';
		num=1, print_num=row+1;
		
		while( sub, cur, n ,0 ) {
			
			d1=lpad(detail_no,2), d2=lpad(num,2);
			data.add( "${ms_no}^${sale_date}^${pos_no}^${bill_no}^${d1}^${d2}^Y^${deal_no}^${sub[menu_info]}#" );
			num++, detail_no++;
		}	
		
		socket.send( data.kr() );
		System.sleep(250);
		not( ipNode[data] ) {
			System.sleep(250);
		}
		sendCnt++;
		
		print("주문 스크린 $ipNode[print_no] 전송=>$data 응답: $ipNode[data]");		
	}

	qrmonNode = _node('QtMonNode');

	
	
	not( time ) time=System.date('MM/dd HH:mm:ss');
	
	
	
	while(cur, order ) { 
		send = "22,5,$order[pos_no],";
		send.add("$order[deal_no],$time,");
		send.add(cur[menu_info]);		
		
		not( cur[kitchen_ip] ) {
			if( orderMessage ) orderMessage.add(', ');
 			orderMessage.add(cur[clplu_nm]);
 			print("주방프린터 IP NOT FOUND.");
 			continue;
 		}
 		send.add(",$cur[kitchen_ip]"); 

		print("주방프린터 출력 ip=$ip $send");
		qrmonNode[data] = null;
	
		main.qtMonSendData(send.kr());
	 	while(n, 6 ) {
  			System.sleep(500);
			if( qrmonNode[data]) break;
 		}
 		not( qrmonNode[data] ) {
 			print("# 주방프린터 출력중 오류가 발생했습니다 : $ipNode" );
 		}
	}
	
		
	if( orderMessage ) {
		order[orderMessage]=orderMessage;
	} else {
		if( sendCnt ) {
			while(cur, order) {
				ipNode=printInfo.findOne('print_no', cur[print_no] );
				not( ipNode[screen_socket] ) {
					continue;
				}
				not( ipNode[data] ) {
					if( orderMessage ) orderMessage.add(", ");
					orderMessage.add(cur[clplu_nm]);
				}
			}
		}
		if( orderMessage ) {
			orderMessage.add("\n주문스크린 응답 오류가 발생했습니다");
		} else if( errorCnt ) {
			orderMessage="주문스크리 출력중 오류가 발생했습니다.";
		}
		if( orderMessage ) {
			orderMessage.add("\n\n상품교환권을 가지고 관리자에게 문의하세요");
			order[orderMessage]=orderMessage;
		}
	}	
	print("############################ order_kitchenPrint END ##################################");

}
KioskHiTec.cancle_kitchenPrintNew(main, record) {
	
	print("# 주문취소 주방 프린터 $record ");
	
	node=_node(main, 'CancleOrderNode');
	
	db=Class.db('kiosk_hitec');	
	info=_node(main, 'CornerInfo'); 
	not( info.childCount() ) {
		db.fetchAll("SELECT print_no, screen_ip, screen_port, kitchen_ip, kitchen_port, note, kitchen_use_yn, screen_use_yn, reg_dt
			 FROM kiosk_print_setup", info.removeAll() );
	}
 	while( ipNode, info ) {
 		ipNode.removeAll();
 	}
 
	time=System.date('MM/dd HH:mm:ss');	

	
	qtmonNode = _node('QtMonNode');	
	node.initNode(record);
	
	print("# 주문취소 상품정보: $node ");
	db.fetchAll( "SELECT a.open_date, a.corner_cd, a.menu_cd, a.price, a.qty, b.goods_nm, c.class_seq, d.print_no
		     FROM tb_sale_detail a, hitec_m10s b, hitec_m03s c, hitec_m21s d
		   WHERE a.menu_cd = b.goods_cd and a.corner_cd = c.clplu_cd and a.menu_cd = d.goods_cd 
				and a.pos_no = #{pos_no} and a.open_date = #{open_date} and a.bill_no = #{bill_no}
				and d.print_use_yn = '1'
			  order by c.class_seq, a.detail_index", node );
	not( node.childCount() ) {
		main[page].alert("주문 상품정보가 없습니다");
		print("주방 취소 주문 상품정보가 없습니다");
		return;
	}
	 	 
	prev='', str='';
	while( cur, node ) {
		ipNode=info.findOne('print_no', cur[print_no]);
		print("node=ipNode=$ipNode cur=$cur============");
		ipNode.addNode().varMap( cur, 'open_date, corner_cd, menu_cd, goods_nm, price, qty');
	}
		
		
	print("맵핑 루트정보 ====$info");
	while( ipNode, info ) {
		
		not( ipNode.childCount() ) continue;
		ip=ipNode[kitchen_ip];
		not( ip ) continue;
		str="23,5,$record[pos_no],";
		str.add("$record[deal_no],$time,");
		while( sub, ipNode ) {
			str.add("$sub[goods_nm]^$sub[qty]\t");
		}
		str.add(",$ip");		
		main._log("# 취소 주방프린터 출력 ip=$ip $str");
		qtmonNode[data] = null;

		main.qtMonSendData(str.kr());
		while(n,10){
			System.sleep(1000);
			if( qtmonNode[data]) break;
		}
		not( qtmonNode[data] ) {
			main._log("# 취소 주방프린터 출력중 오류가 발생했습니다");
		}		
	}
	
	
	main._log("# 취소 주방스크린 출력 시작");
		
	socket = Class.socket('hitec1');	
	ms_no		= record[ms_no];	
	new_open_date = record[new_open_date];
	pos_no		= record[pos_no];
	bill_no		= record[cancle_bill_no];
	deal_no		= record[deal_no].toNumber(); 
	origin_no 	= record[bill_no]; 
	
 	detail_no=1;	
	while( ipNode, info ) {
		not( ipNode.childCount() ) continue; 
		ip			= ipNode[screen_ip];	
		print_no	= ipNode[print_no];	
		not( ip ){
			 continue;
		}		
		data='';
		num=1;
		while( sub, ipNode, n ,0 ) {
			open_date		= sub[open_date];
 			corner_cd	    = sub[corner_cd];
			menu_cd		= sub[menu_cd];
			menu_nm		= sub[goods_nm];
			qty				= -1*sub[qty];
			d1=lpad(detail_no,2), d2=lpad(num,2);
			data.add("${ms_no}^${new_open_date}^${pos_no}^${bill_no}^${d1}^${d2}^N^${deal_no}^${print_no}^${corner_cd}${menu_cd}^${menu_nm}^${qty}^${ms_no}${open_date}${pos_no}${origin_no}^#");
			num++, detail_no++;
		} 
		print("# 주문스크린 아이피 : $ip, $data");
		
		not( socket.connect( ip, 2018, 10) ) {
			main._log("## 주문스크린 연결오류 : $ip");
			continue;
		}
		if( socket.isConnect() ) { 
			
			socket.send( data.kr() );
			main._log("# 취소 주문 스크린 $ip 보내기=>$data");
			recv=socket.readBuffer();
			main._log("# 취소 주문 스크린 결과 =>$recv");
			socket.close();
		}		
	}

	print("############################ cancle_kitchenPrint END ##################################");

}
