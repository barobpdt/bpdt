dbUtil.dbUtil(db, conf, dbtype) {
	not( typeof(parseUtil,'class') ) loadClass('parse');
	not( conf ) conf = get('global.config');
	not( db ) db = conf;
	not( dbtype ) dbtype = 'postgres';
	dataNode = {};
	cf = parseUtil();
}
dbUtil.rootNode() {
 return dataNode;
}
dbUtil.setDb(type, path) {
	cur = instance("${type}.model");
	not( cur.open() ) {
		if( path )	cur.open(path);
	}
	@db = cur;
	return cur;
}
dbUtil.getDb() {
 return db;
}
dbUtil.getPagesDb() {
	return Cf.pageDb();
}
dbUtil.getConfigDb() {
	return conf;
}
dbUtil.tableParse(&sql, root) {
	_desc=callback(chk) {
		ch = in.ch();
		if( in.start('--') ) {
			in.incr(2);
			cur[desc] = in.findPos("\n").trim();
			if( chk ) in.findPos(",");
			return true;
		}
		return false;
	}
	sql.findPos('(',0,1);
	in = sql.match(), num=0;
	while( in.valid() ) {
		num++;
		cur = root.addNode();
		if( in.ch().eq('[') )
			cur[field] = in.match();
		else
			cur[field] = in.move();
		if( cur[field].eq('CONSTRAINT') ) {
			cur[id] = in.move();
			cur[type] = in.findPos('(',0,1).trim();
			cur[info] = in.match();
			in.findPos(',');
			continue();
		}
		if( in.ch().eq('[') ) {
			ty = in.match();
			cur[type] = ty.move().trim();
			if( ty.ch().eq('(') ) cur[size] = ty.match();
		} else {
			cur[type] = in.move();
		}
		if( _desc(true) ) continue;
		if( in.ch().eq('(') ) {
			cur[size] = in.match();
		}
		if( _desc(true) ) continue;
		if( in.ch().eq(',') ) {
			ch = in.incr().ch();
			_desc();
			continue;
		}
		info = in.move();
		switch( info.lower() ) {
		case 'default':
			ch = in.ch();
			if( ch.eq() || ch.eq('(') ) {
				cur[def] = in.match();
			} else {
				cur[def] = in.move();
				if( in.ch().eq('(') ) in.match();
			}
		case 'primary':
			in.move();
			cur[pk] = 'pk';
		case 'null':
			cur[info] = 'null';
		case 'not':
			in.move();
			cur[info] ='not null';
		default:
		}
		if( _desc(true) ) continue;
		if( in.ch().eq(',') ) {
			in.incr();
			_desc();
			continue;
		}
		if( cur[info] )
			cur[info].add(' ');
		else
			cur[info] = '';
		cur[info].add( in.findPos(',').trim() );
		_desc();
	}
}
dbUtil.makeTablesGroup(root) {
	_pk=callback(&s) {
		while( s.valid() ) {
			field = s.findPos(',').trim();
			sub = table.findOne('field',field);
			if( sub ) sub[pk] = 'Y';
		}
	}
	while( table, root ) {
		table.removeAll();
		this.tableParse(table[sql].ref(), table);
		cst = table.findOne('field','CONSTRAINT');
		if( cst ) {
			_pk(cst[info], table);
			table.remove(cst);
		}
	}
}
dbUtil.saveTablesInfo(root, grp, kind) {
	not( grp ) grp = 'common';
	tm = System.localtime();
	db.exec("delete from table_fields where tid in (select tid from table_info where table_grp='$grp')");
	db.exec("delete from table_info where table_grp='$grp'");
	while( table, root ) {
Throw[table_info delete error : check(conf)];
		db.exec("insert into table_info (table_grp, table_nm, table_desc, sql, kind, tm) values('$grp', #{name}, #{desc}, #{sql}, '$kind', '$tm')", table);
Throw[table_info insert error : check(conf)];
		tid = when( dbtype.eq('sqlite'),
			db.lastInsertId(),
			db.value("SELECT currval(pg_get_serial_sequence('table_info','tid'))");
		);
		while( sub, table ) {
			not( sub[field] ) break;
			db.exec("insert into table_fields (tid, field_nm, data_type, data_size, field_pk, field_def, field_info, field_desc, tm) values ('$tid', #{field}, #{type}, #{size}, #{pk}, #{def}, #{info}, #{desc}, '$tm')", sub);
		}
Throw[table_fields insert error : check(conf)];
	}
}
dbUtil.makeSqliteTables(db, grp) {
	root = dataNode.reuse();
	db.fetchAll("select name, sql from sqlite_master where name<>'sqlite_sequence' ", root);
	this.makeTablesGroup(root);
	this.saveTablesInfo(root, grp, 'S');
}
dbUtil.saveClassByFile(fullpath, lastModify) {
	pageDb = this.getPagesDb();
	not( fullpath.find('.') ) {
		className = fullpath;
		fullpath = "data/classes/${className}.class";
	}
	_grp=callback(&path) {
		left = path.findLast('/');
		return left.findLast('/').right().value();
	}
	src = instance('eps.file').readAll(fullpath);
	this.saveClass(pageDb, src.ref(), _grp(fullpath), lastModify );
}
dbUtil.saveClass(pageDb, &src, group, lastModify) {
	tm = System.localtime();
	modifyCheck = when( lastModify,
		"select count(1) from class_mst where  class_grp=#{group} and class_nm=#{classNm} and tm>#{lastModify}";
	);
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
		if( modifyCheck ) {
			if( pageDb.value(modifyCheck, dataNode).eq('0') ) return false;
		}
		not( pageDb.exec("update class_mst set class_desc=#{comment}, type=#{type}, tm='$tm' where class_grp=#{group} and class_nm=#{classNm}", dataNode) ) {
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
				cls.add("\r\n\tclass[$w] = callback($param) {\r\n$body\r\n\t}");
			} else {
				dataNode[type] = 'F';
				cls.add("\r\n\tnot(class[$w]) class[$w] = callback($param) {\r\n$body\r\n\t}");
			}
			if( body.find('#',1) ) {
				fsrc = makeCallback( body.ref() );
			} else {
				fsrc = body;
			}
			if( fsrc.find('/*',1) ) {
				fsrc = stripComment(fsrc.ref());
			}
			if( fsrc.find('//',1) ) {
				fsrc = stripLineComment(fsrc.ref());
			}
			dataNode.put(w, param, body, fsrc, comment);
			not( pageDb.exec("update class_info set class_param=#{param}, class_data=#{body}, class_src=#{fsrc}, note=#{comment}, type=#{type}, tm='$tm' where class_grp='$group' and class_nm=#{classNm} and class_func=#{w}", dataNode) ) {
				pageDb.exec("insert into class_info(class_grp, class_nm, class_func, class_param, class_data, class_src, note, type, tm) values (#{group}, #{classNm}, #{w}, #{param}, #{body}, #{fsrc}, #{comment}, #{type}, '$tm')", dataNode);
			}
		}
		not( blib ) {
			cp = '';
			if( classParam ) cp = ", $classParam";
			cls.add("\r\n\tCf.setClass(class, '$classNm' $cp);");
		}
		cls.add("\r\n\treturn class;");
	}
	return true;
}
dbUtil.makeCreateSample(infoFile, subCode, create) {
	not( infoFile ) infoFile='table';
	not( subCode ) subCode = 'tableGroup';
	node = cf.info(infoFile);
	this.makeTableInfo(node[$subCode].ref(), create);
}
dbUtil.makeTableInfo(&data, create) {
	print("############# makeTableInfo ($create)##################");
	root = cf.makeTreeByTab(data.ref()  );
	_g=callback(&s) {
		group[name] = s.findPos(':').trim();
		if( s.valid() ) group[desc] = s.trim();
	}
	_t=callback(&s) {
		table[type] = 'table';
		table[name] = s.findPos(':').trim();
		if( s.valid() ) table[desc] = s.trim();
	}
	while( group, root ) {
		_g( group[text] );
		while( table, group ) {
			_t( table[text] );
			data='';
			while( field, table) data.add(field[text]);
			if( data ) {
				table[sql] = this.makeCreate(table[name], data.ref(), table[desc]);
			}
		}
		this.makeTablesGroup(group);
		not( create )  continue;
		tm = System.localtime();
		while( table, group ) {
			sql = when( dbtype.eq('sqlite'),
				"SELECT count(1) FROM sqlite_master WHERE name=#{name}",
				"SELECT count(1) FROM information_schema.tables WHERE table_schema='public' and table_name=#{name}"
			);
			tableCnt = conf.value(sql, table);
			if( tableCnt.eq(0) ) {
				conf.exec(table[sql]);
			}
			grp = group[name];
			cnt=db.exec("update table_info set table_desc=#{desc}, sql=#{sql}, kind='$kind', tm='$tm' where table_grp='$grp' and table_nm=#{name}", table);
			if( cnt.eq(0) ) {
				db.exec("insert into table_info (table_grp, table_nm, table_desc, sql, kind, tm) values('$grp', #{name}, #{desc}, #{sql}, '$kind', '$tm')", table);
				tid = when( dbtype.eq('sqlite'),
					db.lastInsertId(),
					db.value("SELECT currval(pg_get_serial_sequence('table_info','tid'))");
				);
			} else {
				tid = db.value("select tid from table_info where table_grp='$grp' and table_nm=#{name}",table);
			}
			print("$dbtype=======> $tableCnt, $cnt, $tid");
			while( sub, table ) {
				not( sub[field] ) break;
				fieldCnt = db.exec("update table_fields set data_type=#{type}, data_size=#{size}, field_pk=#{pk}, field_def=#{def}, field_info=#{info}, field_desc=#{desc}, tm='$tm' where tid='$tid' and field_nm=#{field}",sub);
				if( fieldCnt ) {
					if( tableCnt ) {
						print("field modify");
					}
					continue;
				}
				if( tableCnt ) {
					alter = "alter table $table[name] add column $sub[field] $sub[type]");
					if( sub[size] ) alter.add("($sub[size])");
					if( sub[def] ) alter.add(" default '$sub[default]'");
					conf.exec(alter);
				}
				db.exec("insert into table_fields (tid, field_nm, data_type, data_size, field_pk, field_def, field_info, field_desc, tm) values ('$tid', #{field}, #{type}, #{size}, #{pk}, #{def}, #{info}, #{desc}, '$tm')", sub);
			}
		}
	}
	return root;
}
dbUtil.makeCreate(table, &data, desc) {
	s = "\n/* $desc */\nCREATE TABLE $table (";
	pk = [];
	pkCheck = true;
	while( data.valid() ) {
		sep =",";
		x=data.find('#').size();
		if( x ) {
			y=data.find(",").size();
			if( y.eq(0) ) {
				sep='#';
			} else if( x<y ) {
				sep='#';
			}
		}
		left = data.findPos(sep);
		field = left.move().trim();
		if( field.find('_date') ) {
			type = 'date';
		} else if( field.finds('_cd','_pid') ) {
			type = 'varchar(32)';
		} else if( field.finds('_yn', '_ty') ) {
			type = 'char(1)';
		} else if( field.finds('_nm', '_val', '_grp') ) {
			type = 'varchar(64)';
		} else if( field.finds('_seq', '_idx','_num', '_rank', '_total', '_cnt', '_ord', '_sort', '_point') ) {
			if( pkCheck && field.finds('_seq', '_idx','_num') ) {
				if( pk.size().eq(0) ) {
					if( dbtype.eq('sqlite') )
						type = 'integer autoincrement';
					else
						type = 'serial';
				}
				pk.add(field);
			} else {
				type = 'integer';
			}
		} else if( field.finds('_state', '_type', '_kind', '_mode') ) {
			type = 'varchar(8)';
		} else if( field.find('_dtm') ) {
			type = 'bigint';
		} else if( field.find('_dt') ) {
			type = 'timestamp';
		} else if( field.find('status') ) {
			type = 'varchar(8)';
		} else if( field.start('ref') ) {
			type = 'varchar(64)';
		} else if( field.eq('url','icon') ) {
			type = 'varchar(256)';
		} else if( field.eq('depth','sort','level','tm','pseq','pidx','point','term') ) {
			type = 'integer';
		} else if( field.eq( 'mode', 'type', 'code', 'pcode') ) {
			type = 'varchar(32)';
		} else {
			id1=field.value(1);
			if( id1.eq('id') ) {
				pk.add(field);
				type = 'varchar(16)';
			} else if( field.eq('idx','seq') || id1.eq('idx') ) {
				if( pkCheck ) {
					if( dbtype.eq('sqlite') )
						type = 'integer autoincrement';
					else
						type = 'serial';
					pk.add(field);
				} else {
					type = 'integer';
				}
				pkCheck = false;
			} else {
				val = field;
				val.findPos('_');
				if( val.eq('id') && pkCheck ) {
					pk.add(field);
					type = 'varchar(16)';
				} else {
					type = 'text';
				}
			}
		}
		s.add("\n\t$field $type");
		ch = left.ch();
		if( ch.eq('(') ) {
			in = left.match();
			s.add(" default '$in'");
			ch = left.ch();
		}
		s.add(",");
		if( ch ) {
			s.add("\t\t-- $left");
		}
		if( sep.eq("#") ) {
			pkCheck = false;
		}
	}
	s.add("\n\tuse_yn char(1) default 'Y',\t\t-- 사용여부");
	s.add("\n\tmod_id varchar(16),\t\t-- 수정자명");
	s.add("\n\tmod_dt timestamp,\t\t-- 수정일시");
	s.add("\n\treg_id varchar(16),\t\t-- 등록자명");
	s.add("\n\treg_dt timestamp default now()\t\t-- 등록일시");
	if( pk.size() ) {
		s.add("\n\t, CONSTRAINT PK_${table} PRIMARY KEY  (");
		while( id, pk, n, 0) {
			if( n ) s.add(", ");
			s.add(id);
		}
		s.add(")");
	}
	s.add("\n);\n\n");
	return s;
}
dbUtil.insertQuery(table, &fields) {
	rst = "insert into $table ( $fields ) values (";
	num=0;
	while( fields.valid() ) {
		left = fields.findPos(',').trim();
		if( num ) rst.add(', ');
		rst.add("#{$left}");
		num++;
	}
	rst.add(")");
	return rst;
}
dbUtil.updateQuery(table, &fields, &where) {
	rst="update $table set ";
	num=0, arr=[];
	while( where.valid() ) {
		arr.add( where.findPos(',').trim() );
	}
	while( fields.valid() ) {
		left = fields.findPos(',').trim();
		if( findArrayCode(arr,left) ) continue;
		if( num ) rst.add(', ');
		rst.add("$left=#{$left}");
		num++;
	}
	rst.add(" where ");
	while( field, arr, num, 0 ) {
		if( num ) rst.add(' and ');
		rst.add("$field=#{$field}");
	}
	arr.delete();
	return rst;
}
dbUtil.makeFields(&sql, check, fields) {
	not( fields ) fields={};
	if( check ) {
		cur = fields.addNode({code:check, text: *, width:40} );
	}
	flag = 8|2;
	sql.findPos("select", flag);
	while( sql.valid() ) {
		line = sql.findPos("\n");
		left = line.findPos('--');
		not( line.valid() )
			continue;
		width=null;
		ch =  left.ch();
		if( ch.eq(',') ) {
			left.incr();
			ch =  left.ch();
		}
		if( ch.eq("0") ) {
			left.incr();
			field = left.move();
			if( field.eq("as", "AS") ) {
				left.ch();
				field = left.move();
			} else {
				continue;
			}
		} else if( ch.eq('(') ) {
			left.match();
		} else {
			field = left.move();
			if( field.eq('from','FROM') ) {
				break;
			}
			if( left.ch().eq('.') ) {
				field = left.incr().move();
			}
		}
		as = left.find("as", flag).right();
		if( as ) {
			field = as.move();
		}
		comment = line;
		text = comment.findPos("#").trim();
		if( comment.valid() ) {
			width = comment.trim();
		}
		not( field ) continue;
		cur = fields.addNode();
		cur[code] = field.trim();
		cur[text] = text;
		if( width )
			cur[width] = width;
	}
	return fields;
}
dbUtil.makeConfGroup(path) {
	not(path ) path='data/info';
	ff=instance('eps.filefind'), file=instance('eps.file');
	node={kind:IF};
	_parse=callback(&d) {
		while( d.valid() ) {
			left = d.findPos('##',1);
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
				node[conf_desc]='';
				if( ch.eq('>') ) {
					line.incr();
					node[conf_desc] = line.trim();
				}
				node[conf_cd] = code;
				node[src] = left;
				not( db.exec("update conf_group_detail set src=#{src} where conf_grp_cd=#{conf_grp_cd} and conf_cd=#{conf_cd}", node)  ) {
					db.exec("insert into conf_group_detail (conf_grp_cd, conf_cd, conf_desc, src, kind, modify_dtm, dtm) values (#{conf_grp_cd}, #{conf_cd}, #{conf_desc}, #{src}, #{kind}, #{modify_dtm}, #{dtm})", node);
				}
			}
			line = d.findPos("\n").trim();
		}
	}
	_removeComment=callback(&d) {
		not( d.find("[##") ) return d;
		s='';
		while( d.valid() ) {
			left = d.findPos("[##",1,true);
			d.match("[##", "##]");
			s.add(left);
		}
		return s;
	}
	node[dtm]=System.localtime();
	while( cur, ff.fetchAll(path,'*.inf') ) {
		node[conf_grp_cd] = cur[fileName].findLast('.');
		node[modify_dtm] = cur[modifyDate];
		node[ref] = "$path/$cur[fileName]";
		ok=false;
		if( db.value("select count(1) from conf_group where conf_grp_cd=#{conf_grp_cd}", node).eq("0") ){
			db.exec("insert into conf_group (conf_grp_cd, ref, kind, modify_dtm, dtm) values (#{conf_grp_cd}, #{ref}, #{kind}, #{modify_dtm}, #{dtm})", node);
			ok=true;
		}
		not( ok ) {
			ok = db.exec("update conf_group set modify_dtm=#{modify_dtm} where conf_grp_cd=#{conf_grp_cd} and modify_dtm<#{modify_dtm}", node);
		}
		not( ok ) continue;
		str = file.readAll(node[ref]);
		str = _removeComment( str.ref() );
		_parse( str.ref() );
	}
}
dbUtil.getTableFields(table, field, root) {
	not( field ) field = 'field_nm';
	not( root ) root = {};
	db.fetchAll("select $field from table_fields where 1=1
	   and tid in (select tid from table_info where table_nm='$table')
	   and field_info!='autoincrement'", root);
	return root;
}

parseClass.parseClass(db) {
	dataNode = {};
}
parseClass.parseProp(node, tag, &prop) {
	node[tag]=tag;
	while( prop.valid() ) {
		if( prop.ch().eq(',') ) {
			prop.incr();
			continue;
		}
		k=prop.move();
		if( prop.ch().eq('=') ) {
			prop.incr();
			ch=prop.ch();
			if( ch.eq() ) {
				node[$k]=prop.match();
			} else if( ch.eq('[') ) {
				in=prop.match();
				arr=in.split('parse');
				print("parseProp=$arr");
				node[$k]=arr;
			} else {
				node[$k]=prop.findPos(" \t\n",4).trim();
			}
		} else {
			break;
		}
	}
}
parseClass.parseTag(&data, node) {
	not( node ) {
		node=dataNode;
	}
	while( data.valid() ) {
		ch=data.ch();
		not( ch.eq('<') ) {
			break;
		}
		if( data.ch(1).eq('?') ) {
			data.match('<?','?>');
		}
		cur=data.cur();
		tag=data.incr().move();
		data.pos(cur);
		sub = node.addNode();
		in=data.match("<$tag","</$tag>");
		not( in ) {
			print("reportConf@parseTag error : '$tag' not match");
			break;
		}
		prop=in.findPos(">");
		this.parseProp( sub, tag, prop);
		cp=in.cur();
		if( in.ch().eq('<') ) {
			this.parseTag(in, sub);
		} else {
			sub[data]=in.value();
		}
	}
	return node;
}
parseClass.loadXmlData(&data, node) {
	not( node ) {
		node=dataNode;
	}
	while( data.valid() ) {
		ch=data.ch();
		not( ch.eq('<') ) {
			break;
		}
		if( data.ch(1).eq('?') ) {
			data.match('<?','?>');
		}
		cur=data.cur();
		tag=data.incr().move();
		data.pos(cur);
		in=data.match("<$tag","</$tag>");
		not( in ) {
			print("reportConf@parseTag error : '$tag' not match");
			break;
		}
		prop=in.findPos(">");
		cp=in.cur();
		if( in.ch().eq('<') ) {
			sub = node.addNode();
			this.parseProp( sub, tag, prop);
			this.loadXmlData(in, sub);
		} else {
			node[$tag]=in.value();
		}
	}
	return node;
}
parseClass.find(root, &code, check) {
	node=null;
	while( code.valid() ) {
		field=code.findPos('>').trim();
		node=root.findOne('tag', field);
		not( node ) {
			return null;
		}
		root=node;
	}
	return node;
}
parseClass.parseXml(path, node) {
	print("parseXML path=>$path");
	buf=class('file').readAll(path);
	this.parseTag(buf.ref(), node);
	return node;
}

codeClass.codeClass(db) {
	not(db ) db = get('global.config');
	dataNode = {};
	useYn = null;
}
codeClass.parseCodeValue(&s, node) {
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
				sub[value]=s.findPos(',').trim();
			}
		} else {
			sub[code]=k;
		}
	}
}
codeClass.getCodeNode(code) {
	not( code )
		return null;
	node = dataNode[$code];
	not( node ) {
		node = {code: $code};
		dataNode[$code] = node;
		this.parseCodeValue(conf("cc.$code"), node);
	}
	return node;
}
codeClass.getCodeText(code, val) {
	node=this.getCodeNode(code);
	sub=node.findOne('code', val);
	if( sub ) return sub[value];
	return null;
}
codeClass.getCodeFind(code,val) {
	node=this.setCodeNode(code);
	return node.findOne('code',val);
}
codeClass.ccNode(code) {
	not( code )
		return null;
	node = dataNode[$code];
	not( node ) {
		not( db ) return null;
		node = {code: $code};
		db.fetchAll( "SELECT code, value, ref1 from comm_tree where ref='CC' and pcode=#{code} and use_yn='Y' order by sort", node);
		dataNode[$code] = node;
	}
	return node;
}
codeClass.ccRefNode(code, filter) {
	not( code )
		return null;
	node = dataNode[$code];
	not( node ) {
		not( db ) return null;
		node = {ref1: $code};
		db.fetchAll( "SELECT code, value, ref1 from comm_tree where ref='CC' and use_yn='Y' $filter and pcode in (select code from comm_tree where ref='CC' and ref1=#{ref1}) order by sort", node);
		dataNode[$code] = node;
	}
	return node;
}
codeClass.ccVal(code, ref) {
	node = this.ccRefNode(code);
	not( node ) return null;
	sub = node.findOne('ref1', ref);
	return sub[value];
}
codeClass.useYnVal(ref) {
	node = this.useYnNode();
	not( node ) return null;
	sub = node.findOne('ref1', ref);
	return sub[value];
}
codeClass.ccValue(pcode, ref1) {
	node = this.ccNode(pcode);
	if( node ) {
		sub = node.findOne('ref1', ref1);
		return sub[value];
	}
	return null;
}
codeClass.ccRef(pcode, ref) {
	node = this.ccNode(pcode);
	not( node ) return null;
	sub = node.findOne('ref1', ref);
	return sub[value];
}
codeClass.ccCombo(page, pcode, val, index) {
	combo=page.widget(ginfo('inlineCombo'));
	combo.addItem(this.ccRefNode(pcode), 'ref1,value', '=선택=');
	combo.init(page, index);
	if( val ) combo.value(val);
	return combo;
}
codeClass.useYnNode() {
	not( useYn ) {
		@useYn = {};
		useYn.addNode({ref1:Y, value:사용});
		useYn.addNode({ref1:N, value:미사용});
	}
	return useYn;
}
codeClass.useYnCombo(page,val,index) {
	combo=page.widget(ginfo('inlineCombo'));
	combo.addItem(this.useYnNode(), 'ref1,value' );
	combo.init(page, index);
	if( val ) combo.value(val);
	return combo;
}

dbClass.dbClass(db) {
	dataNode = {};
}
dbClass.insertQuery(table, &fields) {
	rst = "insert into $table ( $fields ) values (";
	num=0;
	while( fields.valid() ) {
		left = fields.findPos(',').trim();
		if( num ) rst.add(', ');
		rst.add("#{$left}");
		num++;
	}
	rst.add(")");
	return rst;
}
dbClass.updateQuery(table, &fields, &where) {
	rst="update $table set ";
	num=0, arr=[];
	while( where.valid() ) {
		arr.add( where.findPos(',').trim() );
	}
	while( fields.valid() ) {
		left = fields.findPos(',').trim();
		if( findArrayCode(arr,left) ) continue;
		if( num ) rst.add(', ');
		rst.add("$left=#{$left}");
		num++;
	}
	rst.add(" where ");
	while( field, arr, num, 0 ) {
		if( num ) rst.add(' and ');
		rst.add("$field=#{$field}");
	}
	arr.delete();
	return rst;
}

fileClass.fileClass() {

}
fileClass.isFile(path) {
	return instance('my.file').isFile(path);
}
fileClass.isFolder(path) {
	return instance('my.file').isFolder(path);
}
fileClass.fileHash(path) {
	return instance('my.file').fileHash(path);
}
fileClass.inode(path) {
	return instance('my.file').inode(path);
}
fileClass.fileSize(path) {
	return instance('my.file').size(path);
}
fileClass.modifyDate(path) {
	return instance('my.file').modifyDate(path);
}
fileClass.fileDelete(path) {
	instance('my.file').delete(path);
}
fileClass.fileMove(path, rename) {
	instance('my.file').move(path, rename);
}
fileClass.readAll(path) {
	if( instance('my.file').isFile(path) ) {
		return instance('my.file').readAll(path);
	} else {
		this.setError();
	}
}
fileClass.save(path, buf) {
	instance('my.file').writeAll(path, buf.ref() );
}
fileClass.files(path, filter, flag) {
	arr=class('util').arr();
	while( cur, instance('my.filefind').fetchAll(path,filter) ) {
		if( cur.type.eq('file') ) {
			fnm=cur[fileName].utf8();
			if( flag.eq(1) ) {
				arr.add("$path/$fnm");
			} else {
				arr.add(fnm);
			}
		}
	}
	return arr;
}
fileClass.folders(path, flag) {
	not( flag ) flag=0;
	arr=class('util').array('folders', true);
	while( cur, instance('my.filefind').fetchAll(path) ) {
		if( cur.type.eq('folder') ) {
			if( flag.eq(0) ) {
				arr.add(cur[fullPath].utf8() );
			} else {
				arr.add(cur[folderName].utf8());
			}
		}
	}
	return arr;
}
fileClass.zipArray(path, arr) {
	instance('my.zip').zip(path, true);
	while(cur, arr ) instance('my.zip').addFile(cur);
	instance('my.zip').close();
}
fileClass.zipFolder(path, folder) {
	instance('my.zip').zip(path, true);
	instance('my.zip').addFolder(folder);
	instance('my.zip').close();
}
fileClass.zipFolderArray(path, arr) {
	instance('my.zip').zip(path, true);
	while(cur, arr ) instance('my.zip').addFolder(cur);
	instance('my.zip').close();
}
fileClass.unzip(zipfile, savePath, flag) {
	instance('my.zip').open(zipfile);
	instance('my.zip').unzip(savePath, flag);
}
fileClass.unZipfile(zipfile, file, savePath, flag) {
	instance('my.zip').open(zipfile);
	instance('my.zip').unzipFile(file, savePath, flag);
}
fileClass.copyFile(arrFile, dest, widget, rename) {
	instance('my.file').copy(arrFile, dest, widget, rename);
}
fileClass.copyFolder(folder, dest, widget, rename) {
	if( instance('my.file').isFolder(folder) ) {
		instance('my.file').copy(folder, dest, widget, rename);
	}
}

widgetUtil.widgetUtil(page) {
	not( typeof(dbUtil,'class') ) loadClass('db');
	this.addClass(dbUtil);
}
widgetUtil.formValues(node) {
	not( node ) node={};
	while( c, page.widgets() ) {
		if( c[tag].eq('button') ) continue;
		id = c[id];
		node[$id] = page[$id].value();
	}
	return node;
}
widgetUtil.setFormValues(node) {
	while( key, node.keys() ) {
		page[$key].value( node[$key] );
	}
}

util.util() {
	arrs=[];
	arrs.size(16);
	dataNode = {aidx:0};
}
util.test() {
	print("util test function called!!!");
}
util.arr(id) {
	if( id ) {
		arr=dataNode[arr#$id];
		not( node ) {
			arr=[];
			dataNode[arr#$id]=arr;
		}
		return arr.reuse();
	}
	n=dataNode[aidx++];
	if( n.eq(16) ) {
		dataNode[aidx]=0;
		n=dataNode[aidx++];
	}
	arr=arrs.get(n);
	return arr.reuse();
}
util.node(id) {
	if( id ) {
		node=dataNode[node#$id];
		not( node ) {
			node={};
			dataNode[$id]=node;
		}
		return node.initNode();
	}
	n=dataNode[nidx++];
	if( n.eq(16) ) {
		dataNode[nidx]=0;
		n=dataNode[nidx++];
	}
	node=dataNode.child(n);
	not( node ) node=dataNode.addNode();
	return node.initNode();
}
util.array(code, reuse) {
	arr=dataNode[$code];
	not( typeof(arr,'array') ) {
		arr=[];
		dataNode[$code]=arr;
	}
	if( reuse ) arr.reuse();
	return arr;
}
util.rateArray(rate, width, arr) {
	total=width, sum=0;
	while( cur, rate) {
		if( cur.find('px') ) {
			w=cur.find('px').trim();
			width-=w;
		} else {
			sum+=cur;
		}
	}
	not( arr ) arr=[];
	while( cur, rate) {
		if( cur.find('px') ) {
			w=cur.find('px').trim();
			arr.add(w);
		} else {
			arr.add( expr( (cur/sum.0)*width) );
		}
	}
	arr.recalc(total);
	return arr;
}
util.loadClass(name) {
	db=instance('pages.model');
	src=class('file').readAll("data/classes/${name}.class");
	arr=this.array('loadClass',true);
	if( name.find('_') ) {
		group=name.findPos("_").trim();
	} else {
		group='common';
	}
	not( group ) {
		return;
	}
	saveClassFile(db, src.ref(), group, arr);
	while( classNm, arr ) {
		loadClassByDb("${group}.${classNm}");
	}
}
util.split(&str, sep) {
	arr=this.arr();
	not(sep ) sep=",";
	while( str.valid() ) {
		arr.add( str.findPos(sep).trim() );
	}
	return arr;
}
util.incr(val, pad) {
	num=expr(val + 1);
	if( pad ) {
		val="$num";
		return val.lpad(pad);
	}
	return num;
}

page.page() {
	cf={};
	this.setDefaultPageKeyDown();
}
page.menuPopup(widget, &str, pos) {
	cf.inject(pageCd);
	menuStr='';
	while( str.valid() ) {
		left=str.findPos(',');
		val=left.findPos(':').trim();
		menuCd='';
		if( val.eq('-') || val.find('.') ) {
			menuCd=val;
		} else {
			menuCd="${pageCd}.${val}";
			not( page.action(menuCd) ) {
				menuCd="tree.${val}";
			}
		}
		if( menuStr ) menuStr.add(",");
		menuStr.add(menuCd);
		/* 메뉴코드 다음에 콜론(:) 인 경우 */
		if( left.valid() ) {
			menuTitle=left.trim();
			page.action(menuCd).text(menuTitle);
		}
	}
	print("menuStr===>$menuStr");
	widget.menu(menuStr, pos);
}
page.openPopup(popup, obj) {
	not( popup ) return;
	if( typeof(obj,'rect') ) {
		rcOpen=obj;
		args(2, flag);
		openPopup( popup, page, rcOpen, flag);
	} else if( typeof(obj,'widget') ) {
		switch(obj.tag) {
		case tree:
			args(2, node, width, height, flag);
			tree=obj;
			rc=tree.nodeRect(node);
			position=tree.mapGlobal( rc.lb());
		case grid:
			args(2, node, col, width, height, flag);
			grid=obj;
			hh=grid.headerHeight();
			hh+=2;
			rc=grid.nodeRect(node, col);
			rc.incrY(hh, true);
			position=grid.mapGlobal( rc.lb());
		case editor:
			args(2, width, height, flag);
			editor=obj;
			rc = editor.cursorRect();
			position = editor.mapGlobal(rc.lt());
		default:
			args(2, width, height, flag);
			geo=obj.geo();
			print();
			position=parent.mapGlobal(geo.lb());
		}
		not(width) width=650;
		not(height) height=560;
		rcOpen = Class.rect(position.move(5,0), width, height);
		openPopup( popup, page, rcOpen, flag);
	} else if( obj ) {
		popup.open(parent, obj);
	} else {
		popup.open();
	}
}
page.openCenter(popup) {
	popup.open( this.getMainPage(), 'center');
}
page.action(code) {
	cf.inject(pageCd);
	actionCode="${pageCd}.$code";
	size=args().size();
	switch(size ) {
	case 2: args(1,text);
		page.action(actionCode).text(text);
	case 3: args(1,text,icon);
		page.action(actionCode, text, icon);
	case 4: args(1,text,icon,trigger);
		page.action(actionCode, text, icon);
		page.action(actionCode).trigger(trigger);
	default: return;
	}
}
page.getMainPage() {
	return class('page').getMainPage(page);
}
page.getParentFunction(funcName) {
	return class('page').getParentFunction(page, funcName);
}
page.dbError() {
	err=db.error();
	if( err ) {
		page.alert("DB 에러 : $err");
		return true;
	}
	return false;
}
page.openInput(title, t1, t2, v1, v2) {
	popup=pageLoad('devPopup.commonInputTwo');
	popup.initPage(title, t1, t2, v1, v2);
	popup.open(this.getMainPage(),'center');
}
page.pageKeyDown(key, mode) {
	not( mode&KEY.ctrl ) {
		return;
	}
	switch( key) {
	case KEY.Q: 	this.popupQuery();
	}
}
page.setDefaultPageKeyDown() {
	page.eventMap(onKeyDown, this.pageKeyDown, 'key, mode');
}
page.popupQuery() {
	cf.inject(projectCd, pageCd);
	not( projectCd ) projectCd='dev';
	popup=Cf.loadPage('dev.main');
	code=cf.pageCode;
	val=popup.detail.value();
	str='';
	if( val ) str.add("\n\n");
	if( grid ) {
		str.add("/* [$pageCd] 그리드 리스트 조회 */\n");
		sql=conf("sql#${projectCd}.${pageCd}#grid");
		not( sql ) sql="\n\n";
		str.add(template() {
conf("sql#${projectCd}.${pageCd}#grid", "$sql", true);
		});
		if( val, or(str) ) str.add("\n\n");
		str.add("/* [$pageCd] 그리드 리스트 총수*/\n");
		sql=conf("sql#${projectCd}.${pageCd}#gridTotal");
		not( sql ) sql="\n\n";
		str.add(template() {
conf("sql#${projectCd}.${pageCd}#gridTotal", "$sql", true);
		});
	}
	arr=cf.keys('query#'), ok=true;
	if( arr.size() ) {
		if( val, or(str) ) str.add("\n\n");
		str.add("/* [$pageCd] 페이지 쿼리 */\n");
		while( key, arr ) {
			qid=key.find('#').right().trim();
			sql=conf("sql#${projectCd}.${pageCd}#${qid}");
			not( sql ) sql="\n\n";
			str.add("\n",template() {
conf("sql#${projectCd}.${pageCd}#${qid}", "$sql", true);
			});
		}
		ok=false;
	}
	if( tree, ok ) {
		if( val, or(str) ) str.add("\n\n");
		str.add("/* [$pageCd] 트리 자식데이터 조회 */\n");
		sql=conf("sql#${projectCd}.${pageCd}#treeChildData");
		not( sql ) sql="\n\n";
		str.add(template() {
conf("sql#${projectCd}.${pageCd}#treeChildData", "$sql", true);
		});
	}
	arr=cf.keys('cc#');
	if( arr.size() ) {
		str.add("\n\n/* [$pageCd] 공통코드*/\n");
		while( key, arr ) {
			cid=key.find('#').right().trim();
			cc=conf("ccl.${pageCd}#${cid}");
			not( cc ) sql="\n\n";
			str.add("\n",template() {
conf("cc.${pageCd}#${cid}", "$cc", true);
			});
		}
		ok=false;
	}
	popup.src.append(str, true);
	popup.open(page,'center');
}
page.arr(id) {
	a=_arr(cf,id);
	return a.reuse();
}
page.node(id) {
	o=_node(cf,id);
	return o.initNode();
}

editorImpl.editorImpl(page) {
	this.addClass(dev.page, dev.EditorSrc);
}

page.page() {
	dataNode = {};
}
page.setPageClass(classCode, page) {
	param=args();
	if( page.pageImpl ) {
		page.alert("페이지 구현 클래스가 이미 등록되었습니다,  $classCode 등록 실패");
	}
	page.pageImpl=newClass(classCode, param );
}
page.openPopup(popup, parent, obj) {
	not( popup ) return;
	if( typeof(obj,'rect') ) {
		rcOpen=obj;
		args(3, flag);
		openPopup( popup, parent, rcOpen, flag);
	} else if( typeof(obj,'widget') ) {
		switch(obj.tag) {
		case tree:
			args(3, node, width, height, flag);
			tree=obj;
			rc=tree.nodeRect(node);
			position=tree.mapGlobal( rc.lb());
		case grid:
			args(3, node, col, width, height, flag);
			grid=obj;
			hh=grid.headerHeight();
			hh+=2;
			rc=grid.nodeRect(node, col);
			rc.incrY(hh, true);
			position=grid.mapGlobal( rc.lb());
		case editor:
			args(3, width, height, flag);
			editor=obj;
			rc = editor.cursorRect();
			position = editor.mapGlobal(rc.lt());
		default:
			args(3, width, height, flag);
			geo=obj.geo();
			position=parent.mapGlobal(geo.lb());
		}
		not(width) width=650;
		not(height) height=560;
		rcOpen = Class.rect(position.move(5,0), width, height);
		openPopup( popup, parent, rcOpen, flag);
	} else if( obj ) {
		popup.open(parent, obj);
	} else {
		popup.open();
	}
}
page.getPage(widget) {
	p=widget;
	while( p ) {
		if( p.tag.eq('page','dialog','main') ) return p;
		p=p.parent();
	}
	return null;
}
page.getPageImpl(page) {
	p=this.getPage(widget);
	return nvl( p.pageImpl, p.pi );
}
page.getPageConfig(widget) {
	cf=null;
	pi=this.getPageImpl(widget);
	if( pi ) pi.inject(cf);
	return cf;
}
page.getMainPage(page,funcName) {
	pp=when( page.tag.eq('page','dialog','main'), page.parentPage, this.getPage(page) );
	not( pp ) return page;
	return this.getMainPage(pp);
}
page.getParentFunction(page,funcName) {
	pp=when( page.tag.eq('page','dialog','main'), page.parentPage, this.getPage(page) );
	not( pp ) return null;
	if( pp[$funcName] ) {
		return pp[$funcName];
	}
	return this.getParentFunction(pp, funcName);
}

pageImplGrid.pageImplGrid(page, pageCd) {
	this.addClass(dev.page);
	cf.val( projectCd: 'dev', pageCd );
	grid=page.grid;
	grid.check('sortEnable',true);
	switch( pageCd ) {
	case pageVar:
		grid.model( instance('pageVar.model'), 'type:타입#45, id:아이디#180, tag:태그#80, object:객체#85, conf:설정#75' );
		grid.eventMap(onDraw, pageVarDraw, 'draw, node, over');
	default break;
	}
}
pageImplGrid.pageVarDraw(draw, node, over) {
	rc=draw.rect();
	if( draw.state(STYLE.Selected) ) {
		draw.fill( rc, '#f0f0f0' );
	} else {
		draw.fill();
	}
	if( over ) draw.rectLine(rc, 4, '#f0c0a0');
	field=grid.field(draw.index());
	switch( field ) {
	case type:
		if( node[type].eq('widget') ) {
			icon="vicon.brick_edit";
		} else {
			icon="vicon.image_link";
		}
		draw.icon(rc.center(16,16), icon);
	default:
		draw.text( rc.incrX(2), node[$field] );
	}
	not( over ) draw.rectLine(rc,4,'#d0d0d0');
}

commFuncImpl.commFuncImpl(mainPage) {
	this.addPage(dev.page);
	db =instance('pages.model');
	funcGroup=mainPage.funcGroup;
	funcName=mainPage.funcName;
	funcGroup.eventMap(onFocusIn, this.funcGroupFocus);
	funcGroup.eventMap(onChange, this.funcGroupChange);
	funcGroup.eventMap(onDraw, this.funcGroupDraw, 'draw, index, over' );
	funcGroup.check('editable', true);
	funcGroup.delegate(true, 24);
	funcName.eventMap(onFocusIn, this.funcNameFocus);
	funcName.eventMap(onChange, this.funcNameChange);
	funcName.eventMap(onDraw, this.funcNameDraw, 'draw, index, over' );
	funcName.check('editable', true);
	funcName.delegate(true, 24);
	if( mainPage.funcHist ) {
		funcHist=mainPage.funcHist;
		funcHist.eventMap(onChange, this.funcHistChange);
		if( mainPage.closeAll ) {
			mainPage.closeAll.eventMap(onClick, this.funcHistCloseAll);
		}
	}
	if( mainPage.content ) {
		mSrcContent=mainPage.content;
		pageDummy=mainPage.widget(conf('widget#popup.dev#funcEdit'));
		mSrcContent.addPage(pageDummy,true);
	}
	this.initForm();
}
commFuncImpl.initForm() {
	root=instance('funcInfo.model').rootNode();
	sub=root.child(0);
	not( sub ) sub=root.addNode({ type:root, title: 공통함수 정보});
	not( sub.childCount() ) {
		db.fetchAll(conf('sql.funcGroup'), sub );
	}
	this.funcGroupMaxStr(sub);
	funcGroup.removeAll().addItem(sub, 'func_grp', '=함수그룹=');
	if( funcHist ) {
		funcHist.removeAll();
	}
}
commFuncImpl.funcGroupFocus() {
	mainPage.delay( callback() {
		funcGroup.selectText();
	}, true);
}
commFuncImpl.funcNameFocus() {
	mainPage.delay( callback() {
		funcName.selectText();
	},true);
}
commFuncImpl.funcGroupChange() {
	val=funcGroup.value();
	root=funcGroup.rootNode(), sub=null;
	not( val ) {
		val=funcGroup.text();
		if( val ) {
			sub=root.addNode().val(type:'funcGroup', func_grp: val, status:'new' );
			this.funcGroupMaxStr(root);
		}
	}
	this.funcGroupVal=val;
	not( val ) return;
	not( sub ) {
		sub=root.findOne('func_grp', val);
	}
	not( sub.childCount() ) {
		db.fetchAll(conf('sql.funcGroupChild'), sub );
	}
	this.funcNameMaxStr(sub);
	funcName.removeAll().addItem(sub, 'func_nm', '=함수명=');
	funcName.focus();
	funcName.selectText();
}
commFuncImpl.funcGroupDraw(draw, index, over) {
	node= class('draw').comboDraw(funcGroup, draw, index, over, this.funcGroupVal, 'func_grp');
	not( node ) return;
	rc=draw.rect();
	rcIcon=rc.width(24).center(16,16);
	rc.incrX(24);
	if( node.status.eq('new') ) {
		draw.font(8,'blod','#f090a0').text(rc.width(16), '*', 'center');
		rc.incrX(16);
	}
	funcCnt="(함수: ${cur[cnt]}개)";
	w=draw.textWidth(funcCnt)+10;
	draw.icon(rcIcon, "vicon.table_gear" );
	draw.font(10).text(rc, node[func_grp]);
	draw.font(8).text(rc.move('end',w), funcCnt, 'right');
}
commFuncImpl.funcNameChange() {
	val=funcName.value(), root=funcName.rootNode();
	not( val ) {
		val=funcName.text();
		if( val ) {
			sub=root.addNode().val(type: 'funcSrc', func_nm: val, status:'new');
			this.funcNameMaxStr(root);
		}
	}
	this.funcNameVal=val;
	not( val ) return;
	not( sub ) {
		sub=root.findOne('func_nm',val);
	}
	this.funcChange(this.funcGroupVal, val, sub);
}
commFuncImpl.funcNameDraw(draw, index, over) {
	node= class('draw').comboDraw(funcName, draw, index, over, this.funcNameVal, 'func_nm');
	not( node ) return;
	rc=draw.rect();
	rcIcon=rc.width(24).center(16,16);
	rc.incrX(24);
	if( node.status.eq('new') ) {
		draw.font(8,'blod','#f090a0').text(rc.width(16), '*', 'center');
		rc.incrX(16);
	}
	switch( node[func_type] ) {
	case 'A':	icon="ficon.script-code";
	case 'C':	icon="ficon.script-globe";
	case 'S':	icon="ficon.script-block";
	case 'T':	icon="ficon.script-attribute-t";
	case 'Z':	icon="ficon.script--exclamation";
	default:	icon="ficon.script-code";
	}
	draw.icon( rcIcon, icon);
	draw.font(10).text( rc,  node.func_nm);
	param=when( node[func_param], "(${node[func_param]})");
	if( param ) {
		w=draw.textWidth(param)+10;
		draw.font(8).text( rc.move('end',w), param, 'right');
	}
}
commFuncImpl.funcHistChange() {
	dist=System.tick() - this.funcChangeTick;
	if( dist<500 ) return;
	val=funcHist.value();
	kind=val.find(":"), code=kind.right().trim();
	if( kind.eq('공통함수') ) {
		group=code.find('.').trim(), name=code.find('.').right().trim();
		this.funcChange(group, name);
	}
}
commFuncImpl.funcHistCloseAll() {
	arr=this.funcHist.@adds;
	findPage=func(funcCode) {
		while( page, mSrcContent.widget() ) {
			if( page.funcCode && page.funcCode.eq(funcCode) ) return page;
		}
		return null;
	};
	while( a, arr ) {
		line=a.ref();
		kind=line.findPos(':').trim();
		if( kind.eq('공통함수') ) {
			funcCode=line.trim();
			page=findPage(funcCode);
			mSrcContent.removePage(page, true);
		}
	}
}
commFuncImpl.funcGroupMaxStr(node) {
	maxStr='';
	while( cur, node ) {
		text="${cur[func_grp]}\t(함수: ${cur[cnt]}개)";
		if( maxStr.size()<text.size() ) maxStr=text;
	}
	funcGroup.addText(maxStr, true);
}
commFuncImpl.funcNameMaxStr(node) {
	maxStr='';
	while( cur, node ) {
		text="${cur[func_nm]}\t($cur[func_param])";
		if( maxStr.size()<text.size() ) maxStr=text;
	}
	funcName.addText(maxStr, true);
}
commFuncImpl.funcChange(group, name, node) {
	this.funcChangeTick=System.tick();
	funcGroup.findLayout().showAll();
	not( group ) {
		group=db.value("select cmsCode from cmsFunc where funcName='$name' order by tm desc");
		if( group ) {
			funcGroup.value(group);
		} else {
		}
	}
	funcCode="${group}.${name}";
	/* 함수 히스토리에 추가한다 */
	if( funcHist && node ) {
		class('widget').comboValue(funcHist, "공통함수: $funcCode");
	}
	not( mSrcContent ) {
		return mainPage.funcChange(group, name);
	}
	ok=false;
	/* 함수선택 및 새로운 함수 편집 페이지 추가 */
	while( page, mSrcContent.widget() ) {
		not( page[funcCode] ) continue;
		if( page[funcCode].eq(funcCode) ) {
			mSrcContent.addPage(page,true);
			ok=true;
		}
	}
	not( ok ) {
		page=mainPage.widget(conf('widget#popup.dev#funcEdit'));
		page[funcCode]=funcCode;
		if( node ) {
			not( node[src] ) {
				not( node[func_grp] ) node[func_grp]=group;
				db.fetch("select funcParam as func_param, funcData as src, note, type as func_type from cmsFunc where cmsCode=#{func_grp} and funcName=#{func_nm}", node);
			}
			page.pageImpl.setCommFunc(node);
		}
		mSrcContent.addPage(page,true);
	}
}
commFuncImpl.classFuncChange(node) {
	funcGroup.findLayout().hideAll();
	mainPage.classFuncChange(node);
}

parseSrc.parseSrc(srcPath, page) {
	this.addClassFunc(dev.page);
	db=instance('help.model');
	not( db.open() ) {
		db.open('data/help.db');
	}
	cf={};
	tm=System.localtime();
	srcData=instance('my.file').readAll(srcPath);
}
parseSrc.parse() {
	this.parseReg(srcData.ref());
	this.parseCallFunc(srcData.ref());
	this.parseExecFunc(srcData.ref());
}
parseSrc.parseReg(&s) {
	while( true ) {
		s.findPos('inline void reg');
		not( s.valid() ) break;
		fc = s.findPos('(').trim();
		fnm=fc.value(0,-1);
		print("fnm=$fnm");
		param=s.findPos(')').trim();
		if( s.ch().eq('{')  ) {
			body=s.match();
			map=this.parseRegMap(body, fnm );
		} else {
			page.alert("parseReg : $fc 함수 분석오류");
			break;
		}
	}
}
parseSrc.parseRegMap(&s, fc) {
	s.findPos('U16'), idx=0;
	map=this.node("map#${fc}");
	while( s.valid() ) {
		s.ch();
		line=s.findPos("\n");
		if( line.start("uid") ) {
			line.findPos('=');
			idx=line.findPos(';').trim();
		} else if(line.start('hash.add') ) {
			line.findPos("(");
			if( line.ch().eq() ) {
				map[$idx]=line.match();
			}
			idx++;
		}
	}
	return map;
}
parseSrc.parseCallFunc(&s) {
	while( true ) {
		s.findPos("bool call");
		not( s.valid()  ) break;
		fc=s.findPos("(").trim();
		not( fc ) continue;
		print("@@ parseCallFunc fc=====$fc");
		if( fc.eq('UserFunc', 'NodeFunc') ) continue;
		param=s.findPos(")").trim();
		not( s.ch().eq('{') ) continue;
		body=s.match();
		if( body.find('switch') ) {
			this.parseCallSwitch(body, fc, param);
		} else if( body.find('ccmp(fnm') ) {
			this.parseCallBody(body, fc, param);
		}
	}
}
parseSrc.parseExecFunc(&s) {
	while( true ) {
		s.findPos("bool exec");
		not( s.valid() ) break;
		fc=s.findPos("(").trim();
		print("@@ parseExecFunc fc=====$fc");
		param=s.findPos(")").trim();
		not( s.ch().eq('{') ) continue;
		body=s.match(1);
		if( fc.eq('ObjectFunc', 'CheckFunc', 'MemberFunc', 'InternalFunc') ) {
			if( fc.eq('CheckFunc') ) {
				this.parseCheckFunc(body, fc, param);
			} else if( fc.eq('ObjectFunc') ) {
				this.parseObjectFunc(body, fc, param);
			}
		} else {
			if( body.find('switch') ) {
				this.parseCallSwitch(body, fc, param);
			} else if( body.find('ccmp(fnm') ) {
				this.parseCallBody(body, fc, param);
			}
		}
	}
}
parseSrc.parseCheckFunc(s, fc, param) {
	map=this.node("map#CheckFunc");
	s.findPos('switch');
	while( true ) {
		s.findPos("case");
		not( s.valid() ) break;
		idx=s.findPos(":").trim();
		if( s.ch().eq('{') ) {
			sp=s.cur();
			line=s.findPos("\n");
			if( line.find("//") ) {
				line.findPos("//");
				fnm=line.move().trim();
			} else {
				fnm=null;
			}
			s.pos(sp);
			body=s.match();
			if( fnm ) {
				map[$fnm]=body;
			}
		}
	}
}
parseSrc.parseObjectFunc(s, fc, param) {
	s.findPos("switch");
	s.findPos("case 'm'");
	s.findPos("stat==2");	// mime
	s.findPos("{",1,1);
	body=s.match(1);
	this.parseCallBody(body, 'MimeData');
	s.findPos("stat==3");	// drag
	s.findPos("{",1,1);
	body=s.match(1);
	this.parseCallBody(body, 'Drag');
	s.findPos("case 'v'");
	s.findPos("{",1,1);
	body=s.match(1);
	body.findPos("calse 0");
	body.findPos("{",1,1);
	sub=body.match(1);
	this.parseCallBody(sub, 'HttpServerThread');
	s.findPos("case '3'");
	s.findPos("{",1,1);
	body=s.match();
	this.parseCallSwitch(body,'NumberFunc');
}
parseSrc.print() {
	while( key, cf.keys("map#") ) {
		map=cf[$key];
		mapKeys=map.keys();
		print("key=$key, $mapKeys");
	}
}
parseSrc.parseCallBody(s, fc, param) {
	map=cf[map#$fc];
	if( map ) {
		// page.alert("parseCallBody : $fc : $cf[map#$fc] 이미 정의됨");
		return;
	}
	map=this.node("map#$fc");
	map[@param]=param;
	cf[object_cd]=fc;
	while( true ) {
		s.findPos('ccmp(fnm');
		not( s.valid() ) break;
		not( s.ch().eq(',') ) continue;
		s.incr();
		if( s.ch().eq() ) {
			fnm=s.match().trim();
			s.findPos('{',1,1);
			map[$fnm]=s.match(1);
			cf[func_nm]=fnm;
			cf[func_src]=map[$fnm];
			db.exec("insert into core_object_func( object_cd, func_nm, func_src, func_idx ) values ( #{object_cd}, #{func_nm}, #{func_src}, #{func_idx})", cf);
			cf[func_idx++];
		}
	}
}
parseSrc.parseCallSwitch(s,fc,param) {
	map=cf[map#$fc];
	left=s.findPos("switch");
	not( map ) {
		if( left.find('ccmp(fnm') ) {
			map=this.node("map#$fc");
			while( true ) {
				left.findPos('ccmp(fnm');
				not( left.valid() ) break;
				not( left.ch().eq(',') ) continue;
				left.incr();
				c=left.ch();
				if( c.eq() ) {
					fnm=left.match().trim();
					if( left.ch().eq(")") ) {
						left.incr();
						if( left.ch().eq('?') ) {
							left.incr();
							val=left.findPos(":").trim();
							map[$val]=fnm;
						} else if( c.eq('{') ) {
							print("xxxxxxxx $fc xxxxxxxxxxx");
						}
					}
				}
			}
		} else {
			page.alert("parseCallSwitch : $fc : 함수 정의가 존재하지 않음");
			return;
		}
	}
	cf[object_cd]=fc;
	while( true ) {
		s.findPos('case');
		not( s.valid() ) break;
		val=s.findPos(':').trim();
		fnm=map[$val];
		not( fnm ) continue;
		if( s.ch().eq('{') ) {
			map[$fnm]=s.match(1);
		} else {
			map[$fnm]="";
		}
		cf[func_nm]=fnm;
		cf[func_src]=map[$fnm];
		cf[func_idx]=val;
		db.exec("insert into core_object_func( object_cd, func_nm, func_src, func_idx ) values ( #{object_cd}, #{func_nm}, #{func_src}, #{func_idx})", cf);
	}
}

pageImplEditor.pageImplEditor(page, pageCd) {
	this.addClass(dev.page, dev.EditorSrc);
	cf.val( projectCd: 'dev', pageCd );
	switch(pageCd ) {
	case commFunc:
		page.save.eventMap(onClick, saveCommFunc);
		page.run.eventMap(onClick, runCommFunc);
		page.func_type.eventMap(onChange, commFuncTypeChange);
		page.func_type.addItem( class('code').getCodeNode('funcSrcType'), 'code,value', '==선택==' );
	case classFunc:
		page.save.eventMap(onClick, saveClassFunc);
		page.func_type.eventMap(onChange, classFuncTypeChange);
		page.func_type.addItem( class('code').getCodeNode('func_type'), 'code,value', '==선택==' );
	}
}
pageImplEditor.setCommFunc(node) {
	cf.currentNode=node;
	page.func_type.value( node[func_type] );
	this.setSrc( "${node[func_nm]}($node[func_param]) {$node[src]}" );
	this.func_desc( node[note] );
}
pageImplEditor.commFuncTypeChange() {
	page.save.enable();
}
pageImplEditor.saveCommFunc() {
	node=cf.currentNode;
	not( page.confirm("$node[func_nm] 함수를 저장 하시겠습니까?") ) return;
	db=instance('pages.model');
	node.tm=System.localtime();
	src=this.src.value();
	type=page.func_type.value();
	node.note=page.func_desc.value();
	node.func_type=type;
	parse=func(&s) {
		rtn=0;
		sp=firstCommentSkip(s);
		s.pos(sp);
		fnm=s.move().trim();
		not( fnm.eq(node[func_nm]) ) {
			not( page.confirm("함수명이 다릅니다 새이름으로 저장할까요?\n함수명 $fnm (이전함수명 $node[func_nm])") ) {
				return 0;
			}
			rtn=2;
		}
		if( s.ch().eq('(') ) {
			param=s.match();
			if( s.ch().eq('{') ) {
				src=s.match(1);
				if( src.find('/*') || src.find('//') ) {
					node[funcSrc]=makeSrc(src);
				}
				node[src]=src;
				node[func_param]=param;
				rtn=1;
			}
		}
		return rtn;
	};
	rtn=parse(src.ref());
	if( rtn.eq(2) ) {
		db.exec( conf('sql#dev.funcInsert'), node)
	} else if( rtn ) {
		not( db.exec( conf('sql#dev.funcUpdate'), node) ) {
			db.exec( conf('sql#dev.funcInsert'), node);
		}
	}
	persist=when( type.eq('S'), true);
	Cf.func(src, persist);
	this.save.disable();
}
pageImplEditor.runCommFunc() {
	node=cf.currentNode;
	type=page.func_type.value();
	persist=when( type.eq('S'), true);
	Cf.func(page.src.value(), persist);
}
pageImplEditor.setClassFunc(node) {

}
pageImplEditor.saveClassFunc() {

}
pageImplEditor.classFuncTypeChange() {
	page.save.enable();
}

previewFunctionImpl.previewFunctionImpl(mainPage) {
	this.addPage(dev.page);
	db =instance('pages.model');
	funcGroup=mainPage.funcGroup;
	funcName=mainPage.funcName;
	funcGroup.eventMap(onFocusIn, this.funcGroupFocus);
	funcGroup.eventMap(onChange, this.funcGroupChange);
	funcGroup.eventMap(onDraw, this.funcGroupDraw, 'draw, index, over' );
	funcGroup.check('editable', true);
	funcGroup.delegate(true, 24);
	funcName.eventMap(onFocusIn, this.funcNameFocus);
	funcName.eventMap(onChange, this.funcNameChange);
	funcName.eventMap(onDraw, this.funcNameDraw, 'draw, index, over' );
	funcName.check('editable', true);
	funcName.delegate(true, 24);
	if( mainPage.funcHist ) {
		funcHist=mainPage.funcHist;
		funcHist.eventMap(onChange, this.funcHistChange);
		if( mainPage.closeAll ) mainPage.closeAll.eventMap(onClick, this.funcHistCloseAll);
		if( mainPage.closeCurrent ) mainPage.closeAll.eventMap(onClick, this.funcHistCloseCurrent);
	}
	if( mainPage.content ) {
		mSrcContent=mainPage.content;
		pageDummy=mainPage.widget(conf('widget#editor.dev#funcEdit'));
		mSrcContent.addPage(pageDummy,true);
	}
	this.initForm();
}
previewFunctionImpl.initForm() {
	root=instance('funcInfo.model').rootNode();
	sub=root.child(0);
	not( sub ) sub=root.addNode({ type:root, title: 공통함수 정보});
	not( sub.childCount() ) {
		db.fetchAll(conf('sql.funcGroup'), sub );
	}
	this.funcGroupMaxStr(sub);
	funcGroup.removeAll().addItem(sub, 'func_grp', '=함수그룹=');
	if( funcHist ) {
		funcHist.removeAll();
	}
}
previewFunctionImpl.funcGroupFocus() {
	mainPage.delay( callback() {
		funcGroup.selectText(true);
	}, true);
}
previewFunctionImpl.funcNameFocus() {
	mainPage.delay( callback() {
		funcName.selectText(true);
	},true);
}
previewFunctionImpl.funcGroupChange() {
	val=funcGroup.value();
	root=funcGroup.rootNode(), sub=null;
	not( val ) {
		val=funcGroup.text();
		if( val ) {
			sub=root.addNode().val(type:'funcGroup', func_grp: val, status:'new' );
			this.funcGroupMaxStr(root);
		}
	}
	this.funcGroupVal=val;
	not( val ) return;
	not( sub ) {
		sub=root.findOne('func_grp', val);
	}
	not( sub.childCount() ) {
		db.fetchAll(conf('sql.funcGroupChild'), sub );
	}
	this.funcNameMaxStr(sub);
	funcName.removeAll().addItem(sub, 'func_nm', '=함수명=');
	funcName.focus();
	funcName.selectText();
}
previewFunctionImpl.funcGroupDraw(draw, index, over) {
	node= class('draw').comboDraw(funcGroup, draw, index, over, this.funcGroupVal, 'func_grp');
	not( node ) return;
	rc=draw.rect();
	rcIcon=rc.width(24).center(16,16);
	rc.incrX(24);
	if( node.status.eq('new') ) {
		draw.font(8,'blod','#f090a0').text(rc.width(16), '*', 'center');
		rc.incrX(16);
	}
	funcCnt="(함수: ${node[cnt]}개)";
	draw.icon(rcIcon, "vicon.table_gear" );
	draw.font(10).text(rc, node[func_grp]);
	draw.font(8).text(rc.move('end',75), funcCnt );
}
previewFunctionImpl.funcNameChange() {
	val=funcName.value(), root=funcName.rootNode();
	not( val ) {
		val=funcName.text();
		if( val ) {
			sub=root.addNode().val(type: 'funcSrc', func_nm: val, status:'new');
			this.funcNameMaxStr(root);
		}
	}
	this.funcNameVal=val;
	not( val ) return;
	not( sub ) {
		sub=root.findOne('func_nm',val);
	}
	this.funcChange(this.funcGroupVal, val, sub);
}
previewFunctionImpl.funcNameDraw(draw, index, over) {
	node= class('draw').comboDraw(funcName, draw, index, over, this.funcNameVal, 'func_nm');
	not( node ) return;
	rc=draw.rect();
	rcIcon=rc.width(24).center(16,16);
	rc.incrX(24);
	if( node.status.eq('new') ) {
		draw.font(8,'blod','#f090a0').text(rc.width(16), '*', 'center');
		rc.incrX(16);
	}
	switch( node[func_type] ) {
	case 'A':	icon="ficon.script-code";
	case 'C':	icon="ficon.script-globe";
	case 'S':	icon="ficon.script-block";
	case 'T':	icon="ficon.script-attribute-t";
	case 'Z':	icon="ficon.script--exclamation";
	default:	icon="ficon.script-code";
	}
	draw.icon( rcIcon, icon);
	draw.font(10).text( rc,  node.func_nm);
	param=when( node[func_param], "(${node[func_param]})");
	if( param ) {
		w=draw.textWidth(param)+10;
		draw.font(8).text( rc.move('end',w), param, 'right');
	}
}
previewFunctionImpl.funcHistChange() {
	dist=System.tick() - this.funcChangeTick;
	if( dist<500 ) return;
	val=funcHist.value();
	kind=val.find(":"), code=kind.right().trim();
	if( kind.eq('공통함수') ) {
		group=code.find('.').trim(), name=code.find('.').right().trim();
		this.funcChange(group, name);
	}
}
previewFunctionImpl.funcHistCloseAll() {
	arr=this.funcHist.@adds;
	findPage=func(funcCode) {
		while( page, mSrcContent.widget() ) {
			if( page.funcCode && page.funcCode.eq(funcCode) ) return page;
		}
		return null;
	};
	while( a, arr ) {
		line=a.ref();
		kind=line.findPos(':').trim();
		if( kind.eq('공통함수') ) {
			funcCode=line.trim();
			page=findPage(funcCode);
			mSrcContent.removePage(page, true);
		}
	}
	this.funcHist.removeAll();
}
previewFunctionImpl.funcHistCloseCurrent() {
	val=this.funcHist.value();
	line=val.ref();
	line.findPos(':');
	code=line.trim();
	while( page, mSrcContent.widget() ) {
		if( page.funcCode && page.funcCode.eq(code) ) {
			mSrcContent.removePage(page);
		}
	}
	this.funcHist.remove(val);
}
previewFunctionImpl.funcGroupMaxStr(node) {
	maxStr='';
	while( cur, node ) {
		text="${cur[func_grp]}\t(함수: ${cur[cnt]}개)";
		if( maxStr.size()<text.size() ) maxStr=text;
	}
	funcGroup.addText(maxStr, true);
}
previewFunctionImpl.funcNameMaxStr(node) {
	maxStr='';
	while( cur, node ) {
		text="${cur[func_nm]}\t($cur[func_param])";
		if( maxStr.size()<text.size() ) maxStr=text;
	}
	funcName.addText(maxStr, true);
}
previewFunctionImpl.funcChange(group, name, node) {
	this.funcChangeTick=System.tick();
	funcGroup.findLayout().showAll();
	not( group ) {
		group=db.value("select cmsCode from cmsFunc where funcName='$name' order by tm desc");
		if( group ) {
			funcGroup.value(group);
		} else {
		}
	}
	funcCode="${group}.${name}";
	/* 함수 히스토리에 추가한다 */
	if( funcHist && node ) {
		class('widget').comboValue(funcHist, @tr("공통함수: [#]",funcCode) );
	}
	not( mSrcContent ) {
		return mainPage.funcChange(group, name);
	}
	ok=false;
	/* 함수선택 및 새로운 함수 편집 페이지 추가 */
	while( page, mSrcContent.widget() ) {
		not( page[funcCode] ) continue;
		if( page[funcCode].eq(funcCode) ) {
			mSrcContent.addPage(page,true);
			ok=true;
		}
	}
	not( ok ) {
		page=mainPage.widget( tr('widget#editor.dev#funcEdit','commFunc') );
		page[funcCode]=funcCode;
		if( node ) {
			not( node[src] ) {
				not( node[func_grp] ) node[func_grp]=group;
				db.fetch("select funcParam as func_param, funcData as src, note, type as func_type from cmsFunc where cmsCode=#{func_grp} and funcName=#{func_nm}", node);
			}
			page.pageImpl.setCommFunc(node);
		}
		mSrcContent.addPage(page,true);
	}
}
previewFunctionImpl.classFuncChange(node) {
	funcGroup.findLayout().hideAll();
	classFuncCode=tr("[#].[#].[#]", node[class_grp], node[class_nm], node[class_func] );
	if( funcHist && node ) {
		class('widget').comboValue(funcHist, @tr("클래스함수: [#]",classFuncCode) );
	}
	not( mSrcContent ) {
		return mainPage.classFuncChange(node);
	}
	ok=false;
	/* 함수선택 및 새로운 함수 편집 페이지 추가 */
	while( page, mSrcContent.widget() ) {
		not( page[classFuncCode] ) continue;
		if( page[classFuncCode].eq(classFuncCode) ) {
			mSrcContent.addPage(page,true);
			ok=true;
		}
	}
	not( ok ) {
		page=mainPage.widget( tr('widget#editor.dev#funcEdit','classFunc') );
		page[classFuncCode]=classFuncCode;
		if( node ) {
			page.pageImpl.setClassFunc(node);
		}
		mSrcContent.addPage(page,true);
	}
}
previewFunctionImpl.setStatus(msg) {
	mainPage.funcStatus.value(msg);
}
previewFunctionImpl.setCommFunc(funcNm, alert) {
	group=db.value("select cmsCode from cmsFunc where funcName='$funcNm' order by tm desc");
	not( group ) {
		if( alert ) mainPage.alert("$funcNm 함수를 찾을수 없습니다.");
		return false;
	}
	funcGroup.value(group);
	mainPage.delay(callback() {
		funcName.value(funcNm);
	},true);
	return true;
}

widget.widget() {
	dataNode = {};
	src='';
}
widget.comboValue(combo,val) {
	not( val ) return false;
	index=combo.find(val);
	if( index.eq(-1) ) {
		combo.addItem(val);
		index=combo.find(val);
		combo.current(index);
		return true;
	} else {
		combo.current(index);
	}
	return false;
}
widget.comboReload(combo, val) {
	root=combo.rootNode();
	combo.removeAll().addItem(root, combo[@key], combo[@title]);
	if( val ) combo.value(val);
}

CoreSourceParse.CoreSourceParse(srcPath, page) {
	this.addClassFunc(dev.page);
	db=instance('help.model');
	not( db.open() ) {
		db.open('data/help.db');
	}
	cf={};
	tm=System.localtime();
	srcData=instance('my.file').readAll(srcPath);
}
CoreSourceParse.parse() {
	this.parseReg(srcData.ref());
	this.parseCallFunc(srcData.ref());
	this.parseExecFunc(srcData.ref());
}
CoreSourceParse.parseReg(&s) {
	while( true ) {
		s.findPos('inline void reg');
		not( s.valid() ) break;
		fc = s.findPos('(').trim();
		fnm=fc.value(0,-1);
		print("fnm=$fnm");
		param=s.findPos(')').trim();
		if( s.ch().eq('{')  ) {
			body=s.match();
			map=this.parseRegMap(body, fnm );
		} else {
			page.alert("parseReg : $fc 함수 분석오류");
			break;
		}
	}
}
CoreSourceParse.parseRegMap(&s, fc) {
	s.findPos('U16'), idx=0;
	map=this.node("map#${fc}");
	while( s.valid() ) {
		s.ch();
		line=s.findPos("\n");
		if( line.start("uid") ) {
			line.findPos('=');
			idx=line.findPos(';').trim();
		} else if(line.start('hash.add') ) {
			line.findPos("(");
			if( line.ch().eq() ) {
				map[$idx]=line.match();
			}
			idx++;
		}
	}
	return map;
}
CoreSourceParse.parseCallFunc(&s) {
	while( true ) {
		s.findPos("bool call");
		not( s.valid()  ) break;
		fc=s.findPos("(").trim();
		not( fc ) continue;
		print("@@ parseCallFunc fc=====$fc");
		if( fc.eq('UserFunc', 'NodeFunc') ) continue;
		param=s.findPos(")").trim();
		not( s.ch().eq('{') ) continue;
		body=s.match();
		if( body.find('switch') ) {
			this.parseCallSwitch(body, fc, param);
		} else if( body.find('ccmp(fnm') ) {
			this.parseCallBody(body, fc, param);
		}
	}
}
CoreSourceParse.parseExecFunc(&s) {
	while( true ) {
		s.findPos("bool exec");
		not( s.valid() ) break;
		fc=s.findPos("(").trim();
		print("@@ parseExecFunc fc=====$fc");
		param=s.findPos(")").trim();
		not( s.ch().eq('{') ) continue;
		body=s.match(1);
		if( fc.eq('ObjectFunc', 'CheckFunc', 'MemberFunc', 'InternalFunc') ) {
			if( fc.eq('CheckFunc') ) {
				this.parseCheckFunc(body, fc, param);
			} else if( fc.eq('ObjectFunc') ) {
				this.parseObjectFunc(body, fc, param);
			}
		} else {
			if( body.find('switch') ) {
				this.parseCallSwitch(body, fc, param);
			} else if( body.find('ccmp(fnm') ) {
				this.parseCallBody(body, fc, param);
			}
		}
	}
}
CoreSourceParse.parseCheckFunc(s, fc, param) {
	map=this.node("map#CheckFunc");
	s.findPos('switch');
	while( true ) {
		s.findPos("case");
		not( s.valid() ) break;
		idx=s.findPos(":").trim();
		if( s.ch().eq('{') ) {
			sp=s.cur();
			line=s.findPos("\n");
			if( line.find("//") ) {
				line.findPos("//");
				fnm=line.move().trim();
			} else {
				fnm=null;
			}
			s.pos(sp);
			body=s.match();
			if( fnm ) {
				map[$fnm]=body;
			}
		}
	}
}
CoreSourceParse.parseObjectFunc(s, fc, param) {
	s.findPos("switch");
	s.findPos("case 'm'");
	s.findPos("stat==2");	// mime
	s.findPos("{",1,1);
	body=s.match(1);
	this.parseCallBody(body, 'MimeData');
	s.findPos("stat==3");	// drag
	s.findPos("{",1,1);
	body=s.match(1);
	this.parseCallBody(body, 'Drag');
	s.findPos("case 'v'");
	s.findPos("{",1,1);
	body=s.match(1);
	body.findPos("calse 0");
	body.findPos("{",1,1);
	sub=body.match(1);
	this.parseCallBody(sub, 'HttpServerThread');
	s.findPos("case '3'");
	s.findPos("{",1,1);
	body=s.match();
	this.parseCallSwitch(body,'NumberFunc');
}
CoreSourceParse.print() {
	while( key, cf.keys("map#") ) {
		map=cf[$key];
		mapKeys=map.keys();
		print("key=$key, $mapKeys");
	}
}
CoreSourceParse.parseCallBody(s, fc, param) {
	map=cf[map#$fc];
	if( map ) {
		// page.alert("parseCallBody : $fc : $cf[map#$fc] 이미 정의됨");
		return;
	}
	map=this.node("map#$fc");
	map[@param]=param;
	cf[object_cd]=fc;
	while( true ) {
		s.findPos('ccmp(fnm');
		not( s.valid() ) break;
		not( s.ch().eq(',') ) continue;
		s.incr();
		if( s.ch().eq() ) {
			fnm=s.match().trim();
			s.findPos('{',1,1);
			map[$fnm]=s.match(1);
			cf[func_nm]=fnm;
			cf[func_src]=map[$fnm];
			db.exec("insert into core_object_func( object_cd, func_nm, func_src, func_idx ) values ( #{object_cd}, #{func_nm}, #{func_src}, #{func_idx})", cf);
			cf[func_idx++];
		}
	}
}
CoreSourceParse.parseCallSwitch(s,fc,param) {
	map=cf[map#$fc];
	left=s.findPos("switch");
	not( map ) {
		if( left.find('ccmp(fnm') ) {
			map=this.node("map#$fc");
			while( true ) {
				left.findPos('ccmp(fnm');
				not( left.valid() ) break;
				not( left.ch().eq(',') ) continue;
				left.incr();
				c=left.ch();
				if( c.eq() ) {
					fnm=left.match().trim();
					if( left.ch().eq(")") ) {
						left.incr();
						if( left.ch().eq('?') ) {
							left.incr();
							val=left.findPos(":").trim();
							map[$val]=fnm;
						} else if( c.eq('{') ) {
							print("xxxxxxxx $fc xxxxxxxxxxx");
						}
					}
				}
			}
		} else {
			page.alert("parseCallSwitch : $fc : 함수 정의가 존재하지 않음");
			return;
		}
	}
	cf[object_cd]=fc;
	while( true ) {
		s.findPos('case');
		not( s.valid() ) break;
		val=s.findPos(':').trim();
		fnm=map[$val];
		not( fnm ) continue;
		if( s.ch().eq('{') ) {
			map[$fnm]=s.match(1);
		} else {
			map[$fnm]="";
		}
		cf[func_nm]=fnm;
		cf[func_src]=map[$fnm];
		cf[func_idx]=val;
		db.exec("insert into core_object_func( object_cd, func_nm, func_src, func_idx ) values ( #{object_cd}, #{func_nm}, #{func_src}, #{func_idx})", cf);
	}
}

draw.draw() {
	dataNode = {};
	src='';
}
draw.textBold(draw, rc, text, size, color, align) {
	draw.save();
	draw.font(size,'blod',color).text(rc, text, align);
	draw.restore();
}
draw.gridOver(draw, node, over) {
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
draw.gridModifyMark(draw, rc, color) {
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
draw.treeIcon(tree, draw, node, over, cancel , iconUse) {
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
draw.comboDraw(combo, draw, index, state, val, field) {
	root=combo.rootNode();
	rc=draw.rect();
	select=false;
	if( state>1 ) {
		select=state & 0x8000;
		over=sate& 0x2000;
	} else {
		over=state;
	}
	if( index.eq(0) && combo[@title] ) {
		if( select ) {
			draw.fill(rc,'#cadeef');
		} else if( over ) {
			draw.fill(rc,'#e0e0e0');
		}
		draw.font(10).text(rc.incrX(4,true), combo[@title]);
		draw.rectLine(rc,4,'#606060');
		return;
	}
	if( combo[@title] ) {
		index-=1;
	}
	node=root.child(index);
	not( node ) return null;
	if( field ) {
		select=node[$field].eq(val);
	}
	if( select ) {
		draw.fill(rc,'#cadeef');
		draw.rectLine(rc,4,'#a0a090');
	} else if( over ) {
		draw.fill(rc,'#e0e0e0');
	}
	return node;
}

pageVarCombo.pageVarCombo(page, combo, comboButton) {
	this.addClass(common.Page);
	db=instance('pages.model');
	currentPage=null;
	combo.delegate(true, 24);
	combo.check('editable', true);
	combo.eventMap(onFocusIn, this.comboFocus);
	combo.eventMap(onChange, this.comboChange);
	combo.eventMap(onDraw, this.comboDraw, 'draw, index, state' );
	if( comboButton ) {
		comboButton.eventMap(onClick, this.comboButtonClick);
	}
	this.makeComboData();
}
pageVarCombo.makeComboData(curPage) {
	@currentPage=nvl(curPage, page);
	root=instance("pageVar.model").rootNode();
	info=Cf.info('funcVar', currentPage, 'init');
	if( update ) {
		not( arr ) arr=[];
		arr.reuse();
		while( cur, root ) {
			if( cur[type].eq('var') ) {
				arr.add(cur);
			}
		}
		while( cur, arr ) {
			root.remove(cur);
		}
	} else {
		root.removeAll();
		while( cur, currentPage.widgets() ) {
			sub=root.addNode();
			sub.initNode(cur);
			sub[type]='widget';
		}
	}
	_parse=func(&s) {
		while( s.valid() ) {
			line=s.findPos("\n");
			id=line.findPos('=').trim();
			if( id.ch().eq('@') ) continue;
			sub=root.addNode();
			sub[type]='var';
			sub[id]=id;
			if( line.find('(') ) {
				sub[tag]=line.findPos('(');
				sub[object]=line.findPos(')');
			} else {
				sub[tag]=line.trim();
			}
		}
	};
	_parse( info.ref() );
	while( key, currentPage.keys(keys) ) {
		if( key.eq('layout','tag') ) continue;
		if( key.ch().eq('@') ) continue;
		a=currentPage[$key], ty=typeof(a);
		if( ty.eq("function") ) continue;
		sub=root.addNode();
		sub[type]='member';
		sub[id]=key;
		if( ty.eq("object") ) {
			if( typeof(a,'class') ) {
				sub[tag]=a[@className];
				sub[object]='class';
			} else if( typeof(a,'widget') ) {
				sub[tag]=a[tag];
			}
		}
		sub[tag]=typeof(a);
	}
	maxStr='';
	while( cur, root ) {
		str="$cur[id]\t[$cur[tag]]  $cur[object]";
		if( maxStr.size()<str.size() ) {
			maxStr=str;
		}
	}
	combo.addText(maxStr, true);
	combo.removeAll().addItem(root, 'id', '=전역변수=');
}
pageVarCombo.comboChange() {
	this.comboVal=combo.value(), cur=null;
	page.pageVarComboChange(this.comboVal, combo);
}
pageVarCombo.comboDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(combo, draw, index, state);
	not( node ) return;
	rc=draw.rect();
	tag=node[tag];
	if( node[object] ) {
		tag.add(': ', node[object]);
	}
	w=draw.textWidth(tag)+15;
	rcIcon=rc.width(18);
	rc.incrX(20);
	if( node[type].eq('widget') ) {
		icon="vicon.brick_edit";
	} else if( node[type].eq('member') ) {
		icon="vicon.email_open";
	} else {
		icon="vicon.image_link";
	}
	draw.icon(rcIcon.center(16,16), icon);
	draw.font(10).text(rc, node[id]);
	if( tag ) {
		draw.font(8).text(rc.move('end',w), "[$tag]", 'right');
	}
}
pageVarCombo.comboFocus() {
	this.focusCombo=combo;
	page.delay(callback() {
		this.focusCombo.selectText(true);
	}, this);
}
pageVarCombo.comboButtonClick() {
	popup=this.getPopup('pageVar',  tr('widget#grid.popup','pageVar'));
	popup.initPage(this);
	class('page').openPopup(popup, page, comboButton, 550, 480);
}

pageFuncCombo.pageFuncCombo(page, combo) {
	db=instance('pages.model');
	currentPage=null;
	currentStep=null;
	dataNode={};
	combo.delegate(true, 24);
	combo.check('editable', true);
	combo.eventMap(onFocusIn, this.comboFocus);
	combo.eventMap(onChange, this.comboChange);
	combo.eventMap(onDraw, this.comboDraw, 'draw, index, state' );
	this.makeComboData();
}
pageFuncCombo.makeComboData(curPage) {
	not( curPage ) curPage=page;
	dataNode.initNode();
	keys=class('util').arr('pageKeys');
	arrEvent=class('util').arr('pageEvent');
	arrCtrlEvent=class('util').arr('pageCtrlEvent');
	arrUserFunc=class('util').arr('pageUserFunc');
	while( key, curPage.keys(keys) ) {
		if( key.ch().eq('@') ) continue;
		a=curPage[$key];
		not( typeof(a).eq("function") ) continue;
		param=Cf.funcParam(a);
		if( key.eq('onInit') ) {
			dataNode.addNode({funcName:$key, sort:1});
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
	while( cur, arrEvent ) dataNode.addNode(cur);
	while( cur, arrCtrlEvent ) dataNode.addNode(cur);
	while( cur, arrUserFunc ) dataNode.addNode(cur);
	maxStr='', maxSize=0;
	while( cur, dataNode ) {
		str="$cur[funcName]\t[$cur[funcParam]]";
		size=str.size();
		if( maxSize<size ) {
			maxStr=str, maxSize=size;
		}
	}
	combo.addText(maxStr, true);
	combo.removeAll().addItem(dataNode, 'funcName', '=페이지 함수=');
	@currentPage=curPage;
}
pageFuncCombo.comboFocus() {
	this.focusCombo=combo;
	page.delay(callback() {
		this.focusCombo.selectText(true);
	}, this);
}
pageFuncCombo.comboChange() {
	val=combo.value();
	root=combo.rootNode(), cur=null;
	not( val ) {
		val=combo.text();
		if( val ) {
			cur=root.addNode();
			cur.funcName=val;
			cur.state(NODE.add, true);
		}
	}
	this.comboVal=val;
	not( val ) return;
	page.pageFuncComboChange(val, combo);
}
pageFuncCombo.comboDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(combo, draw, index, state);
	not( node ) return;
	rc=draw.rect();
	param=node[funcParam];
	w=draw.textWidth(param)+15;
	rcIcon=rc.width(18);
	rc.incrX(20);
	switch( node[sort] ) {
	case 1: icon="ficon.document-code";
	case 2: icon="ficon.document-globe";
	case 3: icon="ficon.document-epub";
	case 4: icon="ficon.document-number";
	case 5: icon="ficon.document-outlook";
	}
	draw.icon(rcIcon.center(16,16), icon);
	draw.font(10).text(rc, node[funcName]);
	if( param ) {
		draw.font(8).text(rc.move('end',w), "($param)", 'right');
	}
}

pageClassCombo.pageClassCombo(page, inheritCombo, varCombo, funcCombo) {
	this.addClassFunc(dev.page);
	cf={};
	db=instance('pages.model');
	currentClass=null;
	varCombo.delegate(true, 24);
	varCombo.check('editable', true);
	funcCombo.delegate(true, 24);
	funcCombo.check('editable', true);
	inheritCombo.eventMap(onChange, this.inheritComboChange);
	varCombo.eventMap(onDraw, this.varComboDraw, 'draw, index, state' );
	varCombo.eventMap(onDraw, this.varComboChange );
	funcCombo.eventMap(onChange, this.funcComboChange );
	funcCombo.eventMap(onDraw, this.funcComboDraw, 'draw, index, state' );
	funcCombo.eventMap(onFocusIn, this.funcComboFocus);
}
pageClassCombo.makeComboData(cls) {
	@currentClass=cls;
	arr=this.arr('inherit'), checkNode=this.node('checkNode');
	arr.add(cls[@className]);
	addClassName=func(cls) {
		while( className, cls[@addClass] ) {
			if( checkNode[$className] ) continue;
			checkNode[$className]=true;
			if( className ) {
				arr.add(className);
				addClassName( Cf.info('class', className) );
			}
		}
	};
	addClassName(cls);
	inheritCombo.removeAll().addItem(arr, null, '==전체==');
}
pageClassCombo.funcComboFocus() {
	this.focusCombo=funcCombo;
	page.delay( callback() {
		this.focusCombo.selectText();
	}, this);
}
pageClassCombo.showAll() {
	inheritCombo.findLayout().showAll();
}
pageClassCombo.hideAll() {
	inheritCombo.findLayout().hideAll();
}
pageClassCombo.inheritComboChange() {
	className=inheritCombo.value();
	not( className ) {
		this.initClassCombo(currentClass, true);
		return;
	}
	cls=currentClass;
	not( className.eq(cls[@className]) ) {
		cls=Cf.info('class', className);
	}
	this.initClassCombo(cls);
}
pageClassCombo.funcComboChange() {
	/* 클래스 함수가 변경시 : 해당 함수 미리보기 화면 팝업 */
	val=funcCombo.value();
	root=funcCombo.rootNode(), cur=null;
	not( val ) {
		val=funcCombo.text();
		if( val ) {
			cur=root.addNode();
			cur[class_func]=val;
			cur.state(NODE.add, true);
		}
	}
	this.funcComboVal=val;
	not( val ) return;
	not( cur ) {
		cur=root.findOne('class_func', val);
	}
	/* 클래스 정보 세팅 */
	inherit=this.inheritCombo;
	inheritClassName=inherit.value();
	if( inheritClassName ) {
		cur[class_grp]=inheritClassName.find('.').trim();
		cur[class_nm]=inheritClassName.find('.').right().trim();
	} else {
		if( cur.status.eq('new') ) {
			this.alert("전체 클래스에서는 클래스 함수를 추가할수 없습니다. 클래스를 선택하세요");
			inherit.showPopup();
			return;
		}
		arr=inherit.rootNode();
		while( className, arr ) {
			not( className.find('.') ) continue;
			cur[class_grp]=className.find('.').trim();
			cur[class_nm]=className.find('.').right().trim();
			if( db.count("select count(1) from class_info where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", cur) ) {
				inheritClassName=className;
				break;
			}
		}
	}
	not( cur[src] ) {
		db.fetch("select case when length(class_src)==0 then class_data else class_src end as src from class_info where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", cur);
	}
	cur[inheritClassName]=inheritClassName;
	cur[currentClass]=this.currentClass;
	/*  메인페이지 인터페이스 */
	page.classFuncComboChange( cur );
}
pageClassCombo.varComboChange() {
	this.varComboVal=varCombo.value();
}
pageClassCombo.varComboDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(varCombo, draw, index, state);
	not( node) return;
	rc=draw.rect();
	w=draw.textWidth(node[type])+10;
	draw.font(10).text(rc.incrX(4,true), node[var]);
	draw.font(8).text(rc.move('end',w), "[${node[type]}]", 'right');
}
pageClassCombo.funcComboDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(funcCombo, draw, index, state);
	not( node ) return;
	rc=draw.rect();
	if( node[type] ) {
		rcIcon=rc.width(20).center(16,16);
		rc.incrX(20);
		if( node[type].eq('A') ) {
			draw.icon(rcIcon,"ficon.document-attribute-c");
		} else {
			ty=node[type].lower();
			draw.icon(rcIcon,"ficon.document-attribute-$ty");
		}
	} else {
		rc.incrX(4);
	}
	draw.font(10).text(rc, node[class_func]);
	if( node[class_param] ) {
		w=draw.textWidth(node[class_param])+30;
		draw.font(8).text(rc.move('end',w), "($node[class_param])",'right' );
	}
}
pageClassCombo.initClassCombo(cls, all) {
	str=Cf.info('funcVar', cls, 'member').str();
	node=this.node('classVar');
	maxStr='';
	while( str.valid() ) {
		line=str.findPos("\n");
		not( line.ch() ) break;;
		not( line.find('=') ) continue;
		var=line.findPos('=').trim();
		if( var.ch().eq('@') ) continue;
		cur=node.addNode();
		cur[var]=var;
		cur[type]=line.trim();
		val="$cur[var]\t    [$cur[type]]";
		if( maxStr.size() < val.size() ) maxStr=val;
	}
	varCombo.addText(maxStr, true);
	varCombo.removeAll().addItem(node,'var','==클래스 변수==');
	funcCombo.removeAll();
	node=this.node('classFunc');
	if( all ) {
		arr=cls.keys().sort();
		while( key,  arr ) {
			if( key.ch().eq('@') ) continue;
			fc=cls[$key];
			not( typeof(fc).eq("function") ) continue;
			cur=node.addNode();
			cur[class_func]=key;
			cur[class_param]=Cf.funcParam(fc);
		}
	} else {
		s=cls[@className];
		not(s ) s=inheritCombo.value();
		if( s.find('.') ) {
			node[class_grp]=s.find('.').trim();
			node[class_nm]=s.find('.').right().trim();
			db.fetchAll("select class_func, class_param, type from class_info where class_grp=#{class_grp} and class_nm=#{class_nm} order by type", node);
		} else {
			node[class_nm]=cls[@className];
			db.fetchAll("select class_func, class_param from class_info where class_nm=#{class_nm} order by type", node);
		}
	}
	maxStr='';
	while( cur, node ) {
		val="$cur[class_func] $cur[type]\t  $cur[class_param] ";
		if( maxStr.size() < val.size() ) maxStr=val;
	}
	funcCombo.addText(maxStr, true);
	funcCombo.addItem(node, 'class_func', '==클래스 함수==');
}

pageSelectCombo.pageSelectCombo(page) {
	this.addClass(common.Page);
	db=instance('pages.model');
	dataModel=instance('pageSelectCombo.model');
	pageGroup=page[pageGroup];
	pageCode=page[pageCode];
	pageSub=page[pageSub];
	if( page[pageGroupButton] ) {
		page[pageGroupButton].eventMap(onClick, this.pageGroupButtonClick);
	}
	if( page[pageCodeButton] ) {
		page[pageCodeButton].eventMap(onClick, this.pageCodeButtonClick);
	}
	pageGroup.delegate(true, 24);
	pageGroup.check('editable', true);
	pageCode.delegate(true, 24);
	pageCode.check('editable', true);
	pageGroup.eventMap(onFocusIn, this.pageGroupFocus);
	pageGroup.eventMap(onChange, this.pageGroupChange);
	pageGroup.eventMap(onDraw, this.pageGroupDraw, 'draw, index, state' );
	pageCode.eventMap(onFocusIn, this.pageCodeFocus);
	pageCode.eventMap(onChange, this.pageCodeChange);
	pageCode.eventMap(onDraw, this.pageCodeDraw, 'draw, index, state' );
	if( pageSub ) {
		pageSub.delegate(true, 24);
		pageSub.eventMap(onChange, this.pageSubChange);
		pageSub.eventMap(onDraw, this.pageSubDraw, 'draw, index, state' );
		pageSub.hide();
	}
	this.initForm();
}
pageSelectCombo.getPageId() {
	a=pageGroup.text(), b=pageCode.text();
	if( a && b ) {
		return "${a}.${b}";
	}
	return null;
}
pageSelectCombo.setComboValue() {
	this.comboChangeValue('pageGroup');
	this.comboChangeValue('pageCode');
}
pageSelectCombo.setComboPopupWidth(combo, root) {
	maxStr='';
	while( cur, root ) {
		note="$cur[code]\t\t    $cur[note]";
		if( maxStr.size()<note.size() ) maxStr=note;
	}
	combo.addText(maxStr, true);
}
pageSelectCombo.initForm() {
	root=dataModel.rootNode();
	db.fetchAll(conf('sql.pageGroupCombo'), root.removeAll() );
	this.setComboPopupWidth(pageGroup, root);
	pageGroup.addItem(root,'code',"페이지 그룹");
	pageCode.removAll();
}
pageSelectCombo.pageGroupChange() {
	cur=this.comboChangeValue('pageGroup');
	not( cur ) return;
	not( cur.childCount() ) {
		db.fetchAll( conf('sql.pageCodeCombo'), cur);
	}
	this.setComboPopupWidth(pageCode, cur);
	while( sub, cur ) {
		pageId=this.getPageId();
		if( get(pageId) ) {
			sub.pageId=pageId;
			sub.runtimePage=get(pageId);
		}
	}
	pageCode.removeAll().addItem(cur, 'code', '페이지코드');
	pageCode.focus();
}
pageSelectCombo.pageGroupDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(pageGroup, draw, index, state );
	not( node ) return;
	rc=draw.rect();
	if( node.state(NODE.add) ) {
		class('draw').textBold(draw, rc.width(16), '*', 8, '#f090a0', 'center');
		rc.incrX(16);
	}
	if( node[note] ) {
		w=draw.textWidth(node[code])+15;
		draw.font(10).text(rc.incrX(4), node[note]);
		draw.font(8).text(rc.move('end',w), "[${node[code]}]", 'right');
	} else {
		draw.font(10).text(rc.incrX(4), node[code]);
	}
}
pageSelectCombo.getValue() {
	group=pageGroup.text();
	code=pageCode.text();
	if( group && code ) return "${group}.${code}";
	return null;
}
pageSelectCombo.pageCodeDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(pageCode, draw, index, state);
	not( node ) return;
	rc=draw.rect();
	if( node.runtimePage ) {
		class('draw').textBold(draw, rc.width(16), '*', 8, '#a090f0', 'center');
		rc.incrX(16);
	} else if( node.state(NODE.add) ) {
		class('draw').textBold(draw, rc.width(16), '*', 8, '#f090a0', 'center');
		rc.incrX(16);
	}
	if( node[note] ) {
		w=draw.textWidth(node[code])+15;
		draw.font(10).text(rc.incrX(4), node[note]);
		draw.font(8).text(rc.move('end',w), "[${node[code]}]", 'right');
	} else {
		draw.font(10).text(rc.incrX(4), node[code]);
	}
	if( node[useyn].eq('N') ) {
		draw.rectLine( rc.incr(3), 5, '#eaa0a0');
	}
}
pageSelectCombo.pageCodeChange() {
	cur=this.comboChangeValue('pageCode');
	if( cur ) {
		not( cur.runtimePage ) {
			cur.runtimePage=get("${this.pageGroupVal}.${cur[code]}");
		}
		this.setPageSubCombo(cur);
		page.pageSelectChange(cur);
	}
}
pageSelectCombo.pageGroupFocus() {
	page.delay(func() {
		if( this.focusCombo==pageGroup ) return;
		this.focusCombo=pageGroup;
		pageGroup.selectText(true);
	}, this);
}
pageSelectCombo.pageCodeFocus() {
	page.delay(func() {
		if( this.focusCombo==pageCode ) return;
		this.focusCombo=pageCode;
		pageCode.selectText(true);
	}, this);
}
pageSelectCombo.pageGroupButtonClick() {
	popup=this.getPopup('pageGroup', tr('widget#grid.popup#crud','pageGroup') );
	popup.initPage(this);
	class('page').openPopup(popup, page, page[pageGroupButton]);
}
pageSelectCombo.pageCodeButtonClick() {
	popup=this.getPopup('pageCode', tr('widget#grid.popup#crud','pageCode') );
	popup.initPage(this);
	class('page').openPopup(popup, page, page[pageCodeButton]);
}
pageSelectCombo.gridDoubleClick(gridCode, node) {
	code=node[code];
	not( code ) {
		return;
	}
	switch( gridCode) {
	case pageGroup:		pageGroup.value(code);
	case pageCode:		pageCode.value(code);
	}
}
pageSelectCombo.setPageSubCombo(node) {
	not( pageSub ) return;
	pageSub.hide();
	page=node.runtimePage;
	node.removeAll();
	while( cur, page.widgets() ) {
		not( cur[tag].eq('tab','div') ) continue;
		while( subPage, cur.widget() ) {
			sub=node.addNode();
			sub.varMap( subPage, 'id, title, icon, tag');
			if( subPage[@funcName] ) {
				sub.luid="#$subPage[@funcName]";
			} else if( subPage[@cms.code] ) {
				group=subPage[@cms.code];
				sub.luid="${group}.${$subPage[id]}";
			} else {
				sub.luid=subPage[pageCd];
			}
			sub.type='subpage';
			sub.page=subPage;
			sub.parentTag=cur[tag];
			sub.parentPage=cur;
		}
	}
	if( page.pageImpl ) {
		arr=page.pageImpl.getPopupArray();
		while( subPopup, arr ) {
			sub=node.addNode();
			sub.varMap( subPopup, 'title, icon, tag');
			sub.luid=subPopup[popupCd];
			sub.type='popup';
			sub.page=subPopup;
		}
	}
	if( node.childCount() ) {
		maxStr='';
		while( sub, node) {
			title="$sub[luid]\t   $sub[title]";
			if( maxStr.size()<title.size() ) maxStr=title;
		}
		pageSub.addText(maxStr, true);
		pageSub.removeAll().addItem(node, 'luid', '서브페이지 선택');
		pageSub.show();
	}
}
pageSelectCombo.pageSubChange() {
	this.pageSubVal=pageSub.value();
}
pageSelectCombo.pageSubDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(pageSub, draw, index, state );
	not( node ) return;
	rc=draw.rect();
	if( node[title] ) {
		w=draw.textWidth(node[title])+15;
		draw.font(10).text(rc.incrX(4), node[luid]);
		draw.font(8).text(rc.move('end',w), "[${node[code]}]", 'right');
	} else {
		draw.font(10).text(rc.incrX(4), node[luid]);
	}
}

ConfManagerEdit.ConfManagerEdit(page) {
	this.addClass(common.Page, dev.EditorSrc );
}
ConfManagerEdit.initPage(root) {

}

EditorSrc.EditorSrc() {
	this.addClass( dev.EditorSrcChange, dev.EditorSrcClick );
	not( cf ) cf={};
	mPopupMode = 'popup';
	mTreeNode=null;
	mSearchPrev='';
	mFuncSrc='';
	saveButton=null, closeButton=null, lockButton=null;
	searchInput=null, searchReplaceButton=null;
	if( page.src || page.editor ) {
		editor=page.src;
	}
	not( editor ) return;
	help=instance('help.model');
	not( help.open() ) {
		help.open('data/help.db');
	}
	this.setEditorEvent();
}
EditorSrc.setEditorEvent() {
	editor.syntax( conf('syntax.dev') );
	editor.eventMap( onMouseClick, this.editorMouseClick, 'pos, keys' );
	editor.eventMap( onChange, this.editorChange );
	editor.eventMap( onKeyDown, this.editorKeyDown, 'key,mode' );
	if( page.save || page.btnSave ) {
		@saveButton=nvl( page.save, page.btnSave );
		saveButton.disable();
	}
	/* 찾기 & 바꾸기 처리*/
	if( page.inputSearch ) {
		@searchInput=page.inputSearch;
		searchInput.eventMap(onFocus, this.searchFocus);
		searchInput.eventMap(onEnter, this.searchEnterKey);
	}
	if( page.btnSearchReplace ) {
		@searchReplaceButton=page.btnSearchReplace;
		searchReplaceButton.eventMap(onClick, this.searchReplaceClick);
	}
	/* 툴버튼 처리 */
	if( page[parentPage].get('tag').eq('div', 'tab') ) {
		if( page[parentPage].get('id').eq('missionPage') ) return;
		page[btnLock].findLayout().hideAll();
		page[cancel].hide();
	} else {
		if( page.btnLock ) {
			@lockButton=page.btnLock;
			lockButton.eventMap(onClick, this.lockClick);
		}
		if( page.btnClose ) {
			@closeButton=page.btnClose;
			closeButton.eventMap(onClick, this.closeClick);
		}
	}
}
EditorSrc.editorMouseClick(pos, keys) {
	not(  keys&KEY.ctrl ) {
		return;
	}
	this.editorClickPos=pos;
	page.delay( callback() {
		pos=this.editorClickPos;
		this.srcMouseClick(pos );
	}, this);
}
EditorSrc.editorChange() {
	if( saveButton ) {
		if( editor.isModify() ) {
			saveButton.enable();
		} else {
			saveButton.disable();
		}
	}
	this.editorTextChange(editor);
}
EditorSrc.searchFocus() {
	searchInput.select();
}
EditorSrc.searchEnterKey() {
	val=searchInput.value();
	not( val ) return;
	@mSearchPrev=val;
	if( val.eq(mSearchPrev) ) {
		editor.searchNext(val);
	} else {
		editor.findAll(val);
	}
}
EditorSrc.setSrc(src) {
	editor.clear();
	if( src.find("^|") ) {
		editor.insert(src, true).clearUndo();
		page.delay( callback() {
			editor.focus();
		},this);
	} else {
		editor.insert(src).clearUndo();
		editor.move(0);
	}
	if( saveButton ) saveButton.disable();
}
EditorSrc.lockClick() {
	if( mPopupMode.eq('popup') ) {
		lock.icon("vicon.application_double");
		@mPopupMode='window';
		page.flags('window');
	} else {
		lock.icon("vicon.application_link");
		@mPopupMode='popup';
		page.flags('popup');
	}
	page.show();
}
EditorSrc.closeClick() {
	if( page.onClose ) {
		page.fireEvent("onClose");
	} else {
		page.hide();
	}
}
EditorSrc.searchReplaceClick() {
	popup=pageLoad('dev.searchAndReplace');
	popup.open();
	popup.initPage(searchInput.value());
	return popup();
}

EditorSrcChange.EditorSrcChange() {

}
EditorSrcChange.indent() {
	line = editor.sp('selectStart').spText('lineStart');
	return indentText( line.ref() );
}
EditorSrcChange.insertIndent(&str, indent, pos) {
	if( pos ) editor.move(pos);
	not( indent ) indent = this.indent();
	rst ='';
	while( str.valid(), num, 0 ) {
		left = str.findPos("\n");
		if( num ) rst.add("\n", indent);
		rst.add(left);
	}
	editor.insert( rst, true);
	return true;
}
EditorSrcChange.matchBraketSelect(type) {
	mode=cf.braketMode;
	len = str.length();
	switch(mode) {
	case 1:
		ep=editor.pos(1, true), sp=ep-len;
	case 2:
		sp=editor.pos(), ep=sp+len;
		if( type.eq('move') ) type='moveEnd';
	case 3:
		sp=editor.pos(-1, true), ep=sp+len;
		if( type.eq('move') ) type='moveEnd';
	case 4:
		ep=editor.pos(), sp=ep-len;
	}
	print("editor MatchBracketSelect : $type, $sp, $ep");
	if( sp>ep ) return;
	if( sp.eq(ep) ) {
		editor.pos(sp);
		return;
	}
	switch(type) {
	case select:			editor.move(sp).move(ep,true);
	case in:				editor.move(sp).move(ep-1,true);
	case move:			editor.pos(sp);
	case moveStart:		editor.pos(sp);
	case moveEnd:		editor.pos(ep);
	}
	return Class.point(sp,ep);
}
EditorSrcChange.matchBraket() {
	parse = func(&s, ss, es, flag) {
		if( flag&0x100 ) {
			ep = s.cur(-1);
			s.match(ss,es,flag);
			sp = s.cur();
		} else {
			sp = s.cur();
			s.match(ss,es,flag);
			ep = s.cur();
		}
		print("sp=$sp, ep=$ep");
		if( sp<ep )
			return s.value(sp, ep, true);
		return null;
	};
	bracket = func(&str,c,flag) {
		switch(c) {
		case '[':		return parse(str,'[',']',flag);
		case ']':		return parse(str,'[',']',flag);
		case '(':		return parse(str,'(',')',flag);
		case ')':		return parse(str,'(',')',flag);
		case '{':		return parse(str,'{','}',flag);
		case '}':		return parse(str,'{','}',flag);
		case '<':		return parse(str,'<','>',flag);
		case '>':		return parse(str,'<','>',flag);
		}
	};
	matchString = func(c,flag) {
		sp = str.cur();
		s.match(c,c,flag);
		ep = str.cur();
		if( sp<ep )
			return str.value(sp, ep, true);
		return null;
	};
	cf.braketMode=0;
	c = editor.sp().spText('nextChar');
	print("editorMatchBracket $c");
	if( c.eq() ) {
		cf.braketModee=5;
		str=editor.sp().spText(20000);
		matchString(str.ref(), 1);
	} if( c.eq(']',')','}','>') ) {
		cf.braketModee=1;
		str=editor.sp(1).spText(-20000), flag=0x100 | 1;
		return bracket(str.ref(),c,flag);
	} else if( c.eq('[','(','{','<') ) {
		cf.braketModee=2;
		str=editor.sp().spText(20000);
		return bracket(str.ref(),c,flag);
	}
	c = editor.sp().spText('prevChar');
	print("editorMatchBracket $c");
	if( c.eq('[','(','{','<') ) {
		cf.braketModee=3;
		str=editor.sp(-1).spText(20000), flag=1;
		return bracket(str.ref(),c,flag);
	} else if( c.eq(']',')','}','>') ) {
		cf.braketModee=4;
		str = editor.sp().spText(-20000), flag=0x100 | 1;
		return bracket(str.ref(),c,flag);
	}
	return null;
}
EditorSrcChange.braketStartMark(&str, sc, ec) {
	flag = 0x100 | 1;
	ep=str.cur(-1);
	in = str.match(sc,ec,flag);
	if( in.size() ) {
		sp = str.cur(-1);
		sp += 1;
		len = str.pos(sp,ep).length();
		if( len>2 ) {
			pos = editor.pos() - len;
			editor.insert(ec);
			editor.mark(pos, pos+1);
			return true;
		}
		return false;
	}
	System.beep();
	return false;
}
EditorSrcChange.braketStartIndet(&str) {
	// ex) str= "\t123 {abc}" 이라면
	flag = 0x100 | 1;				// 문자열 뒤부터 매칭되는 문자열을 찾는다
	// #1. 매칭되는 위치 찾아 indent 얻음
	in = str.match('{','}',flag);			// in="abc", str="\t123 ";
	not( in ) return false;
	not( in.find("\n") ) return null;	// 매칭문자열이 같은줄이라면 무시한다
	indent = str.findLast("\n").right();
	// #2  에디터 현재 위치 내용에 따른 처리
	line = editor.sp().spText('lineStart');
	val = '';
	if( line.check(" \t") ) {		// 현재위치 앞에 공백만 있다면 공백을 제거하기 위해 선택되게함 (ex) line="  }" 공백만 선택됨
		pos = editor.pos();
		start = pos-line.size();
		editor.move(start).move(pos,true);
	} else {						// 현재위치 앞에 문자가 있다면, new line 을 넣어준다 ex) line="abc }";
		val.add("\n");
	}
	// #3. indent 를 넣어준다.
	if( indent ) {
		val.add( indentText(indent) );
	} else {
		val.add( indentText(str.ref()) );
	}
	return val;
}
EditorSrcChange.editorKeyDown(key, mode) {
	editor[prevKey] = 0;
	nextCharCheck=func(c) {
		ch = editor.text('nextChar');
		if( ch.eq(c) ) {
			cur=editor.pos();
			editor.pos(cur+1);
			return true;
		}
		return false;
	};
	_makeSubPageFunc=func(data) {
		if( editor.searchPrev(".widget({") ) {
			editor.move('selectStart');
			if( editor.searchPrev("{") ) {
				editor.move('selectStart');
				editor.move('prevChar');
				val=editor.sp('lineStart').spText().str();
				sv=val.pos();
				val.move();
				if( val.ch().eq('.') ) {
					val.incr();
					val.move();
				}
				ev=val.cur();
				if( val.ch().eq('(') ) {
					funcName=val.value(sv,ev,true).trim();
					editor.move('start');
					editor.insert("test() {\n\tpage=this.${funcName}();\n\tpage.func( template(func) {\n$data^|\n\t});\n}\n#> test();\n\n",true);
				}
			}
		} else {
			System.beep();
		}
	};
	_funcReplace=func(data) {
		sp=editor.pos('lineStart');
		src=editor.sp('lineStart').spText();
		val=editor.sp().spText(4096).str();
		sv=0;
		c=val.ch();
		if( c.eq('(') ) {
			param=val.match();
			c=val.ch();
			if( c.eq('{') ) {
				body=val.match(1);
				ev=val.cur();
				src.add( val.value(sv,ev,true) );
			}
		}
		indent=indentText(src);
		ep=sp+src.length();
		editor.move(sp).move(ep,true);
		src=makeIndent(data, indent);
		print( src, sp, ep, data);
		this.insertIndent(src);
	};
	_funcAdd=func(data) {
		if( editor.search(".widget({") ) {
			sp=editor.pos('lineStart');
			val=editor.sp('lineStart').spText('end').str();
			src='';
			left=val.findPos("{",1,1);
			body=val.match(1);
			src.add(left,"{$body");
			ep=sp+src.length();
			editor.move(ep);
			src=makeIndent(data, "\t");
			print(src, ep);
			this.insertIndent("$src\n\n");
		}
	};
	switch( key ) {
	case KEY.Escape:
		editor.pos('mp',true);
	case KEY.F2:
		str = this.matchBraket();
		mode=cf.braketMode;
		if( str ) {
			pos = editor.pos();
			if( mode.eq(1,4) ) {
				ep=pos-str.length();
			} else {
				ep = pos+str.length();
			}
			if( mode.eq(1) ) {
				pos++;
			} else if( mode.eq(3) ) {
				pos--;
			} else if( mode.eq(4) ) {
				ep--;
			}
			editor.mark(pos, ep);
		}
	case KEY.F3:
		str = editor.text('select');
		not( str ) str=mSearchPrev;
		not( str ) return;
		@mSearchPrev=str;
		if( mode&KEY.shift ) {
			editor.searchPrev(str);
		} else {
			editor.searchNext(str);
		}
		return true;
	case KEY.F4:
		editor.pos('findPos',true);
		return true;
	case KEY.F5:
		val=editor.sp('prevWord','prevChar').spText();
		c=val.ch();
		if( c.eq('.') ) {
			val=editor.sp('prevWord','prevChar','prevWord').spText();
			page.alert("객체함수는 향후 구현 예정");
		} else {
			val=editor.sp('prevWord').spText(50).str();
			fnm=val.move().trim();
			if( val.ch().eq('(') ) {
				if( fnm.eq('conf','tr') ) {
					p=pageLoad('Common.ConfManager');
					p.open();
					val.incr();
					if( val.ch().eq() ) {
						param=val.match().trim();
						p.initPage(param);
					}
				} else if( help.count("select count(1) from core_object_func where object_cate='func' and func_nm='$fnm'") ) {
					page.alert("내장 함수는 향후 구현 예정");
				} else {
					p=pageLoad('Common.PreviewFunction');
					p.open();
					p.setCommFunc(fnm);
				}
			}
		}
		return true;
	case KEY.F6:
		str = editor.text('select');
		not( str ) str = editor.text('word');
		editor.findAll(str);
		return true;
	case KEY.F8:
		not( mode&KEY.ctrl ) return;
		sp=editor.pos(), ep=0;
		editor.move('start');
		while( n,128 ) {
			if( editor.search("#>") ) {
				pos=editor.pos();
				if( sp<pos ) {
					break;
				}
				ep=pos;
			} else {
				break;
			}
		}
		not( ep ) {
			editor.move(sp);
			return;
		}
		editor.move(ep);
		ep=editor.pos('lineEnd');
		editor.move('start').move(ep,true);
		editor.insert("");
		sp-=ep;
		if( sp>0 ) {
			editor.move(sp);
		}
	case KEY.F9:
		sp=editor.pos('lineStart');
		val=editor.sp('lineStart').spText(4096).str();
		sv=val.pos();
		val.move();
		if( val.ch().eq('.') ) {
			val.incr();
			val.move();
		}
		ev=val.cur();
		src=val.value(sv,ev,true);
		sv=ev;
		c=val.ch();
		if( c.eq('(') ) {
			param=val.match();
			c=val.ch();
			if( c.eq('{') ) {
				body=val.match(1);
				ev=val.cur();
				src.add( val.value(sv,ev,true) );
				ep=sp+src.length();
				editor.move(ep);
				_makeSubPageFunc(src);
			} else {
				System.beep();
			}
		} else {
			System.beep();
		}
	case KEY.F10:
		sp=editor.pos('lineStart');
		val=editor.sp('lineStart').spText(4096).str();
		sv=val.pos();
		val.move();
		if( val.ch().eq('.') ) {
			val.incr();
			val.move();
		}
		ev=val.cur();
		src=val.value(sv,ev,true);
		funcName=src.trim();
		sv=ev;
		c=val.ch();
		if( c.eq('(') ) {
			param=val.match();
			c=val.ch();
			if( c.eq('{') ) {
				val.match(1);
				ev=val.cur();
				src.add( val.value(sv,ev,true) );
				ep=sp+src.length();
				editor.move(ep);
				ok=false;
				while( n,16 ) {
					if( editor.search(funcName) ) {
						c=editor.sp().spText('nextChar');
						print(funcName, c);
						if( c.eq('(') ) {
							ok=true;
							break;
						}
					} else {
						break;
					}
				}
				if( ok ) {
					_funcReplace(src);
				} else {
					_funcAdd(src);
				}
			} else {
				System.beep();
			}
		} else {
			System.beep();
		}
	case KEY.Tab:
		if( editor.isSelect() ) return false;
		if( editor.pos('nextTab',true) ) return true;
	case KEY.Up:
		not( mode&KEY.ctrl )  return false;
		ok=false;
		if( editor.isSelect() ) {
			sp=editor.pos('selectStart', 'lineStart'), ep=editor.pos('selectEnd','lineEnd');
			if( editor.pos('selectEnd')==editor.pos('selectEnd','lineStart') ) {
				ep=editor.pos('selectEnd') - 1;
			} else {
				ep=editor.pos('selectEnd','lineEnd');
			}
			str=editor.move(sp).move(ep,true).text('select');
			if( str.find("\n") ) {
				ok=true;
			}
		}
		not( ok ) {
			sp=editor.pos('lineStart'), ep=editor.pos('lineEnd');
			str=editor.move(sp).move(ep,true).text('select');
		}
		editor.move(sp).move(ep+1,true);
		editor.insert('');
		usp=editor.pos('up','lineStart'), uep=editor.pos('up','lineEnd');
		upLine=editor.move(usp).move(uep,true).text('select');
		indent=indentText( upLine );
		upVal=upLine.trim();
		if( upVal ) {
			if( upVal.eq('}') ) {
				indent.add("\t");
			}
			str=incrTab(str.ref(), indent, true);
		}
		len=str.length();
		sp=usp, ep=usp+len;
		editor.insert("$str\n$upLine");
		editor.move(sp).move(ep,true);
		return true;
	case KEY.Down:
		not( mode&KEY.ctrl )  return false;
		ok=false;
		if( editor.isSelect() ) {
			sp=editor.pos('selectStart', 'lineStart'), ep=editor.pos('selectEnd','lineEnd');
			if( editor.pos('selectEnd')==editor.pos('selectEnd','lineStart') ) {
				ep=editor.pos('selectEnd') - 1;
			} else {
				ep=editor.pos('selectEnd','lineEnd');
			}
			str=editor.move(sp).move(ep,true).text('select');
			if( str.find("\n") ) {
				ok=true;
			}
		}
		not( ok ) {
			sp=editor.pos('lineStart'), ep=editor.pos('lineEnd');
			str=editor.move(sp).move(ep,true).text('select');
		}
		end=editor.pos('end');
		dsp=editor.pos('down','lineStart'), dep=editor.pos('down','lineEnd');
		if( dep >= end ) return;
		if( dsp < dep ) {
			downLine=editor.sp(dsp).spText(dep,true);
			indent=indentText( downLine );
			downVal=downLine.trim();
			if( downVal ) {
				if( downVal.ch(-1).eq('{') ) {
					indent.add("\t");
				}
				str=incrTab(str.ref(), indent, true);
			}
			len=str.length(), dlen=downLine.length();
			dep+=1;
			editor.move(sp).move(dep,true);
			editor.insert("$downLine\n$str\n");
			dlen+=1;
			sp+=dlen, ep=sp+len;
			editor.move(sp).move(ep,true);
		} else {
			editor.move(sp).move(dep,true);
			editor.insert("$downLine\n$str");
			sp+=1;
			len=str.length();
			ep=sp+len;
			editor.move(sp).move(ep,true);
		}
		return true;
	case KEY.Return:
		pos=editor.pos();
		if( pos == editor.pos('lineStart') ) return false;
		line = editor.sp('lineStart').spText();
		indent = indentText(line.ref());
		ch = line.prevChar();
		if( ch.eq("{", ":") ) {
			indent.add("\t");
		}
		remain=editor.sp().spText('lineEnd');
		if( remain ) {
			blank=indentText(remain);
			pos=editor.pos() + blank.size();
			editor.move(pos, true);
		}
		editor.insert("\n$indent");
		return true;
	case 34:
		if( nextCharCheck('"') ) return true;
		str=editor.text('prevWord');
		ch=str.prevChar();
		if( ch.eq('=',',','(') ) {
			editor.insert('"^|"', true);
			return true;
		}
		return false;
	case 39:
		if( nextCharCheck("'") ) return true;
		str=editor.text('prevWord');
		ch=str.prevChar();
		if( ch.eq('=',',','(') ) {
			editor.insert("'^|'", true);
			return true;
		}
		return false;
	case 41:
		str = editor.sp().spText(-2048);
		str.add(")");
		return this.braketStartMark( str.ref(), '(', ')', true);
	case 47:
		not( mode&KEY.ctrl ) {
			return false;
		}
		if( editor.isSelect() ) {
			sp=editor.pos('selectStart', 'lineStart'), ep=editor.pos('selectEnd','lineEnd');
			if( editor.pos('selectEnd')==editor.pos('selectEnd','lineStart') ) {
				ep=editor.pos('selectEnd');
			}
			str=editor.move(sp).move(ep,true).text('select');
			if( str.find("\n") ) {
				indent=indentText( str.find("\n") );
				editor.insert( "$indent/*\n$str\n$indent*/", true);
			} else {
				sp=editor.pos('selectStart'), ep=editor.pos('selectEnd');
				str=editor.move(sp).move(ep,true).text('select');
				editor.insert("/* $str */");
			}
		} else {
			line=editor.sp('lineStart').spText('lineEnd');
			sp=editor.pos('lineStart');
			sp+=indentText(line).size();
			editor.move(sp);
			editor.insert('// ');
		}
		return true;
	case 91:
		not( mode&KEY.ctrl )  return;
		if( editor.isSelect() ) {
			ep=editor.pos('selectEnd','lineEnd');
			if( editor.pos('selectEnd')==editor.pos('selectEnd','lineStart') ) {
				ep=editor.pos('selectEnd');
			}
			cur=editor.pos('selectStart');
			editor.pos(cur);
		} else {
			cur=editor.pos();
			ep=editor.pos('lineEnd');
		}
		/* 선택 시작위치 기억 */
		pos=cur;
		/* Braket 시작위치를찾는다 */
		while( n, 16 ) {
			prev = editor.pos('prevWord',true);
			txt=editor.text( prev, cur);
			val=txt.trim();
			ch=val.prevChar();
			if( val.eq('else') || ch.eq(')') ) {
				sp=cur;
				break;
			}
			cur=prev;
		}
		if( sp<pos ) {
			s=editor.sp(sp).spText(pos,true);
			if( s.find("\n") ) {
				editor.pos(pos);
				pos=editor.pos('lineStart');
			} else {
				pos=sp;
			}
		}
		line=editor.sp(sp,'lineStart').spText();
		indent=indentText( line );
		str=editor.sp(pos).spText(ep,true);
		editor.move(sp).move(ep,true);
		if( str.find("\n") ) {
			str=incrTab(str.ref(), "$indent\t", true);
			editor.insert("{\n$str\n$indent}");
		} else {
			val=str.trim();
			editor.insert("{\n${indent}\t${val}\n$indent}");
		}
		return true;
	case 123:
		str=editor.text('prevWord');
		ch=str.prevChar();
		if( str.start('else') || ch.eq(')') ) return this.insertIndent("{\n\t^|\n}");
	case 125:
		str=editor.sp().spText('start');
		str.add("}");
		indent = this.braketStartIndet(str.ref() );
		if( indent ) {
			editor.insert(indent);
		}
	case 93:
		str = editor.sp().spText(-1024);
		str.add("]");
		return this.braketStartMark( str.ref(), '[', ']', true);
	case KEY.Home:
		indent = indentText( editor.sp('lineStart').spText() );
		pos=editor.pos();
		if( pos.eq(indent.size()) ) return false;
		editor.pos('sp', indent.size(), true);
		return true;
	default: editor[prevKey] = key;
	}
	return false;
}
EditorSrcChange.editorTextChange() {
	key = editor[prevKey];
	not( key ) return;
	if( key>256 ) return;
	prev = editor.sp().spText('prevWord', 'prevChar');
	ch = prev.ch();
	switch( ch ) {
	case '#':
		val = prev.value(1);
		if( val.eq('pr') ) {
			sp = editor.pos('ep'), ep = editor.pos();
			editor.move(sp).move(ep,true);
			this.insertIndent('print("^|");' );
		} else if( val.eq('func') ) {
			sp = editor.pos('ep'), ep = editor.pos();
			editor.move(sp).move(ep,true);
			this.insertIndent(".func( template(func) {\n\t^|\n});");
		} else if( val.eq('test') ) {
			sp = editor.pos('ep'), ep = editor.pos();
			editor.move(sp).move(ep,true);
			this.insertIndent("test() {\n\t^|\n}\n#> test();\n\n");
		} else if( val.eq('page') ) {
			sp = editor.pos('ep'), ep = editor.pos();
			editor.move(sp).move(ep,true);
			this.insertIndent("page() {\n\t&page=this.widget({\n\t\tlayout:<page>\n\t\t\t\n\t\t</page>\n\t\tonInit() {\n\t\t}\n\t});\n\treturn page;\n}\n#>\n\n");
			ep=sp+4;
			editor.move(sp).move(ep,true);
		} else if( val.eq('fc') ) {
			sp = editor.pos('ep'), ep = editor.pos();
			editor.move(sp).move(ep,true);
			this.insertIndent("^|() {\n\t^|\n}");
		}
	case '<':
		tag = prev.value(1);
		if( tag.eq('page','layout','row','group','splitter','hbox','vbox') ) {
			this.insertIndent(">\n\t\n</$tag>", null, 'lineEnd');
		} else if( tag.eq('button') ) {
			editor.insert(' id="" text="">', true);
		} else if( tag.eq('tree','grid','editor','combo','radio','div','check','date','time','webview', 'canvas', 'input', 'tab') ) {
			editor.insert(' id="">', true);
		} else if( tag.eq('label') ) {
			editor.insert(' text="">', true);
		} else if( tag.eq('spa') ) {
			editor.insert("ce>");
		}
	default: break;
	}
}

EditorSrcClick.EditorSrcClick() {

}
EditorSrcClick.srcMouseClick(pos) {
	cf.inject( currentNode);
	print("@@ srcMouseClick : srcType=$currentNode[srcType] @@");
	_funcCheck=func(&var, &func) {
		if( func.ch().eq('.','=') ) func.incr();
		funcNm=func.move().trim();
		not( func.ch().eq('(') ) {
			return;
		}
		param = func.match();
		print("_funcCheck var=$var, funcNm=$funcNm, param=$param");
		not( var ) {
			this.userFunctionCheck(funcNm, param);
			return;
		}
		str=var;
		v=str.move();
		if( v.eq('this') ) {
			this.thisFunctionCheck(str, funcNm, param);
		} else if( v.eq('class') ) {
			this.classFunctionCheck(str, funcNm, param);
		} else {
			this.objectFunctionCheck(var, funcNm, param);
		}
	};
	cp=editor.cursorPt(pos);
	func=editor.sp(cp,'prevWord','prevChar').spText(80);
	ch=func.ch();
	print("@@ $func @@");
	if( ch.eq('.') ) {
		sp=editor.sp(cp,'prevWord','prevChar').pos('sp');
		if( sp>32 ) {
			ep=32, sp-=32;
		} else {
			ep=sp, sp=0;
		}
		str=editor.sp(sp).spText(ep);
		ch=str.ch(-1);
		if( ch.eq(')') ) {
			line=str.findLast('(');
			param=line.right();
			var=line.findLast(" \t\n=(,", FIND.chars).right();
			_funcCheck("${var}($param", func.ref() );
		} else if( ch.eq(']') ) {
			line=str.findLast('[');
			param=line.right();
			var=line.findLast(" \t\n=(,", FIND.chars).right();
			_funcCheck("${var}[$param", func.ref() );
		} else {
			var=str.findLast(" \t\n=(,", FIND.chars).right();
			not( var ) {
				var=str;
			}
			print("var====[$var | $str]");
			_funcCheck(var, func.ref() );
		}
	} else {
		_funcCheck(null, func.ref() );
	}
}
EditorSrcClick.nodeFunctionCheck(curNode, funcName, param, flag) {
	if( flag ) {
		ok=false;
	} else {
		a=typeof(curNode,'node'), b=curNode[$funcName];
		if( a && b ) ok=true;
	}
	if(  ok ) {
		editorMemberFunctionClick=class('page').getParentFunction(page,'editorMemberFunctionClick');
		editorMemberFunctionClick(thisNode, funcNm, param);
	} else {
		node=class('util').node().val( funcNm: funcName );
		instance('help.model').fetchAll("select object_cd, func_nm, func_param, func_src from core_object_func where object_cate<>'func' and func_nm=#{funcNm}", node);
		src='';
		while( cur, node ) {
		}
		editorCoreFunctionClick=class('page').getParentFunction(page,'editorCoreFunctionClick');
		editorCoreFunctionClick(funcNm, param, src);
	}
}
EditorSrcClick.thisFunctionCheck(&var, funcNm, param) {
	cf.inject( currentNode);
	if( currentNode.currentClass ) {
		classNode=currentNode.currentClass;
		p=get('Kiosk.PreviewClassFunction');
		not( p ) {
			p=pageLoad('Kiosk.PreviewClassFunction');
			p.size(850, 650);
		}
		p.open();
		print(p, classNode, funcNm);
		p.classFuncChange(classNode, funcNm);
		return;
	}
	if( currentNode.runtimePage ) {
		thisNode=currentNode.runtimePage;
	} else {
		return this.nodeFunctionCheck(null, funcNm, param, true);
	}
	print( thisNode, funcNm, param);
	curNode=null;
	ch=var.ch();
	if( ch.eq('[','.') ) {
		if( ch.eq('[') ) {
			v=var.match().trim();
		} else {
			var.incr();
			v=var.trim();
		}
		curNode=thisNode.get(v);
	} else {
		curNode=thisNode;
	}
	this.nodeFunctionCheck(curNode, funcNm, param, true);
}
EditorSrcClick.objectFunctionCheck(&var, funcNm, param) {
	cf.inject( currentNode);
	if( var.find('(') ) {
		return this.nodeFunctionCheck(null, funcNm, param, true);
	}
	if( currentNode.runtimePage ) {
		thisNode=currentNode.runtimePage;
	} else if( currentNode.currentClass ) {
		thisNode=currentNode.currentClass;
	} else {
		return this.nodeFunctionCheck(null, funcNm, param, true);
	}
	if( var.find('[') ) {
		v=var.findPos('[').trim();
		v.add('.', var.findPos(']').trim() );
		this.nodeFunctionCheck(thisNode.get(v), funcNm, param);
	} else {
		this.nodeFunctionCheck(thisNode.get(var), funcNm, param);
	}
}
EditorSrcClick.userFunctionCheck(fnm, &param) {
	cf.inject( currentNode);
	print("@@@ userFunctionCheck $fnm, $param @@@");
	if( fnm.eq('conf','tr') ) {
		p=pageLoad('Common.ConfManager');
		p.open();
		if( param.ch().eq() ) {
			confCode=param.match().trim();
			print("ConfManager open code=$confCode");
			p.initPage(confCode);
		}
	} else if( help.count("select count(1) from core_object_func where object_cate='func' and func_nm='$fnm'") ) {
		page.alert("���� �Լ� ������ ���� ���� �����Դϴ�");
	} else {
		p=pageLoad('Common.PreviewFunction');
		p.open();
		p.setCommFunc(fnm);
	}
}

PreviewFunctionImpl.PreviewFunctionImpl(page, pageCd) {
	this.addPage(common.Page);
	db =instance('pages.model');
	if( page.funcGroup ) {
		funcGroup=page.funcGroup;
		funcName=page.funcName;
		funcGroup.eventMap(onFocusIn, this.funcGroupFocus);
		funcGroup.eventMap(onChange, this.funcGroupChange);
		funcGroup.eventMap(onDraw, this.funcGroupDraw, 'draw, index, state' );
		funcGroup.check('editable', true);
		funcGroup.delegate(true, 24);
		funcName.eventMap(onFocusIn, this.funcNameFocus);
		funcName.eventMap(onChange, this.funcNameChange);
		funcName.eventMap(onDraw, this.funcNameDraw, 'draw, index, state' );
		funcName.check('editable', true);
		funcName.delegate(true, 24);
	}
	if( page.funcHist ) {
		funcHist=page.funcHist;
		funcHist.eventMap(onChange, this.funcHistChange);
		if( page.closeAll ) page.closeAll.eventMap(onClick, this.funcHistCloseAll);
		if( page.closeCurrent ) page.closeCurrent.eventMap(onClick, this.funcHistCloseCurrent);
	}
	if( page.content ) {
		mSrcContent=page.content;
		class('widget').comboValue(funcHist, tr("공통함수: [#]", "function.new") );
		pageEditor=pageLoad("Common.FuncEditPage",true);
		pageEditor[funcCode]="function.new";
		mSrcContent.addPage(pageEditor,true);
	}
	funcChangeTick=System.tick() - 1000;
	this.initForm();
}
PreviewFunctionImpl.initForm() {
	if( funcGroup ) {
		root=instance('funcInfo.model').rootNode();
		sub=root.child(0);
		not( sub ) sub=root.addNode({ type:root, title: 공통함수 정보});
		not( sub.childCount() ) {
			db.fetchAll(conf('sql.funcGroup'), sub );
		}
		this.funcGroupMaxStr(sub);
		funcGroup.removeAll().addItem(sub, 'func_grp', '=함수그룹=');
		funcGroup.value('function');
	}
	if( funcHist ) {
		funcHist.removeAll();
	}
}
PreviewFunctionImpl.initPage(node) {
	print("@@ previewFunction initPage : pageCd:$pageCd, node:$node @@");
	switch( pageCd ) {
	case classImpl: this.classFuncChange(node);
	case userFunc: this.funcChange(null, node[funcName]);
	}
}
PreviewFunctionImpl.funcGroupFocus() {
	page.delay( callback() {
		funcGroup.selectText(true);
	}, true);
}
PreviewFunctionImpl.funcNameFocus() {
	page.delay( callback() {
		funcName.selectText(true);
	},true);
}
PreviewFunctionImpl.funcGroupChange() {
	val=funcGroup.value();
	root=funcGroup.rootNode(), sub=null;
	not( val ) {
		val=funcGroup.text();
		if( val ) {
			sub=root.addNode().val(type:'funcGroup', func_grp: val, status:'new' );
			this.funcGroupMaxStr(root);
		}
	}
	this.funcGroupVal=val;
	not( val ) {
		node=_node(root,'emptyNode').removeAll();
		funcName.removeAll().addItem(node, 'func_nm', '=함수명=');
		return;
	}
	not( sub ) {
		sub=root.findOne('func_grp', val);
	}
	not( sub.childCount() ) {
		db.fetchAll(conf('sql.funcGroupChild'), sub );
	}
	this.funcNameMaxStr(sub);
	funcName.removeAll().addItem(sub, 'func_nm', '=함수명=');
	funcName.focus();
	funcName.selectText();
}
PreviewFunctionImpl.funcGroupDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(funcGroup, draw, index, state);
	not( node ) return;
	rc=draw.rect();
	rcIcon=rc.width(24).center(16,16);
	rc.incrX(24);
	if( node.status.eq('new') ) {
		draw.font(8,'blod','#f090a0').text(rc.width(16), '*', 'center');
		rc.incrX(16);
	}
	funcCnt="(${node[cnt]})";
	draw.icon(rcIcon, "vicon.table_gear" );
	draw.font(10).text(rc, node[func_grp]);
	draw.font(8).text(rc.move('end',30), funcCnt );
}
PreviewFunctionImpl.funcNameChange() {
	val=funcName.value(), root=funcName.rootNode();
	not( val ) {
		val=funcName.text();
		if( val ) {
			sub=root.addNode().val(type: 'funcSrc', func_nm: val, status:'new');
			this.funcNameMaxStr(root);
		}
	}
	this.funcNameVal=val;
	not( val ) return;
	not( sub ) {
		sub=root.findOne('func_nm',val);
	}
	this.funcChange(this.funcGroupVal, val, sub);
}
PreviewFunctionImpl.funcNameDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(funcName, draw, index, state);
	not( node ) return;
	rc=draw.rect();
	rcIcon=rc.width(24).center(16,16);
	rc.incrX(24);
	if( node.status.eq('new') ) {
		draw.font(8,'blod','#f090a0').text(rc.width(16), '*', 'center');
		rc.incrX(16);
	}
	switch( node[func_type] ) {
	case 'A':	icon="ficon.script-code";
	case 'C':	icon="ficon.script-globe";
	case 'S':	icon="ficon.script-block";
	case 'T':	icon="ficon.script-attribute-t";
	case 'Z':	icon="ficon.script--exclamation";
	default:	icon="ficon.script-code";
	}
	draw.icon( rcIcon, icon);
	draw.font(10).text( rc,  node.func_nm);
	param=when( node[func_param], "(${node[func_param]})");
	if( param ) {
		w=draw.textWidth(param)+10;
		draw.font(8).text( rc.move('end',w), param, 'right');
	}
}
PreviewFunctionImpl.funcHistChange() {
	funcHist.value().split(':').inject(kind, code);
	if( kind.eq('공통함수') ) {
		code.split('.').inject(group, name);
		this.funcChange(group, name);
	} else if( kind.eq('클래스함수') ) {
		while( page, mSrcContent.widget() ) {
			not( page[classFuncCode].eq(code) ) continue;
			mSrcContent.current(page);
			break;
		}
	}
}
PreviewFunctionImpl.funcHistCloseAll() {
	// arr=funcHist.@adds;
	while( page, mSrcContent.widget() ) {
		mSrcContent.removePage(page, true);
	}
	funcHist.removeAll();
}
PreviewFunctionImpl.funcHistCloseCurrent() {
	val=funcHist.value();
	val.split(':').inject(kind, code);
	while( page, mSrcContent.widget() ) {
		if( page.funcCode && page.funcCode.eq(code) ) {
			mSrcContent.removePage(page);
			break;
		} else if( page.classFuncCode && page.classFuncCode.eq(code) ) {
			mSrcContent.removePage(page, true);
			break;
		}
	}
	idx=funcHist.index();
	funcHist.remove(idx);
}
PreviewFunctionImpl.funcGroupMaxStr(node) {
	maxStr='';
	while( cur, node ) {
		text="${cur[func_grp]}\t\t(${cur[cnt]})";
		if( maxStr.size()<text.size() ) maxStr=text;
	}
	funcGroup.addText(maxStr, true);
}
PreviewFunctionImpl.funcNameMaxStr(node) {
	maxStr='';
	while( cur, node ) {
		text="${cur[func_nm]}\t\t($cur[func_param])";
		if( maxStr.size()<text.size() ) maxStr=text;
	}
	funcName.addText(maxStr, true);
}
PreviewFunctionImpl.funcChange(group, name, node) {
	dist=System.tick() - funcChangeTick;
	if( dist<500 ) {
		print("funcChange distance=$dist");
		return;
	}
	@funcChangeTick=System.tick();
	funcGroup.findLayout().showAll();
	if( group ) {
		val=funcGroup.value();
		not( val.eq(group) ) funcGroup.value(group);
	} else {
		this.setCommFunc(name);
		return;
	}
	funcCode="${group}.${name}";
	/* 함수 히스토리에 추가한다 */
	if( funcHist && node ) {
		class('widget').comboValue(funcHist, tr("공통함수: [#]",funcCode) );
	}
	print(funcCode, mSrcContent);
	not( mSrcContent ) {
		return page.funcChange(group, name);
	}
	/* 함수선택 및 새로운 함수 편집 페이지 추가 */
	pageEditor=this.findPageEditor(funcCode);
	if( pageEditor ) {
		val=funcName.value();
		not( val.eq(name) ) funcName.value(name);
		mSrcContent.addPage(pageEditor,true);
	} else {
		pageEditor=pageLoad("Common.FuncEditPage",true);
		pageEditor[funcCode]=funcCode;
		if( node ) {
			not( node[src] ) {
				not( node[func_grp] ) node[func_grp]=group;
				db.fetch("select funcParam as func_param, funcData as src, note, type as func_type, funcDesc as func_desc from cmsFunc where cmsCode=#{func_grp} and funcName=#{func_nm}", node);
			}
			pageEditor.initPage(node);
		}
		mSrcContent.addPage(pageEditor,true);
	}
}
PreviewFunctionImpl.classFuncChange(node) {
	if( funcGroup ) {
		funcGroup.findLayout().hideAll();
	}
	classFuncCode=tr("[#].[#].[#]", node[class_grp], node[class_nm], node[class_func] );
	if( funcHist && node ) {
		class('widget').comboValue(funcHist, tr("클래스함수: [#]",classFuncCode) );
	}
	not( mSrcContent ) {
		return page.classFuncChange(node);
	}
	ok=false;
	/* 함수선택 및 새로운 함수 편집 페이지 추가 */
	while( cur, mSrcContent.widget() ) {
		not( cur[classFuncCode] ) continue;
		if( cur[classFuncCode].eq(classFuncCode) ) {
			mSrcContent.addPage(cur,true);
			ok=true;
		}
	}
	not( ok ) {
		pageEditor=pageLoad("Common.ClassFuncEditPage",true);
		pageEditor[classFuncCode]=classFuncCode;
		pageEditor.initPage(node);
		mSrcContent.addPage(pageEditor,true);
	}
}
PreviewFunctionImpl.setStatus(msg) {
	page.funcStatus.value(msg);
}
PreviewFunctionImpl.findPageEditor(funcCode) {
	not( mSrcContent ) return;
	while( cur, mSrcContent.widget() ) {
		if( cur[funcCode].eq(funcCode) ) {
			return cur;
		}
	}
	return null;
}
PreviewFunctionImpl.setCommFunc(funcNm, alert) {
	group=db.value("select cmsCode from cmsFunc where funcName='$funcNm' order by tm desc");
	if( alert, not(group) ) {
		page.alert("$funcNm 함수를 찾을수 없습니다.");
	}
	funcGroup.value(group);
	this.funcNameVal=funcNm;
	page.delay(callback() {
		combo=this[funcName], funcNm=this[funcNameVal];
		key=combo[@key];
		root=combo.rootNode();
		cur=root.findOne(key,funcNm);
		if( cur ) {
			combo.current(funcNm);
		} else {
			@funcChangeTick=System.tick();
			cur=root.addNode();
			cur[status]="new";
			cur[$key]=funcNm;
			cur.state(NODE.add, true);
			combo.update();
			combo.value(funcNm);
			print("setCommFunc ## funcName=$funcNm, $key");
			pageEditor=this.findPageEditor("function.new");
			if( pageEditor ) {
				pageEditor.initPage(cur);
				mSrcContent.addPage(pageEditor,true);
			}
		}
	},this);
}

EditorSrcImpl.EditorSrcImpl(page, pageCd) {
	this.addClass(common.Page, dev.EditorSrc );
	switch(pageCd ) {
	case commFunc:
		page.func_type.eventMap(onChange, commFuncTypeChange);
		page.func_type.addItem( class('code').getCodeNode('funcSrcType'), 'code,value', '==선택==' );
	case classFunc:
		page.func_type.eventMap(onChange, classFuncTypeChange);
		page.func_type.addItem( class('code').getCodeNode('func_type'), 'code,value', '==선택==' );
	default:
		page.save.eventMap(onClick, saveSrc);
		page.run.eventMap(onClick, runSrc);
	}
	cf.put(pageCd);
}
EditorSrcImpl.initPage(node) {
	print("@@ EditorSrcImpl initPage : pageCd:$pageCd node:$node");
	node.srcType=pageCd;
	switch(pageCd ) {
	case commFunc:		this.setCommFunc(node);
	case classFunc:		this.setClassFunc(node);
	case pageFunc:		this.setPageFunc(node);
	case coreFunc:		this.setCoreFunc(node);
	case coreHelp:		this.setCoreHelp(node);
	case pageSource:	this.setPageSource(node);
	case classSource:	this.setClassSource(node);
	}
}
EditorSrcImpl.setCommFunc(node) {
	cf.currentNode=node;
	page.func_type.value( node[func_type] );
	this.setSrc( "${node[func_nm]}($node[func_param]) {\r\n$node[src]\r\n}" );
	this.func_desc.value( node[note] );
}
EditorSrcImpl.commFuncTypeChange() {
	page.save.enable();
}
EditorSrcImpl.setClassFunc(node) {
	cf.currentNode=node;
	page.func_type.value( node[func_type] );
	if( node[src] ) {
		this.setSrc("${node[class_func]}($node[class_param]) {\r\n$node[src]\r\n}");
	} else {
		this.setSrc("${node[class_func]}($node[class_param]) {\r\n\t^|\r\n}");
	}
	this.func_desc.value( node[note] );
}
EditorSrcImpl.classFuncTypeChange() {
	page.save.enable();
}
EditorSrcImpl.setPageFunc(node) {
	cf.currentNode=node;
	if( node.sort.eq(9) ) {
		page.editorStatus.value(">> 새로운 페이지 작성");
		return;
	}
	if( node[funcName].eq('layout') ) {
		this.setSrc(node[src]);
	} else {
		if( node[src] ) {
			this.setSrc( "${node[funcName]}($node[funcParam]) {\r\n$node[src]}" );
		} else {
			this.setSrc( "${node[funcName]}($node[funcParam]) {\r\n\t^|\r\n}" );
		}
	}
	this.func_desc.value( node[note] );
	page.editorStatus.value(">> ${node[cmsCode]}.${node[pageCode]}");
}
EditorSrcImpl.setCoreFunc(node) {
	cf.currentNode=node;
	this.setSrc(node[src]);
}
EditorSrcImpl.setCoreHelp(node) {
	cf.currentNode=node;
	this.setSrc(node[src]);
}
EditorSrcImpl.setPageSource(node) {
	cf.currentNode=node;
	this.setSrc(node[src]);
}
EditorSrcImpl.setClassSource(node) {
	cf.currentNode=node;
	if( node[classPath] ) {
		this.setSrc( fileRead(node[classPath]) );
	}
}
EditorSrcImpl.saveSrc() {
	node=cf.currentNode;
	switch(pageCd ) {
	case commFunc:	this.saveCommFunc(node);
	case classFunc:		this.setClassFunc(node);
	case pageFunc:		this.setPageFunc(node);
	case coreFunc:		this.setCoreFunc(node);
	case coreHelp:		this.setCoreHelp(node);
	case pageSource:	this.setPageSource(node);
	case classSource:	this.setClassSource(node);
	}
}
EditorSrcImpl.runSrc() {
	node=cf.currentNode;
	switch(pageCd ) {
	case commFunc:	this.runCommFunc(node);
	case classFunc:		this.setClassFunc(node);
	case pageFunc:		this.setPageFunc(node);
	case coreFunc:		this.setCoreFunc(node);
	case coreHelp:		this.setCoreHelp(node);
	case pageSource:	this.setPageSource(node);
	case classSource:	this.setClassSource(node);
	}
}
EditorSrcImpl.saveCommFunc(node) {
	not( page.confirm("$node[func_nm] 함수를 저장 하시겠습니까?") ) return;
	db=instance('pages.model');
	node.tm=System.localtime();
	src=this.src.value();
	type=page.func_type.value();
	node.note=page.func_desc.value();
	node.func_type=type;
	parse=func(&s) {
		rtn=0;
		sp=firstCommentSkip(s);
		s.pos(sp);
		fnm=s.move().trim();
		not( fnm.eq(node[func_nm]) ) {
			not( page.confirm("함수명이 다릅니다 새이름으로 저장할까요?\n함수명 $fnm (이전함수명 $node[func_nm])") ) {
				return 0;
			}
			rtn=2;
		}
		if( s.ch().eq('(') ) {
			param=s.match();
			if( s.ch().eq('{') ) {
				src=s.match(1);
				if( src.find('/*') || src.find('//') ) {
					node[funcSrc]=makeSrc(src);
				}
				node[src]=src;
				node[func_param]=param;
				rtn=1;
			}
		}
		return rtn;
	};
	rtn=parse(src.ref());
	if( rtn.eq(2) ) {
		db.exec( conf('sql#dev.funcInsert'), node)
	} else if( rtn ) {
		not( db.exec( conf('sql#dev.funcUpdate'), node) ) {
			db.exec( conf('sql#dev.funcInsert'), node);
		}
	}
	persist=when( type.eq('S'), true);
	Cf.func(src, persist);
	this.save.disable();
}
EditorSrcImpl.runCommFunc(node) {
	type=page.func_type.value();
	persist=when( type.eq('S'), true);
	Cf.func(page.src.value(), persist);
}

EditorSrc.EditorSrc(page) {
	this.addClass( dev.EditorSrcChange, dev.EditorSrcClick );
	not( cf ) cf={};
	mPopupMode = 'popup';
	mTreeNode=null;
	mSearchPrev='';
	mFuncSrc='';
	saveButton=null, closeButton=null, lockButton=null;
	searchInput=null, searchReplaceButton=null;
	if( page.src || page.editor ) {
		editor=when( page.src, page.src, page.editor);
	}
	not( editor ) return;
	help=Class.db('help');
	not( help.open() ) {
		help.open('data/help.db');
	}
	this.setEditorEvent();
}
EditorSrc.closeClick() {
	if( page.onClose ) {
		page.fireEvent("onClose");
	} else {
		page.hide();
	}
}
EditorSrc.editorChange() {
	if( saveButton ) {
		if( editor.isModify() ) {
			saveButton.enable();
		} else {
			saveButton.disable();
		}
	}
	this.editorTextChange(editor);
}
EditorSrc.editorMouseClick(pos, keys) {
	not(  keys&KEY.ctrl ) {
		return;
	}
	this.editorClickPos=pos;
	page.delay( callback() {
		pos=this.editorClickPos;
		this.srcMouseClick(pos );
	}, this);
}
EditorSrc.lockClick() {
	if( mPopupMode.eq('popup') ) {
		lock.icon("vicon.application_double");
		@mPopupMode='window';
		page.flags('window');
	} else {
		lock.icon("vicon.application_link");
		@mPopupMode='popup';
		page.flags('popup');
	}
	page.show();
}
EditorSrc.searchEnterKey() {
	val=searchInput.value();
	not( val ) return;
	if( val.eq(mSearchPrev) ) {
		editor.searchNext(val);
	} else {
		editor.findAll(val);
	}
}
EditorSrc.searchFocus() {
	searchInput.select();
}
EditorSrc.searchReplaceClick() {
	popup=pageLoad('dev.searchAndReplace');
	popup.open();
	popup.initPage(searchInput.value());
	return popup();
}
EditorSrc.setEditorEvent() {
	editor.syntax( conf('syntax.dev') );
	editor.eventMap( onMouseClick, this.editorMouseClick, 'pos, keys' );
	editor.eventMap( onChange, this.editorChange );
	editor.eventMap( onKeyDown, this.editorKeyDown, 'key,mode' );
	if( page.save || page.btnSave ) {
		@saveButton=nvl( page.save, page.btnSave );
		saveButton.disable();
	}
	/* 찾기 & 바꾸기 처리*/
	if( page.inputSearch ) {
		@searchInput=page.inputSearch;
		searchInput.eventMap(onFocus, this.searchFocus);
		searchInput.eventMap(onEnter, this.searchEnterKey);
	}
	if( page.btnSearchReplace ) {
		@searchReplaceButton=page.btnSearchReplace;
		searchReplaceButton.eventMap(onClick, this.searchReplaceClick);
	}
	/* 툴버튼 처리 */
	if( page[parentPage].get('tag').eq('div', 'tab') ) {
		if( page[parentPage].get('id').eq('missionPage') ) return;
		page[btnLock].findLayout().hideAll();
		page[cancel].hide();
	} else {
		if( page.btnLock ) {
			@lockButton=page.btnLock;
			lockButton.eventMap(onClick, this.lockClick);
		}
		if( page.btnClose ) {
			@closeButton=page.btnClose;
			closeButton.eventMap(onClick, this.closeClick);
		}
	}
}
EditorSrc.setSrc(src) {
	editor.clear();
	if( src.find("^|") ) {
		editor.insert(src, true).clearUndo();
		page.delay( callback() {
			editor.focus();
		},this);
	} else {
		editor.insert(src).clearUndo();
		editor.move(0);
	}
	if( saveButton ) saveButton.disable();
}

CreateXmlEditor.CreateXmlEditor(page) {
	this.addClass(common.Page, dev.EditorSrc );
}
CreateXmlEditor.initPage(node) {
	cf.currentNode=node;
	if( node[src] ) {
		this.setSrc( node[src] );
	} else {
		this.setSrc("<Page>\n\t^|\n</Page>", true);
	}
}

EditPageAttributeGrid.EditPageAttributeGrid(page) {
	this.addClass(common.Page );
	dataModel=Class.model('EditPageAttribute');
	currentTagNode=null;
	grid=page.grid;
	grid.model( dataModel, 'attribute, value, note');
	grid.check('headerHide', true);
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onFilter, this.gridFilter, 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	grid.eventMap(onMouseDown, this.gridMouseDown, 'pos, button');
}
EditPageAttributeGrid.gridWidthReset() {
	this.gridWidthApply=false, this.delayCall=false;
	this.attributeWidth=0, this.valueWidth=0;
	grid.update();
}
EditPageAttributeGrid.initGridData(tagNode) {
	@currentTagNode=null;
	grid.rootNode().initNode();
	if( tagNode ) {
		this.makeTagAttribute(tagNode);
	}
	grid.update();
}
EditPageAttributeGrid.makeTagAttribute(tagNode) {
	if( currentTagNode==tagNode ) {
		return;
	}
	@currentTagNode=tagNode;
	root=grid.rootNode();
	while( sub, root ) {
		not( sub.refNode ) continue;
		if( sub.refNode==tagNode ) {
			return;
		}
	}
	tagNode.visible=true;
	this.makeGridData(tagNode);
}
EditPageAttributeGrid.makeGridData(srcNode, attrRoot) {
	not( typeof(srcNode,'node') ) {
		return;
	}
	root=grid.rootNode();
	_makeAttribute=func(src, depth) {
		arr=src.keys().sort();
		attrNode=[], attrData=[];
		while( attr, arr ) {
			if( attr.ch().eq('@') ) continue;
			val=src[$attr];
			if( attr.eq('id','kind','rect','type','tag','visible','Width','Height') ) {
				continue;
			}
			if( typeof(val,'node') ) {
				attrNode.addPair(attr,val);
			} else {
				attrData.addPair(attr,val);
			}
		}
		_addNode=func(attr, val) {
			node=root.addNode();
			node[attribute]=attr;
			node[srcNode]=src;
			node[depth]=depth+1;
			node[value]=val;
			if( typeof(val,'node') ) {
				node[tag]=nvl(val[tag], 'node');
				node[refNode]=val;
			}
		};
		while( pair, attrNode ) {
			pair.inject(attr, val);
			_addNode(attr, val);
			_makeAttribute(val, depth+1);
		}
		if( src[id] ) 		_addNode('id', src[id]);
		if( src[kind] ) 	_addNode('kind', src[kind]);
		if( src[type] ) 	_addNode('type', src[type]);
		if( src[rect] ) 	_addNode('rect', src[rect]);
		if( src[Width] ) 	_addNode('Width', src[Width]);
		if( src[Height] )	_addNode('Height', src[Height]);
		while( pair, attrData ) {
			pair.inject(attr, val);
			_addNode(attr, val);
		}
		attrNode.delete(), attrData.delete();
	};
	tag=srcNode[tag];
	if( tag ) {
		node=root.addNode();
		node[tag]=tag;
		node[attribute]=tag;
	} else {
		node=root.addNode({tag:root, attribute: $attrRoot});
	}
	node[depth]=0;
	node[refNode]=srcNode;
	_makeAttribute(srcNode, 0);
	this.gridWidthReset();
}
EditPageAttributeGrid.gridMouseDown(pos) {
	node = grid.at(pos);
	not( node[ButtonArray]  ) return;
	not( node.refNode ) return;
	ref=node.refNode;
	idx=node.index();
	node[ButtonArray].inject(margin,detail);
	if( margin.contains(pos) ) {
		root=node.parent();
		if( ref[Margin] ) {
			while(n,16) {
				idx++;
				cur=root.child(idx);
				if( cur[attribute].eq('Margin') ) {
					grid.edit(cur,1);
				}
			}
		} else {
			next=idx+1;
			cur=root.insertNode(next,{attribute:Margin, value:'0,0,0,0', depth:0});
			cur[srcNode]=node[srcNode];
			grid.update();
			grid.edit(cur,1);
		}
		return 'ignore';
	} else if( detail.contains(pos) ) {
		page.alert("상세 노드정보는 구현중입니다");
		return 'ignore';
	}

}
EditPageAttributeGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	ref=node.refNode;
	field=grid.field(draw.index());
	if( over ) {
		not( node.tag.eq('root')  || ref  ) {
			draw.fill(rc,'#d0d0d0');
		}
	}
	if( field.eq('attribute') ) {
		rcGrid=grid.rect();
		if( node.tag.eq('root')  || ref  ) {
			rc.width( rcGrid.width()-6 );
			rcIcon=rc.width(20).center(16,16);
			if( node.tag.eq('root') ) {
				draw.fill(rc,'#b0a09a');
				draw.icon(rcIcon, "ficon.application-plus-black");
			} else {
				if( ref==currentTagNode ) {
					draw.fill(rc,'#b0a09a');
					draw.icon(rcIcon, "ficon.application-plus-black");
				} else {
					draw.fill(rc,'#dacbd0');
					draw.icon(rcIcon, "ficon.application-plus");
				}
			}
			draw.rectLine(rc,4,'#38241F');
			draw.rectLine(rc.incrY(1,true),4,'#E9D5AF');
			rc.incrX(20);
			rcText=rc.incrX(node[depth*20]);
			draw.save().font('bold');
			draw.text(rcText, node[attribute]);
			draw.restore();
		} else {
			rcText=rc.incrX(node[depth*20]);
			draw.rectLine(rc,34,'#b0a09a');
			draw.text(rcText.incrX(4), node[attribute]);
			not( this.gridWidthApply ) {
				x=rc.x();
				x+=draw.textWidth(node[attribute]) + 25;
				if( this.attributeWidth < x ) this.attributeWidth=x;
				if( this.delayCall ) return;
				page.delay(callback() {
					attributeWidth=max(this.attributeWidth, 100);
					valWidth=max(150, this.valueWidth);
					if( valWidth>500 ) {
						valWidth=500;
					}
					w=attributeWidth + valWidth;
					rcGrid=grid.rect();
					noteWidth=rcGrid.width() - w - 6;
					grid.headerWidth(0, attributeWidth);
					grid.headerWidth(1, valWidth );
					grid.headerWidth(2, noteWidth );
					grid.update();
					this.gridWidthApply=true;
				}, this, 500);
				this.delayCall=true;
			}
		}
	} else if( field.eq('value') ) {
		if( node.tag.eq('root')  || ref  ) return;
		draw.rectLine(rc,3,'#804000', 1, 'dash');
		draw.rectLine(rc,4,'#b0a09a');
		attr=node[attribute];
		src=node[srcNode];
		val=src[$attr];
		draw.text(rc.incrX(4), val);
		not( this.gridWidthApply ) {
			x=draw.textWidth(val) + 60;
			if( this.valueWidth < x ) this.valueWidth=x;
		}
	} else if( field.eq('note') ) {
		if( ref  ) {
			node[rect]=rc.move('end',120);
			getRectArray(node, '4,5', 2, 'ButtonArray', 'hbox').inject(a,b);
			draw.ctrl('btn', a, 'Margin');
			draw.ctrl('btn', b, '상세정보');
		} else {
			draw.rectLine(rc,4,'#b0a09a');
			if( node.type.eq('prop') ) {
			}
		}
	}
}
EditPageAttributeGrid.gridFilter(node) {
	if( node[tag].eq('root') || node[depth].eq(0) ) return true;
	if( node.refNode ) {
		return node.refNode.visible;
	} else if( node.srcNode ) {
		return node.srcNode.visible;
	}
	return true;
}
EditPageAttributeGrid.gridClick(node, column) {
	if( node.tag.eq('root') ) {
		return true;
	}
	field=grid.field(column);
	ref= node.refNode;
	if( ref ) {
		ok=true;
	}
	if( ok ) {
		ref.toggle('visible');
		grid.update();
	} else if( field.eq('value', 'note') ) {
		if( field.eq('value') ) {
			if( ref.tag.eq('check') ) {
				node.toggle('value');
				ref="$node[widget]#$prop[code]";
				currentNode[$ref]=node[value];
				grid.update();
			} else {
				grid.edit(node,1);
			}
		} else {
			grid.edit(node,2);
		}
	}
}
EditPageAttributeGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	tag=node.tag;
	setRect=func(&s) {
		in=s.match();
		in.split(',').inject(x,y,w,h);
		src[$attr]=Class.rect(x,y,w,h);
	};
	gridInput=func(field) {
		input = page.widget(conf('widget.gridInput') );
		input.initWidget(page, field);
		src=node[srcNode];
		attr=node[attribute];
		obj=src[$attr];
		node[value]="$obj";
		return input;
	};
	switch( type ) {
	case create:
		if( field.eq('value') ) return gridInput();
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		src=node[srcNode];
		attr=node[attribute];
		obj=src[$attr];
		val="$obj";
		not( val.eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
			}
			if( field.eq('value') ) {
				bconf=false;
				if( typeof(obj,"node") ) {
					src[$attr]=s.split('parse');
				} else if( typeof( obj,"array") ) {
					src[$attr]=s.split('parse');
				} else if( typeof( obj,"rect") ) {
					setRect(data);
					bconf=true;
				} else {
					src[$attr]=data;
					if( attr.eq('Margin') ) bconf=true;
				}
				p=get('Kiosk.EditPageCanvas');
				if( p ) {
					ctrl=p.x;
					if( bconf ) ctrl.conf();
					ctrl.update();
				}
				this.gridWidthReset();
			} else {
				node[$field]=data;
			}
		}
		grid.update();
	default: break;
	}
}

FuncSearchGrid.FuncSearchGrid(page) {
	this.addClass(common.Config );
	db=Class.db('pages');
	dataModel=Class.model('FuncSearch');
	grid=page.grid;
	grid.model(
		dataModel,
		gridMakeField('kind:유형#80px, groupCode:그룹#100px, code: 코드#100px, func: 함수명#120, param:매개변수#50, note: 비고#100' );
	);
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
	grid.check('sortEnable', true);
	/* 그리드 이벤트 맵핑  */
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onChange, this.gridChange, 'node');
	grid.eventMap(onFetchMore, this.gridFetchMore, 'node');
	/* 폼정보  */
	if( page.funcName ) {
		funcNameInput= page.funcName;
	}
	if( page.funcData ) {
		funcDataInput= page.funcData;
	}
	searchSql=null;
}
FuncSearchGrid.search(funcKind, groupCode, code) {
	root=grid.rootNode().removeAll();
	func =funcNameInput.value();
	funcData=funcDataInput.value();
	root[offset]			= 0;
	root[fetchNum]	= 50;
	root.put(groupCode, code, func, funcData);
	filterClass="#[funcData ? and ( class_data like '%'||#{funcData}||'%' or class_src like '%'||#{funcData}||'%') ]";
	sql_a="select 'a' as kind, cmsCode groupCode, '' code, funcName func, funcParam param, note from cmsFunc where 1=1 #[funcData ? and funcData like '%'||#{funcData}||'%']";
	sql_b="select 'b' as kind, cmsCode groupCode, pageCode code, funcName func, funcParam param, note from pageFunc where 1=1 #[funcData ? and funcData like '%'||#{funcData}||'%']";
	sql_c="select 'c' as kind, class_grp groupCode, class_nm code, class_func func, class_param param, note from class_info where 1=1  $filterClass";
	switch(funcKind) {
	case a: 	sql=sql_a;
	case b:	sql=sql_b;
	case c:	sql=sql_c;
	default:	sql="$sql_a\nunion\n$sql_b\nunion\n$sql_c";
	}
	filter="1=1
		#[groupCode ? and groupCode=#{groupCode}]
		#[code ? and code=#{code}]
		#[func ? and func like '%'||#{func}||'%'] ";
	@searchSql="select kind, groupCode, code, func, param, note from ($sql) where $filter limit  #{offset}, 50";
	db.fetchAll(searchSql, root );
	total=db.count("select count(1) from ($sql) where $filter", root);
	page.gridStatus.value("전체 $total 건");
	grid.update();
	page.deletePage.hide();
	gridHeaderWidth(grid);
}
FuncSearchGrid.gridFetchMore(node) {
	node[offset]=node[fetchNum];
	db.fetchAll(searchSql, node);
	node[fetchNum+=50];
	grid.update();
}
FuncSearchGrid.gridChange(node) {
	switch( node[kind] ) {
	case a: sql="select funcData as src from cmsFunc where cmsCode=#{groupCode} and funcName=#{func}";
	case b: sql="select funcData as src from pageFunc where cmsCode=#{groupCode} and pageCode=#{code} and funcName=#{func}";
	case c: sql="select case when length(class_src)>0 then class_src else class_data end as src from class_info where class_grp=#{groupCode} and class_nm=#{code} and class_func=#{func}";
	}
	db.fetch(sql, node);
	src=makeSourceIndentText(node[src].ref(), "\t");
	body="${node[func]}($node[param]) {$src}";
	if( page.appendSrc.checked() ) {
		page.src.append(body);
	} else {
		page.setSrc(body);
	}
	funcData=funcDataInput.value();
	if( funcData ) {
		page.src.findAll(funcData);
	}
}
FuncSearchGrid.gridResize() {
	gridHeaderWidth(grid);
}
FuncSearchGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case kind:	draw.text(rc, getCommCodeValue('funcKind', node[kind]), 'center');
	default:		draw.text(rc, node[$field].trim());
	}
	draw.rectLine(rc,4,'#d0d0d0');
}

DebugEditor.DebugEditor(page) {
	this.addClass(common.Config, dev.EditorSrc );
}
DebugEditor.initPage(node) {
	cf.currentNode=node;
	if( node[src] ) {
		this.setSrc( node[src] );
	} else {
		this.setSrc("<Page>\n\t^|\n</Page>", true);
	}
}

PageFuncsManager.PageFuncsManager(page) {
	this.addClass(common.Config, dev.EditorSrc );
	db=Class.db('pages');
	currentPage=null;
}
PageFuncsManager.initPage(root, append) {
	not( root ) return;
	this[currentNode]=root;
	if( root.currentPage ) {
		@currentPage=root.currentPage;
	}
	src='';
	while( cur, root ) {
		not( cur[checked] ) continue;
		cur[cmsCode]	=root[cmsCode];
		cur[pageCode]	=root[pageCode];
		db.fetch("select case when length(ifnull(funcSrc,''))==0 then funcData else funcSrc end as src, funcParam, note from pageFunc where cmsCode=#{cmsCode} and pageCode=#{pageCode} and funcName=#{funcName}", cur);
		note=cur[note] .trim();
		if( note ) {
			src.add("/* $note */\r\n");
		}
		key=cur[funcName];
		body=makeSourceIndentText(cur[src].ref(), "\t");
		src.add("${cur[funcName]}($cur[funcParam]) {$body}\r\n\r\n");
	}
	if( append ) {
		editor.append("\r\n$src", true);
	} else {
		this.setSrc(src);
	}
	return currentPage;
}
PageFuncsManager.saveSrc() {
	root=this[currentNode];
	root[tm]=System.localtime();
	me=this;
	_save=func(&s) {
		err='',note='';
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
			num=db.exec("update pageFunc set funcSrc=#{funcSrc}, funcData=#{funcData}, funcParam=#{funcParam}, note=#{note}, tm=#{tm} where cmsCode=#{cmsCode} and pageCode=#{pageCode} and funcName=#{funcName}", root);
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
				db.exec("insert into pageFunc( cmsCode, pageCode, funcName, funcParam, funcSrc, funcData, note, tm, sort ) values (#{cmsCode}, #{pageCode}, #{funcName}, #{funcParam}, #{funcSrc}, #{funcData}, #{note}, #{tm}, #{sort} )", root);
			}
			note='';
		}
		me[error]=err;
	};
	parse=func(&s) {
		if( s.find("#>", 1) ) {
			_save( s.findPos("#>", 1) );
		} else {
			_save(s);
		}
	};
	parse( page[src].value() );
	if( page[autoRunCheck].checked() ) {
		this.runSrc(true);
	}
	page.save.disable();
}
PageFuncsManager.runSrc(flag) {
	me=this;
	_run=func(&s) {
		err='';
		while( s.valid() ) {
			c=s.ch();
			not( c ) break;
			if( c.eq('/') ) {
				if( s.ch(1).eq('/') )
					s.findPos("\n");
				 else if( s.ch(1).eq('*') )
					s.match();
				continue;
			}
			sp=s.cur(), s.move();
			c=s.ch();
			if( c.eq('.') ) {
				s.incr(), s.move();
				c=s.ch();
			}
			ep=s.cur();
			func=s.value(sp,ep,true);
			not( c.eq('(') ) {
				err.add("함수 시작오류 : 함수명 : $func\n");
				break;
			}
			param=s.match().trim();
			c=s.ch();
			not( c.eq('{') ) {
				err.add("함수 매개변수 오류: $param");
				break;
			}
			body=s.match(1);
			currentPage.func("${func}($param) { $body }");
		}
		me[error]=err;
	};
	parse=func(&s) {
		if( s.find("#>") ) {
			src=s.findPos("#>");
			_run( src );
			cmd=s.findPos("\n").trim();
			Cf.call("currentPage.${cmd}");
		} else {
			_run(s);
		}
	};
	parse( page[src].value() );
	this.update();
}
PageFuncsManager.update() {

}

Config.Config(page) {
	cf={};
	xmlNode={};
}
Config.loadXml(fileName, root) {
	cf.inject(imagePath);
	not( root ) root=xmlNode;
	xml=fmt( fileRead(fileName) );
	this.parseXml(&xml, root.removeAll() );
}
Config.parseProp(node, tag, &prop) {
	node[tag]=tag;
	while( prop.valid() ) {
		k=prop.findPos('=').trim();
		not( k ) break;
		ch=prop.ch();
		if( ch.eq() ) {
			node[$k]=prop.match();
		} else if( ch.eq('[') ) {
			in=prop.match();
			arr=[];
			while( in.valid() ) {
				arr.add( in.findPos(',').trim() );
			}
			node[$k]=arr;
		} else {
			node[$k]=prop.findPos(" \t\n",4).trim();
		}
	}
}
Config.parseXml(&data, node, fiistNode) {
	not( node ) {
		node=xmlNode;
		node.removeAll();
	}
	while( data.valid() ) {
		ch=data.ch();
		not( ch.eq('<') ) {
			break;
		}
		if( data.ch(1).eq('!') ) {
			data.match('<!--','-->');
			continue;
		}
		if( data.ch(1).eq('?') ) {
			data.match('<?','?>');
			continue;
		}
		sp=data.cur();
		tag=data.incr().move();
		sub = node.addNode();
		not( fiistNode ) {
			fiistNode=sub;
		}
		if( data.ch().eq('-') ) {
			sub[kind]=data.incr().move();
			print("tag--->$tag, $kind");
		}
		if( tag.eq('br', 'space', 'image') ) {
			prop=data.findPos(">");
			this.parseProp( sub, tag, prop);
		} else {
			in=data.find('>');
			if( in.ch(-1).eq('/') ) {
				prop=data.findPos('/>');
				this.parseProp( sub, tag, prop);
			} else {
				data.pos(sp);
				if( sub[kind] ) {
					in=data.match("<$tag-$sub[kind]","</$tag-$sub[kind]>");
				} else {
					in=data.match("<$tag","</$tag>",8);
				}
				not( in ) {
					print("@@ xml parse $tag not match");
					in=data.findPos("</$tag>");
				}
				prop=in.findPos(">");
				this.parseProp( sub, tag, prop);
				if( tag.eq('html', 'text') ) {
					val=in.trim();
					if( val ) sub[data]=val;
				} else {
					if( in.ch().eq('<') ) {
						this.parseXml(in, sub, fiistNode);
					} else {
						val=in.trim();
						if( val ) sub[data]=val;
					}
				}
			}
		}
	}
	return fiistNode;
}

EditorSrc.EditorSrc(page) {
	this.addClass( dev.EditorSrcChange, dev.EditorSrcClick );
	not( cf ) cf={};
	mPopupMode = 'popup';
	mTreeNode=null;
	mSearchPrev='';
	mFuncSrc='';
	saveButton=null, closeButton=null, lockButton=null;
	searchInput=null, searchReplaceButton=null;
	if( page.src || page.editor ) {
		editor=when( page.src, page.src, page.editor);
	}
	not( editor ) return;
	help=Class.db('help');
	not( help.open() ) {
		help.open('data/help.db');
	}
	this.setEditorEvent();
}
EditorSrc.closeClick() {
	if( page.onClose ) {
		page.fireEvent("onClose");
	} else {
		page.hide();
	}
}
EditorSrc.editorChange() {
	if( saveButton ) {
		if( editor.isModify() ) {
			saveButton.enable();
		} else {
			saveButton.disable();
		}
	}
	this.editorTextChange(editor);
}
EditorSrc.editorMouseClick(pos, keys) {
	not(  keys&KEY.ctrl ) {
		return;
	}
	this.editorClickPos=pos;
	page.delay( callback() {
		pos=this.editorClickPos;
		this.srcMouseClick(pos );
	}, this);
}
EditorSrc.lockClick() {
	if( mPopupMode.eq('popup') ) {
		lock.icon("vicon.application_double");
		@mPopupMode='window';
		page.flags('window');
	} else {
		lock.icon("vicon.application_link");
		@mPopupMode='popup';
		page.flags('popup');
	}
	page.show();
}
EditorSrc.searchEnterKey() {
	val=searchInput.value();
	not( val ) return;
	if( val.eq(mSearchPrev) ) {
		editor.searchNext(val);
	} else {
		editor.findAll(val);
	}
}
EditorSrc.searchFocus() {
	searchInput.select();
}
EditorSrc.searchReplaceClick() {
	popup=pageLoad('dev.searchAndReplace');
	popup.open();
	popup.initPage(searchInput.value());
	return popup();
}
EditorSrc.setEditorEvent() {
	editor.syntax( conf('syntax.dev') );
	editor.eventMap( onMouseClick, this.editorMouseClick, 'pos, keys' );
	editor.eventMap( onChange, this.editorChange );
	editor.eventMap( onKeyDown, this.editorKeyDown, 'key,mode' );
	if( page.save || page.btnSave ) {
		@saveButton=nvl( page.save, page.btnSave );
		saveButton.disable();
	}
	/* 찾기 & 바꾸기 처리*/
	if( page.inputSearch ) {
		@searchInput=page.inputSearch;
		searchInput.eventMap(onFocus, this.searchFocus);
		searchInput.eventMap(onEnter, this.searchEnterKey);
	}
	if( page.btnSearchReplace ) {
		@searchReplaceButton=page.btnSearchReplace;
		searchReplaceButton.eventMap(onClick, this.searchReplaceClick);
	}
	/* 툴버튼 처리 */
	if( page[parentPage].get('tag').eq('div', 'tab') ) {
		if( page[parentPage].get('id').eq('missionPage') ) return;
		page[btnLock].findLayout().hideAll();
		page[cancel].hide();
	} else {
		if( page.btnLock ) {
			@lockButton=page.btnLock;
			lockButton.eventMap(onClick, this.lockClick);
		}
		if( page.btnClose ) {
			@closeButton=page.btnClose;
			closeButton.eventMap(onClick, this.closeClick);
		}
	}
}
EditorSrc.setSrc(src) {
	editor.clear();
	if( src.find("^|") ) {
		editor.insert(src, true).clearUndo();
		page.delay( callback() {
			editor.focus();
		},this);
	} else {
		editor.insert(src).clearUndo();
		editor.move(0);
	}
	if( saveButton ) saveButton.disable();
}

ProjectInfoGrid.ProjectInfoGrid(page) {
	this.addClass(common.Config );
	db=Class.db('config');
	dataModel=Class.model('ProjectInfo');
	grid=page.grid;
	grid.model( dataModel, gridMakeField('
		project_cd: 프로젝트 코드#120,
		project_nm:프로젝트 명#250,
		project_desc:설명#300', true)
	);
	grid.check('sortEnable', true);
	grid.eventMap(onChange, this.gridChange, 'node');
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
}
ProjectInfoGrid.initGrid() {
	root=grid.rootNode();
	db.fetchAll("select project_idx, project_cd, project_nm, project_desc, note from project_info where use_yn='Y'", root.removeAll() );
	grid.update();
	page.deleteProject.hide();
	gridHeaderWidth(grid);
}
ProjectInfoGrid.gridChange(node) {
	page.projectChange(node);
}
ProjectInfoGrid.gridDoubleClick(node) {
	page.projectSelect(node);
}
ProjectInfoGrid.gridResize() {
	gridHeaderWidth(grid);
}
ProjectInfoGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
ProjectInfoGrid.gridClick(node, column) {
	field=grid.field(column);
	switch( field ) {
	case check:	gridCheck(grid, node, page.deleteProject );
	case project_desc:
		grid.edit(node, 3);
	}
}
ProjectInfoGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}

PageStateGrid.PageStateGrid(page) {
	this.addClass(common.Config );
	db=Class.db('config');
	dataModel=Class.model('PageState');
	grid=page.grid;
	grid.model( dataModel, gridMakeField('
		page_icon: 아이콘			#60px,
		page_kind: 종류				#70px,
		page_template: 템플릿	#85px,
		page_group: 그룹			#110,
		page_code:페이지코드	#190,
		page_title:타이틀			#220,
		regdt: 등록일시				#130px')
	);
	grid.check('sortEnable', true);
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
}
PageStateGrid.search(node) {
	root=grid.rootNode();
	db.fetchAll("select project_idx, page_icon, page_group, page_code, page_title, page_kind, page_template, regdt from page_info where 1=1 #[project_idx ? and project_idx=#{project_idx}]", root.initNode(node));
	grid.update();
	page.deletePage.hide();
	gridHeaderWidth(grid);
}
PageStateGrid.gridResize() {
	gridHeaderWidth(grid);
}
PageStateGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	case page_kind:
		draw.text(rc, getCommCodeValue('pageKind', node[$field]), 'center');
	case page_template:
		draw.text(rc, getCommCodeValue('pageTemplate', node[$field]) );
	case page_icon:
		if( node[page_icon] ) draw.icon(rc.center(16,16), node[page_icon] );
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}

ProjectManager.ProjectManager(page) {
	this.addClass('common/control.PageBase');
	timelineNode={};
	canvas=page.canvas;
	canvas.eventMap( onDraw, this.canvasDraw, 'draw');
	canvas.eventMap( onMouseDown, this.canvasMouseDown, 'pos');
	canvas.eventMap( onMouseUp, this.canvasMouseUp, 'pos');
	canvas.eventMap( onMouseMove, this.canvasMouseMove, 'pos');
	canvas.eventMap( onEvent, this.commandEvent, 'type, node');
	/* 설정정보 세팅 */
	this.initConfig();
	/* 타이머 설정 */
	canvas.timer( 1000, callback() {
		this.timeout();
	}, this);
	this.initPage();
}
ProjectManager.initConfig() {
	/* 기본설정 정보 (향후 config DB에서 불러온다) */
	cf.debug=true;
	cf.projectId ='main';
	cf.pageXmlPath='data/pageXml';
}
ProjectManager.initPage() {
	cf.pageStart=false;
	cf.pageRate=1;
	this.timelineStart('ShiftMenu', 550, 22, 'in');
	this.setCanvasSize();
}
ProjectManager.loadMainPage(pageXml) {
	root= this.loadPage(pageXml);
	this.mainNode=root;
	this.pageStart();
	this.update();
}
ProjectManager.pageStart() {
	tag=this[mainNode];
	not( tag ) {
		return;
	}
	cf[pageStart]=true;
	cf[pageStartTick]=System.tick();
	cf[classErrorCheck].initNode();
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	arr=_arr(cf,'ActionRects').reuse();
	sx=tag[rect].right() - 100;
	arr.add( Class.rect(0,0,100,100) );
	arr.add( Class.rect(sx,0,100,100) );
}
ProjectManager.addCanvasEvent(type, node) {
	not( node ) return;
	canvas.postEvent(type, node);
}
ProjectManager.addPlayer(player) {
	_arr(this,'MoviewPlayers').add(player);
}
ProjectManager.alert(msg, title) {
	not( title ) title="알림";
	this.closePopup();
	root=xmlNode.child(0);
	popup=null;
	while( cur, root ) {
		not( cur[tag].eq('Popup') ) continue;
		if( cur[id].eq('MessageWindow') ) {
			popup=this.getControl(cur);
			pageNode=popup.loadPage();
			not( pageNode ) {
				page.alert("$popupId 팝업 XML 이 존재하지 않습니다");
				return;
			}
			titleNode=findTag('Title', pageNode);
			messageNode=findTag('Message', pageNode);
			titleNode[title]=title;
			messageNode[message]=msg;
			not( pageNode[pageId] ) pageNode[pageId]=popupId;
			break;
		}
	}
	cf[popupStartTick]=System.tick();
	cf[popupControl]=popup;
	this.timelineStart('FadeInPopup', 2000, 30, 'in', 'start');
	popup.conf();
	this.update();
}
ProjectManager.canvasDraw(draw) {
	/*
	geo=canvas.geo();
		not( geo.eq(this.prevGeo) ) {
		this.chageScroll(geo, this.prevGeo);
		this.prevGeo=geo;
	}
	*/
	draw.begin(canvas);
	while( tm, timelineNode ) {
		not( tm.state(NODE.start) ) continue;
		tid=tm[tid];
		if( Cf.timeLine("${tid}.running") ) {
			this.draw(draw, tm);
			if( cf[popupControl] ) {
				cf[popupControl].draw(draw,tm);
			}
			draw.end();
			return;
		} else {
			tm.state(NODE.start, false);
			this.draw(draw, tm);
			if( cf[popupControl] ) {
				cf[popupControl].draw(draw,tm);
			}
			draw.end();
			return;
		}
	}
	if( cf[stackPage] ) {
		cf[stackPage].draw(draw, null, tm);
	} else {
		this.draw(draw );
		if( cf[popupControl] ) {
			cf[popupControl].draw(draw,tm);
		}
	}
	if( cf[selectedItem] ) {
		rc=cf.selectedItem.rect;
		draw.rectLine(rc.incr(1), 0, '#afa0ea',3);
	}
	if( this.mouseDownAction ) {
		draw.save().pen('#cab0e9', 4);
		draw.polyLine(this[mouseActionPoints]);
		draw.restore();
	}
	draw.end();
}
ProjectManager.canvasMouseDown(pos) {
	while( rc, cf[ActionRects] ) {
		if( rc.contains(pos) ) {
			_arr(this,'mouseActionPoints').reuse();
			this.mouseDownAction=true;
		}
	}
	if( cf[stackPage] ) {
		cf[stackPage].mouseDown(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseDown(pos);
		return;
	}
	this.mouseDown(pos);
}
ProjectManager.canvasMouseMove(pos) {
	if( this.mouseDownAction ) {
		this[mouseActionPoints].add(pos);
		this.update();
	}
	not( cf.mouseMoveUse ) return;
}
ProjectManager.canvasMouseUp(pos) {
	if( this.mouseDownAction ) {
		arr=Cf.direction(this[mouseActionPoints]);
		print("canvasMouseUp=$arr");
		switch( arr.size() ) {
		case 1:
			arr.inject(a);
			switch(a) {
			case Right:
				this.openPopup('OrderConfirmNew');
			case Left:
				page.flags('splash');
				page.open();
			}
		case 2:
			arr.inject(a,b);
			switch(a) {
			case Right:
				switch(b) {
				case Down:	this.loadStackPage('AdminLogin');
				case Left:	this.loadStackPage('WebView');
				default:
				}
			case Left:
				switch(b) {
				case Down:
					page.flags('window');
					page.open();
				default:
				}
			}
		case 3:
			arr.inject(a,b,c);
			if( a.eq('Left'), b.eq('Down'), c.eq('Right') ) {
				_log("#exit kiosk");
				page.exit();
			}
		case 4:
			arr.inject(a,b,c,d);
			if( a.eq('Right'), b.eq('Down'), c.eq('Left'), d.eq('Up') ) {
				pageLoad('dev.main').open();
			}
		default:
		}
		this[mouseActionPoints].reuse();
		this.mouseDownAction=false;
		this.update();
	}
	if( cf[stackPage] ) {
		cf[stackPage].mouseUp(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseUp(pos);
		return;
	}
	this.mouseUp(pos);
}
ProjectManager.chageScroll(geo, prev) {
	while( widget, cf[SubWidgets] ) {
		not( widget ) return;
		if( widget.is('visible') ) {
			dx=geo.x()-prev.x(), dy=geo.y()-prev.y();
			leftTop=widget.geo().lt();
			widget.move(leftTop.incrY(dy));
		}
	}
}
ProjectManager.closeKiosk() {

}
ProjectManager.closePopup(popupId) {
	root=this[mainNode];
	not( root ) {
		root=xmlNode.child(0);
	}
	popup=null;
	if( popupId ) {
		while( cur, root ) {
			not( cur[tag].eq('Popup') ) continue;
			if( cur[id].eq(popupId) ) {
				popup=this.getControl(cur);
				break;
			}
		}
	}
	not( popup ) {
		popup=cf[popupControl];
	}
	/* 탐업창 close 처리 (로딩창 닫기 등) */
	cashOpen=false;
	if( popup ) {
		popup.inject(tag);
		if( tag[id].eq('MessageWindow') ) {
			order=cf[OrderHeader];
			if( order[InputCashOk] ) {
				if( order[DelayCount]>2 ) {
					cashOpen=true;
				}
			}
		}
		popup.closePopup();
	}
	/* 웹뷰를 닫는다 */
	webView=this.webviewWidget;
	if( webView ) {
		webView.hide();
	}
	/* 열려있는 동영상 플레이 닫는다 */
	while( player, _arr(this,'MoviewPlayers') ) {
		player.hide();
	}
	cf[popupControl]=null;
	if( cashOpen ) {
		this.openPopup('SelectCashReceipt');
	}
	this.update();
}
ProjectManager.closeStackPage() {
	not( cf[stackPage] ) return;
	this.closePopup();
	cf[stackPage].closePage();
	cf[stackPage]=null;
	this.update();
}
ProjectManager.commandAdd(type, node) {
	not( node ) return;
	node[command]=type;
	kioskWorker.push(node);
}
ProjectManager.commandEvent(type, tag) {
	switch( type ) {
	default:
		typeNm=getEventTypeName(type);
		_log("commandEvent type not define => type: $typeNm");
	}
}
ProjectManager.commandWork(type, tag) {

}
ProjectManager.conf() {
	tag=xmlNode.child(0);
	this.confLayout(tag);
}
ProjectManager.draw(draw,  timeline) {
	main=this[mainNode];
	not( main ) {
		main=xmlNode.child(0);
		this[mainNode]=main;
	}
	this.drawControl(draw, main, timeline);
	version=conf('version#kiosk.main');
	rc=Class.rect(390,15,200,40);
	draw.font(12,'normal','#f0f0f0');
	draw.text(rc, "ver. $version");
}
ProjectManager.loadStackPage(pageId) {
	root=this[stack$pageId];
	page=null;
	if( root ) {
		page=root.child(0);
	} else {
		node=xmlNode.child(0);
		root=node.addNode({tag:stack, id=$pageId});
		this[stack$pageId]=root;
		file=Class.file();
		fileName="$cf[pageXmlPath]/${pageId}.xml";
		if( file.isFile(fileName) ) {
			this.loadXml(fileName, root.removeAll() );
		}
		not( root.childCount() ) {
			_log("loadStackPage error={ $pageId loading fail}");
			return null;
		}
		page=root.child(0);
		page[type]='vbox';
		setNodeSize(page, true);
		treePage=get('Kiosk.EditPageTree');
		if( treePage ) {
			treePage.tree.update();
		}
	}
	tag=page.child(0);
	ctrl= this.getControl(tag);
	ctrl.conf();
	cf[stackPage]=ctrl;
	this.update();
	return ctrl;
}
ProjectManager.mouseDown(pos) {
	cf[mouseDownTick]=System.tick();
	root=xmlNode.child(0);
	this.mouseDownControl(root, pos);
}
ProjectManager.mouseUp(pos) {
	root=xmlNode.child(0);
	this.mouseUpControl(root, pos);
	cf[mouseDownTick]=0;
}
ProjectManager.openPopup(popupId) {
	this.closePopup();
	root=xmlNode.child(0);
	popup=null;
	while( cur, root ) {
		not( cur[tag].eq('Popup') ) continue;
		if( cur[id].eq(popupId) ) {
			popup=this.getControl(cur);
			pageNode=popup.loadPage();
			not( pageNode ) {
				page.alert("$popupId 팝업 XML 이 존재하지 않습니다");
				return;
			}
			not( pageNode[pageId] ) pageNode[pageId]=popupId;
			break;
		}
	}
	not( popup ) {
		page.alert("$popupId 팝업 정보가 존재하지 않습니다");
		return;
	}
	cf[popupStartTick]=System.tick();
	cf[popupControl]=popup;
	this.timelineStart('FadeInPopup', 2000, 30, 'in', 'start');
	popup.conf();
	this.update();
}
ProjectManager.print(root, depth) {
	not( root ) root=xmlNode;
	not( depth ) depth=0;
	indent='';
	while( n, depth ) indent.add(" ");
	while( cur, root ) {
		print("$indent $cur[tag]=$cur[rect]");
		this.print(cur, depth+1);
	}
}
ProjectManager.setCanvasSize() {
	root=this[mainNode];
	if( root ) {
		w=this.rate(root[Width]), h=this.rate(root[Height]);
		if( w, h ) canvas.size(w,h);
	}
}
ProjectManager.setPageRate(rate) {
	not( rate) return;
	not( typeof(rate,'number') ) {
		rate=rate.toNumber();
	}
	if( rate ) cf.pageRate=rate;
	this.setCanvasSize();
	this.conf();
	this.print();
	this.update();
}
ProjectManager.timelineCheck(cmd, target, style) {
	not( cmd ) {
		while( tm, timelineNode ) {
			not( tm.state(NODE.start) ) continue;
			if( Cf.timeLine("${tm[tid]}.running") ) {
				return true;
			}
		}
		return false;
	}
	if( target ) {
		tm=timelineNode.findOne('tid',cmd);
		not( tm ) {
			_log("$cmd 타임라인을 찾을수 없습니다");
		}
		tm.inject(duration, range, mode);
		tm.state(NODE.start, true);
		tm[drawCount]=0;
		tm.target=target;
		tm.command=target.command;
		tm.timelineStyle=style;
		Cf.timeLine("${cmd}.start", canvas, duration, range, mode);
	} else {
		return Cf.timeLine(cmd);
	}
}
ProjectManager.timelineStart(tid, duration, range, mode, state) {
	not( state ) state="stay";
	tm=timelineNode.findOne('tid',tid);
	not( tm ) {
		tm=timelineNode.addNode();
	}
	tm.state(0);
	if( state.eq('stay') ) {
		tm.state(NODE.stay, true);
	} else {
		tm.state(NODE.start, true);
	}
	tm[tid]=tid;
	tm[startTick]=System.tick();
	tm.put(duration, range, mode);
	if( tm.state(NODE.start)) {
		Cf.timeLine("${tid}.start", canvas, duration, range, mode);
	}
	return tm;
}
ProjectManager.timelineStop(key) {
	if( key ) {
		Cf.timeLine("${key}.stop");
	} else {
		while( key, Cf.timeLine() ) {
			if( Cf.timeLine("${key}.running") ) {
				Cf.timeLine("${key}.stop");
			}
		}
	}
	timelineNode.removeAll();
}
ProjectManager.timeout() {
	not( cf[pageStart] ) {
		return;
	}
	/* 타임아웃 체크 */
	timeoutCount=cf[timeoutCount++];
	mod=timeoutCount % 240;
	not( mod ) {
		print("timeoutCheck ========= $timeoutCount");
	}
}

PageTagTree.PageTagTree(page) {
	this.addClass(common.Config);
	cf.put( pageCd );
	db=Class.db('pages');
	dataModel=Class.model('PageTagTree');
	tree=page.tree;
	tree.check('treeMode', true);
	tree.model(dataModel, 'tag');
	tree.eventMap(onDraw, this.treeDraw, 'draw, node, over');
	tree.eventMap(onChange, this.treeChange, 'node');
	tree.eventMap(onMouseDown, this.treeMouseDown, 'pos, button');
}
PageTagTree.initTree(page) {
	while( cur, page) {
		cur[tag]='page';
	}
	Class.model('PageTagTree').rootNode(page);
	tree.update();
	if( cur ) {
		tree.expand(cur, true, true);
	}
}
PageTagTree.treeDraw(draw, node, over) {
	rc=treeIcon(tree, draw, node, over);
	rcIcon = rc.width(18).center(16,16);
	rc.incrX(20);
	info='';
	tag=node[tag];
	if( node[kind] ) {
		tag.add(":$node[kind]");
	}
	if( node[id] ) info="id=$node[id]";
	switch( node[tag] ) {
	case page:
		draw.icon( rcIcon, "vicon.application_form" );
		draw.save().font('bold');
		draw.text( rc,  tag);
		draw.restore();
	case vbox:
		draw.icon( rcIcon, 'vicon.application_tile_vertical');
		draw.text( rc,  tag);
	case hbox:
		draw.icon( rcIcon, 'vicon.application_tile_horizontal');
		draw.text( rc,  tag);
	case layout:
		draw.icon( rcIcon, 'vicon.application_form_edit');
		draw.text( rc,  tag);
	case [tree,grid] :
		draw.icon( rcIcon, 'vicon.application_side_boxes');
		draw.text( rc,  tag);
	case [group, splitter] :
		draw.icon( rcIcon, 'vicon.application_side_expand');
		draw.text( rc,  tag);
	case [div, tab] :
		draw.icon( rcIcon, 'vicon.application_view_tile');
		draw.text( rc,  tag);
	default:
		draw.icon( rcIcon, 'vicon.page_red');
		draw.text( rc,  tag);
	}
	if( info ) {
		draw.save().font(8,'normal','#60708a');
		w=draw.textWidth(info)+20;
		draw.text(rc.move('end',w), "($info)");
		draw.restore();
	}
}
PageTagTree.treeChange(node) {
	page.treeChange(node);
}
PageTagTree.treeMouseDown(pos, button) {
	if( button.eq('right') ) return 'ignore';
	node = tree.at(pos);
	not( node ) return;
	if( node[tag].eq('div','tab') ) {
		not(  node[@widgetArray] ) node.widget();
		c1=node.childCount(), c2= node[@widgetArray].size();
		if( c1 != c2 ) {
			node.removeAll();
			while( sub, node[@widgetArray] ) {
				cur=node.addNode({tag:subPage});
				cur[id]=when(sub[id], sub[id], sub[@funcName] );
				cur[page]=sub;
			}
			tree.update();
		}
	}
}

PageFuncGrid.PageFuncGrid(page) {
	this.addClass(common.Config );
	db=Class.db('pages');
	currentNode=null;
	currentPage=null;
	dataModel=Class.model('PageFuncInfo');
	grid=page.grid;
	grid.model( dataModel, gridMakeField('
		funcKind:유형#85px,
		funcName:함수명#180,
		funcParam:함수매개변수#120,
		note:비고#200', true)
	);
	grid.check('sortEnable', true);
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
	page.deleteFunc.eventMap(onClick, this.deletePageFuntion );
	page.allSelect.eventMap(onClick, this.allSelectClick);
	page.allCancel.eventMap(onClick, this.allCancelClick);
	page.ok.eventMap(onClick, this.okClick);
	page.cancel.eventMap(onClick, this.cancelClick);
}
PageFuncGrid.initGrid(node, page) {
	root=dataModel.rootNode();
	getNodeFuncInfo(node, root, page);
	total=root.childCount();
	page.gridStatus.value("(총 $total 건)");
	grid.update();
	gridHeaderWidth(grid);
	if( page ) {
		@currentPage=page;
	} else {
		id=node.id;
	}
	@currentNode=node;
}
PageFuncGrid.okClick() {
	root=grid.rootNode();
	cmsCode=currentPage[@cms.code], pageCode=currentPage[id];
	while( cur, root ) {
		if( cur[checked] ) {
			cur.put(cmsCode, pageCode);
			db.fetch("select case when length(ifnull(funcSrc,''))==0 then funcData else funcSrc end as src from pageFunc where cmsCode=#{cmsCode} and pageCode=#{pageCode} and funcName=#{funcName}", cur);
		}
	}
	root[currentPage]=currentPage;
	root.put(cmsCode, pageCode);
	fc=getParentFunc(page, 'addPageFuncsEdit');
	fc(root);
}
PageFuncGrid.gridDoubleClick(node) {
	root=grid.rootNode();
	cmsCode=currentPage[@cms.code], pageCode=currentPage[id];
	while( cur, root ) {
		if( cur[checked] ) {
			cur[checked]=false;
		}
	}
	node.put(cmsCode, pageCode);
	db.fetch("select case when length(ifnull(funcSrc,''))==0 then funcData else funcSrc end as src from pageFunc where cmsCode=#{cmsCode} and pageCode=#{pageCode} and funcName=#{funcName}", cur);
	node[checked]=true;
	root[currentPage]=currentPage;
	root.put(cmsCode, pageCode);
	fc=getParentFunc(page, 'addPageFuncsEdit');
	fc(root, true);
	grid.update();
}
PageFuncGrid.deletePageFuntion() {
	not( page.confirm("선택된 함수를 삭제하시겠습니까?") ) {
		return;
	}
	root=grid.rootNode();
	cmsCode=currentPage[@cms.code], pageCode=currentPage[id];
	arr=[];
	while( cur, root ) {
		if( cur[checked] ) {
			cur.put(cmsCode, pageCode);
			db.exec("delete from pageFunc where cmsCode=#{cmsCode} and pageCode=#{pageCode} and funcName=#{funcName}", cur);
			arr.add(cur);
		}
	}
	while( cur, arr ) {
		root.remove(cur);
	}
	arr.delete();
	grid.update();
}
PageFuncGrid.cancelClick() {
	page.hide();
}
PageFuncGrid.allSelectClick() {
	root=grid.rootNode();
	while( cur, root ) {
		cur[checked]=true;
	}
	grid.update();
}
PageFuncGrid.allCancelClick() {
	root=grid.rootNode();
	while( cur, root ) {
		cur[checked]=false;
	}
	grid.update();
}
PageFuncGrid.gridResize() {
	gridHeaderWidth(grid);
}
PageFuncGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	case funcKind:
		switch(node[sort] ) {
		case 1: icon="ficon.document-code";
			txt='생성함수';
		case 2: icon="ficon.document-globe";
			txt='이벤트함수';
		case 3: icon="ficon.document-epub";
			txt='위젯함수';
		case 4: icon="ficon.document-number";
			txt='사용자함수';
		}
		rcIcon=rc.width(20)..center(16,16);
		rc.incrX(20);
		draw.icon(rcIcon, icon);
		draw.text(rc, txt);
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
PageFuncGrid.gridClick(node, column) {
	field=grid.field(column);
	switch( field ) {
	case check:	gridCheck(grid, node, page.deleteFunc );
	case note:
		fieldNode=grid.fields().findOne('code',field);
		grid.edit(node, fieldNode.index());
	}
}
PageFuncGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}

PageBase.PageBase(page, canvas) {
	this.addClass( 'common.Config' );
	_log=func(msg, alert) {
		tm=System.localtime();
		if( msg.start('##') ) {
			db=Class.db('kiosk_hitec');
			logType='E';
			data=msg.value(2).trim();
			if( alert ) {
				db.exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status) values( 'device', 'qtmon', 'error', '$data', 'R')");
			}
		}
		print("log>> $msg");
	};
}
PageBase.findControl(tagName,root) {
	return this.findTag(tagName, root).get('@control');
}
PageBase.findTag(tagName, root) {
	if( tagName.find('#') ) {
		root=xmlNode.child(0);
		if( tagName.ch().eq('#') ) {
			return findTag(tagName.value(1), root);
		}
		return findSubTag(tagName, root);
	}
	if( root ) {
		if( typeof(root,'bool') ) {
			root=xmlNode;
		}
	} else {
		root=when(tag, tag, xmlNode);
	}
	return findTag(tagName, root);
}
PageBase.getControl(cur, cid) {
	ctrl=cur[@control];
	if( ctrl ) return ctrl;
	not( cid ) {
		cid=cur.tag;
		not( cid ) return;
	}
	if( cur[ClassPath] ) {
		classId="$cur[ClassPath]/control.$cid";
	} else {
		cf.inject(projectId, pageCode);
		classId="${projectId}/${pageCode}/control.$cid";
	}
	classErrorCheck=_node(cf, 'classErrorCheck');
	if( classErrorCheck[$classId] ) return null;
	include(classId);
	ctrl=newClass(classId, cur, this );
	not( ctrl ) {
		classErrorCheck[$classId]=true;
		_log("[error.getControl] $classId 컨트롤 로딩 실패\n cotrolNode: $cur");
		return null;
	}
	cur[@control]=ctrl;
	return ctrl;
}
PageBase.mainControl() {
	p=this;
	while( p ) {
		pp=p.parentCtrl;
		not( pp ) return p;
		p=pp;
	}
	return null;
}
PageBase.confLayout(tag) {
	setNodeSize(tag);
	while( cur, tag ) {
		this.getControl(cur).conf();
	}
}
PageBase.drawControl(draw, tag, timeline) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		this.getControl(cur).draw(draw, timeline);
	}
}
PageBase.mouseDownControl(root, pos) {
	while( cur, root ) {
		not( cur[rect].contains(pos) ) continue;
		this.getControl(cur).mouseDown(pos);
	}
}
PageBase.mouseUpControl(root, pos) {
	while( cur, root ) {
		not( cur[rect].contains(pos) ) continue;
		this.getControl(cur).mouseUp(pos);
	}
}
PageBase.rate(x) {
	not( x ) return null;
	x*=cf.pageRate;
	return x;
}
PageBase.update() {
	not( canvas ) {
		p=this.mainControl();
		p.inject(canvas);
	}
	canvas.redraw();
}

EditPageTree.EditPageTree(page) {
	this.addClass(common.Config);
	cf.put( pageCd );
	db=Class.db('pages');
	dataModel=Class.model('EditPageTree');
	tree=page.tree;
	tree.check('treeMode', true);
	tree.model(dataModel, 'tag');
	tree.eventMap(onDraw, this.treeDraw, 'draw, node, over');
	tree.eventMap(onChange, this.treeChange, 'node');
}
EditPageTree.treeDraw(draw, node, over) {
	rc=treeIcon(tree, draw, node, over);
	rcIcon = rc.width(18).center(16,16);
	rc.incrX(20);
	info='';
	tag=node[tag];
	if( node[kind] ) {
		tag.add(":$node[kind]");
	}
	if( node[id] ) info="id=$node[id]";
	if( node[class] ) {
		if( info ) info.add(", ");
		info.add("class=$node[class]");
	}
	if( node[style] ) {
		if( info ) info.add(", ");
		info.add("style=$node[style]");
	}
	if( node[type] ) {
		if( info ) info.add(", ");
		info.add("type=$node[type]");
	} else if( node[Layout] ) {
		if( info ) info.add(", ");
		info.add("Layout=$node[Layout]");
	}
	switch( node[tag] ) {
	case 'Page':
		draw.icon( rcIcon, "vicon.application_form" );
		draw.save().font('bold');
		draw.text( rc,  tag);
		draw.restore();
	case 'Grid':
		draw.icon( rcIcon, 'vicon.application_side_boxes');
		draw.text( rc,  tag);
	case 'Table':
		draw.icon( rcIcon, 'vicon.application_view_list');
		draw.text( rc,  tag);
	default:
		draw.icon( rcIcon, 'vicon.page_red');
		draw.text( rc,  tag);
	}
	if( info ) {
		draw.save().font(8,'normal','#60708a');
		w=draw.textWidth(info)+20;
		draw.text(rc.move('end',w), "($info)");
		draw.restore();
	}
}
EditPageTree.treeChange(node) {
	page.changeTag(node);
}

ClassComboSelect.ClassComboSelect(page) {
	this.addClass(common.Config);
	not( cf ) cf={};
	db=Class.db('pages');
	inheritCombo	=page.classInheritCombo;
	funcCombo		=page.classFuncCombo;
	varCombo		=page.classVarCombo;
	currentClass=null;
	varCombo.delegate(true, 24);
	funcCombo.delegate(true, 24);
	funcCombo.check('editable', true);
	inheritCombo.eventMap(onChange, this.inheritComboChange);
	varCombo.eventMap(onDraw, this.varComboDraw, 'draw, index, state' );
	varCombo.eventMap(onDraw, this.varComboChange );
	funcCombo.eventMap(onChange, this.funcComboChange );
	funcCombo.eventMap(onDraw, this.funcComboDraw, 'draw, index, state' );
	funcCombo.eventMap(onFocusIn, this.funcComboFocus);
}
ClassComboSelect.makeComboData(cls, all) {
	@currentClass=cls;
	cf.startCombo=false;
	arr=this.arr('inherit'), checkNode=this.node('checkNode');
	arr.add(cls[@className]);
	addClassName=func(cls) {
		while( className, cls[@addClass] ) {
			if( checkNode[$className] ) continue;
			checkNode[$className]=true;
			if( className ) {
				arr.add(className);
				addClassName( Cf.info('class', className) );
			}
		}
	};
	addClassName(cls);
	inheritCombo.removeAll().addItem(arr, null, '==전체==');
	cf.startCombo=true;
	not( cls ) {
		funcCombo.removeAll();
		varCombo.removeAll();
		return;
	}
	if( all ) {
		this.inheritComboChange();
	} else {
		inheritCombo.value(arr[0]);
	}
}
ClassComboSelect.funcComboFocus() {
	this.focusCombo=funcCombo;
	page.delay( callback() {
		this.focusCombo.selectText();
	}, this);
}
ClassComboSelect.showAll() {
	inheritCombo.findLayout().showAll();
}
ClassComboSelect.hideAll() {
	inheritCombo.findLayout().hideAll();
}
ClassComboSelect.inheritComboChange() {
	not( cf.startCombo ) return;
	className=inheritCombo.value();
	not( className ) {
		this.initClassCombo(currentClass, true);
		return;
	}
	cls=currentClass;
	not( className.eq(cls[@className]) ) {
		cls=Cf.info('class', className);
	}
	this.initClassCombo(cls);
	// funcCombo.focus();
}
ClassComboSelect.comboChangeValue(code) {
	combo=this[$code];
	val=combo.value(), root=combo.rootNode(), cur=null;
	key=combo[@key];
	not( val ) {
		val=combo.text();
		if( val ) {
			if( val.eq(this[${code}Val]) ) {
				return null;
			}
			cur=root.addNode();
			cur[$key]=val;
			cur.state(NODE.add, true);
		}
	}
	this[${code}Val]=val;
	not( val ) return null;
	not( cur ) {
		cur=root.findOne(key, val);
	}
	return cur;
}
ClassComboSelect.funcComboChange() {
	/* 클래스 함수가 변경시 : 해당 함수 미리보기 화면 팝업 */
	cur=this.comboChangeValue('funcCombo');
	not( cur ) {
		return false;
	}
	/* 클래스 정보 세팅 */
	inherit=this.inheritCombo;
	inheritClassName=inherit.value();
	if( inheritClassName ) {
		cur[class_grp]=inheritClassName.find('.').trim();
		cur[class_nm]=inheritClassName.find('.').right().trim();
	} else {
		if( cur.state(NODE.add) ) {
			this.alert("전체 클래스에서는 클래스 함수를 추가할수 없습니다. 클래스를 선택하세요");
			inherit.showPopup();
			return;
		}
		arr=inherit.rootNode();
		while( className, arr ) {
			not( className.find('.') ) continue;
			cur[class_grp]=className.find('.').trim();
			cur[class_nm]=className.find('.').right().trim();
			if( db.count("select count(1) from class_info where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", cur) ) {
				inheritClassName=className;
				break;
			}
		}
	}
	not( cur[src] ) {
		db.fetch("select case when length(class_src)==0 then class_data else class_src end as src from class_info where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", cur);
	}
	cur[inheritClassName]=inheritClassName;
	cur[currentClass]=this.currentClass;
	/*  메인페이지 인터페이스 */
	page.classFuncComboChange( cur );
}
ClassComboSelect.varComboChange() {
	this.varComboVal=varCombo.value();
}
ClassComboSelect.varComboDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(varCombo, draw, index, state);
	not( node) return;
	rc=draw.rect();
	w=draw.textWidth(node[type])+10;
	draw.font(10).text(rc.incrX(4,true), node[var]);
	draw.font(8).text(rc.move('end',w), "[${node[type]}]", 'right');
}
ClassComboSelect.funcComboDraw(draw, index, state) {
	if( state.eq(1) ) return;
	node= class('draw').comboDraw(funcCombo, draw, index, state);
	not( node ) return;
	rc=draw.rect();
	if( node[type] ) {
		rcIcon=rc.width(20).center(16,16);
		rc.incrX(20);
		if( node[type].eq('A') ) {
			draw.icon(rcIcon,"ficon.document-attribute-c");
		} else {
			ty=node[type].lower();
			draw.icon(rcIcon,"ficon.document-attribute-$ty");
		}
	} else {
		rc.incrX(4);
	}
	draw.font(10).text(rc, node[class_func]);
	if( node[class_param] ) {
		w=draw.textWidth(node[class_param])+30;
		draw.font(8).text(rc.move('end',w), "($node[class_param])",'right' );
	}
}
ClassComboSelect.initClassCombo(cls, all) {
	str=Cf.info('funcVar', cls, 'member').str();
node=Class.model('ClassVarInfo').rootNode();
node.removeAll();
maxStr='';
while( str.valid() ) {
	line=str.findPos("\n");
	not( line.ch() ) break;;
	not( line.find('=') ) continue;
	var=line.findPos('=').trim();
	if( var.ch().eq('@') ) continue;
	cur=node.addNode();
	cur[var]=var;
	cur[type]=line.trim();
	val="$cur[var]\t    [$cur[type]]";
	if( maxStr.size() < val.size() ) maxStr=val;
}
varCombo.addText(maxStr, true);
varCombo.removeAll().addItem(node,'var','==클래스 변수==');
funcCombo.removeAll();
node=Class.model('ClassFuncInfo').rootNode();
node.removeAll();
if( all ) {
	arr=cls.keys().sort();
	while( key,  arr ) {
		if( key.ch().eq('@') ) continue;
		fc=cls[$key];
		not( typeof(fc).eq("function") ) continue;
		cur=node.addNode();
		cur[class_func]=key;
		cur[class_param]=Cf.funcParam(fc);
	}
} else {
	if( cls[@classBase] ) {
		inherit="${cls[@classBase]}.${cls[@className]}";
		inherit.split('.').inject(a,b);
		node[class_grp]=a, node[class_nm]=b;
		db.fetchAll("select class_func, class_param, type, note from class_func where class_grp=#{class_grp} and class_nm=#{class_nm} order by type", node);
		node[class_nm]="${a}Base";
		db.fetchAll("select class_func, class_param, type, note from class_func where class_grp=#{class_grp} and class_nm=#{class_nm} order by type", node);
		node[inherit]=inherit;
	} else {
		inherit=inheritCombo.value();
		node[inherit]=inherit;
		if( inherit.find('.') ) {
			inherit.split('.').inject(a,b);
			node[class_grp]=a, node[class_nm]=b;
			db.fetchAll("select class_func, class_param, type, note from class_info where class_grp=#{class_grp} and class_nm=#{class_nm} order by type", node);
		} else {
			node[class_nm]=cls[@className];
			db.fetchAll("select class_func, class_param, note from class_info where class_nm=#{class_nm} order by type", node);
		}
	}
}
maxStr='';
while( cur, node ) {
	val="$cur[class_func] $cur[type]\t  $cur[class_param] ";
	if( maxStr.size() < val.size() ) maxStr=val;
}
funcCombo.addText(maxStr, true);
funcCombo.addItem(node, 'class_func', '==클래스 함수==');
page.initClassCombo();
}
ClassComboSelect.parseClassFunc(&s, funcNode) {
	classNm=null;
	while( s.valid() ) {
		c=s.ch();
		if( c.eq('}') || not(c) ) break;
		if( c.eq('/') ) {
			if( s.ch(1).eq('/') ) {
				s.findPos("\n");
			} else {
				s.match();
			}
			continue;
		}
		if( classNm ) {
			funcName=s.move();
			not( s.ch().eq('(') ) break;
			param=s.match();
			not( s.ch().eq('{') ) break;
			src=s.match(1);
			not( classNm.eq(funcName) ) {
				 funcNode.addNode().val( tag:'classFunc', funcName: funcName, funcParam:param, funcSrc: src );
			}
		} else {
			classNm=s.move();
			not( s.ch().eq('{') ) break;
			s.incr();
		}
	}
	printNode(funcNode);
}

TagAttributeGrid.TagAttributeGrid(page) {
	this.addClass(common.Config );
	dataModel=Class.model('EditPageAttribute');
	currentTagNode=null;
	grid=page.grid;
	grid.model( dataModel, 'attribute, value, note');
	grid.check('headerHide', true);
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onFilter, this.gridFilter, 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	grid.eventMap(onMouseDown, this.gridMouseDown, 'pos, button');
}
TagAttributeGrid.gridWidthReset() {
	this.gridWidthApply=false, this.delayCall=false;
	this.attributeWidth=0, this.valueWidth=0;
	grid.update();
}
TagAttributeGrid.initGridData(tagNode) {
	@currentTagNode=null;
	grid.rootNode().initNode();
	if( tagNode ) {
		this.makeTagAttribute(tagNode);
	}
	grid.update();
}
TagAttributeGrid.makeTagAttribute(tagNode) {
	if( currentTagNode==tagNode ) {
		return;
	}
	@currentTagNode=tagNode;
	root=grid.rootNode();
	while( sub, root ) {
		not( sub.refNode ) continue;
		if( sub.refNode==tagNode ) {
			return;
		}
	}
	tagNode.visible=true;
	this.makeGridData(tagNode);
}
TagAttributeGrid.makeGridData(srcNode, attrRoot) {
	not( typeof(srcNode,'node') ) {
		return;
	}
	root=grid.rootNode();
	_makeAttribute=func(src, depth) {
		arr=src.keys().sort();
		attrNode=[], attrData=[];
		while( attr, arr ) {
			if( attr.ch().eq('@') ) continue;
			val=src[$attr];
			if( attr.eq('id','kind','rect','type','tag','visible','Width','Height') ) {
				continue;
			}
			if( typeof(val,'node') ) {
				attrNode.addPair(attr,val);
			} else {
				attrData.addPair(attr,val);
			}
		}
		_addNode=func(attr, val) {
			node=root.addNode();
			node[attribute]=attr;
			node[srcNode]=src;
			node[depth]=depth+1;
			node[value]=val;
			if( typeof(val,'node') ) {
				node[tag]=nvl(val[tag], 'node');
				node[refNode]=val;
			}
		};
		while( pair, attrNode ) {
			pair.inject(attr, val);
			_addNode(attr, val);
			_makeAttribute(val, depth+1);
		}
		if( src[id] ) 		_addNode('id', src[id]);
		if( src[kind] ) 	_addNode('kind', src[kind]);
		if( src[type] ) 	_addNode('type', src[type]);
		if( src[rect] ) 	_addNode('rect', src[rect]);
		if( src[Width] ) 	_addNode('Width', src[Width]);
		if( src[Height] )	_addNode('Height', src[Height]);
		while( pair, attrData ) {
			pair.inject(attr, val);
			_addNode(attr, val);
		}
		attrNode.delete(), attrData.delete();
	};
	tag=srcNode[tag];
	if( tag ) {
		node=root.addNode();
		node[tag]=tag;
		node[attribute]=tag;
	} else {
		node=root.addNode({tag:root, attribute: $attrRoot});
	}
	node[depth]=0;
	node[refNode]=srcNode;
	_makeAttribute(srcNode, 0);
	this.gridWidthReset();
}
TagAttributeGrid.gridMouseDown(pos) {
	node = grid.at(pos);
	not( node[ButtonArray]  ) return;
	not( node.refNode ) return;
	ref=node.refNode;
	idx=node.index();
	node[ButtonArray].inject(margin,detail);
	if( margin.contains(pos) ) {
		root=node.parent();
		if( ref[Margin] ) {
			while(n,16) {
				idx++;
				cur=root.child(idx);
				if( cur[attribute].eq('Margin') ) {
					grid.edit(cur,1);
				}
			}
		} else {
			next=idx+1;
			cur=root.insertNode(next,{attribute:Margin, value:'0,0,0,0', depth:0});
			cur[srcNode]=node[srcNode];
			grid.update();
			grid.edit(cur,1);
		}
		return 'ignore';
	} else if( detail.contains(pos) ) {
		page.alert("상세 노드정보는 구현중입니다");
		return 'ignore';
	}

}
TagAttributeGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	ref=node.refNode;
	field=grid.field(draw.index());
	if( over ) {
		not( node.tag.eq('root')  || ref  ) {
			draw.fill(rc,'#d0d0d0');
		}
	}
	if( field.eq('attribute') ) {
		rcGrid=grid.rect();
		if( node.tag.eq('root')  || ref  ) {
			rc.width( rcGrid.width()-6 );
			rcIcon=rc.width(20).center(16,16);
			if( node.tag.eq('root') ) {
				draw.fill(rc,'#b0a09a');
				draw.icon(rcIcon, "ficon.application-plus-black");
			} else {
				if( ref==currentTagNode ) {
					draw.fill(rc,'#b0a09a');
					draw.icon(rcIcon, "ficon.application-plus-black");
				} else {
					draw.fill(rc,'#dacbd0');
					draw.icon(rcIcon, "ficon.application-plus");
				}
			}
			draw.rectLine(rc,4,'#38241F');
			draw.rectLine(rc.incrY(1,true),4,'#E9D5AF');
			rc.incrX(20);
			rcText=rc.incrX(node[depth*20]);
			draw.save().font('bold');
			draw.text(rcText, node[attribute]);
			draw.restore();
		} else {
			rcText=rc.incrX(node[depth*20]);
			draw.rectLine(rc,34,'#b0a09a');
			draw.text(rcText.incrX(4), node[attribute]);
			not( this.gridWidthApply ) {
				x=rc.x();
				x+=draw.textWidth(node[attribute]) + 25;
				if( this.attributeWidth < x ) this.attributeWidth=x;
				if( this.delayCall ) return;
				page.delay(callback() {
					attributeWidth=max(this.attributeWidth, 100);
					valWidth=max(150, this.valueWidth);
					if( valWidth>500 ) {
						valWidth=500;
					}
					w=attributeWidth + valWidth;
					rcGrid=grid.rect();
					noteWidth=rcGrid.width() - w - 6;
					grid.headerWidth(0, attributeWidth);
					grid.headerWidth(1, valWidth );
					grid.headerWidth(2, noteWidth );
					grid.update();
					this.gridWidthApply=true;
				}, this, 500);
				this.delayCall=true;
			}
		}
	} else if( field.eq('value') ) {
		if( node.tag.eq('root')  || ref  ) return;
		draw.rectLine(rc,3,'#804000', 1, 'dash');
		draw.rectLine(rc,4,'#b0a09a');
		attr=node[attribute];
		src=node[srcNode];
		val=src[$attr];
		draw.text(rc.incrX(4), val);
		not( this.gridWidthApply ) {
			x=draw.textWidth(val) + 60;
			if( this.valueWidth < x ) this.valueWidth=x;
		}
	} else if( field.eq('note') ) {
		if( ref  ) {
			node[rect]=rc.move('end',120);
			getRectArray(node, '4,5', 2, 'ButtonArray', 'hbox').inject(a,b);
			draw.ctrl('btn', a, 'Margin');
			draw.ctrl('btn', b, '상세정보');
		} else {
			draw.rectLine(rc,4,'#b0a09a');
			if( node.type.eq('prop') ) {
			}
		}
	}
}
TagAttributeGrid.gridFilter(node) {
	if( node[tag].eq('root') || node[depth].eq(0) ) return true;
	if( node.refNode ) {
		return node.refNode.visible;
	} else if( node.srcNode ) {
		return node.srcNode.visible;
	}
	return true;
}
TagAttributeGrid.gridClick(node, column) {
	if( node.tag.eq('root') ) {
		return true;
	}
	field=grid.field(column);
	ref= node.refNode;
	if( ref ) {
		ok=true;
	}
	if( ok ) {
		ref.toggle('visible');
		grid.update();
	} else if( field.eq('value', 'note') ) {
		if( field.eq('value') ) {
			if( ref.tag.eq('check') ) {
				node.toggle('value');
				ref="$node[widget]#$prop[code]";
				currentNode[$ref]=node[value];
				grid.update();
			} else {
				grid.edit(node,1);
			}
		} else {
			grid.edit(node,2);
		}
	}
}
TagAttributeGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	tag=node.tag;
	setRect=func(&s) {
		in=s.match();
		in.split(',').inject(x,y,w,h);
		src[$attr]=Class.rect(x,y,w,h);
	};
	gridInput=func(field) {
		input = page.widget(conf('widget.gridInput') );
		input.initWidget(page, field);
		src=node[srcNode];
		attr=node[attribute];
		obj=src[$attr];
		node[value]="$obj";
		return input;
	};
	switch( type ) {
	case create:
		if( field.eq('value') ) return gridInput();
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		src=node[srcNode];
		attr=node[attribute];
		obj=src[$attr];
		val="$obj";
		not( val.eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
			}
			if( field.eq('value') ) {
				bconf=false;
				if( typeof(obj,"node") ) {
					src[$attr]=s.split('parse');
				} else if( typeof( obj,"array") ) {
					src[$attr]=s.split('parse');
				} else if( typeof( obj,"rect") ) {
					setRect(data);
					bconf=true;
				} else {
					src[$attr]=data;
					if( attr.eq('Margin') ) bconf=true;
				}
				p=get('Kiosk.EditPageCanvas');
				if( p ) {
					ctrl=p.x;
					if( bconf ) ctrl.conf();
					ctrl.update();
				}
				this.gridWidthReset();
			} else {
				node[$field]=data;
			}
		}
		grid.update();
	default: break;
	}
}

ClassFuncGrid.ClassFuncGrid(page, comboImpl) {
	this.addClass(common.Config );
	db=Class.db('pages');
	comboImpl.inject(cf);
	dataModel=Class.model('ClassFuncInfo');
	grid=page.grid;
	grid.model( dataModel, this.makeFields('
		class_nm:클래스명#180,
		class_func:클래스 함수#220,
		class_param:함수 매개변수#250,
		note:비고#300', true)
	);
	grid.check('sortEnable', true);
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
	page.deleteFunc.eventMap(onClick, this.deleteClassFuntion);
	page.allSelect.eventMap(onClick, this.allSelectClick);
	page.allCancel.eventMap(onClick, this.allCancelClick);
	page.ok.eventMap(onClick, this.okClick);
	page.cancel.eventMap(onClick, this.cancelClick);
}
ClassFuncGrid.initGrid(impl) {
	if( impl ) {
		@comboImpl=impl;
	}
	root=dataModel.rootNode();
	cls=comboImpl[currentClass];
	if( cls ) {
		root[currentClass]=cls;
		total=root.childCount();
		page.gridStatus.value("(총 $total 건)");
	} else {
		root.removeAll();
		page.gridStatus.value("");
	}
	grid.update();
	gridHeaderWidth(grid);
}
ClassFuncGrid.okClick() {
	root=grid.rootNode();
classNode=root.currentClass;
if( classNode[@classBase] ) {
	table="class_func";
	root[inherit]="${classNode[@classBase]}.${classNode[@className]}";
} else {
	table="class_info";
}
root[inherit].split('.').inject(class_grp, class_nm);
while( cur, root ) {
	if( cur[checked] ) {
		cur.put(class_grp, class_nm);
		db.fetch("select case when length(class_src)==0 then class_data else class_src end as src from $table where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", cur);
	}
}
fc=getParentFunc(page, 'addClassFuncsEdit');
fc(root);
}
ClassFuncGrid.gridDoubleClick(node) {
	root=grid.rootNode();
classNode=root.currentClass;
if( classNode[@classBase] ) {
	table="class_func";
	root[inherit]="${classNode[@classBase]}.${classNode[@className]}";
} else {
	table="class_info";
}
root[inherit].split('.').inject(class_grp, class_nm);
while( cur, root ) {
	if( cur[checked] ) {
		cur[checked]=false;
	}
}
node.put(class_grp, class_nm);
db.fetch("select case when length(class_src)==0 then class_data else class_src end as src from $table where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", node);
node[checked]=true;
fc=getParentFunc(page, 'addClassFuncsEdit');
fc(root, true);
grid.update();
}
ClassFuncGrid.deleteClassFuntion() {
	not( page.confirm("선택된 함수를 삭제하시겠습니까?") ) {
		return;
	}
	root=grid.rootNode();
	root[inherit].split('.').inject(class_grp, class_nm);
	arr=[];
	while( cur, root ) {
		if( cur[checked] ) {
			cur.put(class_grp, class_nm);
			db.exec("delete from class_info where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", cur);
			arr.add(cur);
		}
	}
	while( cur, arr ) {
		root.remove(cur);
	}
	grid.update();
}
ClassFuncGrid.cancelClick() {
	page.hide();
}
ClassFuncGrid.allSelectClick() {
	root=grid.rootNode();
	while( cur, root ) {
		cur[checked]=true;
	}
	grid.update();
}
ClassFuncGrid.allCancelClick() {
	root=grid.rootNode();
	while( cur, root ) {
		cur[checked]=false;
	}
	grid.update();
}
ClassFuncGrid.gridResize() {
	gridHeaderWidth(grid);
}
ClassFuncGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	class('draw').gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			class('draw').gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	case class_nm:
		root=grid.rootNode();
		draw.text(rc, root[inherit]);
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		class('draw').gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
ClassFuncGrid.gridClick(node, column) {
	field=grid.field(column);
	switch( field ) {
	case check:	gridCheck(grid, node, page.deleteFunc );
	case class_func:
		grid.edit(node, 2);
	case note:
		fieldNode=grid.fields().findOne('code',field);
		grid.edit(node, fieldNode.index());
	}
}
ClassFuncGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}
ClassFuncGrid.makeFields(&s, check) {
	fields={};
	if( check ) {
		cur=fields.addNode({code:check, text: *} );
		cur.width = 40;
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

ClassFuncsManager.ClassFuncsManager(page) {
	this.addClass(common.Config, dev.EditorSrc );
	page.save.eventMap(onClick, saveSrc);
	page.run.eventMap(onClick, runSrc);
	db=Class.db('pages');
	currentNode=null;
}
ClassFuncsManager.initPage(root, append) {
		if( root ) {
		@currentNode=root;
	}
	map=_node(cf,'ClassFuncsMap');
	if( root[tail] ) {
		append=true;
		editor.value('');
	}
	not( append ) {
		map.initNode();
	}
	if( root[inherit] ) {
		classInfo=root[inherit];
	} else {
		root.inject(class_grp, class_nm);
		if( class_grp, class_nm ) {
			classInfo="${class_grp}.${class_nm}";
		}
	}
	editor.thisClass=root.currentClass;
	src='';
	while( cur, root ) {
		not( cur[checked] ) continue;
		note=when( cur[note], "/* $cur[note] */");
		if( note ) src.add("$note\r\n");
		key=cur[class_func];
		map[$key]=Class.pair(classInfo, root.currentClass);
		body=makeSourceIndentText(cur[src].ref(), "\t");
		src.add("${cur[class_func]}($cur[class_param]) {$body}\r\n\r\n");
	}
	if( root[tail] ) {
		src.add(root[tail] );
		root[tail]=null;
	}
	if( append ) {
		editor.append("\r\n$src", true);
	} else {
		this.setSrc(src);
	}
}
ClassFuncsManager.saveSrc() {
	me=this;
_save=func(&s) {
	map=_node(cf,'ClassFuncsMap');
	root=currentNode;
	classNode=null;
	tm=System.localtime();
	err='', note='';
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
		pair=map[$func];
		if( pair ) {
			pair.inject(info, classNode);
			info.split('.').inject(class_grp, class_nm);
			root.put(class_grp, class_nm);
		} else {
			classNode=root.currentClass;
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
		if( classNode[@classBase] ) {
			table="class_func";
			classBase="${root[class_grp]}Base";
			if( db.count("select count(1) cnt from class_func where class_grp=#{class_grp} and class_nm='${classBase}' and class_func=#{class_func}", root) ) {
				db.exec("update $table set class_src=#{class_src}, class_data=#{class_data}, class_param=#{class_param}, note=#{note}, tm='$tm' where class_grp=#{class_grp} and class_nm='${classBase}' and class_func=#{class_func}", root);
				ok=true;
			}
		}
		not( ok ) {
			num=db.exec("update ${table} set class_src=#{class_src}, class_data=#{class_data}, class_param=#{class_param}, note=#{note}, tm='$tm' where class_grp=#{class_grp} and class_nm=#{class_nm} and class_func=#{class_func}", root);
			not( num ) {
				db.exec("insert into ${table} ( class_grp, class_nm, class_func, class_param, class_src, class_data, note, tm ) values (#{class_grp}, #{class_nm}, #{class_func}, #{class_param}, #{class_src}, #{class_data}, #{note}, '$tm')", root);
			}
		}
		note='';
	}
	me[error]=err;
};
_parse=func(&s) {
	if( s.find("#>") ) {
		src=s.findPos("#>");
		_save( src );
	} else {
		_save(s);
	}
};
_parse( page[src].value() );
if( page[autoRunCheck].checked() ) {
	this.runSrc(true);
}
page.save.disable();
}
ClassFuncsManager.runSrc(flag) {
	me=this;
_run=func(&s) {
	map=_node(cf,'ClassFuncsMap');
	err='';
	root=currentNode;
	classNode=null;
	while( s.valid() ) {
		c=s.ch();
		not( c ) break;
		if( c.eq('/') ) {
			if( s.ch(1).eq('/') )
				s.findPos("\n");
			 else if( s.ch(1).eq('*') )
				s.match();
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
		pair=map[$func];
		if( pair ) {
			pair.inject(info, classNode);
		} else {
			classNode=root.currentClass;
		}
		Cf.call("classNode.function('$func', func($param) { $body });");
	}
	me[error]=err;
	return classNode;
};
parse=func(&s) {
	if( s.find("#>") ) {
		class=_run( s.findPos("#>") );
		cmd=s.findPos("\n").trim();
		Cf.call("class.$cmd");
	} else {
		_run(s);
	}
};
parse( page[src].value() );
}
ClassFuncsManager.update() {

}

CreateClassEditor.CreateClassEditor(page) {
	this.addClass(common.Config, dev.EditorSrc );
}
CreateClassEditor.initPage(node) {
	cf.editNode=node;
	if( node[PageMode].eq('edit') ) {
		page.save.value("수정");
	} else {
		page.save.value("저장");
	}
	tag=node[tag];
	if( node[src] ) {
		this.setSrc(node[src]);
	} else {
		this.setSrc(fmt(tr('template#class.control')), true);
	}
}

PageBase.messageBox(text, title) {
	this.mainControl().alert( text, title);
}

PageInfoGrid.PageInfoGrid(page) {
	this.addClass(common.Config );
	db=Class.db('config');
	projectCode=Cf[projectCode];
	if( projectCode ) {
		dataModel=Class.model("${projectCode}PageInfo");
	} else {
		dataModel=Class.model('PageInfo');
	}
	grid=page.grid;
	grid.model( dataModel, gridMakeField('page_icon: 아이콘#65, page_group: 그룹#110, page_code:페이지코드#190, page_title:타이틀#220', true) );
	grid.check('sortEnable', true);
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onChange, this.gridChange, 'node');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
}
PageInfoGrid.initGrid(node) {
	root=grid.rootNode();
	db.fetchAll("select project_idx, page_icon, page_group, page_code, page_title from page_info where 1=1 #[project_idx ? and project_idx=#{project_idx}]", root.initNode(node));
	grid.update();
	page.deletePage.hide();
	gridHeaderWidth(grid);
}
PageInfoGrid.gridChange(node) {
	page.pageChange(node);
}
PageInfoGrid.gridDoubleClick(node) {
	page.pageSelect(node);
}
PageInfoGrid.gridResize() {
	gridHeaderWidth(grid);
}
PageInfoGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	case page_icon:
		if( node[page_icon] ) draw.icon(rc.center(16,16), node[page_icon] );
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
PageInfoGrid.gridClick(node, column) {
	field=grid.field(column);
	switch( field ) {
	case check:	gridCheck(grid, node, page.deletePage );
	case note:		grid.edit(node, 2);
	}
}
PageInfoGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}

MessageWindow.MessageWindow(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MessageWindow.conf() {
	tagClearRect(tag);
	setNodeSize(tag, true);
	confNodeLayout(tag);
	cur=this.findTag('Confirm');
	cur[rcButton]=cur[rect].center(339, 89);
}
MessageWindow.draw(draw) {
	draw.effect(
		DRAW.RoundBox, tag[rect].incr(-1), 5, '#cacaca', '#ffffff', 2
	);
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case Title:
			drawNodeText( draw, cur[rect].incrX(40), cur[title], "left", 40, "#8080a0");
		case Message:
			drawNodeText( draw, cur[rect], cur[message], "center", 24, "#60606a");
		case Confirm:
			 rc=cur[rcButton], var=when( rc.eq(this.mouseDownRect),'p','n');
			 img=commonImage('btn_confirm', var);
			 draw.drawImage(rc, img);
		default:
		}
	}
}
MessageWindow.initControl() {
	tag.removeAll();
	tag[BackgroundImage]="${cf[imagePath]}/Type/popoup/pop_notice_bg.png";
	tag[Width]=936, tag[Height]=484;
	tag.addNode({tag: Title, Height:120, title:알림});
	tag.addNode({tag: Message});
	tag.addNode({tag: Confirm, Height:140});
	setNodeSize(tag, true);
}
MessageWindow.mouseDown(pos) {
	while( cur, tag ) {
		switch(cur[tag]) {
		case Title:
		case Message:
		case Confirm:
			if( cur[rcButton].contains(pos) ) {
				this.mouseDownRect=cur[rcButton];
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
MessageWindow.mouseUp(pos) {
	while( cur, tag ) {
		switch(cur[tag]) {
		case Title:
		case Message:
		case Confirm:
			if( cur[rcButton].contains(pos) ) {
				this.mainControl().popupClose();
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

Popup.Popup(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
}
Popup.conf() {
	not( this[mainNode] ) return;
	this.getControl( this[mainNode] ).conf();
}
Popup.draw(draw, timeline) {
	not( this[mainNode] ) return;
	this.getControl( this[mainNode] ).draw(draw, timeline);
}
Popup.getMainNode() {
	return this.mainNode;
}
Popup.mouseDown(pos) {
	not( this[mainNode] ) return;
	main=this[mainNode];
	if( main[autoClose] ) {
		if( cf[popupControl]== this ) {
			not( tag[rect].contains(pos) ) {
				this.popupClose();
			}
		}
	}
	this.getControl( main ).mouseDown(pos);
}
Popup.mouseUp(pos) {
	not( this[mainNode] ) return;
	this.getControl( this[mainNode] ).mouseUp(pos);
}

CanvasBase.CanvasBase() {
	this.addClass('common/control.PageBase');
	page=null, canvas=null;
	timelineNode={};
}
CanvasBase.loadMainPage(pageXml) {
	cf.inject(imagePath, projectId, pageCode);
	root= this.parseXml( fmt(pageXml) );
	if( root[Template] ) {
		tmp=root[Template].ref();
		while( tmp.valid() ) {
			var=tmp.findPos(';').trim();
			xml=fmt( conf(var) );
			this.parseXml(xml.ref(), root);
		}
	}
	this.mainNode=root;
	this.pageStart();
	this.update();
}
CanvasBase.setPageRate(rate) {
	not( rate) return;
	not( typeof(rate,'number') ) {
		rate=rate.toNumber();
	}
	if( rate ) cf.pageRate=rate;
	this.conf();
	this.update();
}
CanvasBase.conf() {
	main=this[mainNode];
	setNodeSize(main, true);
	confNodeLayout(main);
	while( cur, main ) {
		this.getControl(cur).conf();
	}
}
CanvasBase.draw(draw,  timeline) {
	not( cf.pageStart ) return;
	node=this[mainNode];
	if( cf[pageMode].eq('full') ) {
		rc=page.rect();
		not( rc.eq(cf[pageRect]) ) {
			cf[pageRect]=rc;
			node[Width] 		= rc.width();
			node[Height] 		= rc.height();
			node[rect]			= rc;
			canvas.size(rc);
			tagClearRect(node);
			this.conf();
			this.update();
		}
	}
	if( node[bg] ) {
		draw.drawImage(node[rect], imageLoad(node, "bg"), 'fill');
	}
	while( cur, node ) {
		if( cur[tag].eq('Popup') ) continue;
		this.getControl(cur).draw(draw, timeline);
	}
}
CanvasBase.mouseDown(pos) {
	while( cur, this[mainNode] ) {
		if( cur[GlobalMouseUse] ) {
			this.getControl(cur).mouseDown(pos);
			continue;
		}
		not( cur[rect].contains(pos) ) continue;
		if( cf[popupControl] ) {
			not( cur[tag].eq('Popup') ) continue;
		}
		this.getControl(cur).mouseDown(pos);
	}
}
CanvasBase.mouseUp(pos) {
	while( cur, this[mainNode] ) {
		if( cur[GlobalMouseUse] ) {
			this.getControl(cur).mouseUp(pos);
			continue;
		}
		not( cur[rect].contains(pos) ) continue;
		if( cf[popupControl] ) {
			not( cur[tag].eq('Popup') ) continue;
		}
		this.getControl(cur).mouseUp(pos);
	}
}
CanvasBase.mouseMove(pos) {
	while( cur, this[mainNode] ) {
		if( cur[GlobalMouseUse] ) {
			this.getControl(cur).mouseMove(pos);
			continue;
		}
		not( cur[rect].contains(pos) ) continue;
		if( cf[popupControl] ) {
			not( cur[tag].eq('Popup') ) continue;
		}
		this.getControl(cur).mouseMove(pos);
	}
}
CanvasBase.timelineStart(tid, target, style) {
	tm=timelineNode.findOne('tid',tid);
	not( tm ) {
		_log("$tid 타임라인을 찾을수 없습니다");
		return;
	}
	tm.inject(duration, range, mode);
	tm.state(NODE.start, true);
	tm[target]=target;
	tm[command]=target.command;
	tm[timelineStyle]=style;
	cf[currentTimeline]=tm;
	Cf.timeLine("${tid}.start", canvas, duration, range, mode);
}
CanvasBase.timelineStop() {
	while( tm, timelineNode ) {
		key=tm[tid];
		if( Cf.timeLine("${key}.running") ) {
			Cf.timeLine("${key}.stop");
			tm.state(NODE.start, false);
		}
	}
	cf[currentTimeline]=null;
}
CanvasBase.addCanvasEvent(type, node) {
	canvas.postEvent(type, node);
}

CanvasTest.CanvasTest(page) {
	this.addClass('common.CanvasBase');
	canvas=page.canvas;
	canvas.eventMap( onDraw, this.canvasDraw, 'draw');
	canvas.eventMap( onMouseDown, this.canvasMouseDown, 'pos');
	canvas.eventMap( onMouseUp, this.canvasMouseUp, 'pos');
	canvas.eventMap( onMouseMove, this.canvasMouseMove, 'pos');
	canvas.eventMap( onEvent, this.canvasEvent, 'type, node');
	/* 타이머 설정 */
	canvas.timer( 1000, callback() {
		this.timeout();
	}, this);
	/* 설정정보 세팅 */
	this.initConfig();
	this.initPage();
}
CanvasTest.initPage() {
	cf.pageStart=false;
	cf.pageRate=1;
}
CanvasTest.initConfig() {
	/* 기본설정 정보 (향후 config DB에서 불러온다) */
	cf.debug=true;
	cf.pageMode='full';
	cf.projectId ='VrsTest';
	cf.pageCode='GridControl';
	cf.imagePath="project/VrsTest/images";
}
CanvasTest.canvasDraw(draw) {
	tm=getDrawTimeline( timelineNode );
	if( cf[stackPage] ) {
		cf[stackPage].draw(draw, tm);
	} else {
		if( cf[popupControl] ) {
			this.draw(draw);
			cf[popupControl].draw(draw,tm);
		} else {
			this.draw(draw, tm);
		}
	}
	if( cf[selectedItem] ) {
		rc=cf.selectedItem.rect;
		draw.rectLine(rc.incr(1), 0, '#afa0ea',3);
	}
	if( cf[mouseDownAction] ) {
		draw.save().pen('#cab0e9', 4);
		draw.polyLine(cf[mouseActionPoints]);
		draw.restore();
	}
}
CanvasTest.canvasMouseDown(pos) {
	while( rc, cf[ActionRects] ) {
		if( rc.contains(pos) ) {
			_arr(cf,'mouseActionPoints').reuse();
			cf[mouseDownAction]=true;
		}
	}
	if( cf[stackPage] ) {
		cf[stackPage].mouseDown(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseDown(pos);
		return;
	}
	this.mouseDown(pos);
}
CanvasTest.canvasMouseMove(pos) {
	if( cf[mouseDownAction] ) {
		cf[mouseActionPoints].add(pos);
		this.update();
	}
	this.mouseMove(pos);
}
CanvasTest.canvasMouseUp(pos) {
	if( cf[mouseDownAction] && canvasMouseAction(this) ) {
		return;
	}
	if( cf[stackPage] ) {
		cf[stackPage].mouseUp(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseUp(pos);
		return;
	}
	this.mouseUp(pos);
}
CanvasTest.canvasEvent(type, node) {
	switch( type ) {
	case KIOSK.Log:
		if( cf.debugEditor ) cf.debugEditor.append( tag[logMessage], true );
	default: break;
	}
}

Popup.mouseMove(pos) {
	not( this[mainNode] ) return;
	this.getControl( this[mainNode] ).mouseMove(pos);
}

CanvasBase.getWidget(pageId) {
	node=_node(cf,'widgetNode');
	return node[$pageId];
}
CanvasBase.showWidget(pageId, rect, top) {
	node=_node(cf,'widgetNode');
	widget=node[$pageId];
	not( widget ) {
		print("CanvasBase::showWidget page id=======> $pageId $rect");
		widget=pageLoad(pageId, true); /* canvas.widget( getPageString(null,pageId), true ); */
		not( widget ) return;
		widget.flags('splash, top');
		node[$pageId]=widget;
		widget.open();
	}
	rcGlobal=canvas.mapGlobal(rect );
	widget.move(rcGlobal.lt());
	widget.size(rcGlobal.size());
	if( this.currentPageGeo ) {
		this.currentPageGeo=null;
		return;
	}
	not( this.activeEventSet ) {
		this.activeEventSet=true;
		mainPage=getMainPage(page);
		mainPage.eventMap( onActivationChange, this.widgetPageCheck );
		mainPage.eventMap( onMove, this.moveWidget );
	}
	not( widget.is('visible') ) widget.show();
	return widget;
}
CanvasBase.hideWidget(pageId) {
	node=cf[widgetNode];
	not( node ) return;
	if( pageId ) {
		widget=node[$pageId];
		if( widget ) widget.hide();
		return true;
	}
	while( key, node.keys(), n, 0 ) {
		widget=node[$key];
		if( widget ) widget.hide();
	}
	return when(n, true, false);
}
CanvasBase.drawWidgetPage(draw, rect, pageId, cur, top) {
	widget=this.getWidget(pageId);
	not( cur.childCount() ) {
		if( widget ) cur.addNode(widget);
	}
	ok=cf[submenuNode];
	not( ok ) {
		not( page.isActivate(), widget.isActivate() ) ok=true;
	}
	draw.drawWidget(rect, widget );
	if( ok ) {
		widget.hide();
	} else {
		this.showWidget(pageId, rect, top);
	}
}
CanvasBase.widgetPageCheck() {
	mainPage=getMainPage(page);
	state=mainPage.state();
	not( state.eq(this.prevPageState) ) {
		switch( state ) {
		case minimized:
			this.hideWidget();
		case active:
			this.update();
		case fullscreen:
			this.update();
		default:
			if( this[prevPageState].eq('maximized') ) {
				this.hideWidget();
			}
		}
		this.prevPageState=state;
	}
	geo=mainPage.geo();
	not( geo.eq(this.prevPageGeo) ) {
		this.prevPageGeo=geo;
		this.update();
	}
	not( page.is('visible') ) {
		this.hideWidget();
	}
	tick=System.tick();
	not( this.prevActivateTick ) {
		this.prevActivateTick=tick;
	}
	node=cf[widgetNode], keys=node.keys();
	while( key, keys ) {
		widget=node[$key];
		not( widget ) continue;
		if( widget.is('visible') ) {
			flag=when( mainPage.isActivate(), true, false );
			widget.showTop(flag);
		}
	}
}
CanvasBase.moveWidget() {
	mainPage=getMainPage(page);
	geo=mainPage.geo();
	this.currentPageGeo=geo;
	this.prevPageGeo=geo;
	this.update();
}
CanvasBase.loadForm(root, code, rc) {
	xml=conf(code);
	xmlNode=this.parseXml( xml,  root.removeAll() );
	inputs=_node(root, 'InputNode');
	inputs.initNode();
	_parse=func(s) {
		arr=_arr(row,'cells',true);
		while( s.valid() ) {
			c=s.ch();
			if( c.eq('<') ) {
				sp=s.cur(), s.incr();
				tag=s.move();
				prop=s.findPos('>'), data=null;
				if( tag.eq('blank') ) {
					arr.add("#blank");
				} else {
					inputNode=inputs.addNode();
					this.parseProp(inputNode, tag, prop);
					if( data ) inputNode[data]=data;
					idx=inputNode.index(), key="@{$idx}";
					inputs[$key]=inputNode;
					arr.add(key);
				}
				if( s.ch().eq(',') ) {
					s.incr();
				} else {
					data=s.findPos("</$tag>");
					if( s.ch().eq(',') ) {
						s.incr();
					}
				}
			} else {
				arr.add(s.findPos(",").trim() );
			}
		}
	};
	_h=func( r, c ) {
		h=ha.get(r), r+=1;
		while( n,  ha.size(), r ) {
			row=xmlNode.child(r), val=row[cells].get(c);
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
			not( arr[$n].eq('#') ) break;
			w+=wa.get(n);
		}
		return w;
	};
	rc.inject( sx, sy, sw, sh), sx=0, sy=0;
	vr=nvl( xmlNode[vrate], xmlNode.childCount());
	ha=_arr().recalc( min(sh,305), vr );
	wa=_arr().recalc( sw, xmlNode[rate] );
	print(wa, ha, sh, sw);
	while( row, xmlNode ) {
		_parse( row[data].ref() );
	}
	maxCell=0;
	while( row, xmlNode) {
		size=row[cells].size();
		if( size>maxCell ) maxCell=size;
	}
	while( row, xmlNode) {
		size=row[cells].size();
		if( size<maxCell ) {
			ep=maxCell-size;
			while( n, ep ) row[cells].add('#');
		}
	}
	while( h, ha, r, 0 ) {
		row=xmlNode.child(r), arr=row[cells];
		cx=sx, lastCell=0, size=arr.size();
		row[rect]=Class.rect(cx, sy, sw, h);
		while( c, size, 0 ) {
			val=arr[$c];
			if( val.eq('X','#') ) {
				cx+=wa.get(c);
				continue;
			}
			ch=_h(r,c), cw=_w(arr,c, size);
			row[rect $c]=Class.rect(cx, sy, cw, ch);
			if( val.ch().eq('@') ) {
				inputNode=inputs[$val];
				if( inputNode ) {
					inputNode[rect]=row[rect $c];
					print("inputNode == $val, $inputNode, $inputNode[rect] xxx");
				}
			}
			cx+=cw;
			lastCell=c;
		}
		row[lastCell]=lastCell;
		sy+=h;
	}
}
CanvasBase.drawCodeForm(draw, cur, node) {
	rcBody=cur[rect body];
	rc=nvl( node[rect form], rcBody.incr(10) );
	rc.inject( sx, sy, sw, sh);
	curCode=nvl( node[depth].eq(2), node, node[currentRow]);
	rc.bottom( rcBody.bottom() );
	not( cur.childCount() ) {
		this.mainControl().loadForm(cur, 'data#form.CommonCode', rc);
	}
	rc.inject(x,y);
	offset=Class.point(x,y), cur[offset]=offset;
	draw.font(12,'normal', '#606060');
	form=findTag('form', cur);
	lastRow=form.childCount()-1;
	while( row, form, r, 0 ) {
		last=row[lastCell];
		while( val, row[cells], c, 0 ) {
			if( val.eq('X','#blank') ) continue;
			rc=row[rect $c].incrXY(offset, true);
			if( c.eq(last) ) {
				draw.rectLine(rc, 4, '#909090');
			} else {
				draw.rectLine(rc, 34, '#909090');
			}
			if( val ) draw.text(rc, val, 'center');
		}
		rc=row[rect].incrXY(offset, true);
		if( r.eq(0) ) {
			draw.rectLine(rc, 2, '#909090', 2);
		} else if( r.eq(lastRow) ) {
			draw.rectLine(rc, 4, '#909090', 2);
		}
	}
}
CanvasBase.drawSubPage(draw, widget, rect) {
	ok=cf[submenuNode];
	not( ok ) {
		not( page.isActivate(), widget.isActivate() ) ok=true;
	}
	draw.drawWidget(rect, widget );
	if( ok ) {
		widget.hide();
	} else {
		this.showWidget(widget[pageId], rect, top);
	}
}


mainCanvas.mainCanvas(page) {
	this.addClass('common.CanvasBase');
	canvas=page.canvas;
	canvas.eventMap( onDraw, this.canvasDraw, 'draw');
	canvas.eventMap( onMouseDown, this.canvasMouseDown, 'pos');
	canvas.eventMap( onMouseUp, this.canvasMouseUp, 'pos');
	canvas.eventMap( onMouseMove, this.canvasMouseMove, 'pos');
	canvas.eventMap( onEvent, this.canvasEvent, 'type, node');
	page.eventMap( onActivationChange, this.widgetPageCheck );
	page.eventMap( onMove, this.moveWidget );
	/* DB 세팅
	db=Class.db('namzatang_local');
	not( db.open() ) db.open('project/Kiosk/data/namzatang.db');
	*/
	db=Class.db('kiosk_hitec');
	/* 키오스크 Worker 설정 */
	kioskWorker=Class.worker('kiosk');
	kioskWorker.start(callback(node) {
		not( node ) return;
		this.workerProcess(node[command], node);
	});
	/* QTMon 소켓 디스패쳐 설정 */
	kioskDeviceNode={};
	kioskDeviceSocket=Class.socket("KioskDevice");
	kioskDeviceSocketWorker = Class.worker("KioskDevice");
	kioskDeviceSocketWorker.start( func() {
		not( kioskDeviceNode[StartSocket] ) return;
		recv=kioskDeviceSocket.readBuffer();
		if( recv ) {
			kioskDeviceNode[recvData]=recv;
			print("kioskDeviceSocketWorker [$kioskDeviceNode]");
			this.commandAdd( KIOSK.SOCKET_RECV_DATA, kioskDeviceNode);
		}
	});
	/* 키오스크 TransData */
	include('common/kiosk.TransDataControl');
	kioskTransData = newClass('common/kiosk.TransDataControl', this, canvas);
	/* 타이머 설정 */
	canvas.timer( 1000, callback() {
		if( cf[orderStartTick] ) {
			dist=System.tick()- cf[orderStartTick];
			if( dist<12000 ) {
				print("# 주문중 타이머 중지($dist)");
				return;
			}
			cf[orderStartTick]=0;
		}
		this.timeout();
	}, this);
	/* 설정정보 세팅 */
	this.initConfig();
	this.initPage();
}
mainCanvas.initConfig() {
	db.close();
	db.open("data/kiosk.db");
	cf.debug=true;
	cf.pageMode='scroll';
	cf.projectId ='KioskHiTec';
	cf.pageCode='main';
	cf.imagePath=conf('setup#kiosk.imagePath');
	cf.kioskStatus=0;
	/* 윈도우 & 타이머 정보 */
	cf.popupStartTick=System.tick();
	cf.popupControl=null;
	cf.clipRect=null;
	cf.subWidgets={};
	cf.playerWidgets=[];
	cf.kioskStartDtm = System.date('yyyy-MM-dd hh:mm:ss');
	/* QTMon 정보 */
	cf.qtMonHost		= conf('setup#kiosk.qtMonHost');
	cf.qtMonPort		= conf('setup#kiosk.qtMonPort');
	/* 카드 VAN사 타입 */
	/* 1:KCP, 2:KSNET */
	cf.cardVanType	= conf('setup#kiosk.cardVanType');
	not(cf.cardVanType) cf.cardVanType = "1";
	/* 번호 설정 타입 : 주방프린트IP, 주문스크린IP */
	/* 1:코너별(기존), 2:번호별 */
	cf.noSetupType	= conf('setup#kiosk.noSetupType');
	not(cf.noSetupType) cf.noSetupType = "1";
	/* [주문관리]
		- 현금 영수증 구분 : CashReceiptType (Personal, Company, Node)
		- TakeOut 구분 : OrderSelectType(Hall, BoilTakeout, TakeOut)
	******************************************************************/
	cf.CornerInfo			={tag:CornerInfo};
	cf.SetupInfo			={tag:SetupInfo};
	cf.OrderHeader		={tag:OrderHeader};
	cf.PrintInfo				={tag:PrintInfo};
	cf.PosNo='2';
	/* 포스정보 */
	cf[easyCardUrl]	= conf('setup#kiosk.easyCardUrl');
	pos={tag:PosInfo};
	pos.PosNo = "01";
	pos.VanCode = "01";
	pos.VanName = "KCP";
	pos.VanIP = "203.238.36.156";
	pos.VanPort = 19834;
	pos.VanTermID = "1002189855";
	pos.VanData1 = "0031";
	cf.PosInfo=pos;
	/* 타임라인 추가 */
	this.timelineAdd('ShiftMenu', 550, 22, 'in');
	this.timelineAdd('SlideMenu', 550, 22, 'out');
	this.timelineAdd('CornerTabChange', 550, 20, 'in');
	this.timelineAdd('SelectMenu', 650, 30, 'out');
	this.timelineAdd('ShoppingCart', 500, 20, 'out');
	this.timelineAdd('FadeOutPopup', 800, 15, 'out');
	this.timelineAdd('FadeInPopup', 800, 15, 'in');
	this.timelineAdd('PageEffect', 800, 15, 'out');
	this.timelineAdd('SlectLanguage', 600, 15, 'out');
	/* 키오스크 설정 정보 불러오기 */
	setupProcess(db, cf);
	setup=cf[SetupInfo];
	setup[today]=System.date('yyyyMMdd');
	db.fetch("select seq as last_deal_no from tb_key_gen where key_type='DealNo' and key_date=#{today}", setup);
	not( setup[last_deal_no] ) {
		setup[order_start_no++];
		db.exec("insert into tb_key_gen (key_type, key_date, seq) values('DealNo',#{today}, #{order_start_no})", setup);
	}
}
mainCanvas.initPage() {
	Cf[KioskLangSelect]='Kor';
	cf.pageStart=false;
	cf.pageRate=1;
	cf.mainUpdateTick=System.tick();
}
mainCanvas.canvasDraw(draw) {
	not( cf[pageStart] ) {
		return;
	}
	tm=cf[currentTimeline];
	if( tm ) {
		not( Cf.timeLine("${tm[tid]}.running") ) {
			cf[currentTimeline]=null;
		}
	}
	if( cf[stackPage] ) {
		cf[stackPage].draw(draw, tm);
		if( cf[popupControl] ) {
			cf[popupControl].draw(draw,tm);
		}
	} else {
		if( cf[popupControl] ) {
			this.draw(draw, tm);
			cf[popupControl].draw(draw,tm);
		} else {
			this.draw(draw, tm);
		}
	}
	/*
	if( cf[selectedItem] ) {
		rc=cf[selectedItem.rect];
		draw.rectLine(rc.incr(1), 0, '#afa0ea',3);
	}
	*/
	if( cf[mouseDownAction] ) {
		arr=cf[ActionRects];
		while( rc, arr ) {
			draw.rectLine(rc.incr(15),0,"#6B594A");
		}
		if( this[actionDownTick] ) {
			dist=System.tick() - this[actionDownTick];
			if( dist>1000 ) {
				rc=arr[1];
				draw.fill(rc.incr(17),"#6B594A");
			}
		}
		if( cf[mouseActionPoints].size() > 5 ) {
			draw.save().pen('#cab0e9', 4);
			draw.polyLine(cf[mouseActionPoints]);
			draw.restore();
		}
	}
}
mainCanvas.canvasMouseDown(pos) {
	while( rc, cf[ActionRects], num, 0 ) {
		not( rc.contains(pos) ) continue;
		not( cf[mouseActionPoints] ) {
			cf[mouseActionPoints]=[];
		}
		if( num.eq(1) ) {
			cf[mouseDownAction]=true;
			this[actionDownTick]=System.tick();
			this.update();
			return;
		}
		this[actionDownTick]=0;
		not( this[prevMouseTick] ) {
			this[prevMouseTick]=System.tick();
			cf[mouseDownAction]=true;
			this.update();
			return;
		}
		dist=System.tick() - this[prevMouseTick];
		if( dist<2000 ) {
			this[MouseDownCount++];
			if( this[MouseDownCount].eq(2) ) {
				print("xxxxxxx admin xxxxxxxxxx");
				cf[mouseDownAction]=false;
				this.update();
				this[prevMouseTick]=0;
				this[MouseDownCount]=0;
				if( System.processCheck('KioskWatcher.exe') ) {
					Class.web('admin').call('http://localhost:8089/@kiosk.Common.WatcherOpen');
				} else {
					System.run("KioskWatcher.exe");
				}
			}
			return;
		} else {
			this[MouseDownCount]=1;
		}
		this[prevMouseTick]=System.tick();
		cf[mouseDownAction]=true;
		return;
	}
	cf[mouseDownTick]=System.tick();
	if( cf[popupControl] ) {
		cf[popupStartTick]=System.tick();
		cf[popupControl].mouseDown(pos);
	} else if( cf[stackPage] ) {
		cf[stackPage].mouseDown(pos);
	} else {
		this.mouseDown(pos);
	}
}
mainCanvas.canvasMouseMove(pos) {
	if( cf[mouseDownAction] ) {
		cf[mouseActionPoints].add(pos);
		this.update();
	}
	this[canvas].cursor(CURSOR.ArrorCursor);
	this[prevMousePos]=pos;
	this.mouseMove(pos);
}
mainCanvas.canvasMouseUp(pos) {
	if( cf[mouseDownAction] ) {
		cf[mouseDownAction]=false;
		asize = cf[mouseActionPoints].size();
		ok=false;
		while( rc, cf[ActionRects], num, 0 ) {
			not( rc.contains(pos) ) continue;
			if( num.eq(1) ) {
				ok=true;
				break;
			}
		}
		if( this[actionDownTick] && ok ) {
			dist=System.tick() - this[actionDownTick];
			if( dist>1000 ) {
				print("시스템 메뉴 오픈: $dist");
				dialog=this.findControl('Popup#dialog');
				node=this[mainNode], rc=node[rect];
				dialog.popupOpen('SystemMenu', 'popup', rc.center(960,586), rc );
			}
		} else if( asize>10 ) {
			arr=Cf.direction( cf[mouseActionPoints] );
			print("canvasMouseAction=$arr");
			if( arr.size() > 1  ) {
				str=arr.join();
				switch(str) {
				case LeftDownRight:
					this.closeKiosk();
					Cf.exit();
				case LeftDown:
					not( System.processCheck('KioskWatcher.exe') ) {
						System.run("KioskWatcher.exe");
					}
					cf[errorOpen]=false;
					this.popupClose();
					Class.db('kiosk_hitec').exec("update kiosk_error set error_status='S' where error_status='R'");
				case RightLeft:
					cf[mouseDownAction]=false;
					Class.db('kiosk_hitec').exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status, tm) values( 'notify', 'admin', '점검', '잠시 정검중입니다. 조금만 기다려 주세요', 'R', 0 )" );
					this.alert("잠시 정검중입니다. 조금만 기다려 주세요","정검", true);
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
		cf[mouseActionPoints].reuse();
		cf[mouseDownAction]=false;
		this[actionDownTick]=0;
		this.update();
		return;
	}
	this[prevMouseTick]=0;
	cf[mouseDownAction]=false;
	if( cf[popupControl] ) {
		cf[popupControl].mouseUp(pos);
	} else if( cf[stackPage] ) {
		cf[stackPage].mouseUp(pos);
		return;
	} else {
		this.mouseUp(pos);
	}
}
mainCanvas.canvasEvent(type, node) {
	print("############# canvasEvent $type ##################");
	switch( type ) {
	case KIOSK.Log:
		if( cf.debugEditor ) cf.debugEditor.append( tag[logMessage], true );
	default:
		this.commandProcess(type, node );
	}
}
mainCanvas.timeout() {
	not( cf[pageStart] ) {
		return;
	}
	lastPos=this[lastMousePos];
	if( this[prevMousePos].eq(lastPos) ) {
		this[canvas].cursor(CURSOR.BlankCursor);
	} else {
		this[lastMousePos]=this[prevMousePos];
	}
	if( cf[mouseDownAction] ) {
		asize = cf[mouseActionPoints].size();
		if( asize<8 ) {
			this.update();
		}
	}
	dist=System.tick()-cf[mainUpdateTick];
	not( cf[kioskStatus] ) {
		not( cf[popupControl] ) {
			node=this[mainNode], rc=node[rect];
			this.popupOpen('MainLoading', rc.incr(15), rc );
		}
		if( cf[kioskStartTick] ) {
			cf[kioskStatus]=1;
		} else {
			if( dist>5000 ) {
				not( cf[kioskStartTick] ) {
					cf[kioskStartTick] = System.tick();
				}
				cf[mainUpdateTick]	= System.tick();
				cf[kioskStatus]=1;
				updateDidCheck(db, cf, 1500000 );
			}
		}
		return;
	}
	if( dist > 10000 ) {
		/* 마우스 클릭중에는 체크하지 않는다 */
		dist=System.tick() - cf[mouseDownTick];
		if( dist<800 ) {
			return;
		}
		cf[mainUpdateTick]=System.tick();
		return;
	}
	switch( cf[kioskStatus] ) {
	case 1:
		cf[kioskStatus++];
		this.workerAdd( KIOSK.CornerMenuReload );
	case 2:
		cf[kioskStatus++];
		timeout = System.tick() - cf[kioskStartTick];
		_log("페이지 새로고침 완료 : $timeout");
	case 3:
		cf[kioskStatus++];
		this.findControl('#CornerTab').makeDisplayTab(true);
		this.popupClose();
		this.goHome(true);
	default:
		this.popupCloseCheck();
	}
}
mainCanvas.workerAdd(type, node) {
	not( node ) {
		node=cf;
	}
	node[command]=type;
	kioskWorker.push(node);
}
mainCanvas.workerProcess(type, node) {
	_log("workerProcess 타입 => type: $type");
	switch( type ) {
	case KIOSK.CornerMenuReload:
		this.reloadCornerTab();
		System.sleep(1500);
		node=_node('QtMonNode');
		socket=node[socket];
		not( socket ) {
			socket=Class.socket("QtMon");
			node[socket]=socket;
			node[socketStartTick]=System.tick();
		}
		dist=System.tick() - cf[pageStartTick];
		if( dist>13000 ) {
			not( socket.isConnect() ) {
				not( socket.connect( cf[qtMonHost], cf[qtMonPort], 2000) ) {
					socket.close();
					_log("## 디바이스 연결오류, 호스트:$cf[qtMonHost], 포트: $cf[qtMonPort]", true);
					return;
				}
				this.qtMonSendData('01,4,1,0,1,0');
				this.qtMonSendData('21,01,1,1');
			}
		}
		cf[kioskStatus]=2;
	case KIOSK.MasterTransData:
		kioskTransData.masterDownload();
	}
}
mainCanvas.commandAdd(type, node) {
	not( node ) return;
	canvas.postEvent(type, node);
}
mainCanvas.commandProcess(type, node) {
	typeNm=getEventTypeName(type);
	print("@@ commandProcess 타입 => type: $typeNm @@");
	switch( type ) {
	case KIOSK.Log:
		cf.debugEditor.append( node[logMessage], true );
	case KIOSK.CornerTabChange:
		this.timelineStart('CornerTabChange', node, 'FadeInOut');
	case KIOSK.EashCheckError:
		msg=node[ErrorMessage];
		not( cf[CashReceiptType] ) {
			this.closePopup();
		}
		this.alert(msg, "결제오류");
	case KIOSK.EasyCheckOk:
		_log("# 카드결제 완료 : $node");
		order_completeCardProcess(db, node, cf[OrderHeader], this);
	case KIOSK.CornerMenuLoadFinish:
		this.goHome();
		_log("# CornerMenu Load OK");
	case KIOSK.SOCKET_RECV_DATA:
			data=node[recvData];
			if( data ) {
				this.qtMonRecvData(data.ref());
				node[recvData]=null;
			}
	case KIOSK.SOCKET_SEND_DATA:
			data=node[sendData];
			if( data ) {
				this.qtMonSendData(data.ref());
				node[sendData]=null;
			}
	case KIOSK.TRANS_DATA_OK:
			this.updateKioskData(node);
	case KIOSK.OrderComplete:
		order=cf[OrderHeader];
			not( order_completeProcess(db, order, this ) ) {
				page.alert(order[error]);
				_log("## 주문 완료처리중 오류가 발생했습니다 : $order");
			}
			cf[OrderHeader.PayType]=null;
	default:
	}
}

MainTitle.MainTitle(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MainTitle.initControl() {
	tag.addNode({tag:LanguageSelect, class:layer, Margin: [20,130], Height:66, Width:100});
	setNodeSize(tag, true);
}
MainTitle.conf() {
	setNodeSize(tag,true);
	confNodeLayout(tag);
	cur=this.findTag('HomeButton'), img=imageLoad(cur,'src');
	cur[rcButton]=img.center( cur[rect]);
	cur=this.findTag('LanguageSelect');
	cur[SelectMode]=true;
}
MainTitle.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Logo:
			/* 로고 이미지 가운데 정렬
			rc=cur[rect];
			draw.drawImage( img.center(rc), img);
			*/
			rc=Class.rect(50,60,488,111), img=kioskImage('001');
			draw.drawImage( rc, img);
			/* 버전 텍스트 (2017-03-06) */
			setup=cf[SetupInfo];
			cf[kioskVersion]= "버전: $setup[kiosk_ver]";
			not( cf[kioskVersion] ) {
				cf[kioskVersion]="버전: 1.0.13";
			}
			rc=Class.rect(20,5,200,24);
			draw.font(8,'normal','#C46D57').text(rc, cf[kioskVersion] );
		case HomeButton:
			img=imageLoad(cur,'src');
			draw.drawImage(cur[rcButton], img );
		case LanguageSelect:
			not( cur[rect body] ) {
				rc=Class.rect(820,40,230,70);
				divideRect(cur, rc, '10,*,10', 'left,body,right');
				divideRect(cur, cur[rect body], '65,65,65,65', 'a,b,c,d');
			}
			draw.drawImage(cur[rect left], 	commonImage('lang01') );
			draw.drawImage(cur[rect body], commonImage('lang02') );
			draw.drawImage(cur[rect right], 	commonImage('lang03') );
			draw.drawImage(cur[rect a].center(46,32), commonImage('langKor') );
			draw.drawImage(cur[rect b].center(46,32), commonImage('langEng') );
			draw.drawImage(cur[rect c].center(46,32), commonImage('langCha') );
			draw.drawImage(cur[rect d].center(46,32), commonImage('langJpn') );
		default:
		}
	}
}
MainTitle.mouseDown(pos) {
	cur=this.findTag('HomeButton'), img=imageLoad(cur,'src');
	if( cur[rcButton].contains(pos) ) {
		this.mainControl().goHome(true);
	} else {
		cur=this.findTag('LanguageSelect');
		if( cur[SelectMode] ) {
			sel=true;
			if( cur[rect a].contains(pos) ) {
				Cf[KioskLangSelect]='Kor';
			} else if( cur[rect b].contains(pos) ) {
				Cf[KioskLangSelect]='Eng';
			} else if( cur[rect c].contains(pos) ) {
				Cf[KioskLangSelect]='Cha';
			} else if( cur[rect d].contains(pos) ) {
				Cf[KioskLangSelect]='Jpn';
			} else {
				sel=false;
			}
			if( sel ) {
				this.findControl('#CornerTab').makeDisplayTab();
				cart=this.findControl('MenuCart#orderView');
				cart.removeAllMenu();
				cart.drawHeader();
				if( cf[tabMode].eq('best') ) {
					this.findControl('#CornerTab').setBestTabNode();
				}
			}
			cur[SelectMode]=true;
		} else {
			rc=cur[rect body].center(52,36);
			if( rc.contains(pos) ) {
				cur[SelectMode]=true;
				this.mainControl().timelineStart('SlectLanguage', cur, 'Open');
			}
		}
	}
}
MainTitle.mouseUp(pos) {

}

CornerTab.CornerTab(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
CornerTab.initControl() {
	setNodeSize(tag, true);
}
CornerTab.conf() {
	if( cf[tabMode].eq('best') ) {
		tag.tabCount=4;
		divideRect(tag, tag[rect], "200, 60,*,60", "best, leftButton, tabs, rightButton" );
		tag[tabsRect]=tag[rect tabs];
		tag[bestRect]=tag[rect best].incrX(25).incrY(20);
		not( tag[tabBest] ) {
			tag[tabBest]="${cf[imagePath]}/type/tab_best_select.png";
			imageLoad(tag,'tabBest');
		}
		not( tag[tabBestNode] ) {
			tag[tabBestNode]={};
		}
	} else {
		tag.tabCount=5;
		divideRect(tag, tag[rect], "60,*,60", "leftButton, tabs, rightButton" );
		tag[tabsRect]=tag[rect tabs];
	}
	cf[CornerTabTag]=tag;
}
CornerTab.draw(draw,  timeline) {
	drawNodeStyle(draw, tag);
	/* 추천버튼 그리기 */
	if( cf[tabMode].eq('best') ) {
		rc=tag[bestRect];
		title=when( Cf[KioskLangSelect].eq('Kor'), "추천", "Best");
		if( tag[currentTabBest] ) {
			img=imageLoad(tag,'tabBest');
			draw.drawImage(rc, img);
			drawNodeText(draw, rc, title, 'center', 'TabSelect');
			rc=tag[rect leftButton], img=imageLoad(tag,'LeftButton', 'd');
			draw.drawImage(img.center(rc), img);
			tag.inject( currentTabBlock, currentTabIndex);
			tabDraw = tag[tabDrawNode].child( currentTabBlock );
			rc=tag[rect rightButton];
			if( tabDraw[endBlock] ) {
				type='d';
			} else {
				type=when( rc.eq(this.mouseDownRect), 'p', 'n');
			}
			img=imageLoad(tag,'RightButton', type);
			draw.drawImage(img.center(rc), img);
			draw.drawImage( tag[rect tabs], this.getTabDraw(block) );
			return;
		} else {
			drawNodeImage(draw, rc, tag, 'TabImage', 'nor');
			drawNodeText(draw, rc, title, 'center', 'TabNormal');
		}
	}
	/* 탭출력 영역이 없다면 리턴한다(makeDisplayTab에서 생성) */
	not( tag[tabDrawNode].childCount() ) {
		return;
	}
	/* 현재블록번호, 탭위치번호, 이전/탭/다음버튼 영역*/
	tag.inject( currentTabBlock, currentTabIndex);
	not( currentTabBlock ) currentTabBlock=0;
	/* 메모리에 그려논 탭정보를 찾는다 */
	tabDraw = tag[tabDrawNode].child( currentTabBlock );
	/* 왼쪽 버튼 */
	rc=tag[rect leftButton];
	if( currentTabBlock >0 ) {
		type=when( rc.eq(this.mouseDownRect), 'p', 'n');
	} else {
		type='d';
	}
	img=imageLoad(tag,'LeftButton', type);
	draw.drawImage(img.center(rc), img);
	/* 오른쪽 버튼*/
	rc=tag[rect rightButton];
	if( tabDraw[endBlock] ) {
		type='d';
	} else {
		type=when( rc.eq(this.mouseDownRect), 'p', 'n');
	}
	img=imageLoad(tag,'RightButton', type);
	draw.drawImage(img.center(rc), img);
	if( timeline ) {
		style=timeline[timelineStyle];
		if( Cf.timeLine('ShiftMenu.running') ) {
			frame=Cf.timeLine('ShiftMenu.current');
			switch( style ) {
			case ExpandLeft:
				this.drawExpand( draw, tabDraw, frame, true);
			case ExpandRight:
				this.drawExpand( draw, tabDraw, frame);
			case NextBlock:
				this.drawBlock( draw, frame, rarr, true);
			case PrevBlock:
				this.drawBlock( draw, frame, rarr);
			}
		} else {
			block=false;
			switch( style ) {
			case NextBlock:
				tag[currentTabBlock++];
				tag[currentTabIndex]=0;
				block=true;
			case PrevBlock:
				idx=tag[tabCount]-1;
				tag[currentTabBlock--];
				tag[currentTabIndex]=idx;
				block=true;
			default:
				this.drawTab(draw);
			}
			if( block ) {
				tab=this.getTabNode();
				this.currentTabChange(tab );
			}
		}
	} else {
		this.drawTab(draw);
	}
}
CornerTab.mouseDown(pos) {
	tag.inject( currentTabBlock, currentTabIndex );
	not( currentTabBlock ) currentTabBlock=0;
	tabDraw = tag[tabDrawNode].child( currentTabBlock );
	this.mouseDownRect=null;
	/* 추천 */
	if( cf[tabMode].eq('best') ) {
		if( tag[bestRect].contains(pos) ) {
			this.setBestTabNode();
		}
	}
	/* 이전 버튼(화살표) */
	if( tag[rect leftButton].contains(pos) ) {
		if( currentTabBlock>0 ) {
			tag[currentTabBest]=false;
			this.mainControl().timelineStart('ShiftMenu', tag, 'PrevBlock');
			this.mouseDownRect=tag[rect leftButton];
			this.update();
		}
		return;
	}
	/* 다음 버튼(화살표) */
	else if( tag[rect rightButton].contains(pos) ) {
		not(tabDraw[endBlock]) {
			tag[currentTabBest]=false;
			this.mainControl().timelineStart('ShiftMenu', tag, 'NextBlock');
			this.mouseDownRect=tag[rect rightButton];
			this.update();
		}
		return;
	}
	/* 코너 */
	else{
		while( rc, tag[TabRectArray], idx, 0 ) {
			not( rc.contains(pos) ) continue;
			tab = this.getTabNode(currentTabBlock, idx);
			if( tab ) {
				tag[currentTabBest]=false;
				tag[currentTabIndex]=idx;
				this.currentTabChange(tab);
				return;
			}
		}
	}
}
CornerTab.mouseUp(pos) {
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
CornerTab.reload() {
	db.fetchAll(conf('sql#hitec.cornerTab'), tag.removeAll());
	while( cur, tag ) {
		db.fetchAll(conf('sql#hitec.menuList'), cur.removeAll() );
	}
	_log("Corner Tab Reload=> 메뉴탭 갯수 : $tag.childCount()");
	this.mainControl().workerAdd( KIOSK.CornerMenuReload, tag );
}
CornerTab.loadMenus() {
	cf.inject(imagePath);
	menuPath="$imagePath/menus";
	num=0;
	while( tabs, tag ) {
		while( menu, tabs ) {
			menu[src]="$menuPath/$menu[goods_img]";
			imageLoad(menu, 'src');
			num++;
		}
	}
	_log("loadMenus : 메뉴 로딩 완료 (메뉴 $num 개 로딩)");
}
CornerTab.test() {
	cf[tabMode]='best';
	tag[tabDrawNode]=null;
	this.conf();
	this.makeDisplayTab();
}
CornerTab.makeDisplayTab(isHome, goMain) {
	displayArray	=_arr(tag,'DisplayTabArray', true);
	while( tab, tag, n, 0 ) {
		cur=displayArray.add(tab);
		tab[displayTabIndex]=n;
	}
	totalCnt=displayArray.size();
	not( 	totalCnt ) {
		print("makeDisplayTab 탭갯수 오류 : $totalCnt");
		return;
	}
	rectArray =_arr(tag,'TabRectArray', true);
	tabDrawNode=tag[tabDrawNode];
	blockCnt=totalCnt/tag.tabCount, mod=totalCnt%tag.tabCount;
	if( mod ) blockCnt++;
	rcTabs=tag[rect tabs];
	not( rcTabs ) return;
	_log("탭그리기 영역 => $rcTabs");
	sw=rcTabs.width(), sh=rcTabs.height();
	/* 탭을 블럭 단위로 그리기 위한 객체생성 */
	if( tabDrawNode ) {
		if( blockCnt > tabDrawNode.childCount() ) {
			dist= blockCnt - tabDrawNode.childCount();
			while( num, dist ) {
				tabDraw=tabDrawNode.addNode();
				tabDraw[drawObject]=Class.draw(sw, sh);
			}
			_log("makeDisplayTab: 탭블럭 노드추가 : ($dist 개 추가)");
		} else {
			_log("makeDisplayTab: 탭블럭 변경사항 없음");
		}
	} else {
		tabDrawNode={};
		while( num, blockCnt ) {
			tabDraw=tabDrawNode.addNode();
			tabDraw[drawObject]=Class.draw(sw, sh);
		}
		tag[tabDrawNode]=tabDrawNode;
		_log("makeDisplayTab: 탭블럭 노드생성( $tabDrawNode.childCount() 개 생성)");
	}
	/* 탭블럭 초기화 & draw */
	rcTabs.lt().inject(rx, ry);
	st=0;
	sx=0, sy=24, ry+=24;
	sw=180, sh=90, space=10;
	first=true;
	lang=Cf[KioskLangSelect].lower();
	while( tabDraw, tabDrawNode, num, 0 ) {
		et= min( st+tag.tabCount, totalCnt  );
		tabDraw[startTab]=st, tabDraw[endTab]=et;
		tabDraw[endBlock]=when( et.eq(totalCnt), true, false );
		do=tabDraw[drawObject];
		do.fill();
		sx=0;
		_log("tabBlock $num=($st, $et) rectArray=$rectArray");
		while( n, et, st ) {
			tab=displayArray.get(n);
			rc=Class.rect(sx, sy, sw, sh ), sx+=sw+space;
			drawNodeImage(do, rc, tag, 'TabImage', 'nor');
			drawNodeText(do, rc, tab[$lang], 'center', 'TabNormal');
			/* 마우스 클릭처리를 위해 탭영역 배열추가*/
			if( first ) {
				rectArray.add( Class.rect(rx, ry, sw, sh) );
				rx+=sw+space;
			}
		}
		first=false;
		if( tabDraw[endBlock]  ) {
			break;
		}
		st+=tag.tabCount;
	}
	_log("makeDisplayTab: 탭영역 그리기 완료");
	tag[currentTabBlock]=0;
	tag[currentTabIndex]=0;
	currentTab=null;
	if( cf[tabMode].eq('best') ) {
		currentTab=tag[tabBestNode];
		isHome=true;
		tag[currentTabBest]=true;
	} else {
		currentTab=when( goMain, tag[currentTab], this.getTabNode() );
	}
	print("makeDisplayTab: 탭출력 영역 세팅 : $tag[tabsRect], 탭:  $currentTab[corner_nm]");
	if( goMain ) {
		return;
	}
	this.currentTabChange(currentTab, isHome);
}
CornerTab.currentTabChange(tab, isHome, fromMain) {
	not( tab ) {
		_log("# currentTabChange : tab is not valid = $tab");
		return;
	}
	if( cf[cardButtonCheck] ) {
		cf[cardButtonCheck]=false;
	}
	if( isHome ) {
		tag[currentTab]=null;
		if( fromMain ) {
			isHome=false;
		}
	}
	if( tag[currentTab] ) {
		if( tab==tag[currentTab] ) {
			print("동일한 탭선택 : $tab[corner_nm]");
			return;
		}
	}
	tag[currentTab]=tab;
	tag[currentTabBest]=false;
	idx=tab[displayTabIndex];
	block=0;
	while( drawNode, tag[tabDrawNode] ) {
		if( drawNode[startTab]<=idx ) {
			if( idx<drawNode[endTab] ) {
				tag[currentTabBlock]=block;
				tag[currentTabIndex]=idx-drawNode[startTab];
				break;
			}
		}
		block++;
	}
	print("### currentTabChange.changeTab : (tab=$tab)");
	this.findControl('MenuList#menuView').changeTab(tab);
	/* 홈버튼 클릭시 탭이동 애니메이션 생략한다 */
	not( isHome ) {
		print("currentTabChange : (tab name=$tab[corner_nm])");
		this.mainControl().timelineStart('CornerTabChange', tab, 'FadeInOut');
		/* this.update(); */
	}
}
CornerTab.getTabNode(block, index) {
	not( isset(block) ) block=tag[currentTabBlock];
	not( isset(index) ) index=tag[currentTabIndex];
	tabIndex = block * tag.tabCount;
	tabIndex+= index;
	return tag[DisplayTabArray].get(tabIndex);
}
CornerTab.getTabDraw(block) {
	not( block ) block=tag[currentTabBlock];
	tabDraw = tag[tabDrawNode].child(block);
	return tabDraw[drawObject];
}
CornerTab.drawTab(draw, block, idx) {
	not( isset(block) ) 	block=tag[currentTabBlock];
	not( isset(idx) ) 	idx=tag[currentTabIndex];
	/* 메모리에서 화면으로 그려준다 */
	draw.drawImage( tag[rect tabs], this.getTabDraw(block) );
	/* 선택된 탭은 따로 그려준다 */
	rc=tag[rect];
	rect 		= tag[TabRectArray].get(idx);
	not( rect ) {
		print("drawTab rect is null : ($block, $idx)");
		this.makeDisplayTab(true);
		this.update();
		return;
	}
	tab		= this.getTabNode(block, idx);
	rect.bottom(rc.bottom());
	img=imageLoad(tag,'TabImage', 'select');
	draw.drawImage(rect, img);
	lang=Cf[KioskLangSelect].lower();
	drawNodeText(draw, rect, tab[$lang], 'center', 'TabSelect');
}
CornerTab.drawExpand(draw, tabDraw, frame, left) {
	darr=tag[DisplayTabArray];
	rarr=tag[TabRectArray];
	tabDraw.inject(startTab, endTab);
	tabCnt=endTab-startTab;
	if( left ) {
		/* 끝에서 왼쪽으로 펼쳐진다 */
		sp=tabCnt-1;
	} else {
		/* 시작부터 오른쪽으로 펼쳐진다 */
		sp=0;
	}
	sx=rarr.get(sp).x(), idx=0;
	lang=Cf[KioskLangSelect].lower();
	while( n, endTab, startTab ) {
		tab=darr.get(n);
		if( frame.eq(0) ) {
			cx=rarr.get(idx).x();
			if( left ) {
				dw=sx-cx;
			} else {
				dw=cx-sx;
			}
			/* 시작위치와 현재위치 폭을기준으로 배열생성 */
			_arr(tab,'RecalcArray').recalc(dw, 25);
		} else {
			/* 타임라인 만큼 이동처리 */
			dx=sx;
			if( left ) {
				dx-=tab[RecalcArray].sum(0,frame);
			} else {
				dx+=tab[RecalcArray].sum(0,frame);
			}
			rc=rarr.get(idx);
			rc.x(dx);
			img=imageLoad(tag,'TabImage', 'nor');
			draw.drawImage(rc, img);
			drawNodeText(draw, rc, tab[$lang], 'center', 'TabNormal');
		}
		idx++;
	}
}
CornerTab.drawBlock(draw, frame, rarr, next) {
	tag[rect tabs].inject(tx, ty, tw, th);
	/* 애니메이션을 위한 임시 드로우 객체 생성  */
	not( tag[blockDraw] ) {
		tag[blockDraw]=Class.draw(tw*2, th);
	}
	blockDraw = tag[blockDraw];
	if( frame.eq(0 ) ) {
		blockDraw.fill();
		rc=Class.rect(0,0,tw,th);
		rcRight=rc.move('right');
		if( next ) {
			nextBlock=tag[currentTabBlock ]+ 1;
			d1 = tag[tabDrawNode].child( tag[currentTabBlock] );
			d2 = tag[tabDrawNode].child( nextBlock );
			blockDraw.drawImage( rc, d1[drawObject] );
			blockDraw.drawImage( rcRight, d2[drawObject] );
		} else {
			prevBlock=tag[currentTabBlock] - 1;
			d1 = tag[tabDrawNode].child( prevBlock );
			d2 = tag[tabDrawNode].child( tag[currentTabBlock] );
			blockDraw.drawImage( rc, d1[drawObject] );
			blockDraw.drawImage( rcRight, d2[drawObject] );
		}
		_arr(tag,'BlockRecalcArray').recalc(tw, 25);
	} else {
		if( next ) {
			sx=0;
			sx+=tag[BlockRecalcArray].sum(0,frame);
		} else {
			sx=tw;
			sx-=tag[BlockRecalcArray].sum(0,frame);
		}
		draw.drawImage( tag[tabsRect], blockDraw, sx, 0 );
	}
}

mainCanvas.qtMonSendData(send) {
	node=_node('QtMonNode');
	socket=node[socket];
	not( socket ) {
		socket=Class.socket("QtMon");
		node[socket]=socket;
		node[socketStartTick]=System.tick();
	}
	not( socket.isConnect() ) {
		not( socket.connect( cf.qtMonHost, cf.qtMonPort) ) {
			_log("## QtMon connect error host:$cf[qtMonHost], port: $cf[qtMonPort] ");
			return;
		}
	}
	not( send ) return;
	s='$';
	s.add(send);
	socket.sendBuffer("$s\n");
	print("QtMon 전송내용 : $s");
}
mainCanvas.qtMonRecvData(recv) {
	not( recv.ch().eq('$') ) {
		_log("QtMon 결과가 유효한 형식이 아닙니다 : 응답결과=$recv");
	}
	val=recv.value(1);
	switch( val.findPos(',').trim() ) {
	case 21: 	/* 인쇄 */
		print("qtMon 영수증 인쇄 응답 => $val");
		val.split().inject( reqType, subCode, paramCount, printKind, papper, printHeadUp, jam, papperCheck  );
		if( subCode.eq('02') ) {
			msg='', err=false, errorCode=null;
			if( printKind.eq(0) ) {
				msg.add("영수증 프린터 : ");
			} else {
				msg.add("주방 프린터 $printKind : ");
			}
			if( papperCheck.eq('1') ) {
				msg.add("용지 부족 ");
				err=true;
				errorCode='05';
			}
			if( printHeadUp.eq('1') ) {
				this.alert("<font size=18>프린터 헤더 열림 오류가 발생했습니다</font>",'오류', true);
			}
			if( papper.eq('1') ||  jam.eq('1') ) {
				if( papper.eq('1') ) msg.add("용지 없음");
				if( jam.eq('1') ) msg.add("용지 걸림");
				msg.add(" 오류가 발생했습니다");
				err=true;
				errorCode='02';
			}
			if( err ) {
				not( errorCode ) errorCode='01';
				cf[printErrorTick]=System.tick();
				kiosk_SendError(this, msg, errorCode, cf, db );
			} else if( cf[printErrorTick] ) {
			}
		} else if( subCode.eq('04') ) {
			success=printKind;
			if( success.eq('0') ) {
				kiosk_SendError(this, '영수증 출력 오류가 발생했습니다', '01', cf, db );
				return;
			}
		}
	case 22:
		print("qtMon 주방프린터 응답 => $val");
		val.split().inject( reqType, paramCount,  success );
		if( success.eq(0)){
			kiosk_SendError(this,'주방프린터 출력 오류', '09', cf, db );
			return;
		}
	default:
		print("qtMon 기타 응답 => $val");
	}
	print("xxxxxxxxxxxxxx qtMonRecvData xxxxxxxxxxxxxxxxxx");
}
mainCanvas.popupCloseCheck() {
	if( cf[errorOpen] ) {
		return;
	}
	not( cf[popupControl] ) {
		order=cf[OrderHeader];
		if( order[total_qty] ) {
			return;
		}
		dist=System.tick() - cf[mouseDownTick];
		if( dist>55000 ) {
			cf[mouseDownTick]=System.tick();
			not( cf[stackPage] ) {
				this.goHome(true);
			}
		}
		return;
	}
	dist = System.tick() - cf[popupStartTick];
	popup=cf[popupControl].getMainNode();
	not( popup[tag] ) {
		cf[popupControl]=null;
		this.update();
		return;
	}
	switch( popup[tag] ) {
	case OrderConfirm:
		if( dist > 35000 ) {
			this.popupClose();
		}
	case Loading:
		if( dist > 40000 ) {
			this.popupClose();
		}
	case MessageWindow:
		if( cf[orderMessage] ) {
			if( dist > 35000 ) {
				cf[orderMessage]=null;
				this.popupClose();
			}
		} else if( dist > 10000 ) {
			this.popupClose();
		}
	case CompleteOrder:
		if( dist > 2500 ) {
			order=cf[OrderHeader];
			_log("# 주문 완료 : $order ");
			msg=order[orderMessage];
			this.popupClose();
			cf[OrderHeader].initNode();
			cart=this.findControl('MenuCart#orderView');
			cart.removeAllMenu();
			btns=this.findControl('MainStatus#buttonsView');
			btns[mouseDownRect]=null;
			this.goHome(true);
			if( msg ) {
				_log("# 주문 완료 오류메시지 : $msg ");
				cf[orderMessage]=msg;
				if( msg.find("\n") ) {
					this.alert(msg,"주문전송 오류");
				} else {
					this.alert("$msg \n\n매장에 상품 교환권을 제출하세요", "매장에 주문 전송이 안되었습니다.");
				}
			}
		}
	default:
		dist=System.tick() - cf[mouseDownTick];
		if( dist>60000 ) {
			cf[mouseDownTick]=System.tick();
			this.goHome(true);
		}
	}
}
mainCanvas.qtMonConnectCheck() {
	return;
	this.qtMonSendData();
}
mainCanvas.orderCheck() {
	order=cf[OrderHeader];
	/* 현금입력 완료되면 일정시간후에 현금영수증 선택창으로 이동 */
	if( order[InputCashOk] ) {
		if( order[DelayCount++].eq(1) ) {
			this.openPopup('SelectCashReceipt');
		}
	}
	/* 주문 처리중이라면 */
	if( order[PayType] ) return return true;
	return false;
}
mainCanvas.goHome(flag) {
	if( cf[errorOpen] ) return;
	cf[cardButtonCheck]=false;
	this.popupClose();
	cf[OrderHeader].initNode();
	if( flag ) {
		this.reloadCornerTab();
	}
	check=false;
	arr=cf[ActionRects];
	rc=arr[1];
	if( rc ) {
		x=rc.x();
		if( x<980 ) {
			check = true;
		}
	} else {
		check=true;
	}
	if( check ) {
		not( typeof( cf[ActionRects],'array') ) {
			cf[ActionRects]=[];
		}
		cf[ActionRects].reuse();
		cf[ActionRects].add( Class.rect(0,0,100,100) );
		cf[ActionRects].add( Class.rect(980,0,100,100) );
	}
	prevLang=Cf[KioskLangSelect];
	Cf[KioskLangSelect]='Kor';
	/* 추천 처리 */
	check=false;
	val=db.value("SELECT set_val FROM hitec_m06s where set_cd='006' ");
	if( val.eq('1') ) {
		not( cf[tabMode].eq('best') ) {
			this.findControl('#CornerTab').setTabMode('best');
			check = true;
		}
	} else {
		if( cf[tabMode].eq('best') ) {
			this.findControl('#CornerTab').setTabMode('normal');
			check = true;
		}
	}
	if( check ) {
		this.findControl('#CornerTab').makeDisplayTab(true, flag);
	}
	cart=this.findControl('MenuCart#orderView');
	cart.removeAllMenu();
	not( prevLang.eq('Kor') ) {
		not( check ) {
			this.findControl('#CornerTab').makeDisplayTab(true, flag);
		}
		cf[lastLogSeq]=0;
		cart.drawHeader();
	}
	btns=this.findControl('MainStatus#buttonsView');
	btns[mouseDownRect]=null;
	if( flag && cf[mainPageUse] ) {
		db=Class.db('kiosk_hitec');
		db.fetch("SELECT max(open_date) as open_date FROM kiosk_open_close", cf);
		this.confMain();
		this.pageOpen('MainPage');
		return;
	}
	if( cf[tabMode].eq('best') ) {
		this.findControl('#CornerTab').setBestTabNode();
	}
}


MenuList.MenuList(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MenuList.initControl() {
	setNodeSize(tag, true);
}
MenuList.conf() {
	confNodeLayout(tag);
	while( cur, tag ) this.getControl(cur).conf();
}
MenuList.draw(draw, tm) {
	while( cur, tag ) this.getControl(cur).draw(draw, tm);
}
MenuList.mouseDown(pos) {
	while( cur, tag ) this.getControl(cur).mouseDown(pos);
}
MenuList.mouseUp(pos) {
	while( cur, tag ) this.getControl(cur).mouseUp(pos);
}

MenuCart.MenuCart(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MenuCart.initControl() {
	this.setCurrentPage('AdPanel');
}
MenuCart.conf() {
	rc=tag[rect];
	while( cur, tag ) {
		cur[rect]=rc;
		this.getControl(cur).conf();
	}
}
MenuCart.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	page=this.getControl( tag[CurrentPage] );
	if( page ) page.draw(draw, tm);
}
MenuCart.mouseDown(pos) {
	page=this.getControl( tag[CurrentPage] );
	if( page ) page.mouseDown(pos);
}
MenuCart.mouseUp(pos) {
	page=this.getControl( tag[CurrentPage] );
	if( page ) page.mouseUp(pos);
}
MenuCart.setCurrentPage(code) {
	tag[CurrentPage]=this.findTag(code);
	if( code.eq('AdPanel') ) {
		this.mainControl().timelineStart('ShiftMenu', this, 'ExpandLeft');
	} else {
		this.update();
	}
}

ShoppingCart.ShoppingCart(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
ShoppingCart.initControl() {
	not( tag[type] ) tag[type]='vbox';
	tag.removeAll();
	tag.addNode({tag:Header, Height:72});
	tag.addNode({tag:List} );
	setNodeSize(tag, true);
	this[OrderTotalQty]		=0;
	this[OrderTotalPrice]		=0;
	this[currentPageBlock]	=0;
	this[rowCount] 				=5;
	_node(this,'pageNode');
	_arr(this,'HeaderWidthArray');
	_arr(this,'QtyRateArray');
	_arr(this,'PriceRateArray');
	_arr(this,'ScrollHeightArray');
	_arr(this,'ScrollRateArray');

}
	ShoppingCart.conf() {
	confNodeLayout(tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case Header:
			cur[rect].inject(sx,sy, sw, sh);
			cur.removeAll();
			arr=this[HeaderWidthArray].recalc( sw, '486,168,250,176', true);
			while( w, arr, c, 0 ) {
				sub=cur.addNode({tag:HeaderNode});
				sub[rect]=Class.rect(sx, sy, w, sh);
				switch( c) {
				case 1:	this[QtyRateArray].recalc(w,'55,*,55', true);
				case 2:	this[PriceRateArray].recalc(w,'*,55', true);
				}
			}
			this.drawHeader(cur);
		case List:
			cur[rect].inject(x,y,w,h);
			this[ListHeight]=h/5;
			this[ScrollHeightArray].recalc( h, '200,20,200', true);
			this[ScrollRateArray].recalc(h, 30);
			this[rcList]=Class.rect(x,y,w-170,h), listRightTop=this[rcList].rt();
			this[rcScrollBar]=Class.rect( listRightTop, 170, h);
			getDrawObject(this, 'ListBackgroundDC', w, h*2);
		default:
		}
	}
}
ShoppingCart.draw(draw,  timeline) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Header:
			draw.drawImage( cur[rect], getDrawObject(this, 'HeaderDrawObject') );
		case List:
			page=this.getPage();
			not( page ) return;
			this.inject( rcList, rcScrollBar);
			draw.fill(cur[rect],'#ffffff');
			isAnimation=false;
			if( timeline ) {
				if( timeline[tid].eq('ShoppingCart') && Cf.timeLine('ShoppingCart.running') ) {
					isAnimation=true;
				}
			}
				if( isAnimation  ) {
			/* 애니메이션 모드일 경우*/
				frame=Cf.timeLine('ShoppingCart.current');
				style=timeline[timelineStyle];
				switch( style ) {
				case [ScrollUp,ScrollDown] :
					block=this[currentPageBlock];
					if( frame >18 ) {
						Cf.timeLine('ShoppingCart.stop');
						if( style.eq('ScrollUp') ) {
							this[currentPageBlock]=block-1;
						} else {
							this[currentPageBlock]=block+1;
						}
						this.drawScrollBar(cur);
						this.update();
						return;
					}
					dc=getDrawObject(this, 'ListBackgroundDC');
					if( style.eq('ScrollUp') ) {
						startY=rcList.height();
						if( frame.eq(0) ) {
							dc.fill();
							rc=Class.rect(0,0,rcList.size());
							prev=this.getPage(block-1);
							dc.drawImage(rc,prev[drawObject]);
							dc.drawImage(rc.move('down'),page[drawObject]);
						} else {
							 sy=frame*19;
							 startY-=sy;
						}
					} else {
						startY=0;
						if( frame.eq(0) ) {
							dc.fill();
							rc=Class.rect(0,0,rcList.size());
							next=this.getPage(block+1);
							dc.drawImage(rc,page[drawObject]);
							dc.drawImage(rc.move('down'),next[drawObject]);
						} else {
							 sy=frame*19;
							 startY+=sy;
						}
					}
					draw.drawImage( rcList, dc, 0, startY);
				case [QtyMinus,QtyPlus] :
					draw.drawImage( rcList, page[drawObject]);
					row=this[modifyRow];
					draw.fill(row[rcQty].incr(2), '#ffffff');
					if( frame.eq(0) ) {
						if( style.eq('QtyMinus') ) {
							if( row[qty]>1 ) {
								row[qty--];
							}
						} else {
							row[qty++];
						}
					} else {
						qty=row[qty];
						fontSize=20;
						fontSize+=frame;
						if( frame>10 ) {
							Cf.timeLine('ShoppingCart.stop');
							this.drawListPage(cur, page);
							this.recalcList(cur);
						}
						drawNodeText(draw, row[rcQty], qty, 'center', fontSize, 'bold', "#505050" );
					}
				case RowDelete:
					draw.drawImage( rcList, page[drawObject]);
					row=this[modifyRow];
					if( frame<15 ) {
						opa=20;
						opa+=frame*10;
						draw.opacity(opa);
						draw.fill(row[rect], '#ffffff');
						draw.opacity(100);
					} else {
						Cf.timeLine('ShoppingCart.stop');
						this.removeMenu(row);
					}
				default:
				}
			} else {
			/* 애니메이션 모드가 아닐때(수량 삭제버튼 처리) */
				draw.drawImage( rcList, page[drawObject]);
				rcDown=this.mouseDownRect;
				if( this.mouseDownRow ) {
					row=this.mouseDownRow;
					row.inject(rcQtyMinus, rcQtyPlus, rcRowDelete);
					if( rcQtyMinus.eq(rcDown) ) {
						drawNodeImage(draw, rcDown, tag, 'OrderMinusImage', 'p' );
					} else if( rcQtyPlus.eq(rcDown) ) {
						drawNodeImage(draw, rcDown, tag, 'OrderPlusImage', 'p' );
					} else if( rcRowDelete.eq(rcDown) ) {
						drawNodeImage(draw, rcDown, tag, 'OrderDeleteImage', 'p' );
					}
				} else if( this.mouseDownRect ) {
					this.inject(rcScrollUp, rcScrollDown);
					if( rcScrollUp.eq(rcDown) ) {
						drawNodeImage(draw, rcDown, tag, 'ScrollUp', 'p' );
					} else if( rcScrollDown.eq(rcDown) ) {
						drawNodeImage(draw, rcDown, tag, 'ScrollDown', 'p' );
					}
				}
			}
			if( this.currentRow ) {
				selectRow=this[currentRow];
				if( selectRow[rect] ) draw.fill(selectRow[rect], '#dac0ba1a');
			}
			/* 스크롤 버튼 그리기 */
			draw.drawImage( rcScrollBar,getDrawObject(this, 'ScrollDrawObject') );
			draw.rectLine(cur[rect], 134, '#6A4000');
		default:
		}
	}
}
ShoppingCart.mouseDown(pos) {
	this.inject(rcScrollUp, rcScrollDown, currentPageBlock, pageBlockCount);
	/* 마우스 클릭영역 초기화 */
	this.mouseDownRect=null;
	this.mouseDownRow=null;
	/* 마우스 영역 처리 */
	if( rcScrollUp.contains(pos) ) {
		if( currentPageBlock >0 ) {
			this.mouseDownRect=rcScrollUp;
		}
	} else if( rcScrollDown.contains(pos) ) {
		last= pageBlockCount-1;
		if( currentPageBlock<last ) {
			this.mouseDownRect=rcScrollDown;
		}
	} else {
		list=findTag('List', tag);
		page=this.getPage();
		page.inject(startRow, endRow);
		while( n, endRow, startRow ) {
			row=list.child(n);
			not( row[rect].contains(pos) ) continue;
			row.inject(rcQtyMinus, rcQtyPlus, rcRowDelete);
			if( rcQtyMinus.contains(pos) ) {
				if( row[qty]>1 ) {
					this.mouseDownRect=rcQtyMinus;
				} else {
					return;
				}
			} else if( rcQtyPlus.contains(pos) ) {
				this.mouseDownRect=rcQtyPlus;
			} else if( rcRowDelete.contains(pos) ) {
				this.mouseDownRect=rcRowDelete;
			}
			if( this.mouseDownRect )
				this.mouseDownRow=row;
			break;
		}
	}
	/* 마우스 영역이 있다면 다시 그리기 */
	if( this.mouseDownRect ) {
		this.update();
	}
}
ShoppingCart.mouseUp(pos) {
	not( this.mouseDownRect ) {
		return;
	}
	page=this.getPage();
	page.inject(startRow, endRow, endPage);
	this.inject(rcScrollUp, rcScrollDown);
	if( rcScrollUp.contains(pos) ) {
		this.currentRow=null;
		this.mainControl().timelineStart('ShoppingCart', node, 'ScrollUp');
	} else if( rcScrollDown.contains(pos) ) {
		this.currentRow=null;
		this.mainControl().timelineStart('ShoppingCart', node, 'ScrollDown');
	} else if( this.mouseDownRow ) {
		row=this.mouseDownRow;
		if( row[rect].contains(pos) ) {
			row.inject(rcQtyMinus, rcQtyPlus, rcRowDelete);
			/* 변경된 주문을 저장한후 애니메이션 끝난후 초기화 해준다 */
			this.modifyRow=row;
			if( rcQtyMinus.contains(pos) ) {
				this.mainControl().timelineStart('ShoppingCart', node, 'QtyMinus');
			} else if( rcQtyPlus.contains(pos) ) {
				this.mainControl().timelineStart('ShoppingCart', node, 'QtyPlus');
			} else if( rcRowDelete.contains(pos) ) {
				this.mainControl().timelineStart('ShoppingCart', node, 'RowDelete');
			}
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.mouseDownRow=null;
		this.update();
	}
}

AdPanel.AdPanel(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
AdPanel.initControl() {
	tag[RateArray]=[];
	tag.removeAll();
	/* 타이틀은 삭제한다.
	tag.addNode({tag:AdTitle, Height:72});
	*/
	tag.addNode({tag:AdImage});
	setNodeSize(tag, true);

}
	AdPanel.conf() {
	confNodeLayout(tag);
	not( tag[DrawImage] ) {
		cur=this.findTag('AdImage');
		tag[DrawImage]=Class.draw(cur[rect]);
		this.initAdNode();
	}
}
AdPanel.draw(draw, timeline) {
	while( cur, tag ) {
		rc=cur[rect];
		switch( cur[tag] ) {
		case AdTitle:
			setFont(draw, 32, '#373739');
			draw.drawImage(rc, imageLoad(tag,'TitleImage') );
			draw.rectLine(rc,24,'#A5670A');
			draw.text( rc.incrX(40), tag[TitleText]);
		case AdImage:
			ok=true;
			if( timeline ) {
				style=timeline[timelineStyle];
				if( style.eq('ExpandLeft','ExpandRight'), Cf.timeLine('ShiftMenu.running') ) {
					frame= Cf.timeLine('ShiftMenu.current');
					if( frame.eq(0) ) {
						_arr(tag,'MoveRateArray').recalc(cur[rect].width(),20);
					} else {
						dx=tag[MoveRateArray].sum(0,frame);
						x=cur[rect].width() - dx;
						draw.drawImage(cur[rect].x(x), kioskImage('003') );
					}
					ok=false;
				}
			}
			if( ok ) {
				draw.drawImage(cur[rect], kioskImage('003') );
			}
		default:
		}
	}
}
AdPanel.mouseDown(pos) {

}
AdPanel.mouseUp(pos) {

}

MainStatus.MainStatus(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MainStatus.initControl() {
	tab[type]='hbox';
	setNodeSize(tag, true);
}
MainStatus.conf() {
	confNodeLayout(tag);
	while( cur, tag ) this.getControl(cur).conf();
}
MainStatus.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) this.getControl(cur).draw(draw, tm);
}
MainStatus.mouseDown(pos) {
	while( cur, tag ) this.getControl(cur).mouseDown(pos);
}
MainStatus.mouseUp(pos) {
	while( cur, tag ) this.getControl(cur).mouseUp(pos);
}

ListBox.ListBox(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
ListBox.initControl() {
	rowCnt=3, cellCnt=4;
	/*
	tag[FontText]=[24,#303030,bold];
	tag[FontDesc]=[16,#a0a0a0];
	tag[FontPrice]=[18,#909090];
	tag[FontPrice1]=[26,#503090];
	*/
	tag[currentTab]=null;
	tag.put( rowCnt, cellCnt );
}
ListBox.conf() {
	cornerTag=cf[CornerTabTag];
	/* 추천 탭 */
	if( cf[tabMode].eq('best') && cornerTag[currentTabBest] ) {
		tab=cornerTag[tabBestNode];
		this.changeTab(tab, true);
	} else {
		tab=tag[currentTab];
	}
	not( tab ) return;
	print("상품 리스트 박스 view_type: $cf[view_type], pageCnt: $tab[pageCnt]");
	if( cf[view_type].eq('A') ) {
		this.makeThumbRect();
	} else {
		this.makeListRect();
	}
	print("리스트 영역생성: $tab[gridCnt] "):
	this.drawPage(tab);
}
ListBox.draw(draw, timeline) {
	page=this.getPage();
	not( page  ) {
		return;
	}
	if( timeline ) {
		if( Cf.timeLine("CornerTabChange.running") ) {
			frame=Cf.timeLine("CornerTabChange.current");
			if( tag[prevTab] ) {
				if( frame.eq(0) ) {
					tprev=tag[prevTab.displayTabIndex], tcur=tag[currentTab.displayTabIndex];
					tc=abs(tprev-tcur);
					/* 인접한 탭인경우 슬라이드효과 else 투명효과*/
					if( tc.eq(1) ) {
						drawObject=this[SlideDrawObject];
						tag[rect].inject(x,y,w,h);
						not( drawObject ) {
							ww=w*2;
							print("ListBox draw => SlideDrawObject : $ww, $h");
							drawObject=Class.draw( ww, h );
							this[SlideDrawObject]=drawObject;
						}
						rc=Class.rect(0,0,w,h);
						prev=this.getPage( tag[prevTab] );
						drawObject.fill();
						if( tprev<tcur ) {
							drawObject.drawImage(rc, prev.drawObject);
							drawObject.drawImage(rc.move('right'), page.drawObject);
							timeline[mode]='SlideRight';
						} else {
							drawObject.drawImage(rc, page.drawObject);
							drawObject.drawImage(rc.move('right'), prev.drawObject);
							timeline[mode]='SlideLeft';
						}
						_arr(this,'RecalcArray').recalc(w, 20);
					} else {
						timeline[mode]='FadeInOut';
					}
				}
				drawObject=this[SlideDrawObject];
				switch(timeline[mode]) {
				case SlideRight:
					sx=0;
					sx+=this[RecalcArray].sum(0,frame);
					draw.drawImage(tag[rect], drawObject, sx, 0);
					return;
				case SlideLeft:
					sx=tag[rect].width();
					sx-=this[RecalcArray].sum(0,frame);
					draw.drawImage(tag[rect], drawObject, sx, 0);
					return;
				case FadeInOut:
					prev=this.getPage( tag[prevTab] );
					in=frame * 5;
					out=100-in;
					draw.opacity(out);
					draw.drawImage( tag[rect], prev.drawObject );
					draw.opacity(in);
					draw.drawImage( tag[rect], page.drawObject );
					draw.opacity(100);
				default:
				}
			} else {
				opa=frame * 5;
				draw.opacity(opa);
				draw.drawImage( tag[rect], page.drawObject );
			}
		} else if( Cf.timeLine("SelectMenu.running") ) {
			frame=Cf.timeLine("SelectMenu.current");
			draw.drawImage( tag[rect], page.drawObject );
			menu=this[currentSelectMenu];
			if( frame.eq(0) ) {
				offset=tag[rect].lt();
				arr=_arr(tag,'SelectMenuPoints').reuse();
				rcMenu=menu[rcImage].move(offset, true);
				arr.add(rcMenu.lt() );
				arr.add(Class.point(500,10) );
				arr.add(Class.point(350, 5000));
				a=_arr(tag,'SelectMenuPaths').recalc(arr, 40);
			} else {
				paths=tag[SelectMenuPaths];
				menu[rcImage].inject(x,y,w,h);
				in=frame * 3;
				out=100-in;
				w-=in, h-=in;
				rc=Class.rect(paths.get(frame), w, h), img=loadMenu(cf, menu);
				not( img ) {
					img=commonImage('no_img');
				}
				if( rc ) {
					draw.opacity(out);
					draw.drawImage(img.center(rc), img);
					draw.opacity(100);
				} else {
					Cf.timeLine('SelectMenu.stop');
				}
			}
		} else {
			draw.opacity(100);
			draw.drawImage( tag[rect], page.drawObject );
		}
	} else {
		draw.drawImage( tag[rect], page.drawObject );
	}
	if( this.menuDrawCheck ) {
		offset=tag[rect].lt(), menu=this.mouseDownMenu;
		rc=menu[rcImage].incrXY(offset), img=loadMenu(cf, menu);
		draw.drawImage(rc.incr(-10), img);
		this.menuDrawCheck=false;
	}
}
ListBox.mouseDown(pos) {
	tab= tag[currentTab];
	tab.inject(gridCnt, currentPage);
	sp=gridCnt*currentPage, ep=sp+gridCnt;
	while( n, ep, sp ) {
		menu=tab.child(n);
		not( menu ) break;
		not( menu[rect].contains(pos) ) continue;
		not( menu[menu_cd] ) return;
		if( menu[sold_yn].eq('Y') || not(menu[sale_ok]) ) return;
		this.mouseDownMenu=menu;
		this.menuDrawCheck=true;
		this.update();
		break;
	}
}
ListBox.mouseUp(pos) {
	if( this.menuDrawCheck ) {
		this.update();
	}
	tab= tag[currentTab];
	tab.inject(gridCnt,  currentPage);
	sp=gridCnt*currentPage;
	ep=sp+gridCnt;
	while( n, ep, sp ) {
		menu=tab.child(n);
		not( menu ) break;
		not( menu[rect].contains(pos) ) continue;
		if( this.mouseDownMenu==menu ) {
			this.selectMenu(menu);
			return;
		}
		break;
	}
	if( this.mouseDownMenu ) {
		this.update();
	}
}

PagePanel.PagePanel(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
PagePanel.initControl() {
	tag[type]='hbox';
	tag.addNode({tag:PrevButton, Width:126} );
	tag.addNode({tag:PageNai} );
	tag.addNode({tag:NextButton, Width:126} );
	setNodeSize(tag, true);
}
PagePanel.conf() {
	confNodeLayout(tag);
}
PagePanel.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	tab=this[currentTab];
	not( tab ) return;
	tab.inject(currentPage, pageNode);
	total=pageNode.childCount();
	while( cur, tag ) {
		switch( cur[tag] ) {
		case PrevButton:
			if( currentPage>0 ) {
				type = when( cur[rect].eq(this.mouseDownRect), 'p', 'n' );
			} else {
				type = 'd';
			}
			rc=cur[rect], img=imageLoad(tag,'PrevImage', type);
			draw.drawImage(img.center(rc), img);
		case PageNai:
			this[pageNaviRect].inject(sx, sy );
			while( n, total ) {
				type=when( n.eq(currentPage), 'on', 'off');
				rc=Class.rect(sx, sy, 55, 55 ), img=imageLoad( tag, 'PageImage', type);
				draw.drawImage(img.center(rc), img);
				sx+=60;
			}
		case NextButton:
			next=currentPage+1;
			if( next<total ) {
				type = when( cur[rect].eq(this.mouseDownRect), 'p', 'n' );
			} else {
				type = 'd';
			}
			rc=cur[rect], img=imageLoad(tag,'NextImage', type);
			draw.drawImage(img.center(rc), img);
		default:
		}
	}
}
PagePanel.mouseDown(pos) {
	tab=this[currentTab];
	tab.inject(currentPage, pageNode);
	total=pageNode.childCount();
	while( cur, tag ) {
		switch( cur[tag] ) {
		case PrevButton:
			if( currentPage<=0 ) {
				if( currentPage<0 ) tab[currentPage]=0;
				continue;
			}
			if( cur[rect].contains(pos) ) {
				this.mouseDownRect = cur[rect];
				break;
			}
		case PageNai:
			if( this[pageNaviRect].contains(pos) ) {
				this.mouseDownRect = this[pageNaviRect];
				break;
			}
		case NextButton:
			last=total-1;
			if( last<=currentPage ) {
				if( last<currentPage ) tab[currentPage]=last;
				return;
			}
			if( cur[rect].contains(pos) ) {
				this.mouseDownRect = cur[rect];
				break;
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
PagePanel.mouseUp(pos) {
	tab=this[currentTab];
	tab.inject(currentPage, pageNode);
	total=pageNode.childCount();
	while( cur, tag ) {
		switch( cur[tag] ) {
		case PrevButton:
			if( cur[rect].contains(pos) ) {
				if( cur[rect].eq(this.mouseDownRect) ) {
					tab[currentPage--];
				}
			}
		case PageNai:
			this[pageNaviRect].inject(sx, sy );
			while( n, total ) {
				rc=Class.rect(sx, sy, 55, 55 );
				if( rc.contains(pos) ) {
					tab[currentPage]=n;
					this.findControl('#ListBox').drawPage(tab);
				}
				sx+=60;
			}
		case NextButton:
			if( cur[rect].contains(pos) ) {
				if( cur[rect].eq(this.mouseDownRect) ) {
					tab[currentPage++];
					this.findControl('#ListBox').drawPage(tab);
				}
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

OrderInfo.OrderInfo(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
OrderInfo.initControl() {
	tag.removeAll();
	tag.addNode({tag: OrderQty});
	tag.addNode({tag: OrderPrice});
	setNodeSize(tag, true);
}
OrderInfo.conf() {
	confNodeLayout(tag);
	wa=_arr().recalc(tag[rect].width(), "300,250");
	while( cur, tag, r, 0 ) {
		cur[rect].inject(x,y,w,h);
		if( r.eq(0) ) {
			y+=8;
		}
		while( cw, wa, c, 0 ) {
			rc=Class.rect(x,y,cw,h), x+=cw;
			switch( c ) {
			case 0:	cur[rect label]	= rc.incrX(30);
			case 1:	cur[rect value]	= rc.incrW(-50);
			}
		}
	}
}
OrderInfo.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	lang=Cf[KioskLangSelect].lower();
	if( lang.eq('kor') ) {
		lang_a='주문수량', lang_b='주문금액';
	} else {
		lang_a='Total Qty.', lang_b='Total';
	}
	setFont(draw, 20, '#505050', 'bold');
	this.findControl('MenuCart.ShoppingCart', true).inject(OrderTotalQty, OrderTotalPrice);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case OrderQty:
			not( cur[rect label] ) {
				this.conf();
				return;
			}
			drawNodeText(draw, cur[rect label], "$lang_a :", "left", "OrderInfo");
			drawNodeText(draw, cur[rect value], OrderTotalQty, "right", "OrderInfo");
			rc=cur[rect].incrX(10);
			draw.rectLine(rc.incrW(-25), 4, '#d0d0d0', 1, 'dot');
		case OrderPrice:
			not( cur[rect label] ) {
				this.conf();
				return;
			}
			price=util_priceComma( OrderTotalPrice );
			drawNodeText(draw, cur[rect label], "$lang_b :", "left", "OrderInfo");
			drawNodeText(draw, cur[rect value], price, "right", "OrderPrice");
		default:
		}
	}
}
OrderInfo.mouseDown(pos) {

}
OrderInfo.mouseUp(pos) {

}

MainButtons.MainButtons(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MainButtons.initControl() {
	tag[type]='hbox';
	setNodeSize(tag,true);
}
MainButtons.conf() {
	tag[rect].inject(x,y,w,h);
		while( cur, tag, n, 0 ) {
		switch(n ) {
		case 0: w=180;
			rc=Class.rect(x,y,w,h);
			cur[rect]=rc.center(170,115);
			x+=w;
		case 1:
			w=360;
			rc=Class.rect(x,y,w,h);
			cur[rect]=rc.center(350,115);
		default:
		}
	}
}
MainButtons.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	lang=Cf[KioskLangSelect].lower();
	if( lang.eq('kor') ) {
		lang_0="전체\n취소", lang_1='결제하기';
	} else {
		lang_0='Clear All', lang_1='Card Payment';
	}
	while( cur, tag, n, 0 ) {
		rc=cur[rect], type= when( cur[rect].eq(this.mouseDownRect), 'p', 'n');
		not( rc.valid() ) {
			this.conf();
			this.update();
			return;
		}
		/* 주문내역이 있을경우 주문하기 버튼을 빨간색으로 (2017-03-06) */
		if( cur[tag].eq('SelectCard') && type.eq('n') ) {
			/* if( cur[@srca] ) cur[@srca]=null; */
			items=cf[orderItemList];
			if( items.childCount() ) {
				type='a';
			}
		}
		draw.drawImage(rc, imageLoad(cur,'src',type) );
		drawNodeText(draw, cur[rect], fmt("lang_$n",true), 'center', 'MainButton');
	}
}
MainButtons.mouseDown(pos) {
	this.mouseDownRect=null;
	while( cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		this.mouseDownRect=cur[rect];
		switch( cur[tag] ) {
		case ClearAll:
			cf[cardButtonCheck]=false;
			orderList=this.findControl('MenuCart#orderView').getOrderList();
			if( orderList.size() ) {
				this.mainControl().goHome();
			}
		case SelectCard:
			this.cardButtonClick();
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
MainButtons.mouseUp(pos) {
	if( this.mouseDownRect ) {
		while( cur, tag ) {
			not( cur[rect].contains(pos) ) continue;
			not( cur[tag].eq('SelectCard') ) {
				this.mouseDownRect=null;
				this.update();
			}
		}
	}
}

AdPanel.initAdNode() {
	root=this.findTag('AdImage');
	root[date]=System.date('yyyyMMdd');
	db.fetchAll( conf('sql#hitec.did_schedule'), root.removeAll() );
	while( node, root ) {
		/* 이미지가 아니라면 무시 */
		not( node[ad_gubun].eq('2') ) {
			node[down_yn]='Y';
			continue;
		}
		fileNm=node[ad_contents_url].findLast('/').right().trim();
		node[SaveImagePath]="$cf[imagePath]/did/$fileNm";
		/* 이미지 로딩실패시 다운로드 처리 */
		not( imageLoad(node,'SaveImagePath') ) {
			this.mainControl().workerAdd(KIOSK.Download, node);
		}
	}
	/* DID정보가 새로 세팅되었으므로 보여질 노드를 선택한다 */
	this.setDisplayNode();
}

mainCanvas.showSubWidget(id, rc) {
	widget=cf[subWidgets.$id];
	not( widget ) {
		switch(id) {
		case loading:
			cf.inject(imagePath);
			widget=canvas.widget({tag:canvas});
			widget.style("background-color: rgba(0,0,0,0%)");
			widget.playGif("$imagePath/loading2.gif");
		case webview:
			widget=canvas.widget({tag:webview});
			widget.scroll('hide');
		default:
			widget=canvas.widget(conf("widget#kiosk.$id") );
			if( id.eq('moviePlayer') ) {
				cf[playerWidgets].add(widget);
			}
		}
		widget.flags('top,splash');
		widget.open();
		cf[subWidgets.$id]=widget;
	}
	if( rc ) {
		rcGlobal=canvas.mapGlobal(rc);
		widget.move(rcGlobal.lt());
		widget.size(rcGlobal.size());
		widget.show();
	}
	return widget;
}
mainCanvas.hideSubWidget() {
	while( id, cf[subWidgets].keys() ) {
		widget=cf[subWidgets.$id];
		if( widget ) widget.hide();
	}
}

AdPanel.setDisplayNode(var) {
	/* [exam]
		setDisplayNode({ad_code:xxxx, ad_gubun:3, ad_contents_url:data/did/a.mp4} );
		setDisplayNode('A00003');
	*/
	update=var;
	ad=this.findTag('AdImage');
	node=null;
	if( typeof(var,'node') ) {
		find=ad.findOne('ad_code', var[ad_code]);
		if( find ) {
			find.varMap(var,'ad_contents_url, ad_gubun', true);
			node=find;
		} else {
			cur=ad.addNode().initNode(var);
			node=cur;
		}
		if( node[ad_gubun].eq('2') ) {
			node[down_yn]='Y';
			node[SaveImagePath]=node[ad_contents_url];
		}
	} else  {
		if( var ) {
			node=ad.findOne('ad_code', var);
		}
		not( node ) {
			while( cur, ad ) {
				if( cur[select_yn].eq('Y') ) {
					node=cur;
					break;
				}
			}
			not( node ) {
				node=ad.child(0);
			}
		}
	}
	/* 다른 서브윈도우가 있다면 모두 닫는다 */
	this.mainControl().hideSubWidget();
	/* 노드 정보 적용 */
	switch( node[ad_gubun] ) {
	case 2:
		draw=tag[DrawImage];
		img=imageLoad(node,'SaveImagePath');
		if( draw && img ) {
			rc=draw.rect();
			draw.drawImage(rc, img );
		}
	case 3:
		widget=this.mainControl().showSubWidget('moviePlayer');
		widget.initPage( node[ad_contents_url] );
	case 4:
		this.mainControl().showSubWidget('webview').url( node[ad_contents_url] );
	}
	tag[CurrentDisplayNode]=node;
	if( update ) {
		this.update();
	}
	return node;
}


PopupTestCanvas.PopupTestCanvas(page) {
	this.addClass('common.CanvasBase');
	canvas=page.canvas;
	canvas.eventMap( onDraw, this.canvasDraw, 'draw');
	canvas.eventMap( onMouseDown, this.canvasMouseDown, 'pos');
	canvas.eventMap( onMouseUp, this.canvasMouseUp, 'pos');
	canvas.eventMap( onMouseMove, this.canvasMouseMove, 'pos');
	canvas.eventMap( onEvent, this.canvasEvent, 'type, node');
	page.eventMap( onActivationChange, this.widgetPageCheck );
	page.eventMap( onMove, this.moveWidget );
	/* 타이머 설정 */
	canvas.timer( 1000, callback() {
		this.timeout();
	}, this);
	/* 설정정보 세팅 */
	this.initConfig();
	this.initPage();
}
PopupTestCanvas.initConfig() {
	cf.debug=true;
	cf.pageMode='scroll';
	cf.projectId ='KioskHiTec';
	cf.pageCode='PopupTest';
	cf.imagePath="project/KioskHiTec/images";
	loadCommonImage(cf);
}
PopupTestCanvas.initPage() {
	cf.pageStart=false;
	cf.pageRate=1;
}
PopupTestCanvas.canvasDraw(draw) {
	tm=getDrawTimeline( timelineNode );
	if( cf[stackPage] ) {
		cf[stackPage].draw(draw, tm);
		if( cf[popupControl] ) {
			cf[popupControl].draw(draw,tm);
		}
	} else {
		if( cf[popupControl] ) {
			this.draw(draw);
			cf[popupControl].draw(draw,tm);
		} else {
			this.draw(draw, tm);
		}
	}
	if( cf[selectedItem] ) {
		rc=cf.selectedItem.rect;
		draw.rectLine(rc.incr(1), 0, '#afa0ea',3);
	}
	if( cf[mouseDownAction] ) {
		draw.save().pen('#cab0e9', 4);
		draw.polyLine(cf[mouseActionPoints]);
		draw.restore();
	}
}
PopupTestCanvas.canvasMouseDown(pos) {
	while( rc, cf[ActionRects] ) {
		if( rc.contains(pos) ) {
			_arr(cf,'mouseActionPoints').reuse();
			cf[mouseDownAction]=true;
		}
	}
	if( cf[stackPage] ) {
		if( cf[popupControl] ) {
			cf[popupControl].mouseDown(pos);
		}  else {
			cf[stackPage].mouseDown(pos);
		}
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseDown(pos);
		return;
	}
	this.mouseDown(pos);
}
PopupTestCanvas.canvasMouseMove(pos) {
	if( cf[mouseDownAction] ) {
		cf[mouseActionPoints].add(pos);
		this.update();
	}
	this.mouseMove(pos);
}
PopupTestCanvas.canvasMouseUp(pos) {
	if( cf[mouseDownAction] && canvasMouseAction(this) ) {
		return;
	}
	if( cf[stackPage] ) {
		if( cf[popupControl] ) {
			cf[popupControl].mouseUp(pos);
		}  else {
			cf[stackPage].mouseUp(pos);
		}
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseUp(pos);
		return;
	}
	this.mouseUp(pos);
}
PopupTestCanvas.canvasEvent(type, node) {
	switch( type ) {
	case KIOSK.Log:
		if( cf.debugEditor ) cf.debugEditor.append( tag[logMessage], true );
	default: break;
	}
}
PopupTestCanvas.timeout() {
	not( cf[pageStart] ) {
		return;
	}
	/* 타이머 처리로직 구현*/
	if( cf[inputFocusRect] ) {
		mod=cf[cursorIndex++];
		this.update();
	}

}

	Popup.popupClose() {
	cf[inputNode]=null;
	not( this[mainNode] ) return;
	ctrl=this.getControl( this[mainNode] );
	if( ctrl.popupCloseEvent ) {
		ctrl.popupCloseEvent();
	}
	if( ctrl.drawFadeOut ) {
		this.timelineStart('FadeOutPopup', this, 'FadeOut');
	}
	if( cf[errorOpen] ) return;
	cf[popupControl]=null;
	this[mainNode]=null;
	this.update();

}
	Popup.popupOpen(tagId, path, var) {
	not( tagId ) return;
	this.mainControl().popupClose();
	cur=tag.findOne('tag', tagId);
	not( cur ) {
		cur=tag.addNode();
		cur[tag]=tagId;
	}
	check=cf[classErrorCheck];
	if( path ) {
		cur[ClassPath]=path;
		classId="$path/control.$tagId";
	} else {
		cf.inject(projectId, pageCode);
		classId="${projectId}/${pageCode}/control.$tagId";
	}
	if( check[$classId] ) {
		check[$classId]=false;
	}
	not( this.getControl(cur) ) {
		_log("popupOpen error: 태그아이디: $tagId");
		return false;
	}
	/* 팝업 영역 설정 */
	mainNode=this.mainControl().getMainNode();
	args(2,rect, target, style );
	not( rect ) {
		if( cur[Width] && cur[Height] ) {
			w=cur[Width], h=cur[Height];
			rect=Class.rect(0,0,w,h);
		} else{
			not( target ) {
				target=mainNode[rect];
			}
			rect=target.incr(40);
		}
	}
	/* 대상이
		- 영역이: 영역가운데,
		- 포인트: x,y 위치
		오픈 위치를 세팅
	*/
	if( typeof(target,'point') ) {
		tag[rect]=rect.incrXY( target, true);
	} else {
		not( typeof(target,'rect') ) {
			target=mainNode[rect];
		}
		if( style.eq('popup') ) {
			if( target.right() < rect.right() ) {
				dist= rect.right()-target.right(), dist*=-1;
				rect.incrX(dist);
			}
			tag[rect]=rect.incrY(8, true);
		} else {
			tag[rect]=target.center(rect);
		}
	}
	/* 자식에 설정된 영역 삭제 */
	tagClearRect(cur);
	cur[rect]=tag[rect];
	this.mainNode=cur;
	cf[popupStartTick]=System.tick();
	cf[popupControl]=this;
	ctrl=this.getControl(cur);
	if( ctrl.initPage ) {
		ctrl.initPage();
	}
	if( ctrl.drawFadeIn ) {
			this.mainControl().timelineStart('FadeInPopup', this, 'FadeIn');
	}
	this.conf();
	this.update();
	return ctrl;
}

MyControl.MyControl(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MyControl.initControl() {
	setNodeSize(tag, true);
	tag.addNode({tag:Calendar, ClassPath:common});
}
MyControl.conf() {
	while( cur, tag ) {
		this.getControl(cur).conf();
	}
}
MyControl.draw(draw, tm) {
	while( cur, tag ) {
		this.getControl(cur).draw(draw, tm);
	}
}
MyControl.mouseDown(pos) {
	while( cur, tag ) {
		this.getControl(cur).mouseDown(pos);
	}
	this.findControl('Popup#dialog').popupOpen('Calendar', 'common');
	this.findControl('Popup#stack').stackPageLoad('AdminLogin');
}
MyControl.mouseUp(pos) {
	while( cur, tag ) {
		this.getControl(cur).mouseUp(pos);
	}
}


Calendar.Calendar(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
Calendar.initControl() {
	not( tag[type] ) tag[type]='vbox';
	tag.removeAll();
	tag.addNode({tag: YearLabel});
	tag.addNode({tag: MonthLabel});
	tag.addNode({tag: PrevButton});
	tag.addNode({tag: NextButton});
	tag.addNode({tag: Days});
	commonImage('calendar_bg').imageSize().inject(w,h);
	tag[Width]=w, tag[Height]=h;
	tag[rect]=null;
	setNodeSize(tag, true);
}
Calendar.conf() {
	offset=tag[rect].lt();
	offset.inject(ox, oy);
	while( cur, tag ) {
		switch(cur[tag] ) {
		case YearLabel:		cur[rect]=Class.rect(157,18,212,75).move(offset, true);
		case MonthLabel:	cur[rect]=Class.rect(421,18,100,75).move(offset, true);
		case PrevButton:
			cur[rect]=Class.rect(25,18,83,75).move(offset, true);
		case NextButton:
			cur[rect]=Class.rect(631,18,83,75).move(offset, true);
		case Days:
			sx=2, sy=155, sw=105, sh=94;
			sx+=ox, sy+=oy;
			while( row, 6 ) {
				cx=sx;
				while( col, 7 ) {
					cur[rect $row $col]=Class.rect(cx, sy, sw, sh);
					cx+=sw;
				}
				sy+=sh;
			}
		}
	}
}
Calendar.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	not( tag[Year] ) tag[Year]=System.date('yyyy');
	not( tag[Month] ) tag[Month]=System.date('MM');
	draw.drawImage(tag[rect], commonImage('calendar_bg') );
	while( cur, tag ) {
		switch(cur[tag] ) {
		case YearLabel:
			drawNodeText( draw, cur[rect],"$tag[Year] 년" , 'right', 26, '#f0f0f0');
		case MonthLabel:
			drawNodeText( draw, cur[rect],"$tag[Month] 월" , 'left', 26, '#f0f0f0');
		case PrevButton:
			var=when( cur[rect].eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage(cur[rect], commonImage('btn_prev',var) );
		case NextButton:
				var=when( cur[rect].eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage(cur[rect], commonImage('btn_next',var) );
		case Days:
			year=tag[Year];
			month=lpad(tag[Month],2);
			tm			=System.localtime("${year}-${month}-01" );
			days		=System.date(tm,'daysInMonth');
			day		=System.date(tm,'dayOfWeek');
			if( day.eq(7) ) day=0;
			m1="$tag[Year]-$tag[Month]", m2=System.date('yyyy-MM');
			curMonth = when( m1.eq(m2), true );
			dd=System.date('dd');
			dayNum=1;
			while( row, 6 ) {
				while( col, 7, day ) {
					rc=cur[rect $row $col];
					if( curMonth && dayNum.eq(dd) ) {
						draw.fill(rc.incr(1), '#c0a0aa');
					}
					switch( col ) {
					case 0:	color='#fa707a';
					case 6:	color='#7070fa';
					default: 	color='#70707a';
					}
					drawNodeText( draw, rc, dayNum , 'center', 18, color);
					if( dayNum.eq(days) ) {
						break;
					}
					dayNum+=1;
				}
				day=0;
				if( dayNum.eq(days) ) break;
			}
		default:
		}
	}
}
Calendar.mouseDown(pos) {
	tag[SelectType]=null;
	while( cur, tag ) {
		switch(cur[tag] ) {
		case YearLabel:		 tag[SelectType]='year';
		case MonthLabel:	 tag[SelectType]='month';
		case PrevButton:
			not( cur[rect].contains(pos) ) continue;
			month=tag[Month];
			month-=1;
			if( month<1 ) {
				tag[Month]=12;
			} else {
				tag[Month]=month;
			}
			this.mouseDownRect=cur[rect];
		case NextButton:
			not( cur[rect].contains(pos) ) continue;
			month=tag[Month];
			month+=1;
			if( month>12 ) {
				tag[Month]=1;
			} else {
				tag[Month]=month;
			}
			this.mouseDownRect=cur[rect];
		case Days:
			year=tag[Year];
			month=lpad(tag[Month], 2);
			tm			=System.localtime("${year}-${month}-01" );
			days		=System.date(tm,'daysInMonth');
			day		=System.date(tm,'dayOfWeek');
			if( day.eq(7) ) day=0;
			dayNum=1;
			while( row, 6 ) {
				while( col, 7, day ) {
					rc=cur[rect $row $col];
					if( rc.contains(pos) ) {
						this.setDateSelect(year, month, dayNum);
						return;
					}
					if( dayNum.eq(days) ) {
						break;
					}
					dayNum+=1;
				}
				day=0;
				if( dayNum.eq(days) ) {
					break;
				}
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
Calendar.mouseUp(pos) {
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}

}
	Calendar.test() {
	this.initControl();
	this.conf();

}

	NumberPad.NumberPad(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();

}
	NumberPad.initControl() {
	tag[Width]=452, tag[Height]=536;
	setNodeSize(tag, true);

}
	NumberPad.conf() {
	cf.inject( imagePath);
	tag[rect].inject(sx, sy, sw, sh);
	bw=134, bh=119;
	sx+=15, sy+=15;
	num=1;
	imageLoad(tag, 'bg', "${imagePath}/main/common/bg_452x536.png");
	while( row,4 ) {
		cx=sx;
		while( col,3 ) {
			if( row.eq(3) ) {
				switch(col) {
				case 0:	img="login_num_00_[#].png";
				case 1:	img="login_num_del_[#].png";
				case 2:	img="login_num_re_[#].png";
				}
			} else {
				img="login_num_0${num}_[#].png";
				num++;
			}
			tag[rect#$row $col]=Class.rect(cx, sy, bw, bh), cx+=bw+10;
			tag[img#$row $col]="${imagePath}/admin/$img";
		}
		sy+=bh+10;
	}
	printNode(tag);
}

NumberPad.draw(draw, tm) {
	draw.drawImage( tag[rect], imageLoad(tag, 'bg') );
	while( row,4 ) {
		while( col,3 ) {
			rc=tag[rect#$row $col];
			var=when( rc.eq(this.mouseDownRect), 'p', 'n');
			img=imageLoad(tag, "img#$row $col", var);
			draw.drawImage(rc, img);
		}
	}
}
NumberPad.mouseDown(pos) {
	while( row,4 ) {
		while( col,3 ) {
			not( tag[rect#$row $col].contains(pos) ) continue;
			this.mouseDownRect=tag[rect#$row $col];
			this.buttonClick(row, col);
			break;
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
NumberPad.mouseUp(pos) {
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

Popup.stackPageOpen(tagId, path) {
	not( tagId ) return;
	cur=tag.findOne('tag', tagId);
	not( cur ) {
		cur=tag.addNode();
		cur[tag]=tagId;
	}
	check=cf[classErrorCheck];
	if( path ) {
		cur[ClassPath]=path;
		classId="$path/control.$tagId";
	} else {
		cf.inject(projectId, pageCode);
		classId="${projectId}/${pageCode}/control.$tagId";
	}
	if( check[$classId] ) {
		check[$classId]=false;
	}
	this.stackPageUpdate(cur);

}
	Popup.stackPageClose() {
	cf[stackPage]=null;
	if( this[mainNode] ) {
		ctrl=this.getControl( this[mainNode] );
		if( ctrl.pageCloseEvent ) {
			ctrl.pageCloseEvent();
		}
		if( ctrl.pageCloseEffect ) {
			this.timelineStart('PageEffect',  this, 'close');
		}
	}
	this[mainNode]=null;
	this.update();
}

AdminLogin.AdminLogin(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
AdminLogin.conf() {
	tagClearRect(tag);
	cf.inject(imagePath);
	confNodeLayout(tag);
	/* 숫자입력 패드 영역 설정 */
	cur=findTag('AdminNumberPad', tag);
	bw=cur[ButtonWidth], bh=cur[ButtonHeight];
	cur[Margin].inject(sx, sy);
	num=1;
	while( row,4 ) {
		cx=sx;
		while( col,3 ) {
			if( row.eq(3) ) {
				switch(col) {
				case 0:	img="login_num_00_[#].png";
				case 1:	img="login_num_del_[#].png";
				case 2:	img="login_num_re_[#].png";
				}
			} else {
				img="login_num_0${num}_[#].png";
				num++;
			}
			cur[rect#$row $col]=Class.rect(cx, sy, bw, bh), cx+=bw+10;
			cur[img#$row $col]="${imagePath}/admin/$img";
		}
		sy+=bh+10;
	}
}
AdminLogin.draw(draw, timeline) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case LogInButton:
			drawNodeButton(draw, cur, null, this);
		case ExitButton:
			drawNodeButton(draw, cur, null, this);
		case AdminNumberPad:
			while( row,4 ) {
				while( col,3 ) {
					rc=cur[rect#$row $col];
					var=when( rc.eq(this.mouseDownRect), 'p', 'n');
					img=imageLoad(cur, "img#$row $col", var);
					draw.drawImage(rc, img);
				}
			}
		case [UserNameInput, PassWordInput] :
			if( cur[text] ) drawNodeText(draw, cur[rect], cur[text], 'left', 'TableHeader');
		case StoreTitle:
			if( cf[store_nm] ) drawNodeText(draw, cur[rect], cf[store_nm], 'left', 'PopupTitle');
		case CurrentTime:
			drawNodeText(draw, cur[rect],System.date('HH:mm:ss'), 'left', 36, '#f0f0f0', 'bold');
		case CurrentDate:
			drawNodeText(draw, cur[rect],System.date('yyyy-MM-dd'), 'left', 16, '#f0f0f0');
		case CurrentPosNo:
			drawNodeText(draw, cur[rect],cf[storeNo], 'left', 22, '#f0f0f0');
			draw.rectLine(cur[rect], 4, '#d0d0d0');
		default:
		}
	}
	if( cf[inputFocusRect] ) {
		draw.effect(
			DRAW.RoundBox, cf[inputFocusRect].incrXY(1,3), 5, '#303040', '#f9f90030', 4
		);
		mod=cf[cursorIndex] %2;
		not( mod ) {
			input=cf[inputNode];
			tw=textWidth(24, input[text],'bold', 4);
			input[rect].inject(x,y,w,h);
			x+=tw, y+=10, h-=14;
			draw.fill( Class.rect(x,y,2,h), '#909080a0');
		}
	}
}
AdminLogin.exitButtonClick() {
	this.findControl('Popup#stack').stackPageClose();
}
AdminLogin.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
}
AdminLogin.loginButtonClick() {
	user=findTag('UserNameInput', tag);
	pwd=findTag('PassWordInput', tag);
	this.setLoginConfig( user[text], pwd[text] );
}
AdminLogin.mouseDown(pos) {
	while( cur, tag ) {
		switch(cur[tag]) {
		case AdminNumberPad:
			num=1;
			while( row,4 ) {
				while( col,3 ) {
					rc=cur[rect#$row $col];
					if( row.eq(3) ) {
						switch( col ) {
						case 0: key=0;
						case 1: key='Delete';
						case 2: key='Reset';
						}
					} else {
						key=num;
						num++;
					}
					not( rc.contains(pos) ) {
						continue;
					}
					this.mouseDownRect=rc;
					this.numberButtonClick(key);
					this.update();
					return;
				}
			}
		case UserNameLabel:
			if( cur[rect].contains(pos) ) {
				cf[inputNode]=this.findTag('UserNameInput');
				cf[inputFocusRect]=cur[rect];
				this.update();
				return;
			}
		case PassWordLabel:
			if( cur[rect].contains(pos) ) {
				cf[inputNode]=this.findTag('PassWordInput');
				cf[inputFocusRect]=cur[rect];
				this.update();
				return;
			}
		default:
			not( cur[rect].contains(pos) ) continue;
			this.mouseDownRect=cur[rect];
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
AdminLogin.mouseUp(pos) {
	while( cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag]) {
		case LogInButton:	this.loginButtonClick();
		case ExitButton:		this.exitButtonClick();
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
AdminLogin.numberButtonClick(key) {
	not( cf[inputFocusRect] ) {
		user=this.findTag('UserNameLabel');
		cf[inputNode]=this.findTag('UserNameInput');
		cf[inputFocusRect]=user[rect];
	}
	input=cf[inputNode];
	not( input ) return;
	not( isset(input[text]) ) {
		input[text]='';
	}
	switch(key ) {
	case Delete:
		val=input[text];
		input[text]=val.value(0,-1);
	case Reset:
		input[text]='';
	default:
		input[text].add(key);
	}
	cf[cursorIndex]=0;
	this.update();
}

AdminHome.AdminHome(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
AdminHome.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
}
AdminHome.conf() {
	confNodeLayout(tag);
}
AdminHome.draw(draw, timeline) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		if( cur[tag].eq('Description') ) continue;
		drawNodeButton(draw, cur, null, this);
	}
}
AdminHome.exitButtonClick() {
	this.mainControl().pageOpen('AdminLogin');
}
AdminHome.mouseDown(pos) {
	this.mouseDownRect=null;
	while( cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		loginInfo=_node('LoginInfo');
		if( loginInfo[loginStartTick] ) {
			dist=System.tick() - loginInfo[loginStartTick];
			if( dist>200000 ) {
				this.findControl('#Content').pageLoad('LoginView');
				this.mainControl().alert("로그인 세션이 말료되었습니다. 다시 로그인 하세요", "알림");
			}
		} else {
			this.findControl('#Content').pageLoad('LoginView');
			this.mainControl().alert("로그인 정보가 없습니다. 로그인후 이용하세요.", "알림");
			return;
		}
		this.mouseDownRect=when(cur[rcButton], cur[rcButton], cur[rect]);
		break;
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
AdminHome.mouseUp(pos) {
	not( this.mouseDownRect ) {
		return;
	}
	while( cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch( cur[tag] ) {
		case SalesCloseButton:		this.findControl('#Content').pageLoad('AdminSaleClose');
		case SalesStatusButton:	this.findControl('#Content').pageLoad('AdminSaleStatus');
		case GoKiosk:
			this.findControl('#Content').pageLoad('LoginView');
			System.timeout(100);
			main=this.mainControl();
			main[page].hide();
		case ExitButton:
			rc=Class.rect(0,0,936,560);
			this.findControl('#Content').popupOpen('ExitButtons', this, rc, 'center', 'popup');
		default:
		}
		break;
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

SaleOpenView.SaleOpenView(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SaleOpenView.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag:Inputs, RowNum:8, StartY:228, RowHeight:109, TopMargin:20});
	tag.addNode({tag:NumberPad});
	tag.addNode({tag:SaleOpenButton});
}
SaleOpenView.conf() {
	confNodeLayout(tag);
	cf.inject(imagePath );
	rc=tag[rect].height(90);
	tag[rcBtnClose]=rc.move('end', 100 ).center(41,42);
	main=this.mainControl();
	main[page].inject(cf);
	db=Class.db('kiosk_hitec');
	db.fetch( conf("sql#kiosk.hitec#StoreInfo"), cf);
	while( cur, tag ) {
		switch(cur[tag]) {
		case Inputs:
			sy=cur[StartY];
			while( row, cur[RowNum] ) {
				x=348, y=sy+cur[TopMargin];
				cur[rect#$row]=Class.rect(x,y,296,76);
				sy+=cur[RowHeight];
			}
		case NumberPad:
			cur[rect]=Class.rect(40,1700,677,187);
			cur[rect].inject(sx, sy, sw, sh );
			/* 버튼 폭,높이 */
			bw=104, bh=89;
			/* 버튼 키정보 */
			num=1, arr=_arr(cur,'keys',true);
			sy+=2;
			while( row, 2 ) {
				cx=sx;
				while(col, 6 ) {
					rc=Class.rect(cx, sy, bw, bh );
					if( col<5 ) {
						if( num.eq(10) ) num=0;
						cur[rect#$num]=rc;
						cur[img#$num]="${imagePath}/admin/sub_num_0${num}_[#].png";
						arr.add(num);
						num++;
					} else {
						if( row.eq(0) ) {
							cur[rect#Delete]=rc;
							cur[img#Delete]="${imagePath}/admin/sub_num_del_[#].png";
							arr.add('Delete');
						} else {
							cur[rect#Reset]=rc;
							cur[img#Reset]="${imagePath}/admin/sub_num_re_[#].png";
							arr.add('Reset');
						}
					}
					cx+=bw+4;
				}
				sy+=bh+4;
			}
		case SaleOpenButton:
			cur[rect]=Class.rect(780,1700,262,187);
			cur[src]="${imagePath}/admin/start_btn_[#].png";
		default:
		}
	}
}
SaleOpenView.draw(draw) {
	drawNodeStyle(draw, tag);
	rc=tag[rcBtnClose];
	var=when( this[mouseDownRect].eq(rc), 'p', 'n');
	draw.drawImage(rc, commonImage('page_close', var) );
	main=this.mainControl();
	main[page].inject(cf);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Inputs:
			while( row, cur[RowNum] ) {
				rc=cur[rect#$row];
				draw.rectLine(rc , 0, '#f0909066', 2);
				draw.font(16,'bold','#d0d0d0');
				switch( row ) {
				case 0:
					draw.text(rc, System.date('yyyy-MM-dd'), 'center');
				case 1:
					draw.text(rc, cf[ms_nm], 'center');
				case 2:
					draw.text(rc, cf[master_nm], 'center');
				case 3:
					draw.font(16,'bold','#30303a');
					draw.text(rc.incrX(12), tag[pwd], 'left');
				}
			}
		case NumberPad:
			while( key, cur[keys] ) {
				rc=cur[rect#$key];
				var=when( this[mouseDownRect].eq(rc), 'p', 'n');
				img=imageLoad(cur, "img#$key", var);
				draw.drawImage(rc, img);
			}
		case SaleOpenButton:
			drawNodeButton(draw, cur, null, this);
		default:
		}
	}
}
SaleOpenView.mouseDown(pos) {
	this.mouseDownRect=null;
	if( tag[rcBtnClose].contains(pos) ) {
		this.mouseDownRect=tag[rcBtnClose];
		this.pageCloseButtonClick();
		this.update();
		return;
	}
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Inputs:
			while( row, cur[RowNum] ) {
				not( cur[rect#$row].contains(pos) ) continue;
				this.mouseDownRect=cur[rect#$row];
			}
		case NumberPad:
			while( key, cur[keys] ) {
				not( cur[rect#$key].contains(pos) ) continue;
				this.mouseDownRect=cur[rect#$key];
				this.numberKeyDown(key);
			}
		case SaleOpenButton:
			if( cur[rect].contains(pos) ) {
				this.mouseDownRect=cur[rect];
				this.openStoreButtonClick();
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
SaleOpenView.mouseUp(pos) {
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

SaleCloseView.SaleCloseView(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SaleCloseView.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag:SaleCloseButton});
	tag.addNode({tag:Form1});
	tag.addNode({tag:Form2});
	tag.addNode({tag:Form3});
	tag.addNode({tag:Form4});
	tag.addNode({tag:Form5});
	tag.addNode({tag:Form6});
	tag.addNode({tag:Form7});
	tag.addNode({tag:Form21});
	tag.addNode({tag:Form22});
	tag.addNode({tag:Form23});
	this.dataNode={};
}
SaleCloseView.conf() {
	confNodeLayout(tag);
	cf.inject(imagePath );
	rc=tag[rect].height(90);
	tag[rcBtnClose]=rc.move('end', 100 ).center(41,42);
	topMargin=nvl(tag[topMargin], 127);
	while( cur, tag ) {
		switch(cur[tag]) {
		case SaleCloseButton:
			cur[rect]=Class.rect(780,1670, 262,187);
			cur[src]="${imagePath}/admin/close_btn_[#].png";
		case Form1:
			sx=46, sy=197, sw=325, sh=52, sy+=topMargin;
			while( col, 3 ) {
				cur[rect#$col]=Class.rect(sx, sy, sw, sh);
				sx+=sw+4;
			}
		case Form2:
			sx=46, sy=321, sw=325, sh=52, sy+=topMargin;
			while( col, 3 ) {
				cur[rect#$col]=Class.rect(sx, sy, sw, sh);
				sx+=sw+4;
			}
		case Form3:
			sx=295, sy=454, sw=240, sh=55, sy+=topMargin;
			while( row, 2 ) {
				cur[rect#$row]=Class.rect(sx, sy, sw, sh);
				sy+=sh+4;
			}
		case Form4:
			sx=295, sy=644, sw=240, sh=55, sy+=topMargin;
			while( row, 4 ) {
				cur[rect#$row]=Class.rect(sx, sy, sw, sh);
				sy+=sh+4;
			}
		case Form5:
			sx=295, sy=947, sw=240, sh=55, sy+=topMargin;
			while( row, 4 ) {
				cur[rect#$row]=Class.rect(sx, sy, sw, sh);
				sy+=sh+4;
			}
		case Form6:
			sx=295, sy=1249, sw=240, sh=55, sy+=topMargin;
			while( row, 3 ) {
				cur[rect#$row]=Class.rect(sx, sy, sw, sh);
				sy+=sh+4;
			}
		case Form7:
			sx=295, sy=1435, sw=240, sh=55, sy+=topMargin;
			cur[rect]=Class.rect(sx, sy, sw, sh);
		case Form21:
			sx=716, sy=454, sw=106, sh=55, sy+=topMargin;
			while( row, 3 ) {
				cur[rect#$row]=Class.rect(sx, sy, sw, sh);
				sy+=sh+4;
			}
		case Form22:
			sx=716, sy=700, sw=106, sh=55, sy+=topMargin;
			while( row, 3 ) {
				cur[rect#$row]=Class.rect(sx, sy, sw, sh);
				sy+=sh+4;
			}
		case Form23:
			sx=830, sy=1005, sw=205, sh=55, sy+=topMargin;
			while( row, 3 ) {
				cur[rect#$row]=Class.rect(sx, sy, sw, sh);
				sy+=sh+4;
			}
		}
	}
}
SaleCloseView.draw(draw) {
	drawNodeStyle(draw, tag);
	cf=this[dataNode];
	/* 페이지 닫기 버튼 */
	rc=tag[rcBtnClose];
	var=when( this[mouseDownRect].eq(rc), 'p', 'n');
	draw.drawImage(rc, commonImage('page_close', var) );
	draw.font(16,'bold','#d0d0d0');
	while( cur, tag ) {
		switch(cur[tag]) {
		case SaleCloseButton:
			drawNodeButton(draw, cur, null, this);
		case Form1:
			/*영업일자, 개점시간, 마감시간*/
			while( col, 3 ) {
				rc=cur[rect#$col];
				switch( col ) {
				case 0:
					day=util_formatDate(cf[open_date] );
					draw.fill( rc,'#CA9030').rectLine( rc, 0, '#f0a0a066');
					if( day ) draw.text( rc, day, 'center');
				case 1:
					date=when( cf[reg_open_dtm], cf[reg_open_dtm].replace('T', ' '), '');
					draw.text( rc, "$date", 'center');
					draw.rectLine( rc, 0, '#f0f0f0');
				case 2:
					date=when( cf[reg_close_dtm], cf[reg_close_dtm].replace('T', ' '), '');
					draw.text( rc, "$date", 'center');
					draw.rectLine( rc, 0, '#f0f0f0');
				default:
				}
			}
		case Form2:
			/*총매출액, 매출취소, 실매출액*/
			while( col, 3 ) {
				rc=cur[rect#$col];
				switch( col ) {
				case 0:
					price=util_priceComma(cf[total_amt]);
					draw.text( rc, price, 'center');
				case 1:
					price=util_priceComma(cf[cancle_amt]);
					draw.text( rc, price, 'center');
				case 2:
					price=util_priceComma(cf[real_amt=total_amt-cancle_amt]);
					draw.text( rc, price, 'center');
				default:
				}
				draw.rectLine( rc, 0, '#f0f0f0');
			}
		case Form3:
			/*신용카드판매, 모바일쿠폰*/
			rc=cur[rect#$0];
			price=util_priceComma(cf[real_amt]);
			draw.text( rc, price, 'center');
			draw.rectLine( rc, 0, '#f0f0f0');
		case Form4:
			/*개점준비금*/
			while( row, 4 ) {
				draw.rectLine( cur[rect#$row], 0, '#f0a0a066');
			}
		case Form5:
			/*수시입금*/
			while( row, 4 ) {
				draw.rectLine( cur[rect#$row], 0, '#f0a0a066');
			}
		case Form6:
			/*수시출금*/
			while( row, 3 ) {
				draw.rectLine( cur[rect#$row], 0, '#f0a0a066');
			}
		case Form7:
			/*정산 현금합계*/
			draw.rectLine( cur[rect], 0, '#f0a0a066');
		case Form21:
			/*입금된 현금잔액*/
			while( row, 3 ) {
				draw.rectLine( cur[rect#$row], 0, '#f0a0a066');
			}
		case Form22:
			/*반환용 현금잔액*/
			while( row, 3 ) {
				draw.rectLine( cur[rect#$row], 0, '#f0a0a066');
			}
		case Form23:
			/*실제 현금합계*/
			while( row, 3 ) {
				draw.rectLine( cur[rect#$row], 0, '#f0a0a066');
			}
		}
	}
}
SaleCloseView.mouseDown(pos) {
	/* 페이지 닫기 버튼 */
	if( tag[rcBtnClose].contains(pos) ) {
		this.mouseDownRect=tag[rcBtnClose];
		this.pageCloseButtonClick();
	}
	cur=this.findTag('SaleCloseButton');
	if( cur[rect].contains(pos) ) {
		this.mouseDownRect=cur[rect];
		this.saleCloseClick();
		this.update();
		return;
	}
	cur=this.findTag('Form1');
	rc=cur[rect#0];
	if( rc.contains(pos) ) {
		this.currentInput=cur;
		rcOpen=rc.move('down').size(738,860);
		this.findControl('#Content').popupOpen('Calendar', this, rcOpen );
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
SaleCloseView.mouseUp(pos) {
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

SaleStatusView.SaleStatusView(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SaleStatusView.initControl() {
	not( tag[type] ) tag[type]='vbox';
	form=tag.addNode({tag:FormSearch, Height:210});
	form.addNode({tag:date, field:SaleDate});
	form.addNode({tag:combo, field:Pos});
	form.addNode({tag:input, field:SaleNo, type:number});
	form.addNode({tag:button, field:Search});
	tag.addNode({tag:GridControl, ClassPath:common, Margin:[35,40]});
	tag.addNode({tag:Buttons, Height:225});
	setNodeSize(tag, true);
}
SaleStatusView.conf() {
	cf.inject(imagePath );
	rc=tag[rect].height(90);
	tag[rcBtnClose]=rc.move('end', 100 ).center(41,42);
	topMargin=nvl(tag[topMargin], 127);
	_form=func() {
	sy=146, sy+=topMargin;
	while( cell, cur ) {
		switch( cell[field] ) {
		case SaleDate:
			cell[text] = null;
			cell[rect] = Class.rect(186, sy, 183, 55);
		case Pos:
			cell[rect] = Class.rect(475, sy, 65, 55);
		case SaleNo:
			cell[rect] = Class.rect(703, sy, 179, 55);
		case Search:
			rc=Class.rect(905, 230, 130, 85);
			cell[rect] = rc.center(123,65);
			cell[imgSearch]="${imagePath}/admin/cancel_search01_[#].png";
		}
	}
	};
	while( cur, tagRect(tag, true) ) {
		switch( cur[tag] ) {
		case FormSearch: 	_form(cur);
		case GridControl: 	this.getControl(cur).conf();
		default:
		}
	}
}
SaleStatusView.draw(draw) {
	drawNodeStyle(draw, tag);
	/* 페이지 닫기 버튼 */
	rc=tag[rcBtnClose];
	var=when( this[mouseDownRect].eq(rc), 'p', 'n');
	draw.drawImage(rc, commonImage('page_close', var) );
	while( cur, tag ) {
		switch( cur[tag] ) {
		case FormSearch:
			draw.font(14,'bold','#30303a');
			while( cell, cur ) {
				rc=cell[rect];
				switch( cell[field] ) {
				case SaleDate: 	draw.text( rc, cell[text], 'center' );
				case Pos: 			draw.text( rc, cell[text], 'center' );
				case SaleNo:		draw.text( rc, cell[text], 'left' );
				case Search:
					draw.drawImage(rc, commonImage('btn_bg') );
					draw.font(16,'bold','#f0f0f0').text(rc, "조회", "center");
				default:
				}
			}
		case GridControl:
			this.getControl(cur).draw(draw);
		default:
		}
	}
}
SaleStatusView.mouseDown(pos) {
	/* 페이지 닫기 버튼 */
	if( tag[rcBtnClose].contains(pos) ) {
		this.mouseDownRect=tag[rcBtnClose];
		this.pageCloseButtonClick();
		this.update();
		return;
	}
	cf[inputNode]=null;
	while( cur, tag ) {
		switch( cur[tag] ) {
		case FormSearch:
			while( cell, cur ) {
				not( cell[rect].contains(pos) ) continue;
				this.currentRecord=null;
				this.currentInput=cell;
				rc=cell[rect];
				switch( cell[field] ) {
				case SaleDate:
					rcOpen=rc.move('down').size(738,860);
					this.findControl('#Content').popupOpen('Calendar', this, rcOpen );
				case Pos:
					rcOpen=rc.move('down').size(450, 350);
					node=this.findControl('#Content').popupOpen('CommCombo', this, rcOpen );
					this.getControl(node).setCommCode('kiosk#SaleCancelYn','취소 여부');
				case SaleNo:
					rcOpen=rc.move('down').size(452, 536);
					this.findControl('#Content').popupOpen('NumberPad', this, rcOpen );
					this.mainControl().setInputNode(cell);
				case Search:
					this.mouseDownRect=cur[SearchRect];
					this.update();
					this.search();
					return;
				default:
				}
			}
		case GridControl:
			this.getControl(cur).mouseDown(pos);
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
SaleStatusView.mouseUp(pos) {
	this.findControl('GridControl').mouseUp(pos);
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

SoldOutView.SoldOutView(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SoldOutView.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag: ListBody} );
	tag.addNode({tag: ListButtons} );
	dataNode=_node(cf,'dataNode').removeAll();
	db=getProjectDb(cf);
	db.fetchAll("select menu_cd, menu_nm, sale_price, use_yn from tb_menu_mst ", dataNode);
	this.startRow=0;
}
SoldOutView.conf() {
	confNodeLayout(tag);
	cf.inject(imagePath );
	rc=tag[rect].height(90);
	tag[rcBtnClose]=rc.move('end', 100 ).center(41,42);
	topMargin=nvl(tag[topMargin], 127);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case ListBody:
			arr=_arr(cur,'HeaderInfo', true);
			arr.add( Class.pair(44,179) );
			arr.add( Class.pair(227,388) );
			arr.add( Class.pair(619,256) );
			arr.add( Class.pair(880,156) );
			sy=216, sh=75, sy+=topMargin;
			while( row, 16 ) {
				while( pair, arr , col, 0) {
					pair.inject(sx, sw);
					cur[rect $row@$col]=Class.rect(sx, sy, sw, sh);
				}
				sy+=sh+4;
			}
			cur[imgOn]="${imagePath}/admin/checkbox_on.png";
			cur[imgOff]="${imagePath}/admin/checkbox_off.png";
			imageLoad(cur,'imgOn');
			imageLoad(cur,'imgOff');
		case ListButtons:
			sy=1486, sy+=topMargin;
			cur[imgUp]="${imagePath}/admin/btn_up_[#].png";
			cur[imgDown]="${imagePath}/admin/btn_down_[#].png";
			cur[UpRect]		= Class.rect(841,sy,78,78);
			cur[DownRect]	= Class.rect(942,sy,78,78);
		default:
		}
	}
}
SoldOutView.draw(draw) {
	drawNodeStyle(draw, tag);
	/* 페이지 닫기 버튼 */
	rc=tag[rcBtnClose];
	var=when( this[mouseDownRect].eq(rc), 'p', 'n');
	draw.drawImage(rc, commonImage('page_close', var) );
	dataNode=cf[dataNode];
	while( cur, tag ) {
		switch( cur[tag] ) {
		case ListBody:
			idx=this.startRow;
			draw.font(18,'normal','#d0d0d0');
			while( row, 16 ) {
				sub=dataNode.child(idx), idx++;
				not( sub ) {
					break;
				}
				while( pair, cur[HeaderInfo] , col, 0) {
					rc=cur[rect $row@$col];
					switch(col) {
					case 0: drawNodeText(draw, rc, sub[menu_cd], 'center');
					case 1: drawNodeText(draw, rc.incrX(8), sub[menu_nm] );
					case 2: drawNodeText(draw, rc.incrW(-10), util_priceComma(sub[sale_price]), 'right' );
					case 3:
						img=when( sub[use_yn].eq('Y'), imageLoad(cur,'imgOff'), imageLoad(cur,'imgOn') );
						draw.drawImage( img.center(rc), img);
					default:
					}
				}
			}
		case ListButtons:
			idx=this.startRow;
			total=dataNode.childCount(), ep=idx+16;
			if( ep<total ) {
				imgDown=imageLoad(cur, 'imgDown', 'n');
			} else {
				imgDown=imageLoad(cur, 'imgDown', 'd');
			}
			if( idx>0 ) {
				imgUp=imageLoad(cur, 'imgUp', 'n');
			} else {
				imgUp=imageLoad(cur, 'imgUp', 'd');
			}
			draw.drawImage( cur[UpRect], imgUp);
			draw.drawImage( cur[DownRect], imgDown);
		default:
		}
	}
}
SoldOutView.mouseDown(pos) {
	/* 페이지 닫기 버튼 */
	if( tag[rcBtnClose].contains(pos) ) {
		this.mouseDownRect=tag[rcBtnClose];
		this.pageCloseButtonClick();
		this.update();
		return;
	}
	dataNode=cf[dataNode];
	idx=this.startRow;
	while( cur, tag ) {
		switch( cur[tag] ) {
		case ListBody:
			col=3;
			while( row, 16 ) {
				rc=cur[rect $row@$col];
				if( rc.contains(pos) ) {
					img=imageLoad(cur,'imgOff');
					rcCheck=img.center(rc);
					if( rc.contains(pos) ) {
						idx+=row;
						sub=dataNode.child(idx);
						this.soldOutClick(sub);
						this.update();
						return;
					}
				}
			}
		case ListButtons:
			if( cur[UpRect].contains(pos) ) {
				if( idx>0 ) {
					idx-=16;
					if( idx<0 ) idx=0;
					this.startRow=idx;
					this.mouseDownRect= cur[UpRect];
				}
			} else if( cur[DownRect].contains(pos) ) {
				total=dataNode.childCount();
				idx+=16;
				if( idx<total ) {
					this.startRow=idx;
					this.mouseDownRect= cur[UpRect];
				}
			}
		}
	}
	if( this.mouseDownRect ) this.update();
}
SoldOutView.mouseUp(pos) {
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

TurnOutView.TurnOutView(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
TurnOutView.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag: Search});
	tag.addNode({tag: Form1});
	tag.addNode({tag: Form2});
	tag.addNode({tag: NumberPad});
	tag.addNode({tag: TurnOutButton});
}
TurnOutView.conf() {
	confNodeLayout(tag);
	cf.inject(imagePath );
	while( cur, tag ) {
		switch(cur[tag]) {
		case Search:
			sy=192, sy+=127;
			cur[rect#Input]		= Class.rect(239,sy,352,74);
			cur[rect#Button]		= Class.rect(588,sy,84,74);
			cur[imgButton]		="${imagePath}/admin/cancel_search_[#].png";
		case Form1:
			sx=350, sy=296, sw=320, sh=94, sy+=127;
			while( row, 3 ) {
				cur[rect#$row]	= Class.rect(sx,sy,sw,sh);
				sy+=sh;
			}
		case Form2:
			sx=350, sy=791, sw=686, sh=190, sy+=127;
			sw/=2, sh/=2;
			while( row, 2 ) {
				cx=sx;
				while( col, 2 ) {
					cur[rect#$row $col]=Class.rect(cx,sy,sw,sh);
					cx+=sw;
				}
				sy+=sh;
			}
			cur[imgAllOut]		= "${imagePath}/admin/turnout_all_btn_[#].png";
			cur[imgReset]		= "${imagePath}/admin/turnout_reset_btn_[#].png";
		case NumberPad:
			sx=700, sy=190, sy+=127;
			bw=104, bh=89;
			num=1;
			while( row,4 ) {
				cx=sx;
				while( col,3 ) {
					if( row.eq(3) ) {
						switch(col) {
						case 0:	img="sub_t_num_00_[#].png";
						case 1:	img="sub_t_num_del_[#].png";
						case 2:	img="sub_t_num_re_[#].png";
						}
					} else {
						img="sub_t_num_0${num}_[#].png";
						num++;
					}
					cur[rect#$row $col]=Class.rect(cx, sy, bw, bh), cx+=bw+10;
					cur[img#$row $col]="${imagePath}/admin/$img";
				}
				sy+=bh+10;
			}
		case TurnOutButton:
			sy=602, sy+=127;
			cur[rect]		= Class.rect(700,sy,334,111);
			cur[src]		= "${imagePath}/admin/turnout_btn_[#].png";
		default:
		}
	}
}
TurnOutView.draw(draw) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case Search:
			 rc=cur[rect#Button];
			 var=when( rc.eq(this.mouseDownRect), 'p', 'n');
			 img=imageLoad(cur, 'imgButton', var);
			 draw.drawImage(rc, img);
		case Form1:
			while( row, 3 ) {
				rc=cur[rect#$row];
				draw.rectLine( rc.incr(5), 0, '#c0c0c0',2);
			}
		case Form2:
			while( row, 3 ) {
				while( col, 2 ) {
					rc=cur[rect#$row $col];
					switch( col ) {
					case 0: 	img=imageLoad(cur,'imgAllOut');
					case 1:	img=imageLoad(cur,'imgReset');
					}
					draw.drawImage( rc.incr(5), img);
				}
			}
		case NumberPad:
			while( row,4 ) {
				while( col,3 ) {
					rc=cur[rect#$row $col];
					var=when( rc.eq(this.mouseDownRect), 'p', 'n');
					img=imageLoad(cur, "img#$row $col", var);
					draw.drawImage(rc, img);
				}
			}
		case TurnOutButton:
			rc=cur[rect];
			var=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage(rc, imageLoad(cur,'src',var) );
		default:
		}
	}
}
TurnOutView.mouseDown(pos) {

}
TurnOutView.mouseUp(pos) {

}

AdminLogin.setLoginConfig(storeNo, password) {
	not( storeNo ) {
		this.messageBox("사용자 정보가 입력되지 않았습니다.", "알림");
		return;
	}
	not( password ) {
		this.messageBox("비밀번호가 입력되지 않았습니다.", "알림");
		return;
	}
	db=Class.db('kiosk');
	not( db.open() ) {
		cf.inject(projectId, storeCode);
		path="project/$projectId/data/${storeCode}.db";		_log("# db open path => $path");
		not( storeCode ) return;
		db.open(path);
	}
	cf.put(storeNo, password);
	not( db.count("select count(1) from tb_store_mst where store_no=#{storeNo}", cf) ) {
		this.messageBox("사용자 정보가 없습니다.", "알림");
		return;
	}
	not ( db.count("select count(1) from tb_store_mst where store_no=#{storeNo} and local_pw=#{password}", cf) ) {
		this.messageBox("비밀번호가 일치하지 않았습니다.", "알림");
		return;
	}
	db.fetch("select store_nm, tel, addr1 as addr from tb_store_mst where store_no=#{storeNo}", cf);
	this.update();
	this.goPage("AdminHome");
	Cf[LoginConfig]=cf;
}

SoldOutView.test() {
	this.initControl();
	this.conf();
	this.update();
}
SoldOutView.soldOutClick(sub) {
	if( sub[use_yn].eq('Y') ) {
		sub[use_yn]='N';
	} else {
		sub[use_yn]='Y';
	}
	db=Class.db('kiosk');
	db.exec("update tb_menu_mst set use_yn=#{use_yn} where menu_cd=#{menu_cd}", sub);
	this.update();
}

AdminTitle.AdminTitle(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
AdminTitle.initControl() {
	not( tag[type] ) tag[type]='vbox';
	tag.removeAll();
	tag.addNode({tag: StoreTitle} );
	tag.addNode({tag: CurrentTime} );
	tag.addNode({tag: CurrentDate} );
	tag.addNode({tag: CurrentPosNo} );
	login=Cf[LoginConfig];
	if( login ) cf.varMap(login, 'store_nm, addr');
}
AdminTitle.conf() {
	while( cur, tag ) {
		switch( cur[tag]) {
		case StoreTitle: 			cur[rect]=Class.rect(35,31,658,78);
		case CurrentTime:		cur[rect]=Class.rect(864,17,200,111);
		case CurrentPosNo:	cur[rect]=Class.rect(680,31,174,45);
		case CurrentDate:		cur[rect]=Class.rect(680,64,174,45);
		}
	}
}
AdminTitle.draw(draw, timeline) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch( cur[tag]) {
		case StoreTitle:
			if( cf[store_nm] ) drawNodeText(draw, cur[rect], cf[store_nm], 'left', 'PopupTitle');
		case CurrentTime:
			drawNodeText(draw, cur[rect],System.date('HH:mm:ss'), 'left', 36, '#f0f0f0', 'bold');
		case CurrentPosNo:
			drawNodeText(draw, cur[rect],cf[storeNo], 'left', 22, '#f0f0f0');
			draw.rectLine(cur[rect], 4, '#d0d0d0');
		case CurrentDate:
			drawNodeText(draw, cur[rect],System.date('yyyy-MM-dd'), 'left', 16, '#f0f0f0');
		}
	}
}
AdminTitle.mouseDown(pos) {

}
AdminTitle.mouseUp(pos) {

}

AdminPage.AdminPage(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
AdminPage.initControl() {
	setNodeSize(tag, true);
}
AdminPage.conf() {
	confNodeLayout(tag);
	while( cur, tag ) this.getControl(cur).conf();
}
AdminPage.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) this.getControl(cur).draw(draw, tm);
}
AdminPage.mouseDown(pos) {
	while( cur, tag ) this.getControl(cur).mouseDown(pos);
}
AdminPage.mouseUp(pos) {
	while( cur, tag ) this.getControl(cur).mouseUp(pos);
}

Popup.stackPageLoad(pageId) {
	cur=tag.findOne('id', pageId);
	not( cur ) {
		src=conf("page#xml.kiosk#$pageId");
		not( src ) {
			return;
		}
		cf.inject(imagePath, projectId, pageCode);
		cur= this.parseXml( fmt(src), tag );
		cur[id]=pageId;
	}
	this.stackPageUpdate(cur);
}
Popup.stackPageUpdate(cur) {
	this.mainNode=cur;
	ctrl=this.getControl(cur);
	/* 자식에 설정된 영역 삭제 */
	tagClearRect(cur);
	tag[rect]=this.mainControl().getPageRect();
	cur[rect]=tag[rect];
	cf[stackPage]=this;
	this.conf();
	/* 페이지 시작 효과처리 */
	if( ctrl.pageOpenEvent ) {
		this.timelineStart('PageEffect',  this, 'open');
	} else {
		this.update();
	}
}

protocalTest.protocalTest(page) {
	this.addClass(common.Config, dev.EditorSrcChange, dev.EditorSrcClick );
	db=Class.db('config');
	/* #################### Tree #################### */
	tree=page.tree;
	tree.check('treeMode', true);
	tree.model(Class.model('protocalTestTree'), 'tag');
	tree.eventMap(onContextMenu, this.treeContentMenu, 'pos');
	tree.eventMap(onDraw, this.treeDraw, 'draw, node, over');
	tree.eventMap(onChange, this.treeChange, 'node');
	tree.eventMap(onMouseClick, this.treeMouseClick, 'pos, button');
	/* #################### Grid #################### */
		grid=page.grid;
	dataModel=Class.model('protocalTestGrid');
	grid.check('sortEnable', true);
	/* 그리드 이벤트 맵핑  */
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onChange, this.gridChange, 'node');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	this.initPage();
	/* 그리드 헤더폭을 자동 계산  */
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
	/* #################### Button #################### */
	deleteButton=page.deleteButton;
	deleteButton.eventMap( onClick, this.deleteButtonClick );
	/* #################### editor #################### */
	editor=  page.src;
	not( editor ) return;
	editor.syntax( conf('syntax.sql') );
	editor.eventMap( onMouseClick, this.editorMouseClick, 'pos, keys' );
	editor.eventMap( onChange, this.editorChange );
	editor.eventMap( onKeyDown, this.editorKeyDown, 'key,mode' );
}
protocalTest.initPage() {
	this.initTree();
	this.initGrid();
}
protocalTest.initTree() {
	root=tree.rootNode().removeAll();
	root.addNode({ tag:root, title: 드림하이테크 전문 응답 테스트, depth:0});
	tree.update();
}
protocalTest.contextReloadNode() {
	node=this.contextNode;
}
protocalTest.contextDeleteNode() {
	node=this.contextNode;
}
protocalTest.treeContentMenu(pos) {
	node=tree.at(pos);
	not( node ) return;
	this.contextNode=node;
	not( page.action('protocalTest.reload') ) {
		pageActionAdd(page, 'protocalTest.reload', '새로고침',
			'vicon.arrow_rotate_clockwise', this.contextReloadNode);
		pageActionAdd(page, 'protocalTest.delete', '노드삭제',
			'vicon.cancel_default', this.contextDeleteNode);
		pageActionAdd(page, 'protocalTest.add', '자식코드 등록',
			'vicon.add_defalut', this.contextAddCode);
		pageActionAdd(page, 'protocalTest.download', '다운로드',
			'ficon.inbox-download', this.downloadImages);
		pageActionAdd(page, 'protocalTest.makeData', '데이터생성',
			'vicon.database_go', this.makeProtocalData);
	}
	str="protocalTest.reload, -, protocalTest.delete";
	str.add(",-,protocalTest.download, protocalTest.makeData");
	print("treeContentMenu=> $str");
	page.menu(str, pos.incrY(14) );
}
protocalTest.treeDraw(draw, node, over) {
	rc=treeIcon(tree, draw, node, over);
	rcIcon = rc.width(18).center(16,16);
	rc.incrX(20);
	switch( node[tag] ) {
	case root:
		draw.icon( rcIcon, "vicon.application_form" );
		draw.save().font('bold');
		draw.text( rc,  node[title]);
		draw.restore();
	default:
		switch(node[depth]) {
		case 1:	icon='vicon.application_side_boxes';
		case 2:	icon='vicon.application_view_list';
		case 3:	icon= 'vicon.page_red';
		default: 	icon= 'vicon.page_red';
		}
		draw.icon( rcIcon, icon );
		draw.text( rc,  node[tag]);
	}
}
protocalTest.treeMouseClick(pos, button) {
	if( button.eq('right') ) return 'ignore';
}
protocalTest.treeChange(node) {
	cur=null;
	if( node[tag].eq('HEADER') ) {
		cur=node;
	} else {
		cur=findTag('HEADER', node);
	}
	sub=cur.child(0);
	if( sub[tag].eq('DETAIL') ) {
		data=sub.child(0);
		fields=grid.fields();
		fields.removeAll();
		/* 테이블 생성 및 저장
		this.makeKioskData(node[tag], data[fieldsArray], sub);
		*/
		while( k, data[fieldsArray] ) {
			field=fields.addNode();
			field[code]=k;
			field[text]=k;
			w1=textWidth(12, data[$k]), w2=textWidth(12, k);
			field[width]=max(w1, w2);
		}
		grid.fields(fields);
		grid.change(sub);
	} else {
		grid.update();
	}
	total=grid.rootNode().childCount();
	page[gridStatus].value("(총 ${total} 건)");
}
protocalTest.treeFilter(node) {
	return true;
}
protocalTest.initGrid() {
	fields=grid.fields();
	gridMakeField( conf('data#fields.protocalTest'),true, fields);
	grid.model( dataModel, fields);
	gridHeaderWidth(grid);
}
protocalTest.searchGrid() {
	root=grid.rootNode();
	/* 조회 쿼리를 넣어준다*/
	db.fetchAll("", root.removeAll() );
	grid.update();
	page.deletePage.hide();
	gridHeaderWidth(grid);
}
protocalTest.gridChange(node) {

}
protocalTest.gridDoubleClick(node) {

}
protocalTest.gridResize() {
	gridHeaderWidth(grid);
}
protocalTest.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
protocalTest.gridClick(node, column) {
	grid.edit(node, column);
}
protocalTest.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}
protocalTest.responseXML(data, combo) {
	val=combo.value();
	not( val ) {
		return;
	}
	root=tree.rootNode().child(0);
	tree.expand(root, true);
	cur=root.findOne("tag", val );
	not( cur ) {
		cur=root.addNode();
		cur[tag]=val;
		cur[type]='protocal';
	}
	this.parseXml( data, cur.removeAll() );
	tree.current(cur);
	tree.update();
	editor.append(data);
	if( page[autoApply].checked() ) {
		page.postEvent(1, cur);
	}
}
protocalTest.responseFinish(url) {

}
protocalTest.parseXml(&data, node, fiistNode) {
	not( node ) {
		node=xmlNode;
		node.removeAll();
	}
	while( data.valid() ) {
		ch=data.ch();
		not( ch.eq('<') ) {
			break;
		}
		if( data.ch(1).eq('!') ) {
			data.match('<!--','-->');
			continue;
		}
		if( data.ch(1).eq('?') ) {
			data.match('<?','?>');
			continue;
		}
		sp=data.cur();
		tag=data.incr().move();
		sub = node.addNode();
		not( fiistNode ) {
			fiistNode=sub;
		}
		if( data.ch().eq('-') ) {
			sub[kind]=data.incr().move();
			print("tag--->$tag, $kind");
		}
		if( tag.eq('br', 'space', 'image') ) {
			prop=data.findPos(">");
			this.parseProp( sub, tag, prop);
		} else {
			in=data.find('>');
			if( in.ch(-1).eq('/') ) {
				prop=data.findPos('/>');
				this.parseProp( sub, tag, prop);
			} else {
				data.pos(sp);
				if( sub[kind] ) {
					in=data.match("<$tag-$sub[kind]","</$tag-$sub[kind]>");
				} else {
					in=data.match("<$tag","</$tag>",8);
				}
				not( in ) {
					print("@@ xml parse $tag not match");
					in=data.findPos("</$tag>");
				}
				prop=in.findPos(">");
				this.parseProp( sub, tag, prop);
				if( tag.eq('html', 'text') ) {
					val=in.trim();
					if( val ) sub[data]=val;
				} else {
					if( in.ch().eq('<') ) {
						this.parseXml(in, sub, fiistNode);
					} else {
						val=in.trim();
						if( val ) sub[data]=val;
					}
				}
			}
		}
	}
	return fiistNode;
}

PageBase.test() {
	x=this.findControl('#CornerTab');
	print();
}

ListBox.changeTab(tab, bconf) {
	not( tab ) {
		return;
	}
	/* 이전 탭정보 저장 */
	tag[prevTab] = tag[currentTab];
	/* view_type에따라 보여질 메뉴수를 결정 */
	tag.inject(rowCnt, cellCnt);
	if( cf[view_type].eq('A') ) {
		gridCnt	= rowCnt*cellCnt;
	} else {
		gridCnt	= 4;
	}
	totalCnt= tab.childCount();
	pageCnt= totalCnt/gridCnt, mod=totalCnt%gridCnt;
	if( mod ) {
		pageCnt++;
	}
	/* 메뉴 페이지 설정(메모리 DC)
		[tab 설정값]
			- pageNode:		페이지 그리기 정보
			- pageCount: 		전체 페이지 수
			- currentPage:	현재 페이지 변호 (전체 페이지중)
			- gridCount: 		한페이지 그리드 수
	*/
	ignore=false;
	pageNode	= tab[pageNode];
	if( pageNode ) {
		/* 메뉴수가 변경된 경우 처리 */
		if( pageCnt != pageNode.childCount() ) {
			dist= pageCnt - pageNode.childCount();
			if( dist>0 ) {
				while( num, dist ) pageNode.addNode();
			}
		} else {
			/* 변경된 내용이 없으므로 무시한다.*/
			ignore=true;
		}
	} else {
		/* 처음 탭 페이지 생성 */
		print("ListBox::changeTab : 페이지수=$pageCnt, 그리드수=$gridCnt");
		pageNode={};
		while( num, pageCnt ) {
			pageNode.addNode();
		}
		tab[pageNode]=pageNode;
	}
	tab[currentPage]=0;
	tab.put( gridCnt, pageCnt );
	tag[currentTab]=tab;
	/* 페이지 네이게이션 처리 */
	this.findControl('MenuList#pageView').changeTab(tab);
	/* 무조건 새로그리도록 수정  =================================
	if( ignore ) {
		return;
	}
	============================================================ */
	/* 페이지 영역 초기화 및 메뉴그리기 */
	while( page, pageNode, num, 0 ) {
		page.state(0);
		page[pageNum]=num;
		if( page[drawObject] ) {
			page[drawObject].fill();
		} else {
			print("리스트 DrawObject 생성 : $tag[rect]");
			page[drawObject]=Class.draw( tag[rect].size() );
		}
	}
	if( bconf ) {
		return;
	}
	this.conf();
}
ListBox.makeListRect(tab) {
	cf.inject( imagePath );
	not( tab ) tab=tag[currentTab];
	space=tag[space];
	margin=nvl(space,0), margin/=2;
	rc=tag[rect].incr(margin);
	rc.inject(sx, sy, sw, sh);
	idx=0;
	rowCnt=4;
	ra=rateArr(sh,rowCnt);
	while( num, tab[pageCnt] ) {
		cy=sy;
		while( h, ra , row, 0 ) {
			menu=tab.child(idx), idx++;
			not( menu ) {
				menu=tab.addNode({tag:menu});
			}
			menu[rect]=Class.rect(sx, cy, sw, h).incr(space);
			cy+=h;
		}
	}
}
ListBox.makeThumbRect(tab) {
	not( tab ) tab=tag[currentTab];
	space=tag[space];
	margin=nvl(space,0), margin/=2;
	rc=tag[rect].incr(margin);
	rc.inject(sx, sy, sw, sh);
	idx=0;
	tag.inject(rowCnt, cellCnt);
	ra=rateArr(sh,rowCnt), ca=rateArr(sw,cellCnt);
	while( num, tab[pageCnt] ) {
		cy=sy;
		while( h, ra, row, 0 ) {
			cx=sx;
			while( w, ca, cell, 0 ) {
				menu=tab.child(idx), idx++;
				not( menu ) {
					continue;
				}
				menu[rect]=Class.rect(cx,cy,w,h).incr(space);
				cx+=w;
			}
			cy+=h;
		}
	}
}
ListBox.getPage(tab) {
	not( tab ) tab=tag[currentTab];
	if( tab && tab[pageNode] ) {
		return tab[pageNode].child( tab[currentPage] );
	}
	return null;
}
ListBox.drawPage(tab) {
	not( tab ) tab=tag[currentTab];
	page=this.getPage(tab);
	not( page ) {
		_log("Menu ListBox :: drawPage:: <current page is null>");
	}
	offset=tag[rect].lt();
	if( cf[view_type].eq('A') ) {
		/* 메뉴 아이템 박스 이미지 처리 */
		not( tag[drawThumbBox] ) {
			menu=tab.child(0);
			if( menu[rect] ) {
				this.makeThumbBox(menu[rect] );
			}
		}
		this.drawThumbPage(page.drawObject, tab, offset);
	} else {
		this.drawListPage(page.drawObject, tab, offset);
	}
	page.state(NODE.set, true);
}
ListBox.drawListPage(draw ,tab, offset) {
	tab.inject(gridCnt, currentPage);
	sp=gridCnt*currentPage;
	ep=sp+gridCnt;
	draw.mode();
	draw.fill();
	lang=Cf[KioskLangSelect].lower();
	while( n, ep, sp ) {
		menu=tab.child(n);
		not( menu ) break;
		if( offset ) {
			rect=menu[rect].incrXY(offset,false);
		} else {
			rect=menu[rect];
		}
		getRectArr(menu, rect, '350,*,200', 'hbox', 'menuRects').inject(rcImage, rcText, rcPrice);
		drawRowBox=tag[drawRowBox];
		not( drawRowBox ) {
			drawRowBox=Class.draw(rect.size());
			drawRowBox.mode();
			r=Class.rect(0,0,rect.size());
			drawRowBox.effect( DRAW.RoundBox, r, 5, '#101010', '#2a2523', 2 );
			tag[drawRowBox]=drawRowBox;
		}
		draw.opacity(40);
		draw.drawImage( rect, drawRowBox );
		draw.opacity(15);
		draw.fill(rcImage, '#ffffff');
		draw.rectLine(rcImage, 3, '#cacaca');
		draw.opacity(100);
		if( menu[menu_cd] ) {
			img=loadMenu(cf, menu);
			not( img ) {
				img=commonImage('no_img');
			}
			draw.drawImage(rcImage, img);
			menuName=menu[$lang];
			menu[rcImage]=rcImage;
			menu[lang]=menuName;
			drawNodeText( draw, rcText, menuName, 'left', 'MenuName' );
			/*
			getRectArr(menu, rcText.incrX(25), '*,100', 'vbox', 'menuRects').inject(a,b);
			drawNodeText( draw, a, menu[menu_nm], 'left', 'MenuName' );
			drawNodeText( draw, b, menu[menu_de], 'left', tag[FontDesc] );
			*/
			price=util_priceComma(menu[sale_price]);
			drawNodeText( draw, rcPrice, "$price 원", 'left', 'MenuPrice' );
			/* 품절 및 판매중지 처리 */
			if( menu[sale_ok] ) {
				if( menu[sold_yn].eq('Y') ) {
					draw.fill(rcImage, '#c0c0caba');
					draw.font(42,'bold','#282827','나눔바른고딕').text(rcImage, "준비중", "center");
					draw.font(42,'bold','#f0f0f050','나눔바른고딕').text(rcImage.move(1,1), "준비중", "center");
				}
			} else {
				draw.fill(rcImage, '#c0c0caba');
				draw.font(42,'bold','#282827','나눔바른고딕').text(rcImage, "준비중", "center");
				draw.font(42,'bold','#f0f0f050','나눔바른고딕').text(rcImage.move(1,1), "준비중", "center");
			}
		} else {
			img=imageLoad(tag, 'MenuBlankImage');
			draw.drawImage( img.center(rcImage), img);
		}
	}
}
ListBox.drawThumbPage(draw, tab, offset) {
	tab.inject(gridCnt, currentPage);
	sp=gridCnt*currentPage, ep=sp+gridCnt;
	draw.mode();
	draw.fill();
	lang=Cf[KioskLangSelect].lower();
	while( n, ep, sp ) {
		menu=tab.child(n);
		not( menu ) {
			 continue;
		}
		if( offset ) {
			rect=menu[rect].incrXY(offset,false);
		} else {
			rect=menu[rect];
		}
		/* 메뉴 외부 박스 배경
		draw.drawImage( rect, tag[drawThumbBox] );
		*/
		if( menu[menu_cd] ) {
			getRectArr(menu, rect, '195,30,*', 'vbox', 'menuRects').inject(rcImage, rcText, rcPrice);
			/* 외부 박스 그리기
			iw=rcImage.width(), ih=rcImage.height(), ih-=15;
			draw.drawImage( rect, tag[drawThumbImgBox], 0, 0, iw, ih );
			*/
			draw.drawImage( rect, commonImage('menu_box') );
			/* 메뉴 이미지 그리기 */
			rc=rcImage.incrH(-13), img=loadMenu(cf,menu);
			not( img ) {
				img=commonImage('no_img');
			}
			draw.drawImage( rc, img);
			menu[rcImage]=rc;
			/* 이미지 경계라인 그리기*/
			draw.rectLine( rc, 4, '#cac0c0');
			/* 추천, 신규 아이콘 출력 */
			if( menu[disp_type].eq('01','02') ) {
				rc=Class.rect( rcImage.lt(), 89,64), img=imageLoad( tag, "Icon_DispType${menu[disp_type]}");
				draw.drawImage( rc.move(5,5), img);
			}
			menuName=menu[$lang];
			menu[lang]=menuName;
			/* 메뉴명 가격 출력 */
			price=util_priceComma(menu[sale_price]);
			drawNodeText( draw, rcText, menuName, 'center', 'MenuName');
			drawNodeText( draw, rcPrice, price, 'center', 'MenuPrice');
			/* 품절 및 판매중지 처리 (2017-03-06 폰트크기 조정) */
			if( menu[sale_ok] ) {
				if( menu[sold_yn].eq('Y') ) {
					draw.fill(rcImage.incrH(-13), '#c0c0caba');
					draw.font(42,'bold','#282827','나눔바른고딕').text(rcImage, "준비중", "center");
					draw.pen('#f0f0f050').text(rcImage.move(2,2), "준비중", "center");
				}
			} else {
				draw.fill(rcImage.incrH(-13), '#c0c0caba');
				draw.font(42,'bold','#282827','나눔바른고딕').text(rcImage, "준비중", "center");
				draw.pen('#f0f0f050').text(rcImage.move(2,2), "준비중", "center");
			}
		}
		/* 메뉴 없을경우
		else {
			img=imageLoad(tag, 'MenuBlankImage');
			draw.drawImage( img.center(rect), img);
		}
		*/
	}
}

CanvasBase.timelineAdd(tid, duration, range, mode) {
	tm=timelineNode.findOne('tid',tid);
	not( tm ) {
		tm=timelineNode.addNode();
	}
	tm[tid]=tid;
	tm[startTick]=System.tick();
	tm.put(duration, range, mode);
}

ListBox.selectMenu(menu) {
	if( Cf.timeLine('SelectMenu.running') ) {
		Cf.timeLine('SelectMenu.stop');
	}
	print("메뉴선택: $menu[menu_nm]");
	tab=menu.parent();
	this.findControl('MenuCart#orderView').addMenu(menu);
	this[currentSelectMenu]=menu;
	this.mainControl().timelineStart('SelectMenu', tag, 'AddCart');
}

PagePanel.changeTab(tab) {
	this[currentTab]=tab;
	/* 현재 탭의 페이지 갯수 */
	total=tab[pageNode].childCount();
	/* 페이지 출력 전체영역 */
	pageNavi = tag.child(1);
	/* 페이지 갯수에따른 출력영역 */
	pw=total*60;
	this[pageNaviRect] = pageNavi[rect].center(pw, 55);
}

ShoppingCart.drawHeader(header) {
	not( header ) {
		header =this.findTag('Header');
	}
	drawObject=getDrawObject(this, 'HeaderDrawObject',  header[rect] );
	drawObject.fill();
	sx=0, sy=0, sh=header[rect].height();
	arr=this[HeaderWidthArray];
	lang=Cf[KioskLangSelect].lower();
	if( lang.eq('kor') ) {
		lang_0="메뉴", lang_1='수량', lang_2='가격';
	} else {
		lang_0="Menu", lang_1='Qty.', lang_2='Price';
	}
	while( w, arr, c, 0 ) {
		rc=Class.rect(sx,sy,w,sh), sx+=w;
		switch(c) {
		case 0:
			drawObject.fill(rc, '#E8E5E0');
			drawNodeText(drawObject, rc.incrX(80), fmt("lang_$c",true), "left", 'TableHeader');
		case 1:
			drawObject.fill(rc, '#E8E5E0');
			drawNodeText(drawObject, rc, fmt("lang_$c",true), "center", 'TableHeader');
		case 2:
			rcBk=rc.incrW( arr.get(3) );
			drawObject.fill(rcBk, '#E8E5E0');
			drawNodeText(drawObject, rc, fmt("lang_$c",true), "center", 'TableHeader');
			break;
		default:
		}
	}
}
ShoppingCart.drawList(list, redraw) {
	this.inject(pageNode, currentPageBlock, rowCount, rcList );
	totalCount=list.childCount();
	not( totalCount ) {
		print("# drawList invalid ( totalCount==0 )");
		return;
	}
	pageBlockCount=totalCount/rowCount, mod=totalCount%rowCount;
	if( mod ) {
		pageBlockCount++;
	}
	/* 페이지 정보 계산 */
	rcList.inject(sx, sy, sw, sh);
	if( pageBlockCount<1 ) {
		print("# drawList invalid ( rcList=>$rcList, pageBlockCount=$pageBlockCount )");
	}
	offset=Class.point(sx,sy);
	page=this.getPage( pageBlockCount-1);
	not( page ) {
		page=pageNode.addNode({tag:PageNode});
		page[drawObject]=Class.draw(rcList);
	}
	/* 페이지 정보 리셋 */
	startRow=0;
	while( n, pageBlockCount ) {
		page=pageNode.child(n);
		page[startRow]=startRow, startRow+=rowCount;
		page[endRow]=min(totalCount,  startRow);
		page[endPage]=when( page[endRow].eq(totalCount), true, false);
		if( redraw || n.eq(currentPageBlock) ) {
			this.drawListPage(list, page);
		}
		if( page[endPage] ) break;
	}
	this[pageBlockCount]=pageBlockCount;
	this.drawScrollBar(list);
}
ShoppingCart.drawListPage(list, page) {
	/* 주문내역 페이지 처리(페이지당 주문 5개)*/
	page.inject(startRow, endRow, drawObject);
	offset=this[rcList].lt();
	rc=drawObject.rect();
	dw=rc.width(), dh=this[ListHeight], dx=0, dy=0;
	drawObject.fill('#ffffff');
	warr=this[HeaderWidthArray];
	if( warr.size()<3 ) {
		print("drawListPage HeaderWidthArray not valid======$warr [rect width:$dw]");
		warr.recalc( dw, '486,168,250,176');
	}
	while( n, endRow, startRow ) {
		/* List 태그의 자식 영역 세팅 & 메모리에 주문내역 그리기 */
		row=list.child(n);
		dx=0;
		rcRow=Class.rect(dx, dy, dw, dh);
		row[rect]=rcRow.incrXY(offset);
		while( w, warr, c, 0 ) {
			rcCell=Class.rect(dx, dy, w, dh), dx+=w;
			switch( c ) {
			case 0:
				/* 메뉴 */
				menuName=row[lang];
				not( menuName ) menuName=row[menu_nm];
				drawNodeText(drawObject, rcCell.incrX(20), menuName, 'left', 'TableList');
			case 1:
				/* 수량 */
				qx=rcCell.x();
				while( qw, this[QtyRateArray], x, 0 ) {
					r=Class.rect(qx, dy, qw, dh), qx+=qw;
					switch(x) {
					case 0:
						r0=r.center(45,45);
						ty=when( row[qty].eq(1), 'd', 'n' );
						drawNodeImage(drawObject, r0, tag, 'OrderMinusImage', ty);
						row[rcQtyMinus]=r0.incrXY(offset);
					case 1:
						r0=r.center( r.width(), 45);
						drawObject.rectLine(r0, 0, '#c0c0c0', 2);
						drawNodeText(drawObject, r0, row[qty], 'center', 'TableList');
						row[rcQty]=r0.incrXY(offset);
					case 2:
						r0=r.center(45,45);
						drawNodeImage(drawObject, r0, tag, 'OrderPlusImage', 'p');
						row[rcQtyPlus]=r0.incrXY(offset);
					}
				}
			case 2:
				/* 가격 */
				qx=rcCell.x();
				while( qw, this[PriceRateArray], x, 0 ) {
					r=Class.rect(qx, dy, qw, dh), qx+=qw;
					switch(x) {
					case 0:
						row.inject(sale_price, qty);
						sum=sale_price*qty;
						r0=r.center( r.width(), 45);
						price=util_priceComma(sum);
						drawNodeText(drawObject, r0, "$price 원", 'right', 'TableList');
					case 1:
						r0=r.center(45,45);
						drawNodeImage(drawObject, r0, tag, 'OrderDeleteImage', 'p');
						row[rcRowDelete]=r0.incrXY(offset);
					}
				}
			default:
			}
		}
		drawObject.rectLine(rcRow, 4, '#d0d0d0',1 );
		dy+=dh;
	}
}
ShoppingCart.drawScrollBar(list) {
	not( list ) {
		list=findTag('List', tag);
	}
	this.inject(rcScrollBar, ScrollHeightArray, currentPageBlock, pageBlockCount, rcUp, rcDown);
	not( rcScrollBar.valid() ) {
		print("drawScrollBar rect invalid=>$rcScrollBar");
		this.mainControl().confMain();
		return;
	}
	drawObject=this[ScrollDrawObject];
	not( drawObject ) {
		print("drawScrollBar rect=>$rcScrollBar");
		drawObject=Class.draw(rcScrollBar.size());
		this[ScrollDrawObject]=drawObject;
	}
	drawObject.fill();
	print("# drawScrollBar => start");
	if( rcUp && rcDown ) {
		ty='d';
		if( currentPageBlock>0 ) {
			ty='n';
		}
		img=imageLoad(tag,'ScrollUp', ty);
		drawObject.drawImage(rcUp, img);
		ty='d', last=pageBlockCount-1;
		if( currentPageBlock<last ) {
			ty='n';
		}
		img=imageLoad(tag,'ScrollDown', ty);
		drawObject.drawImage(rcDown, img);
	} else {
		offset=rcScrollBar.lt();
		rc=drawObject.rect();
		rc.inject(sx,sy,sw,sh);
		while( h, ScrollHeightArray, n, 0 ) {
			rcCur=Class.rect(sx, sy, sw, h), sy+=h;
			print("# drawScrollBar = $rcCur");
			switch(n) {
			case 0:
				ty='d';
				if( currentPageBlock>0 ) {
					ty='n';
				}
				img=imageLoad(tag,'ScrollUp', ty), rc=img.center(rcCur);
				this[rcUp]=rc, this[rcScrollUp]=rc.incrXY(offset);
				drawObject.drawImage(rc, img);
				last=pageBlockCount-1;
			case 2:
				ty='d';
				if( currentPageBlock<last ) {
					ty='n';
				}
				img=imageLoad(tag,'ScrollDown', ty), rc=img.center(rcCur);
				this[rcDown]=rc, this[rcScrollDown]=rc.incrXY(offset);
				drawObject.drawImage(rc, img);
			default:
			}
		}
	}
	print("# drawScrollBar => end");
}
ShoppingCart.addMenu(menu, itemQty) {
	list=this.findTag('List');
	not( list ) {
		print("# addMenu list not valid [tag=> $tag]");
		this.mainControl().confMain();
		return;
	}
	not( cf[orderItemList] ) {
		cf[orderItemList]=list;
	}
	menu.inject( corner_cd, menu_cd, menu_nm, sale_price, lang);
	row=list.findOne('menu_cd', menu_cd);
	if( row ) {
		if( itemQty ) {
			row[qty+=itemQty];
		} else {
			row[qty++];
		}
	} else {
		row=list.addNode({tag:OrderItem});
		row[qty]=nvl(itemQty, 1);
		row.put(corner_cd, menu_cd, menu_nm, sale_price, lang);
		not( row[corner_cd] ) {
			p=menu.parent();
			if( p ) {
				row[corner_cd]=p[corner_cd];
			}
		}
	}
	this.findControl('#MenuCart').setCurrentPage('ShoppingCart');
	this.currentRow=row;
	this.recalcList(list);
	this.inject( currentPageBlock, rowCount );
	not( rowCount ) {
		print("#addMenu rowCount == 0 ");
		return;
	}
	this[prevPageBlock]=currentPageBlock;
	this[currentPageBlock]=row.index()/rowCount;
	this.drawList(list);
	/*
	this.movePage();
	*/
}
ShoppingCart.getOrderList() {
	list=findTag('List', tag);
	return list;
}
ShoppingCart.getPage(block) {
	not( isset(block) ) block=this[currentPageBlock];
	return this[pageNode].child(block);
}
ShoppingCart.movePage() {
	this.inject(prevPageBlock, currentPageBlock);
	this.update();
}
ShoppingCart.recalcList(list) {
	totalQty=0, totalPrice=0;
	not( list ) list=findTag('List', tag);
	while( row, list ) {
		row.inject(sale_price, qty);
		sum=sale_price * qty;
		row[sum_price]=sum;
		totalPrice+=sum;
		totalQty+=qty;
	}
	this[OrderTotalQty]=totalQty;
	this[OrderTotalPrice]=totalPrice;
	this.update();
}
ShoppingCart.removeAllMenu() {
	list=findTag('List', tag);
	if( list.childCount() ) {
		list.removeAll();
		this.recalcList(list);
		this.findControl('#MenuCart').setCurrentPage('AdPanel');
	}
}
ShoppingCart.removeMenu(menu) {
	list=findTag('List', tag);
	row=list.findOne('menu_cd', menu[menu_cd]);
	upScroll=false;
	if( row ) {
		idx=row.index();
		if( idx >0 ) {
			mod= idx%5;
			if( mod.eq(0) ) {
				upScroll=true;
			}
		}
		if( row==this[currentRow] ) {
			this[currentRow]=null;
		}
		list.remove(row);
		not( list.childCount() ) {
			this.recalcList(list);
			this.findControl('#MenuCart').setCurrentPage('AdPanel');
			return;
		}
	}
	this.recalcList(list);
	this.drawList(list, true);
	if( upScroll ) {
		this.mainControl().timelineStart('ShoppingCart', node, 'ScrollUp');
	}
}
ShoppingCart.setCurrentPage(pageTag) {
	gridNode		= tag.parent();
	cur				= findTag(pageTag, gridNode);
	if( pageTag.eq('ShoppingCart') ) {
		while( page, cf[didWidgets] ) {
			page.hide();
		}
	}
	this.getControl(gridNode).setCurrentPage(cur);
}

MainLoading.MainLoading(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MainLoading.initControl() {
	cf.inject(imagePath);
	tag[TitleImage]		="${imagePath}/main/intro_logo.png";
	tag[FooterImage]	="${imagePath}/type/intro_logo.png";
	tag.addNode({tag:TitleBox, Height:400});
	tag.addNode({tag:LoadingBox, Height:300});
	tag.addNode({tag:Fotter});
}
MainLoading.conf() {
	setNodeSize(tag, true);
	confNodeLayout(tag);
}
MainLoading.draw(draw, tm) {
	show=true;
	if( tm ) {
		frame= this.drawFadeIn(draw, tm.tid );
		if( frame<15 ) show=false;
	}
	draw.mode();
	draw.effect(
		DRAW.RoundBox, tag[rect].incr(2), 15, '#cacaca', '#ffffff', 2
	);
	while( cur, tag ) {
		switch(cur[tag] ) {
		case TitleBox:
			img=imageLoad(tag,'TitleImage');
			draw.drawImage(img.center(cur[rect]), img);
		case LoadingBox:
			if( show ) this.showLoading(cur[rect]);
		case Footer:
			img=imageLoad(tag,'FooterImage');
			img.imageSize().inject(w,h);
			rc=cur[rect].move('bottom', h+40);
			draw.drawImage(rc.center(w,h), img);
		}
	}
}
MainLoading.drawFadeIn(draw, tid) {
	frame=Cf.timeLine("${tid}.current");
	if( Cf.timeLine("${tid}.running") ) {
		opa=30;
		opa+=frame*7;
	} else {
		opa=100;
	}
	draw.opacity(opa);
	return frame;
}

MainLoading.showLoading(rc) {
	widget=tag[loadingWidget];
	main=this.mainControl();
	canvas=main.canvas;
	not( widget ) {
		widget=canvas.widget({tag:canvas});
		widget.flags('splash, top');
		widget.playGif("$cf[imagePath]/main/loading2.gif");
		tag[loadingWidget]=widget;
	}
	rcGlobal=canvas.mapGlobal(rc.center(300,300));
	widget.move(rcGlobal.lt());
	widget.size(rcGlobal.size());
	widget.show();
	return lw;
}
MainLoading.popupCloseEvent() {
	tag[loadingWidget].hide();
}

mainCanvas.getMainNode() {
	return this.mainNode;
}


OrderConfirm.OrderConfirm(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
OrderConfirm.initControl() {
	cf.inject(imagePath);
	tag.removeAll();
	tag[Width]="936", tag[Height]="1244";
	tag.addNode({tag:Title, Height:114});
	tag.addNode({tag:OrderTitle, Margin:[20,5], Height:125});
	tag.addNode({tag:OrderList, Margin:[20,5]});
	tag.addNode({tag:OrderInfo, Margin:[20,15,20,20], Height:90});
	tag.addNode({tag:Buttons, Height:130});
	tag[title]="주문확인"
	tag[BackgroundImage]	="${imagePath}/Common/popup_order_confirm_bg.png";
	tag[CloseButton]			="${imagePath}/Common/popup_close_[#].png";
	tag[OrderDeleteButton]	="${imagePath}/Type/list_del_[#].png";
	setNodeSize(tag, true);
}
OrderConfirm.conf() {
	tagClearRect(tag);
	setNodeSize(tag);
	confNodeLayout(tag);
	while(cur, tag ) {
		cur[rect].inject(sx, sy, sw, sh);
		switch(cur[tag] ) {
		case Title:
			temp=_arr(this,'TempRate').recalc(sw, '*,80', true);
			temp.inject(a,b);
			cur[rcTitle]=Class.rect(sx, sy, a, sh), sx+=a;
			cur[rcClose]=Class.rect(sx, sy, b, sh);
			img=imageLoad(tag, 'CloseButton', 'n');
			img.imageSize().inject(w,h);
			cur[rcCloseButton]=cur[rcClose].center(w,h);
		case OrderList:
			this.confList(cur);
		case OrderInfo:
			temp=_arr(this,'TempRate').recalc(sw, '160,110,*,180,160,20', true);
			temp.inject(a1,a2,x,b1,b2);
			sy-=20;
			cur[rcTitleQty]	=Class.rect(sx,sy,a1,sh), sx+=a1;
			cur[rcQty]			=Class.rect(sx,sy,a2,sh), sx+=a2+x;
			cur[rcTitlePrice]	=Class.rect(sx,sy,b1,sh), sx+=b1;
			cur[rcPrice]		=Class.rect(sx,sy,b2,sh);
		case Buttons:
			cur[rcButtonCancel]		=cur[rect].width(400).center(353, 93);
			cur[rcButtonOrder]			=cur[rect].move('end', 400).center(353, 93);
		default:
		}
	}
	this.paymentClick=false;
}
OrderConfirm.draw(draw, timeline) {
	this.drawFadeIn(draw, timeline);
	lang=Cf[KioskLangSelect].lower();
	if( lang.eq('kor') ) {
		lang_title='주문확인 (카드)';
		lang_0="메뉴", lang_1='수량', lang_2='가격', lang_3='삭제';
		lang_qty='주문수량', lang_price='주문금액';
	} else {
		lang='eng';
		lang_title='Order Confirm (Card)';
		lang_0="Menu", lang_1='Qty.', lang_2='Price', lang_3='Delete';
		lang_qty='Total Qty.', lang_price='Total Price';
	}
	drawNodeStyle(draw, tag);
	while(cur, tag ) {
		switch(cur[tag] ) {
		case Title:
			drawNodeText(draw, cur[rcTitle], lang_title, "left", "PopupTitle");
			drawNodeImage(draw, cur[rcCloseButton], tag, 'CloseButton', 'n', true);
		case OrderTitle:
			draw.html( cur[rect], conf("message#kiosk.OrderConfirm_$lang") );
		case OrderList:
			/* 헤더 배경색 */
			draw.fill(cur[rect].height(80), '#b0b0b0');
			while(c,4) {
				switch(c) {
				case 0:
					drawNodeText(draw, cur[rcMenuOption], lang_0, "left", "OrderHeader");
				case 1:
					drawNodeText(draw, cur[rcQty], lang_1, "center", "OrderHeader");
				case 2:
					drawNodeText(draw, cur[rcPrice], lang_2, "center", "OrderHeader");
				case 3:
					drawNodeText(draw, cur[rcDelete], lang_3, "center", "OrderHeader");
				}
			}
			sp=this.startRow, ep=sp+9;
			while( n, ep, sp ) {
				sub=cur.child(n);
				not( sub ) {
					break;
				}
				draw.rectLine( sub[rect].incr(5), 4, '#d0d0d0', 1, 'dash');
				while(c,4) {
					switch(c) {
					case 0:
						menuName=sub[lang];
						not( menuName ) menuName=sub[menu_nm];
						drawNodeText(draw, sub[rcMenuOption].incrX(30), menuName, 'left','OrderInfo');
					case 1:
						drawNodeText(draw, sub[rcQty], sub[qty], 'center','OrderInfo');
					case 2:
						price=sub[qty]*sub[sale_price];
						priceSum=util_priceComma(price);
						drawNodeText(draw, sub[rcPrice].incrW(-10), "$priceSum 원", 'right','OrderInfo');
					case 3:
						ty=when( sub[rcDeleteButton].eq(this.mouseDownRect), 'p', 'n');
						drawNodeImage(draw, sub[rcDeleteButton], tag, 'OrderDeleteButton', ty, true);
					}
				}
			}
			if( cur[rcStatus] ) {
				rc=cur[rcStatus];
				draw.fill(rc, '#E8E5E0').rectLine(rc,0,'#c0c0c0');
				idx=sp+1, total=cur.childCount();
				if( sp>0 ) {
					var=when( this.mouseDownRect.eq(cur[rcUp]), 'p','n');
				} else {
					var='d';
				}
				imgUp=commonImage('btn_up',var);
				if( ep<total ) {
					var=when( this.mouseDownRect.eq(cur[rcDown]), 'p','n');
				} else {
					var='d';
				}
				imgDown=commonImage('btn_down',var);
				draw.drawImage( imgUp.center(cur[rcUp]), imgUp);
				draw.drawImage( imgDown.center(cur[rcDown]), imgDown);
			}
		case OrderInfo:
			sc=this.findControl('MenuCart#orderView');
			qty=sc[OrderTotalQty];
			price=util_priceComma(sc[OrderTotalPrice]);
			drawNodeText(draw, cur[rcTitleQty], 	"$lang_qty :",	'left','OrderInfo');
			drawNodeText(draw, cur[rcQty], 		"$qty 건", 		'right', 'OrderInfo');
			drawNodeText(draw, cur[rcTitlePrice], "$lang_price :", 	'left', 'OrderInfo');
			drawNodeText(draw, cur[rcPrice], 		"$price 원", 	'right', 18, '#C00D12', 'bold');
		case Buttons:
			drawCommButton( draw, cur[rcButtonCancel], 'popup_cancel', 'cancel', this, lang);
			drawCommButton( draw, cur[rcButtonOrder], 'popup_confirm', 'order', this, lang);
		default:
		}
	}
}
OrderConfirm.mouseDown(pos) {
	while(cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case Title:
			if( cur[rcCloseButton].contains(pos) ) {
				this.mouseDownRect=cur[rcCloseButton];
				break;
			}
		case OrderList:
			sp=this.startRow, ep=sp+9, total=cur.childCount();
			while( n, ep, sp ) {
				sub=cur.child(n);
				not( sub ) {
					break;
				}
				if( sub[rcDelete].contains(pos) ){
					this.mouseDownRect=cur[rcDelete];
					break;
				}
			}
			if( total>9 ) {
				if( cur[rcUp].contains(pos) ) {
					this.mouseDownRect=cur[rcUp];
				} else if( cur[rcDown].contains(pos) ) {
					this.mouseDownRect=cur[rcDown];
				}
			}
		case Buttons:
			if( cur[rcButtonCancel].contains(pos) ) {
				this.mouseDownRect=cur[rcButtonCancel];
			} else if( cur[rcButtonOrder].contains(pos) ) {
				this.mouseDownRect=cur[rcButtonOrder];
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
OrderConfirm.mouseUp(pos) {
	while(cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case Title:
			if( cur[rcCloseButton].contains(pos) ) {
				this.orderCancel();
				break;
			}
		case OrderList:
			sp=this.startRow, ep=sp+9, total=cur.childCount();
			while( n, ep, sp ) {
				sub=cur.child(n);
				not( sub ) {
					break;
				}
				if( sub[rcDelete].contains(pos) ){
					sc=this.findControl('MenuCart#orderView');
					orderList=sc.getOrderList();
					menu=orderList.findOne('menu_cd', sub[menu_cd]);
					if( menu ) {
						sc.removeMenu(menu, true );
						not( orderList.childCount() ) {
							this.orderCancel();
							return;
						}
						idx=sub.index();
						if( idx>0 ) {
							mod=idx%9;
							if( mod.eq(0) ) {
								sp-=9;
								if( sp<0 ) sp=0;
								this.startRow=sp;
							}
						}
						cur.remove(sub);
						this.conf();
					}
					break;
				}
			}
			if( total>9 ) {
				if( cur[rcUp].contains(pos) ) {
					this.mouseDownRect=null;
					sp-=9;
					if( sp<0 ) sp=0;
					this.startRow=sp;
					this.conf();
					this.update();
					return;
				} else if( cur[rcDown].contains(pos) ) {
					if( ep< total ) {
						this.mouseDownRect=null;
						this.startRow=ep;
						this.conf();
						this.update();
						return;
					}
				}
			}
		case Buttons:
			if( cur[rcButtonCancel].contains(pos) ) {
				if( this.paymentClick ) return;
				this.orderCancel();
			} else if( cur[rcButtonOrder].contains(pos) ) {
				if( this.paymentClick ) return;
				this.paymentClick=true;
				this.orderConfirm();
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
OrderConfirm.selectConfirm(selectType) {
	cf[OrderSelectType]=selectType;
	switch( cf[OrderHeader.PayType] ) {
	case Cash:
		this.mainControl().openPopup('SelectCashNew');
	case Card:
		this.mainControl().openPopup('SelectCardNew');
	}
}


Popup.test() {
	x=this.findControl('Popup#dialog');
	x.popupOpen('OrderConfirm', 'popup', 936, 1244);
	p=Cf[KioskWatcher];
	p.inject(cf);
	db=Class.db('kiosk_hitec');
	not( cf[prod_img_url] ) {
		db.fetch("select prod_img_url from hitec_x10s limit 1 offset 0", cf);
	}
	_log("다운로드 시작: tag=$tag, 이미지 경로: $path URL: $url");
	url=cf[prod_img_url], path=conf("setup#kiosk.imagePath");
	root=_node();
	db.fetchAll("SELECT goods_cd, goods_img FROM hitec_m10s", root);
	p.downloadMenuImage(url, root, "$path/menus");
}

CompleteOrder.CompleteOrder(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
CompleteOrder.initControl() {
	cf.inject(imagePath);
	tag.removeAll();
	tag[Width]=936, tag[Height]=823;
	tag[BackgroundImage]	="${imagePath}/Type/popoup/pop_finish_bg.png";
	tag[CompleteImage]		="${imagePath}/Type/receipt/receipt_img.png";
	tag.addNode({tag: Title, Height:114});
	tag.addNode({tag: CompleteMessage, Height:240});
	tag.addNode({tag: OrderInfo, Margin: [34,0,34,30]});
	node=this.findTag('OrderInfo');
	node.addNode({tag: OrderNumber});
	node.addNode({tag: OrderImage, Height:300});
}

CompleteOrder.conf() {
	tagClearRect(tag);
	setNodeSize(tag, true);
	confNodeLayout( tag );
	node=this.findTag('OrderInfo');
	setNodeSize(node, true);
	confNodeLayout( node );
}
CompleteOrder.draw(draw, tm) {
	lang=Cf[KioskLangSelect].lower();
	if( lang.eq('kor') ) {
		lang_title="결제완료", lang_msg="결제가 완료되었습니다\n영수증을 확인하세요", lang_order='주문번호';
	} else {
		lang_title="Order Completed", lang_msg='The Payment has been completed.', lang_order='Order Number';
	}
	draw.effect(
		DRAW.RoundBox, tag[rect].incr(-2), 4, '#463E3C', null, 2
	);
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag] ) {
		case Title:
			draw.font(32, 'normal','#404040', '나눔바른고딕').text(cur[rect].incrX(40), lang_title);
		case CompleteMessage:
			draw.font(26, 'normal', '#6a6060', '나눔바른고딕', 4).text(cur[rect], lang_msg, "center" );
		case OrderInfo:
			order=cf[OrderHeader];
			while( sub, cur ) {
				switch(sub[tag]) {
				case OrderNumber:
					draw.font(28, 'bold', '#fa304a', '나눔바른고딕').text(sub[rect], "$lang_order : $order[deal_no]", "center");
				case OrderImage:
					img=imageLoad(tag,'CompleteImage');
					draw.drawImage( img.center(sub[rect]), img );
				}
			}
		}
	}
}
CompleteOrder.mouseDown(pos) {

}
CompleteOrder.mouseUp(pos) {

}
CompleteOrder.drawOrderInfo(draw, cur) {
	order=cf[OrderHeader];
	while( sub, cur ) {
		switch(sub[tag]) {
		case OrderNumber:
			draw.font(28, '#fa304a').text(sub[rect], "주문번호 : $order[DealNo]", "center");
		case OrderImage:
			img=imageLoad(tag,'CompleteImage');
			draw.drawImage( img.center(sub[rect]), img );
		}
	}
}

mainCanvas.popupOpen(pageId, var) {
	if( cf[errorOpen] ) return;
	this.popupClose();
	var=null;
	switch( pageId ) {
	case MainLoading:
		pageNode=this[mainNode], rc=pageNode[rect];
		this.findControl('Popup#dialog').popupOpen(pageId, 'popup', rc.incr(15), rc);
	case CardImage:
		rc=Class.rect(0,0,863,1352), point=Class.point(108,527);
		this.findControl('Popup#dialog').popupOpen(pageId, 'popup', rc, point );
	default:
		this.findControl('Popup#dialog').popupOpen(pageId, 'popup', var);
	}
}
mainCanvas.popupClose() {
	this.findControl('Popup#dialog').popupClose();
}

SelectCash.SelectCash(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SelectCash.initControl() {
	tag[Width]=936, tag[Height]=764;
	tag[BackgroundImage]="${cf[imagePath]}/Common/kr/pop_cash01_bg.png";
	tag.addNode({tag: Player, class:layer, Width:318 Height:410 Margin:[30,167,0,0]});
	tag.addNode({tag: AmountOfPayment, class:layer, Width:936 Height:60 Margin:[0,276,87,0]});
	tag.addNode({tag: InputAmount, class:layer, Width:936 Height:60 Margin:[0,350,87,0]});
	tag.addNode({tag: BalanceAmount, class:layer, Width:936 Height:60 Margin:[0,424,87,0]});
	tag.addNode({tag: CancelButton, class:layer, Width:339 Height:89 Margin:[296,644,0,0]});
	setNodeSize(tag, true);
}
SelectCash.conf() {
	offset=tag[rect].lt();
	confNodeLayout(tag, offset);
	/* 현금 입력정보 초기화 */
	order=cf[OrderHeader];
	this.findControl('MenuCart#orderView').inject(OrderTotalPrice);
	order[OrderTotalPrice]=OrderTotalPrice;
	order[InputCashPrice]=0;
	order[DelayCount]=0;
	order[SaleType]='O';
	order[InputCashOk]=false;
	price=order[OrderTotalPrice];
	this.mainControl().qtMonSendData( "02,3,1,1,0,$price");
	/* 동영상 플레이어 보이게 하기*/
	node=this.findTag('Player');
	path=null;
	not( node[src] ) {
		path="${cf[imagePath]}/Common/flash/cash1.mp4";
		node[src]=path;
	}
	this.showMoviePlayer(node[rect], path);
}
SelectCash.drawFadeIn() {
	popupFadeIn(draw, timeline);
}
SelectCash.draw(draw, timeline) {
	this.drawFadeIn(draw, timeline);
	drawNodeStyle(draw, tag);
	order=cf[OrderHeader];
	while( cur, tag ) {
		switch(cur[tag] ) {
		case Player:
			this.showMoviePlayer(cur[rect]);
		case AmountOfPayment:
			price=util_priceComma(order[OrderTotalPrice]);
			drawNodeText(draw, cur[rect], price, "right", 22, '#fa2030');
		case InputAmount:
			price=order[InputCashPrice];
			price=util_priceComma(price);
			not( price ) price=0;
			drawNodeText(draw, cur[rect], util_priceComma(price), "right", 22, '#707a7a');
		case BalanceAmount:
			price= order[OrderTotalPrice];
			price-=order[InputCashPrice];
			if( price<=0 ) {
				order[InputCashOk]=true;
			}
			drawNodeText(draw, cur[rect], util_priceComma(price), "right", 22, '#707a7a');
		case CancelButton:
			not( order[InputCashOk] ) {
				img=commonImage('btn_cancel');
				draw.drawImage(cur[rect], img);
			}
		}
	}
}
SelectCash.mouseDown(pos) {
	while( cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case CancelButton: this.mouseDownRect=cur[rect];
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
SelectCash.mouseUp(pos) {
	while( cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case CancelButton: this.cancelButtonClick();
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
SelectCash.popupCloseEvent() {
	if( this[playerWidget] )  {
		this[playerWidget].hide();
	}
}
SelectCash.cancelButtonClick() {
	order=cf[OrderHeader];
	/* 투입한 금액이 있다면, 모두 배출하도록 한다*/
	if( order[InputCashPrice] ) {
		this.mainControl().qtMonSendData("04,1,$order[InputCashPrice]");
	}
	this.mainControl().popupClose();
}
SelectCash.showMoviePlayer(rc, path) {
	main=this.mainControl();
	canvas=main.canvas;
	player=this.playerWidget;
	not( player ) {
		player=canvas.widget(conf('widget#kiosk.moviePlayer'), true);
		player.flags('top');
		player.open();
		this.mainControl().addPlayer(player);
		this.playerWidget=player;
		cf[playerWidgets].add(player);
	}
	rcGlobal=canvas.mapGlobal(rc);
	player.move(rcGlobal.lt());
	player.size(rcGlobal.size());
	if( path ) {
		player.flags('splash');
		player.initPage(path);
	}
	player.show();
}

mainCanvas.closeKiosk() {
	while( cur, cf[playerWidgets] ) {
		cur.quit();
	}
}

protocalTest.parseProp(node, tag, &prop) {
	node[tag]=tag;
	idx=node.index();
	arr=null;
	not( idx ) arr = _arr(node,'fieldsArray');
	while( prop.valid() ) {
		k=prop.findPos('=').trim();
		not( k ) break;
		if( arr ) arr.add(k);
		ch=prop.ch();
		if( ch.eq() ) {
			node[$k]=prop.match().trim();
		} else if( ch.eq('[') ) {
			in=prop.match();
			arr=[];
			while( in.valid() ) {
				arr.add( in.findPos(',').trim() );
			}
			node[$k]=arr;
		} else {
			node[$k]=prop.findPos(" \t\n",4).trim();
		}
	}
}
protocalTest.makeKioskData(tag, fields, root) {
	db=Class.db('kiosk_hitec');
	table="HITEC_${tag}";
	not( db.count("select count(1) from pg_tables where schemaname='public' and tablename='${table}'") ) {
		sql="create table ${table} (";
		while( k, fields, n, 0 ) {
			if( k.finds('_GB', '_YN') ) {
				type='char(1)';
			} else {
				type='text';
			}
			sql.add("$k $type,");
		}
		sql.add("use_yn char(1) DEFAULT 'Y', tm integer DEFAULT 0, reg_dt timestamp without time zone DEFAULT now() )");
		db.exec(sql);
	}
	a='', b='';
	while( k, fields, n, 0 ) {
		if( n ) {
			a.add(',');
		} else {
			b.add(k);
		}
		a.add(k);
	}
	a.add(",tm");
	switch( tag ) {
	case M03S: 	b='CLPLU_CD';
	case M10S:		b='GOODS_CD';
	case M12S: 	b='GOODS_CD';
	case M05S:		b='CLPLU_CD';
	case M06S:		b='SET_CD';
	case M60S:		b='VAN_CD';
	}
	if( page.checkSendAll ) {
		db.exec("delete from $table");
	}
	ins=getQuery(table, a);
	upd=getQuery(table, a, b);
	print(ins, upd);
	tm=System.localtime();
	while( cur, root ) {
		cur[tm]=tm;
		if( tag.eq("M10S", "M12S") ) {
			/* 다국어 처리 */
			if( cur[JP_NM] ) {
				decode	= cur[JP_NM].decode('a2u');
				cur[JP_NM]=decode;
			}
			if( cur[CN_NM_GAN] ) {
				decode	= cur[CN_NM_GAN].decode('a2u');
				cur[CN_NM_GAN]=decode;
			}
		}
		if( cur[PROC_GB].eq('D') ) {
			db.exec("update $table set use_yn='N' where $b=#{$b}", cur);
		} else {
			if( tag.eq("M23S") ) {
				if( cur[PROC_GB].eq('A') ) {
					db.exec(ins, cur);
				} else {
					db.exec(upd, cur);
				}
			} else {
				not( db.exec(upd, cur) ) {
					db.exec(ins, cur);
				}
			}
		}
	}
}
protocalTest.test() {
	pageActionAdd(page, 'protocalTest.download', '다운로드',
			'ficon.inbox-download', this.downloadImages);
}
protocalTest.downloadImages(type) {
	node=tree.current();
	cur=null;
	not( type ) type=node[tag];
	if( node[tag].eq('HEADER') ) {
		cur=node;
	} else {
		cur=findTag('HEADER', node);
	}
	sub=cur.child(0);
	not( sub[tag].eq('DETAIL') ) {
		page.alert("$node 는 detail 정보가 없습니다");
		return;
	}
	db=Class.db('kiosk_hitec');
	switch( type ) {
	case M10S:
		/*
		page.downloadImage(sub);
		*/
		root=_node();
		db.fetchAll("select goods_cd, goods_img from hitec_m10s where goods_img<>''", root);
		page.downloadImage(root);
	case M06S:
		root=_node();
		db.fetchAll("select set_cd, set_val from hitec_m06s where set_val<>'' ", root);
		page.downloadM06s(root);
	case M03S:
		root=_node();
		db.fetchAll("select clplu_cd, clplu_nm, img_file_nm from hitec_m03s where use_yn='Y' ", root);
		page.downloadM03s(root);
	default:
	}
}
protocalTest.makeProtocalData() {
	node=tree.current();
	cur=null;
	if( node[tag].eq('HEADER') ) {
		cur=node;
	} else {
		cur=findTag('HEADER', node);
	}
	sub=cur.child(0);
	if( sub[tag].eq('DETAIL') ) {
		data=sub.child(0);
		this.makeKioskData(node[tag], data[fieldsArray], sub);
	}
}

ListBox.makeThumbBox(rect) {
	a=tag[drawThumbBox], b=tag[drawThumbImgBox];
	not( a ) {
		a=Class.draw(rect.size()), b=Class.draw(rect.size());
		rc=Class.rect(0,0,rect.size());
		a.effect( DRAW.RoundBox, rc, 15, '#cacaca', '#fafafa', 2 );
		b.effect(DRAW.RoundBox, rc, 15, '#cacaca', '#eae0da', 2);
		tag[drawThumbBox]=a;
		tag[drawThumbImgBox]=b;
	}
}

Popup.pageOpen(pageId) {
	this.findControl('Popup#stack').stackPageLoad(pageId);
}
Popup.pageClose() {
	this.findControl('Popup#stack').stackPageClose();
}

mainCanvas.pageOpen(pageId) {
	if( cf[errorOpen] ) return;
	this.findControl('Popup#stack').stackPageLoad(pageId);
}
mainCanvas.pageClose() {
	this.findControl('Popup#stack').stackPageClose();
}

MainPage.MainPage(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MainPage.conf() {
	setNodeSize(tag, true);
	confNodeLayout(tag);
	while( cur, tag ) this.getControl(cur).conf();
}
MainPage.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) this.getControl(cur).draw(draw, tm);
}
MainPage.initControl() {

}
MainPage.mouseDown(pos) {
	while( cur, tag ) this.getControl(cur).mouseDown(pos);
}
MainPage.mouseUp(pos) {
	while( cur, tag ) this.getControl(cur).mouseUp(pos);
}

KioskMain.KioskMain(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
KioskMain.initControl() {
	setNodeSize(tag, true);
}
KioskMain.conf() {
	setNodeSize(tag, true);
	confNodeLayout(tag);
	cf.inject(imagePath);
	tag[btnOrder]="$imagePath/main/btn_order_[#].png";
	_confCorner=func(node ) {
		rect=node[rect].move(83, 230).incrW(-115);
		rect.inject(sx, sy, sw, sh);
		bw=220, bh=250;
		right=sx+sw;
		cx=sx;
		tag[CornerTabNode]=this.findTag('#CornerTab');
		while( cur, tag[CornerTabNode], n, 0 ) {
			rc=Class.rect(cx, sy, bw, bh), cx+=bw+11;
			cur[rectButton]=rc;
			if( cx>right ) {
				cx=sx;
				sy+=bh+10;
			}
		}
	};
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			not( cur[src] ) {
				cur[src]="$imagePath/main/main_logo.png";
			}
		case Banner:
			not( cur[src] ) {
				cur[src]="$imagePath/main/main_event.png";
			}
		case Corners:
			_confCorner(cur);
		default:
		}
	}
	db=Class.db('kiosk_hitec');
	db.fetch("SELECT max(open_date) as open_date FROM kiosk_open_close", cf);
}
KioskMain.draw(draw, tm) {
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			rc=cur[rect], img=kioskImage('001');
			not( img ) {
				this.mainControl().reloadCornerTab();
				img=kioskImage('001');
			}
			draw.drawImage( img.center(rc), img);
		case Banner:
			draw.drawImage(cur[rect], kioskImage('002') );
		case Corners:
			draw.font(18,'normal','#d0d0d0');
			while( tab, this.findTag('#CornerTab'), n, 0 ) {
				/* 1. 주문하기 버튼*/
				rc=tab[rectButton];
				draw.drawImage(rc, kioskImage(tab[corner_cd]) );
				if( rc.eq(this.mouseDownRect) ) {
					draw.fill(rc, '#d0d0d0a0');
				}
				/* 2. 코너명
				rc=tab[rectCornerName], tw=draw.textWidth(tab[corner_nm]);
				if( tw>rc.width() ) {
					draw.text(rc.incrY(10), tab[corner_nm], 'wrap');
				} else  {
					draw.text(rc, tab[corner_nm], 'center');
				}
				*/
			}
		case Footer:
		default:
		}
	}
	rc=tag[rect].move('bottom', 32);
	date=util_formatDate(cf[open_date]);
	draw.font(14,'normal','#C46D57').text(rc.move('end', 185),  "영업일: $date" );
	draw.font(14,'normal','#C46D57').text(rc.width(200).incrX(20),  cf[kioskVersion]);
}
KioskMain.mouseDown(pos) {
	this.mouseDownRect=null;
	while( tab, tag[CornerTabNode] ) {
		not( tab[rectButton].contains(pos) ) continue;
		this.mouseDownRect=tab[rectButton];
		break;
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
KioskMain.mouseUp(pos) {
	while( tab, tag[CornerTabNode] ) {
		not( tab[rectButton].contains(pos) ) continue;
		this.changeConer(tab);
		break;
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
KioskMain.changeConer(tab) {
	if( this[changeConerTick] ) {
		dist=System.tick() - this[changeConerTick];
		if( dist<500 ) {
			return;
		}
	}
	this[changeConerTick]=System.tick();
	tabs=this.findTag('#CornerTab');
	while( cur, tabs ) {
		dow=System.date('dayOfWeek');
		switch( dow ) {
		case 1:  day=" when view_mon_yn='Y' ";
		case 2:  day=" when view_tue_yn='Y' ";
		case 3:  day=" when view_wed_yn='Y' ";
		case 4:  day=" when view_thu_yn='Y' ";
		case 5:  day=" when view_fri_yn='Y' ";
		case 6:  day=" when view_sat_yn='Y' ";
		case 7:  day=" when view_sun_yn='Y' ";
		}
		cur[time]=System.date('HHmm');
		sql=fmt(conf('sql#hitec.menuList') );
		db.fetchAll(sql, cur.removeAll() );
		while( menu, cur ) {
			menu.inject(day_ok, time_ok);
			if( day_ok.eq('1') && time_ok.eq('1') ) {
				menu[sale_ok]=true;
			}
		}
	}
	/*
	this.mainControl().makeDisplayTab(true);
	*/
	System.timeout(250);
	this.mainControl().pageClose();
	this.findControl('#CornerTab').currentTabChange(tab, true, true);
}

QueryTest.QueryTest(page) {
	this.addClass(common.Config, dev.EditorSrcChange, dev.EditorSrcClick );
	db=Class.db('config');
	/* #################### Tree #################### */
	tree=page.tree;
	tree.check('treeMode', true);
	tree.model(Class.model('QueryTestTree'), 'value');
	tree.eventMap(onChildData, this.treeChildData, 'node');
	tree.eventMap(onDraw, this.treeDraw, 'draw, node, over');
	tree.eventMap(onChange, this.treeChange, 'node');
	/* #################### Grid #################### */
	grid=page.grid;
	dataModel=Class.model('QueryTestGrid');
	grid.check('sortEnable', true);
	/* 그리드 이벤트 맵핑  */
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onChange, this.gridChange, 'node');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	/* 그리드 헤더폭을 자동 계산  */
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
	/* #################### editor #################### */
	editor=  page.sqlEditor;
	not( editor ) return;
	editor.syntax( conf('syntax.sql') );
	editor.eventMap( onMouseClick, this.editorMouseClick, 'pos, keys' );
	editor.eventMap( onChange, this.editorChange );
	editor.eventMap( onKeyDown, this.sqlEditorKeyDown, 'key,mode' );
	this.initPage();
}
QueryTest.initTree() {

}
QueryTest.treeDraw(draw, node, over) {
	rc=treeIcon(tree, draw, node, over);
	rcIcon = rc.width(18).center(16,16);
	rc.incrX(20);
	switch( node[type] ) {
	case ROOT:
		draw.icon( rcIcon, "vicon.application_form" );
		draw.save().font('bold');
		draw.text( rc,  node[title]);
		draw.restore();
	default:
		switch(node[depth]) {
		case 1:	icon='vicon.application_side_boxes';
		case 2:	icon='vicon.application_view_list';
		case 3:	icon= 'vicon.page_red';
		}
		draw.icon( rcIcon, icon );
		if( node[@field] ) {
			field=node[@field];
			val=getCommCodeValue("@vrs#${field}", node[value]);
			draw.text(rc, val);
		} else {
			draw.text( rc,  node[value]);
		}
		if( node[type] ) {
			type=node[type];
			if( type.start('timestamp') ) {
				type='date';
			} else if( type.find('varying') ) {
				type='varchar';
			}
			w=draw.textWidth(type)+10;
			draw.text(rc.move('end',w), type);
		}
	}
}
QueryTest.treeMouseClick(pos, button) {
	if( button.eq('right') ) return 'ignore';
}
QueryTest.treeChange(node) {
	p=node.parent();
	if( p[kind].eq('table_info') ) {
		table=node[value];
		switch( db[dbms] ) {
		case sqlite:
			sql=null;
			sqlTable="SELECT * FROM $table limit 0, 50";
		case mssql:
			sql="select 2 depth, column_name as value, data_type + '(' +cast(character_maximum_length as varchar) +')' as type from information_schema.columns where table_name=#{value} order by ordinal_position";
			sqlTable="SELECT TOP 10 * FROM $table";
		case postgres:
			sql="select 2 depth, column_name as value,data_type as type from information_schema.columns where table_name=#{value}";
			sqlTable="SELECT * FROM $table limit 50 offset 0";
		}
		if( sql ) db.fetchAll(sql, node);
		if( sqlTable ) {
			this.makeGrid(sqlTable);
			editor.move('end');
			editor.insert("\n$sqlTable\n");
		}
		if( this.prevSelectNode ) {
			tree.expand(this.prevSelectNode, false);
		}
		tree.expand(node);
		this.prevSelectNode=node;
	}
	if( node[type] ) {
		val=node[value].lower();
		page[sqlEditor].insert(" $val");
		page[sqlEditor].focus();
	}
}
QueryTest.initGrid() {
	fields=grid.fields();
	gridMakeField(tr('data#fields.QueryTest'),true, fields);
	grid.model( dataModel, fields);
	gridHeaderWidth(grid);
}
QueryTest.searchGrid() {
	root=grid.rootNode();
	/* 조회 쿼리를 넣어준다*/
	db.fetchAll("", root.removeAll() );
	grid.update();
	page.deletePage.hide();
	gridHeaderWidth(grid);
}
QueryTest.gridChange(node) {

}
QueryTest.gridDoubleClick(node) {

}
QueryTest.gridResize() {
	gridHeaderWidth(grid);
}
QueryTest.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
QueryTest.gridClick(node, column) {
	field=grid.field(column);
	switch( field ) {
	case check:	gridCheck(grid, node, page.deletePage );
	default:			grid.edit(node, column);
	}
}
QueryTest.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
				page[applyData].enable();
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}
QueryTest.initPage() {
	root=tree.rootNode().removeAll();
	root.addNode({depth:0, type:ROOT, title: 조회트리});
	page[applyData].enable(false);
	page[runQuery].eventMap( onClick, this.runQueryClick);
	tree.update();
	combo=page[dbCombo];
	setCommCombo(combo, 'kiosk#dbinfo', '=DB선택=');
	combo.eventMap( onChange, this.dbInfoChange );
	grid.model(dataModel, gridMakeField('id:id, value:value',true) );
}
QueryTest.test() {
	grid.model(dataModel, gridMakeField('id:id, value:value',true) );
}
QueryTest.runQueryClick() {
	sql=editor.text('select');
	not( sql ) {
		sql=editor.value();
	}
	not( sql ) {
		page.alert("쿼리를 입력하세요");
	}
	type=sql.move().trim().lower();
	if( type.eq('insert', 'update', 'delete') ) {
		db.exec(sql);
		err=db.error();
		if( err ) {
			p=this[page];
			p.alert("DB 실행 오류 : $err");
		}
		return;
	}
	dataNode=_node(this,'dataNode');
	sep=conf('message.StringSep');
	if( sql.find(sep) ) {
		s=sql.str();
		query=s.findPos(sep);
		while( s.valid() ) {
			key=s.findPos("\n").trim(), codeQuery=s.findPos(sep);
			not( key ) break;
			db.fetchAll(codeQuery, dataNode.removeAll() );
			makeCommCode(key, dataNode);
		}
		sql=query.trim();
	}
	if( page[checkTreeNode].checked() ) {
		this.makeTree(sql, dataNode.removeAll() );
	} else {
		this.makeGrid(sql);
	}
}
QueryTest.dbInfoChange() {
	combo=page[dbCombo];
	val=combo.value();
	not( val ) return;
	@db=Class.db(val);
	if( val.eq('kiosk_local') ) {
		not( db.open() ) {
			path="data/namzatang.db";
			not( Class.file().isFile(path) ) {
				projectCode=Cf[projectCode];
				path="project/$projectCode/data/namzatang.db";
			}
			db.open(path);
		}
	}
	not( db[dbms] ) db[dbms]='sqlite';
	this.setDbTableInfo();
}
QueryTest.setDbTableInfo(dbms) {
	root=tree.rootNode().child(0);
	switch( db[dbms] ) {
	case sqlite:
		sql="select 1 depth, tbl_name as value from sqlite_master order by name ";
	case mssql:
		sql="select 1 depth, table_name as value from information_schema.tables order by table_name";
	case postgres:
		sql="select 1 depth, tablename as value from pg_tables where schemaname='public' order by tablename";
	}
	db.fetchAll(sql, root.removeAll() );
	root[kind]='table_info';
	root[title]="테이블 정보";
	tree.update();
	tree.expand(root, true);
}
QueryTest.makeTree(sql, dataNode) {
	root=tree.rootNode().child(0);
	groupBy=page[inputTitle].value();
	db.fetchAll( sql, dataNode );
	print( sql, dataNode);
	makeTreeNode(root.removeAll(), dataNode, groupBy);
	root[kind]='make_tree';
	root[title]="쿼리 조회";
	tree.update();
}
QueryTest.makeGrid(sql) {
	root=grid.rootNode();
	db.fetchAll(sql, root.removeAll(), true ), err=db.error();
		if( err ) {
			page.alert("DB조회 오류 :\n $err");
			grid.update();
			return;
		}
	node= root.child(0);
	if( node ) {
		s="";
		while( field, root[@fields], n, 0) {
			w=gridMaxFiledWidth(root, field);
			if( n ) s.add(",");
			s.add("$field:$field #", min(w,350) );
		}
		model=grid.model(), fields=grid.fields();
		gridMakeField(s, true, fields);
		grid.model(model, fields);
	}
	total=root.childCount();
	page[gridStatus].value("(총 $total 건)");
	page[applyData].enable(false);
	grid.update();
}
QueryTest.sqlEditorKeyDown(key, mode) {
	if( this.editorKeyDown( key, mode) )
		return true;
	not( mode&KEY.ctrl ) return false;
	switch(key) {
	case KEY.R: 			this.runQueryClick();
	}
	return false;
}

mainCanvas.getPageRect() {
	node=this.mainNode;
	return node[rect];
}
mainCanvas.reloadCornerTab() {
	print("## 코너 정보처리 ##");
	/* 환경설정 처리(추천처리는 goHome함수에서 한다 */
	val=db.value("SELECT set_val FROM hitec_m06s where set_cd='004' ");
	if( val.eq('0') ) {
		cf[view_type]='B';
	} else {
		cf[view_type]='A';
	}
	val=db.value("SELECT set_val FROM hitec_m06s where set_cd='007' ");
	if( val.eq('1') ) {
		cf[mainPageUse]=true;
	} else {
		cf[mainPageUse]=false;
	}
	cf.inject( imagePath );
	tabs=this.findTag('#CornerTab'), menuNode=_node('MainImages');
	cf[mainUpdateTick]=System.tick();
	adNode=_node(cf, 'AdImages');
	db.fetchAll(conf("sql#hitec.adImage"), adNode.removeAll() );
	while( cur, adNode ) {
		code=cur[set_cd];
		if( cur[set_val] ) {
			/* 이전 광고 이미지와 다르거나 없다면 */
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
	/* 코너 새로고침은 당분간 제거 */
	if( tabs.childCount() ) {
		root=_node();
		db.fetchAll(conf('sql#hitec.cornerTab'), root.removeAll() );
		while(cur, root ) {
			find=tabs.findOne('corner_cd', cur[corner_cd]);
			if( find ) {
				not( cur[kor].eq(find[kor]) ) {
					if( cur[kor].find("  ") ) {
						kor=cur[kor].replace("  ","\n");
						find[kor]=kor;
					} else {
						find[kor]=cur[kor];
					}
				}
				not( cur[eng].eq(find[eng]) ) {
					find[kor]=cur[eng];
				}
				not( cur[img_file_nm].eq(find[img_file_nm]) ) {
					find[img_file_nm]=cur[img_file_nm];
				}
			}
		}
	} else {
		db.fetchAll(conf('sql#hitec.cornerTab'), tabs.removeAll() );
		while(cur, tabs ) {
			if( cur[kor].find("  ") ) {
				kor=cur[kor].replace("  ","\n");
				cur[kor]=kor;
			}
		}
	}
	while( cur, tabs ) {
		if( cur[img_file_nm] ) {
			code=cur[corner_cd];
			imgSrc="$imagePath/menus/corner/$cur[img_file_nm]";
			not( imgSrc.eq(menuNode[$code]) ) {
				_log("# 코너이미지 추가 src=$imgSrc");
				if( menuNode[@$code] ) {
					menuNode[@$code]=null;
				}
				menuNode[$code]=imgSrc;
			}
		}
		/*
		dow=System.date('dayOfWeek');
		switch( dow ) {
		case 1:  day=" when view_mon_yn='Y' ";
		case 2:  day=" when view_tue_yn='Y' ";
		case 3:  day=" when view_wed_yn='Y' ";
		case 4:  day=" when view_thu_yn='Y' ";
		case 5:  day=" when view_fri_yn='Y' ";
		case 6:  day=" when view_sat_yn='Y' ";
		case 7:  day=" when view_sun_yn='Y' ";
		}
		cur[time]=System.date('HHmm');
		sql=fmt(conf('sql#hitec.menuList') );
		db.fetchAll(sql, cur.removeAll() );
		while( menu, cur ) {
			menu.inject(day_ok, time_ok);
			if( day_ok.eq('1') && time_ok.eq('1') ) {
				menu[sale_ok]=true;
			}
		}
		*/
	}
	status=cf[kioskStatus], total=tabs.childCount();
	_log("# 매장정보 새로고침 => 매장 갯수 : $total, 키오스크 상태: $status");
}

PingTestGrid.PingTestGrid(page) {
	this.addClass(common.Config );
	db=Class.db('config');
	grid=page.grid;
	dataModel=Class.model('PingTest');
	grid.check('sortEnable', true);
	/* 그리드 이벤트 맵핑  */
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onChange, this.gridChange, 'node');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	/* 필드 비율만큼 그리드 헤더폭을 자동 계산  */
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
	this.initGrid();
}
PingTestGrid.initGrid() {
	grid.model( dataModel, gridMakeField('
		target: 아이콘				#65,
		target_nm: 그룹			#110,
		ip: 아이피					#95,
		error_count: 오류건		#70,
		error_msg: 에러내용	#350,
		status: 상태				#70,
		reg_dt: 등록일시 			#120', grid)
	);
}
PingTestGrid.gridChange(node) {

}
PingTestGrid.gridDoubleClick(node) {

}
PingTestGrid.gridResize() {
	gridHeaderWidth(grid);
}
PingTestGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	default:
		draw.text(rc, node[$field].trim());
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
PingTestGrid.gridClick(node, column) {
	field=grid.field(column);
	switch( field ) {
	case check:	gridCheck(grid, node, page.deletePage );
	case note:		grid.edit(node, 2);
	}
}
PingTestGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}

processInfoViewGrid.processInfoViewGrid(page) {
	this.addClass(common.Config );
	db=Class.db('config');
	grid=page.grid;
	/* 그리드 이벤트 맵핑  */
	grid.eventMap(onDraw, this.gridDraw, 'draw, node, over');
	grid.eventMap(onClicked, this.gridClick, 'node, column');
	grid.eventMap(onChange, this.gridChange, 'node');
	grid.eventMap(onDoubleClicked, this.gridDoubleClick , 'node');
	grid.eventMap(onEditEvent, this.gridEditEvent, 'type, node, data, index');
	/* 필드 비율만큼 그리드 헤더폭을 자동 계산  */
	if( gridHeaderWidth(grid) ) {
		grid.eventMap(onResize, this.gridResize);
	}
	/* 삭제 버튼이 있을경우 아이디를 넣어준다 */
	deleteButton=page.deleteButton;
}
processInfoViewGrid.initPage() {
	name=conf('setup.kiosk#processCheckName').trim();
	sycle=conf('setup.kiosk#processCheckSycle').trim();
	not( sycle ) {
		sycle=60;
	}
	page[porcessName]=name;
	page[sycleSpin].value(sycle);
	page[porcessName].eventMap( onTextChange, this.processNameChange, 'text');
	grid.eventMap(onMouseDown, this.gridMouseDown,  'pos, button');
	grid.eventMap(onFilter, this.gridFilter, 'node' );
	dataModel=Class.model('processInfo');
	grid.model( dataModel, gridMakeField('
		processId: 아이디	#100,
		processName: 프로세스명#350,
		threadCount: 쓰레드수#90', true)
	);
	grid.check('sortEnable', true);
}
processInfoViewGrid.gridChange(node) {
	page[processName].value( node[processName]);
}
processInfoViewGrid.gridDoubleClick(node) {

}
processInfoViewGrid.gridResize() {
	gridHeaderWidth(grid);
}
processInfoViewGrid.gridDraw(draw, node, over) {
	rc=draw.rect();
	field=grid.field(draw.index());
	gridOver(draw, node, over);
	switch( field ) {
	case check:
		rcIcon=rc.center(16,16);
		if( node.state(NODE.add) )
			gridModifyMark(draw, rc, '#a090ea');
		if( node[checked] )
			draw.icon(rcIcon, 'func.check');
		else
			draw.icon(rcIcon, 'func.add');
	default:
		draw.text(rc, node[$field]);
	}
	if( node.state(NODE.modify), node[modify#$field] ) {
		gridModifyMark(draw, rc);
	}
	draw.rectLine(rc,4,'#d0d0d0');
}
processInfoViewGrid.gridClick(node, column) {
	field=grid.field(column);
	switch( field ) {
	case check:	gridCheck(grid, node, page.deletePage );
	case note:		grid.edit(node, 2);
	}
}
processInfoViewGrid.gridEditEvent(type, node, data, index) {
	field=grid.field(index);
	switch( type ) {
	case create:
		return null;
	case geometry:
		rc=data;
		return rc;
	case finish:
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify,true);
				node[modify#$field]=true;
			}
			node[$field]=data;
		}
		grid.update();
	default: break;
	}
}
processInfoViewGrid.search() {
	root=grid.rootNode();
	System.processInfo(root.removeAll() );
	grid.update();
}
processInfoViewGrid.processNameChange(text) {
	this[filterText]=text;
	grid.update();
}
processInfoViewGrid.gridFilter(node) {
	if( this[filterText] ) {
		name=node[processName].lower();
		a=this[filterText].lower();
		if( name.start(a) ) {
			return true;
		}
		return false;
	}
	return true;
}
processInfoViewGrid.gridMouseDown(pos, button) {
	hh= grid.headerHeight();
	node = grid.at(pos.incrY(hh) );
	field=node[@code];
	print( field, node);
	if( field.eq('check') ) {
		gridCheck(grid, node, page.killProcess );
		return 'ignore';
	}
}

mainCanvas.test() {
	req=_node("EasyCardNode");
	data="result_({'SUC':'00','RQ01':'D1','RQ02':'6205401','RQ03':'A','RQ04':'4332900000000000=00002013700000090621','RQ05':'****','RQ06':'00','RQ07':'3000','RQ08':'','RQ09':'','RQ10':'','RQ11':'','RQ12':'','RQ13':'272','RQ14':'1487119293','RQ15':'web1487119293','RS01':'P','RS02':'A','RS03':'7439','RS04':'0000','RS05':'016','RS06':'0000','RS07':'1702150941373','RS08':'150963463497','RS09':'30045144','RS10':'N*','RS11':'016','RS12':'KB?÷???????','RS13':'00081658989','RS14':'KB???????','RS15':'d','RS16':'','RS17':'               KICC?????','RS18':'Y','RS19':'2148608930','RS20':''})";
	this.easyCardReadData(data, req);
}

SaleOpenView.numberKeyDown(key) {
	text=tag[pwd];
	not( text ) text='';
	if( key.eq('0') ) {
		text.add('0');
		tag[pwd]=text;
	} else if( key.eq('Delete') ) {
		tag[pwd]=text.value(0,-1);
	} else if( key.eq('Reset') ) {
		tag[pwd]='';
	} else {
		text.add(key);
		tag[pwd]=text;
	}
	this.update()
}
SaleOpenView.pageCloseButtonClick() {
	this.mainControl().pageOpen('AdminHome');
}

SaleCloseView.pageCloseButtonClick() {
	this.findControl('#Content').pageLoad('AdminHome');
}

SaleStatusView.pageCloseButtonClick() {
	this.findControl('#Content').pageLoad('AdminHome');
}

SoldOutView.pageCloseButtonClick() {
	this.mainControl().pageOpen('AdminHome');
}

mainCanvas.alert(msg, title, error) {
	dialog=this.findControl('Popup#dialog');
	if( error ) {
		/* 주문중일때는 오류 출력 무시 */
		order=cf[OrderHeader];
		if( order[total_qty] ) {
			return;
		}
		cf[errorOpen]=true;
		cf[errorOpenTick]=System.tick();
		dialog.popupOpen('ErrorWindow','popup');
	} else {
		dialog.popupOpen('MessageWindow','common');
	}
	cur=dialog.getMainNode();
	this.getControl(cur).initPage(msg, title);
}
mainCanvas.pageStart() {
	node=this[mainNode];
	not( node ) {
		return;
	}
	cf[classErrorCheck].initNode();
	node[rect]=null;
	size=page.size();
	not( node[Width] ) {
		node[Width] = size.width();
	}
	not( node[Height] ) {
		node[Height] = size.height();
	}
	setNodeSize(node, true);
	this.conf();
	canvas.size(node[rect]);
	/*마우스 포인트 삭제*/
	this.canvas.cursor(CURSOR.BlankCursor);
	loadCommonImage(cf);
	logPath=conf('setup.kiosk#logPath');
	if( logPath ) {
		Cf.debug(true, logPath);
	}
	cf[pageStart] =true;
	cf[pageStartTick] 		= System.tick();
	cf[mainUpdateTick]	= System.tick();
	cf.currentLanguage='kor';
	arr=_arr(cf,'ActionRects', true);
	arr.add( Class.rect(0,0,65,65) );
	arr.add( Class.rect(1015,0,65,65) );
	db.exec("update kiosk_error set error_status='S' where error_status='R' ");
	System.timeout(500);
	/* qtMon 생성 */
	page.qtMonStart();
	if(cf[noSetupType].eq('1')) {
		/* 주문 스크린 생성 (생성체크 오류시 오류내역전송 및 알림창 ) */
		corner=cf[CornerInfo], idx=1, screenOrderIndex=1;
		while( cur, corner ) {
			not( cur[screen_ip] ) {
				db.fetch("SELECT screen_ip FROM kiosk_corner_setup where clplu_cd=#{clplu_cd}", cur);
			}
			if( cur[screen_ip] ) {
				idx++;
			}
		}
		while( cur, corner ) {
			not( cur[screen_ip] ) {
				continue;
			}
			fc=page[screenOrderStart$screenOrderIndex];
			fc(cur);
			screenOrderIndex++;
		}
		not( idx.eq(screenOrderIndex) ) {
			kiosk_SendError(this,'주문스크린 생성 오류', '99', cf, db );
		}
		_log("페이지 시작 성공 : 메인영역:$node[rect], 로그파일: $logPath 스크린오더 인덱스: $idx==$screenOrderIndex");
	} else if(cf[noSetupType].eq('2')) {
		printInfo=cf[PrintInfo], idx=1, screenOrderIndex=1;
		db.fetchAll("SELECT print_no, screen_ip, screen_port, kitchen_ip, kitchen_port, note, kitchen_use_yn, screen_use_yn, reg_dt FROM kiosk_print_setup", printInfo.removeAll() );
		while( cur, printInfo ) {
			print("프린트정보 $cur");
			if( cur[screen_ip] ) {
				idx++;
			}
		}
		while( cur, printInfo ) {
			not( cur[screen_ip] ) {
				continue;
			}
			fc=page[screenOrderStart$screenOrderIndex];
			fc(cur);
			screenOrderIndex++;
		}
		not( idx.eq(screenOrderIndex) ) {
			kiosk_SendError(this,'주문스크린 생성 오류', '99', cf, db );
		}
		_log("페이지 시작 성공 : 메인영역:$node[rect], 로그파일: $logPath 스크린오더 인덱스: $idx==$screenOrderIndex");
	}
}

MessageWindow.initPage(msg, title) {
	cur=this.findTag('Message');
	cur[message]=msg;
	if( title ) {
		cur=this.findTag('Title');
		cur[title]=title;
	}
	this.update();
}

ErrorWindow.ErrorWindow(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
ErrorWindow.initControl() {
	tag[Width]=710, tag[Height]=702;
	tag[BackgroundImage]="${cf[imagePath]}/main/error_window.png";
	setNodeSize(tag, true);
}
ErrorWindow.conf() {
	rc=tag[rect];
	tag[rcButtonClose]=rc.height(55).move('end', 55);
	tag[rcErrorMessage]=rc.move('bottom', 76);
}
ErrorWindow.draw(draw, tm) {
	this.drawFadeIn(draw, tm);
	drawNodeStyle(draw, tag);
	rc=tag[rcErrorMessage], msg=tag[errorMessage];
	draw.html( rc.incr(10), msg);
}
ErrorWindow.mouseDown(pos) {
	if( tag[rcButtonClose].contains(pos) ) {
		print("error widnow close button down");
	}
}
ErrorWindow.mouseUp(pos) {

}
ErrorWindow.initPage(msg, title) {
	tag[errorMessage]=msg;
	this.update();
}

OrderConfirm.orderConfirm() {
	/*
	this.mainControl().popupOpen('SelectCard');
	order		= cf[OrderHeader];
	posInfo	= cf[PosInfo];
	cart		= this.findControl('MenuCart#orderView');
	node=_node("EasyCardNode");
	data="easyCardReadData: {'SUC':'00','RQ01':'D1','RQ02':'6196144','RQ03':'A','RQ04':'4265860000000000=00002011000000032391','RQ05':'****','RQ06':'00','RQ07':'10000','RQ08':'','RQ09':'','RQ10':'','RQ11':'','RQ12':'','RQ13':'909','RQ14':'1483144826','RQ15':'web1483144826','RS01':'P','RS02':'A','RS03':'0856','RS04':'0000','RS05':'016','RS06':'0000','RS07':'1612310940436','RS08':'310951660312','RS09':'30001526','RS10':'N*','RS11':'016','RS12':'KB 기업카드','RS13':'00081658989','RS14':'KB국민카드','RS15':'d','RS16':'','RS17':'               KICC로제출','RS18':'Y','RS19':'2148608930','RS20':''})";
	data.str();
	data.findPos('{',1,1);
	node.parseJson(data);
	order_completeCardProcess(this.mainControl(), node, order, cart, posInfo);
	*/
	/* 카드 삽입 멘트 */
	System.playWave(getLocalPath('/data/wave/a.wav'));
	/* 카드 결제 설명 이미지 팝업  */
	this.mainControl().popupOpen('CardImage');
	System.timeout(250);
	/* 주문 객체 세팅  */
	order = cf[OrderHeader];
	cart=this.findControl('MenuCart#orderView');
	totalPrice = cart[OrderTotalPrice];
		/* 윈도우 Topmost 속성 제거 */
		main=this.mainControl();
		page=main.page;
		page.showTop(false);
		setup=cf[SetupInfo];
		_log("### ms_cat_id=$setup[ms_cat_id]");
		/* KSNET */
		qtMonNode = _node('QtMonNode');
		qtMonNode[data] = null;
		/* 50초 대기 */
		main.qtMonSendData("24,6,IC,01,0200,N,00,$totalPrice,50,$setup[ms_cat_id]");
		System.timeout( 2000, func() {
			while( n, 102 ) {
				if( @finish ) break;
				if( qtMonNode[data] ) {
					print("QtMonReadData : $qtMonNode[data]");
					return @event.finish(); // 타임아웃 중간에 종료시킴
				}
				System.sleep(500);
			}
		});
		/* qtMon 응답 데이터 처리 */
		ch=qtMonNode[data].ch();
		if( ch.eq('$') ) {
			recv=qtMonNode[data].value(1);
			/* 명령번호, 파라메터수, 상태, 오류 메시지 , 카드번호 , 승인일시 , 승인번호 , 매입사코드 , 발급사이름*/
			recv.split().inject(commandNum, paramCount, status, message, cardNo,
				arvDate, arvNo, purCardNo, purCardNm);
			if( commandNum.eq(24) && status.eq('O') ) {
				node           = _node('CardInfoNode');
				node[RQ04] = cardNo;
				node[RS07] = arvDate;
				node[RS09] = arvNo;
				node[RS05] = purCardNo;
				node[RS12] = purCardNm.utf8();
				order_completeCardProcess(main, node, order, cart);
			} else {
				msg = message.utf8();
				main.alert("카드 승인중 오류가 발생했습니다.\n오류내용: $msg", "알림");
				/* 결제하기 버튼 초기화 */
				ctrl=this.findControl('MainStatus#buttonsView');
				ctrl[mouseDownRect]=null;
			}
		} else {
			main.alert("결제 대기 시간을 초과하였습니다.", "알림");
			/* 결제하기 버튼 초기화 */
			ctrl=this.findControl('MainStatus#buttonsView');
			ctrl[mouseDownRect]=null;
		}
		/* 윈도우 Topmost 속성 설정 */
	   page.showTop(true);
}
OrderConfirm.orderCancel() {
	this.mainControl().popupClose();
	ctrl=this.findControl('MainStatus#buttonsView');
	ctrl[mouseDownRect]=null;
	this.update();
}
OrderConfirm.drawFadeIn(draw, timeline) {
	popupFadeIn(draw, timeline);
}

ErrorWindow.drawFadeIn(draw, timeline) {
	popupFadeIn(draw, timeline);
}

AdminMenuCanvas.AdminMenuCanvas(page) {
	this.addClass('common.CanvasBase');
	canvas=page.canvas;
	canvas.eventMap( onDraw, this.canvasDraw, 'draw');
	canvas.eventMap( onMouseDown, this.canvasMouseDown, 'pos');
	canvas.eventMap( onMouseUp, this.canvasMouseUp, 'pos');
	canvas.eventMap( onMouseMove, this.canvasMouseMove, 'pos');
	canvas.eventMap( onEvent, this.canvasEvent, 'type, node');
	page.eventMap( onActivationChange, this.widgetPageCheck );
	page.eventMap( onMove, this.moveWidget );
	/* 타이머 설정 */
	canvas.timer( 500, callback() {
		this.timeout();
	}, this);
	/* 설정정보 세팅 */
	this.initConfig();
	this.initPage();
}
AdminMenuCanvas.initConfig() {
	cf.debug=true;
	cf.pageMode='full';
	cf.projectId ='KioskHiTec';
	cf.pageCode='AdminMenu';
	cf.imagePath=conf('setup#kiosk.imagePath');
	/* 키오스크 설정정보 조회 */
	cf.SetupInfo={tag:SetupInfo};
	setup=cf[SetupInfo];
	Class.db('kiosk_hitec').fetch("
	   SELECT A.ms_no, A.pos_no, A.service_start_time, A.service_end_time, A.refresh_time, A.order_start_no, A.order_end_no,
					A.qt_mon_ip, A.qt_mon_port, A.emp_id, A.emp_pw, A.kiosk_id, A.kiosk_pw,
					B.van_cd, B.ms_cat_id
			  FROM kiosk_setup A
			   LEFT JOIN hitec_m60s B ON A.ms_no = B.ms_no AND A.pos_no = B.pos_no AND B.use_yn = 'Y'
			WHERE A.use_yn='Y'
			  LIMIT 1",  setup);
	/* 카드 VAN사 타입 */
	/* 1:KCP, 2:KSNET */
	cf.cardVanType	= conf('setup#kiosk.cardVanType');
	not(cf.cardVanType) cf.cardVanType = "2";
	/* 번호 설정 타입 : 주방프린트IP, 주문스크린IP */
	/* 1:코너별(기존), 2:번호별 */
	cf.noSetupType	= conf('setup#kiosk.noSetupType');
	not(cf.noSetupType) cf.noSetupType = "2";
	this.timelineAdd('FadeInPopup', 800, 15, 'in');
}
AdminMenuCanvas.initPage() {
	cf.pageStart=false;
	cf.pageRate=1;
}
AdminMenuCanvas.canvasDraw(draw) {
	tm=getDrawTimeline( timelineNode );
	if( cf[stackPage] ) {
		cf[stackPage].draw(draw, tm);
	} else {
		if( cf[popupControl] ) {
			this.draw(draw);
			cf[popupControl].draw(draw,tm);
		} else {
			this.draw(draw, tm);
		}
	}
	if( cf[selectedItem] ) {
		rc=cf.selectedItem.rect;
		draw.rectLine(rc.incr(1), 0, '#afa0ea',3);
	}
	if( cf[mouseDownAction] ) {
		draw.save().pen('#cab0e9', 4);
		draw.polyLine(cf[mouseActionPoints]);
		draw.restore();
	}
	if( cf[submenuNode] ) {
		node=cf[submenuNode];
		draw.drawImage(node[rect submenu].incrX(1), node[draw submenu]);
		menu=this.findControl('MyMenu');
		while(sub, node) {
			not( sub[rect].eq(menu.mouseOverRect) ) continue;
			rc=sub[rect].incrX(-10).incrW(4);
			if( sub==this[currentMenu] ) {
				draw.fill(rc, '#c0cad060');
			} else {
				draw.fill(rc, '#dacad040');
			}
		}
	}
	if( cf[inputNode] && cf[inputFocusDraw] ) {
		mod=cf[cursorIndex] %2;
		not( mod ) {
			cur=cf[inputNode];
			cur[rect].inject(x,y,w,h);
			tw=textWidth(14, cur[text],'bold');
			x+=tw-6, y+=5, h-=10;
			draw.fill( Class.rect(x,y,2,h), '#909080a0');
		}
	}
}
AdminMenuCanvas.canvasMouseDown(pos) {
	while( rc, cf[ActionRects] ) {
		if( rc.contains(pos) ) {
			_arr(cf,'mouseActionPoints').reuse();
			cf[mouseDownAction]=true;
		}
	}
	if( cf[stackPage] ) {
		cf[stackPage].mouseDown(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseDown(pos);
		return;
	}
	this.mouseDown(pos);
}
AdminMenuCanvas.canvasMouseMove(pos) {
	if( cf[mouseDownAction] ) {
		cf[mouseActionPoints].add(pos);
		this.update();
	}
	this.mouseMove(pos);
}
AdminMenuCanvas.canvasMouseUp(pos) {
	if( cf[mouseDownAction] && canvasMouseAction(this) ) {
		return;
	}
	if( cf[stackPage] ) {
		cf[stackPage].mouseUp(pos);
		return;
	} else if( cf[popupControl] ) {
		cf[popupControl].mouseUp(pos);
		return;
	}
	this.mouseUp(pos);
}
AdminMenuCanvas.canvasEvent(type, node) {
	switch( type ) {
	case KIOSK.Log:
		if( cf.debugEditor ) cf.debugEditor.append( tag[logMessage], true );
	default: break;
	}
}
AdminMenuCanvas.timeout() {
	not( cf[pageStart] ) {
		return;
	}
	/* 타이머 처리로직 구현*/
	if( cf[inputNode] ) {
		cf[cursorIndex++];
		this.update();
	}
	if( cf[popupCloseCheck] ) {
		cf[popupCloseCheck]=false;
		this.findControl('Popup#dialog').popupClose();
	}
}


MyMenu.MyMenu(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
MyMenu.initControl() {
	/* 전역 마우스 이벤트 처리 */
	tag[GlobalMouseUse]=true;
	setNodeSize(tag, true);
	tag.addNode({tag: MenuBar});
	tag.addNode({tag: MenuStatus});
	this.setMenu();
}
MyMenu.conf() {
	/* 메뉴 영역 설정 */
	confMenu=func(node, rc) {
		divideRect(node, rc,
			'250,*',
			'image, menu'
		);
		node[rect menu].inject(x,y,w,h), right=x+w;
		while( menu, node ) {
			tw=textWidth(14, menu[value])+40;
			menu[rect]			=Class.rect(x,y,tw,h), x+=tw;
			menu[rect sep]		=Class.rect(x,y,8,h), x+=8;
		}
		tw=right-x;
		if( tw<=0 ) tw=1;
		node[rect background]=Class.rect(x,y,tw,h);
	};
	/* 영역설정*/
	divideRect( tag, tag[rect], "60,*", null, true);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case MenuBar:	confMenu(cur, cur[rect]);
		case MenuStatus:
		default:
		}
	}
}
MyMenu.draw(draw, tm) {
	draw.mode();
	drawNodeStyle(draw, tag);
	/* 메뉴 그리기 */
	drawMenu=func(node) {
		draw.drawImage( node[rect image], commonImage('menu_bg'), 'fill' );
		rcLogo=node[rect image],width(200);
		draw.font(16,'bold','#d0d0d0').text( rcLogo, '고속도로 KIOSK', 'center');
		while( menu, node ) {
			rcText=menu[rect text].incrY(18);
			divideRect(menu, menu[rect], '9,*,8', 'left, text, right');
			if( menu[selected] ) {
				draw.drawImage( menu[rect left], 	commonImage('menu_tab1') );
				draw.drawImage( menu[rect text], 	commonImage('menu_tab2'), 'fill' );
				draw.drawImage( menu[rect right],	commonImage('menu_tab3') );
				textImage(draw, menu, rcText, menu[value], 'menuSelect');
			} else {
				draw.drawImage( menu[rect], commonImage('menu_bg'), 'fill' );
				menu[drawObject]=null;
				textImage(draw, menu, rcText, menu[value], 'menuNormal');
				draw.drawImage( menu[rect text].move('end', 12), commonImage('menu_down') );
			}
			draw.drawImage( menu[rect sep], 	commonImage('menu_bg'), 'fill' );
			if( menu[rect].eq(this.mouseOverRect) ) {
				draw.fill(rcText.incr(-3), '#c0cad040');
			}
		}
		draw.drawImage( node[rect background], commonImage('menu_bg'), 'fill' );
	};
	/* 그리기 태그 처리 */
	while( cur, tag ) {
		switch( cur[tag] ) {
		case MenuBar: drawMenu(cur);
		case MenuStatus:
			rc=cur[rect];
			draw.fill( rc, '#ffffff').rectLine( rc, 4, '#202020');
			draw.rectLine( cur[rect].incrY(1,true), 4, '#c0c0c0');
			draw.font(12, "normal", '#9090a0').text(rc.incrX(10), cur[text]);
		default:
		}
	}
}
MyMenu.mouseDown(pos) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case MenuBar:
			if( cf[submenuNode] ) {
				while(sub, cf[submenuNode] ) {
					not( sub[rect].contains(pos) ) continue;
					this.setCurrentMenu(sub);
					return;
				}
			}
			while( menu, cur ) {
				not( menu[rect].contains(pos) ) continue;
				if( menu[selected] && cf[submenuNode] ) return;
				this.menuChange(menu);
				return;
			}
		default:
		}
	}
	if( cf[submenuNode] ) {
		this.closeSubmenu();
	}
}
MyMenu.mouseUp(pos) {

}
MyMenu.test() {
	this.setMenu();
	this.conf();
	this.update();
}
MyMenu.setMenu(data) {
	not( data ) data=conf('data.menu');
	root=findTag('MenuBar',tag).removeAll();
	parseMenu=func(s) {
		menu=null;
		while( s.valid() ) {
			line=s.findPos("\n");
			not( line.ch() ) continue;
			if( line.ch().eq('-') ) {
				line.incr();
				sub=menu.addNode({tag:submenu});
				sub[value]=line.findPos('#').trim();
				if( line.valid() ) sub.parseJson(line);
			} else {
				line.findPos('.');
				menu=root.addNode({tag:menu});
				menu[value]=line.trim();
			}
		}
	};
	parseMenu(data.ref() );
	printNode(root);
}
MyMenu.mouseMove(pos) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case MenuBar:
			if( cf[submenuNode] ) {
				while(sub, cf[submenuNode] ) {
					not( sub[rect].contains(pos) ) continue;
					if( sub[rect].eq(this.mouseOverRect) ) return;
					this.mouseOverRect=sub[rect];
					this.update();
					return;
				}
			}
			while( menu, cur ) {
				not( menu[rect].contains(pos) ) continue;
				if( menu[selected] ) {
					break;
				}
				if( menu[rect].eq(this.mouseOverRect) ) return;
				this.mouseOverRect=menu[rect];
				this.update();
				return;
			}
		default:
		}
	}
	if( this[mouseOverRect] ) {
		this[mouseOverRect]=null;
		this.update();
	}
}
MyMenu.menuChange(menu) {
	while( cur, this.findTag('MenuBar') ) {
		if( cur==menu ) continue;
		if( cur[selected] ) cur[selected]=false;
	}
	menu[selected]=true;
	this.setSubmenu(menu);
	this.update();
}
MyMenu.setSubmenu(node, rc) {
	not( rc ) rc=node[rect].move('down');
	rc.inject(x, y, w, h);
	draw=node[draw submenu];
	not( draw ) {
		/* 최대 폭/높이 계산 */
		mw=0, mh=0;
		while( cur, node ) {
			tw=textWidth(12, cur[value] ) +35;
			if( mw<tw ) mw=tw;
			mh+=35;
		}
		if( mw< w ) mw=w;
		mh+=20;
		draw = Class.draw( mw, mh );
	}
	rcDraw=draw.rect();
	/* 출력 영역 */
	node[rect submenu]=Class.rect(x, y, rcDraw.size() );
	offset=Class.point(x,y);
	/* 그리기 영역 */
	divideRect( node, rcDraw, '*,14', 'body, bottom', true);
	divideRect( node, node[rect body], 		'9,*,10', 'box11,box12, box13');
	divideRect( node, node[rect bottom],	'9,*,10', 'box21,box22, box23');
	rcBody=node[rect box12];
	/* 배경 그리기 */
	draw.drawImage( node[rect box11], commonImage('submenu_box11'), 'fill' );
	draw.fill( rcBody, '#ffffff' );
	draw.drawImage( node[rect box13], commonImage('submenu_box13'), 'fill' );
	draw.drawImage( node[rect box21], commonImage('submenu_box21') );
	draw.drawImage( node[rect box22], commonImage('submenu_box22'), 'fill' );
	draw.drawImage( node[rect box23], commonImage('submenu_box23') );
	/* 텍스트 그리기 */
	rcBody.inject(x,y,w,h), x+=5, y+=8, h=34;
	while( cur, node ) {
		rc=Class.rect(x,y,w,h), y+=h;
		cur[rect]=rc.incrXY(offset, true);
		textImage(draw, cur, rc, cur[value], 'submenu' );
	}
	node[draw submenu]=draw;
	cf[submenuNode]=node;
	widgetNode=_node(cf,'widgetNode');
	while( pid, widgetNode.keys() ) widgetNode[$pid].hide();
	this.update();
}
MyMenu.setCurrentMenu(menu) {
	not( menu ) return;
	this[currentMenu]=menu;
	this.closeSubmenu();
	if( menu[id].eq('ClosePage') ) {
		this.findControl('#Content').pageLoad('LoginView');
		System.timeout(100);
		kw=Cf[KioskWatcher];
		kw.hide();
		return;
	}
	loginInfo=_node('LoginInfo');
	if( loginInfo[loginStartTick] ) {
		dist=System.tick() - loginInfo[loginStartTick];
		if( dist>200000 ) {
			this.findControl('#Content').pageLoad('LoginView');
			System.timeout(100);
			this.mainControl().alert("로그인 세션이 말료되었습니다. 다시 로그인 하세요", "알림");
			return;
		}
	} else {
		this.findControl('#Content').pageLoad('LoginView');
		System.timeout(100);
		this.mainControl().alert("로그인 정보가 없습니다. 로그인후 이용하세요.", "알림");
		return;
	}
	cf[inputFocusDraw]=null;
	/* 페이지 이동처리 */
	content = this.findControl('#Content');
	switch( menu[id] ) {
	case SalesOpen:
		content.pageLoad("AdminSaleOpen");
	case SlaesClose:
		content.pageLoad("AdminSaleClose");
	case SalesStatus:
		content.pageLoad("AdminSaleStatus");
	case SoldOut:
		content.pageLoad("AdminSoldOut");
	case ErrorView:
		content.pageLoad("ErrorView", "admin");
	case LogView:
		content.showPage('KioskLogViewer');
	case DbConnect:
		content.showPage('dbManager');
	case SetupInterface:
		content.pageLoad("KeyboardTool");
	case ClosePage:
		this.findControl('#Content').pageLoad('LoginView');
		System.timeout(100);
		kw=Cf[KioskWatcher];
		kw.hide();
	case ExitPage:
		main=this.mainControl();
		mainPage=main[page];
		not( mainPage.confirm("프로그램을 닫으면 자동업데이트 및 에러처리를 하지못하게 됩니다.\n프로그램을 닫으시겠습니까?") ) {
			return;
		}
		mainPage[tray].hide();
		Cf.exit();
	case InterfaceManager:
		content.pageLoad("KeyboardTool");
	case NetworkTool:
		content.showPage('PingTest');
	case OrderTool:
		content.showPage('OrderTool');
	case DbTool:
		content.showPage('DbQuery');
	case ProcessTool:
		content.showPage('processInfoView');
	case ProtocalTool:
		content.showPage('protocalTest');
	case KeyboardTool:
		content.pageLoad("KeyboardTool");
	default:
		content.pageLoad(menu[id],"admin");
	}
	print(menu,  content.pageLoad );
	status=this.findTag('MenuStatus');
	p=menu.parent();
	status[text]="관리자 메뉴 : $p[value] > $menu[value]";
}
MyMenu.closeSubmenu() {
	while( cur, this.findTag('MenuBar') ) {
		if( cur[selected] ) cur[selected]=false;
	}
	cur=this[currentMenu];
	if( cur ) {
		pp=cur.parent();
		pp[selected]=true;
	}
	/* 서브 메뉴 창을 닫고, 선택 메뉴를 변경한다. */
	cf[submenuNode]=null;
	this.update();
}


Content.Content(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this[pageNode]=null;
	this.addClass('common/control.PageBase');
	this.initControl();
}
Content.initControl() {
	setNodeSize(tag, true);
}
Content.conf() {
	main=this[pageNode];
	not( main ) return;
	tagClearRect(main);
	if( cf[pageMode].eq('full') ) {
		main.varMap(tag, 'rect, Width, Height', true);
		setNodeSize(main, true);
	} else {
		main[rect]=null;
		setNodeSize(main, true);
		if( main[rect] ) {
			rc=main[rect], r=rc.right(), b=rc.bottom();
			tag[rect]=rc;
			ctrl=this.mainControl();
			ctrl[canvas].size(r,b);
		}
	}
	this.getControl(main).conf();
}
Content.findPageNode(tagName) {
	return findTag(tagName, this[pageNode]);
}
Content.draw(draw, tm) {
	/* 위젯 그리기 */
	subpage=this[currentShowPage];
	if( subpage ) {
		this.mainControl().drawSubPage(draw, subpage, tag[rect]);
		return;
	}
	/* 캔버스 그리기 */
	subpage=this[pageNode];
	if( subpage ) {
		if( subpage[bg] ) {
			tag[bg]=subpage[bg];
		}
		if( tag[bg] ) {
			draw.drawImage(tag[rect], imageLoad(tag, "bg"), 'fill');
		}
		this.getControl(subpage).draw(draw, tm);
	}
}
Content.mouseDown(pos) {
	main=this[pageNode];
	not( main ) return;
	this.getControl(main).mouseDown(pos);
}
Content.mouseUp(pos) {
	main=this[pageNode];
	not( main ) return;
	this.getControl(main).mouseUp(pos);
}
Content.mouseMove(pos) {
	main=this[pageNode];
	not( main ) return;
	this.getControl(main).mouseMove(pos);
}
Content.setPage(pageId, projectId) {
	not( projectId ) projectId=cf[projectId];
	id="${projectId}.${pageId}";
	node=tag.findOne('id',id);
	if( node ) {
		print("## $id =>$node");
		this.setPageNode(node);
		return;
	}
	cf.inject(imagePath);
	pageXml=getPageXml(projectId, pageId);
	node=this.parseXml( fmt(pageXml), tag );
	node[id]=id;
	if( node[tag].eq('Page') ) {
		node[tag]="SubPage";
	}
	/* 서브페이지 처리 (팝업, 클래스 경로 설정) */
	arr=_arr();
		while( cur, node ) {
		if( cur[tag].eq('Popup') ) {
			not( cur[ClassPath] ) cur[ClassPath]='common';
			arr.add(cur);
		} else {
			not( cur[ClassPath] ) cur[ClassPath]="${projectId}/${pageId}";
		}
	}
	while( cur, arr ) {
		node.remove(cur);
	}
	root=xmlNode.child(0);
	while( cur, arr ) {
		root.addNode(cur);
	}
	this.setPageNode(node);
	/* test code */
	switch( pageId ) {
	case MyGrid:
		gird=this.findPageNode('GridContol');
		this.getControl(grid).test();
	default:
		cur=node.child(0);
		this.getControl(cur).test();
	}
}
Content.setPageNode(node) {
	this[pageNode]=node;
	this.conf();
	this.update();
}
Content.test() {
	tag.removeAll();
	this.pageLoad('LoginView');
}
Content.showPage(pageId, projectId) {
	print("showPage=========$pageId");
	cf[inputNode]=null;
	if( this[currentShowPage] ) {
		this[currentShowPage].hide();
	}
	not( projectId ) projectId=cf[projectId];
	id="${projectId}.${pageId}";
	widget=this.mainControl().showWidget(id, tag[rect]);
	not( widget ) return null;
	find=tag.findOne('pageId', id);
	not( find ) {
		widget[id]=pageId;
		widget[pageId]=id;
		tag.addNode(widget);
	}
	this[currentShowPage]=widget;
	return widget;
}


AdminMenuCanvas.pageStart() {
	node=this[mainNode];
	not( node ) {
		return;
	}
	cf[classErrorCheck].initNode();
	node[rect]=null;
	size=page.size();
	not( node[Width] ) {
		node[Width] = size.width();
	}
	not( node[Height] ) {
		node[Height] = size.height();
	}
	setNodeSize(node, true);
	this.conf();
	canvas.size(node[rect]);
	cf[lastUpdateTick]=System.tick();
	loadCommonImage(cf);
	cf.pageStart	=true;
	cf.pageStartTick 	=System.tick();
	logPath=conf('setup.kiosk#logPath');
	if( logPath ) {
		Cf.debug(true, logPath);
	}
	setup=_node('SetupInfo');
	Class.db('kiosk_hitec').fetch(conf("sql#hitec.selectKioskSetup"),  setup);
	this.findControl('Content').pageLoad('LoginView');
	_log("페이지 시작 성공 : 메인영역:$node[rect], 로그파일: $logFile");

}

	Content.pageLoad(pageId, path) {
	print("Content::pageLoad => $pageId 로딩 ");
	cf.currentPage = pageId; /* 현재페이지 설정 */
	cf[inputNode]=null;
	cur=tag.findOne('id', pageId);
	not( cur ) {
		src=conf("page#xml.kiosk#$pageId");
		if( src ) {
			cf.inject(imagePath, projectId, pageCode);
			cur= this.parseXml( fmt(src), tag );
		} else {
			cur=tag.addNode();
			cur[tag]	=pageId;
		}
		not( path ) {
			path=nvl( cur[ClassPath], 'tool' );
		}
		cur[id]=pageId;
		cur[ClassPath]=path;
	}
	if( cur[ClassPath].eq('admin') ) {
		cf.pageMode='scroll';
	} else {
		cf.pageMode='full';
	}
	ctrl=this.getControl(cur);
	not( ctrl ) {
		print("Content::pageLoad error => $pageId 로딩 실패 ");
		return;
	}
	this[pageNode]=cur;
	this.conf();
	if( this[currentShowPage] ) {
		this[currentShowPage].hide();
		this[currentShowPage]=null;
	}
	/* 자동으로 조회되도록 한다 */
	if( ctrl.search ) {
		ctrl.search();
	}
	/* 페이지 열기 애니메이션 */
	if( ctrl.pageOpenEvent ) {
		this.timelineStart('PageEffect',  this, 'open');
	} else {
		this.update();
	}
}

LoginView.LoginView(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
LoginView.conf() {
	cf[inputFocusDraw] = false;
	cf.inject(imagePath);
	rc=tag[rect].center(705,850);
	confNodeLayout(tag, rc.lt() );
	/* 숫자입력 패드 영역 설정 */
	cur=this.findTag('AdminNumberPad');
	bw=cur[ButtonWidth], bh=cur[ButtonHeight];
	cur[rect].inject(sx, sy);
	num=1;
	while( row,4 ) {
		cx=sx;
		while( col,3 ) {
			if( row.eq(3) ) {
				switch(col) {
				case 0:	img="login_num_00_[#].png";
				case 1:	img="login_num_del_[#].png";
				case 2:	img="login_num_re_[#].png";
				}
			} else {
				img="login_num_0${num}_[#].png";
				num++;
			}
			cur[rect#$row $col]=Class.rect(cx, sy, bw, bh), cx+=bw+10;
			cur[img#$row $col]="${imagePath}/admin/$img";
		}
		sy+=bh+10;
	}
}
LoginView.draw(draw, timeline) {
	draw.drawImage( tag[rect], commonImage('admin_bg1'), 'fill');
	tag[rect].inject(x,y);
	rc=tag[rect].center(705,850);
	draw.drawImage(rc, imageLoad(tag, "bkImage") );
	while( cur, tag ) {
		switch(cur[tag]) {
		case LogInButton:
			var=when( cur[rect].eq(this.mouseDownRect), 'p', 'n');
			img=imageLoad(cur, "src", var);
			draw.drawImage(cur[rect], img);
		case AdminNumberPad:
			while( row,4 ) {
				while( col,3 ) {
					rc=cur[rect#$row $col];
					var=when( rc.eq(this.mouseDownRect), 'p', 'n');
					img=imageLoad(cur, "img#$row $col", var);
					draw.drawImage(rc, img);
				}
			}
		case UserNameLabel:
			if( cur[text] ) drawNodeText(draw, cur[rect].center(315,55), cur[text], 'left', 'TableHeader');
		case PassWordLabel:
			if( cur[text] ) {
				text='', size=cur[text].size();
				while(n,size) {
					text.add('*');
				}
				drawNodeText(draw, cur[rect].center(315,55), text, 'left', 'TableHeader');
			}
		default:
		}
	}
	if( cf[inputFocusRect] ) {
		draw.effect(
			DRAW.RoundBox, cf[inputFocusRect], 5, '#303040', '#f9f90030', 4
		);
		mod=cf[cursorIndex] %2;
		not( mod ) {
			input=cf[inputNode], rc=cf[inputFocusRect].center(315,55);
			rc.inject(x,y,w,h);
			tw=textWidth(24, input[text],'bold', 4);
			x+=tw, y+=10, h-=14;
			draw.fill( Class.rect(x,y,2,h), '#909080a0');
		}
	}
}
LoginView.exitButtonClick() {
	this.findControl('Popup#stack').stackPageClose();
}
LoginView.initControl() {
	tag[bkImage]="$cf[imagePath]/main/common/LoginView_bg.png";
}
LoginView.loginButtonClick() {
	user=findTag('UserNameLabel', tag);
	pwd=findTag('PassWordLabel', tag);
	this.setLoginConfig( user[text], pwd[text] );
}
LoginView.mouseDown(pos) {
	while( cur, tag ) {
		switch(cur[tag]) {
		case AdminNumberPad:
			num=1;
			while( row,4 ) {
				while( col,3 ) {
					rc=cur[rect#$row $col];
					if( row.eq(3) ) {
						switch( col ) {
						case 0: key=0;
						case 1: key='Delete';
						case 2: key='Reset';
						}
					} else {
						key=num;
						num++;
					}
					not( rc.contains(pos) ) {
						continue;
					}
					this.mouseDownRect=rc;
					this.numberButtonClick(key);
					this.update();
					return;
				}
			}
		case UserNameLabel:
			if( cur[rect].contains(pos) ) {
				cf[inputNode]=this.findTag('UserNameLabel');
				cf[inputFocusRect]=cur[rect];
				this.update();
				return;
			}
		case PassWordLabel:
			if( cur[rect].contains(pos) ) {
				cf[inputNode]=this.findTag('PassWordLabel');
				cf[inputFocusRect]=cur[rect];
				this.update();
				return;
			}
		default:
			not( cur[rect].contains(pos) ) continue;
			this.mouseDownRect=cur[rect];
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
LoginView.mouseUp(pos) {
	while( cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag]) {
		case LogInButton:	this.loginButtonClick();
		case ExitButton:		this.exitButtonClick();
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
LoginView.numberButtonClick(key) {
	not( cf[inputFocusRect] ) {
		user=this.findTag('UserNameLabel');
		cf[inputNode]=this.findTag('UserNameLabel');
		cf[inputFocusRect]=user[rect];
	}
	input=cf[inputNode];
	not( input ) return;
	not( isset(input[text]) ) {
		input[text]='';
	}
	switch(key ) {
	case Delete:
		val=input[text];
		input[text]=val.value(0,-1);
	case Reset:
		input[text]='';
	default:
		input[text].add(key);
	}
	cf[cursorIndex]=0;
	this.update();
}
LoginView.setLoginConfig(kiosk_id, kiosk_pw) {
	not( kiosk_id ) {
		this.mainControl().alert("사용자 정보가 입력되지 않았습니다.", "알림");
		return;
	}
	not( kiosk_pw ) {
		this.mainControl().alert("비밀번호가 입력되지 않았습니다.", "알림");
		return;
	}
	db=Class.db('kiosk_hitec');
	cf=_node('LoginInfo');
	cf.put(kiosk_id, kiosk_pw);
	not( db.count("select count(1) from kiosk_setup where kiosk_id=#{kiosk_id}", cf) ) {
		this.messageBox("사용자 정보가 없습니다.", "알림");
		return;
	}
	cnt=db.count("select count(1) as cnt  from kiosk_setup where kiosk_id=#{kiosk_id} and kiosk_pw=#{kiosk_pw}", cf);
	not( cnt ) {
		this.messageBox("비밀번호가 일치하지 않았습니다.", "알림");
		return;
	}
	db.fetch("select ms_no, pos_no from kiosk_setup where kiosk_id=#{kiosk_id}", cf);
	cf[loginStartTick]=System.tick();
	pwd=findTag('PassWordLabel', tag);
	pwd[text]='';
	this.findControl('#Content').pageLoad('AdminHome');
}


Content.popupOpen(pageId, parent, rect, style, path) {
	not( path ) {
		path='common';
	}
	not( style ) style="popup";
	tag=parent[tag];
	dlg=this.findControl('Popup#dialog');
	dlg.popupOpen(pageId, path, rect, tag[rect], style);
	node=dlg[mainNode];
	if( node ) {
		node[openerControl]=parent;
		node[autoClose]=true;
	}
	return node;
}

MyMenu.setMenuStatus(info) {
	this.update();
}

Calendar.setDateSelect(year, month, day) {
	not( year ) return;
	ctrl=tag[openerControl];
	if( ctrl ) {
		dd=lpad(day,2);
		ctrl.setCalendarDate("$year-$month-$dd");
	}
	parentCtrl.popupClose();
}

SaleStatusView.confListBody(cur, start) {
	not( start ) start=0;
	dataNode=_node(cf,'dataNode');
	total=dataNode.childCount();
	not( total ) return;
	sp=start, ep=sp+16, hh=64;
	if( ep>total ) ep=total;
	cur[rect].inject(sx, sy, sw, sh);
	while( n, ep, sp ) {
		row=dataNode.child(n);
		row[rect]=Class.rect(sx, sy, sw, hh), sy+=hh;
	}
	this.startRow=sp;
}
SaleStatusView.setCalendarDate(date) {
	input=this.currentInput;
	input[text]=date;
	this.update();
}

CommCombo.CommCombo(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
CommCombo.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag: TopTitle});
	tag.addNode({tag: Body});
	tag.addNode({tag: Status});
}
CommCombo.conf() {
	cf.inject(imagePath);
	tag[rect].inject(sx, sy, sw, sh);
	cw=tag[ContentWidth];						not( cw ) cw=600;
	sw=cw+60;
	_setCodeData=func(node, cur) {
		not( node ) {
			return;
		}
		cell=nvl( tag[CellCount], 3 );
		cur[rect 2].inject(x,y,w,h);
		wa=_arr(tag,'WidthArray').recalc(w, cell );
		idx=0, cy=y+25, ch=60;
		while( row, 5 ) {
			cx=x;
			while( cw, wa, col, 0 ) {
				sub=node.child(idx), idx++;
				not( sub ) {
					break;
				}
				sub[rect]=Class.rect(cx,cy,cw,ch), cx+=cw;
			}
			cy+=ch;
		}
	};
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			cx=sx, sh=99;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		case Body:
			cx=sx, sh=tag[ContentHeight];		not( sh ) sh=450;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
			_setCodeData( tag[CodeNode], cur );
		case Status:
			cx=sx, sh=40;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		}
		cur[rect]=Class.rect(sx, sy, sw, sh),	sy+=sh;
	}
}
CommCombo.draw(draw, timeline) {
	setDrawOpacity(draw, timeline);
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box1$idx") );
			}
			drawNodeText(draw, cur[rect 2], tag[CodeText], 'left', 32, '#f0f0f0');
		case Body:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box2$idx") );
			}
			draw.save().font(15,'normal','#404040');
			while( sub, tag[CodeNode] ) {
				rc=sub[rect];
				rcIcon=rc.width(38).center(28,28);
				rc.incrX(38);
				imgId=when( sub[checked], "radio_on", "radio_off");
				draw.drawImage(rcIcon, commonImage(imgId) );
				draw.text(rc, sub[value]);
				not( sub[textWidth] ) {
					sub[textWidth]=draw.textWidth(sub[value]) + 38;
				}
			}
			draw.restore();
		case Status:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box3$idx") );
			}
		}
	}
	if( this.mouseOverNode ) {
		node=this.mouseOverNode;
		rc=node[rect].incrX(38).width(node[textWidth] );
		draw.fill(rc,'#d0d0d0').rectLine(rc, 0, '#c0c0c0');
		draw.font(15,'normal','#606090').text(rc.incrX(4), node[value]);
	}
}
CommCombo.mouseDown(pos) {
	this.mouseOverNode=null;
	while( sub, tag[CodeNode] ) {
		if( sub[rect].contains(pos) ) {
			ctrl=tag[openerControl];
			if( ctrl ) {
				if( sub[code].eq('00') ) {
					sub[code]='';
				}
				ctrl.setCommCode(sub);
				cf[popupCloseCheck]=true;
			}
			sub[checked]=true;
		} else {
			sub[checked]=false;
		}
	}
	this.update();
}
CommCombo.mouseUp(pos) {

}
CommCombo.setCommCode(code, text, targetNode, cellCount, def) {
	node= when( typeof(code,'node'), code,  getCommCodeNode(code, def) );
	tag[CodeText]	= text;
	tag[CodeNode]	= node;
	tag[TargetNode]	= targetNode;
	tag[CellCount]	= nvl(cellCount,3);
	this.conf();
	this.update();
}
CommCombo.setOptionValue(code) {
	while( cur, tag[CodeNode] ) {
		if( cur[code].eq(code) ) {
			cur[checked]=true;
		} else {
			cur[checked]=false;
		}
	}
	this.update();
}
CommCombo.mouseMove(pos) {
	while( sub, tag[CodeNode] ) {
		if( sub[rect].contains(pos) ) {
			if( this.mouseOverNode==sub ) return;
			if( sub[rect].width() < sub[textWidth] ) {
				this.mouseOverNode=sub;
				this.update();
				return;
			}
		}
	}
	if( this.mouseOverNode ) {
		this.mouseOverNode=null;
		this.update();
	}
}
CommCombo.test() {
	loadCommonImage(cf);
	this.conf();
	this.update();
}

SaleStatusView.setCommCode(node) {
	input=this.currentInput;
	input[text]=node[code];
	this.update();
}

NumberPad.buttonClick(row, col) {
	val=null;
	if( row.eq(3) ) {
		switch(col) {
		case 0:	val='0';
		case 1:	val='delete';
		case 2:	val='reset'
		}
	} else {
		row*=3, row+=col;
		row+=1;
		val=row;
	}
	ctrl=tag[openerControl];
	if( ctrl ) {
		input=ctrl.currentInput;
		if( input ) {
			this.setInputText(input, val);
		} else {
			ctrl.setNumberPadValue(val);
		}
	}
}

SaleStatusView.search(form) {
	setup=_node('SetupInfo');
	model=Class.model('SaleStatus');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	form=this.findTag('FormSearch');
	date=form.child(0);
	cancle=form.child(1);
	deal=form.child(2);
	not( date[text] ) {
		db.fetch("SELECT max(open_date) as open_date FROM kiosk_open_close where close_date is null", date);
		date[text]=util_formatDate( date[open_date]);
	}
	if( form ) {
		root[open_date]	= date[text].replace('-','');
		root[pos_no]		= setup[pos_no];
		root[cancle_yn]=cancle[text];
		root[bill_no]=deal[text];
	}
	db.fetchAll( conf("sql#hitec.SaleStatus"), root.removeAll() );
	this.findControl('GridControl').setModel(root, "
		pos_no: 		포스			#15,
		bill_no: 			거래번호	#30,
		sale_date:		날짜			#40,
		arv_dt:			승인일	 	#40,
		total_amt:		결제금액	#20,
		cancle_bill_no: 취소번호 #30,
		cancle_yn:		취소여부	#35
	");
	this.dataModel=model;
}
SaleStatusView.test() {
	rc=Class.rect(0,0,936,1244);
	this.findControl('#Content').popupOpen('CancleConfirm', this, rc, 'center', 'popup');
}

KeyboardTool.KeyboardTool(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
KeyboardTool.initControl() {
	setNodeSize(tag, true);
}
KeyboardTool.conf() {
	setNodeSize(tag, true);
	confNodeLayout(tag);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Form:
			rc=cur[rect].incr(20).height(65);
			addFormElement(cur, 'address', 'input', rc, 450, 40, '주소 : ');
		case Keyboard:
			this.getControl(cur).conf();
		default:
		}
	}
}
KeyboardTool.draw(draw, tm) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Form:
			drawFormElement(draw, cur, 'address' );
		case Keyboard: this.getControl(cur).draw(draw, tm);
		default:
		}
	}
}
KeyboardTool.mouseDown(pos) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Form:
			while( sub, cur ) {
				not( sub[rect].contains(pos) ) continue;
				if( sub[tag].eq('input') ) {
					this.mainControl().setInputNode(sub);
				}
				return;
			}
		case Keyboard: this.getControl(cur).mouseDown(pos);
		default:
		}
	}
}
KeyboardTool.mouseUp(pos) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Form:
		case Keyboard: this.getControl(cur).mouseUp(pos);
		default:
		}
	}
}

Keyboard.Keyboard(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
	automata=Cf.automata();
}
Keyboard.initControl() {
	cf.inject(imagePath);
	tag[bg]="$imagePath/tool/keyboard/keyborad_bg.png";
	img=imageLoad(tag,'bg');
	img.imageSize().inject(w,h);
	tag[Width]=w, tag[Height]=h;
	setNodeSize(tag);
	this.makeKeyMap('EngLower');
	this.makeKeyMap('EngUpper');
	this.makeKeyMap('KorLower');
	this.makeKeyMap('KorUpper');
	this.makeKeyMap('NumOper');
	this.langMode='kor';
	tag[keyMap]='KorLower';
}
Keyboard.conf() {
	ox=8, oy=8, bw=67, bh=60, gabX=6, gabY=7;
	tag[rect].inject(sx, sy);
	sy+=oy;
	while( n, 4 ) {
		switch(n) {
		case 0:
			cx=sx+ox;
			while( c, 10 ) {
				tag[rc $n $c]=Class.rect(cx,sy,bw,bh), cx+=bw+gabX;
			}
			c+=1, w=876-cx;
			if( w<60 ) w=110;
			tag[rc $n $c]=Class.rect(cx,sy,w,bh);
			sy+=bh+gabY;
		case 1:
			cx=sx+24;
			while( c, 10 ) {
				tag[rc $n $c] = Class.rect(cx,sy,bw,bh), cx+=bw+gabX;
			}
			c+=1, w=876-cx;
			if( w<60 ) w=110;
			tag[rc $n $c]=Class.rect(cx,sy,w,bh);
			sy+=bh+gabY;
		default:
			cx=sx+ox;
			while( c, 12 ) {
				tag[rc $n $c] = Class.rect(cx,sy,bw,bh), cx+=bw+gabX;
			}
			sy+=bh+gabY;
			if( n.eq(3) ) {
				r1=tag[rc $n 4], r2=tag[rc $n 7];
				tag[rcSpace]=mergeRect(r1,r2);
			}
		}
	}
}
Keyboard.draw(draw, tm) {
	kid=this[keyId];
	not( kid ) {
		kid=this[langMode].upper(1);
		if( this.flag("keyMode", KM.Shift) ) {
			kid.add("Upper");
		} else {
			kid.add("Lower");
		}
	}
	keyMap=this[key#$kid];
	draw.fill(tag[rect],'#1a1a1a');
	draw.font(14,'normal','#fafafa');
	keyText=func() {
		key=keyMap[k $n $c];
		switch(key) {
		case comma:	draw.text(rc,',' ,'center');
		case dot:			draw.text(rc, '.','center');
		case bs:
			img=commonImage('icon_bs');
			draw.drawImage( img.center(rc), img);
		case enter:
			img=commonImage('icon_enter');
			draw.drawImage( img.center(rc), img);
		case etc:
			img=commonImage('icon_etc');
			draw.drawImage( img.center(rc), img);
		default:
			draw.text(rc, key, 'center');
		}
	};
	while( n, 4 ) {
		switch(n) {
		case 0:
			while( c, 11 ) {
				rc=tag[rc $n $c];
				if( rc.eq(this.mouseDownRect) ) {
					rc.incr(-4);
					draw.fill(rc,'#80808a').rectLine(rc, 0, '#20202a');
				} else {
					draw.fill(rc,'#333333').rectLine(rc, 0, '#20202a');
				}
				keyText();
			}
		case 1:
			while( c, 11 ) {
				rc=tag[rc $n $c];
				if( rc.eq(this.mouseDownRect) ) {
					rc.incr(-2);
					draw.fill(rc,'#80808a').rectLine(rc, 0, '#20202a');
				} else {
					draw.fill(rc,'#333333').rectLine(rc, 0, '#20202a');
				}
				keyText();
			}
		case 2:
			while( c, 12 ) {
				rc=tag[rc $n $c];
				if( c.eq(0,11) ) {
					if( rc.eq(this.mouseDownRect) ) {
						draw.fill(rc,'#7a7070').rectLine(rc, 0, '#20202a');
					} else if( this.flag("keyMode",KM.ShiftOn) ) {
						draw.fill(rc,'#c06a7b').rectLine(rc, 0, '#d09ab0', 2);
					} else if( this.flag("keyMode",KM.Shift) ) {
						draw.fill(rc,'#9a9090').rectLine(rc, 0, '#20202a');
					} else {
							draw.fill(rc,'#4d4d4d').rectLine(rc, 0, '#20202a');
					}
					keyText();
				} else {
					if( rc.eq(this.mouseDownRect) ) {
						rc.incr(-4);
						draw.fill(rc,'#80808a').rectLine(rc, 0, '#20202a');
					} else {
						draw.fill(rc,'#333333').rectLine(rc, 0, '#20202a');
					}
					keyText();
				}
			}
		case 3:
			while( c, 12 ) {
				if( c.eq(4,5,6,7) ) {
					not( c.eq(4) ) continue;
					rc=tag[rcSpace];
					if( rc.eq(this.mouseDownRect) ) {
						rc.incr(-2);
						draw.fill(rc,'#80808a').rectLine(rc, 0, '#20202a');
					} else {
						draw.fill(rc,'#333333').rectLine(rc, 0, '#20202a');
					}
				} else {
					rc=tag[rc $n $c], def=true;
					if( c.eq(1) ) {
						 if( this.flag("keyMode",KM.Ctrl) ) {
							draw.fill(rc,'#9a9090').rectLine(rc, 0, '#20202a');
						} else {
							draw.fill(rc,'#4d4d4d').rectLine(rc, 0, '#20202a');
						}
						def=false;
					} else if( c.eq(8) ) {
						if( this[langMode].eq('kor') ) {
							draw.fill(rc,'#707a7a').rectLine(rc, 0, '#40404a');
							def=false;
						}
					}
					if( def ) {
						if( rc.eq(this.mouseDownRect) ) {
							draw.fill(rc,'#7a7070').rectLine(rc, 0, '#20202a');
						} else {
							draw.fill(rc,'#4d4d4d').rectLine(rc, 0, '#20202a');
						}
					}
					keyText();
				}
			}
		default:
		}
	}
}
Keyboard.mouseDown(pos) {
	update=func() {
		this.mouseDownRect=rc;
		this.update();
	};
	while( n, 4 ) {
		switch(n) {
		case 0:
			while( c, 11 ) {
				rc=tag[rc $n $c];
				not( rc.contains(pos) ) continue;
				return update();
			}
		case 1:
			while( c, 11 ) {
				rc=tag[rc $n $c];
				not( rc.contains(pos) ) continue;
				return update();
			}
		case 2:
			while( c, 12 ) {
				rc=tag[rc $n $c];
				not( rc.contains(pos) ) continue;
				return update();
			}
		case 3:
			while( c, 12 ) {
				if( c.eq(4,5,6,7) ) {
					not( c.eq(4) ) continue;
					rc=tag[rcSpace];
				} else {
					rc=tag[rc $n $c];
				}
				not( rc.contains(pos) ) continue;
				return update();
			}
		default:
		}
	}
}
Keyboard.mouseUp(pos) {
	update=func(rc) {
		if( this.mouseDownRect ) {
			this.mouseDownRect=null;
			this.update();
		}
		if( rc ) {
			this.keyDown(n,c);
		}
	};
	while( n, 4 ) {
		switch(n) {
		case 0:
			while( c, 11 ) {
				rc=tag[rc $n $c];
				not( rc.contains(pos) ) continue;
				return update(rc);
			}
		case 1:
			while( c, 11 ) {
				rc=tag[rc $n $c];
				not( rc.contains(pos) ) continue;
				return update(rc);
			}
		case 2:
			while( c, 12 ) {
				rc=tag[rc $n $c];
				not( rc.contains(pos) ) continue;
				return update(rc);
			}
		case 3:
			while( c, 12 ) {
				if( c.eq(4,5,6,7) ) {
					not( c.eq(4) ) continue;
					rc=tag[rcSpace];
				} else {
					rc=tag[rc $n $c];
				}
				not( rc.contains(pos) ) continue;
				return update(rc);
			}
		default:
		}
	}
	update();
}
Keyboard.makeKeyMap(code) {
	s=conf("data.key#$code").str();
	keys=_node(this,"key#$code", true);
	while( s.valid(), n, 0 ) {
		line=s.findPos("\n");
		while( line.valid(), c, 0 ) {
			keys[k $n $c]=line.findPos(",").trim();
		}
	}
}
Keyboard.keyDown(n,c) {
	this.inject(keyMode, langMode );
	kid=this[keyId];
	not( kid ) {
		kid=when( this.flag('keyMode',KM.Shift), 'EngUpper', 'EngLower');
	}
	keyMap=this[key#$kid];
	addMode=when( this[leftString], 'leftString','doneString');
	_value=func(val ) {
		switch( val ) {
		case enter: 		val="\n";
		case tab: 		val="\t";
		case dot:			val='.';
		case comma:	val=',';
		}
		if( langMode.eq('kor') ) {
			if( val.isAlpha() ) {
				this[ingString].add(val);
				automata.toString(this, addMode);
			} else {
				automata.toString(this, addMode);
				this[doneString].add(this[doingString]);
				this.clear();
				this[$addMode].add(val);
			}
		} else {
			automata.toString(this, addMode);
			this[$addMode].add(val);
		}
		this.setInputText(addMode);
		if( this.flag('keyMode',KM.Shift) ) {
			not( this.flag('keyMode',KM.ShiftOn) ) {
				this.flag('keyMode',KM.Shift, false);
			}
		}
		this.flag('keyMode',KM.Ctrl, false);
	};
	ing=this[ingString];
	switch(n ) {
	case 0:
		if( c.eq(10) ) {
			if( ing ) {
				if( ing.size().eq(1) ) {
					this.clear();
				} else {
					this[ingString]=ing.value(0,-1);
					automata.toString(this, addMode);
				}
			} else {
				if( this.flag('keyMode',KM.Ctrl) ) {
					this[$addMode]='';
					this.flag('keyMode',KM.Ctrl, false);
				} else {
					str=this[$addMode];
					this[$addMode]=str.substr(0,-1);
				}
			}
			this.setInputText(addMode);
		} else {
			_value( keyMap[k $n $c] );
		}
	case 1:
		if( c.eq(10) ) {
			_value("\n");
		} else {
			_value( keyMap[k $n $c] );
		}
	case 2:
		if( c.eq(0,11) ) {
			if( this.flag('keyMode',KM.ShiftOn) ) {
				this.flag('keyMode',KM.Shift, false);
				this.flag('keyMode',KM.ShiftOn, false);
			} else if( this.flag('keyMode',KM.Shift) ) {
				this.flag('keyMode',KM.ShiftOn, true);
			} else {
				this.flag('keyMode',KM.Shift, true);
			}
		} else {
			_value( keyMap[k $n $c] );
		}
	case 3:
		switch( c) {
		case 0:
			if( this[keyId].eq('NumOper') ) {
				this[keyId]=null;
			} else {
				this[keyId]='NumOper';
			}
		case 1:
			flag=when( this.flag('keyMode',KM.Ctrl), false, true);
			this.flag('keyMode',KM.Ctrl, flag);
		case 3:	_value( "\t" );
		case 4:	_value( " " );
		case 8:
			if( this[langMode].eq('kor') ) {
				this[langMode]='eng';
			} else {
				this[langMode]='kor';
			}
		case 11:
			this[$addMode]='';
			this.setInputText(addMode);
		default:
			_value( keyMap[k $n $c] );
		}
	default:
	}
	this.update();
}
Keyboard.setString(str, type, pos) {
	len=str.length(), ok=false;
	if( isset(pos) ) {
		if( pos<len ) {
			ok=true;
		}
	}
	this.clear();
	if( type ) {
		this.flag('keyMode',KM.Shift, false);
		this.flag('keyMode',KM.ShiftOn, false);
		this[keyId]=null;
		switch( type ) {
		case num:
			this[keyId]='NumOper';
		case upper:
			this[langMode]='eng';
			this.flag('keyMode',KM.Shift, true);
			this.flag('keyMode',KM.ShiftOn, true);
		default:
			this[langMode]='eng';
		}
	}
	if( ok ) {
		this[leftString]=str.substr(0,pos), this[rightString]=str.substr(pos);
		this[doneString]='';
	} else {
		this[leftString]='', this[rightString]='';
		this[doneString]=str;
	}
}

AdminMenuCanvas.setInputNode(node) {
	cf[inputNode]=node;
	cf[inputFocusDraw]=true;
	cf[cursorIndex]=0;
}

Keyboard.setInputText(addMode) {
	input = cf[inputNode];
	not( input ) {
		this.clear();
	}
	str='';
	str.add(this[$addMode], this[doingString]);
	if( addMode.eq('leftString') ) {
		str.add(this[rightString]);
	}
	print(ing, this[doingString] );
	input[text]=str;
}
Keyboard.clear() {
	automata.clear();
	this[doingString]='';
	this[ingString]='';
}

MainButtons.cardButtonClick() {
	orderList=this.findControl('MenuCart#orderView').getOrderList();
	if( orderList.childCount() ) {
		if( cf[cardButtonCheck] ) {
			return;
		}
		cf[cardButtonCheck]=true;
		System.timout(100);  /* 2017.03.08 송호성 추가 */
		db=Class.db('kiosk_hitec');
		corner	= cf[CornerInfo];
		tabCount=this.findTag('#CornerTab').childCount();
		not( tabCount.eq(corner.childCount()) ) {
			db.fetchAll(conf("sql#hitec.CornerInfo"), corner.removeAll() );
			cc=corner.childCount();
			print("코너정보 새로고침 코너수 : $tabCount == $cc");
		}
		not( cf[orderItemList] ) {
			cf[orderItemList]=orderList;
		}
		err=null;
		while( cur, corner ) {
			cur[itemCnt]=0;
		}
		total=0, qty=0;
		while( item, orderList ) {
			cur=corner.findOne('clplu_cd', item[corner_cd]);
			cur[itemCnt++];
			not( item[sale_price] ) {
				print("메뉴 주문 금액이 일치하지 않습니다 item=>$item");
				return this.orderCancel("메뉴 주문 금액이 일치하지 않습니다","알림");
			}
			not( item[qty] ) {
				print("메뉴 주문 수량이 일치하지 않습니다 item=>$item");
				return this.orderCancel("메뉴 주문 수량이 일치하지 않습니다","알림");
			}
			total+=item[sale_price*qty];
			qty+=item[qty];
		}
		cart = this.findControl('MenuCart#orderView');
		not( total.eq(cart[OrderTotalPrice]) ) {
			print("주문 금액이 일치하지 않습니다 주문금액: total=>$total 결제금액: $cart[OrderTotalPrice]");
			return this.orderCancel("주문 금액이 일치하지 않습니다","알림");
		}
		not( qty.eq(cart[OrderTotalQty]) ) {
			print("주문 금액이 일치하지 않습니다 주문수량: qty=>$total 결제수량: $cart[OrderTotalQty]");
			return this.orderCancel("주문 수량이 일치하지 않습니다","알림");
		}
		while( cur, corner ) {
			not( cur[screen_socket] ) {
				continue;
			}
			not( cur[itemCnt] ) {
				continue;
			}
			socket=cur[screen_socket];
			socket[connected]=true;
			if( socket.isConnect() ) {
				cur[data]='';
				socket.send("IsSvrAlive");
				System.timeout(100);
				while(n,4) {
					if( cur[data] ) {
						break;
					}
					System.sleep(800);
				}
				not( cur[data] ) {
					err="$cur[clplu_nm] 주문 스크린 응답정보가 없습니다. 관리자에게 문의하세요";
				}
			} else {
				err="$cur[clplu_nm] 주문 스크린에 연결할수 없습니다. 관리자에게 문의하세요";
			}
			if( err ) break;
		}
		cf[cardButtonCheck]=false;
		if( err ) {
			main=this.mainControl();
			kiosk_SendError(main, err, '08', cf, db);
			main.alert(err,"오류",true);
			return;
		}
		cf[OrderHeader.PayType]='Card';
		this.mainControl().popupOpen('OrderConfirm');
	} else {
		lang=Cf[KioskLangSelect].lower();
		if( lang.eq('kor') ) {
			lang_0="주문 정보가 없습니다. 주문정보를 확인하세요", lang_1='알림';
		} else {
			lang_0='Please check the order list', lang_1='Info.';
		}
		this.mainControl().alert(lang_0, lang_1);
		this.mouseDownRect=null;
	}
}

mainCanvas.easyCardReadData(&data, node) {
	node[SUC]=null;
	node[RS04]=null;
	data.findPos('{',1,1);
	str=data.utf8();
	print("easyCardReadData: $str");
	node.parseJson(str);
	not( node[SUC] ) {
		print("easyCardReadData : parseJson 오류 -> $data");
		node.parseJson(data);
	}
	print("easyCardReadData: $node");
	if( node[SUC].eq('00') ) {
		/* 카드승인 성공*/
		if( node[RS04].eq("0000") ) {
			System.playWave(getLocalPath('/data/wave/b.wav'));
			order		= cf[OrderHeader];
			posInfo	= cf[PosInfo];
			cart		= this.findControl('MenuCart#orderView');
			order_completeCardProcess(this, node, order, cart, posInfo);
		} else {
			cf[orderStartTick]=0;
			if( cf[CashReceiptType] ) {
				msg="Easy Check 승인중";
			} else {
				msg="카드 승인중";
			}
			this.alert("$msg 오류가 발생했습니다.\n오류코드: $node[RS04]\n오류내용: $node[RS16]\n$node[RS17]", "결제오류");
			/* 결제하기 버튼 초기화 */
			ctrl=this.findControl('MainStatus#buttonsView');
			ctrl[mouseDownRect]=null;
		}
	} else {
		cf[orderStartTick]=0;
		this.alert("카드승인 결제 오류가 발생했습니다.\n오류: $node[MSG]", "결제오류");
		/* 결제하기 버튼 초기화 */
		ctrl=this.findControl('MainStatus#buttonsView');
		ctrl[mouseDownRect]=null;
	}
}
mainCanvas.easyCardError(&data, node) {
	cf[orderStartTick]=0;
	this.alert("Easy Check 인증중 오류가 발생했습니다.\n오류: $data", "결제오류");
	/* 결제하기 버튼 초기화 */
	ctrl=this.findControl('MainStatus#buttonsView');
	ctrl[mouseDownRect]=null;
}
mainCanvas.hitecResponseOk(&data, node) {

}
mainCanvas.hitecResponseError(&data, node) {

}
mainCanvas.easyCardCall(reqCd) {
	req=_node("EasyCardNode");
	if( req[status] ) {
		dist=System.tick()-req[startTick];
		if( dist>6000 ) {
			log("## easyCardRequestCall: $req[status] 대기시간 초과. 연결을 초기화 합니다");
			req[status]=null;
		}
		return false;
	}
	req[url]=cf[easyCardUrl];
	moneyPaperType='';
	money			='';
	depth 			='';
	cancelDt		='';
	cancelNo		='';
	productCd		='';
	productCd		='';
	saleNo			='';
	webMsg		='';
	keyInYn 		="Y";
	termNo 			='';
	vat 				='';
	timeout 			=30;
	addField		='';
	receiveHandle	='';
	termType		='';
	disType			='';
	option			='';
	extOption		='';
	switch( reqCd ) {
	case D1:
		args(1, money, depth, saleNo, webMsg, vat );
		not( depth ) 		depth='00';
		not( saleNo ) 	saleNo = System.localtime();
		not( webMsg )	webMsg ="web${saleNo}";
		not( keyInYn )	keyInYn ='Y';
		not( vat )			vat= "A";
	case B1:
		args(1, moneyPaperType, money);
		not( moneyPaperType ) moneyPaperType='00';
	}
	value="$reqCd^$moneyPaperType^$money^$depth^$cancelDt^$cancelNo^$productCd^$saleNo^$webMsg^$keyInYn^$termNo^$timeout^$vat^$addField^$receiveHandle^$termType^$disType^$option^$extOption";
	_log("easyCardRequestCall: $func, $value");
	param=_node( req,'param').initNode();
	param[REQ]=value;
	param[callback]="result_${tick}";
	req[status]		='start';
	req[startTick]		=System.tick();
	cf[orderStartTick]=System.tick();
	page.easyCardSend(req);
}

SelectCard.SelectCard(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SelectCard.initControl() {
	cf.inject(imagePath);
	tag[Width]=936, tag[Height]=764;
	tag[BackgroundImage]="${imagePath}/Common/kr/pop_card01_bg.png";
	cur=tag.addNode({tag: CardImage, Margin:[32,164,0,0], Width:319, Height:410, class:layer});
	cur[src]="${imagePath}/Common/kr/iccard.png";
	tag.addNode({tag: Price, Margin:[610,270,90,0], Width:329, Height:80, class:layer});
	tag.addNode({tag: Message, Margin:[0,569,0,107]});
	cur=tag.addNode({tag: CancelButton, Margin:[296,644,0,0], class:layer, Width:339, Height:89});
	cur[src]="${imagePath}/Common/kr/pop_cancel_[#].png";
	setNodeSize(tag, true);
}
SelectCard.conf() {
	offset=tag[rect].lt();
	confNodeLayout(tag, offset);
	order=this.findControl('MenuCart#orderView');
	this[totalPrice]=order[OrderTotalPrice];
}
SelectCard.draw(draw, tm) {
	draw.mode();
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag] ) {
		case CardImage:
			draw.drawImage( cur[rect], imageLoad(cur,'src') );
			draw.rectLine( cur[rect], 0, '#c0c0c0');
		case Price:
			price=util_priceComma(this[totalPrice]);
			drawNodeText(draw, cur[rect], price, 'right', 'SubTitle');
		case Message:
			drawNodeText(draw, cur[rect], cur[text], 'center', 'OrderInfo');
		case CancelButton:
			ty=when( cur[rect].eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( cur[rect], imageLoad(cur,'src',ty) );
		default:
		}
	}
}
SelectCard.mouseDown(pos) {
	btn = this.findTag('CancelButton');
	if( btn[rect].contains(pos) ) {
		this.mouseDownRect=btn[rect];
		this.update();
	}
}
SelectCard.mouseUp(pos) {
	btn= this.findTag('CancelButton');
	if( btn[rect].contains(pos) ) {
		if( btn[rect].eq(this.mouseDownRect) ) {
			this.mainControl().popupClose();
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

GridControl.GridControl(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
GridControl.initControl() {
	setNodeSize(tag, true);
		tag.addNode({tag: Header, Height:65});
		tag.addNode({tag: Body });
		tag.addNode({tag: Status, Height:85});
		this.listNum=16;
		this.listHeight=40;
		this.startRow=0;
}
GridControl.conf() {
	while( cur, tagRect(tag, true) ) {
		switch( cur[tag] ) {
		case Header:
			cur[rect].inject(sx, sy, sw, sh );
			makeHeaderWidth(cur, sw);
			header=cur;
			cx=sx, ch=header[Height];
			while( sub, header, n, 0 ) {
					cw=sub[width];
					sub[rect]=Class.rect( cx, sy, cw, ch), cx+=cw;
				}
			case Body:
				hh=cur[rect].height();
				hh/=this.listHeight;
				this.listNum=hh;
				this.confBody(cur);
			case Status:
				rectRateArray(cur[rect],'140, 0, 130, *, 200, 55, 8, 55,10').inject(
					rcApply, space, rcAdd, space, rcStatus, rcUp, space, rcDown
				);
				cur.put(rcApply, space, rcAdd, space, rcStatus, rcUp, space, rcDown);
		default:
		}
	}
}
GridControl.confBody(body) {
	header=this.findTag('Header');
	not( body ) body=this.findTag('Body');
	sp=this.startRow, ep=sp+this.listNum, ch=this.listHeight;
	root=this.dataNode;
	body[rect].inject(sx, sy, sw, sh );
	while( n, ep, sp ) {
		record=root.child(n);
		not( record ) break;
		cx=sx;
		record[rect]=Class.rect( cx, sy, sw, ch);
		while( sub, header, col, 0 ) {
			cw=sub[width];
			record[rect $col]=Class.rect( cx, sy, cw, ch), cx+=cw;
		}
		sy+=ch;
	}
}
GridControl.draw(draw, tm) {
	sp=this.startRow, ep=sp+this.listNum, root=this.dataNode;
	funcDraw=parentCtrl.drawGrid;
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Header:
			header=cur;
			draw.font(15, 'bold', '#f0f0f0');
			while( sub, header, col, 0 ) {
				rc=sub[rect];
				draw.fill( rc , '#424243');
				draw.text(rc, sub[text], 'center');
				if( col )
					draw.rectLine( rc, 234, '#606062', 4);
				else
					draw.rectLine( rc, 0, '#606062', 4);
			}
		case Body:
			draw.rectLine( cur[rect], 13, '#606062', 4);
			draw.font(14, 'normal', '#f0f0f0');
			/******* 데이터 존재하지 않을 때 *****
			not( root.childCount() ) {
				rc=cur[rect].height(60);
				draw.fill(rc, '#4f4f50').rectLine( rc, 134, '#606062', 4);
				draw.text(rc, "조회된 결과가 없습니다", "center");
				continue;
			}
			*******/
			while( n, ep, sp ) {
				record=root.child(n);
				not( record ) {
					break;
				}
				while( sub, header, col, 0 ) {
					rc=record[rect $col], field=sub[code];
					draw.fill( rc, '#4f4f50');
					if( typeof(funcDraw,'function') ) {
						funcDraw(draw, rc, record, field);
					} else {
						draw.text( rc.incrX(5), record[$field], "center");
					}
					if( col )
						draw.rectLine( rc, 34, '#606062', 4);
					else
						draw.rectLine( rc, 134, '#606062', 4);
					if( record[@$field] ) {
						this.modifyMark(draw, rc);
					}
				}
			}
		case Status:
			draw.fill( cur[rect], '#606062');
			draw.rectLine( cur[rect], 134, '#606062', 4);
			/* 적용버튼 */
			rcBtn=cur[rcApply].center(123, 65);
			ty=when( rcBtn.eq(this.mouseDownRect), 'p', 'n');
			img=commonImage('btn_bg', ty);
			draw.drawImage(rcBtn, img );
			draw.font(16,'normal','#f0f0f0').text(rcBtn, "적용", "center");
			/* 추가버튼(영동고속도로 버전은 삭제해야함 */
			if(cf[currentPage].eq('KioskInfoPage')) {
				if(cf[noSetupType].eq('2')) {
					addBtn=cur[rcAdd].center(123, 65);
					ty=when( addBtn.eq(this.mouseDownRect), 'p', 'n');
					img=commonImage('btn_bg', ty);
					draw.drawImage(addBtn, img );
					draw.font(16,'normal','#f0f0f0').text(addBtn, "추가", "center");
				}
			}
			/* status */
			idx=sp+1, total=root.childCount();
			draw.font(14, 'normal', '#f0f0f0');
			draw.text( cur[rcStatus].incr(10), "$idx/$total" );
			/* navi */
			/* scroll up */
			if( sp>0 ) {
				var=when( this.mouseDownRect.eq(cur[rcUp]), 'p','n');
			} else {
				var='d';
			}
			imgUp=commonImage('btn_up',var);
			/* scroll down */
			if( ep<total ) {
				var=when( this.mouseDownRect.eq(cur[rcDown]), 'p','n');
			} else {
				var='d';
			}
			imgDown=commonImage('btn_down',var);
			draw.drawImage( imgUp.center(cur[rcUp]), imgUp);
			draw.drawImage( imgDown.center(cur[rcDown]), imgDown);
		default:
		}
	}
	if( this.tooltipRect ) {
		rc=this.tooltipRect, record=this.currentRecord, field=this.currentField;
		text=record[$field];
		draw.fill(rc,'#4f4f50').rectLine(rc.incrY(-2), 0, '#606062', 2);
		draw.font(14,'normal','#eaeaf0').text(rc.incrX(5), text);
	}
}
GridControl.mouseDown(pos) {
	this.mouseDownRect=null;
	header=null;
	while( cur, tag ) {
		switch( cur[tag] ) {
			case Header:
				header=cur;
			case Body:
			sp=this.startRow, ep=sp+this.listNum, root=this.dataNode;
			while( row, ep, sp ) {
				record=root.child(row);
				not( record ) {
					break;
				}
				if( record[rect].contains(pos) ) {
					while( sub, header, col, 0 ) {
						if( record[rect $col].contains(pos) ) {
							this.gridClick(record, sub[code], record[rect $col] );
						}
					}
					this.update();
					return;
				}
			}
		case Status:
			rcBtn=cur[rcApply].center(123, 65);
			addBtn=cur[rcAdd].center(123, 65);
			if( rcBtn.contains(pos) ) {
				this.mouseDownRect=rcBtn;
				this.applyClick();
			} else if( addBtn.contains(pos) ) {
				/*  영동고속도록 버전은 삭제해야 함 */
				if(cf[noSetupType].eq('2')) {
					this.mouseDownRect=addBtn;
					this.addClick();
				}
			} else if( cur[rcUp].contains(pos) ) {
				this.mouseDownRect=cur[rcUp];
				this.goPageUpDown('up');
			} else if( cur[rcDown].contains(pos) ) {
				this.mouseDownRect=cur[rcDown];
				this.goPageUpDown('down');
			}
		default:
		}
		if( this.mouseDownRect ) {
			this.update();
			return;
		}
	}
	if( this.tooltipRect ) {
		this.tooltipRect=null;
		this.update();
	}
}
GridControl.mouseUp(pos) {
	if( this.mouseDownRect  ) {
		this.mouseDownRect=null;
		this.update();
	}
}
GridControl.setModel(root, fields) {
	_header=func(node, &str) {
		node.removeAll();
		while( str.valid() ) {
			line=str.findPos(',');
			not( line.ch() ) continue;
			cur = node.addNode({tag:field});
			cur[code]=line.findPos(':').trim();
			cur[text]=line.findPos('#').trim();
			cur[width]=line.trim();
		}
	};
	_header( this.findTag('Header'), fields.ref() );
	this.startRow=0;
	this.dataNode=root;
	this.conf();
	this.update();
}
GridControl.goPageUpDown(type) {
	sp=this.startRow, ep=sp+this.listNum, root=this.dataNode;
	total=root.childCount();
	switch(type) {
	case up:
		if( sp>0 ) {
			sp-=this.listNum;
			if( sp<0 ) sp=0;
			this.startRow=sp;
		} else {
			return;
		}
	case down:
		if( ep< total ) {
			this.startRow=ep;
		} else {
			return;
		}
	}
	this.confBody();
	this.update();
}

AdminMenuCanvas.test() {
	this.findControl('Popup#dialog').popupClose();
	this.findControl('#Content').pageLoad("GoodsInfoPage","admin");
}

RestInfo.RestInfo(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
RestInfo.initControl() {
	tag.addNode({tag: Title, Height:92});
	tag.addNode({tag: Form, Height:110});
	tag.addNode({tag: GridControl, ClassPath:common, Margin:[15,5]});
	tag.addNode({tag: Footer, Height:52});
	setNodeSize(tag, true);
}
RestInfo.conf() {
	confNodeLayout(tag);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
		case Form:
		case GridControl:
			this.getControl(cur).conf();
		default:
		}
	}
}
RestInfo.draw(draw, tm) {
	draw.drawImage( tag[rect], commonImage('admin_bg'), 'fill');
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			draw.drawImage(cur[rect], commonImage('admin_title'),0,0 );
		case Form:
		case GridControl: this.getControl(cur).draw(draw, tm);
		default:
		}
	}
}
RestInfo.mouseDown(pos) {
	this.findControl('GridControl').mouseDown(pos);
}
RestInfo.mouseUp(pos) {
	this.findControl('GridControl').mouseUp(pos);
}
RestInfo.search() {
	model=Class.model('RestInfo');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	db.fetchAll( conf("sql#kiosk.hitec#RestInfo"), root.removeAll() );
	this.findControl('GridControl').setModel(root, "
		rest_nm:휴게소#30,
		ms_nm:지점명#30,
		master_nm:업주명#25,
		biz_no:사업자번호#35,
		bill_addr:주소#65,
		tel_no:전화번호#35,
		fin_time:종료시간#30
	");
}

GridControl.gridClick(record, field, rect) {
	if( this.tooltipRect ) {
		this.tooltipRect=null;
	}
	this.currentRecord=record;
	this.currentField=field;
	tw=textWidth(14, record[$field]), rw=rect.width()-5;
	if( tw > rw ) {
		dist=tw-rw;
		this.tooltipRect=rect.incrW(dist);
		this.update();
	}
	parentCtrl.gridClick(record, field);
}

RestInfoPage.RestInfoPage(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
RestInfoPage.conf() {
	confNodeLayout(tag);
	while( cur, tag ) {
		switch( cur[tag] ) {
		case GridControl:
			this.getControl(cur).conf();
		default:
		}
	}
}
RestInfoPage.draw(draw, tm) {
	draw.drawImage( tag[rect], commonImage('admin_bg'), 'fill');
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			draw.drawImage(cur[rect], commonImage('admin_title'),0,0 );
			draw.font(28,'bold','#fd9437').text(cur[rect].incrX(25), "휴게소 설정");
		case GridControl:
			not( this.dataModel ) {
				this.search();
			}
			this.getControl(cur).draw(draw, tm);
		default:
		}
	}
}
RestInfoPage.initControl() {
	tag.addNode({tag: Title, Height:92});
	tag.addNode({tag: GridControl, ClassPath:common, Margin:[15,25]});
	setNodeSize(tag, true);
}
RestInfoPage.mouseDown(pos) {
	this.findControl('GridControl').mouseDown(pos);
}
RestInfoPage.mouseUp(pos) {
	this.findControl('GridControl').mouseUp(pos);
}
RestInfoPage.search() {
	model=Class.model('RestInfo');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	db.fetchAll( conf("sql#hitec.RestInfo"), root.removeAll() );
	this.findControl('GridControl').setModel(root, "
		rest_nm:휴게소#30,
		ms_nm:지점명#30,
		master_nm:업주명#25,
		biz_no:사업자번호#35,
		bill_addr:주소#65,
		tel_no:전화번호#35,
		fin_time:종료시간#30
	");
	this.dataModel=model;
}

InputTextForm.InputTextForm(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
InputTextForm.initControl() {
	tag.removeAll();
	tag.addNode({tag: Title, Height:99});
	tag.addNode({tag: Body});
	tag.addNode({tag: Status, Height:40});
	body=this.findTag('Body');
	body.addNode({tag: Form});
	body.addNode({tag: Keyboard, ClassPath:common, Height:276});
}
InputTextForm.conf() {
	while( cur, tagRect(tag,true) ) {
		rectRateArray(cur[rect],'30,*,30').inject( rc0, rc1, rc2);
		cur.put( rc0, rc1, rc2);
	}
	body=this.findTag('Body');
	while( cur, tagRect(body) ) {
		switch(cur[tag]) {
		case Form:
		case Keyboard: this.getControl(cur).conf();
		}
	}
}
InputTextForm.draw(draw, timeline) {
	this.drawFadeIn(draw, timeline);
	while( cur, tag ) {
		switch(cur[tag]) {
		case Title:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rc$col], commonImage("popup_box1$idx") );
			}
			drawNodeText(draw, cur[rc1], tag[title], 'left', 32, '#f0f0f0');
		case Body:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rc$col], commonImage("popup_box2$idx") );
			}
		case Status:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rc$col], commonImage("popup_box3$idx") );
			}
		}
	}
	while( cur, this.findTag('Body') ) {
		switch(cur[tag]) {
		case Form:
			input=cur.child(0);
			draw.font(14,'bold', '#80808a').text( input[rectLabel], "$input[label] : " );
			rc=input[rect].incr(-4);
			draw.rectLine(rc, 0, '#30303a');
			draw.font(14,'bold', '#40404a').text( input[rect], input[text] );
			rc=input[rcApply];
			ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage(rc, commonImage('btn_bg', ty) );
			draw.font(16,'normai','#f0f0f0').text(rc, "적용", "center" );
		case Keyboard: this.getControl(cur).draw(draw, timeline);
		}
	}
}
InputTextForm.mouseDown(pos) {
	while( cur, this.findTag('Body') ) {
		switch(cur[tag]) {
		case Form:
			input=cur.child(0);
			if( input[rcApply].contains(pos) ) {
				this.mouseDownRect=input[rcApply];
				this.applyClick(input);
				this.update();
				return;
			}
		case Keyboard: this.getControl(cur).mouseDown(pos);
		}
	}
}
InputTextForm.mouseUp(pos) {
	while( cur, this.findTag('Body') ) {
		switch(cur[tag]) {
		case Form:
		case Keyboard: this.getControl(cur).mouseUp(pos);
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
InputTextForm.drawFadeIn(draw, timeline) {
	popupFadeIn(draw, timeline);
}

GridControl.applyClick() {
	parentCtrl.applyClick(this.dataNode, this );
}

RestInfoPage.gridClick(record, field) {
	target=tag[rect], ctrl=null;
	rc=Class.rect(0,0,890,560);
	switch(field ) {
	case biz_no:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '사업자번호', '사업자 변호 변경');
	case master_nm:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '업주명', '업주명 변경');
	case ms_nm:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '지점명', '지점명 변경');
	case tel_no:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '전화번호', '전화번호 변경');
	case fin_time:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '종료시간', '종료시간 변경');
	case bill_addr:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주소', '지점 주소변경');
	default:
	}
	if( ctrl ) {
		tag=ctrl[tag];
		tag[autoClose]=true;
	}
}

InputTextForm.setForm(record, field, label, title, type) {
	form=this.findTag('Form');
	rcForm=rectVCenter( form[rect].incr(10), 65, 10 );
	cur=form.child(0);
	not( cur ) {
		cur=form.addNode();
	}
	tag[title]=title;
	labelWidth= textWidth(14, "$label : ")+5;
	rectRateArray( rcForm, "$labelWidth,$400,15,123,*").inject(rcLabel, rcInput, space, rcApply);
	cur.put(record, field, label);
	cur[text]			= record[$field];
	cur[rect]			= rectVCenter(rcInput,30);
	cur[rectLabel]	= rectVCenter(rcLabel,30);
	cur[rcApply]		= rcApply;
	this.update();
	this.mainControl().setInputNode(cur);
	xx=this.findControl('Keyboard');
	this.findControl('Keyboard').setString(cur[text], type);
}
InputTextForm.applyClick(input) {
	rec=input[record], field=input[field];
	prev=rec[$field];
	not( prev.eq(input[text]) ) {
		rec[$field]=input[text];
		rec[@$field]=true;
		rec.state(NODE.modify, true);
	}
	this.findControl('Popup#dialog').popupClose();
}

GridControl.modifyMark(draw, rc) {
	arr=_arr();
	x=rc.right()-10, y=rc.y();
	sp=Class.point(x,y);
	arr.add(sp);
	arr.add(rc.rt()); y+=8;
	arr.add(Class.point(rc.right(),y));
	arr.add(sp);
	draw.polygon(arr,'fill','#c05060');
}

CornerInfoPage.CornerInfoPage(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
CornerInfoPage.conf() {
	while( cur, tagRect(tag) ) {
		switch( cur[tag] ) {
		case GridControl:
			this.getControl(cur).conf();
		default:
		}
	}
	not( this.dataModel ) {
		this.search();
	}
}
CornerInfoPage.draw(draw, tm) {
	draw.drawImage( tag[rect], commonImage('admin_bg'), 'fill');
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			draw.drawImage(cur[rect], commonImage('admin_title'),0,0 );
			draw.font(28,'bold','#fd9437').text(cur[rect].incrX(25), "코너정보 설정");
		case GridControl: this.getControl(cur).draw(draw, tm);
		default:
		}
	}
}
CornerInfoPage.initControl() {
	tag.addNode({tag: Title, Height:92});
	tag.addNode({tag: GridControl, ClassPath:common, Margin:[15,20]});
	tag.addNode({tag: Footer, Height:52});
	setNodeSize(tag, true);
}
CornerInfoPage.mouseDown(pos) {
	this.findControl('GridControl').mouseDown(pos);
}
CornerInfoPage.mouseUp(pos) {
	this.findControl('GridControl').mouseUp(pos);
}
CornerInfoPage.search() {
	model=Class.model('CornerInfo');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	db.fetchAll( conf("sql#hitec.CornerInfo"), root.removeAll() );
	this.findControl('GridControl').setModel(root, "
		clplu_nm: 			코너명			#55,
		img_file_nm: 		코너 이미지	#45,
		kitchen_ip:			주방IP		 	#40,
		kitchen_port:		주방포트		#30,
		order_did_ip:		주문IP			#40,
		order_did_port:	주문포트		#30,
		note:비고								#25,
	");
	this.dataModel=model;
}
CornerInfoPage.gridClick(record, field) {
	target=tag[rect], ctrl=null;
	rc=Class.rect(0,0,890,560);
	switch(field ) {
	case note:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '비고', '비고 설정');
	case kitchen_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터IP', '주방프린터 아이피 설정');
	case kitchen_port:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터 포트', '주방프린터 포트 설정');
	case order_did_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주문스크린 IP', '주문스크린 아이피 설정');
	case order_did_port:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주문스크린 포트', '주문스크린 포트 설정');
	default:
	}
	if( ctrl ) {
		tag=ctrl[tag];
		tag[autoClose]=true;
	}
}

RestInfoPage.applyClick() {

}

KioskInfoPage.KioskInfoPage(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
KioskInfoPage.initControl() {
	tag[Width]=1080, tag[Height]=1793;
	tag.addNode({tag: Title, Height:92});
	form=tag.addNode({tag: Form, Margin:[15,20], Height:450} );
	/* 폼설정 */
	row=form.addNode({tag: Row});
	row.addNode({tag:input, field:kiosk_id, label: 어드민 아이디, type:number} );
	row.addNode({tag:input, field:kiosk_pw, label: 비밀번호, type:number} );
	row=form.addNode({tag: Row});
	row.addNode({tag:input, field:ms_no, label: 매장그룹번호, type:upper} );
	row.addNode({tag:input, field:pos_no, label: 포스번호, type:number, width:80} );
	row=form.addNode({tag: Row});
	row.addNode({tag:input, field:emp_id, label: 사원 아이디, type:upper} );
	row.addNode({tag:input, field:emp_pw, label: 사원 비밀번호, type:upper} );
	row=form.addNode({tag: Row});
	row.addNode({tag:input, field:service_start_time, label: 서비스 시작시간, type:number, width:80, option:true} );
	row.addNode({tag:input, field:service_end_time, label: 서비스 종료시간, type:number, width:80, option:true} );
	row=form.addNode({tag: Row});
	row.addNode({tag:input, field:order_start_no, label: 주문 시작번호, type:number, width:80} );
	row.addNode({tag:input, field:order_end_no, label: 주문 끝번호, type:number, width:80} );
	row=form.addNode({tag: Row});
	row.addNode({tag:input, field:qt_mon_ip, label: QtMon 아이피, type:num} );
	row.addNode({tag:input, field:qt_mon_port, label: 포트, type:number, width:80} );
	row=form.addNode({tag: Row});
	row.addNode({tag:input, field:refresh_time, label: 갱신주기, type:number, width:100} );
	row.addNode({tag:check, field:data_reset_yn, label: 데이터초기화, width:40} );
	row=form.addNode({tag: Row, Height:120});
	row.addNode({tag:button, id:apply, text: 적용하기} );
	tag.addNode({tag: Chapter, Margin:[15,0], text:코너 설정, Height:50});
	tag.addNode({tag: GridControl, ClassPath:common, Margin:[25,5]} );
	tag.addNode({tag: Buttons, Margin:[15,20], Height:260});
}
KioskInfoPage.conf() {
	while( cur, tagRect(tag,true) ) {
		switch( cur[tag] ) {
		case Form:
			this.confForm(cur);
		case GridControl:
			this.getControl(cur).conf();
		case Buttons:
			this.confButton(cur);
		default:
		}
	}
}
KioskInfoPage.draw(draw, tm) {
	draw.drawImage( tag[rect], commonImage('admin_bg'), 'fill');
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			draw.drawImage(cur[rect], commonImage('admin_title'),0,0 );
			draw.font(28,'bold','#fd9437').text(cur[rect].incrX(25), "키오스크 설정");
		case Form:
			this.drawForm(draw, cur);
		case Chapter:
			if(cf[noSetupType].eq('1')) {
				draw.font(18,'bold','#fd9437').text(cur[rect].incrX(25), "* 매장 코너 아이피 설정");
			} else if(cf[noSetupType].eq('2')) {
				draw.font(18,'bold','#fd9437').text(cur[rect].incrX(25), "* 프린트 번호 및 아이피 설정");
			}
		case GridControl:
			this.getControl(cur).draw(draw, tm);
		default:
		}
	}
}
KioskInfoPage.confForm(form) {
	maxWidth=func(c) {
		mw=0;
		while( row, form ) {
			cell=row.child(c);
			not( cell[label] ) continue;
			tw=textWidth(14, cell[label],'bold') + 15;
			if( mw<tw ) mw=tw;
		}
		return mw;
	};
	m0=maxWidth(0), m1=maxWidth(1);
	print( m0, m1 );
	form[rect].inject(sx, sy, sw, sh);
	ch=40;
	while( row, form ) {
		cx=sx;
		rowHeight=nvl( row[Height],ch);
		row[rect]=Class.rect(cx, sy, sw, rowHeight);
		rectRateArray(row[rect],"$m0,250,50,$m1,$250,*").inject(label0, input0, space, label1, input1);
		row.put(label0, input0, label1, input1);
		while( cell, row, c, 0 ) {
			rc=row[input$c].incr(4);
			if( cell[width] ) {
				rc.width(cell[width]);
			}
			cell[rcLabel]=row[label$c], cell[rcInput]=rc;
			switch( cell[tag] ) {
			case button:
				cell[rect]=row[rect].width(130).center(123, 65);
			case check:
				cell[rect]=rc.center(35,35);
			}
		}
		sy+=rowHeight;
	}
	tag[formStart]=System.tick();
}
KioskInfoPage.confButton(cur) {

}
KioskInfoPage.mouseDown(pos) {
	dist=System.tick()-tag[formStart];
	if( dist<500 ) {
		return;
	}
	input=func(cell) {
		cell[rect]=cell[rcInput];
		rcOpen=null, keyboard=null;
		switch(cell[type] ) {
		case number:
			rcOpen=cell[rect].move('down').size(452, 536);
			this.findControl('#Content').popupOpen('NumberPad', this, rcOpen );
		default:
			rcOpen=cell[rect].move('down').size(895, 275);
			keyboard=this.findControl('#Content').popupOpen('Keyboard', this, rcOpen );
			this.getControl(keyboard).setString(cell[text], cell[type]);
		}
		if( rcOpen ) {
			print( cell, ctrl, rcOpen);
			this.mainControl().setInputNode(cell);
			this.getControl(keyboard).setString(cell[text]);
		}
	};
	form=this.findTag('Form');
	while( row, form ) {
		while( cell, row, c, 0 ) {
			switch( cell[tag] ) {
			case button:
				if( cell[rect].contains(pos) ) {
					this.mouseDownRect=cell[rect];
					this.applyKiosk();
					this.update();
				}
			case check:
				if( cell[rect].contains(pos) ) {
					cell.toggle('checked');
					this.update();
				}
			default:
				not( cell[rcInput].contains(pos) ) continue;
				this.currentInput=cell;
				input(cell);
			}
		}
	}
	this.findControl('GridControl').mouseDown(pos);
}
KioskInfoPage.mouseUp(pos) {
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
	this.findControl('GridControl').mouseUp(pos);
}
KioskInfoPage.drawForm(draw, form) {
	while( row, form ) {
		while( cell, row ) {
			rc=cell[rcInput];
			switch( cell[tag] ) {
			case button:
				rc=cell[rect], ty=when(rc.eq(this.mouseDownRect), 'p','n'), img=commonImage('btn_bg', ty);
				draw.drawImage(rc, img );
				draw.font(16,'bold','#f0f0f0').text(rc, "설정적용", "center");
			case check:
				rc=cell[rect], type=when( cell[checked], 'on', 'off'), img=commonImage("check_$type");
				draw.drawImage(rc, img);
			default:
				draw.font(14, 'bold','#40404a');
				draw.fill(rc,'#fafaf0').draw.rectLine( rc, 0, '#20202a');
				draw.text(rc, cell[text]);
				if( cell[field].eq('refresh_time') ) {
					draw.font(14, 'normal','#f0f0f0').text(rc.move('right',5), "초");
				}
			}
			if( cell[label] ) {
				draw.font(14, 'bold', '#f0f0fa').text(cell[rcLabel], "$cell[label] :");
			}
		}
	}
}


GoodsInfoPage.GoodsInfoPage(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
GoodsInfoPage.initControl() {
	tag[Width]=1080, tag[Height]=1793;
	tag.removeAll();
	tag.addNode({tag: Title, Height:92});
	/* 폼영역 */
	form=tag.addNode({tag: Form, Margin:[15,20], Height:60} );
	row=form.addNode({tag: Row});
	row.addNode({tag:combo, field:clplu_cd, label: 코너명});
	row.addNode({tag:combo, field:sold_out_yn, label: 품절여부});
	row.addNode({tag:button, field:search, label: 조회});
	tag.addNode({tag: GridControl, ClassPath:common, Margin:[15,20]});
	tag.addNode({tag: Buttons, Height:65});
}
GoodsInfoPage.conf() {
	while( cur, tagRect(tag,true) ) {
		switch( cur[tag] ) {
		case Form:
			this.confForm(cur);
		case GridControl:
			this.getControl(cur).conf();
		}
	}
	not( this.dataModel ) {
		this.search();
	}
	this.pageStartTick=System.tick();
}
GoodsInfoPage.draw(draw, tm) {
	draw.drawImage( tag[rect], commonImage('admin_bg'), 'fill');
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			draw.drawImage(cur[rect], commonImage('admin_title'),0,0 );
			draw.font(28,'bold','#fd9437').text(cur[rect].incrX(25), "상품정보 조회");
		case Form:
			this.drawForm(draw, cur);
		case GridControl:
			this.getControl(cur).draw(draw, tm);
		}
	}
}
GoodsInfoPage.mouseDown(pos) {
	dist=System.tick() - this.pageStartTick;
	if( dist<500 ) return;
	form=this.findTag('Form');
	_corner=func() {
		db=Class.db('kiosk_hitec');
		x=Cf[CommCodeNode];
		node=_node(x,'corner');
		node.removeAll();
		sub=node.addNode();
		sub[code]="";
		sub[value]="전체";
		db.fetchAll(conf("sql#cc.corner"), node);
		while( c, node ) print("c=====$c");
		return node;
	}
	while( row, form ) {
		while( cell, row, c, 0 ) {
			not( cell[rect].contains(pos) ) continue;
			switch( cell[field] ) {
			case clplu_cd:
				rcOpen=cell[rcInput].move('down').size(450, 350);
				node=this.findControl('#Content').popupOpen('CommCombo', this, rcOpen );
				this.getControl(node).setCommCode( _corner(),'코너선택', cell, 3, '전체');
				this.currentInpu=cell;
			case sold_out_yn:
				rcOpen=cell[rcInput].move('down').size(450, 350);
				node=this.findControl('#Content').popupOpen('CommCombo', this, rcOpen );
				this.getControl(node).setCommCode('kiosk#soldOut','품절여부', cell, 3, '전체');
				this.currentInpu=cell;
			case search:
				this.search(form);
			}
		}
	}
	this.findControl('GridControl').mouseDown(pos);
}
GoodsInfoPage.mouseUp(pos) {
	this.findControl('GridControl').mouseUp(pos);
}
GoodsInfoPage.confForm(form) {
	maxWidth=func(c) {
		mw=0;
		while( row, form ) {
			cell=row.child(c);
			tw=textWidth(14, cell[label],'bold') + 15;
			if( mw<tw ) mw=tw;
		}
		return mw;
	};
	m0=maxWidth(0), m1=maxWidth(1);
	print( m0, m1 );
	form[rect].inject(sx, sy, sw, sh);
	ch=40;
	while( row, form ) {
		cx=sx;
		row[rect]=Class.rect(cx, sy, sw, ch);
		rectRateArray(row[rect],"$m0,250,50,$m1,$90,*,130").inject(
			label0, input0, space, label1, input1,space, button
		);
		row.put(label0, input0, label1, input1);
		while( cell, row, c, 0 ) {
			switch( cell[tag] ) {
			case button:
				cell[rect]=button.center(123,65);
			default:
				rc=row[input$c].incr(4);
				if( cell[width] ) rc.width(cell[width]);
				cell[rcLabel]=row[label$c], cell[rcInput]=rc;
				cell[rect]=rc.incr(-2);
			}
		}
		sy+=ch;
	}
}
GoodsInfoPage.drawForm(draw, form) {
	while( row, form ) {
		while( cell, row ) {
			switch( cell[tag] ) {
			case button:
				draw.drawImage(cell[rect], commonImage('btn_bg') );
				draw.font(16,'bold','#f0f0f0').text(cell[rect], "조회", "center");
			default:
				draw.font(14, 'bold', '#f0f0fa').text(cell[rcLabel], "$cell[label] :");
				rc=cell[rect];
				draw.font(14, 'bold','#40404a');
				draw.fill(rc,'#eeeaea').draw.rectLine( rc, 0, '#20202a');
				draw.text(rc, cell[text]);
			}
		}
	}
}
GoodsInfoPage.search(form) {
	model=Class.model('GoodsInfo');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	if( form ) {
		root[clplu_cd]	= form[clplu_cd];
		root[sold_yn]		= form[sold_out_yn];
	}
	db.fetchAll( conf("sql#hitec.GoodsInfo"), root.removeAll() );
	this.findControl('GridControl').setModel(root, "
		clplu_cd: 		코너명		#40,
		goods_nm: 	상품명		#55,
		uprice:			가격		 	#30,
		goods_img:	이미지		#40,
		start_time:		판매시작	#30,
		end_time:		판매종료	#30,
		sold_yn:		품절			#25
	");
	this.dataModel=model;
}
GoodsInfoPage.gridClick(record, field) {
	target=tag[rect], ctrl=null;
	rc=Class.rect(0,0,890,560);
	switch(field ) {
	case note:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '비고', '비고 설정');
	case kitchen_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터IP', '주방프린터 아이피 설정');
	case kitchen_port:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터 포트', '주방프린터 포트 설정');
	case order_did_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주문스크린 IP', '주문스크린 아이피 설정');
	case order_did_port:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주문스크린 포트', '주문스크린 포트 설정');
	default:
	}
	if( ctrl ) {
		tag=ctrl[tag];
		tag[autoClose]=true;
	}
}

AdPanel.test() {
	node=tag[CurrentDisplayNode];
	not( node ) {
		draw.rectLine( rc.incr(10), 0, '#ff0000');
		return;
	}
	switch( node[ad_gubun] ) {
	case 2:
		/* 이미지 출력 (이미지 경우만 애니메이션 효과적용) */
		if( timeline ) {
			style=timeline[timelineStyle];
			if( style.eq('ExpandLeft','ExpandRight'), Cf.timeLine('ShiftMenu.running') ) {
				frame= Cf.timeLine('ShiftMenu.current');
				if( frame.eq(0) ) {
					tag[RateArray].recalc(rc.width(),20);
				} else {
					dx=tag[RateArray].sum(0,frame);
					x=rc.width() - dx;
					draw.drawImage(rc.x(x), tag[DrawImage] );
					return;
				}
			}
		}
		draw.drawImage(rc, tag[DrawImage] );
	case 3:
		/* 동영상 출력 */
		this.mainControl().showSubWidget('moviePlayer', rc);
	case 4:
		/* 웹브라우져 출력 */
		this.mainControl().showWebview('webview',rc);
	default:
		draw.drawImage(rc, tag[DrawImage] );
	}
}


GridControl.search() {
	model=Class.model('GoodsInfo');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	db.fetchAll( conf("sql#hitec.goodsInfo"), root.removeAll() );
	this.findControl('GridControl').setModel(root, "
		clplu_cd: 		코너명			#40,
		goods_cd: 		상품코드 		#25,
		goods_nm: 	상품명 			#45,
		uprice:			가격			 	#30,
		goods_img:	상품이미지		#30,
		start_time:		시작시간		#25,
		end_time:		종료시간		#25,
		sold_yn:		품절여부		#20
	");
	this.dataModel=model;
}

GoodsInfoPage.setCommCode(node) {
	form=this.findTag('Form');
	cur= this.currentInpu;
	field=cur[field], code=node[code];
	form[$field]=code;
	cur[text]=node[value];
}
GoodsInfoPage.drawGrid(draw, rc, record, field) {
	switch( field ) {
	case clplu_cd:
		text=getCommCodeValue('corner', record[$field]);
		draw.text(rc, text, 'center');
	case sold_yn:
		text=getCommCodeValue('kiosk#soldOut', record[$field]);
		draw.text(rc, text, 'center');
	case uprice:
		text=util_priceComma( record[$field]);
		draw.text(rc.incrW(-15), text, 'right');
	case goods_nm:
		draw.text(rc.incrX(8), record[$field]);
	case goods_img:
		draw.text(rc.incrX(4), record[$field]);
	default:
		draw.text(rc, record[$field], 'center');
	}
}

PingTestGrid.search() {
	root=grid.rootNode();
	/* 조회 쿼리를 넣어준다*/
	db=Class.db('kiosk_hitec');
	db.fetchAll( conf("sql#hitec.searchPingError"), root.removeAll() );
	gridHeaderWidth(grid);
	grid.update();

}

	KioskInfoPage.search() {
	model=Class.model('CornerInfo');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	if(cf[noSetupType].eq('1')) {
		/* 코너별 아이피 설정 */
		db.fetchAll( conf("sql#hitec.CornerInfo"), root.removeAll() );
		this.findControl('GridControl').setModel(root, "
			clplu_nm: 			코너명			#55,
			img_file_nm: 		코너 이미지	#45,
			kitchen_ip1:		주방 IP			#40,
			screen_ip:			주문스크린 IP	#40,
			note:                 비고				#25,
		");
	} else if(cf[noSetupType].eq('2')) {
		/* 프린트 번호별 아이피 설정 */
		db.fetchAll( "SELECT print_no, screen_ip, screen_port, kitchen_ip, kitchen_port, note
			FROM kiosk_print_setup
			ORDER BY print_no ASC", root.removeAll() );
		this.findControl('GridControl').setModel(root, "
			print_no: 			프린트번호		#10,
			screen_ip:			주문스크린 IP	#40,
			kitchen_ip:		    주방 IP			#40,
			note:                 비고				#40,
		");
	}
	this.dataModel=model;
	form=this.findTag('Form');
	db.fetch( conf('sql#hitec.selectKioskSetup'), form);
	while( row, form ) {
		while( cell, row ) {
			field=cell[field];
			not( field ) continue;
			cell[text]=form[$field];
		}
	}
}
KioskInfoPage.applyKiosk() {
	main=this.mainControl();
	not( main[page].confirm("키오스크 설정내용을 적용할까요?") ) {
		return;
	}
	form=this.findTag('Form');
	while( row, form ) {
		while( cell, row ) {
			field=cell[field];
			not( field ) continue;
			switch( cell[tag] ) {
			case check:
				form[$field]=when( cell[checked],'Y','N');
			default:
				form[$field]=cell[text];
			}
			not( cell[option] ) {
				not( form[$field] ) {
					main[page].alert( "$cell[label] 정보가 입력되지 않았습니다");
					return;
				}
			}
		}
	}
	db=Class.db('kiosk_hitec');
	db.exec("update kiosk_setup set use_yn='N' where use_yn='Y' ");
	db.exec(conf('sql#hitec.addKioskSetup'), form);
	if( db.error() ) {
		main[page].alert("키오스크 설정내용을 저장중 오류가 발생했습니다");
		return;
	}
	if( main[page].confirm("주문시작 번호를 초기화 할까요 ?") ) {
		db.fetch("select max(open_date) as open_date from kiosk_open_close where close_date is null", form);
		db.exec("delete from tb_key_gen where key_type='DealNo' and key_date=#{open_date}", form);
	}
	main[page].alert("키오스크 설정내용을 저장했습니다");
}
KioskInfoPage.gridClick(record, field) {
	target=tag[rect], ctrl=null;
	rc=Class.rect(0,0,890,560);
	switch(field ) {
	case print_no:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '프린트번호', '프린트번호 설정', 'num');
	case note:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '비고', '비고 설정','kor');
	case kitchen_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터IP', '주방프린터 IP 설정', 'num');
	case kitchen_ip1:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터IP1', '주방프린터 IP1 설정', 'num');
	case kitchen_ip2:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터IP2', '주방프린터 IP2 설정', 'num');
	case screen_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', 'common', rc, target, 'center');
		ctrl.setForm(record, field, '주문스크린 IP', '주문스크린 아이피 설정', 'num');
	default:
	}
	if( ctrl ) {
		tag=ctrl[tag];
		tag[autoClose]=true;
	}
}

NumberPad.setInputText(input, num) {
	text=input[text];
	not( text ) text='';
	if( num.eq('0') ) {
		text.add('0');
		input[text]=text;
	} else if( num.eq('delete') ) {
			input[text]=text.value(0,-1);
	} else if( num.eq('reset') ) {
		input[text]='';
	} else {
		text.add(num);
		input[text]=text;
	}
}

ErrorView.ErrorView(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
ErrorView.initControl() {
	tag[Width]=1080, tag[Height]=1793;
	tag.removeAll();
	tag.addNode({tag: Title, Height:92});
	/* 키오스크 에러 */
	tag.addNode({tag: SubTitle, id:sub1,  Margin:[15,20], Height:60} );
	form=tag.addNode({tag: Form, id:form1, Margin:[15,4], Height:85} );
	form.addNode({tag:date, field:reg_dt, label: 날짜});
	form.addNode({tag:combo, field:error_type, label: 유형});
	form.addNode({tag:button, field:search});
	tag.addNode({ tag: GridControl, id: Errorkiosk, ClassPath:common, Margin:[15,20], Height:435});
	/* 로그 에러 */
	tag.addNode({tag: SubTitle, id:sub2, Margin:[15,20], Height:60} );
	form=tag.addNode({tag: Form, id:form2, Margin:[15,4], Height:85} );
	form.addNode({tag:date, field:reg_dt, label: 날짜});
	form.addNode({tag:combo, field:log_type, label: 유형, text:오류, code:E});
	form.addNode({tag:button, field:search});
	tag.addNode({tag: GridControl, id: ErrorLog, ClassPath:common, Margin:[15,20]});
	tag.addNode({tag: Buttons, Height:65});
}
ErrorView.conf() {
	while( cur, tagRect(tag,true) ) {
		switch( cur[tag] ) {
		case Form:
			this.confForm(cur);
		case GridControl:
			this.getControl(cur).conf();
		}
	}
	not( this.dataModel ) {
		this.search();
	}
}
ErrorView.draw(draw, tm) {
	draw.drawImage( tag[rect], commonImage('admin_bg'), 'fill');
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Title:
			draw.drawImage(cur[rect], commonImage('admin_title'),0,0 );
			draw.font(28,'bold','#fd9437').text(cur[rect].incrX(25), "오류내역 조회");
		case SubTitle:
			switch(cur[id]) {
			case sub1:	draw.font(18,'bold','#f0f0f0').text(cur[rect].incrX(25), "* 키오스크 오류 내역");
			case sub2:	draw.font(18,'bold','#f0f0f0').text(cur[rect].incrX(25), "* 로그 내역");
			}
		case Form:
			this.drawForm(draw, cur);
		case GridControl:
			this.getControl(cur).draw(draw, tm);
		}
	}

}
	ErrorView.mouseDown(pos) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case Form:
			while( cell, cur ) {
				not( cell[rect].contains(pos) ) continue;
				this.currentInput=cell;
				rc=cell[rect];
				switch( cell[tag] ) {
				case date:
					rcOpen=rc.move('down').size(738,860);
					this.findControl('#Content').popupOpen('Calendar', this, rcOpen );
				case combo:
					rcOpen=rc.move('down').size(450, 350);
					node=this.findControl('#Content').popupOpen('CommCombo', this, rcOpen );
					if( cur[id].eq('form1') ) {
						this.getControl(node).setCommCode('kiosk#errorType','오류 유형');
					} else {
						this.getControl(node).setCommCode('kiosk#errorKind','로그 유형');
					}
				case button:
					this.mouseDownRect=rc;
					this.update();
					if( cur[id].eq('form1') ) {
						this.searchError();
					} else {
						this.searchLog();
					}
				}
			}
		case GridControl: 	this.getControl(cur).mouseDown(pos);
		default:
		}
	}
}
ErrorView.mouseUp(pos) {
	while( cur, tag ) {
		switch( cur[tag] ) {
		case GridControl: 	this.getControl(cur).mouseUp(pos);
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
ErrorView.confForm(form) {
	cell=form.child(0), m0=textWidth(14, cell[label],'bold') + 15;
	cell=form.child(1), m1=textWidth(14, cell[label],'bold') + 15;
	fw=form[rect].width()-20;
	rect=form[rect].center( fw, 40);
	rectRateArray(rect,"$m0,180,50,$m1,$90,*,130").inject(
		label0, input0, space, label1, input1,space, button
	);
	form.put(label0, input0, label1, input1);
	while( cell, form, c ) {
		switch( cell[tag] ) {
		case button:
			cell[rect]=button.center(123,65);
		default:
			rc=form[input$c].incr(4);
			if( cell[width] ) rc.width(cell[width]);
			cell[rcLabel]=form[label$c], cell[rcInput]=rc;
			cell[rect]=rc.incr(-2);
		}
	}
}
ErrorView.drawForm(draw, form) {
	draw.rectLine( form[rect], 0, '#c0c0c0');
	while( cell, form ) {
		switch( cell[tag] ) {
		case button:
			draw.drawImage(cell[rect], commonImage('btn_bg') );
			draw.font(16,'bold','#f0f0f0').text(cell[rect], "조회", "center");
		default:
			if( cell[tag].eq('date') ) {
				not( cell[text] ) cell[text]=System.date('yyyy-MM-dd');
			}
			draw.font(14, 'bold', '#f0f0fa').text(cell[rcLabel], "$cell[label] :");
			rc=cell[rect];
			draw.font(14, 'bold','#40404a');
			draw.fill(rc,'#eeeaea').draw.rectLine( rc, 0, '#20202a');
			draw.text(rc, cell[text]);
		}
	}
}
ErrorView.search(form) {
	this.searchError();
	this.searchLog();
}
ErrorView.gridClick(record, field, grid) {
	target=tag[rect], ctrl=null;
	rc=Class.rect(0,0,890,560);
	switch(field ) {
	case note:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '비고', '비고 설정');
	case kitchen_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터IP', '주방프린터 아이피 설정');
	case kitchen_port:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주방프린터 포트', '주방프린터 포트 설정');
	case order_did_ip:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주문스크린 IP', '주문스크린 아이피 설정');
	case order_did_port:
		ctrl=this.findControl('Popup#dialog').popupOpen('InputTextForm', null, rc, target, 'center');
		ctrl.setForm(record, field, '주문스크린 포트', '주문스크린 포트 설정');
	default:
	}
	if( ctrl ) {
		tag=ctrl[tag];
		tag[autoClose]=true;
	}
}
ErrorView.setCommCode(node) {
	input=this.currentInput;
	input[code]=node[code];
	input[text]=node[value];
	this.update();
}
ErrorView.find(id) {
	while( cur, tag ) {
		not( cur[id] ) continue;
		if( cur[id].eq(id) ) return cur;
	}
	return null;
}
ErrorView.searchError() {
	grid=this.find('Errorkiosk'), form=this.find('form1');
	model=Class.model('ErrorKiosk');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	if( form ) {
		not(  form[reg_dt] )  form[reg_dt]=System.date('yyyy-MM-dd');
		root[reg_dt]		= form[reg_dt];
		root[error_type]	= from[error_type];
	}
	db.fetchAll( conf("sql#hitec.ErrorKiosk"), root.removeAll() );
	this.getControl(grid).setModel(root, "
		error_type: 	오류유형		#20,
		error_kind: 		종류				#25,
		error_data:		오류내용		#200,
		status:			상태				#15,
		reg_dt:			등록일시		#150
	");
}
ErrorView.searchLog() {
	grid=this.find('ErrorLog'), form=this.find('form2');
	model=Class.model('ErrorLog');
	root=model.rootNode();
	db=Class.db('kiosk_hitec');
	if( form ) {
		a=form.child(0), b=form.child(1);
		root[reg_dt]		= a[text];
		root[log_type]	= b[code];
	}
	db.fetchAll( conf("sql#hitec.ErrorLog"), root.removeAll() );
	this.getControl(grid).setModel(root, "
		log_type: 		로그유형	#20,
		log_msg:		로그내용	#500,
		reg_dt:			등록일시	#110
	");
}
ErrorView.a() {
	this.initControl();
	this.conf();
	this.search();
}
ErrorView.setCalendarDate(date) {
	input=this.currentInput;
	input[text]=date;
	this.update();
}

KioskInfoPage.applyClick(root, grid) {
	if(cf[noSetupType].eq('1')) {
		/****************** 영동고속도로 *****************/
		update=getQuery('kiosk_corner_setup', 'clplu_cd, kitchen_ip1	, kitchen_ip2, screen_ip, kitchen_use_yn, screen_use_yn, note', 'clplu_cd');
		insert=getQuery('kiosk_corner_setup', 'clplu_cd, kitchen_ip1, kitchen_ip2, screen_ip, kitchen_use_yn, screen_use_yn, note' );
		cnt=0;
		main=this.mainControl();
		db=Class.db('kiosk_hitec');
		while( cur, root ) {
			not( cur.state(NODE.modify) ) {
				continue;
			}
			a=cur[screen_ip].trim();
			cur[screen_use_yn]=when( a, 'Y','N');
			a=cur[kitchen_ip1].trim(), b=cur[kitchen_ip2];
			if( a || b ) {
				cur[kitchen_use_yn]='Y';
			} else {
				cur[kitchen_use_yn]='N';
			}
			not( db.exec(update, cur) ) {
				db.exec(insert, cur);
				print( insert, cur );
			}
			cnt++;
			if( db.error() ) {
				err=db.error();
				main[page].alert("매장코너 적용중 오류가 발생했습니다. errror: $err");
				return;
			}
		}
		main[page].alert("매장코너 아이피 ${cnt}건 적용하였습니다.");
	} else if(cf[noSetupType].eq('2')) {
		/* 영동고속도로 이후 */
		/* 삭제 후 데이터 등록 */
		main=this.mainControl();
		db=Class.db('kiosk_hitec');
		db.exec("delete from kiosk_print_setup");
		insert=getQuery('kiosk_print_setup', 'print_no, screen_ip, kitchen_ip, screen_use_yn, kitchen_use_yn,  note' );
		cnt=0;
		while( cur, root ) {
			a=cur[screen_ip].trim();
			cur[screen_use_yn]=when( a, 'Y','N');
			a=cur[kitchen_ip].trim();
			cur[kitchen_use_yn]=when( a, 'Y','N');
			if(cur[screen_use_yn].eq('N') && cur[kitchen_use_yn].eq('N'))
				continue;
			db.exec(insert, cur);
			print( insert, cur );
			cnt++;
			if( db.error() ) {
				err=db.error();
				main[page].alert("매장코너 적용중 오류가 발생했습니다. errror: $err");
				return;
			}
		}
		main[page].alert("매장코너 아이피 ${cnt}건 적용하였습니다.");
	}
	this.search();
}

ListBox.a() {
	tag[drawRowBox]=null;
		tag[drawRowImgBox]=null;
	this.conf();
}


SaleOpenView.openStoreButtonClick() {
	page=Cf[KioskWatcher];
	page.inject(cf);
	pwd=tag[pwd];
	cf[today]=System.date( 'yyyyMMdd' );
	db=Class.db('kiosk_hitec');
	db.fetch("select open_date, close_date from kiosk_open_close where open_date=#{today}", cf);
	if( cf[close_date] ) {
		page.alert("$cf[today] 는 이미 마감처리되었습니다");
		return;
	}
	if( cf[open_date] ) {
		page.alert("$cf[today] 는 이미 개점처리되었습니다");
		return;
	}
	if( db.count("select count(1) from kiosk_open_close") ) {
		while( n, 10 ) {
			db.fetch("select max(open_date) as open_date from kiosk_open_close where close_date is null", cf);
			/* 모두 마감됨 */
			not( cf[open_date] ) {
				break;
			}
			db.fetch("select close_date from kiosk_open_close where open_date=#{open_date}", cf);
			not( cf[close_date] ) {
				/* 사용자 취소 */
				not( page.confirm("$cf[open_date] 는 마감되지 않았습니다. 자동으로 마감할까요?") ) {
					return;
				}
				db.exec("update kiosk_open_close set close_date=#{open_date}, close_time='2400', reg_reopen_dtm=now() where open_date=#{open_date}  ", cf);
			}
		}
	}
	cf[open_time]=System.date('HHmm');
	db.exec(conf("sql#kiosk.hitec#OpenStore"), cf);
	today=System.date('yyyy-MM-dd');
	page.alert("$today 개점처리가 완료 되었습니다");
	if( pwd.eq(cf[kiosk_pw]) ) {
	} else {
		print("키오스크 관리자 비밀번호와 일치하지 않습니다");
	}
}
SaleOpenView.search() {
	tag[pwd]='';
}


SaleCloseView.search(date) {
	setup=_node('SetupInfo');
	page=Cf[KioskWatcher];
	cf=this[dataNode].initNode();
	cf[ms_no]=setup[ms_no];
	cf[pos_no]=setup[pos_no];
	cf[reg_close_dtm]=null;
	cf[close_date]=null;
	cf[nextClose]=null;
	db=Class.db('kiosk_hitec');
	if( date ) {
		cf[open_date]=date;
	} else {
		db.fetch("SELECT max(open_date) as open_date FROM kiosk_open_close", cf);
		not( cf[open_date]  ) {
			cf[open_date]=System.date('yyyyMMdd');
			db.exec("insert into kiosk_open_close(open_type,  open_date,  open_time,  reg_open_dtm, status) values( 'A', #{open_date}, '0000', now(), 'R')", cf );
		}
	}
	db.fetch("select open_date, open_time, close_date, close_time, reg_open_dtm, reg_close_dtm from kiosk_open_close where open_date=#{open_date}", cf);
	db.fetch( conf('sql#hitec.admin#SaleCloseSearch'), cf);
}
SaleCloseView.setCalendarDate(date) {
	from=this.currentInput;
	day=date.replace("-","");
	print("xxxxxxxxxxxxxx $day xxxxxxxxxxxxx");
	this.search(day);
	this.update();
}
SaleCloseView.saleCloseClick() {
	cf=this[dataNode];
	if( cf[total_amt].eq('0') ) {
		main=this.mainControl();
		not( main[page].confirm("영업일자 $cf[open_date]에 매출이 없습니다. 마감하시겠습니까?") ) {
			return;
		}
	}
	rc=Class.rect(0,0,936,1244);
	popup=this.findControl('#Content').popupOpen('SaleCloseConfirm', this, rc, 'center', 'popup');
	this.getControl(popup).setDataNode( this.dataNode, this);
}


AdminMenuCanvas.qtMonSendData(send) {
	not( send ) return;
	node=_node('QtMonNode');
	socket=node[socket];
	if( socket.isConnect() ) {
		s='$';
		s.add(send);
		socket.sendBuffer("$s\n");
		print("xxxxxxxxxxxxxxxxxxxx qtMon connnect xxxxxxxxxxxxxxxxxxxx");
	} else {
		socket.close();
		print("xxxxxxxxxxxxxxxxxxxx qtMon nodt connnect xxxxxxxxxxxxxxxxxxxx");
	}
}
AdminMenuCanvas.qtMonRecvData(recv) {
	not( recv.ch().eq('$') ) {
		_log("QtMon 결과가 유효한 형식이 아닙니다 : 응답결과=$recv");
	}
	val=recv.value(1);
	_log("qtMonRecvData => $val");
	switch( val.findPos(',').trim() ) {
	case 21: 	/* 인쇄 */
		val.split().inject( reqType, subCode, paramCount, printKind, papper, printHeadUp, jam  );
		print("#영수증 인쇄 로그 => $reqType, $subCode, $paramCount, $printKind, $papper, $printHeadUp,$ jam ");
		if( subCode.eq('02') ) {
			msg='';
			if( printKind.eq(0) ) {
				msg.add("영수증 프린터 : ");
			} else {
				msg.add("주방 프린터 $printKind : ");
			}
			if( papper.eq('1') || printHeadUp.eq('1') || jam.eq('1') ) {
				if( papper.eq('1') ) msg.add("용지 없음 ");
				if( printHeadUp.eq('1') ) msg.add("프린터 헤더 열림 ");
				if( jam.eq('1') ) msg.add("용지 걸림");
				msg.add("오류가 발생했습니다");
				print("# $msg");
			}
		} else if( subCode.eq('03') ) {
			success=printKind;
			if( success.eq('0') ) {
				print( '# 영수증 출력 오류가 발생했습니다' );
				return;
			}
		}
	case 22:
		val.split().inject( reqType, paramCount,  success );
		print("#주방 프린트 로그 => $reqType, $paramCount,  $success");
		if( success.eq(0)){
			print('주방프린터 출력 오류');
			return;
		}
	default:
		print("# qtMonRecvData 타입정의 없음: $val");
	}
	print("xxxxxxxxxxxxxxxx qtMonRecvData xxxxxxxxxxxxxxxx");
}
AdminMenuCanvas.easyCardReadData(&data, node) {
	data.findPos('{',1,1);
	str=data.utf8();
	node.parseJson(str);
	_log("easyCardReadData: $node");
	if( node[SUC].eq('00') ) {
		/* 카드승인 성공*/
		if( node[RS04].eq("0000") ) {
			record		= cf[record];
			cancle_completeCardProcess(this,record);
		} else {
			if( cf[CashReceiptType] ) {
				msg="Easy Check 승인중";
			} else {
				msg="카드 승인중";
			}
			this.alert("$msg 오류가 발생했습니다.\n오류코드: $node[RS04]\n오류내용: $node[RS16]\n$node[RS17]", "결제오류");
		}
	} else {
		this.alert("카드승인 결제 오류가 발생했습니다.\n오류: $node[MSG]", "결제오류");
	}
}
AdminMenuCanvas.easyCardError(&data, node) {
	this.alert("Easy Check 인증중 오류가 발생했습니다.\n오류: $data", "결제오류");
}
AdminMenuCanvas.easyCardCall(reqCd,record) {
	req=_node("EasyCardNode");
	if( req[status] ) {
		dist=System.tick()-req[startTick];
		if( dist>6000 ) {
			_log("## easyCardRequestCall: $req[status] 대기시간 초과. 연결을 초기화 합니다");
			req[status]=null;
		}
		return false;
	}
	cf[easyCardUrl]	= conf('setup#kiosk.easyCardUrl');
	cf[record] = record;
	req[url]=cf[easyCardUrl];
	print(record[sale_amt]);
	moneyPaperType='';
	money = record[total_amt];
	depth 			='';
	cancelDt = record[arv_dt].value(0,6);
	cancelNo =record[trdata9];
	productCd		='';
	productCd		='';
	saleNo			='';
	webMsg		='';
	keyInYn 		="Y";
	termNo 			='';
	vat            = "A";
	timeout 			=30;
	addField		='';
	receiveHandle	='';
	termType		='';
	disType			='';
	option			='';
	extOption		='';
	value="$reqCd^$moneyPaperType^$money^$depth^$cancelDt^$cancelNo^$productCd^$saleNo^$webMsg^$keyInYn^$termNo^$timeout^$vat^$addField^$receiveHandle^$termType^$disType^$option^$extOption";
	/*
	value= 'D4^^1000^^130813^06093541^23^1234567890^WEB1234567890^^';
	*/
	_log("easyCardRequestCall: $func, $value");
	param=_node( req,'param').initNode();
	param[REQ]=value;
	param[callback]="result_${tick}";
	req[status]		='start';
	req[startTick]		=System.tick();
	page.easyCardSend(req);
}


SaleStatusView.gridClick(record, field) {
	this.currentRecord=record;
	rc=Class.rect(0,0,936,1244);
	popup=this.findControl('#Content').popupOpen('CancleConfirm', this, rc, 'center', 'popup');
	db=Class.db('kiosk_hitec');
	db.fetchAll(conf('sql#hitec.SaleStatusDetail'), record.removeAll() );
	this.getControl(popup).setRecord(record);
}
SaleStatusView.drawGrid(draw, rc, record, field) {
	if( cf[popupControl] && field.eq('trdata9') ) {
		if( record==this.currentRecord ) {
			rcRow=record[rect];
			draw.fill(rcRow, '#c0808a90');
		}
	}
	val=record[$field];
	switch( field ) {
	case total_amt:
		price=util_priceComma(val);
		draw.text(rc.incrW(-5), price,'right');
	case cancle_yn:
		rcBox=rc.center(110,30);
		if( val.eq('Y') ) {
			draw.fill(rcBox,'#daa0a0').rectLine(rcBox, 0, '#f08080', 1);
			draw.text(rcBox, '취소됨', 'center');
		} else {
			draw.fill(rcBox,'#606060').rectLine(rcBox, 0, '#30303a', 1);
			draw.text(rcBox, '결제취소', 'center');
		}
	default:
		draw.text(rc, val,'center');
	}
}

CancleConfirm.CancleConfirm(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
CancleConfirm.initControl() {
	cf.inject(imagePath);
	tag[Width]="936", tag[Height]="1244";
	tag.addNode({tag:Title, Height:114});
	tag.addNode({tag:OrderTitle, Margin:[20,5], Height:132});
	tag.addNode({tag:OrderList, Margin:[20,5]});
	tag.addNode({tag:OrderInfo, Margin:[20,15,20,20], Height:95});
	tag.addNode({tag:Buttons, Height:145});
	tag[title]="주문취소 (카드)"
	tag[BackgroundImage]	="${imagePath}/Common/popup_bg.png";
	tag[CloseButton]			="${imagePath}/Common/popup_close_[#].png";
	tag[OrderDeleteButton]	="${imagePath}/Type/list_del_[#].png";
	setNodeSize(tag, true);
}
CancleConfirm.conf() {
	tagClearRect(tag);
	setNodeSize(tag);
	confNodeLayout(tag);
	root= this.dataNode;
	while(cur, tag ) {
		cur[rect].inject(sx, sy, sw, sh);
		switch(cur[tag] ) {
		case Title:
			temp=_arr(this,'TempRate').recalc(sw, '*,80', true);
			temp.inject(a,b);
			cur[rcTitle]=Class.rect(sx, sy, a, sh), sx+=a;
			cur[rcClose]=Class.rect(sx, sy, b, sh);
			img=imageLoad(tag, 'CloseButton', 'n');
			img.imageSize().inject(w,h);
			cur[rcCloseButton]=cur[rcClose].center(w,h);
		case OrderList:
			temp=_arr(this,'TempRate').recalc(sw, '*,125,185', true);
			cx=sx, sh=80;
			while( cw, temp, c, 0 ) {
				rcCell=Class.rect(cx, sy, cw, sh), cx+=cw;
				switch(c) {
				case 0: cur[rcMenuOption]=rcCell;
				case 1: cur[rcQty]=rcCell;
				case 2: cur[rcPrice]=rcCell;
				case 3: cur[rcDelete]=rcCell;
				}
			}
			sy+=80, sh=70;
			while( sub, root, r,0 ) {
				cx=sx;
				sub[rect]=Class.rect(cx, sy, sw, sh);
				while( cw, temp, c, 0 ) {
					rcCell=Class.rect(cx, sy, cw, sh), cx+=cw;
					switch(c) {
					case 0: sub[rcMenuOption]=rcCell;
					case 1: sub[rcQty]=rcCell;
					case 2: sub[rcPrice]=rcCell;
					case 3: sub[rcDelete]=rcCell;
						img=imageLoad(tag,'OrderDeleteButton', 'n');
						img.imageSize().inject(w,h);
						sub[rcDeleteButton]=sub[rcDelete].center(w,h);
					default:
					}
				}
				sy+=sh;
			}
		case OrderInfo:
			temp=_arr(this,'TempRate').recalc(sw, '160,110,*,180,160,20', true);
			temp.inject(a1,a2,x,b1,b2);
			sy-=20;
			cur[rcTitleQty]	=Class.rect(sx,sy,a1,sh), sx+=a1;
			cur[rcQty]			=Class.rect(sx,sy,a2,sh), sx+=a2+x;
			cur[rcTitlePrice]	=Class.rect(sx,sy,b1,sh), sx+=b1;
			cur[rcPrice]		=Class.rect(sx,sy,b2,sh);
		case Buttons:
			rectRateArray( cur[rect], '3,3,3' ).inject(a,b,c);
			cur[rcClose]				=a.center(290, 86);
			cur[rcRePrint]			=b.center(290, 86);
			cur[rcOrderCancle]	=c.center(290, 86);
		default:
		}
	}
	this.cancleClick=false;
}
CancleConfirm.draw(draw, timeline) {
	lang_0="메뉴", lang_1='수량', lang_2='가격';
	lang_qty='주문수량', lang_price='주문금액';
	drawNodeStyle(draw, tag);
	root= this.dataNode;
	while(cur, tag ) {
		switch(cur[tag] ) {
		case Title:
			drawNodeText(draw, cur[rcTitle], "주문취소 (카드)", "left", "PopupTitle");
			drawNodeImage(draw, cur[rcCloseButton], tag, 'CloseButton', 'n', true);
		case OrderTitle:
			draw.html( cur[rect], root[note] );
		case OrderList:
			draw.fill(cur[rect].height(80), '#b0b0b0');
			while(c,4) {
				switch(c) {
				case 0:
					drawNodeText(draw, cur[rcMenuOption], lang_0, "left", "OrderHeader");
				case 1:
					drawNodeText(draw, cur[rcQty], lang_1, "center", "OrderHeader");
				case 2:
					drawNodeText(draw, cur[rcPrice], lang_2, "center", "OrderHeader");
				}
			}
			while( sub, root ) {
				draw.rectLine( sub[rect].incr(5), 4, '#d0d0d0', 1, 'dash');
				while(c,4) {
					switch(c) {
					case 0:
						drawNodeText(draw, sub[rcMenuOption].incrX(30), sub[menu_nm], 'left','OrderInfo');
					case 1:
						drawNodeText(draw, sub[rcQty], sub[qty], 'center','OrderInfo');
					case 2:
						price=sub[qty]*sub[price];
						priceSum=util_priceComma(price);
						drawNodeText(draw, sub[rcPrice].incrW(-10), "$priceSum 원", 'right','OrderInfo');
					case 3:
						ty=when( sub[rcDeleteButton].eq(this.mouseDownRect), 'p', 'n');
						drawNodeImage(draw, sub[rcDeleteButton], tag, 'OrderDeleteButton', ty, true);
					}
				}
			}
		case OrderInfo:
			qty=root[OrderTotalQty];
			price=util_priceComma(root[OrderTotalPrice]);
			drawNodeText(draw, cur[rcTitleQty], 	"$lang_qty :",	'left','OrderInfo');
			drawNodeText(draw, cur[rcQty], 		"$qty 건", 		'right', 'OrderInfo');
			drawNodeText(draw, cur[rcTitlePrice], "$lang_price :", 	'left', 'OrderInfo');
			drawNodeText(draw, cur[rcPrice], 		"$price 원", 	'right', 18, '#C00D12', 'bold');
		case Buttons:
			rc=cur[rcClose], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_cancel',ty) );
			drawNodeText(draw, rc, "닫기", "center", 'PopupButton');
			rc=cur[rcRePrint], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "재발행", "center", 'PopupButton');
			rc=cur[rcOrderCancle], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			if( root[cancle_yn].eq('Y') ) {
				drawNodeText(draw, rc, "원영수증", "center", 'PopupButton');
			} else {
				drawNodeText(draw, rc, "주문취소", "center", 'PopupButton');
			}
		default:
		}
	}
}
CancleConfirm.mouseDown(pos) {
	while(cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case Title:
			if( cur[rcCloseButton].contains(pos) ) {
				this.mainControl().popupClose();
				break;
			}
		case Buttons:
			if( cur[rcClose].contains(pos) ) {
				this.mouseDownRect=cur[rcClose];
			} else if( cur[rcRePrint].contains(pos) ) {
				this.mouseDownRect=cur[rcRePrint];
			} else if( cur[rcOrderCancle].contains(pos) ) {
				this.mouseDownRect=cur[rcOrderCancle];
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
CancleConfirm.mouseUp(pos) {
	while(cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case Buttons:
			if( cur[rcClose].contains(pos) ) {
				this.findControl('Popup#dialog').popupClose();
			} else if( cur[rcRePrint].contains(pos) ) {
				this.orderRePrintClick();
			} else if( cur[rcOrderCancle].contains(pos) ) {
				root = this.dataNode;
				main = this.mainControl();
				if( root[cancle_yn].eq('Y') ){
					order_realPrint(main,root);
					return;
				}
				if( this.cancleClick ) return;
				this.cancleClick=true;
				this.orderCancleClick();
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
CancleConfirm.orderRePrintClick() {
	record=this.dataNode;
	main = this.mainControl();
	if( record[cancle_yn].eq('Y') ) {
		cancle_cardPrint(main,record);
		return;
	}
	/* 원래영수증 출력 */
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
		pm_no = "$cur[pos_no] - $record[bill_no]";
		s.add("$cur[menu_nm]^$cur[price]^$cur[qty]^${total_amt}^${total_amt}^${class_seq}_${corner_nm}^${pm_no}\t");
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
	   /* 카드종류	,	승인일시	,	승인금액	,	할부조건	,	승인번호	,	가맹점번호	,	알림 */
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
CancleConfirm.orderCancleClick() {
	main = this.mainControl();
	/* 카드 VAN사 연결 */
	print('###################################VAN###################################');
	print(cf[cardVanType]);
	if(cf[cardVanType].eq('1')) {
		/* KCP */
		main.easyCardCall('D4',this.dataNode);
	} else if(cf[cardVanType].eq('2')) {
		/* KSNET */
		setup=cf[SetupInfo];
		print("### ms_cat_id=$setup[ms_cat_id]");
		/* qtMon 전송(30초간 blocking) */
		qtMonNode = _node('QtMonNode');
		qtMonNode[data] = null;
		record = this.dataNode;
		cancelDt = record[arv_dt].value(0,6);
		cancelNo =record[trdata9];
		money = record[total_amt];
		/* 명령,파라미터수,거래구분,업무구분,전문구분,거래형태,승인일자,승인번호,승인금액,타임아웃 */
		main.qtMonSendData("25,7,IC,01,0420,N,$cancelDt,$cancelNo,$money,30,$setup[ms_cat_id]");
		while( n, 62 ) {
			if( qtMonNode[data] ) {
				break;
			}
			System.sleep(500);
		}
		/*****
		System.timeout( 60000, func() {
			while( n, 62 ) {
				if( @finish ) break;
				if( qtMonNode[data] ) {
					return @event.finish(); // 타임아웃 중간에 종료시킴
				}
				System.sleep(500);
			}
		});
		*****/
		/* qtMon 응답 데이터 처리 */
		ch=qtMonNode[data].ch();
		if( ch.eq('$') ) {
			recv=qtMonNode[data].value(1);
			/* 명령번호, 파라메터수, 상태, 오류 메시지 , 카드번호 , 승인일시 , 승인번호 , 매입사코드 , 발급사이름*/
			recv.split().inject(commandNum, paramCount, status, message, cardNo,
				arvDate, arvNo, purCardNo, purCardNm);
			if( commandNum.eq(25) && status.eq('O') ) {
				cancle_completeCardProcess(main, record);
			} else {
				msg = message.utf8();
				main.alert("카드 취소중 오류가 발생했습니다.\n오류내용: $msg", "알림");
			}
		} else {
			main.alert("카드 취소 응답오류", "알림");
		}
	}
}
CancleConfirm.setRecord(record) {
	cur=record.child(0);
	Class.db('kiosk_hitec').fetch("select bill_no from tb_sale_header where sale_seq=#{sale_seq}", cur);
	cur.inject( bill_no, pos_no );
	reg_dt=cur[reg_dt].replace('T',' ');
	bill_no=cur[bill_no];
	pos_no=cur[pos_no];
	note=conf('message#kiosk.SaleCancle');
	record[note]= fmt(note);
	record[bill_no]= bill_no;
	a=0, b=0;
	while( cur, record ) {
		a+=cur[qty], b+=cur[price*qty];
	}
	record[OrderTotalQty]=a, record[OrderTotalPrice]=b;
	this.dataNode=record;
	this.conf();
}


AdminMenuCanvas.alert(msg, title, error) {
	dialog=this.findControl('Popup#dialog');
	main=this[mainNode];
	rc=Class.rect(0,0,936,484), target=main[rect];
	/*
	if( error ) {
		cf[errorOpen]=true;
		cf[errorOpenTick]=System.tick();
		dialog.popupOpen('ErrorWindow','popup', rc, target);
	} else {
	}
	*/
	dialog.popupOpen('MessageWindow','common', rc, target);
	cur=dialog.getMainNode();
	this.getControl(cur).initPage(msg, title);
}
AdminMenuCanvas.popupClose() {
	cf[popupControl]=null;
	this.update();
}


SaleCloseConfirm.SaleCloseConfirm(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SaleCloseConfirm.initControl() {
	cf.inject(imagePath);
	tag.removeAll();
	tag.addNode({tag:Title, Height:114});
	tag.addNode({tag:Message, Margin:[20,5]});
	tag.addNode({tag:Buttons, Height:145});
	tag[title]="업무 마감 확인"
	tag[CloseButton]			="${imagePath}/Common/popup_close_[#].png";
	setNodeSize(tag, true);
}
SaleCloseConfirm.conf() {
	tag[rect]=null;
	tag[Width]="920", tag[Height]="550";
	tagClearRect(tag);
	setNodeSize(tag);
	confNodeLayout(tag);
	root= this.dataNode;
	while(cur, tag ) {
		cur[rect].inject(sx, sy, sw, sh);
		switch(cur[tag] ) {
		case Title:
			temp=_arr(this,'TempRate').recalc(sw, '*,80', true);
			temp.inject(a,b);
			cur[rcTitle]=Class.rect(sx, sy, a, sh), sx+=a;
			cur[rcClose]=Class.rect(sx, sy, b, sh);
			img=imageLoad(tag, 'CloseButton', 'n');
			img.imageSize().inject(w,h);
			cur[rcCloseButton]=cur[rcClose].center(w,h);
		case Message:
		case Buttons:
			rectRateArray( cur[rect], '3,3,3' ).inject(a,b,c);
			cur[rcClose]						=a.center(290, 86);
			cur[rcSaleCloseCancle]		=b.center(290, 86);
			cur[rcSaleClose]				=c.center(290, 86);
		default:
		}
	}
	this.cancleClick=false;
}
SaleCloseConfirm.draw(draw, timeline) {
	popupFadeIn(draw, timeline);
	draw.fill( tag[rect], '#f0f0f0' );
	root= this.dataNode;
	dataNode=this.dataNode;
	while(cur, tag ) {
		switch(cur[tag] ) {
		case Title:
			rc=cur[rect].incr(2);
			draw.fill(rc, '#c0c0c0').rectLine(rc,4,'#a0a0a0');
			drawNodeText(draw, cur[rect], tag[title], "left", "PopupTitle");
			drawNodeImage(draw, cur[rcCloseButton], tag, 'CloseButton', 'n', true);
		case Message:
			openDay=util_formatDate( dataNode[open_date] );
			drawNodeText(draw, cur[rect], "$openDay 업무마감 처리", "center", "TableHeader");
		case Buttons:
			rc=cur[rcClose], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_cancel',ty) );
			drawNodeText(draw, rc, "닫기", "center", 'PopupButton');
			t1='n', t2=when( cur[rcSaleClose].eq(this.mouseDownRect), 'p', 'n');
			print("################# $dataNode[next_close] ");
			if( dataNode[close_date] ) {
				if( dataNode[next_close] ) {
					t1='d';
				} else {
					t1=when( cur[rcSaleCloseCancle].eq(this.mouseDownRect), 'p', 'n');
				}
			} else {
				t1='d';
			}
			rc=cur[rcSaleCloseCancle];
			draw.drawImage( rc, commonImage('popup_confirm',t1) );
			drawNodeText(draw, rc, "마감취소", "center", 'PopupButton');
			rc=cur[rcSaleClose];
			draw.drawImage( rc, commonImage('popup_confirm',t2) );
			 if( dataNode[close_date] ) {
				drawNodeText(draw, rc, "마감전표 출력", "center", 'PopupButton');
			 } else {
				drawNodeText(draw, rc, "업무마감", "center", 'PopupButton');
			 }
		default:
		}
	}
}
SaleCloseConfirm.mouseDown(pos) {
	while(cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case Title:
			if( cur[rcCloseButton].contains(pos) ) {
				this.mainControl().popupClose();
				break;
			}
		case Buttons:
			if( cur[rcClose].contains(pos) ) {
				this.mouseDownRect=cur[rcClose];
			} else if( cur[rcSaleCloseCancle].contains(pos) ) {
				this.mouseDownRect=cur[rcRePrint];
			} else if( cur[rcSaleClose].contains(pos) ) {
				this.mouseDownRect=cur[rcOrderCancle];
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
SaleCloseConfirm.mouseUp(pos) {
	while(cur, tag ) {
		not( cur[rect].contains(pos) ) continue;
		switch(cur[tag] ) {
		case Buttons:
			if( cur[rcClose].contains(pos) ) {
				this.findControl('Popup#dialog').popupClose();
			} else if( cur[rcSaleCloseCancle].contains(pos) ) {
				this.saleCloseCancle();
			} else if( cur[rcSaleClose].contains(pos) ) {
				this.saleCloseOk();
			}
		default:
		}
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
SaleCloseConfirm.setDataNode(dataNode, parent) {
	page=main[page];
	this.dataNode=dataNode;
	this.opener=parent;
	day=60*60*24, openDay=util_formatDate( dataNode[open_date] );
	local=System.localtime(openDay);
	local+=day;
	dataNode[nextDay]=System.date(local,'yyyyMMdd');
	dataNode[next_close]=null;
	db=Class.db('kiosk_hitec');
	if( dataNode[close_date] ) {
		db.fetch( "select close_date as next_close from kiosk_open_close where open_date=#{nextDay}", dataNode);
		print("xxxxxxxxxxxx $dataNode[next_close] ###############");
	}
}
SaleCloseConfirm.saleCloseOk() {
	main=this.mainControl();
	dataNode=this.dataNode;
	if( dataNode[next_close] ) return;
	page=main[page];
	openDay=util_formatDate( dataNode[open_date] );
	db=Class.db('kiosk_hitec');
	/* 키오스크 정보 체크 */
	print("업무마감 시작 : $dataNode");
	not(dataNode[ms_no]) {
		db.fetch(conf("sql#hitec.selectKioskSetup"),  dataNode);
	}
	/* 마감전표 출력 */
	if( dataNode[close_date] ) {
		this.reprint(main, dataNode, db)
		kiosk_saleConerPrint(main, dataNode, db);
		main.alert("$openDay 마감전표를 출력했습니다", "알림");
		return;
	}
	/* 업무마감 */
	not( dataNode[nextDay] ) {
		local=System.localtime(openDay);
		day=60*60*24, local+=day;
		dataNode[nextDay]=System.date(local,'yyyyMMdd');
		not( dataNode[nextDay] ) {
			page.alert("영업일 조회중 오류가 발생했습니다");
			return;
		}
	}
	if( dataNode[saleCloseClick] ) return;
	dataNode[saleCloseClick]=true;
	this.saleReSend(db, dataNode);
	System.timeout(500);
	close_date=System.date('yyyyMMdd');
	dataNode[close_date]=System.date('yyyyMMdd');
	dataNode[close_time]=System.date('HHmm');
	db.exec("update kiosk_open_close set close_date=#{close_date}, close_time=#{close_time}, reg_close_dtm=now() where open_date=#{open_date}", dataNode);
	err=db.error();
	if( err ) {
		page.alert("업무마감 처리중 오류가 발생했습니다 error: $err");
		dataNode[saleCloseClick]=false;
		return;
	}
	db.exec("insert into kiosk_open_close ( open_type, open_date, open_time, reg_open_dtm ) values ( 'C', #{nextDay},'0000', now() )",dataNode);
	err=db.error();
	if( err ) {
		page.alert("업무마감 처리중 오류가 발생했습니다 error: $err");
		dataNode[saleCloseClick]=false;
		return;
	}
	kiosk_saleConerPrint(main, dataNode, db);
	kiosk_SaleClose(main, dataNode, db);
	this.saveSale( db  );
	this[opener].search();
	dataNode[saleCloseClick]=false;
	this.findControl('#Content').popupOpen('SaleExitButtons', this, Class.rect(0,0,800,650), 'center', 'popup');

}
	SaleCloseConfirm.saleCloseCancle() {
	dataNode=this.dataNode;
	if( dataNode[next_close] ) return;
	main=this.mainControl();
	page=main[page];
	openDay=util_formatDate( dataNode[open_date] );
		/* 마감  취소 */
	db=Class.db('kiosk_hitec');
	openDay=util_formatDate( dataNode[open_date] );
	if( dataNode[total_amt] ) {
		page.alert("매출액이 있으면 마감을 취소할수 없습니다.");
		return;
	}
	not( page.confirm("$openDay  마감을 취소할까요?") ) {
		return;
	}
	db.exec("delete from kiosk_open_close where open_date>#{open_date} and close_date is null", dataNode);
	db.exec("update kiosk_open_close set close_date=null, close_time=null, reg_close_dtm=null where open_date=#{open_date}", dataNode);
	main.alert("$openDay $dataNode 마감취소가 완료되었습니다","알림");
	this[opener].search();
}

Loading.Loading(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
Loading.closePopup() {
	this[pageStart]=false;
	this[loadingWidget].hide();
}
Loading.conf() {
	tag[Width]=550, tag[Height]=550;
	confNodeLayout(tag);
	this[pageStart]=true;
	this.showLoading();
}
Loading.draw(draw, timeline) {
	draw.effect(
		DRAW.RoundBox, tag[rect].incr(2), 15, '#cacaca', '#ffffffff', 2
	);
	rcStatus=tag[rect].move('bottom', 150);
	draw.font(16, 'bold', '#30303a');
	draw.text(rcStatus, "결제처리중입니다. 잠시만 기다려주세요\n\nProcessing ...", 'center');
	this.showLoading(tag[rect]);
}
Loading.initControl() {
	tag[Width]=550, tag[Height]=550;
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
}
Loading.mouseDown(pos) {

}
Loading.mouseUp(pos) {

}
Loading.showLoading(rc) {
	widget=tag[loadingWidget];
	main=this.mainControl();
	canvas=main.canvas;
	not( widget ) {
		widget=canvas.widget({tag:canvas});
		widget.flags('splash, top');
		widget.playGif("$cf[imagePath]/main/loading2.gif");
		tag[loadingWidget]=widget;
	}
	rcGlobal=canvas.mapGlobal(rc.center(300,300));
	widget.move(rcGlobal.lt());
	widget.size(rcGlobal.size());
	widget.show();
	return lw;
}
Loading.popupCloseEvent() {
	tag[loadingWidget].hide();
}
Loading.test() {
	this.mainControl().popupOpen('Loading');
}

SaleCloseConfirm.reprint(main, setup, db) {
	db.fetch("select to_char(reg_open_dtm,'yyyymmddHH24mi') as open_dtm , to_char(reg_close_dtm,'yyyymmddHH24mi') as close_dtm from kiosk_open_close where open_date=#{open_date}", setup);
	qrmonNode = _node('QtMonNode');
	qrmonNode[data] = null;
	s = '21,11,29,';
	/*총매출액 ,0 ,카드결재,기타,취소매출,0,카드결재,실매출액,0,카드결재,공급가액,부가세액,주문건수,취소건수,0,*/
	/*현금 관련 전문*/
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"),  setup);
	s.add("$setup[ms_nm],$setup[biz_no],$setup[open_date],$setup[pos_no],$setup[master_nm],$setup[open_dtm],$setup[close_dtm],");
	/*총 매출*/
	db.fetch("select sum(total_amt) as sale_total , count(*) as sale_count  from tb_sale_header where open_date = #{open_date}", setup);
	s.add("$setup[sale_total],0,$setup[sale_total],0,");
	/*취소 매출*/
	db.fetch("select sum(total_amt) as cancel_total, count(*) as cancel_count  from tb_sale_header where cancle_yn = 'Y' and open_date = #{open_date}", setup);
	s.add("$setup[cancel_total],0,$setup[cancel_total],");
	print("setup ------------------------------------===> $setup");
	/*실 매출*/
	db.fetch(conf("sql#kiosk.hitec#saleAmt"), setup);
	s.add("$setup[sale_real_total],0,$setup[sale_real_total],$setup[kong_sum_amt],$setup[bu_sum_amt],$setup[sale_count],$setup[cancel_count],");
	s.add('0^0^0^0^0	,0^0^0^0^0	,0^0^0^0	,0^0^0^0^0	,0^0^0^0	,0,0,0');
	main._log("# 마감 전표 == $s");
	main.qtMonSendData(s.kr() );
	while(n,10){
		System.sleep(1000);
		if(qrmonNode[data]) break;
	}
}
SaleCloseConfirm.saveSale(db) {
	dataNode=this.dataNode;
	openDate=dataNode[open_date];
	db.fetchAll("select * from tb_sale_header where open_date=#{open_date}", dataNode.removeAll() );
	file=Class.file('test');
	s='';
	while( cur, dataNode ) {
		s.add("$cur\n");
	}
	file.writeAll("data/test/${openDate}_header.txt", s);
	db.fetchAll("select * from tb_sale_detail where open_date=#{open_date}", dataNode.removeAll() );
	s='';
	while( cur, dataNode ) {
		s.add("$cur\n");
	}
	file.writeAll("data/test/${openDate}_detail.txt", s);
}

mainCanvas.confMain() {
	ok=true;
	cur=this.findTag('CornerTab');
	arr=cur[Margin], size=arr.size();
	not( size.eq(4) ) {
		ok=false;
	}
	not( arr[1].eq(210) ) {
		cur[Margin]=[0,210,0,0];
		ok=false;
	}
	cur=this.findTag('MainTitle');
	while( sub, cur ) {
		switch(sub[tag]) {
		case HomeButton:
			sub[Margin].inject(x,y);
			not( x.eq(980) ) {
				sub[Margin]=[980,130,20,0];
				ok=false;
			}
		case LanguageSelect:
			sub[Margin].inject(x,y);
			not( x.eq(20)  ) {
				sub[Margin]=[20,130];
				ok=false;
			}
		default:
		}
	}
	cur=this.findTag('MainStatus');
	while( sub, cur ) {
		arr=sub[Margin];
		switch(sub[tag]) {
		case OrderInfo:
			sub[Margin].inject(x,y);
			not( x.eq(20) ) {
				sub[Margin]=[20,14,40,26];
				ok=false;
			}
		case MainButtons:
			sub[rect].inject(x,y);
			not( x.eq(525) ) {
				sub[rect]=Class.rect(525,1770,555,150);
				this.getControl(sub).conf();
			}
		default:
		}
	}
	not( ok ) {
		print("######################### 그리기 영역 오류 #########################");
		printNode( this[mainNode], 0);
		this.conf();
	}
}

LoginView.search() {
	loginInfo=_node('LoginInfo');
	loginInfo[loginStartTick]=0;
	cur=this.findTag('UserNameLabel');
	cf[inputNode]=cur;
	cf[inputFocusRect]=cur[rect];
}

ExitButtons.ExitButtons(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
ExitButtons.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag: TopTitle});
	tag.addNode({tag: Body});
	tag.addNode({tag: Status});
}
ExitButtons.conf() {
	cf.inject(imagePath);
	tag[rect].inject(sx, sy, sw, sh);
	cw=sw-60;
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			cx=sx, sh=99;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		case Body:
			cx=sx, sh=tag[ContentHeight];		not( sh ) sh=450;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
			_setCodeData( tag[CodeNode], cur );
			rectRateArray( cur[rect 2], '3,3,3' ).inject(a,b,c);
			cur[rcClose]				=a.center(290, 86);
			cur[rcReBoot]			=b.center(290, 86);
			cur[rcShotDown]		=c.center(290, 86);
		case Status:
			cx=sx, sh=40;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		}
		cur[rect]=Class.rect(sx, sy, sw, sh),	sy+=sh;
	}
}
ExitButtons.draw(draw, timeline) {
	setDrawOpacity(draw, timeline);
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box1$idx") );
			}
			drawNodeText(draw, cur[rect 2], "시스템 종료 선택", 'left', 32, '#f0f0f0');
		case Body:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box2$idx") );
			}
			rc=cur[rcClose], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_cancel',ty) );
			drawNodeText(draw, rc, "닫기", "center", 'PopupButton');
			rc=cur[rcReBoot], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "재시작", "center", 'PopupButton');
			rc=cur[rcShotDown], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "PC종료", "center", 'PopupButton');
		case Status:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box3$idx") );
			}
		}
	}
}
ExitButtons.mouseDown(pos) {
	cur=this.findTag('Body');
	if( cur[rcClose].contains(pos) ) {
		this.mouseDownRect=cur[rcClose];
	} else if( cur[rcReBoot].contains(pos) ) {
		this.mouseDownRect=cur[rcReBoot];
	} else if( cur[rcShotDown].contains(pos) ) {
		this.mouseDownRect=cur[rcShotDown];
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
ExitButtons.mouseUp(pos) {
	cur=this.findTag('Body');
	if( cur[rcClose].contains(pos) ) {
		this.findControl('Popup#dialog').popupClose();
	} else if( cur[rcReBoot].contains(pos) ) {
		System.timeout(500);
		System.reboot(true);
	} else if( cur[rcShotDown].contains(pos) ) {
		System.timeout(500);
		System.shutdown(true);
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
ExitButtons.mouseMove(pos) {

}

SaleCloseConfirm.drawFadeIn(draw, timeline) {
	popupFadeIn(draw, timeline);
}

SystemMenu.SystemMenu(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SystemMenu.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag: TopTitle});
	tag.addNode({tag: Body});
	tag.addNode({tag: Status});
}
SystemMenu.conf() {
	cf.inject(imagePath);
	tag[rect].inject(sx, sy, sw, sh);
	cw=sw-60;
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			cx=sx, sh=99;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		case Body:
			cx=sx, sh=tag[ContentHeight];		not( sh ) sh=450;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
			_setCodeData( tag[CodeNode], cur );
			rc=cur[rect 2].height(200);
			rectRateArray( rc, '3,3,3' ).inject(a,b,c);
			cur[rcAdmin]				=a.center(290, 86);
			cur[rcAlert]				=b.center(290, 86);
			cur[rcAlertClose]		=c.center(290, 86);
			rc=mergeRect(cur[rcAdmin], cur[rcAlertClose]);
			cur[rcAppClose]		=rc.move('down', 35);
		case Status:
			cx=sx, sh=40;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		}
		cur[rect]=Class.rect(sx, sy, sw, sh),	sy+=sh;
	}
	not( tag[CloseButton] ) {
		cf.inject(imagePath);
		tag[CloseButton] ="${imagePath}/Common/popup_close_[#].png";
	}
	img=imageLoad(tag, 'CloseButton', 'n');
	img.imageSize().inject(w,h);
	cur=this.findTag('TopTitle');
	rc=cur[rect].move('end', 80);
	tag[rcCloseButton]=rc.center(w,h);
}
SystemMenu.draw(draw, timeline) {
	setDrawOpacity(draw, timeline);
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box1$idx") );
			}
			drawNodeText(draw, cur[rect 2], "시스템 메뉴", 'left', 32, '#f0f0f0');
			drawNodeImage(draw, tag[rcCloseButton], tag, 'CloseButton', 'n', true);
		case Body:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box2$idx") );
			}
			rc=cur[rcAdmin], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "관리자 화면", "center", 'PopupButton');
			rc=cur[rcAlert], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "점검창 열기", "center", 'PopupButton');
			rc=cur[rcAlertClose], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "점검창 닫기", "center", 'PopupButton');
			rc=cur[rcAppClose], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.fill(rc,'#a0a0a0').rectLine(rc, 0, '#808080', 4);
			drawNodeText(draw, rc, "키오스크 프로그램 닫기", "center", 'PopupButton');
			rc= cur[rcAppClose].move('down', 50).height(100);
			draw.fill(rc,'#ffffff').rectLine(rc, 0, '#c0c0c0', 1);
			draw.font(16,'normal','#10101a').text(rc,"키오스크 시작시간 : $cf[kioskStartDtm]", "center");
		case Status:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box3$idx") );
			}
		}
	}
}
SystemMenu.mouseDown(pos) {
	cur=this.findTag('Body');
	if( cur[rcAdmin].contains(pos) ) {
		this.mouseDownRect=cur[rcAdmin];
	} else if( cur[rcAlert].contains(pos) ) {
		this.mouseDownRect=cur[rcAlert];
	} else if( cur[rcAlertClose].contains(pos) ) {
		this.mouseDownRect=cur[rcAlertClose];
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
SystemMenu.mouseUp(pos) {
	main=this.mainControl();
	if( tag[rcCloseButton].contains(pos) ) {
		main.popupClose();
	}
	cur=this.findTag('Body');
	if( cur[rcAdmin].contains(pos) ) {
		print("관리자 화면가기");
		if( System.processCheck('KioskWatcher.exe') ) {
			Class.web('admin').call('http://localhost:8089/@kiosk.Common.WatcherOpen');
		} else {
			System.run("KioskWatcher.exe");
		}
	} else if( cur[rcAlert].contains(pos) ) {
		print("점검창 열기");
		Class.db('kiosk_hitec').exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status, tm) values( 'notify', 'admin', '점검', '잠시 정검중입니다. 조금만 기다려 주세요', 'R', 0 )" );
		main.alert("잠시 정검중입니다. 조금만 기다려 주세요","정검", true);
	} else if( cur[rcAlertClose].contains(pos) ) {
		print("점검창 닫기");
		not( System.processCheck('KioskWatcher.exe') ) {
			System.run("KioskWatcher.exe");
		}
		cf[errorOpen]=false;
		main.popupClose();
		Class.db('kiosk_hitec').exec("update kiosk_error set error_status='S' where error_status='R'");
	} else if( cur[rcAppClose].contains(pos) ) {
		print("키오스크 종료");
		main.closeKiosk();
		Cf.exit();
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}
SystemMenu.mouseMove(pos) {

}
SystemMenu.a() {
	this.conf();
	this.findControl('Popup#dialog').popupOpen('SystemMenu', 'popup');
}

OrderConfirm.confList(cur) {
	cur[rect].inject(sx, sy, sw, sh);
	arr=_arr(this,'TempRate').recalc(sw, '*,125,185,100', true);
	/* 헤더 영역 */
	cx=sx, sh=80;
	while( cw, arr, c, 0 ) {
		rcCell=Class.rect(cx, sy, cw, sh), cx+=cw;
		switch(c) {
		case 0: cur[rcMenuOption]=rcCell;
		case 1: cur[rcQty]=rcCell;
		case 2: cur[rcPrice]=rcCell;
		case 3: cur[rcDelete]=rcCell;
		}
	}
	/* 리스트 영역 */
	sy+=80, sh=70;
	sp=this.startRow, ep=sp+10;
	while( n, ep, sp ) {
		sub=cur.child(n);
		not( sub ) {
			break;
		}
		cx=sx;
		sub[rect]=Class.rect(cx, sy, sw, sh);
		while( cw, arr, c, 0 ) {
			rcCell=Class.rect(cx, sy, cw, sh), cx+=cw;
			switch(c) {
			case 0: sub[rcMenuOption]=rcCell;
			case 1: sub[rcQty]=rcCell;
			case 2: sub[rcPrice]=rcCell;
			case 3: sub[rcDelete]=rcCell;
				img=imageLoad(tag,'OrderDeleteButton', 'n');
				img.imageSize().inject(w,h);
				sub[rcDeleteButton]=sub[rcDelete].center(w,h);
			default:
			}
		}
		sy+=sh;
	}
	rc=cur[rect];
	cur[rcStatus]=rc.move('bottom',72);
	rectRateArray(cur[rcStatus],'*, 55, 12, 55,26').inject(space, rcUp, space, rcDown);
	print("주문확인창 : ($sp, $ep, $cur[rcStatus])");
	cur.put( rcUp, rcDown);
}
OrderConfirm.initPage() {
	this.startRow=0;
	orderList=this.findControl('MenuCart#orderView').getOrderList();
	not( orderList.childCount() ) {
		this.mainControl().closePopup();
		return;
	}
	cur=this.findTag('OrderList');
	cur.removeAll();
	cur[rcStatus]=null;
	while( row, orderList ) {
		cur.addNode().initNode(row);
	}
}

SaleExitButtons.SaleExitButtons(tag, parentCtrl) {
	parentCtrl.inject(db, cf, xmlNode);
	this.addClass('common/control.PageBase');
	this.initControl();
}
SaleExitButtons.conf() {
	cf.inject(imagePath);
	tag[rect].inject(sx, sy, sw, sh);
	cw=sw-60;
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			cx=sx, sh=99;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		case Body:
			cx=sx, sh=tag[ContentHeight];		not( sh ) sh=450;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
			_setCodeData( tag[CodeNode], cur );
			rectRateArray( cur[rect 2], '4,4' ).inject(a,b);
			cur[rcReBoot]			=a.center(290, 86);
			cur[rcShotDown]		=b.center(290, 86);
		case Status:
			cx=sx, sh=40;
			cur[rect 1]=Class.rect(cx,sy,30,sh), 	cx+=30;
			cur[rect 2]=Class.rect(cx,sy,cw,sh), 	cx+=cw;
			cur[rect 3]=Class.rect(cx,sy,30,sh);
		}
		cur[rect]=Class.rect(sx, sy, sw, sh),	sy+=sh;
	}
}
SaleExitButtons.draw(draw, timeline) {
	tag[autoClose]=false;
	opener=tag[openerControl];
	info=opener[dataNode];
	setDrawOpacity(draw, timeline);
	drawNodeStyle(draw, tag);
	while( cur, tag ) {
		switch(cur[tag]) {
		case TopTitle:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box1$idx") );
			}
			openDay=util_formatDate( info[open_date] );
			draw.font(22, 'normal', '#f0f0f0');
			draw.text( cur[rect 2], "${openDay} 업무마감 시스템 종료선택");
		case Body:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box2$idx") );
			}
			rc=cur[rect].height(120);
			draw.font(22,'bold','#30303a');
			draw.text(rc.incr(20), "업무마감후 재시작 또는 PC종료 하세요", "center");
			rc=cur[rcReBoot], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "재시작", "center", 'PopupButton');
			rc=cur[rcShotDown], ty=when( rc.eq(this.mouseDownRect), 'p', 'n');
			draw.drawImage( rc, commonImage('popup_confirm',ty) );
			drawNodeText(draw, rc, "PC종료", "center", 'PopupButton');
		case Status:
			while( col, 3 ) {
				idx=col+1;
				draw.drawImage(cur[rect $idx], commonImage("popup_box3$idx") );
			}
		}
	}
}
SaleExitButtons.initControl() {
	not( tag[type] ) tag[type]='vbox';
	setNodeSize(tag, true);
	tag.removeAll();
	tag.addNode({tag: TopTitle});
	tag.addNode({tag: Body});
	tag.addNode({tag: Status});
}
SaleExitButtons.mouseDown(pos) {
	cur=this.findTag('Body');
	if( cur[rcReBoot].contains(pos) ) {
		this.mouseDownRect=cur[rcReBoot];
	} else if( cur[rcShotDown].contains(pos) ) {
		this.mouseDownRect=cur[rcShotDown];
	}
	if( this.mouseDownRect ) {
		this.update();
	}
}
SaleExitButtons.mouseMove(pos) {

}
SaleExitButtons.mouseUp(pos) {
	cur=this.findTag('Body');
	if( cur[rcReBoot].contains(pos) ) {
		System.timeout(500);
		System.reboot(true);
	} else if( cur[rcShotDown].contains(pos) ) {
		System.timeout(500);
		System.shutdown(true);
	}
	if( this.mouseDownRect ) {
		this.mouseDownRect=null;
		this.update();
	}
}

SaleCloseConfirm.saleReSend(db, node) {
	print("매출 재전송 시작");
	setup = _node(tag,'ReSendNode');
	setup.initNode(node);
	db.fetch(conf("sql#kiosk.hitec#StoreInfo"),  setup);
	sql="SELECT
	sale_seq, ms_no, open_date, pos_no, bill_no, deal_no, detail_cnt, slip_cnt, total_amt, sale_amt, vat_amt, to_char(reg_dt,'yyyymmddhh24miss') as datetime
	FROM tb_sale_header  where open_date=#{open_date} and send_yn<>'Y' and cancle_yn<>'Y' ";
	sqlDetail="SELECT
	detail_index,  corner_cd, menu_cd, price, qty, total_amt, sale_amt
	FROM tb_sale_detail where sale_seq=#{sale_seq}";
	sqlPayment="SELECT
	 arv_dt, trdata1, trdata5 as pur_card_no, trdata9 as appr_no, trdata11 as pur_card_nm, trdata13 as res_no
	FROM tb_sale_payment where sale_seq=#{sale_seq}";
	sqlVan="select
	van_cd
	from hitec_m60s
	where
	ms_no = #{ms_no} and pos_no = #{pos_no}
	order by log_seq desc
	limit 1 offset 0";
	sqlCardCd="select
	std_card_cdrint_no,van_card_cd
	FROM hitec_m23s
	where van_cd = #{van_cd}
	   and van_card_cd = #{pur_card_no}
	   and use_yn = 'Y'
	order by log_seq desc
	limit 1 offset 0";
	db.fetchAll(sql, setup.removeAll() );
	headerXml='<HEADER SALE_DATE="${open_date}" MS_NO="${ms_no}" POS_NO="${pos_no}" BILL_NO="${bill_no}" REST_CD="${rest_cd}" OPER_CD="${oper_cd}" BR_CD="${br_cd}" SALE_FG="0"
	DATETIME="${datetime}" SALE_TOT="${total_amt}"  SALE_AMT="${sale_amt}" CASH_AMT="0" CARD_AMT="${total_amt}" ETC_AMT="0" DC_AMT="0" DETAIL_CNT="${detail_cnt}" TABLE_NO="${deal_no}" SLIP_CNT="1" ORG_BILL_NO="">';
	detailXml='<DETAIL LINE_NO="${detail_index}" CLASS_CD="${corner_cd}" GOODS_CD="${menu_cd}" UPRICE="${price}" SALE_QTY="${qty}" SALE_TOT="${total_amt}" SALE_AMT="${sale_amt}" DC_AMT="0" />';
	cardXml='<SLIP>
	<CARD SEQ="01" CARD_NO="${card_no}******" INPUT_FG="0" APPR_AMT="${card_total_amt}" APPR_NO="${appr_no}" APPR_DATE="20${appr_date}" APPR_TIME="${appr_time}" VALID_TERM="202108" INST_MCNT="00" CARD_CD="${std_card_cdrint_no}" VAN_CD="${van_cd}" PUR_CARD_CD="${pur_card_no}" PUR_CARD_NM="${pur_card_nm}" /></SLIP></HEADER>
	</DRIM-RH2>';
	db.fetch(sqlVan, setup);
	setup.inject(open_date, ms_no, pos_no, rest_cd, oper_cd, br_cd, van_cd);
	print(open_date, ms_no, pos_no, rest_cd, oper_cd, van_cd);
	req={  method:'POST', header: {} };
	while( cur, setup, num, 0 ) {
		cur.inject( bill_no, datetime, total_amt, sale_amt, detail_cnt, deal_no);
		card_total_amt = total_amt;
		_header=fmt(headerXml);
		xml='<?xml version="1.0" encoding="euc-kr" ?><DRIM-RH2><TELEX-HD TELEX_ID="A10R" MSG_CD="0000" />';
		xml.add("\n$_header");
		db.fetchAll(sqlDetail, cur.removeAll() );
		while( detail, cur ) {
			detail.inject(detail_index,  corner_cd, menu_cd, price, qty, total_amt, sale_amt);
			_detail=fmt(detailXml);
			xml.add("\n$_detail");
		}
		cur[van_cd]=van_cd;
		db.fetch(sqlPayment, cur );
		db.fetch(sqlCardCd, cur);
		cur.inject(arv_dt, pur_card_no, appr_no, pur_card_nm, std_card_cdrint_no);
		card_no=cur[trdata1].value(0,6);
		appr_date=arv_dt.value(0,6);
		appr_time=arv_dt.value(6,12);
		_card=fmt(cardXml);
		xml.add("\n$_card");
		print("#매출전문 : $xml");
		web=Class.web('kiosk1');
		web[data] = xml.kr();
		req[url]='http://61.78.39.134/telex_rh2/A10_Rcv.php';
			web.call( req, callback(type,data) {
			switch(type) {
				case read:
					data = data.utf8();
					db.exec("update tb_sale_header set send_yn='Y' where sale_seq=#{sale_seq}", cur);
					print("# 카드결제 전문 응답 =>$data ");
				case error:
					print("## 카드결제 전문 응답오류 =>$data",true);
			}
		});
	}
	print("매출 재전송 시작 종료 전송건수($num) ");
}

PageFuncsGrid.okClick() {
	root=grid.rootNode();
	cmsCode=currentPage[@cms.code], pageCode=currentPage[id];
	root[currentPage]=currentPage;
	root.put(cmsCode, pageCode);
	fc=getParentFunc(page, 'addPageFuncsEdit');
	fc(root);
}

MainButtons.orderCancel(msg) {
	cf[cardButtonCheck]=false;
	this.mainControl().goHome();
	if( msg ) {
		System.timeout(500);
		this.mainControl().alert(msg,"알림");
	}
	return;
}

CornerTab.setTabMode(mode) {
	cf[tabMode]=mode;
	this.conf();
	tabDrawNode=tag[tabDrawNode];
	tabDrawNode.delete();
	tag[tabDrawNode]=null;
	this.makeDisplayTab();
	canvas.redraw();
}

mainCanvas.setTabMode(mode) {
	cf[tabMode]=mode;
	this.conf();
	tabDrawNode=tag[tabDrawNode];
	tabDrawNode.delete();
	tag[tabDrawNode]=null;
	this.makeDisplayTab();
	this.redraw();
}

CardImage.CardImage(tag, parentCtrl) {
	parentCtrl.inject(db, cf);
	this.addClass('common/control.PageBase');
	this.initControl();
}
CardImage.initControl() {
	cf.inject( imagePath);
	imageLoad(tag, 'bg', "${imagePath}/main/card_help.png");
}
CardImage.conf() {
	tag[rcTitle]				=tag[rect].height(122);
	tag[rcMsg]				=tag[rect].incrY(150).height(122);
	tag[rcCardRead]		=tag[rect].incrY(635).height(60);
	tag[rcCardMsr]		=tag[rect].incrY(1005).height(60);
}
CardImage.draw(draw, timeline) {
	rc=tag[rect];
	img=	imageLoad(tag, 'bg' );
	draw.drawImage( rc, img);
	if( Cf[KioskLangSelect].eq('Kor') ) {
		draw.font(34,'bold','#f0f0f0',"나눔바른고딕");
		draw.text(tag[rcTitle], "신용카드 결제알림", "center");
		draw.font(25,'normal','#f0404a',"나눔바른고딕");
		draw.text(tag[rcMsg], "결제가 완료될 때까지\n카드를 빼지 마세요!", "center");
		draw.font(20,'normal','#30303a',"나눔바른고딕");
		draw.text(tag[rcCardRead], "신용카드를 그림과 같이 IC카드 리더기에 꽃아주세요", "center");
		draw.text(tag[rcCardMsr].incrY(10), "IC 카드 결제 오류시 마그네틱이 위로 향하게 읽혀주세요", "center");
	} else {
		draw.font(34,'bold','#f0f0f0',"나눔바른고딕");
		draw.text(tag[rcTitle], "Credit Card Payment", "center");
		draw.font(25,'normal','#f0404a',"나눔바른고딕");
		draw.text(tag[rcMsg], "Do not remove your card\nuntil the payment is completed!", "center");
		rcError=tag[rcCardMsr].incrY(-40);
		draw.font(20,'normal','#30303a',"나눔바른고딕");
		draw.text(tag[rcCardRead], "Please insert your card into the card reader as shown in the picture.", "center");
		draw.text(rcError.incrH(50), "If the IC card payment error,\n please read the card with the magnetic stripe facing up.", "center");
	}
	draw.rectLine( rc, 0, '#463E3C', 2);
}
CardImage.test() {
	this.mainControl().popupOpen('CardImage');
}

CornerTab.setBestTabNode(reset) {
	not( tag[tabBestNode].childCount() ) {
		cf[lastLogSeq]=null;
	}
	seq=db.value("SELECT max(log_seq) as log_seq FROM hitec_m10s");
	if( seq.eq(cf[lastLogSeq]) ) {
		print("## setBestTabNode : 메뉴변경 내용없읍");
		not( tag[currentTabBest] ) {
			print("# 추천탭으로 이동");
			this.findControl('MenuList#menuView').changeTab(tag[tabBestNode]);
		}
		tag[currentTabBest]=true;
		tag[currentTab]=null;
		tag[currentTabBlock]=0;
		tag[currentTabIndex]=0;
		this.update();
		return;
	}
	cf[lastLogSeq]=seq;
	dow=System.date('dayOfWeek');
	day=null;
	switch( dow ) {
	case 1:  day="and view_mon_yn='Y' ";
	case 2:  day="and view_tue_yn='Y' ";
	case 3:  day="and view_wed_yn='Y' ";
	case 4:  day="and view_thu_yn='Y' ";
	case 5:  day="and view_fri_yn='Y' ";
	case 6:  day="and view_sat_yn='Y' ";
	case 7:  day="and view_sun_yn='Y' ";
	}
	sql="select
	'menu' as tag, corner_cd, menu_cd, menu_nm, menu_nm as kor, eng, jpn, cha, goods_img, sale_price, 1 as sale_ok, sold_yn, rec_menu_seq
	from (
	select
		clplu_cd as corner_cd,
		goods_cd as menu_cd,
		goods_nm as menu_nm,
		eng_nm as eng, jp_nm as jpn, cn_nm_gan as cha,
		goods_img,
		uprice as sale_price,
		rec_menu_seq,
		sold_yn
	from
		hitec_m10s
	where
		1=1 ${day} and rec_menu_yn ='Y' and use_yn='Y'  and view_yn='Y'
	) X
	order by sold_yn, rec_menu_seq";
	tag[currentTabBest]=true;
	tag[currentTab]=null;
	tag[currentTabBlock]=0;
	tag[currentTabIndex]=0;
	db.fetchAll(sql, tag[tabBestNode].removeAll() );
	this.findControl('MenuList#menuView').conf();
	this.mainControl().reloadCornerMenu();
	this.update();
}

mainCanvas.reloadCornerMenu(tab) {
	if( this[changeConerTick] ) {
		dist=System.tick() - this[changeConerTick];
		if( dist<5000 ) {
			return;
		}
	}
	this[changeConerTick]=System.tick();
	tabs=this.findTag('#CornerTab');
	while( cur, tabs ) {
		dow=System.date('dayOfWeek');
		switch( dow ) {
		case 1:  day=" when view_mon_yn='Y' ";
		case 2:  day=" when view_tue_yn='Y' ";
		case 3:  day=" when view_wed_yn='Y' ";
		case 4:  day=" when view_thu_yn='Y' ";
		case 5:  day=" when view_fri_yn='Y' ";
		case 6:  day=" when view_sat_yn='Y' ";
		case 7:  day=" when view_sun_yn='Y' ";
		}
		cur[time]=System.date('HHmm');
		sql=fmt(conf('sql#hitec.menuList') );
		db.fetchAll(sql, cur.removeAll() );
		while( menu, cur ) {
			menu.inject(day_ok, time_ok);
			if( day_ok.eq('1') && time_ok.eq('1') ) {
				menu[sale_ok]=true;
			}
		}
	}
}

GridControl.addClick() {
	parentCtrl.addClick(this.dataNode, this );
}

KioskInfoPage.addClick(root, grid) {
	main=this.mainControl();
	db=Class.db('kiosk_hitec');
	insert=getQuery('kiosk_print_setup', 'print_no' );
	db.fetch("SELECT COALESCE(MAX(print_no::integer), 0) + 1 AS print_no FROM kiosk_print_setup", cf);
	db.exec(insert, cf);
	if( db.error() ) {
		err=db.error();
		main[page].alert("매장코너 적용중 오류가 발생했습니다. errror: $err");
		return;
	}
	this.search();
}

TransDataControl.TransDataControl(parentControl, canvas) {
	cf={};
	_json=callback(url, dataNode) {
		Class.web('tros').call(url, callback(type, data) {
			parentControl._log("$type : $data");
			switch( type ) {
			case read:	dataNode.removeAll().parseJson(data.ref());
			case finish:	this.masterDbInsert(dataNode);
			case error:	dataNode[error]=data;
			}
		});
	};
	_post=callback(node) {
		node.method='post';
		Class.web('tros').call(node, callback(type, data) {
			parentControl._log("$type : $data");
			switch( type ) {
			case finish:	this.postFinish(node);
			case error:	cf[error]=data;
			}
		});
	};
	db=Class.db('namzatang_local');
}
TransDataControl.masterDownload(modifyDate) {
	// host='http://namzatang.happy-mate.co.kr';
	// URL : http://tros.sportsbridge.co.kr:18008
	host='http://tros.sportsbridge.co.kr:18008';
	service="$host/work/api/kiosk/cli.masterdown.asp";
	date=System.date('yyyyMMdd');
	modify_date =modifyDate;
	api_id='tros';
	store_no='1001';
	code="${api_id}${store_no}${date}";
	api_key=code.encode('sha256');
	url="$service?api_id=${api_id}&api_key=${api_key}&store_no=${store_no}&date=${date}";
	if( modifyDate ) {
		url.add("&modifyDate=${modifyDate}");
	}
	dataNode=Class.model('masterDownload').rootNode();
	_json(url, dataNode.initNode() );
}
TransDataControl.send(cur) {
	url='http://namzatang.happy-mate.co.kr/work/api/kiosk/cli.masterdown.asp';
	type='xxx';
	cur.put(url, type);
	param=_node(cur,'param');
	header=_node(cur,'header');
	header[ContentType] = "application/x-www-form-urlencoded; charset=UTF-8;"
	date=System.date('yyyyMMdd');
	api_id='tros';
	store_no='1001';
	code="${api_id}${store_no}${date}";
	api_key=code.encode('sha256');
	param.put(api_id, store_no,...);
	_post(cur);
}
TransDataControl.sendAll() {
	while( cur, cf ) {
	}
}
TransDataControl.upload() {
	db.fetchAll("", cf.removeAll() );
	sendAll();
}
TransDataControl.postFinish(node) {
	node.send_yn=true;
	this.sendAll();
}
TransDataControl.masterDbInsert(node) {
	cur=node[TB_STORE_MST];
	// db.exec("delete from TB_STORE_MST");
	if( cur[STORE_NO] ) {
		count=db.exec("UPDATE TB_STORE_MST SET STORE_NM=#{STORE_NM},BIZ_NO=#{BIZ_NO},BIZ_CAT=#{BIZ_CAT},BIZ_CON=#{BIZ_CON},OWNER_NM=#{OWNER_NM},TEL=#{TEL},ADDR1=#{ADDR1},ADDR2=#{ADDR2},OPEN_DATE=#{OPEN_DATE},CLOSE_DATE=#{CLOSE_DATE},CALL_TYPE=#{CALL_TYPE},LOCAL_IP=#{LOCAL_IP},LOCAL_PORT=#{LOCAL_PORT},LOCAL_PW=#{LOCAL_PW},KCN_PRT_TYPE=#{KCN_PRT_TYPE},USE_YN=#{USE_YN},FAX=#{FAX},CUS_PRT_YN=#{CUS_PRT_YN},BKR_STORE_NO=#{BKR_STORE_NO} WHERE STORE_NO=#{STORE_NO}", cur);
		not( count ) db.exec("INSERT INTO TB_STORE_MST (STORE_NO, STORE_NM, BIZ_NO, BIZ_CAT, BIZ_CON,OWNER_NM, TEL, ADDR1, ADDR2, OPEN_DATE,CLOSE_DATE, CALL_TYPE, LOCAL_IP, LOCAL_PORT, LOCAL_PW,KCN_PRT_TYPE, USE_YN, FAX, CUS_PRT_YN, BKR_STORE_NO ) VALUES (#{STORE_NO}, #{STORE_NM}, #{BIZ_NO}, #{BIZ_CAT}, #{BIZ_CON}, #{OWNER_NM}, #{TEL}, #{ADDR1}, #{ADDR2}, #{OPEN_DATE}, #{CLOSE_DATE}, #{CALL_TYPE}, #{LOCAL_IP}, #{LOCAL_PORT}, #{LOCAL_PW}, #{KCN_PRT_TYPE}, #{USE_YN}, #{FAX}, #{CUS_PRT_YN}, #{BKRSTORENO} )", cur);
	}
	if( this.dbError(@funcName, 'TB_STORE_MST') ) {
		return;
	}
	// db.exec("delete from TB_POS_MST");
	while( cur, node[TB_POS_MST] ) {
		count=db.exec("UPDATE TB_POS_MST SET BKR_POS_NO=#{BKR_POS_NO},MOBILE_YN=#{MOBILE_YN},VAN_CD=#{VAN_CD},VAN_NM=#{VAN_NM},VAN_IP=#{VAN_IP},VAN_PORT=#{VAN_PORT},VAN_DATA1=#{VAN_DATA1},VAN_DATA2=#{VAN_DATA2},VAN_DATA3=#{VAN_DATA3},VAN_DATA4=#{VAN_DATA4},VAN_DATA5=#{VAN_DATA5},VAN_TMNL_ID=#{VAN_TMNL_ID},CHANGE_NO_ST=#{CHANGE_NO_ST},CHANGE_NO_EN=#{CHANGE_NO_EN)} WHERE STORE_NO=#{STORE_NO} AND POS_NO=#{POS_NO}", cur);
		not( count ) db.exec("INSERT INTO TB_POS_MST (STORE_NO, POS_NO, BKR_POS_NO, MOBILE_YN, VAN_CD,VAN_NM, VAN_IP, VAN_PORT, VAN_DATA1, VAN_DATA2,VAN_DATA3, VAN_DATA4, VAN_DATA5, VAN_TMNL_ID, CHANGE_NO_ST,CHANGE_NO_EN) VALUES (#{STORE_NO}, #{POS_NO}, #{BKR_POS_NO}, #{MOBILE_YN}, #{VAN_CD}, #{VAN_NM}, #{VAN_IP}, #{VAN_PORT}, #{VAN_DATA1}, #{VAN_DATA2}, #{VAN_DATA3}, #{VAN_DATA4}, #{VAN_DATA5}, #{VAN_TMNL_ID}, #{CHANGE_NO_ST}, #{CHANGE_NO_EN} )",cur);
	}
	if( this.dbError(@funcName, 'TB_POS_MST') ) {
		return;
	}
	// db.exec("delete from TB_MENU_MST");
	while( cur, node[TB_MENU_MST] ) {
		count=db.exec("UPDATE TB_MENU_MST SET MENU_TYPE=#{MENU_TYPE},CLASS_CD=#{CLASS_CD},MENU_NM=#{MENU_NM},MENU_NM_CN=#{MENU_NM_CN},MENU_NM_EN=#{MENU_NM_EN},MENU_NM_JP=#{MENU_NM_JP},SALE_PRICE=#{SALE_PRICE},EVENT_PRICE=#{EVENT_PRICE},EVENT_ST=#{EVENT_ST},EVENT_EN=#{EVENT_EN},EVENT_NO=#{EVENT_NO},DISP_TYPE=#{DISP_TYPE},OPT_YN=#{OPT_YN},TAX_YN=#{TAX_YN},TAKEOUT_YN=#{TAKEOUT_YN},SALE_ST=#{SALE_ST},SALE_EN=#{SALE_EN},KCN_PRT_YN=#{KCN_PRT_YN},PRT1_YN=#{PRT1_YN},PRT2_YN=#{PRT2_YN},PRT3_YN=#{PRT3_YN},PRT4_YN=#{PRT4_YN},PRT5_YN=#{PRT5_YN},USE_YN=#{USE_YN},BKR_MENU_CD=#{BKR_MENU_CD},MENU_DE=#{MENU_DE},MENU_DE_CN=#{MENU_DE_CN},MENU_DE_EN=#{MENU_DE_EN},MENU_DE_JP=#{MENU_DE_JP},BKR_ITEM_TP=#{BKR_ITEM_TP},BKR_SET_YN=#{BKR_SET_YN},BKR_MENU_GB=#{BKR_MENU_GB},SIZEUP_YN=#{SIZEUP_YN},MENU_SIZEUP_CD=#{MENU_SIZEUP_CD},OPEN_TIME=#{OPEN_TIME},CLOSE_TIME=#{CLOSE_TIME},BUY_QTY=#{BUY_QTY} WHERE STORE_NO=#{STORE_NO} AND MENU_CD=#{MENU_CD}", cur);
		not( count ) db.exec("INSERT INTO TB_MENU_MST (STORE_NO, MENU_CD, MENU_TYPE, CLASS_CD, MENU_NM,MENU_NM_CN, MENU_NM_EN, MENU_NM_JP,SALE_PRICE, EVENT_PRICE, EVENT_ST, EVENT_EN, EVENT_NO,DISP_TYPE, OPT_YN, TAX_YN, TAKEOUT_YN, SALE_ST,SALE_EN, KCN_PRT_YN, PRT1_YN, PRT2_YN, PRT3_YN,PRT4_YN, PRT5_YN, USE_YN, BKR_MENU_CD, MENU_DE,MENU_DE_CN, MENU_DE_EN, MENU_DE_JP, BKR_ITEM_TP, BKR_SET_YN,BKR_MENU_GB, SIZEUP_YN, MENU_SIZEUP_CD, OPEN_TIME, CLOSE_TIME, BUY_QTY ) VALUES (#{STORE_NO}, #{MENU_CD}, #{MENU_TYPE}, #{CLASS_CD}, #{MENU_NM}, #{MENU_NM_CN}, #{MENU_NM_EN}, #{MENU_NM_JP}, #{SALE_PRICE}, #{EVENT_PRICE}, #{EVENT_ST}, #{EVENT_EN}, #{EVENT_NO}, #{DISP_TYPE}, #{OPT_YN}, #{TAX_YN}, #{TAKEOUT_YN}, #{SALE_ST}, #{SALE_EN}, #{KCN_PRT_YN}, #{PRT1_YN}, #{PRT2_YN}, #{PRT3_YN}, #{PRT4_YN}, #{PRT5_YN}, #{USE_YN}, #{BKR_MENU_CD}, #{MENU_DE}, #{MENU_DE_CN}, #{MENU_DE_EN}, #{MENU_DE_JP}, #{BKR_ITEM_TP}, #{BKR_SET_YN}, #{BKR_MENU_GB}, #{SIZEUP_YN}, #{MENU_SIZEUP_CD}, #{OPEN_TIME}, #{CLOSE_TIME}, #{BUY_QTY} )",cur);
	}
	if( this.dbError(@funcName, 'TB_MENU_MST') ) {
		return;
	}
	// db.exec("delete from  TB_CORNER_MST");
	while( cur, node[TB_CORNER_MST] ) {
		count=db.exec("UPDATE TB_CORNER_MST SET CORNER_NM=#{CORNER_NM},DP_RANK=#{DP_RANK},USE_YN=#{USE_YN},CORNER_NM_CN=#{CORNER_NM_CN},CORNER_NM_EN=#{CORNER_NM_EN},CORNER_NM_JP=#{CORNER_NM_JP},VIEW_TYPE=#{VIEW_TYPE},OPEN_TIME=#{OPEN_TIME},CLOSE_TIME=#{CLOSE_TIME} WHERE STORE_NO=#{STORE_NO} AND CORNER_CD=#{CORNER_CD}", cur);
		not( count ) db.exec("INSERT INTO TB_CORNER_MST (STORE_NO, CORNER_CD, CORNER_NM, DP_RANK, USE_YN,CORNER_NM_CN, CORNER_NM_EN, CORNER_NM_JP, VIEW_TYPE, OPEN_TIME,CLOSE_TIME) VALUES ( #{STORE_NO}, #{CORNER_CD}, #{CORNER_NM}, #{DP_RANK}, #{USE_YN}, #{CORNER_NM_CN}, #{CORNER_NM_EN}, #{CORNER_NM_JP}, #{VIEW_TYPE}, #{OPEN_TIME}, #{CLOSE_TIME})",cur);
	}
	if( this.dbError(@funcName, 'TB_CORNER_MST') ) {
		return;
	}
	// db.exec("delete from  TB_CORNER_MENU");
	while( cur, node[TB_CORNER_MENU] ) {
		count=db.exec("UPDATE TB_CORNER_MENU SET DP_RANK=#{DP_RANK} WHERE STORE_NO=#{STORE_NO} AND CORNER_CD=#{CORNER_CD} AND MENU_CD=#{MENU_CD}", cur);
		not( count ) db.exec("INSERT INTO TB_CORNER_MENU (STORE_NO, CORNER_CD, MENU_CD, DP_RANK) VALUES (#{STORE_NO}, #{CORNER_CD}, #{MENU_CD}, #{DP_RANK} )",cur);
	}
	if( this.dbError(@funcName, 'TB_CORNER_MENU') ) {
		return;
	}
	while( cur, node[TB_DID_SCHEDULE] ) {
		count=db.exec("UPDATE TB_DID_SCHEDULE SET AD_TITLE=#{AD_TITLE},AD_GUBUN=#{AD_GUBUN},AD_CONTENTS_URL=#{AD_CONTENTS_URL},SDATE=#{SDATE},EDATE=#{EDATE},SOUND_CFC=#{SOUND_CFC},SEND_CFC=#{SEND_CFC},AD_RUN=#{AD_RUN},DP_RANK=#{DP_RANK} WHERE STORE_NO=#{STORE_NO} AND AD_CODE=#{AD_CODE}", cur);
		not( count ) db.exec("INSERT INTO TB_DID_SCHEDULE (STORE_NO, AD_CODE, AD_TITLE, AD_GUBUN, AD_CONTENTS_URL,SDATE, EDATE, SOUND_CFC, SEND_CFC, AD_RUN,DP_RANK) VALUES (#{STORE_NO}, #{AD_CODE}, #{AD_TITLE}, #{AD_GUBUN}, #{AD_CONTENTS_URL}, #{SDATE}, #{EDATE}, #{SOUND_CFC}, #{SEND_CFC}, #{AD_RUN}, #{DP_RANK} )",cur);
	}
	if( this.dbError(@funcName, 'TB_DID_SCHEDULE') ) {
		return;
	}
	// db.exec("delete from  TB_OPTION_GROUP");
	while( cur, node[TB_OPTION_GROUP] ) {
		count=db.exec("UPDATE TB_OPTION_GROUP SET OPT_GP_NM=#{OPT_GP_NM},REQUIRE_YN=#{REQUIRE_YN},MULTI_YN=#{MULTI_YN},OPT_GP_NM_CN=#{OPT_GP_NM_CN},OPT_GP_NM_EN=#{OPT_GP_NM_EN},OPT_GP_NM_JP=#{OPT_GP_NM_JP},MULTI_CNT=#{MULTI_CNT},OPT_CNT=#{OPT_CNT},USE_YN=#{USE_YN} WHERE STORE_NO=#{STORE_NO} AND OPT_GP_CD=#{OPT_GP_CD}", cur);
		not( count ) db.exec("INSERT INTO TB_OPTION_GROUP (STORE_NO, OPT_GP_CD, OPT_GP_NM, REQUIRE_YN, MULTI_YN,OPT_GP_NM_CN, OPT_GP_NM_EN, OPT_GP_NM_JP,MULTI_CNT, OPT_CNT, USE_YN) VALUES (#{STORE_NO}, #{OPT_GP_CD}, #{OPT_GP_NM}, #{REQUIRE_YN}, #{MULTI_YN}, #{OPT_GP_NM_CN}, #{OPT_GP_NM_EN}, #{OPT_GP_NM_JP}, #{MULTI_CNT}, #{OPT_CNT}, #{USE_YN} )",cur);
	}
	if( this.dbError(@funcName, 'TB_OPTION_GROUP') ) {
		return;
	}
	// db.exec("delete from  TB_PRT_RECEIPT");
	while( cur, node[TB_PRT_RECEIPT] ) {
		count=db.exec("UPDATE TB_PRT_RECEIPT SET MSG=#{MSG},ALIGN=#{ALIGN},PRT_FONT=#{PRT_FONT} WHERE STORE_NO=#{STORE_NO} AND POSITION=#{POSITION} AND LINE=#{LINE}",cur);
		not( count ) db.exec("INSERT INTO TB_PRT_RECEIPT (STORE_NO, POSITION, LINE, MSG, ALIGN,PRT_FONT) VALUES (#{STORE_NO}, #{POSITION}, #{LINE}, #{MSG}, #{ALIGN}, #{PRT_FONT} )",cur);
	}
	if( this.dbError(@funcName, 'TB_PRT_RECEIPT') ) {
		return;
	}
	// db.exec("delete from  TB_SETOPTION_MENU");
	while( cur, node[TB_SETOPTION_MENU] ) {
		count=db.exec("UPDATE TB_SETOPTION_MENU SET DP_RANK=#{DP_RANK} WHERE STORE_NO=#{STORE_NO} AND MENU_CD=#{MENU_CD} AND SETOPT_CD=#{SETOPT_CD}",cur);
		not( count ) db.exec("INSERT INTO TB_SETOPTION_MENU (STORE_NO, MENU_CD, SETOPT_CD, DP_RANK) VALUES (#{STORE_NO}, #{MENU_CD}, #{SETOPT_CD}, #{DP_RANK} )",cur);
	}
	if( this.dbError(@funcName, 'TB_SETOPTION_MENU') ) {
		return;
	}
	// db.exec("delete from  TB_SETOPTION_HEADER");
	while( cur, node[TB_SETOPTION_HEADER] ) {
		count=db.exec("UPDATE TB_SETOPTION_HEADER SET SETOPT_SNM=#{SETOPT_SNM},SETOPT_NM=#{SETOPT_NM},SETOPT_NM_EN=#{SETOPT_NM_EN},SETOPT_NM_JP=#{SETOPT_NM_JP},SETOPT_NM_CN=#{SETOPT_NM_CN},USE_YN) VALUES (#{STORE_NO}=#{USE_YN) VALUES (#{STORE_NO}},#{SETOPT_CD}=#{#{SETOPT_CD}},#{SETOPT_SNM}=#{#{SETOPT_SNM}},#{SETOPT_NM}=#{#{SETOPT_NM}},#{SETOPT_NM_EN}=#{#{SETOPT_NM_EN}},#{SETOPT_NM_JP=#{#{SETOPT_NM_JP} WHERE STORE_NO=#{STORE_NO} AND SETOPT_CD=#{SETOPT_CD}",cur);
		not( count ) db.exec("INSERT INTO TB_SETOPTION_HEADER (STORE_NO, SETOPT_CD, SETOPT_SNM, SETOPT_NM, SETOPT_NM_EN,SETOPT_NM_JP, SETOPT_NM_CN, USE_YN) VALUES (#{STORE_NO}, #{SETOPT_CD}, #{SETOPT_SNM}, #{SETOPT_NM}, #{SETOPT_NM_EN}, #{SETOPT_NM_JP}, #{SETOPT_NM_CN}, #{USE_YN} )",cur);
	}
	if( this.dbError(@funcName, 'TB_SETOPTION_HEADER') ) {
		return;
	}
	// db.exec("delete from  TB_SETOPTION_DETAIL");
	while( cur, node[TB_SETOPTION_DETAIL] ) {
		count=db.exec("UPDATE TB_SETOPTION_DETAIL SET DP_RANK=#{DP_RANK},BASIC_CK=#{BASIC_CK} WHERE STORE_NO=#{STORE_NO} AND SETOPT_CD=#{SETOPT_CD} AND MENU_CD=#{MENU_CD}", cur);
		not( count ) db.exec("INSERT INTO TB_SETOPTION_DETAIL (STORE_NO, SETOPT_CD, MENU_CD, DP_RANK, BASIC_CK) VALUES (#{STORE_NO}, #{SETOPT_CD}, #{MENU_CD}, #{DP_RANK}, #{BASIC_CK} )",cur);
	}
	if( this.dbError(@funcName, 'TB_SETOPTION_DETAIL') ) {
		return;
	}
	print(parentControl, canvas);
	this.log("Master Data Apply Ok !!!");
	canvas.postEvent(KIOSK.TRANS_DATA_OK, node);
	/*
	while( cur, node[TB_OPTION_SUB] ) db.exec("INSERT INTO TB_OPTION_SUB (STORE_NO, OPT_GP_CD, OPT_CD, OPT_NM, OPT_PRICE,OPT_NM_CN, OPT_NM_EN, OPT_NM_JP,DP_RANK, USE_YN) VALUES (#{STORE_NO}, #{OPT_GP_CD}, #{OPT_CD}, #{OPT_NM}, #{OPT_PRICE}, #{OPT_NM_CN}, #{OPT_NM_EN}, #{OPT_NM_JP}, #{DP_RANK}, #{USE_YN} )",cur);
	while( cur, node[TB_OPTION_MENU] ) db.exec("INSERT INTO TB_OPTION_MENU (STORE_NO, OPT_GP_CD, MENU_CD) VALUES (#{STORE_NO}, #{OPT_GP_CD}, #{MENU_CD} )",cur);
	while( cur, node[TB_MSG_MST] ) db.exec("INSERT INTO TB_MSG_MST (STORE_NO, MSG_NO, LINE, MSG_ST, MSG_EN,MSG, ALIGN, USE_YN, PRT_FONT) VALUES (#{STORE_NO}, #{MSG_NO}, #{LINE}, #{MSG_ST}, #{MSG_EN}, #{MSG}, #{ALIGN}, #{USE_YN}, #{PRT_FONT} )",cur);
	while( cur, node[TB_SETMAIN_DETAIL] ) db.exec("INSERT INTO TB_SETMAIN_DETAIL (STORE_NO, MENU_CD, SETMAIN_CD, DP_RANK, MENU_QTY) VALUES (#{STORE_NO}, #{MENU_CD}, #{SETMAIN_CD}, #{DP_RANK}, #{MENU_QTY} )",cur);
	*/
}
TransDataControl.dbError(funcName, tableName) {
	err=db.error();
	if( err ) {
		this.log("$funcName : $tableName error : $err");
		return true;
	}
	return false;
}
TransDataControl.log(msg) {
	print("#### log : $msg ###");
	this[logMessage]=msg;
	canvas.postEvent(KIOSK.Log, this);
}