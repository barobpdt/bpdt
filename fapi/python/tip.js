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
menu : tree
Auth : entity
User : entity
	userId pk(str)
	userName str
	email
	delYn yn(N)
	ref
	data
	createDtm
	modifyDtm
	
UserAuth 
	userId fk(user.userId)
	authCode str(16) notnull
	authName text
	
Item
	itemCode str(16) pk
	itemName text
	
Order
	orderNo pk(str)
	itemId fk(item.id)

OrderDetail
	orderNo fk(order.orderNo)
		
]
~~ 
getTableInfo(&s, node) {
	not(node) node=_node()
	startIndex = -1;
	info = '', ss=''
	while(s.valid()) {
		if( lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		cnt = indentCount(s)
		c=s.ch()
		if(c.eq('#')) continue;
		line = s.findPos("\n")
		if(startIndex.eq(-1)) {
			startIndex = cnt
		}
		if( startIndex.eq(cnt)) {
			if(ss) {
				cur=node.addNode()
				cur.info = info
				cur.data = ss
			}
			info = line
			ss=''
			continue;
		}
		ss.add(line,"\r\n")
	}
	return node;
} 