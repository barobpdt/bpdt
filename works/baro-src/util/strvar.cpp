#include "strvar.h"
#include <QDebug>
#include "../fobject/FMutex.h"

FMutex gMutexStrVarGrow;

ConstBuffer		gBuf;

bool isEquals(LPCC src, LPCC str, int len) {
    if( src==NULL || str==NULL ) return false;
    while( len>0 ) {
        if( *src==0 || *str==0 || (*str++ != *src++) ) return false;
        len--;
    }
    return true;
}

bool isOperation(int ch) {
    return
    ( ch<0 || ch==95 ) ? false :
    ( ch<48 || (ch>57 && ch<65 ) || (ch>90 && ch<97 ) || ch>122 ) ? true : false;
}

bool lowcmp(LPCC data, LPCC find, int size, int remain ) {
    if( size==0 )
        size = slen(find);
    int n=0;
    if( size>0 && remain>=size && data )
    {
        for( ; n<size && *find; n++) if(tolower(*find++)!=tolower(*data++) ) return false;
    }
    return (n==size) ? true: false;
}


int slen(LPCC str) {
    return (str)? strlen(str) : 0;
}



bool isnum(LPCC str, int* len, bool* chk) {
    if( str==NULL || *str==0 )
        return false;
    LPCC tmp = str;
    if( *str=='+' || *str=='-' ) {
        str++;
    }
    int cnt=0;
    if( chk ) *chk = false;
    for (int c=0; c<32 && *str; c++, str++)  {
        if( IS_DIGIT(*str) ) {
            continue;
        } else if(*str=='.' ) {
            if( c==0 || cnt>0 ) return false;
            if( chk ) *chk = true;
            cnt++;
            continue;
        } else
            return false;
    }
    if( len ) *len = str-tmp;
    return str-tmp>0 ? true: false;
}
int StartWith( LPCC buf, LPCC cmp, int len, int size  )
{
    if( buf==NULL || cmp==NULL ) return 0;
    if( len==-1 ) len = slen(cmp);
    if( size==-1 ) {
        LPCC tmp = buf;
        size = 0;
        while( *tmp++ ) if( size++>len ) break;
    }
    return ( size>=len && memcmp(buf, cmp, len )==0 ) ? len: 0;
}

int IndexOf( const char* str, char c, int size, int num )
{
    if( str==NULL || *str==0 ) return -1;
    int n = 0 ;
    if( size==0 ) size = strlen(str);
    for( int i=0 ; i<size ; i++ )
    {
        if( str[i]==0) break;
        if( str[i]==c )
        {
            if( n==num) return i;
            n++ ;
        }
    }
    return -1 ;
}

int LastIndexOf( LPCC str, char c, int size ) {
    if( str==NULL || *str==0 ) return -1 ;
    if( size==0 ) size = strlen( str ) ;
    for( int i=size-1 ; i>=0 ; i-- )
    {
        if( str[i]==c ) return i ;
    }
    return -1 ;
}


char* Trim( char* src ) {
    if( src==NULL) return NULL ;
    if( src[0]==0) return src ;  //(na) 0817
    int len = slen( src ) ;
    int i = 0 , ii = 0 , num = 0, size ;
    BOOL bChk = FALSE ;
    for( i=len-1 ; i>=0; i-- )
    {
        if( ISBLANK(src[i]) ) num++ ;
        else break ;
    }
    size = len-num ;
    if( size==0 )
    {
        src[0] = 0 ;
        return src ;
    }
    for( i=0; i<size; i++ )
    {
        if( bChk==FALSE )
        {
            if( ISBLANK(src[i]) ) continue ;
            bChk = TRUE ;
        }
        if( bChk )
        {
            src[ii++] = src[i] ;
        }
    }
    src[ii] = 0 ;
    return src ;
}
bool ccmp(LPCC code, LPCC data ) {
    return ( code && data && strcmp(code,data)==0 ) ? true : false;
}


ConstBuffer::ConstBuffer(int s): buffer(NULL), position(NULL) {
    if( s==0x2FFF ) s=0x2FFFF;
    size=s;
    create();
}

ConstBuffer::~ConstBuffer()
{
    SAFE_FREE(buffer);
    buffer=position=NULL;
}

int ConstBuffer::length()
{
    return (position&&buffer) ? (int)(position-buffer):0;
}

char* ConstBuffer::get()
{
    return buffer;
}
void ConstBuffer::reuse()
{
    position = buffer;
}
void ConstBuffer::create()
{
    buffer =(char *) malloc( size );
    reuse();
}

char* ConstBuffer::add(LPCC s, int sz) {
    if( s==NULL || sz>=size ) return NULL;
    if( sz==0 ) sz = strlen(s);
    if( length()+sz >= size ) reuse();
    char* str = position;
    memcpy(position,s,sz);
    position+=sz;
    *position++ = 0;
    return str;
}
char* ConstBuffer::alloc(int sz)
{
    int remain = size - (position-buffer);
    if( remain < sz )
        reuse();
    return position;
}

char* ConstBuffer::get(int sz)
{
    int remain = size - (position-buffer);
    if( remain < sz )
        reuse();
    char* str = position;
    position+=sz;
    *position++ = 0;
    return str;
}

char* ConstBuffer::fmt(const char * fm, ... ) {
    int remain = size - (position-buffer);
    if( remain < 256 )
        reuse();
    char* buf=position;
    va_list ap;
    va_start(ap, fm);
    vsprintf(buf, fm, ap);
    va_end(ap);
    position += strlen(buf);
    *position++ = 0;
    return buf;
}

void ConstBuffer::incr(int sz)
{
    int remain = size - (position-buffer);
    if( remain < sz ) reuse();
    position+=sz;
    *position++ = 0;
}


StrVar::StrVar() : buffer(NULL), position(NULL), size(0)
{
}

StrVar::StrVar(LPCC value) : buffer(NULL), position(NULL), size(0)
{
    if( value )
    {
        create( slen(value) );
        if(size) add(value);
    }
}


StrVar::~StrVar()
{
    close();
}

int		StrVar::length()
{
    return (position&&buffer)?(int)(position-buffer):0;

}
char*	StrVar::get(int cur) {
    /*(na)
    if( size>0xFFFFFF ) {
        return NULL;
    }
    */
    if( buffer==NULL )
        return NULL;
    *position ='\0';
    if( cur==0 )
        return buffer;
    if( abs(cur)>=size ){
        return NULL;
    }
    return cur>0 ? buffer+cur : position+cur;
}


char*	StrVar::getbuffer(int offset)
{
    if( size==0 )
        create(64);
    if( offset>=size ) {
        grow(offset);
    }
    return buffer + offset;
}

void	StrVar::movepos( int movesize)
{
    if( (length()+movesize) < size )
        position += movesize;
}
void	StrVar::setpos( int pos)
{
    if( pos>=length() )
        grow(pos-length()+2);
    position = buffer+pos;
}

StrVar*	StrVar::reuse()
{
    position = buffer;
    if( buffer ) *buffer = 0;
    return this;
}
void	StrVar::create(int s)
{
    if( buffer )
    {
        reuse();
        return;
    }
    for( int n=1;n<24;n++ )
    {
        if( s > (1<<n) ) continue;
        size = 1<<n;
        break;
    }
    if(size)
    {
        buffer = (char *) malloc( size );
        reuse();
    }
}
void StrVar::close()
{
    if( size==0 ) return;
    SAFE_FREE(buffer);
    reuse();
}
void StrVar::grow(int bufsize)
{
    int pos = length();
    if( size > (pos + bufsize) )
        return;
    if( size==0 )
        create(64);
    if( size>0 ) {
		gMutexStrVarGrow.EnterMutex();
        while( ((size_t)pos+bufsize+4) >= size ) {
            size = size *2;
        }

		if (void* mem = std::realloc(buffer, size)) {
			buffer = static_cast<char*>(mem);
			position = buffer + pos;
		} else {
			close();
			create(size);
			qDebug("#0#strvar realloc fail size:%d", (int)size);
		}
		gMutexStrVarGrow.LeaveMutex();
    }
}

StrVar& StrVar::set(LPCC s, int sz)
{
    reuse();
    if( sz>0xFFFFF || sz==0 ) {
        return *this;
    }
    if( sz>0 ) {
        add(s, sz);
    } else if( sz==-1 ) {
        add(s,slen(s));
    } else if( sz==-2 ) {
        sz=slen(s);
        for( int n=0; n<sz; n++ ) {
            if( s[n]==-30 && ((sz-n)>2) && s[n+1]==-128  && s[n+2]==-87 ) {
                add('\n');
                n+=2;
            } else {
                add(s[n]);
            }
        }
    }
    return *this;
}

StrVar& StrVar::set(int n)
{
    reuse();
    add(n);
    return *this;
}

StrVar& StrVar::set(StrVar& str) {
    reuse();
    add( str.get(), str.length());
    return *this;
}

StrVar& StrVar::set( StrVar* str, int sp, int ep ) {
    if( str==NULL || sp>=ep ) return *this;
    reuse();
    add( str, sp, ep);
    return *this;
}

StrVar&	StrVar::add(LPCC s, int sz)
{
    if( s ) {
        if( sz==-1 )
            sz = slen(s);
        if( sz>0 ) {
            if( buffer==NULL || size==0  )
                create( 64 );
            grow(sz);
            memcpy(position,s,sz);
            position+=sz;
        }
    }
    return *this;
}
StrVar& StrVar::add(char c)
{
    if( buffer==NULL || size==0  )
        create( 64 );
    grow(2);
    *(position++) = c;
    return *this;
}
StrVar&	StrVar::add(int c)
{
    format(16,"%d",c);
    return *this;
}

StrVar& StrVar::add(StrVar* str, int sp, int ep)
{
    if( sp<0 ) return *this;
    if( str && str->buffer )
    {
        if( ep==0 )
            ep = str->length();
        // version 1.0.4 this != str
        if( sp<ep && this != str )
            add( str->buffer + sp, ep - sp);
    }
    return *this;
}


char*	StrVar::memalloc(int sz)
{
    grow(length()+sz);
    return position;
}

char*	StrVar::alloc(int sz)
{
    if( sz>size ) {
        grow(sz);
    }
    return buffer;
}


char* StrVar::format(int sz, const char * fmt, ... )
{
    if( buffer==NULL || size==0  )
        create( sz );
    char* buf = memalloc(sz);
    va_list ap;
    va_start(ap, fmt);
    vsprintf( buf, fmt, ap);
    va_end(ap);
    position += slen(buf);
    return buf;
}
char* StrVar::fmt(const char * fm, ... )
{
    if( buffer==NULL || size==0  )
        create( 512 );
    reuse();
    char* buf = memalloc(512);
    va_list ap;
    va_start(ap, fm);
    vsprintf(buf, fm, ap);
    va_end(ap);
    position += slen(buf);
    return buf;
}

char* StrVar::ftime(const char * fmt, struct tm* time_s )
{
    if( fmt==NULL || time_s==NULL )
        return NULL;
    if( buffer==NULL || size==0  )
        create( 64 );
    reuse();
    char* buf = memalloc(64);
    strftime( buf, 64, fmt, time_s ); // "%a, %d %b %Y %H:%M:%S GMT"
    position += slen(buf);
    return buf;
}

char* StrVar::readFile( LPCC filename, int* len )
{
    if( filename==NULL)
    {
        if( len ) *len=0;
        return NULL;
    }
    FILE* fp = fopen( filename, "r" ) ;
    if( len ) *len = 0 ;

    // int filelen = GetFileLength( fp ) ;
    reuse();
    if( fp )
    {
        int read = 0 ;
        char buf[2048];
        while(true )
        {
            int r = fread( buf, 1, 2048, fp ) ;
            if( r<=0 ) break ;
            add(buf,r);
            read+= r ;
            if( r<2048 ) break ;
        }
        if( len ) *len = read ;
        fclose(fp);
        return get() ;
    }
    return NULL ;
}
char StrVar::charAt( int pos, int end)
{
    if( end==0 ) end = length();
    if( pos<0 ) pos = end+pos;
    return (buffer && pos>=0 && pos<end ) ? buffer[pos]: 0;
}
void StrVar::setAt( int pos, char value, int end)
{
    if( end==0 ) end = length();
    if( pos<0 ) pos = end+pos;
    if( pos<0 || pos>end ) return;
    buffer[pos] = value;
}



int StrVar::findPos( LPCC find, int start, int end, U32 flag, LPCC mask )
{
    if( !find || find[0]==0 )
        return -1;

    if( end==-1 || end>length() ) {
        end = length();
        if( end<0 || end> this->size ) {
            return -1;
        }
    }
    // bool chk=ccmp(find,"</define>");
    if( start==end ) {
        return -1;
    }
    int s = min(start, end);
    int e = max(start, end);

    int p = 0;
    char* str = get(); //+ s;
    if( str==NULL )
        return -1;
    int lenfind = flag&FIND_CHARS ? 1 : strlen( find );
    /*
    if( ccmp(find," ") )
        flag |= FIND_BLANK;
    */
    if( s + lenfind > e )
        return -1;
    bool skip = false, wchk = true;
    char sch = 0,
         ch =0;

    if( start < end ) {
        end-=(lenfind-1);
        for( int n=s; n<end; n++ ) {
            ch = str[n];
            if( flag&FIND_SKIP ) {
                if( ch=='\'' || ch=='"' ) {	//
                    if( skip ) {
                        if( sch==ch ) {
                            if( sch=='\'' ) {
                                skip = false;
                            } else if( n>1 ) {
                                if( str[n-1]!='\\' )
                                    skip = false;
                                else if(str[n-2]=='\\' )
                                    skip = false;
                            }
                        }
                    } else {
                        sch = ch;
                        skip = true;
                    }
                }
                if( skip ) {
                    if( ch=='\n' ) skip = false;
                    continue;
                }
                if( flag&FIND_NOCASE ) {
                    if( lowcmp(str+n,find,lenfind, e-n) ) {
                        if( flag&FIND_FUNCSTR ) {
                            p = n+lenfind;
                            char c=str[p];
                            if( ISBLANK(c) || c=='(' || c=='>' ) return n;
                        }
                        else
                            return n;
                    }
                } else if( ch==find[0] && memcmp( str+n, find, lenfind )==0 ) {
                    if( flag&FIND_WORD ) {
                        if( n==0 ) {
                            wchk = true;
                        } else {
                            int sp = n-1;
                            wchk = ISBLANK(str[sp]) || IS_OPERATION(str[sp]) ? true: false;
                        }
                        if( wchk ) {
                            p = n+lenfind;
                            if( (p<e) && (ISBLANK(str[p]) || IS_OPERATION(str[p])) ) {
                                return n;
                            }
                        }
                    } else if( flag&FIND_FUNCSTR ) {
                        p = n+lenfind;
                        char c=str[p];
                        if( ISBLANK(c) || c=='(' || c=='>' ) return n;
                    }
                    else
                        return n;
                }
            } else if( flag&FIND_CHARS ) {
                LPCC buf = find;
                while( *buf ) {
                    if( *buf++==ch)
                        return n;
                }
            } else if( flag&FIND_NOCASE ) {
                if( lowcmp(str+n,find,lenfind, e-n) ) {
                    if( flag&FIND_FUNCSTR ) {
                        p = n+lenfind;
                        char c=str[p];
                        if( ISBLANK(c) || c=='(' || c=='>' ) return n;
                    }
                    else
                        return n;
                }
            } else if( ch==find[0] && memcmp( str+n, find, lenfind )==0 ) {
                if( flag&FIND_WORD ) {
                    if( n==0 ) {
                        wchk = true;
                    } else {
                        int sp = n-1;
                        wchk = ISBLANK(str[sp]) || IS_OPERATION(str[sp]) ? true: false;
                    }
                    if( wchk ) {
                        p = n+lenfind;
                        if( (p<e) && (ISBLANK(str[p]) || IS_OPERATION(str[p])) ) {
                            return n;
                        }
                    }
                } else if( flag&FIND_FUNCSTR ) {
                    p = n+lenfind;
                    char c=str[p];
                    if( ISBLANK(c) || c=='(' || c=='>' ) return n;
                }
                else
                    return n;
            }
        }
    } else {
        char c = find[lenfind-1];
        for( int n=e-1; n>=s; n-- ) {
            int offset = n-lenfind+1;
            if( offset<0 )
                break;
            ch = str[n];


            if( flag&FIND_SKIP ) {
                if( ch=='\'' || ch=='"' ) {
                    if( skip ) {
                        if( sch==ch && (n>0&&str[n-1]!='\\')  )
                            skip = false;
                    } else {
                        sch = ch;
                        skip = true;
                    }
                }
                if( skip ) {
                    if( ch=='\n' )
                        skip = false;
                    continue;
                }
                if( flag&FIND_NOCASE ) {
                    if( lowcmp(str+offset,find,lenfind, e-offset) )
                        return offset;
                } else if( ch==c ) {
                    if( memcmp( str+offset, find, lenfind )==0 ) return offset;
                }
            } else if( flag&FIND_NOCASE ) {
                if( lowcmp(str+offset,find,lenfind, e-offset) )
                    return offset;
            } else if( flag&FIND_CHARS ) {
                LPCC buf = find;
                while( *buf ) { if(*buf++==ch) return offset; }
            } else if( ch==c && ((n+1)-lenfind)>=0 ) {
                if( memcmp( str+offset, find, lenfind )==0 ) {
                    if( flag&FIND_WORD ) {
                        int sp = offset;
                        if( sp==0 ) {
                            wchk = true;
                        } else {
                            p = sp-1;
                            wchk = ISBLANK(str[p]) || IS_OPERATION(str[p]) ? true: false;
                        }
                        if( wchk ) {
                            p = sp+lenfind;
                            if( (p<e) && (ISBLANK(str[p]) || IS_OPERATION(str[p])) ) {
                                return n;
                            }
                        }
                    } else if( flag&FIND_FUNCSTR ) {
                        p = offset+lenfind;
                        char c=str[p];
                        if( ISBLANK(c) || c=='(' || c=='>' ) return offset;
                    } else
                        return offset;
                }
                // n+=lenfind;
            }
        }

    }
    return -1;
}

char StrVar::nextChar(int& sp, int len)
{
    if(len==0) len =length();
    char c= charAt(sp,len);
    if( !ISBLANK(c) ) return charAt(sp);
    while( sp<len )
    {
        c = charAt(sp,len);
        if( !ISBLANK(c) ) break;
        sp++;
    }
    return c;
}


int StrVar::nextWord(int& s, int& ep, int end)
{
    if(s>=ep ) return -1;
    if(end==0) end=length();
    char c = nextChar(s,end);
    int sp = s;
    while(sp<=end)
    {
        c = charAt(sp);

        if( sp!=s && ( ISBLANK(c) || IS_OPERATION(c) )  )
        {
            ep = sp;
            return ep;
        }
        sp++;
    }
    return -1;
}

int StrVar::indexOf(char find, int sp, int ep)
{
    if( ep==0 ) ep = length();
    if( ep>length() ) ep = length();
    for( int n=sp; n<ep; n++)
    {
        if( charAt(n)==find) return n;
    }
    return -1;
}
int StrVar::lastIndexOf(char c, int s, int e )
{
    if( e==-1 )
        e = length();
    if( s >= e )
        return -1;
    if( e==length() ) e--;
    for( int n=e ; n>=s ; n-- )
    {
        if( charAt(n)==c )
            return n;
    }
    return -1 ;
}

int StrVar::find( LPCC txt, int start, int end, U32 flag )
{
    if (txt==NULL || start<0 ) return -1;
    if( end==0 ) end = length();
    int lPos = findPos(txt, start, end, flag);
    if (lPos >= 0)
    {
        return lPos;
    }
    return -1;
}

void StrVar::trim()
{
    char* cur = position;
    cur--;
    for(int n=0;n<1024 && cur; n++) {
        if( ISBLANK( *cur ) )
            cur--;
        else
            break;
    }
    int dist = position - cur;
    if( dist>1 ) cur++;
    position = cur;
}

int StrVar::trim(int& sp, int& ep)
{
    if( ep>length() ) return 0;
    for(int n=0;n<1024 && (sp+1)<ep; n++)
    {
        if( ISBLANK( charAt(sp) ) ) sp++;
        else break;
    }
    if( ISBLANK( charAt(ep-1)) )
    {
        for(int n=0;n<1024 && sp<ep; n++)
        {
            //Log("###############>(%c)\n", charAt(ep) );
            if( ISBLANK( charAt(ep-1) ) ) ep--;
            else break;
        }
    }
    return sp;
}

LPCC StrVar::trimv(int sp, int ep)
{
    trim(sp,ep);
    return get(sp,ep);
}

LPCC StrVar::get(int sp, int ep)
{
    if( buffer==NULL )
        return NULL;
    if( ep < 0 ) {
        ep=length()+ep;
    }
    if( ep >=length() ) {
        return get(sp);
    } else if( sp<ep ) {
        int len = ep-sp;
        char* buf = gBuf.get(len+1);
        memcpy( buf, buffer+sp, len);
        buf[len] = 0;
        return buf;
    }
    return NULL;
}
LPCC StrVar::getBuffer(char* buf, int bufsize, int sp, int ep) {
    if( buffer==NULL || buf==NULL )
        return NULL;
    if( ep < 0 ) {
        ep=length();
    }
    if( sp<ep ) {
        if( ep >=length() ) return get(sp);
        int len = ep-sp;
        if( len < bufsize ) {
            memcpy( buf, buffer+sp, len);
            buf[len] = 0;
        }
    } else {
        buf[0]=0;
    }
    return buf;
}

LPCC StrVar::getvalue()
{
    return get(0,length() );
}


char* StrVar::replace(char ch, char re)
{
    if( buffer==NULL )
        return NULL;
    char* cur = buffer;
    for( ; cur<position; cur++ ) if(*cur==ch) *cur=re;
    return get();
}

bool StrVar::convert(int sp, int ep)
{
    if( buffer==NULL )
        return FALSE;
    if( sp==0 && ep==0 ) {
        sp = 0;
        ep = length();
    }
    if( ep>length() ) {
        ep = length();
    }
    char *start, *end, *cur, *txt;
    start = cur = txt = buffer + sp;
    end = buffer + ep;
    for( ; *cur && cur<end; cur++ )
    {
        if( *cur=='\\' ) {
            cur++;
            switch(*cur)
            {
            case '\\': *txt++ = '\\'; break;
            case 'r': *txt++ = '\r'; break;
            case 'n': *txt++ = '\n'; break;
            case 't': *txt++ = '\t'; break;
            case 'b': *txt++ = '\b'; break;
            case 's': *txt++ = ' '; break;
            case 'S': *txt++ = '$'; break;
            case 'c': *txt++ = ','; break;
            case 'z': *txt++ = ';'; break;
            case '<': *txt++ = '['; break;
            case '>': *txt++ = ']'; break;
            default:
                {
                    *txt++ = '\\';
                    *txt++ = *cur;
                }
                break;
            }
        }
        else
            *txt++ = *cur;
    }
    if( txt<end )
    {
        end = (char*)(txt - start);
        end+=sp;
        while(txt<end)
            *txt++ = ' ';
        return TRUE;
    }
    return FALSE;
}

int StrVar::savefile(LPCC filenm)
{
    FILE* fp = fopen(filenm,"w+");
    if( fp )
    {
        fwrite(buffer,1,length(),fp);
        fclose(fp);
        return 1;
    }
    return 0;
}

int StrVar::remove(int pos, int cnt)
{
    int len = length();
    if (cnt == 0 || len < pos+cnt) return 0;
    int movelen=position - (buffer+pos) + 1; //strlen(buffer+pos+cnt)+1;
    memmove(buffer+pos, buffer+pos+cnt, movelen);
    position = buffer+(len-cnt);
    *position = 0;
    return movelen;
}

int StrVar::insert(int pos, LPCC str, int cnt )
{
    int len = length();
    if( cnt==0 ) cnt = slen(str);
    if( pos == len )
    {
        add(str,cnt);
        return cnt;
    }
    if( pos<0 || pos>=len || str==NULL || cnt==0 ) return 0;
    grow(cnt+4);
    int movelen= position - (buffer+pos) + 1;
    memmove(buffer+pos+cnt,buffer+pos,movelen);
    memcpy(buffer+pos,str,cnt);
    position = buffer+len+cnt;
    // *position = 0;
    return movelen;
}

bool StrVar::cmp(int sp, int ep, LPCC buf, int bsize)
{
    if( buffer && sp>=0 ) {
        if( bsize==0 )
            bsize = slen(buf);
        while( sp<ep && bsize-->=0 )
            if( buffer[sp++]!=*buf++ ) return false;
        return true;
    }
    return false;
}
bool StrVar::cmp(LPCC buf)
{
    return ccmp(get(), buf);
}

void StrVar::incr(int len) {
    if( length()+len < size ) position+= len;
}

bool StrVar::checkVar(char ch) {
    return ( 2<length() && buffer[0]=='\b' && buffer[1]==ch ) ? true : false;
}
void StrVar::object(LPVOID buf, int size) {
    if( buffer==NULL)
        create(16);
    set((LPCC)&buf,size);
}
/*
LPVOID StrVar::object() {
    if( buffer==NULL ) return NULL;
    void** obj = (void**)buffer;
    return (LPVOID)(*obj);
}
*/
void StrVar::setPointer( LPCC buf, int len ) {
    buffer = (char*)buf;
    position = buffer+len;
}

StrVar& StrVar::addU16(U16 n, int pos) {
    if( pos!=-1 )
        setpos(pos);
    add((LPCC)&n,sizeof(U16));
    return *this;
}
StrVar& StrVar::addU32(U32 n, int pos) {
    if( pos!=-1 )
        setpos(pos);
    add((LPCC)&n,sizeof(U32));
    return *this;
}
StrVar& StrVar::addUL64(UL64 n, int pos) {
    if( pos!=-1 )
        setpos(pos);
    add((LPCC)&n,sizeof(UL64));
    return *this;
}
StrVar& StrVar::addDouble(double n, int pos) {
    if( pos!=-1 )
        setpos(pos);
    add((LPCC)&n,sizeof(double));
    return *this;
}
StrVar& StrVar::addObject(LPVOID obj, int pos) {
    if( pos!=-1 )
        setpos(pos);
    add((LPCC)&obj,sizeof(LPVOID));
    return *this;
}
StrVar& StrVar::addInt(int n, int pos) {
    if( pos!=-1 )
        setpos(pos);
    add((LPCC)&n,sizeof(int));
    return *this;
}

U16 StrVar::getU16(int pos) {
    if( buffer==NULL)
        create(8);
    U16 n = 0;
    if( pos+sizeof(U16) <= length() ) {
        memcpy((void*)&n, (buffer+pos), sizeof(U16));
    }
    return n;
}
U32 StrVar::getU32(int pos) {
    if( buffer==NULL)
        create(8);
    U32 n = 0;
    if( pos+sizeof(U32) <= length() ) {
        memcpy((void*)&n, (buffer+pos), sizeof(U32));
    }
    return n;
}
int StrVar::getInt(int pos) {
    if( buffer==NULL)
        create(8);
    int n = 0;
    if( pos+sizeof(int) <= length() ) {
        memcpy((void*)&n, (buffer+pos), sizeof(int));
    }
    return n;
}
UL64 StrVar::getUL64(int pos) {
    if( buffer==NULL)
        create(8);
    UL64 n = 0;
    if( pos+sizeof(UL64) <= length() ) {
        memcpy((void*)&n, (buffer+pos), sizeof(UL64));
    }
    return n;
}
double StrVar::getDouble(int pos) {
    if( buffer==NULL)
        create(16);
    double n = 0;
    if( pos+sizeof(double) <= length() ) {
        memcpy((void*)&n, (buffer+pos), sizeof(double));
    }
    return n;
}

LPVOID StrVar::getObject(int pos) {
    if( buffer==NULL)
        create(16);
    void** obj = (void**)(buffer+pos);
    return (LPVOID)(*obj);
}


StrVar& StrVar::putU16(U16 n, int pos) {
    if( size>=(pos+sizeof(U16)) )
        memcpy(buffer+pos,&n,sizeof(U16));
    return *this;
}
StrVar& StrVar::putU32(U32 n, int pos) {
    if( size>=(pos+sizeof(U32)) )
        memcpy(buffer+pos,&n,sizeof(U32));
    return *this;
}
StrVar& StrVar::putUL64(UL64 n, int pos) {
    if( size>=(pos+sizeof(UL64)) )
        memcpy(buffer+pos,&n,sizeof(UL64));
    return *this;
}
StrVar& StrVar::putDouble(double n, int pos) {
    if( size>=(pos+sizeof(double)) )
        memcpy(buffer+pos,&n,sizeof(double));
    return *this;
}
StrVar& StrVar::putObject(LPVOID obj, int pos) {
    if( size>=(pos+sizeof(LPVOID)) )
        memcpy(buffer+pos,&obj,sizeof(LPVOID));
    return *this;
}
StrVar& StrVar::putInt(int n, int pos) {
    if( size>=(pos+sizeof(int)) )
        memcpy(buffer+pos,&n,sizeof(int));
    return *this;
}


StrVar& StrVar::setVar(char c, U16 f, LPVOID p ) {
    reuse();
    if( p==NULL ) {
        add("\b").add('9').addU16(0);
    } else {
        add('\b').add(c).addU16(f).addObject(p);
    }
    return *this;
}
StrVar& StrVar::setVar(char c, U16 f ) {
    reuse();
    add('\b').add(c).addU16(f);
    return *this;
}



ParseVar::ParseVar(StrVar* var, int sp, int ep) : pvar(var)
{
    if( sp<0 ) sp=0;
    SetVar(var, sp, ep<0? 0: ep==0? -1: ep);
}

ParseVar::~ParseVar()
{

}

void ParseVar::reuse()
{
    pvar = NULL;
    wsp = cur = prev = start = wep = 0;
}

void ParseVar::SetVar(StrVar* var, int sp, int ep )
{
    reuse();
    if( var ) {
        pvar = var;
        // pbuf = var->get();
        if( sp<0 ) sp=0;
        if( ep<0 || ep>var->length() )
            ep = var->length();
        if( sp>ep )
            sp = ep;
        SetPosition(sp, ep );
    }
}

void ParseVar::SetPosition(int sp, int ep)
{
    wsp = cur = prev = start = sp;
    wep = ep==-1 && pvar ? pvar->length(): ep;
}
void ParseVar::SetPosition()
{
    cur = prev = start = wsp;
}

int ParseVar::size()
{
    return (wep>wsp ) ? wep - wsp : 0;
}

int ParseVar::subSize()
{
    return (cur>prev) ? cur - prev : 0;
}
LPCC ParseVar::v(int sp, int ep) {
    if( pvar ) {
        if( sp==-1 ) sp=prev;
        if( ep==-1 ) ep=cur;
        if( sp<wsp ) sp=wsp;
        if( ep>wep ) ep=wep;
        if( sp<ep ) {
            pvar->trim(sp, ep);
            return sp<ep ? pvar->get(sp,ep): NULL;
        }
    }
    return NULL;
}

LPCC ParseVar::get(int pos, int* size)
{
    if(pos==-1)
        pos = start;
    if( size ) {
        *size = wep>pos ? wep-pos : 0;
    }
    return GetChars(pos, wep);
}

int ParseVar::remain()
{
    return start<wep ? wep-start : 0;
}

char ParseVar::CharAt(int pos)
{
    return ( pvar && pos< wep ) ? pvar->charAt(pos,wep) : 0;
}

void ParseVar::SetAt(int pos, char ch)
{
    if( pvar && pos< wep )
        pvar->setAt(pos,ch,wep);
}
StrVar* ParseVar::GetVar() {
    return pvar;
}
LPCC ParseVar::GetBuffer(int sp)
{
    if( sp==-1 ) sp = wsp;
    return ( pvar && sp < wep ) ? pvar->get(sp) : NULL;
}

int ParseVar::FindPos( LPCC find, int ep, U32 flag, LPCC mask )
{
    if( ep==-1) ep = ( flag & FIND_BACK )  ? wsp: wep;
    int pos = pvar->findPos(find, start, ep, flag, mask);
    if( pos!=-1 )
        cur = pos;
    return pos;
}


LPCC ParseVar::GetChars(int sp, int ep)
{
    if( pvar==NULL ) return NULL;
    if( sp==-1 || sp<wsp )
        sp = wsp;
    if( ep==-1 || ep>wep )
        ep = wep;
    return ep>=pvar->length() ? pvar->get(sp): pvar->get(sp,ep);
}

LPCC ParseVar::NextWord(int ep)
{
    prev = start;
    if(ep==0 ) ep = wep;
    if(pvar && pvar->nextWord(start,ep)!=-1 )
    {
        LPCC rtn = GetChars(start, ep);
        cur = start = ep;
        return rtn;
    }
    return NULL; //GetChars(start, end);
}

bool ParseVar::NextMove()
{
    int e = wep;
    if( pvar && pvar->nextWord(start,e)!=-1 ) {
        prev = start;
        cur = start = e;
        return true;
    }
    return false; //GetChars(start, end);
}

char ParseVar::PrevChar(int ep )
{
    if( pvar==NULL ) return 0;
    int ps = ( ep==-1 ) ? start : ep;
    int pe = cur;
    if( cur < 1 ) return 0;
    char c = pvar->charAt(cur, wep);
    while( cur>0 )
    {
        if( ISBLANK(c) )
            cur--;
        else
            break;
        c = pvar->charAt(cur, wep);
    }
    ps = (pe==cur) ? cur-1 : cur;
    return pvar->charAt(ps, wep);
}

LPCC ParseVar::PrevWord(int ep)
{
    if( pvar==NULL ) return NULL;
    int pos = ( ep==-1 ) ? wep-1 : ep;
    int end=pos;
    char c = pvar->charAt(pos, wep);
    while( pos>=wsp ) {
        if( ISBLANK(c) || IS_OPERATION(c) ) {
            if( pos != end ) {
                pos++;
                break;
            }
        }
        pos--;
        c = pvar->charAt(pos, wep);
    }
    prev=pos;
    return GetChars(pos, end);
}

BOOL ParseVar::StartWith(LPCC word, int pos, U32 flag )
{
    int len = slen(word);
    if( pos==-1 ) pos = start;
    if( len==0 || (wep - pos) < len )
        return FALSE;
    LPCC line = pvar->buffer+pos;
    if( pvar && pvar->buffer && word[0]==CharAt(pos) && memcmp(line,word,len)==0 ) {
        if( flag ) {
            prev = pos;
            start = pos + len;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL ParseVar::FindNext(LPCC str, int* ep)
{
    if( pvar==NULL ) return FALSE;
    int epos = (ep) ? *ep : wep;
    int pos = pvar->find(str, start, epos);
    if( pos!=-1 )
    {
        prev = start;
        cur = pos;
        if(ep) *ep = pos;
        start = pos + slen(str);
        return TRUE;
    }
    return FALSE;
}


char ParseVar::GetChar(int dist)
{
    return ( pvar && start+dist<wep )  ? pvar->charAt(start+dist, wep ) : 0;
}

char ParseVar::NextChar(int blankcheck)
{
    if( pvar==NULL ) return 0;
    char c=0;
    cur = start;
    if( (blankcheck & BLANK_CHECK) && start<wep)
    {
        c = pvar->charAt(start,wep);
        if( ISBLANK(c) )
            return c;
    }
    else if(start<wep)
        c = pvar->nextChar(start, wep);
    if( blankcheck & MOVE_STARTPOS )
    {
        start++;
    }
    return (start<wep) ? c : 0;
}

BOOL ParseVar::IsNextChar(char c)
{
    if( pvar==NULL ) return FALSE;
    BOOL rtn = FALSE;
    if( c==' ' && ISBLANK(c) )
        rtn = TRUE;
    else if( c == pvar->charAt(start,wep) )
        rtn = TRUE;
    if( rtn )
        start++;
    return rtn;
}

int ParseVar::IndexOf(char c)
{
    if( pvar==NULL ) return -1;
    cur = start;
    int rtn = pvar->indexOf(c, start, wep );
    if( rtn !=-1)
    {
        cur = rtn;
        start = rtn+1;
    }
    return rtn;
}

int ParseVar::IndexOf(char c, int sp, int ep)
{
    return (pvar) ? pvar->indexOf(c,sp, ep) : -1;
}

bool ParseVar::MatchStringPos( char ch ) {
    int sp = start+1;
    char c=0, p=0;
    if( ch=='\'' ) {
        while( sp<wep ) {
            if( CharAt(sp)==ch ) {
                prev = start+1;
                start = sp+1;
                cur = sp;
                return true;
            }
            sp++;
        }
        return false;
    }
    while( sp<wep ) {
        c = CharAt(sp);
        if( c==ch ) {
            // version 1.0.3
            if( p=='\\' && (sp-start)>2 ) {
            } else {
                prev = start+1;
                start = sp+1;
                cur = sp;
                return true;
            }
        }
        if( p=='\\' && c=='\\' )
            p = 0;
        else
            p = c;
        sp++;
    }
    return false;
}


bool ParseVar::MatchWordPos( LPCC st, LPCC et, UINT flag )
{
    if( pvar==NULL ) return false;
    int e=wep;
    int stlen=slen(st);
    int etlen=slen(et);
    if( st==0 || et==0 )
        return false;

    if( flag & FIND_BACK ) // back
    {
        //char c=pvar->charAt(pe-1);
        //char d=pvar->charAt(pe-2);
        int sp, pos, epos, lev, fe ;
        sp = pos = prev;
        epos = lev = fe = 0;
        while( true)
        {
            char c = pvar->charAt(pos,wep) ;
            fe = pvar->find(st,pos,epos,flag) ;
            if(fe!=-1)
            {
                while( true)
                {
                    while(pos>fe)
                    {
                        bool end=true;
                        if( et[0]==pvar->charAt(pos,wep) )
                        {
                            if( etlen==1 || ccmp(et,GetChars(pos-etlen,pos)) )
                            {
                                lev++;
                                pos-=etlen;
                                end=false;
                            }
                        }
                        if(end) break;
                    }
                    int n = (pos==(cur-etlen)) ?
                        pvar->find(et,pos-1,fe,flag) :
                        pvar->find(et,pos,fe,flag) ;
                    if( n==-1 || pos==n ) break ;
                    lev++ ;
                    pos = n -1;
                }
            }
            else
                return false ;
            if( lev>0) lev--;
            if( lev==0|| pos==fe) break ;
            pos = fe-1 ;
            while(true)
            {
                bool end=true;
                if( st[0]==pvar->charAt(pos,wep) )
                {
                    if( stlen==1 || ccmp(st,GetChars(pos-stlen,pos)) )
                    {
                        if(--lev==0)
                        {
                            fe=pos;
                            break;
                        }
                        end=false;
                        pos-=stlen;
                    }
                }
                if(end) break;
            }
            if( lev==0) break;

        }
        cur = sp;
        prev = fe-1;
        return true ;
    }
    else
    {
        bool chk=ccmp(st,et);
        int pos = chk ? start+1 : start;
        if( !chk && isEquals(et,pvar->get(pos+1),etlen) ) {
            if( start==pos ) {
                prev = cur = (pos+1);
            } else {
                prev = pos;
                cur = pos+1;
            }
            start = cur+etlen;
            return true;
        }
        int epos = e;
        int fe =0;
        int lev = 0;

        if( flag & FIND_TAG )
        {
            int tmp = pvar->find("<",pos+1,epos,flag);
            if( tmp>0)
            {
                fe = pvar->find("/>", pos+1, tmp,flag);
                if(fe>0)
                {
                    e = fe;
                    return true ;
                }
            }
        }
        for( int x=0; x<512; x++ )
        {
            fe = pvar->find(et,pos,epos,flag) ;
            if(fe!=-1) {
                int prevNum=-1;
                while( true ) {
                    // Log("pos(%d, %d %d)\n",pos,fe,lev);
                    int n = pvar->find(st,pos,fe,flag) ;
                    if( prevNum==n || n==-1 ) break;
                    prevNum=n;
                    lev++ ;
                    pos = n+stlen ;
                }
            }
            else
                return false ;
            if(lev>0) lev--;
            if(lev==0 || pos>fe) break ;
            pos = fe+1;
            while(true)
            {
                bool isend=true;
                if( et[0]==pvar->charAt(pos,wep) )
                {
                    if( etlen==1 || isEquals(et, pvar->get(pos), etlen) ) // ccmp(et,GetChars(pos,pos+etlen)) )
                    {
                        if(--lev==0)
                        {
                            fe=pos;
                            break;
                        }
                        isend=false;
                        pos+=etlen;
                    }
                }
                if(isend) break;
            }
            if(lev==0) break;
        }
        prev = start + stlen;
        cur = fe;
        start = fe+etlen;
        //cur = start;
        //e = fe;
        return true;
    }
    return false ;
}

LPCC ParseVar::value(int sp, int ep)
{
    if( ep==0 ) ep = start;
    return (pvar) ? pvar->get(sp, ep) : NULL;
}

LPCC ParseVar::value()
{
    return (pvar) ? pvar->get(wsp, wep) : NULL;
}

LPCC ParseVar::Trim(int sp, int ep)
{
    if( pvar )
    {
        if( ep>wep ) ep = wep;
        if( sp>ep ) return NULL;
        pvar->trim(sp, ep);
        return GetChars(sp,ep);
    }
    return NULL;
}

LPCC ParseVar::GetValue()
{
    return ( pvar && wsp<wep) ? pvar->get(wsp, wep) : NULL;
}

LPCC ParseVar::GetValue(int pos)
{
    return ( pvar && pos<wep) ? pvar->get(pos) : NULL;
}
int ParseVar::GetSize()
{
    return ( pvar && wsp<wep) ? wep-wsp : 0;
}

XParseVar::XParseVar() : ParseVar(NULL,0,0) {
}

XParseVar::XParseVar(StrVar* var, int sp, int ep) : ParseVar(var,sp,ep)  {
}

XParseVar::XParseVar(XParseVar& pv) : ParseVar(pv.GetVar(),pv.prev,pv.cur)  {
}

void XParseVar::append(StrVar* var) {
    if( pvar ) {
        pvar->add(var,0);
        wep = pvar->length();
    }
}

void XParseVar::append(LPCC buf, int size) {
    if( size==-1 )
        size = slen(buf);
    if( pvar ) {
        pvar->add(buf,size);
        wep = pvar->length();
    }
}

int XParseVar::findPrev(LPCC find, int ep, int sp, U32 flag ) {
    if( pvar && find ) {
        if( ep==0 ) {
            if( start==wep )
                ep = wep - 1;
            else if( prev>0 && start==prev )
                ep = start<cur ? cur : start-1;
            else
                ep = start<prev ? cur: wep;
        }
        if( flag==0 )
            flag = FIND_SKIP;
        int pos = pvar->findPos(find,ep,sp,flag,NULL);
        if( pos!=-1 ) {
            prev = start;
            cur = pos;
            start = pos + slen(find);
        } else {
            start = 0;
            prev = cur;
        }
        return pos;
    }
    return -1;
}

bool XParseVar::findNext(LPCC find, U32 flag ) {
    if( pvar ) {
        flag|= FIND_SKIP;
        int pos = pvar->findPos(find,start,wep,flag,NULL );
        if( pos!=-1) {
            prev = start;
            cur = pos;
            start = cur + (flag&FIND_CHARS ? 1 : slen(find));
            return true;
        }
    }
    return false;
}

int XParseVar::findPos(LPCC find, int ep, U32 flag ) {
    if( pvar ) {
        if( ep==0 )
            ep = wep;
        if( ep<=start )
            return -1;
        if( flag==0 )
            flag = FIND_SKIP;
        int pos = pvar->findPos(find,start,ep,flag,NULL );
        if( pos!=-1) {
            prev = start;
            start = cur = pos;
            return pos;
        }
    }
    return -1;
}

XParseVar& XParseVar::findEnd(LPCC find, U32 flag ) {
    if( pvar ) {
        if( flag==0 )
            flag = FIND_SKIP;
        int pos = pvar->findPos(find,start,wep,flag,NULL );
        cur = ( pos!=-1 ) ? pos : wep;
        prev = start;
        start = cur + (flag&FIND_CHARS ? 1 : slen(find));
    }
    return *this;
}

int XParseVar::find(LPCC find, LPCC mask, U32 flag, int sp, int ep ) {
    if( pvar ) {
        if( sp==-1 )
            sp = start;
        if( ep==-1 )
            ep = wep;
        if(flag==0 )
            flag|= FIND_FORWORD;
        return pvar->findPos(find,sp,ep,flag, mask);
    }
    return -1;
}


bool XParseVar::valid() {
    return (start<wep) ? true : false;
}

LPCC XParseVar::left(int* size) {
    int sp = cur<prev ? wsp: prev;
    if( pvar==NULL || cur==prev )  {
        if( size )
            *size = 0;
        return NULL;
    }
    if( size ) {
        int ep = cur;
        pvar->trim(sp,ep);
        *size = ep>sp ? ep-sp: 0;
    }
    return size ? pvar->get(sp) :  pvar->get(sp,cur);

}

LPCC XParseVar::right(int* size) {
    if( pvar==NULL || wep<=start ) {
        if( size )
            *size = 0;
        return NULL;
    }
    if( size )
        *size = wep>start ? wep-start: 0;
    return size ? pvar->get(start) :  pvar->get(start, start>prev ? wep: prev);
}

LPCC XParseVar::getValue( int* size  ) {
    if( pvar==NULL || wsp>=wep )
        return NULL;
    if( size )
        *size = wep-wsp;
    return size ? pvar->get(wsp) :  pvar->get(wsp,wep);
}
LPCC XParseVar::getBuffer(char* buf, int buflen, int sp, int ep) {
    if( pvar && sp>=wsp && ep<=wep ) {
        return pvar->getBuffer(buf, buflen, sp, ep);
    }
    return NULL;
}
char XParseVar::ch(int dist) {
    int pos = dist<0 ? wep+dist : start+dist;
    if( pos<0 )
        pos = 0;
    if( pvar==NULL )
        return 0;

    char c= pvar->charAt(pos,wep);
    if( dist!=0 || c==0 )
        return c;
    if( !ISBLANK(c) )
        return c;
    start++;

    while( start<wep) {
        c = pvar->charAt(start,wep);
        if( !ISBLANK(c) )
            break;
        start++;
    }
    return ISBLANK(c)? 0: c;
}

XParseVar& XParseVar::movePos( int pos ) {
    if( start+pos<wsp || start+pos > wep ) {
        prev = start;
        return *this;
    }
    pos+= start;
    prev = start;
    start = cur = pos;
    return *this;
}

XParseVar& XParseVar::setPos( int pos ) {
    prev = start;
    start = pos<wsp? wsp: pos>wep? wep : pos;
    return *this;
}



XParseVar& XParseVar::nextCheck(LPCC chk) {
    char c = ch();
    while( *chk ) {
        if( *chk++==c ) {
            start++;
            break;
        }
    }
    return *this;
}


LPCC XParseVar::getNext( int* wsize, int num ) {
    int ep = wep;
    int sp = start+num;
    if( wsize ) {
        *wsize = 0;
    }
    if( pvar && pvar->nextWord(sp,ep)!=-1 ) {
        if( wsize && ep>sp ) {
            *wsize = ep-sp;
        }
        LPCC rtn = wsize ? GetBuffer(sp) : GetChars(sp, ep);
        prev = sp;
        cur = start = ep;
        return rtn;
    }
    return NULL;
}



XParseVar& XParseVar::incr() {
    start++;
    return *this;
}
bool XParseVar::startString(LPCC find, U32 flag) {
    int len = slen(find);
    LPCC str = GetBuffer(start);
    if( len<wep-start ) {
        for( int n=0;n<len;n++) {
            if( flag&1 ) {
                if( tolower(find[n])!=tolower(str[n]) ) return false;
            } else {
                if( find[n]!=str[n] ) return false;
            }
        }
        return true;
    }
    return false;
}


XParseVar& XParseVar::moveNext(int ep)
{
    int e = ep>0 ? ep : wep;
    if( pvar )
        pvar->nextWord(start,e);
    start = e;
    return *this; //GetChars(start, end);
}


char XParseVar::prevChar(int ep, int* pos ) {
    int ps = ep==0? start-1 : ep>wsp && ep<wep ? ep :  wep-1;
    char c = pvar->charAt(ps, wep);
    while( ps>=0 )
    {
        if( ISBLANK(c) )
            ps--;
        else
            break;
        c = pvar->charAt(ps, wep);
    }
    if( pos && ps>=0 )
        *pos = ps;
    prev=ps;
    return c;
}

int XParseVar::charCount(char ch) {
    if( pvar==NULL )
        return 0;
    prev = start;
    char c=0;
    int cnt=0;
    for( ; start<wep; start++ ) {
        c = pvar->charAt(start);
        if( c=='\r' ) continue;
        if( c!=ch ) break;
        cnt++;
    }
    return cnt;
}


void XParseVar::setSub(XParseVar& pv ) {
    SetVar(pv.GetVar(), pv.prev, pv.cur);
}

bool XParseVar::match( LPCC st, LPCC et, UINT flag, LPCC mask )
{
    if( pvar==NULL ) {
        qDebug("match not valid (%s,%s)", st, et);
        return false;
    }
    if( st==0 || et==0 )
        return false;
    int stlen=slen(st);
    int etlen=slen(et);
    int e=wep;
    if( flag & FIND_BACK ) // back
    {
        //char c=pvar->charAt(pe-1);
        //char d=pvar->charAt(pe-2);
        if( start-wsp<0 ) return false;
        int sp, pos;
        sp = pos = start;
        int epos = wsp;
        int lev=0, fe=0;
        if( etlen==1 ) {
            if( st[0]==pvar->charAt(pos-1) ) {
                prev = pos-1;
                start = cur = pos;
                return true;
            }
        }
        for( int n=0; n<2048 && pos>epos; n++ ) {
            fe = pvar->findPos(st,pos,epos,flag,mask);
            if( fe!=-1 || fe==pos) {
                int ps=fe+etlen, pe=pos+1;
                lev++;
                while( ps<pe ) {
                    if( etlen==1 && pvar->charAt(ps,wep)==et[0] ) {
                        lev--, ps++;
                        continue;
                    }
                    ps = pvar->findPos(et, ps, pe, flag, mask);
                    if( ps==-1 )
                        break;
                    lev-- ;
                    ps+=etlen;
                }
                if( lev==0 ) {
                    break;
                }

                if( stlen==1 ) {
                    fe--;
                    while( fe>=epos ) {
                        if( st[0]==pvar->charAt(fe,wep) ) {
                            lev++;
                            if( lev==0 ) break;
                            fe--;
                        } else break;
                    }
                    if( lev==0 ) break;
                }
                pos = fe;
            }
            else
                return false;
        }
        cur = sp;
        // start = fe-1;
        start = fe;
        prev = start+ stlen; // (stlen>1?stlen:1);
        return true;
    } else {
        int pos = (ccmp(st,et)) ? start+1 : start;
        if( isEquals(et,pvar->get(pos+1),etlen) ) {
            prev = cur = (pos+1);
            start = cur+etlen;
            return true;
        }
        int epos = e;
        int fe =0;
        int lev = 0;
        /*
        if( flag & FIND_TAG ) {
            int tmp = pvar->findPos("<",pos+1,epos,flag,mask);
            if( tmp>0)
            {
                fe = pvar->findPos("/>", pos+1, tmp,flag,mask);
                if(fe>0)
                {
                    e = fe;
                    return true ;
                }
            }
        }
        */
        while( true )
        {
            fe = pvar->findPos(et,pos,epos,flag,mask) ;
            if(fe!=-1)
            {
                while( true )
                {
                    int n = pvar->findPos(st,pos,fe,flag,mask);
                    if( n==-1 ) break;
                    lev++ ;
                    pos = n+stlen ;
                }
            }
            else
                return false ;
            if(lev>0) lev--;
            if(lev==0 || pos>fe) break ;
            pos = fe+1;
            while(true)
            {
                bool isend=true;
                if( et[0]==pvar->charAt(pos,wep) )
                {
                    if( etlen==1 || isEquals(et, pvar->get(pos), etlen) ) // ccmp(et,GetChars(pos,pos+etlen)) )
                    {
                        if(--lev==0)
                        {
                            fe=pos;
                            break;
                        }
                        isend=false;
                        pos+=etlen;
                    }
                }
                if(isend) break;
            }
            if(lev==0) break;
        }
        prev = start + stlen;
        cur = fe;
        start = fe+etlen;
        //cur = start;
        //e = fe;
        return true;
    }
    return false ;
}
