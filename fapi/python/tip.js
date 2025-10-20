/* update 
## 개별업데이트
from sqlalchemy import update
stmt = (
    update(user_table)
    .where(user_table.c.name == "patrick")
    .values(fullname="Patrick the Star")
)
print(stmt)

## 멀티업데이트
from sqlalchemy import bindparam
stmt = (
    update(user_table)
    .where(user_table.c.name == bindparam("oldname"))
    .values(name=bindparam("newname"))
)
with engine.begin() as conn:
    conn.execute(
        stmt,
        [
            {"oldname": "jack", "newname": "ed"},
            {"oldname": "wendy", "newname": "mary"},
            {"oldname": "jim", "newname": "jake"},
        ],
    )

## 환경변수 등록	
$Env:MY_NAME = "Wade Wilson"
echo "Hello $Env:MY_NAME"

# Mac, linux
export MY_NAME="Wade Wilson"

import os
name = os.getenv("MY_NAME", "World")
print(f"Hello {name} from Python")
*/


s=#[
]
~~ 
@baro.schemaMake(parent) {
	nl=conf('str.newline')
	projectName = parent.projectName
	not(projectName) projectName = 'test'

	while(table, parent) {
		table.inject(line, jsonData)
		if(jsonData) table.parseJson(jsonData)
		table.name = line.trim()
		not(table.tableName) {
			table.tableName = @baro.dbConvertName(table.name)
		}
		while(field, table) {
			@baro.schemaMakeField(table, field)
		}
		@baro.schemaApplySource(table)
	}
}
@baro.schemaMakeField(table, field) {
	
}
@baro.schemaApplySource(table) {
	ss=''
	while(field, table) {
		table.inject(line, jsonData,comment)
		ss.add("$line{$jsonData}$comment", nl)
	}
	key = _s('table#${projectName}.${table.name}')
	prev = conf(key)
	not( ss.eq(prev)) {
		conf(key, ss, true)
	}
}
@baro.dbConvertName(&s) {
	ss=''
	sz=s.size()
	while(n=0,sz ){
		c=s.ch(n)
		if(c.is('upper')) {
			if(n) ss.add('_')
		}
		ss.add(c.upper())
	}
	return ss;
}
 
@baro.schemaCreate(parent, &s) {
	not(s.ch()) return;
	tableNode=null
	startIndex = -1;
	Cf.error(true)
	while(s.valid()) {
		if( Cf.error() ) break;
		if( lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		cnt = indentCount(s)
		c=s.ch()
		if(c.eq('#')) continue;
		if(startIndex.eq(-1)) {
			startIndex = cnt
		}
		// print("xxxxxxxxx", startIndex, cnt)
		if( startIndex.eq(cnt)) {
			tableNode = parent.addNode()
			parse(tableNode)
			continue;
		}
		parse( tableNode.addNode() )
	}
	return node;
	
	parse = func(cur) {
		if( lineCheck(s,'{') ) {
			cur.line = s.findPos('{',0,1).trim()
			data = s.match() if(typeof(data,'bool')) return print("${cur.line} 매칭오류")
			cur.jsonData = data.trim()
			if( lineCheck(s,'--') ) {
				cur.comment = s.findPos("\n").trim()
			}
		} else {
			if( lineCheck(s,'--') ) {
				cur.line = s.findPos('--').trim()
				cur.comment = s.findPos("\n").trim()
			} else {
				cur.line = s.findPos("\n").trim()
			}
		}
	};
}	