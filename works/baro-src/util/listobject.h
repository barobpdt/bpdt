#ifndef LISTOBJECT_H
#define LISTOBJECT_H

#include "strvar.h"


#ifdef _WIN32
  #define atoll(S) _atoi64(S)
#endif

#ifdef WIDGET_USE
#ifndef _WIN32
QString KR( QString str );
unsigned long GetTickCount();
#endif
#endif

int _intVal(LPCC val, int def=0);
bool checkObject(StrVar* var, char ch);
bool checkFuncObject(StrVar* var, char ch, U16 state);
bool ccmpl(LPCC code, int csize, LPCC data, int dsize=0 );
bool ccmpl(LPCC code, LPCC data, int size=0 );
bool svcmp(StrVar* sv, LPCC val);

bool isoper(char c);
bool isNumberVar(StrVar* var);
bool isVarTrue(StrVar* var);
int toInteger(StrVar* var);
UL64 toUL64(StrVar* var);
double toDouble(StrVar* var);
LPCC toString(StrVar* var, StrVar* rst=NULL, bool objectCheck=false, int depth=0);
LPCC toLower(LPCC str);
LPCC toUpper(LPCC str);
void setNodeTagValue(LPCC tag, StrVar* sv, StrVar* sub, int sp, int ep);


//==================================================================
//
// Box class
//
//==================================================================

class WBoxNode
{
public:
    U16			type,uid;
    StrVar		value;
public:
    WBoxNode();
    ~WBoxNode();
    char* get()		{ return value.get(); }
    int length()	{ return value.length(); }
};


template<class Etype>
class List {
protected:

    struct Node {
        Etype Element;
        Node *Next;
        Node( Etype E = 0, Node *N = NULL ):
            Element( E ), Next( N ){ }
    };

    Node *List_Head;
    Node *Current_Pos;
    Node *Prev_Pos;
    int nSize ;

public:
    void destroyList( );
    void removeAll( );
    void deleteList( );

    List( List &Value);

    List( ) : nSize(0), List_Head(new Node), Current_Pos(List_Head),Prev_Pos(NULL){ }
    ~List( ){ destroyList( ); }
    int				size() { return nSize; }
    const List &	operator=( List & Value );
    const List &	operator++( );
    bool			operator!( )const;
    const Etype &	operator( )( )const;
    bool				isEmpty( )const{ return List_Head->Next == NULL; }
    int				find( const Etype & X);
    int				find_Previous( const Etype &X );
    bool			removeObject( const Etype &X ) ;
    const Etype&	first() {
        Prev_Pos=NULL;
        if( List_Head->Next )
        {
            Prev_Pos = List_Head;
            Current_Pos = List_Head->Next;
        } else {
            Current_Pos = List_Head;
        }
        return Current_Pos->Element;
    }
    const Etype&	next();
    const Etype&	current();
    int				swap();
    void			header(){ Current_Pos = List_Head; }
    void			add( const Etype &X );
    void			insert( const Etype &X );
    void			insertFirst(const Etype & X);
    const Etype &	pop();
};

template< class Etype >
inline const Etype& List<Etype>::pop()
{
    Node* next = List_Head->Next;
    if(next) {
        List_Head->Next = next->Next;
        Current_Pos=next;
        nSize--;
        return next->Element;
    } else {
        nSize=0;
        Current_Pos=List_Head;
        return List_Head->Element;
    }
}

template< class Etype >
inline int List<Etype>::swap() {
    Node* next=Current_Pos->Next;
    if( Prev_Pos && Current_Pos && next) {
        Prev_Pos->Next=next;
        Current_Pos->Next=next->Next;
        next->Next=Current_Pos;
        Prev_Pos=next;
        //Current_Pos=next;
        return 1;
    }
    return 0;
}

template< class Etype >
inline const Etype& List<Etype>::next() {
    if(Current_Pos ) {
        Node* next=Current_Pos->Next;
        return next? next->Element: NULL;
    }
    return List_Head->Element;
}

template< class Etype >
inline const Etype& List<Etype>::current() {
    if(Current_Pos != NULL)
        return Current_Pos->Element;
    else
        return List_Head->Element;
}

template< class Etype >
inline const List<Etype>& List<Etype>::operator++( ) {
    if(Current_Pos != NULL)
    {
        Prev_Pos = Current_Pos;
        Current_Pos = Current_Pos->Next;
    }
    return *this;
}

template< class Etype >
inline const Etype& List<Etype>::operator( )( )const {
    if(Current_Pos != NULL)
        return Current_Pos->Element;
    else
        return List_Head->Element;
}

template< class Etype >
inline bool List<Etype>::operator!( )const {
    if(List_Head == Current_Pos) return 0;
    return Current_Pos != NULL;
}

template< class Etype >
inline int List<Etype>::find( const Etype &X ) {
    Node *P;
    for( P = List_Head->Next; P != NULL; P = P->Next )
        if( P->Element == X )
        {
            Current_Pos = P;
            return 1;
        }
    return 0;
}

template< class Etype >
inline int List<Etype>::find_Previous( const Etype &X ) {
    Node *P;
    for( P = List_Head; P->Next != NULL; P = P->Next )
       if( P->Next->Element == X )
        {
            Current_Pos = P;
            return 1;
        }
    return 0;
}


template< class Etype >
inline bool List<Etype>::removeObject( const Etype &X ) {
    Node *Cell_To_Delete;
    if( find_Previous( X ))
    {
        Cell_To_Delete = Current_Pos->Next;
        Current_Pos->Next = Cell_To_Delete->Next;
        delete Cell_To_Delete;
        nSize-- ;
        return true;
    }
    return false;
}

template< class Etype >
inline void List<Etype>::add( const Etype &X ) {

    for(header(); Current_Pos->Next != NULL; Current_Pos = Current_Pos->Next);
    Node *P = new Node( X, Current_Pos->Next );

    if( P == NULL )
        fprintf(stderr, "Out Of Space");
    else
    {
        Current_Pos->Next = P;
        Current_Pos = Current_Pos->Next;
        nSize++ ;
    }
}

template< class Etype >
inline void List<Etype>::insert( const Etype &X ) {
    if(Current_Pos == NULL) {
        for(header(); Current_Pos->Next != NULL; Current_Pos = Current_Pos->Next);
    }
    Node *P = new Node( X, Current_Pos->Next );

    if( P == NULL )
        fprintf(stderr, "Out Of Space");
    else
    {
        Current_Pos->Next = P;
        Current_Pos = Current_Pos->Next;
        nSize++ ;
    }
}

template< class Etype >
inline void List<Etype>::insertFirst( const Etype &X )
{
    Node *P = new Node( X, NULL );

    if( P == NULL )
        fprintf(stderr, "Out Of Space");
    else
    {
        P->Next = List_Head->Next;
        List_Head->Next = P;
        Current_Pos = P;
        nSize++ ;
    }
}

template< class Etype >
inline const List<Etype>& List<Etype>::operator=(List<Etype> &Value) {
    if(this == &Value)
        return *this;
    deleteList( );
    Current_Pos = List_Head = new Node;
    for( Value.first( ); !Value; ++Value) {
        Current_Pos->Next = new Node( Value( ), Current_Pos->Next );
        Current_Pos = Current_Pos->Next;
    }
    Current_Pos->Next = 0;
    first( );
    Value.first( );
    return *this;
}

template< class Etype >
inline void List<Etype>::destroyList( ) {
    Node *P = List_Head->Next, *Temp;
    while( P != NULL )
    {
        Temp = P->Next;
        SAFE_DELETE(P);
        P = Temp;
    }
    SAFE_DELETE(List_Head);
    nSize = 0 ;
}

template< class Etype >
inline void List<Etype>::deleteList() {
    Node *P = List_Head->Next, *Temp;
    while( P != NULL )
    {
        Temp = P->Next;
        SAFE_DELETE(P);
        P = Temp;
    }
    List_Head->Next = NULL;
    Current_Pos = List_Head;
    nSize = 0 ;
}

template< class Etype >
inline void List<Etype>::removeAll() {
    Node *P = List_Head->Next, *Temp;
    while( P != NULL )
    {
        Temp = P->Next;
        if( P->Element)
        {
            delete P->Element;
            P->Element=NULL;
        }
        delete P;
        P = Temp;
    }
    List_Head->Next = NULL;
    Current_Pos = List_Head;
    nSize = 0 ;
}



class XListArr
{
public:
    char		*buffer;
    char		*bufferpos;
    int			total, last;
    U32         buffersize;
    U16         xflag, objsize;

public:
    XListArr(bool newInst=false);
    ~XListArr();
    bool isNodeFlag(U16 flag)	{ return (xflag & flag)>0 ? true: false; }
    U16 setNodeFlag(U16 flag)	{ return xflag|=flag; }
    U16 clearNodeFlag(U16 flag) { return xflag&=~flag; }

    BOOL IsCreate();
    virtual void Create(int size=8);
    virtual void Close();
    XListArr* reuse();
    int size();
    int length();

    void	grow_buffer();
    BOOL	grow(int txtlen );
    StrVar*	add();
    StrVar*	get(int n);
    StrVar*	getObject(int n);
    StrVar*	nextObject();
    int		remove(int n=0, int end=0);
    StrVar*	insert(int n );
    StrVar*	operator[] (int n);
};


template<class Etype>
class ListObject
{
public:
    char		*buffer;
    char		*bufferpos;
    size_t		buffersize;
    U16		total;
    U16		cur;
    U16		objsize, last;

public:
    ListObject();
    ~ListObject();
    BOOL IsCreate();
    virtual void Create(int size=0);
    virtual void Close();
    virtual void reuse();
    int size();
    int length();

    void	grow_buffer();
    BOOL	grow(int txtlen );
    Etype*	add();
    Etype*	add(const Etype& X );
    void	set(const Etype& X );
    Etype*	get(int n);
    Etype*	getObject(int n);
    Etype*	nextObject();
    Etype	getvalue(int n);
    int		remove(int n);
    Etype*	insert(int n, Etype& X );
    Etype*	put(int n, Etype X );
    Etype*	operator[] (int n);

    int		findObject(LPVOID obj);
    bool	removeObject(LPVOID obj);
    bool	slice(Etype obj);


    Etype*	get();
    Etype* Add(int n, Etype& X);
};


//----------------------------------------------------------------------------------------
//
// HashObject
//
//----------------------------------------------------------------------------------------

template<class Etype>
class HashObject
{
public:
    char		*buffer;
    char		*bufferpos;
    size_t		buffersize;
    size_t		keysize;
    U16		objsize;
    U16		locsize;
    U16		total;
    U16		last;
    U16		cur;
    U16		maxsize;
    U32		*lockey;
    char		*key;
    char		*keypos;
public:
    HashObject();
    ~HashObject();
    //===========================================================================================

    BOOL IsCreate();
    void Create();
    void Close();
    //===========================================================================================

    int		size();
    int		length();


    Etype*	get(int n);
    Etype&	getvalue(int n);
    int     remove(int n);
    Etype*  operator[] (int n);
    void	first();
    void	next();
    BOOL	end();
    Etype*	get();

public:

    void    reuse();
    void	grow_buffer();
    void	grow_key();
    void	grow_location();
    BOOL	grow(int txtlen );
    BOOL 	grow(int objlen, int keylen);
    BOOL    grow();
    Etype*	add(LPCC code, const Etype& X );
    Etype*	add(LPCC code);
    BOOL    find(LPCC code, int idx=-1);
    Etype&	current();
    int		set(Etype& X );
    BOOL    startWith(LPCC code, int idx=-1);
    Etype*  put(int n, LPCC code, Etype& X );
    Etype&  putKey(UL64 idx, Etype& X );
    void	putData(LPCC code, Etype& X );

    int		removeLoc(int n, int size  );
    int		insertLoc(int n, int size  );
    int		removeKey(int n);
    int		insertKey(int n, LPCC code );
    Etype*	insert(LPCC findcode, LPCC code, Etype& X );
    BOOL	removecode(LPCC findcode );
    bool	changeValue(int n, Etype& X );


    LPCC    code();
    LPCC    code(int n);
    Etype*	object();
    Etype*	object(LPCC code);
    Etype	value(int n, Etype def);
    Etype	value();
    Etype	value(LPCC code);
    //
    // 정수를 키로 사용하는 리스트에 사용
    //
    BOOL	removeCode(LPCC findcode );
    Etype&	addKey(UL64 idx, Etype& X );
    Etype&	addKey(UL64 idx);
    bool	findKeyPos(UL64 idx);
    Etype&	findKey();
    int		deleteKey(UL64 idx);

};


template<class Etype>
class WHash
{
    HashObject<Etype>* list;
    int total;
    U16 hsize;
    U16 curkey;
    U16 curhash;
    U16 curobj;
public:
    WHash(int s=4);
    ~WHash();
    void	reuse();
    void	Create(int s);
    void	Close();
    LPCC	Code();
    Etype*	Get();
    Etype*	First();
    Etype*	Next();
    Etype	get();
    Etype	first();
    Etype	next();

    BOOL	IsCreate();
    U32		GetHash(LPCC key);
    BOOL	Grow(LPCC code);
    Etype&	addKey(UL64 idx, Etype& X);
    Etype&	addKey(UL64 idx);
    Etype&	putKey(UL64 idx, Etype& X);
    Etype&	findKey();
    bool	findKeyPos(UL64 idx);
    int		deleteKey(UL64 idx);
    LPCC	code();
    Etype*	add(LPCC code, Etype& X);
    void	put(LPCC code, Etype& X);
    int		set(Etype& X);
    Etype*	get(LPCC code);
    Etype	value(LPCC code);
    int		remove(LPCC code);
    int		size();
};


class WBox {
public:
    WHash<WBoxNode> hash;
public:
    WBox(int s=4);
    ~WBox();
public:
    virtual void Create(int s=4);
    virtual void Close();

    BOOL isCreate();
    WBoxNode* GetBox(LPCC code);
    WBoxNode* GetHash(LPCC code);
    StrVar* GetVar(LPCC code);
    WBoxNode* First() { return hash.First(); }
    WBoxNode* Next() { return hash.Next(); }
    LPCC getCurCode() { return hash.Code(); }
    void reuse() { hash.reuse(); }

    void set(LPCC code,LPCC value,int size=-1, U16 type=0);
    LPCC get(LPCC code);
    BOOL isset(LPCC code);
    bool remove(LPCC code);
    StrVar* gv(LPCC code);

    int incrNum(LPCC code, int incr=1);
    bool isNum(LPCC code);
    bool isBool(LPCC code) { return checkObject(gv(code),'3'); }
    bool isTrue(LPCC code) { return checkFuncObject(gv(code),'3',1); }
    bool isNull(LPCC code) { return checkFuncObject(gv(code),'9',0); }
    bool isNode(LPCC code) { return checkFuncObject(gv(code),'n',0); }
    bool isArray(LPCC code) { return checkFuncObject(gv(code),'a',0); }
    int getInt(LPCC code, int def=0);
    bool getBool(LPCC code, bool def=false);
    U32 getU32(LPCC code, U32 def=0);
    UL64 getLong(LPCC code, UL64 def=0);
    double getDouble(LPCC code, double def=0);
    void setInt(LPCC code, int val);
    void setU32(LPCC code, U32 val);
    void setLong(LPCC code, UL64 val);
    void setDouble(LPCC code, double val);
    void setBool(LPCC code, bool val);
    void setNull(LPCC code);
    bool cmp(LPCC field, LPCC val);
};


class TreeNode : public WBox {
public:
    TreeNode(int sz=4, bool newInst=false);
    ~TreeNode();
public:
    int childCount()			{ return xchilds.size(); }
    TreeNode* child(int n)		{ return n<childCount() ? xchilds.getvalue(n): NULL; }
    TreeNode* parent()			{ return xparent; }
    bool isNodeState(U16 flag)	{ return (xstat & flag)>0 ? true: false; }
    U16 setNodeState(U16 flag)			{ return xstat|=flag; }
    U16 clearNodeState(U16 flag)		{ return xstat&=~flag; }
    bool isNodeFlag(U32 flag)	{ return (xflag & flag)>0 ? true: false; }
    U32 setNodeFlag(U32 flag)			{ return xflag|=flag; }
    U32 clearNodeFlag(U32 flag)		{ return xflag&=~flag; }

    bool reset(bool reuse=false);
    bool remove(TreeNode* node);
    bool removeAll(bool del=false, bool reset=false);
    TreeNode* reuseAll(bool del=false);
    int row();
    int find(TreeNode* node);
    int clone(TreeNode* srcNode, bool overwrite=true );
    TreeNode* getNode(LPCC code);
    TreeNode* setNode(LPCC code);
    TreeNode* addNode(LPCC code=NULL, bool reuse=false);
    XListArr* addArray(LPCC code, bool reuse=false);
    XListArr* getArray(LPCC code);
    TreeNode* addChild(TreeNode* node);

    int setJsonValue(XParseVar& pv, StrVar* sv, char ch, WBox* box=NULL, LPCC field=NULL);
    int setJson(StrVar* data, int sp=0, int ep=-1 );
    int setJson(XParseVar& pv, WBox* box=NULL, bool bchk=false );
    int setArrayValue(XListArr* arrs, XParseVar& pv, WBox* box=NULL);
    LPCC log(StrVar* rst=NULL, bool bChild=false, int depth=0);
public:
    U16							xtype, xstat;
    U32                         xflag;
    TreeNode*					xparent;
    ListObject<TreeNode*>		xchilds;

#ifdef NODE_OBJECT_USE
    XListArr                    xobjects;
    ListObject<TreeNode*>		xnodes;
#else
    ListObject<XListArr*>		xarrs;
#endif
};
//
//
//

template< class Etype >
inline ListObject<Etype>::ListObject()
{
        buffer = bufferpos = NULL;
        buffersize = 0;
        total = cur = 0;
        objsize = 0;
        last = 0;
}
template< class Etype >
inline ListObject<Etype>::~ListObject()
{
}
template< class Etype >
inline BOOL ListObject<Etype>::IsCreate()
{
        return (buffer) ? TRUE: FALSE;
}
template< class Etype >
inline void ListObject<Etype>::Create(int size)
{
        objsize = sizeof(Etype);
        if(size)
                buffersize = size;
        else
                buffersize = 256;

        buffer =(char *) malloc( buffersize );
        reuse();
}
template< class Etype >
inline void ListObject<Etype>::Close()
{
        SAFE_FREE(buffer);
}
template< class Etype >
inline void ListObject<Etype>::reuse()
{
        bufferpos = buffer;
        total = 0;
        cur = 0;
}

template< class Etype >
inline int ListObject<Etype>::size() { return total; }
template< class Etype >
inline int ListObject<Etype>::length() { return (bufferpos-buffer); }

template< class Etype >
inline void	ListObject<Etype>::grow_buffer()
{
        size_t length = bufferpos - buffer;
        // buffersize = buffersize *2;
        // Log("******** grow buffer %d %d *******\n", length , buffersize );
        buffer =(char *) realloc(buffer,buffersize);
        bufferpos = buffer + length;	// readjust current position
}
template< class Etype >
inline BOOL	ListObject<Etype>::grow(int txtlen )
{
        if( txtlen==0 ) return FALSE;
        int tlen = (bufferpos-buffer) + txtlen;
        if(tlen>=buffersize)
        {
                while(tlen>=buffersize ) buffersize = buffersize *2;
                grow_buffer();
        }
        return TRUE;
}
template< class Etype >
inline Etype*	ListObject<Etype>::add(const Etype& X )
{
    if( buffer==NULL) Create();
    if( !grow(objsize) ) return NULL;
    cur = total;
    Etype* rtn = (Etype*)bufferpos;
    memcpy(bufferpos,&X,objsize);
    bufferpos+=objsize;
    total++;
    last = total;
    return rtn;
}

template< class Etype >
inline Etype*	ListObject<Etype>::add()
{
    Etype X;
    return add(X);
}

template< class Etype >
inline void	ListObject<Etype>::set(const Etype& X )
{
        if( buffer==NULL) Create();
        if( !grow(objsize) ) return;
        memcpy(bufferpos,&X,objsize);
        bufferpos+=objsize;
        total++;
                        if( total>last )
                                last = total;
}

template< class Etype >
inline Etype*	ListObject<Etype>::get(int n)
{
        if( buffer==NULL ) Create();
        int offset = n * objsize;
        return ( n>=0 && n<total ) ? (Etype*)(buffer + offset): NULL;
}

template< class Etype >
inline Etype*	ListObject<Etype>::getObject(int n) {
    if( buffer==NULL ) Create();
    int offset = n * objsize;
    if( n>=0 && n<last ) {
        return (Etype*)(buffer + offset);
    }
    Etype e;
    return add(e);
}

template< class Etype >
inline Etype*	ListObject<Etype>::nextObject() {
    if( buffer==NULL ) Create();
    int offset = cur * objsize;
    if( offset<buffersize && total<last && cur<last ) {
        cur++;
        return (Etype*)(buffer + offset);
    }
    return add(Etype());
}

template< class Etype >
inline Etype	ListObject<Etype>::getvalue(int n)
{
        Etype* obj = NULL;
        if( n<0 || n>=total )
        {
                obj = (total>0) ? (Etype*)(buffer): NULL;
                return (obj) ? (Etype)(*obj): NULL;
        }
        if( buffer==NULL )
            Create();
        int offset = n * objsize;
        obj = (Etype*)(buffer + offset);
        return (Etype)(*obj);
}

template< class Etype >
inline int ListObject<Etype>::remove(int n)
{
    int len = length();
    int nPos = n*objsize;
    int nCount = objsize;
    if(len==objsize ) {
        if(n>0) {
            return 0;
        }
        bufferpos = buffer;
        total = 0;
        cur = 0;
        return objsize;
    }
    if( nCount == 0 || len < nPos+nCount) {
        return 0;
    }
    char* dest = buffer+nPos+nCount;
    int movelen = bufferpos - dest;
    memmove(buffer+nPos, dest, movelen);
    bufferpos = buffer+(len-nCount);
    total--;
    return movelen;
}

template< class Etype >
inline Etype* ListObject<Etype>::insert(int n, Etype& X )
{
        int len = length();
        int nPos = n*objsize;
        if( nPos == len )
        {
                return add(X);
        }
        if( nPos>len || !grow(objsize) ) return NULL;
        int movelen= bufferpos - (buffer+nPos);
        Etype* rtn = (Etype*)buffer+nPos;
        memmove(buffer+nPos+objsize,buffer+nPos,movelen);
        memcpy(buffer+nPos,&X,objsize);
        bufferpos = buffer+len+objsize;
        total++;
        return rtn;
}

template< class Etype >
inline Etype* ListObject<Etype>::put(int n, Etype X )
{
        int len = length();
        int nPos = n*objsize;
        if( nPos == len )
            return add(X);
        if( nPos>len || !grow(objsize) )
            return add(X);
        // int movelen= bufferpos - (buffer+nPos);
        Etype* rtn = (Etype*)buffer+nPos;
        memcpy(buffer+nPos,&X,objsize);
        bufferpos = buffer+len+objsize;
        return rtn;
}

template< class Etype >
inline Etype* ListObject<Etype>::operator[] (int n)
{
        return get(n);
}

template< class Etype >
inline int ListObject<Etype>::findObject(LPVOID obj) {
    for( int n=0,num=size();n<num;n++) {
        if( getvalue(n)==(Etype)obj )
            return n;
    }
    return -1;
}

template< class Etype >
inline bool ListObject<Etype>::slice(Etype obj) {
    if( total>0 ) {
        int tot = (int)total-1;
        Etype* e = NULL;
        for(;tot>=0;tot--) {
            e=(Etype*)( buffer + (tot * objsize) );
            if( obj==*e ) {
                total = tot;
                bufferpos = (char*)e;
                return true;
            }
        }
    }
    return false;
}

template< class Etype >
inline bool ListObject<Etype>::removeObject(LPVOID obj) {
    int n = findObject(obj);
    if( n==-1 ) return false;
    remove(n);
    return true;
}


template< class Etype >
inline Etype*	ListObject<Etype>::get()
{
        return ( cur>=0 && cur<total ) ? get(cur): NULL;
}
template< class Etype >
inline Etype* ListObject<Etype>::Add(int n, Etype& X)
{
        if( n<size() )
        {
                int offset = n * objsize;
                return (Etype*)(buffer + offset);
        }
        return add(X);
}

//
//
//


template< class Etype >
inline HashObject<Etype>::HashObject()
{
        lockey = NULL;
        key = keypos = NULL;
        buffer = bufferpos = NULL;
        keysize = 0;
        buffersize = 0;
        total = 0;
        cur = 0;
        maxsize = 0;
        locsize  = objsize = 0;
}
template< class Etype >
inline HashObject<Etype>::~HashObject()
{
        Close();
}

//===========================================================================================

template< class Etype >
inline BOOL HashObject<Etype>::IsCreate()
{
        return (buffer) ? TRUE: FALSE;
}
template< class Etype >
inline void HashObject<Etype>::Create()
{
    objsize = sizeof(Etype);
    buffersize = 256;
    keysize = 256;
    locsize = 32;
    key =(char *) malloc( keysize );
    lockey =(U32*) malloc( locsize*sizeof(U32) );
    buffer =(char *) malloc( buffersize );
    memset(lockey,0,locsize*sizeof(U32) );
    reuse();
}
template< class Etype >
inline void HashObject<Etype>::Close()
{
    SAFE_FREE(buffer);
    SAFE_FREE(lockey);
    SAFE_FREE(key);
}
//===========================================================================================

template< class Etype >
inline int HashObject<Etype>::size() { return total; }
template< class Etype >
inline int HashObject<Etype>::length() { return (bufferpos-buffer); }


template< class Etype >
inline Etype*	HashObject<Etype>::get(int n)
{
        if( buffer==NULL ) Create();
        int offset = n * objsize;
        return ( n>=0 && n<total ) ? (Etype*)(buffer + offset): NULL;
}
template< class Etype >
inline Etype&	HashObject<Etype>::getvalue(int n)
{
        Etype* obj = NULL;
        if( n<0 || n>=total )
        {
                obj = (Etype*)(buffer);
                return (Etype&)(*obj);
        }
        if( buffer==NULL ) Create();
        int offset = n * objsize;
        obj = (Etype*)(buffer + offset);
        return (Etype&)(*obj);
}
template< class Etype >
inline int HashObject<Etype>::remove(int n)
{
        int len = length();
        int nPos = n*objsize;
        int nCount = objsize;
        if (nCount == 0 || len < nPos+nCount) return 0;
        int movelen = bufferpos - (buffer+nPos);
        memmove(buffer+nPos, buffer+nPos+nCount, movelen);
        bufferpos = buffer+(len-nCount);
        total--;
        return movelen;
}
template< class Etype >
inline Etype* HashObject<Etype>::operator[] (int n)
{
        return get(n);
}
template< class Etype >
inline void	HashObject<Etype>::first()
{
        cur =0;
}
template< class Etype >
inline void	HashObject<Etype>::next()
{
        cur++;
}
template< class Etype >
inline BOOL	HashObject<Etype>::end()
{
        return (cur<total)? FALSE: TRUE;
}
template< class Etype >
inline Etype*	HashObject<Etype>::get()
{
        return ( cur>=0 && cur<total ) ? get(cur): NULL;
}
template< class Etype >
inline void HashObject<Etype>::reuse()
{
        // Log("reuse total=%d, maxsize=%d\n", total, maxsize );
        maxsize = total;
        total = 0;
        cur = 0;
        bufferpos = buffer;
        keypos = key;
}
template< class Etype >
inline void	HashObject<Etype>::grow_buffer()
{
        size_t length = bufferpos - buffer;
        // buffersize = buffersize *2;
        // qDebug("******** grow buffer %d %d *******", length , buffersize );
        buffer =(char *) realloc(buffer,buffersize);
        bufferpos = buffer + length;	// readjust current position
}
template< class Etype >
inline void	HashObject<Etype>::grow_key()
{
        size_t length = keypos - key;
        key =(char *) realloc(key,keysize);
        keypos = key + length;	// readjust current position
}
template< class Etype >
inline void	HashObject<Etype>::grow_location() {
        locsize *= 2;
        lockey =(U32*)realloc(lockey,locsize*sizeof(U32) );
}

template< class Etype >
inline BOOL	HashObject<Etype>::grow(int txtlen ) {
        if( txtlen==0 ) return FALSE;
        int tlen = (bufferpos-buffer) + txtlen;
        if(tlen>=buffersize)
        {
                while(tlen>=buffersize ) buffersize = buffersize *2;
                grow_buffer();
        }
        return TRUE;
}
template< class Etype >
inline BOOL 	HashObject<Etype>::grow(int objlen, int keylen) {
        if( objlen==0 || keylen==0 )
            return FALSE;
        int tlen = (bufferpos-buffer) + objlen;
        int klen = (keypos-key) + keylen;
        if(tlen>=buffersize)
        {
                while(tlen>=buffersize ) buffersize = buffersize *2;
                grow_buffer();
        }
        if(klen>=keysize)
        {
                while(klen>=keysize ) keysize = keysize *2;
                grow_key();
        }
        if( locsize <= total+1 )
            grow_location();
        return TRUE;
}
template< class Etype >
inline BOOL HashObject<Etype>::grow() {
        int n = (buffersize - (bufferpos-buffer)) / objsize;
        if( n<8 )
        {
                n = 8-n;
                return grow( objsize*n, 64 );
        }
        return TRUE;
}
template< class Etype >
inline Etype*	HashObject<Etype>::add(LPCC code, const Etype& X )
{
        if( buffer==NULL ) Create();
        int klen = slen(code);
        if( klen==0 || !grow( objsize, klen+1) )
            return NULL;
        Etype* rtn = (Etype*)bufferpos;
        if( total+1 > maxsize )
                memcpy(bufferpos,&X,objsize);
        bufferpos+=objsize;
        cur = total;
        lockey[total] = (U32)(keypos - key);
        memcpy(keypos,code,klen);
        keypos+=klen;
        *keypos++ = 0;
        keypos++;
        total++;
        return rtn;
}

template< class Etype >
inline Etype*	HashObject<Etype>::add(LPCC code )
{
    Etype X;
    return add(code,X);
}

template< class Etype >
inline BOOL HashObject<Etype>::find(LPCC code, int idx)
{
    if( code==NULL )
    {
        if( buffer==NULL )
        {
            Etype e;
            add("#",e);
        }
        return find("#");
    }
    if( slen(code)==0 ) return FALSE;
    if( buffer==NULL )
        Create();
    int n = 0;

    if( idx!=-1 )
        n = idx;
    if( n>=total )
    {
        return FALSE;
    }
    for( ;n<total;n++)
    {
        U32 kpos = lockey[n];
        LPCC tmp = key+kpos;
        if( code[0]== tmp[0] && ccmp(code,tmp) )
        {
            cur = n;
            return TRUE;
        }
    }
    return FALSE;
}
template< class Etype >
inline Etype&	HashObject<Etype>::current()
{
        if( buffer==NULL )
        {
                Etype e;
                add("#",e);
                cur = 0;
        }
        Etype* obj = NULL;
        int n = ( cur>=0 && cur<total ) ? cur: 0;
        int offset = n * objsize;
        obj = (Etype*)(buffer + offset);
        return (Etype&)(*obj);
}
template< class Etype >
inline int	HashObject<Etype>::set(Etype& X )
{
        if( cur<total )
        {
                char* curpos = buffer + (cur*objsize);
                memcpy(curpos,&X,objsize);
                return 1;
        }
        return 0;
}
template< class Etype >
inline BOOL HashObject<Etype>::startWith(LPCC code, int idx)
{
        if( code==NULL || code[0]==0 ) return FALSE;
        if( buffer==NULL ) Create();
        int klen = slen(code);
        int n = 0;
        if( idx!=-1 ) n=idx;
        if( n>=total )
        {
                n = 0;
                return FALSE;
        }
        for( ;n<total;n++)
        {
                if( code[0]== *(key+lockey[n]) && memcmp(code,key+lockey[n],klen)==0 )
                {
                        cur = n;
                        return TRUE;
                }
        }
        return FALSE;
}
template< class Etype >
inline Etype* HashObject<Etype>::put(int n, LPCC code, Etype& X )
{
        int len = length();
        int nPos = n*objsize;
        if( nPos == len )
            return add(code,X);
        if( nPos>len || !grow(objsize) )
            return add(code,X);
        // int movelen= bufferpos - (buffer+nPos);
        Etype* rtn = (Etype*)buffer+nPos;
        memcpy(buffer+nPos,&X,objsize);
        // bufferpos = buffer+len+objsize;
        bufferpos = buffer+len;
        return rtn;
}

template< class Etype >
inline bool HashObject<Etype>::changeValue(int n, Etype& X )
{
    int len = length();
    int nPos = n*objsize;
    if( nPos<=len  ) {
        Etype* rtn = (Etype*)buffer+nPos;
        memcpy(buffer+nPos,&X,objsize);
        // bufferpos = buffer+len+objsize;
        return true;
    }
    return false;
}
template< class Etype >
inline Etype& HashObject<Etype>::putKey(UL64 idx, Etype& X )
{
        int len = length();
        int nPos = cur*objsize;
        if( nPos == len )
            return addKey(idx,X);
        if( nPos>len || !grow(objsize) )
            return addKey(idx,X);
        // int movelen= bufferpos - (buffer+nPos);
        Etype* rtn = (Etype*)buffer+nPos;
        memcpy(buffer+nPos,&X,objsize);
        bufferpos = buffer+len;
        return *rtn;
}

template< class Etype >
inline void	HashObject<Etype>::putData(LPCC code, Etype& X ) {
        if( buffer==NULL ) Create();
        if( find(code) ) {
                put(cur,code,X);
        }
}


template< class Etype >
inline int HashObject<Etype>::removeLoc(int n, int size  )
{
        for(int i=n;i<total-1;i++)
        {
                if( lockey[i+1]< size ) return 0;
                lockey[i] = lockey[i+1]-size;
        }
        return 1;
}
template< class Etype >
inline int HashObject<Etype>::insertLoc(int n, int size  )
{
        int len = total*2;
        int nPos = n*2;
        int klen = 2;
        if( nPos>len  ) return 0;
        if( len+klen>=locsize)
        {
                while(len+size>=locsize )
                    locsize = locsize *sizeof(U16);
                grow_location();
        }
        for(int i=total-1;i>=n;i--)
        {
                lockey[i+1] = lockey[i]+size;
        }
        lockey[n] = (U32)nPos;
        return 1;
}
template< class Etype >
inline int HashObject<Etype>::removeKey(int n)
{
        int len = (keypos-key);
        int nPos = lockey[n];
        LPCC k = key+nPos;
        int klen = slen(k)+1;
        if ( len < 0 || len < nPos+klen) return 0;
        removeLoc(n,klen);
        int movelen = keypos - (key+nPos);
        memmove(key+nPos, key+nPos+klen, movelen);
        keypos = key+(len-klen);
        return movelen;
}
template< class Etype >
inline int HashObject<Etype>::insertKey(int n, LPCC code )
{
        int len = (keypos-key);
        int nPos = lockey[n];
        int klen = slen(code)+1;
        if( len < 0 || nPos>len  ) return 0;
        if( len+klen>=keysize)
        {
                while(len+klen>=keysize )
                    keysize = keysize *2;
                grow_key();
        }
        insertLoc(n,klen);
        int movelen= keypos - (key+nPos);
        memmove(key+nPos+klen,key+nPos,movelen);
        memcpy(key+nPos,code,klen);
        keypos = key+len+klen;
        return movelen;
}
template< class Etype >
inline Etype*	HashObject<Etype>::insert(LPCC findcode, LPCC code, Etype& X )
{
        if( find(findcode) )
        {
                if( insertKey(cur,code) ) return insert(cur,X);
        }
        return NULL;
}

template< class Etype >
inline LPCC HashObject<Etype>::code()
{
        return  ( cur>=0 && cur<total ) ? key+lockey[cur] : NULL;
}
template< class Etype >
inline LPCC HashObject<Etype>::code(int n)
{
        return  ( n>=0 && n<total ) ? key+lockey[n] : NULL;
}
template< class Etype >
inline Etype*	HashObject<Etype>::object()
{
        return ( cur>=0 && cur<total ) ? get(cur): NULL;
}
template< class Etype >
inline Etype*	HashObject<Etype>::object(LPCC code)
{
        return ( find(code) ) ? get(cur): NULL;
}
template< class Etype >
inline Etype	HashObject<Etype>::value(int n, Etype def)
{
        return ( n>=0 && n<total ) ? getvalue(n): def;
}
template< class Etype >
inline Etype	HashObject<Etype>::value()
{
return ( cur>=0 && cur<total ) ? getvalue(cur): NULL;
}
template< class Etype >
inline Etype	HashObject<Etype>::value(LPCC code)
{
        return  ( find(code) ) ? getvalue(cur) : NULL;
}

//
// 정수를 키로 사용하는 리스트에 사용
//

template< class Etype >
inline BOOL	HashObject<Etype>::removeCode(LPCC findcode )
{
    if( find(findcode) )
    {
        if( removeKey(cur ) ) return remove(cur);
    }
    return NULL;
}



template< class Etype >
inline Etype& HashObject<Etype>::addKey(UL64 idx, Etype& X )
{
        if( buffer==NULL ) Create();
        int klen = sizeof(UL64);
        if( !grow( objsize, klen) ) {
            return (Etype&)X;
        }
        Etype* obj = (Etype*)bufferpos;
        memcpy(bufferpos,&X,objsize);
        bufferpos+=objsize;
        cur = total;
        lockey[total] = (U32)(keypos - key);
        memcpy(keypos,&idx,klen);
        keypos+=klen;
        total++;
        return (Etype&)(*obj);
}

template< class Etype >
inline Etype& HashObject<Etype>::addKey(UL64 idx) {
    Etype X;
    return addKey(idx,X);
}

template< class Etype >
inline Etype& HashObject<Etype>::findKey()
{
    Etype* value = (Etype*)buffer;
    return value[cur];
}

template< class Etype >
inline bool HashObject<Etype>::findKeyPos(UL64 idx)
{
    UL64* pcode = (UL64*)key;
    for(int n=0; n<total ; n++) {
        if( pcode[n]==idx )
        {
            cur = n;
            return true;
        }
    }
    return false;
}
template< class Etype >
inline int HashObject<Etype>::deleteKey(UL64 idx)
{
    UL64* pcode = (UL64*)key;
    Etype* value = (Etype*)buffer;
    for(int n=0; n<total ; n++) {
        if( pcode[n]==idx ) {
            if( removeKey(n) )
                return remove(n);
        }
    }
    return 0;
}

//
//
//

template< class Etype >
inline WHash<Etype>::WHash(int s)
{
        hsize = 0;
        list = NULL;
        curhash = curobj = 0;
        if(s>0) Create(s);
}
template< class Etype >
inline WHash<Etype>::~WHash()
{
        // Close();
}
template< class Etype >
inline void WHash<Etype>::reuse()
{
        if( list == NULL ) return;
        for( int n=0; n<hsize; n++)
        {
                list[n].reuse();
        }
        total = 0;
}
template< class Etype >
inline void WHash<Etype>::Create(int s)
{
        if( list==NULL )
        {
                hsize = s;
                total = 0;
                list = new HashObject<Etype>[hsize];
                for( int n=0; n<hsize; n++)
                {
                        list[n].Create();
                }
        }
        //else
        //	reuse();
}
template< class Etype >
inline void WHash<Etype>::Close()
{
        if(list)
        {
                for( int n=0; n<hsize; n++)
                {
                        list[n].Close();
                }
                delete []list;
        }
        list = NULL;
}
template< class Etype >
inline LPCC WHash<Etype>::Code()
{
        // return ( list && curkey<hsize ) ? list[curkey].code(list[curkey].cur) : NULL;
        return ( list && curhash<hsize ) ? list[curhash].code(curobj) : NULL;
}
template< class Etype >
inline Etype* WHash<Etype>::Get()
{
        // return ( list && curkey<hsize ) ? list[curkey].get(list[curkey].cur) : NULL;
        return ( list && curhash<hsize ) ? list[curhash].get(curobj) : NULL;
}
template< class Etype >
inline Etype* WHash<Etype>::First()
{
        curhash = curobj = 0;
        for( ;curhash<hsize; curhash++)
        {
                if( list[curhash].size() ) return list[curhash].get(curobj);
        }
        return NULL;
}
template< class Etype >
inline Etype* WHash<Etype>::Next()
{
        if( list && curhash<hsize)
        {
                curobj++;
                if( curobj >= list[curhash].size() )
                {
                        curhash++;
                        while( curhash<hsize && list[curhash].size()==0 )
                                curhash++;
                        curobj=0;
                }
                return (curhash<hsize) ? list[curhash].get(curobj) : NULL;
        }
        return NULL;
}
template< class Etype >
inline Etype WHash<Etype>::get() {
        // return ( list && curkey<hsize ) ? list[curkey].get(list[curkey].cur) : NULL;
        return ( list && curhash<hsize ) ? list[curhash].getvalue(curobj) : NULL;
}
template< class Etype >
inline Etype WHash<Etype>::first() {
        curhash = curobj = 0;
        for( ;curhash<hsize; curhash++) {
                if( list[curhash].size() ) return list[curhash].getvalue(curobj);
        }
        return NULL;
}
template< class Etype >
inline Etype WHash<Etype>::next() {
        if( list && curhash<hsize) {
                curobj++;
                if( curobj >= list[curhash].size() ) {
                        curhash++;
                        while( curhash<hsize && list[curhash].size()==0 )
                                curhash++;
                        curobj=0;
                }
                return (curhash<hsize) ? list[curhash].getvalue(curobj) : NULL;
        }
        return NULL;
}
template< class Etype >
inline BOOL WHash<Etype>::IsCreate() { return (list) ? TRUE: FALSE; }

template< class Etype >
inline U32 WHash<Etype>::GetHash(LPCC key)
{
        U32 Hash_Val = 0;
        int count = 0;
        while(*key && count<5 )
        {
                Hash_Val = (Hash_Val << 5) + *key++; // (Hash_Val << 5) +
                count++;
        }
        return Hash_Val % hsize;
}
template< class Etype >
inline BOOL WHash<Etype>::Grow(LPCC code)
{
        if( list && slen(code) )
        {
                int key = GetHash(code);
                return list[key].grow();
        }
        return FALSE;
}


template< class Etype >
inline Etype& WHash<Etype>::addKey(UL64 idx, Etype& X)
{
        curkey = (U16)(idx % hsize);
        total++;
        return list[curkey].addKey(idx,X);
}
template< class Etype >
inline Etype& WHash<Etype>::putKey(UL64 idx, Etype& X)
{
    if( findKeyPos(idx) ) {
        curkey = (U16)(idx % hsize);
        return list[curkey].putKey(idx,X);
    }
    return addKey(idx,X);
}

template< class Etype >
inline Etype& WHash<Etype>::addKey(UL64 idx)
{
    curkey = (U16)(idx % hsize);
    total++;
    return list[curkey].addKey(idx);
}

template< class Etype >
inline bool WHash<Etype>::findKeyPos(UL64 idx)
{
    curkey = (U16)(idx % hsize);
    return (list) ? list[curkey].findKeyPos(idx): false;
}

template< class Etype >
inline Etype& WHash<Etype>::findKey()
{
    return list[curkey].findKey();
}


template< class Etype >
inline int WHash<Etype>::deleteKey(UL64 idx)
{
    curkey = (U16)(idx % hsize);
    total--;
    return (list) ? list[curkey].deleteKey(idx): 0;
}


template< class Etype >
inline LPCC WHash<Etype>::code()
{
        return (list) ? list[curkey].code(): NULL;
}
template< class Etype >
inline Etype* WHash<Etype>::add(LPCC code, Etype& X)
{
        if( slen(code)==0 ) return NULL;
        if( list==NULL ) Create(2);
        curkey = (U16)GetHash(code);
        total++;
        return list[curkey].add(code,X);
}
template< class Etype >
inline void WHash<Etype>::put(LPCC code, Etype& X)
{
        if( slen(code)==0 ) return;
        if( list==NULL ) Create(2);
        curkey = (U16)GetHash(code);
        list[curkey].putData(code,X);
}
template< class Etype >
inline int WHash<Etype>::set(Etype& X)
{
        return (list) ? list[curkey].set(X): 0;
}
template< class Etype >
inline Etype* WHash<Etype>::get(LPCC code)
{
        if( slen(code)==0 || list==NULL ) return NULL;
        curkey = (U16)GetHash(code);
        return list[curkey].object(code);
}
template< class Etype >
inline Etype WHash<Etype>::value(LPCC code)
{
        if( slen(code)==0 || list==NULL ) return NULL;
        curkey = (U16)GetHash(code);
        return list[curkey].value(code);
}
template< class Etype >
inline int WHash<Etype>::remove(LPCC code)
{
        if( slen(code)==0 || list==NULL )
                return 0;
        curkey = (U16)GetHash(code);
        if( list[curkey].removeCode(code) )
            total--;
        return 1;
}

template< class Etype >
inline int WHash<Etype>::size() {return total; }


typedef XListArr ARR, *PARR;
bool makeTextValue(XParseVar& pv, StrVar* rst, TreeNode* tn=NULL );
bool parsePageNode(TreeNode* root, StrVar* var, int sp, int ep, TreeNode* base=NULL, U32 flags=0, StrVar* rst=NULL);
#endif // LISTOBJECT_H
