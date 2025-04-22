#ifndef DB_UTIL_H
#define DB_UTIL_H

#include "listobject.h"
#include "sqlite3.h"

#define  FLAG_CFSET 0x1
#define  FLAG_BINDED 0x10
#define  FLAG_FETCHED 0x20

class DbUtil : public WBox {
public:
    DbUtil(TreeNode* cf=NULL);
    ~DbUtil();
    bool err(int stat, LPCC msg=NULL);
    bool open(LPCC filenm);
    bool isNodeFlag(U32 flag)	{ return (xflag & flag)>0 ? true: false; }
    U32 setNodeFlag(U32 flag)	{ return xflag|=flag; }
    U32 clearNodeFlag(U32 flag) { return xflag&=~flag; }

    bool closeStmt();
    bool close();
    LPCC getFiledValue(int idx, StrVar* rst, int* size=NULL);
    LPCC field(int idx);
    bool exec(LPCC sql);
    bool bind(LPCC sql=NULL);
    bool prepare(LPCC sql);
    bool fetch();
    bool fetchNode(TreeNode* node=NULL);
    bool fetchResult(StrVar* rst, U16 flag=0);
    TreeNode* fetchTreeNode(TreeNode* root=NULL, bool reuse=false, bool fieldUse=false);
    bool putData(WBox* node);

    LPCC fetchValue(int idx=0);
    int fetchInt(int idx=0);
    UL64 fetchUL64(int idx=0);
    DbUtil& select(LPCC fmt,...);
    DbUtil& query(LPCC sql);
    DbUtil& exec();
    DbUtil& bindStr(LPCC value, int size=-1);
    DbUtil& bindVar(StrVar* sv);
    DbUtil& bindBuffer(LPVOID value, int size);
    DbUtil& bindLong( UL64 dot);
    DbUtil& bindInt( int dot);

    void begin();
    void commit();
    int columnCount();
    int tableCount(LPCC tbl=NULL);

    // 이미 모든레코드를 패치한후 처리하는 함수
    bool isopen() { return xdb ? true : false; }

public:
    sqlite3*			xdb;
    sqlite3_stmt*		xstmt;
    TreeNode*           xcf;
    ListObject<StrVar>	xbinds;
    StrVar				xvar,xmsg;
    U32					xflag;
    int					xrow;

private:
    sqlite3_stmt* stmt(LPCC sql);

};

#endif // DB_UTIL_H
