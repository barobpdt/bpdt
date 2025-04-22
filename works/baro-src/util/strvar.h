#ifndef STRVAR_H
#define STRVAR_H

#include "def.h"

int slen(LPCC str);
bool isnum(LPCC str, int* len=NULL, bool* chk=NULL);
bool isEquals(LPCC src, LPCC str, int len);
bool isOperation(int ch);
bool ccmp(LPCC code, LPCC data );
bool lowcmp(LPCC data, LPCC find, int size, int remain );

int StartWith( LPCC buf, LPCC cmp, int len=-1, int size=-1  );
int IndexOf( const char* str, char c, int size=0, int num=0 );
int LastIndexOf( LPCC str, char c, int size=0 );
char* Trim( char* src );

class ConstBuffer
{
public:
    char	*buffer;
    char	*position;
    size_t	size;
public:
    ConstBuffer(int size=0xFFFFF);
    ~ConstBuffer();
    int		length();
    char*	get();
    char*	get(int sz);
    void	reuse();
    void	create();
    char*	add(LPCC s, int sz=0);
    char*	alloc(int size);
    void	incr(int sz);
    char*	fmt(const char * fm, ... );
};

class StrVar
{
public:
    char		*buffer;
    char		*position;
    size_t		size;
public:
    StrVar();
    StrVar(LPCC value);
    ~StrVar();
    void setPointer(LPCC buf, int len);

    int		length();
    char*	get(int cur=0);
    StrVar*	reuse();
    void	create(int s=32);
    void	close();
    void	grow(int bufsize);

    StrVar&	set(LPCC s, int sz=-1);
    StrVar&	set(int n);
    StrVar& set(StrVar& str);
    StrVar& set(StrVar* str, int sp, int ep);


    StrVar&	add(LPCC s, int sz=-1);
    StrVar&	add(char c);
    StrVar&	add(int c);
    StrVar&	add(StrVar* str, int sp=0, int ep=0);

    char*	getbuffer(int bufsize);
    void	setpos( int pos);
    void	movepos( int movesize);
    char*	memalloc(int sz);
    char*	alloc(int sz);
    char*	format(int sz, const char * fmt, ... );
    char*	ftime(const char * fmt, struct tm* time_s);
    char*	fmt(const char * fm, ... );

    char charAt( int pos, int end=0 );
    void setAt( int pos, char value=0, int end=0 );
    int findPos( LPCC find, int start=0, int end=-1, U32 flag=0, LPCC mask=NULL );
    char nextChar(int& sp, int len=0);

    int nextWord(int& s, int& ep, int end=0);

    int indexOf(char find, int sp=0, int ep=0);
    int lastIndexOf(char c, int s=0, int e=-1 );
    int find( LPCC txt, int start=0, int end=0, U32 flag=0 ) ;
    int trim(int& sp, int& ep);
    LPCC trimv(int sp, int ep);
    LPCC get(int sp, int ep);
    LPCC getvalue();
    LPCC getBuffer(char* buf, int bufsize, int sp, int  ep);
    void trim();
    char* replace(char ch, char re);

    int remove(int pos, int cnt);
    int insert(int pos, LPCC str, int cnt );
    bool cmp(int sp, int ep, LPCC buf, int bufsize=0);
    bool cmp(LPCC buf);
    void incr(int len);
    bool checkVar(char ch);
    bool convert(int sp=0, int ep=0);

    void object(LPVOID buf, int size=sizeof(LPVOID));

    int savefile(LPCC filenm);
    char* readFile( LPCC filename, int* size=NULL );

    StrVar& addU16(U16 n, int pos=-1);
    StrVar& addU32(U32 n, int pos=-1);
    StrVar& addUL64(UL64 n, int pos=-1);
    StrVar& addObject(LPVOID obj, int pos=-1);
    StrVar& addInt(int n, int pos=-1);
    U16 getU16(int pos=0);
    U32 getU32(int pos=0);
    int getInt(int pos=0);
    UL64 getUL64(int pos=0);
    LPVOID getObject(int pos);
    StrVar& setVar(char c, U16 f, LPVOID p );
    StrVar& setVar(char c, U16 f );
    StrVar& addDouble(double n, int pos=-1);
    double getDouble(int pos);

    StrVar& putU16(U16 n, int pos);
    StrVar& putU32(U32 n, int pos);
    StrVar& putUL64(UL64 n, int pos);
    StrVar& putDouble(double n, int pos);
    StrVar& putObject(LPVOID obj, int pos);
    StrVar& putInt(int n, int pos);
};

class ParseVar
{
public:
    StrVar*				pvar;
    int cur;
    int prev;
    int start;
    int wsp;
    int wep;
public:
    ParseVar(StrVar* var=NULL, int sp=0, int ep=0);
    ~ParseVar();

    int size();
    LPCC get(int pos=-1, int* size=NULL);
    int remain();
    void SetPosition(int sp, int ep=0);
    void SetPosition();

    void SetVar(StrVar* var, int sp=0, int ep = -1 ) ;
    void SetAt(int pos, char ch=0);
    char CharAt(int pos);
    LPCC GetChars(int sp, int ep=-1);
    LPCC NextWord(int ep=0);
    char NextChar(int blankcheck=0);
    char GetChar(int dist=0);
    BOOL IsNextChar(char c);


    BOOL FindNext(LPCC str, int* ep=NULL);
    char PrevChar(int ep=-1);
    LPCC PrevWord(int ep=-1);


    int IndexOf(char c);
    int IndexOf(char c, int sp, int ep);
    bool MatchWordPos( LPCC st, LPCC et, U32 flag=0 );
    LPCC value(int sp, int ep=0);
    LPCC value();
    int FindPos( LPCC find, int ep=-1, U32 flag=0, LPCC mask=NULL );
    LPCC GetBuffer(int sp=-1);
    LPCC Trim(int sp, int ep);
    StrVar* GetVar();
    char*	Convert(int sp=0, int ep=0);
    void	reuse();
    // bool	NextPos();
    bool	NextMove();
    bool	MatchStringPos( char ch );
    BOOL	StartWith(LPCC word, int pos=-1, U32 flag=0 );

    //
    // utils
    //
    LPCC	v(int sp=-1, int ep=-1);
    LPCC	GetValue();
    LPCC	GetValue(int pos);
    int		GetSize();
    int		subSize();
};


class XParseVar : public ParseVar  {
public:
    XParseVar();
    XParseVar(StrVar* var, int sp=0, int ep=0);
    XParseVar(XParseVar& pv);
public:
    void append(StrVar* var);
    void append(LPCC buf, int size=-1);
    bool findNext(LPCC find, U32 flag=FIND_FORWORD );
    int findPos(LPCC find, int ep=0, U32 flag=FIND_FORWORD );
    int findPrev(LPCC find, int ep=0, int sp=0, U32 flag=FIND_FORWORD );
    int find(LPCC find, LPCC mask=NULL, U32 flag=0, int sp=-1, int ep=-1 );
    bool valid();
    LPCC left(int* size=NULL);
    LPCC right(int* size=NULL);
    LPCC getValue(int* size=NULL);
    LPCC getBuffer(char* buf, int buflen, int sp, int ep);
    XParseVar& findEnd(LPCC find, U32 flag=FIND_FORWORD );
    XParseVar& moveNext(int ep=0);
    XParseVar& movePos(int pos);
    XParseVar& nextCheck(LPCC chk);
    LPCC getNext(int* wsize=NULL, int num=0);

    XParseVar& incr();
    bool	startString(LPCC find, U32 flag=0);
    char	prevChar(int ep=0, int* pos=NULL);
    bool	match( LPCC st, LPCC et, UINT flag=0, LPCC mask=NULL );
    char ch(int dist=0);
    XParseVar& setPos(int pos=-1);
    int charCount(char ch);
    void setSub(XParseVar& pv );
    int startPos() {
        ch();
        return start;
    }
};

extern ConstBuffer		gBuf;

#endif // STRVAR_H
