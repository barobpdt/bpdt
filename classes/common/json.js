<script module="@json">
	listData(node, fields, listCode, skip) {
		/*
			checkField => 첫번째 자식노드 필드정보 : 자식노드 각각 필드정보
			skip => 노드 프로퍼티 무식 (리스트만 생성)
		*/
		rst='';
		not( listCode ) listCode='children';
		if( listCode=='none') {
			listCode=''
		} else if(node.get(listCode)) {
			listCode=''
		}
		fa=null, checkField=true;
		not(fields) fields=node.var(fields);
		if( fields ) {
			if( typeof(fields,'bool') ) {
				checkField=false;
			} else if( typeof(fields,'array') ) {
				fa=fields;
			} else {
				fa=fields.split(',');
			}
		} else {
			checkField=node.var(checkField);
		}
		if( skip ) {
			rst.add("[");
		} else {
			num=0;
			rst.add("{");
			if(node.var(propMode)) {
				checkField=false;
				while( key, node.keys() ) {
					if( key.ch('@') ) continue;
					val=node.get(key);
					if(typeof(val,'func','node','array')) continue;
					rst.add(Cf.jsValue(key), ':', Cf.jsValue(val) );
					num++;
				}
			} else {
				while( key, node.keys() ) {
					if( key.ch('@') ) continue;
					val=node.get(key);
					if(typeof(val,'null','func')) continue;
					if( num ) rst.add(',');
					chk=typeof(val,'num') || typeof(val,'bool') || val.eq('true','false')
					if(chk) {
						rst.add(Cf.jsValue(key), ':', val )
						continue;
					}
					if(typeof(val,'node')) {
						rst.add(Cf.jsValue(key), ':', this.nodeStr(val) );
					} else if(typeof(val,'array')) {
						rst.add(Cf.jsValue(key), ':', this.arrayStr(val) );
					} else {
						if( typeof(val,'num') || val.eq('true','false') ) {
							rst.add(Cf.jsValue(key), ':', val )
						} else {
							rst.add(Cf.jsValue(key), ':', Cf.jsValue(val) )
						}
					}
					num++;
				}
				if( typeof(fa,'array') ) {
					if( num ) rst.add(',');
					rst.add('"fields":[')
					while( key, fa, idx, 0 ) {
						if( idx ) rst.add(',');
						rst.add( Cf.jsValue(key) );
					}
					rst.add("]");
					num++;
				}
			}
			not( listCode ) {
				rst.add('}')
				return rst;
			} 
			if(num) rst.add(',');
			rst.add(' "',listCode,'":[');
		}
		
		idx=0;
		while( cur, node ) {
			if( idx ) rst.add(',');
			if(cur.var(useCheck)) continue;
			idx++;
			rst.add("{");
			propCheck=when(cur.tag, cur.tag.eq('object', 'array'));
			if(propCheck) {
				fieldArray=cur.keys();
			} else {
				fieldArray=fa;
				not(fieldArray) {
					fieldArray=cur.keys();
					if(checkField) fa=fieldArray;
				}
			}
			num=0;
			while( key, fieldArray ) {
				if( num ) rst.add(","); 
				val=cur.get(key);
				chk=typeof(val,'num') || typeof(val,'bool') || typeof(val,'null') || val.eq('true','false')
				if(chk) {
					rst.add(Cf.jsValue(key), ':', val )
					continue;
				}
				if(typeof(val,'node')) {
					rst.add(Cf.jsValue(key), ':', this.nodeStr(val) );
				} else if(typeof(val,'array')) {
					rst.add(Cf.jsValue(key), ':', this.arrayStr(val) );
				} else {
					rst.add(Cf.jsValue(key), ':', Cf.jsValue(val) )
				}
				num++;
			}
			if( cur.childCount() ) { 
				not(propCheck ) {
					if( num ) rst.add(",");
					rst.add(' "',listCode,'":', this.listData(cur, when(checkField,null,fa), listCode, true));
				}
			}
			rst.add("}");
		}
		if(skip) {
			rst.add("]");		
		} else {
			rst.add("]}");
		}
		return rst;
	}
	
	nodeStr(node, depth) {
		not(depth) depth=1;
		if(node.cmp('tag','array')) return this.arrayStr(node, depth);
		if(depth>4) return Cf.jsValue("node");
		rst='';
		rst.add("{");
		num=0, depth+=1;
		while( key, node.keys() ) {	
			if( key.ch('@') ) continue;
			if( key.eq('children') ) continue;
			val=node.get(key);
			if(typeof(val,'func')) continue;
			if( num ) rst.add(',');
			chk=typeof(val,'num') || typeof(val,'bool') || typeof(val,'null') || val.eq('true','false')
			if(chk) {
				rst.add(Cf.jsValue(key), ':', val )
				continue;
			}
			if(typeof(val,'node')) {
				rst.add(Cf.jsValue(key), ':', this.nodeStr(val,depth) );
			} else if(typeof(val,'array')) {
				rst.add(Cf.jsValue(key), ':', this.arrayStr(val,depth) );
			} else {
				rst.add(Cf.jsValue(key), ':', Cf.jsValue(val) )
			}
			num++; 
		}
		rst.add("}");
		return rst;
	}
	arrayStr(arr, depth) {
		not(depth) depth=1;
		if(depth>4) return Cf.jsValue("array");
		rst='';
		rst.add("[");
		num=0, depth+=1;
		while( val, arr ) {	
			if( key.ch('@') ) continue; 
			if(typeof(val,'func')) continue;
			if( num ) rst.add(',');
			chk=typeof(val,'num') || typeof(val,'bool') || typeof(val,'null') || val.eq('true','false')
			if(chk) {
				rst.add( val )
				continue;
			}
			if(typeof(val,'node')) {
				rst.add( this.nodeStr(val,depth) );
			} else if(typeof(val,'array')) {
				rst.add(  this.arrayStr(val,depth) );
			} else {
				rst.add( Cf.jsValue(val) )
			}
			num++; 
		}
		rst.add("]");
		return rst;
	}
	toString(node) {
		return this.nodeStr(node)
	} 
</script>