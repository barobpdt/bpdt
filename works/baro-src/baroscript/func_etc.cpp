#include "func_util.h"
#include <stdlib.h>
#include <signal.h>
#include <QtNetwork/QNetworkInterface>
#include <QDate>
#include <QTime>

#define ENV_USE

#ifdef MAC_USE
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <iphlpapi.h>

    inline U16 hashMacAddress(PIP_ADAPTER_INFO info) {
        U16 hash = 0;
        for (U32 i = 0; i < info->AddressLength; i++) {
            hash += (info->Address[i] << ((i & 1) * 8));
        }
        return hash;
    }

    inline void getMacHash(U16 &mac1, U16 &mac2) {
        IP_ADAPTER_INFO AdapterInfo[32];
        DWORD dwBufLen = sizeof(AdapterInfo);

        DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
        if (dwStatus != ERROR_SUCCESS)
            return; // no adapters.

        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        mac1 = hashMacAddress(pAdapterInfo);
        if (pAdapterInfo->Next)
            mac2 = hashMacAddress(pAdapterInfo->Next);

        // sort the mac addresses. We don't want to invalidate
        // both macs if they just change order.
        if (mac1 > mac2) {
            U16 tmp = mac2;
            mac2 = mac1;
            mac1 = tmp;
        }
    }

    inline U16 getVolumeHash() {
        DWORD serialNum = 0;

        // Determine if this volume uses an NTFS file system.
        GetVolumeInformationA("c:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0);
        U16 hash = (U16)((serialNum + (serialNum >> 16)) & 0xFFFF);

        return hash;
    }

    inline U16 getCpuHash() {
        int cpuinfo[4] = {0, 0, 0, 0};
        __cpuid(cpuinfo, 0);
        U16 hash = 0;
        U16 *ptr = (U16 * )(&cpuinfo[0]);
        for (U32 i = 0; i < 8; i++)
            hash += ptr[i];

        return hash;
    }

    inline const char *getMachineName() {
        static char computerName[1024];
        DWORD size = 1024;
        GetComputerNameA(computerName, &size);
        return &(computerName[0]);
    }  
#else
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#ifdef DARWIN
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/if_types.h>
#else //!DARWIN
#include <linux/if.h>
#include <linux/sockios.h>
#endif //!DARWIN

    inline const char *getMachineName() {
        static struct utsname u;

        if (uname(&u) < 0) {
            return "unknown";
        }
        qDebug("system name = %s\n", u.sysname);
        qDebug("node name   = %s\n", u.nodename);
        qDebug("release     = %s\n", u.release);
        qDebug("version     = %s\n", u.version);
        qDebug("machine     = %s\n", u.machine);

        return u.nodename;
    }


    //---------------------------------get MAC addresses ------------------------------------unsigned short-unsigned short----------
    // we just need this for purposes of unique machine id. So any one or two mac's is fine.
    inline unsigned short hashMacAddress(unsigned char *mac) {
        unsigned short hash = 0;

        for (unsigned int i = 0; i < 6; i++) {
            hash += (mac[i] << ((i & 1) * 8));
        }
        return hash;
    }

    inline void getMacHash(unsigned short &mac1, unsigned short &mac2) {
        mac1 = 0;
        mac2 = 0;

#ifdef DARWIN

        struct ifaddrs* ifaphead;
       if ( getifaddrs( &ifaphead ) != 0 )
          return;

       // iterate over the net interfaces
       bool foundMac1 = false;
       struct ifaddrs* ifap;
       for ( ifap = ifaphead; ifap; ifap = ifap->ifa_next )
       {
          struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifap->ifa_addr;
          if ( sdl && ( sdl->sdl_family == AF_LINK ) && ( sdl->sdl_type == IFT_ETHER ))
          {
              if ( !foundMac1 )
              {
                 foundMac1 = true;
                 mac1 = hashMacAddress( (unsigned char*)(LLADDR(sdl))); //sdl->sdl_data) + sdl->sdl_nlen) );
              } else {
                 mac2 = hashMacAddress( (unsigned char*)(LLADDR(sdl))); //sdl->sdl_data) + sdl->sdl_nlen) );
                 break;
              }
          }
       }

       freeifaddrs( ifaphead );

#else // !DARWIN

        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0)
            return;

        // enumerate all IP addresses of the system
        struct ifconf conf;
        char ifconfbuf[128 * sizeof(struct ifreq)];
        memset(ifconfbuf, 0, sizeof(ifconfbuf));
        conf.ifc_buf = ifconfbuf;
        conf.ifc_len = sizeof(ifconfbuf);
        if (ioctl(sock, SIOCGIFCONF, &conf)) {
            return;
        }

        // get MAC address
        bool foundMac1 = false;
        struct ifreq *ifr;
        for (ifr = conf.ifc_req; (char *)ifr < (char *)conf.ifc_req + conf.ifc_len; ifr++) {
            if (ifr->ifr_addr.sa_data == (ifr + 1)->ifr_addr.sa_data)
                continue;  // duplicate, skip it

            if (ioctl(sock, SIOCGIFFLAGS, ifr))
                continue;  // failed to get flags, skip it
            if (ioctl(sock, SIOCGIFHWADDR, ifr) == 0) {
                if (!foundMac1) {
                    foundMac1 = true;
                    mac1 = hashMacAddress((unsigned char *)&(ifr->ifr_addr.sa_data));
                } else {
                    mac2 = hashMacAddress((unsigned char *)&(ifr->ifr_addr.sa_data));
                    break;
                }
            }
        }

        close(sock);

#endif // !DARWIN

        // sort the mac addresses. We don't want to invalidate
        // both macs if they just change order.
        if (mac1 > mac2) {
            unsigned short tmp = mac2;
            mac2 = mac1;
            mac1 = tmp;
        }
    }

    inline unsigned short getVolumeHash() {
        // we don't have a 'volume serial number' like on windows. Lets hash the system name instead.
        unsigned char *sysname = (unsigned char *)getMachineName();
        unsigned short hash = 0;

        for (unsigned int i = 0; sysname[i]; i++)
            hash += (sysname[i] << ((i & 1) * 8));

        return hash;
    }

#ifdef DARWIN
#include <mach-o/arch.h>
    inline unsigned short getCpuHash()
    {
        const NXArchInfo* info = NXGetLocalArchInfo();
        unsigned short val = 0;
        val += (unsigned short)info->cputype;
        val += (unsigned short)info->cpusubtype;
        return val;
    }

#else // !DARWIN

    static void getCpuid(unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
#ifdef __arm__
        *eax = 0xFD;
        *ebx = 0xC1;
        *ecx = 0x72;
        *edx = 0x1D;
        return;
#else
        asm volatile("cpuid" :
            "=a" (*eax),
            "=b" (*ebx),
            "=c" (*ecx),
            "=d" (*edx) : "0" (*eax), "2" (*ecx));
#endif
    }

    inline unsigned short getCpuHash() {
        unsigned int cpuinfo[4] = {0, 0, 0, 0};
        getCpuid(&cpuinfo[0], &cpuinfo[1], &cpuinfo[2], &cpuinfo[3]);
        unsigned short hash = 0;
        unsigned int *ptr = (&cpuinfo[0]);
        for (unsigned int i = 0; i < 4; i++)
            hash += (ptr[i] & 0xFFFF) + (ptr[i] >> 16);

        return hash;
    }

#endif // !DARWIN
#endif

LPCC machineHash(StrVar* rst) {
    StrVar* var=cfVar("machineHash");
    if( var==NULL ) return NULL;
    if( var->length()>0 ) {
        rst->add(var);
        return var->get();
    }
    var->add( "#");
    var->add( getMachineName() );
    var->add( ";#");
    var->add( (int)getCpuHash() );
    var->add( (int)getVolumeHash() );
    rst->add(var);
    return var->get();
}
#endif

LPCC getDirectionValue(U32 flag, StrVar* rst) {
    rst->reuse();
    if( flag&DIRECT_LEFT ) {
        rst->add("Left");
        if( flag&DIRECT_UP) {
            rst->add("Up");
        } else if( flag&DIRECT_DOWN) {
            rst->add("Down");
        }
    } else if( flag&DIRECT_RIGHT ) {
        rst->add("Right");
        if( flag&DIRECT_UP) {
            rst->add("Up");
        } else if( flag&DIRECT_DOWN) {
            rst->add("Down");
        }
    } else if( flag&DIRECT_UP) {
        rst->add("Up");
    } else if( flag&DIRECT_DOWN) {
        rst->add("Down");
    }
    return rst->get();
}

LPCC getColorText(QColor& clr, StrVar* rst) {
    rst->format(32, "#%X%X%X", clr.red(), clr.green(), clr.blue());
    if( clr.alpha()>=0 ) rst->format(8,"%X",clr.alpha());
    return rst->get();
}

U32 getDirection(StrVar* p1, StrVar* p2, float base) {
    StrVar* sv=p1;
    int x1=0, x2=0, y1=0, y2=0;
    if( SVCHK('i',2) ) {
        x1=sv->getInt(4), y1=sv->getInt(8);
    } else return 0;
    sv=p2;
    if( SVCHK('i',2) ) {
        x2=sv->getInt(4), y2=sv->getInt(8);
    } else return 0;
    int dx=x2-x1, dy=y2-y1;
    int ax=abs(dx), ay=abs(dy);
    if( ax<10 && ay<10 )
        return 0;
    U32 flag=0;
    float rate=0.0;
    bool plus=true;
    if( dx==0 || dy==0 ) {
        if( dx==0 ) {
            if( dy<0 ) {
                flag=DIRECT_UP;
            } else {
                flag=DIRECT_DOWN;
            }
        } else {
            if( dx>0 ) {
                flag=DIRECT_RIGHT;
            } else {
                flag=DIRECT_LEFT;
            }
        }
    } else if( ax>ay ) {
        rate=(float)(ay/(float)ax);
    } else {
        rate=(float)(ax/(float)ay);
        plus=false;
    }
    if( flag==0 ) {
        if( dx>0 ) {
            if( plus ) {
                if( rate<=base ) flag=DIRECT_RIGHT;
                else {
                    if( dy<0 ) flag=DIRECT_RIGHT|DIRECT_UP;
                    else flag=DIRECT_RIGHT|DIRECT_DOWN;
                }
            } else {
                if( rate<=base ) {
                    if( dy<0 ) flag=DIRECT_UP;
                    else flag=DIRECT_DOWN;
                } else {
                    if( dy<0 ) flag=DIRECT_RIGHT|DIRECT_UP;
                    else flag=DIRECT_RIGHT|DIRECT_DOWN;
                }
            }
        } else {
            if( plus ) {
                if( rate<base ) flag=DIRECT_LEFT;
                else {
                    if( dy<0 ) flag=DIRECT_LEFT|DIRECT_UP;
                    else flag=DIRECT_LEFT|DIRECT_DOWN;
                }
            } else {
                if( rate<base ) {
                    if( dy<0 ) flag=DIRECT_UP;
                    else flag=DIRECT_DOWN;
                } else {
                    if( dy<0 ) flag=DIRECT_LEFT|DIRECT_UP;
                    else flag=DIRECT_LEFT|DIRECT_DOWN;
                }
            }
        }
    }
    return flag;
}
/***************************** 3차(cubic) 베지어 곡선을 만드는 코드 ***************************************/
typedef struct {
     float x;
     float y;
} Point2D;

/******************************************************
cp는 4개의 요소가 있는 배열이다:
cp[0]은 시작점 (A)
cp[1]은 1번째 제어점 (B)
cp[2]는 2번째 제어점 (C)
cp[3]은 끝점 (D) t는 매개변수 값, 0 ≤ t ≤ 1
*******************************************************/
inline Point2D PointOnCubicBezier( Point2D* cp, float t ) {
     float ax, bx, cx;
     float ay, by, cy;
     float tSquared, tCubed;
     Point2D result;

     /* 다항식 계수를 계산한다 */
     cx = 3.0 * (cp[1].x - cp[0].x);
     bx = 3.0 * (cp[2].x - cp[1].x) - cx;
     ax = cp[3].x - cp[0].x - cx - bx;

     cy = 3.0 * (cp[1].y - cp[0].y);
     by = 3.0 * (cp[2].y - cp[1].y) - cy;
     ay = cp[3].y - cp[0].y - cy - by;

     /* 매개변수 값 t에서 곡선 점을 계산한다 */
     tSquared = t * t;
     tCubed = tSquared * t;

     result.x = (ax * tCubed) + (bx * tSquared) + (cx * t) +
         cp[0].x;
     result.y = (ay * tCubed) + (by * tSquared) + (cy * t) +
         cp[0].y;

     return result;
}

inline double factorial(int n) {
    static double a[33];
    a[0] = 1.0;
    a[1] = 1.0;
    a[2] = 2.0;
    a[3] = 6.0;
    a[4] = 24.0;
    a[5] = 120.0;
    a[6] = 720.0;
    a[7] = 5040.0;
    a[8] = 40320.0;
    a[9] = 362880.0;
    a[10] = 3628800.0;
    a[11] = 39916800.0;
    a[12] = 479001600.0;
    a[13] = 6227020800.0;
    a[14] = 87178291200.0;
    a[15] = 1307674368000.0;
    a[16] = 20922789888000.0;
    a[17] = 355687428096000.0;
    a[18] = 6402373705728000.0;
    a[19] = 121645100408832000.0;
    a[20] = 2432902008176640000.0;
    a[21] = 51090942171709440000.0;
    a[22] = 1124000727777607680000.0;
    a[23] = 25852016738884976640000.0;
    a[24] = 620448401733239439360000.0;
    a[25] = 15511210043330985984000000.0;
    a[26] = 403291461126605635584000000.0;
    a[27] = 10888869450418352160768000000.0;
    a[28] = 304888344611713860501504000000.0;
    a[29] = 8841761993739701954543616000000.0;
    a[30] = 265252859812191058636308480000000.0;
    a[31] = 8222838654177922817725562880000000.0;
    a[32] = 263130836933693530167218012160000000.0;
    if( n<0 || n>32 ) return 0;
    return a[n];
}

inline double Ni(int n, int i)
{
    double ni;
    double a1 = factorial(n);
    double a2 = factorial(i);
    double a3 = factorial(n - i);
    ni =  a1/ (a2 * a3);
    return ni;
}

// Calculate Bernstein basis
inline double Bernstein(int n, int i, double t) {
    double basis;
    double ti; /* t^i */
    double tni; /* (1 - t)^i */

    /* Prevent problems with pow */

    if (t == 0.0 && i == 0)
        ti = 1.0;
    else
        ti = pow(t, i);

    if (n == i && t == 1.0)
        tni = 1.0;
    else
        tni = pow((1 - t), (n - i));

    //Bernstein basis
    basis = Ni(n, i) * ti * tni;
    return basis;
}

void getBezier2D(XListArr* b, XListArr* p, int cpts) {
    if( b==NULL || p==NULL ) return;
    int bsize = b->size();
    int npts = (bsize) / 2;
    int icount, jcount;
    double step, t;

    // Calculate points on curve

    icount = 0;
    t = 0;
    step = (double)1.0 / (cpts - 1);


    StrVar* sv=NULL;
    StrVar* next=NULL;
    for (int i1 = 0; i1 != cpts; i1++) {
        if ((1.0 - t) < 5e-6)
            t = 1.0;

        jcount = 0;
        double s1=0.0, s2=0.0;

        for (int i = 0; i != npts; i++) {
            double basis = Bernstein(npts - 1, i, t);
            sv=b->get(jcount);
            if( sv ) {
                s1+= basis * toDouble(sv);
                s2+= basis * toDouble(b->get(jcount+1));
                sv=p->get(icount);
                if( sv==NULL ) break;
                sv->setVar('4',0).addDouble(s1);
                next=p->get(icount+1);
                if( next ) {
                    next->setVar('4',0).addDouble(s2);
                }
            }
            jcount = jcount +2;
        }

        icount += 2;
        if( icount>=cpts )
            break;
        t += step;
    }
}

bool recalcArrayPixcel(XListArr* a, XParseVar& pv, int wid, StrVar* rst, double rate ) {
    int emptyCnt=0;
    int tot=0;
    int totalWidth=wid;
    StrVar* sv=NULL;
    StrVar* var=pv.GetVar();
    LPCC val=NULL;
    while( pv.valid() ) {
        pv.findEnd(",");
        int sp=pv.prev, ep=pv.cur;
        int pos=var->findPos("px",sp,ep);
        if( pos>0 ) {
            var->trim(sp,pos);
            val=var->get(sp,pos);
            if( isnum(val) ) {
                int num=atoi(val);
                if( rate > 0.0 ) {
                    double dnum=(double)(num* rate);
                    num=(int)dnum;
                }
                wid-=num;
                if( wid<0 ) wid=5;
                a->add()->add(val);
            } else {
                a->add();
                emptyCnt++;
            }
        } else {
            var->trim(sp,ep);
            val=var->get(sp,ep);
            if( isnum(val) ) {
                int num=atoi(val);
                tot+=num;
                a->add()->setVar('0',0).addInt(num);
            } else {
                a->add();
                emptyCnt++;
            }
        }
    }
    if( emptyCnt>0 ) {
        int ww=0;
        if( wid> tot ) {
            ww=(wid-tot) / emptyCnt;
        } else {
            for( int n=0, sz=a->size(); n<sz; n++ ) {
                sv=a->get(n);
                if( sv->length()==0 ) {
                    sv->setVar('0',0).addInt(1);
                    tot+=ww;
                }
            }
            return true;
        }
        for( int n=0, sz=a->size(); n<sz; n++ ) {
            sv=a->get(n);
            if( sv->length()==0 ) {
                sv->setVar('0',0).addInt(ww);
                tot+=ww;
            }
        }
    }
    int sum=0, sz=a->size();
    double c=0;
    for( int n=0;n<sz;n++) {
        sv = a->get(n);
        if( sv && sv->length() ) {
            if( SVCHK('0',0) ) {
                int x=sv->getInt(4);
                c=(double)(x/(double)tot)*wid;
                sv->setVar('0',0).addInt((int)c);
                sum+=(int)c;
            } else {
                c=toInteger(sv);
                sv->setVar('0',0).addInt((int)c);
                sum+=(int)c;
            }
        }
    }
    tot=sum;
    if( tot>0 && sz>0 ) {
        int d = tot-totalWidth;
        int e = abs(d);
        if( e>0 ) {
            bool bsn = d<0 ? true: false;
            if( e<sz ) {
                for( int n=0;n<e;n++) {
                    sv = a->get(n);
                    int x = toInteger(sv);
                    if( bsn ) x+=1;
                    else if(x>1) x-=1;
                    sv->setVar('0',0).addInt(x);
                }
            } else {
                int nw=(int)(e/(sz*1.0));
                if( nw>0 ) {
                    for( int n=0;n<sz;n++) {
                        sv = a->get(n);
                        int x = toInteger(sv);
                        if( bsn ) x+=nw;
                        else if(x>nw) x-=nw;
                        sv->setVar('0',0).addInt(x);
                    }
                }
            }
            int tw=0;
            for( int n=0;n<sz;n++) {
                tw+= toInteger(a->get(n));
            }
            d=totalWidth-tw;
            if( abs(d) ) {
                sv = a->get(sz-1);
                if( sv ) {
                    int x=toInteger(sv);
                    x+=d;
                    sv->setVar('0',0).addInt(x);
                }
            }
        }
    }
    rst->setVar('a',0,(LPVOID)a);
    return true;
}

//#> etc util
bool getSystemEnv(StrVar* rst, LPCC type, LPCC  val, bool overwrite )  {
#ifdef ENV_USE
    bool ret=true;
    if( slen(type)==0 ) {
        QString str;
        CHAR infoBuf[INFO_BUFFER_SIZE];
        rst->set("SYSTEM Environment Variables");
        LPCC lpszVariable;
        LPVOID lpvEnv = GetEnvironmentStrings();
        if (lpvEnv ) {
            for( lpszVariable = (LPCC)lpvEnv; *lpszVariable; lpszVariable++) {
                int n=0;
                for(; n<INFO_BUFFER_SIZE && *lpszVariable; n++) infoBuf[n] = *lpszVariable++;
                infoBuf[n] = 0;
                rst->add("\n").add(infoBuf);
            }
        }
        FreeEnvironmentStrings((LPTSTR)lpvEnv);
    }else if( val==NULL ) {
        LPCC txt=getenv(type);
        rst->set(txt);
    } else {
        // if( setenv(type, val, overwrite?1:0)==0 ) ret=false;
    }
    return ret;
#else
    return false;
#endif
}



QPixmap* getStrVarPixmap(StrVar* sv, bool alpha, PU32 ref ) {
    if( sv==NULL )
        return NULL;
    QPixmap* p=NULL;
    if( ref ) *ref = 0;
    if( SVCHK('i',6) ) {
        p = (QPixmap*)SVO;
    } else if( SVCHK('i',7) ) {
        p = new QPixmap();
        p->loadFromData((const uchar *)sv->get(FUNC_HEADER_POS+4), sv->getInt(FUNC_HEADER_POS));
        if( ref ) *ref = 1;
    } else if( SVCHK('n',0) ) {
        TreeNode* nd = (TreeNode*)SVO;
        if( nd && nd->xstat==FNSTATE_DRAW ) {
            sv=nd->gv("@img");
            if( SVCHK('i',9) ) {
                QImage* img = (QImage*)SVO;
                p= new QPixmap(QPixmap::fromImage(*img));
                if( ref ) *ref = 3;
            }
        }
    } else {
        LPCC code = toString(sv);
        p = getPixmap(code,alpha);
    }
    return p;
}

#define JPEG_SOI 0xffd8
#define JPEG_SOS 0xffda
#define JPEG_EOI 0xffd9
#define JPEG_APP1 0xffe1
inline bool readWord( QIODevice &sdevice, unsigned short *target, bool invert=true ) {
    unsigned short t;
    if (sdevice.read((char*)&t, 2) != 2) return false;
    if (invert)
        *target = ((t&255) << 8) | ((t>>8)&255);
    else
        *target = t;
    return true;
}

inline bool exifScanloop( QIODevice &jpegFile, unsigned int &tnOffset, unsigned int &tnLength ) {
    // LOOP THROUGH TAGS
    while (1) {
        unsigned short tagid, tagLength;
        if (!readWord( jpegFile, &tagid) ) return 0;
        if (tagid == JPEG_EOI || tagid == JPEG_SOS) {
                // Data ends
            break;
        }
        if (!readWord( jpegFile, &tagLength) ) return 0;

        if (tagid == JPEG_APP1) {
            char str[6];
            jpegFile.read(str,6 );

            // Store the current position for offset calculation
            int basepos = jpegFile.pos();

            // read tiff - header
            unsigned short tifhead[2];
            for (int h=0; h<2; h++) {
                if (!readWord(jpegFile, &tifhead[h])) return false;
            }
            if (tifhead[0] != 0x4949) {
                return false;
            }

            while (1) {
                unsigned int offset;
                jpegFile.read( (char*)&offset, 4);
                if (offset==0) break;
                jpegFile.seek( basepos + offset );

                unsigned short fields;
                if (!readWord(jpegFile, &fields, false)) return false;
                while (fields>0) {
                    char ifdentry[12];
                    jpegFile.read( ifdentry, 12 );
                    unsigned short tagnumber = (((unsigned short)ifdentry[0]) | (unsigned short)ifdentry[1]<<8);
                            // Offset of the thumbnaildata
                    if (tagnumber == 0x0201) {
                        memcpy( &tnOffset, ifdentry+8, 4 );
                        tnOffset += basepos;

                    } else  // Length of the thumbnaildata
                        if (tagnumber == 0x0202) {
                            memcpy( &tnLength, ifdentry+8, 4 );
                        };
                    fields--;
                    if (tnOffset != 0 && tnLength!=0) return true;
                }
            }
            return false;
        }
        jpegFile.seek( jpegFile.pos() + tagLength-2 );
    }
    return false;
}
inline QImage fetchThumbnail(LPCC filenm) {
    QFile jpegFile(KR(filenm));
    QImage empty;
    if(!jpegFile.open( QIODevice::ReadOnly	) ) return empty;
    unsigned short jpegId;
    if(!readWord( jpegFile, &jpegId )) return empty;
    if(jpegId!= JPEG_SOI) return empty;          // JPEG SOI must be here

    unsigned int tnOffset = 0;
    unsigned int tnLength = 0;
    if (exifScanloop( jpegFile, tnOffset, tnLength)) {
            // Goto the thumbnail offset in the file
        jpegFile.seek( tnOffset );
            // Use image reader to decode jpeg-encoded thumbnail
        QByteArray tnArray = jpegFile.read( tnLength );
        QBuffer buf( &tnArray, 0 );
        QImageReader reader(&buf);
        reader.setAutoDetectImageFormat( false );
        reader.setFormat("jpg");
        return reader.read();
    }
    return empty;
}

bool loadPixmapImage(LPCC filenm, LPCC ext, bool bfile, StrVar* rst, StrVar* imgBuffer, bool thumb) {
    if( rst==NULL ) return false;
    if( slen(filenm)==0 ) return false;
    if( ccmp(filenm,"buffer") && imgBuffer ) {
        QByteArray ba;
        QPixmap* p = new QPixmap();
        if( slen(ext)==0 ) ext="PNG";
        ba.append(imgBuffer->get(), imgBuffer->length() );
        p->loadFromData((const uchar *)ba.constData(), ba.size(), ext );
        rst->setVar('i',6,(LPVOID)p);
        return true;
    }
    rst->reuse();
    if( bfile ) {
        int pos=LastIndexOf( filenm,'.');
        if( pos>0 ) {
            ext=filenm+pos+1;
            int len=slen(ext);
            for( int n=0;n<len;n++) {
                rst->add( (char)toupper(ext[n]) );
            }
            ext=rst->get();
        }
    }
    if( slen(ext)==0 ) {
        QPixmap* pic=getPixmap(filenm,true);
        if( pic ) {
            rst->setVar('i',6, (LPVOID)pic);
            return true;
        }
    } else {
        StrVar* sv=getStrVar("imageFileMap",filenm );
        if( SVCHK('i',6) && imgBuffer==NULL ) {
            rst->reuse()->add(sv);
            return true;
        }
        QImage img;
        if( ccmp(ext,"JPG") || ccmp(ext,"JPEG") ) {
            if( thumb ) {
                img = fetchThumbnail(filenm);
                if( img.isNull() ) thumb=false;
            }
            if( ccmp(ext,"JPG") ) ext="JPEG";
        } else if( thumb ) {
            thumb=false;
        }
        if( thumb==false ) {
            img.load(KR(filenm),ext);
        }
        if( img.isNull() ) {
            qDebug("#0#loadImage is null :%s (%s)\n", filenm, ext);
            rst->setVar('3',0);
        } else {
            QByteArray ba;
            QBuffer buffer( &ba );
            buffer.open( QIODevice::WriteOnly );
            img.save( &buffer, ext);
            if( ba.size() ) {
                int sz = ba.size();
                QPixmap* p = new QPixmap();
                p->loadFromData((const uchar *)ba.constData(), ba.size(), ext );
                rst->setVar('i',6,(LPVOID)p);
                setStrVar("imageFileMap",filenm, rst, true );
            } else {
                qDebug()<<"#9#loadImage error : "<<filenm<<" ext: "<< KR(ext);
                rst->setVar('3',0);
            }
        }
    }
    return false;
}

inline bool setRegExpPattern(QRegExp& rx, QString pattern, QString date ) {
    rx.setPattern(pattern);
    return rx.exactMatch(date);
}
QDateTime getDateTime(LPCC date) {
    QDateTime dateTime;
    if( slen(date)<8 )
        return dateTime;
    QString dateTimeString = KR(date);
    QRegExp rx;
    if( setRegExpPattern(rx,"([0-9]{4})([0-9]{2})([0-9]{2})[T]*([0-9]{2})([0-9]{2})([0-9]{2})[Z]*",dateTimeString)) {
        QDate date;
        QTime time;
        const QStringList &list = rx.capturedTexts();
        date.setDate(list[1].toInt(), list[2].toInt(), list[3].toInt());
        time.setHMS(list[4].toInt(), list[5].toInt(), list[6].toInt());
        // dateTime.setTimeSpec(Qt::UTC);
        dateTime.setDate(date);
        dateTime.setTime(time);
    } else if( setRegExpPattern(rx,"([0-9]{4})[\\-/]([0-9]{2})[\\-/]([0-9]{2})\\s+([0-9]{2})[:]([0-9]{2})[:]([0-9]{2})[.]?([0-9]{3})?",dateTimeString)) {
        QDate date;
        QTime time;
        const QStringList &list = rx.capturedTexts();
        date.setDate(list[1].toInt(), list[2].toInt(), list[3].toInt());
        time.setHMS(list[4].toInt(), list[5].toInt(), list[6].toInt());
        // dateTime.setTimeSpec(Qt::UTC);
        dateTime.setDate(date);
        dateTime.setTime(time);
    } else {
        dateTime = QDateTime::fromString(dateTimeString);
        qDebug() << "DataDataModelSupport::GetDateTimeValue() - before setting time-spec" << dateTime;
        dateTime.setTimeSpec(Qt::UTC);
        qDebug() << "DataDataModelSupport::GetDateTimeValue() - after setting time-spec" << dateTime;
    }
    return dateTime;
}

UL64 getTimet(LPCC ty, int n, QDateTime& dt) {
    if( ty[0]=='s' ) {
        dt.addSecs(n);
    } else if( ty[0]=='m' ) {
        dt.addSecs(n*60);
    } else if( ty[0]=='h' ) {
        dt.addSecs(n*60*60);
    } else if( ty[0]=='M' ) {
        dt.addMonths(n);
    } else {
        dt.addDays(n);
    }
    return dt.toLocalTime().toTime_t();
}
QString dateTimeString(LPCC date,LPCC fmt) {
    return getDateTime(date).toString(KR(fmt));
}
LPCC getFileInode(LPCC filePath, StrVar* rst) {
#ifdef _WIN32
    QString fnm = KR(filePath);
    HANDLE h = CreateFile(fnm.toStdWString().data(), 0,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
    if( h != INVALID_HANDLE_VALUE) {
        BY_HANDLE_FILE_INFORMATION hi;
        GetFileInformationByHandle(h,&hi);
        CloseHandle(h);
        rst->reuse();
        rst->format(32,"%08X.%08X.%08X",hi.dwVolumeSerialNumber,hi.nFileIndexHigh,hi.nFileIndexLow);
        return rst->get();
    }
#endif
    rst->reuse();
    return NULL;
}


bool isConnectedToNetwork() {
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;
    for (int i = 0; i < ifaces.count(); i++) {
        QNetworkInterface iface = ifaces.at(i);
        if ( iface.flags().testFlag(QNetworkInterface::IsUp)
             && !iface.flags().testFlag(QNetworkInterface::IsLoopBack) )
        {

#ifdef DEBUG
            // details of connection
            qDebug() << "name:" << iface.name() << endl
                    << "ip addresses:" << endl
                    << "mac:" << iface.hardwareAddress() << endl;
#endif

            // this loop is important
            for (int j=0; j<iface.addressEntries().count(); j++) {
#ifdef DEBUG
                qDebug() << iface.addressEntries().at(j).ip().toString()
                        << " / " << iface.addressEntries().at(j).netmask().toString() << endl;
#endif
                result=true;
            }
        }
        if( result ) break;
    }
    return result;
}


//#> parseXml

int parseXmlSub(TreeNode* root, StrVar* d, LPCC tag, int sp, int ep) {
    XParseVar pv(d, sp, ep);
    pv.ch();
    while( pv.valid() ) {
        LPCC next = pv.getNext();
        char ch = pv.ch();
        if( ch=='=' ) {
            pv.incr();
            ch = pv.ch();
        }
        if( ch=='\'' || ch=='"' ) {
            if( pv.MatchStringPos(ch) ) {
                LPCC value = pv.left();
                // qDebug()<<"parseXmlSub tag:["<<tag<<"] code:"<<KU(next)<<"="<<KU(value);
                root->set(next, value);
            }
        } else {
            root->set(next, pv.getNext());
        }
        pv.ch();
    }
    return 1;
}

int parseXmlNode(TreeNode* root, StrVar* d, LPCC tag, int ntype, TreeNode* chkNode, int sp, int ep) {
    if( chkNode==NULL )
        return 0;
    XParseVar pv(d, sp, ep);
    char c;
    int rtn = 0;
    bool fst = true;
    while( pv.valid() ) {
        sp = pv.findPos("<");
        if( sp==-1 ) {
            if( fst && ntype==XML_DATA ) {
                StrVar* sv = chkNode->gv(tag);
                if( sv && sv->length() && sv->charAt(0)!='\b' ) {
                    tag = sv->get();
                }
                if( !ccmp(tag,"0") ) {
                    LPCC value = pv.value();
                    // qDebug()<<"parseXmlNode [tag:"<<tag<<"]="<<KU(value);
                    root->set(tag, value );
                }
            }
            return 1;
        }
        pv.incr();
        c = pv.ch();
        switch( c) {
        case '/': {
            pv.incr();
            LPCC ctag = pv.getNext();
            pv.findEnd(">");
        } break;
        case '?': {
            pv.findEnd("?>");
        } break;
        case '!': {
            pv.incr();
            if( pv.ch()=='[' && pv.match("[CDATA[","]]") ) {
                if( fst && ntype==XML_DATA) {
                    StrVar* sv = chkNode->gv(tag);
                    if( sv && sv->length() && sv->charAt(0)!='\b' ) {
                        tag = sv->get();
                    }
                    if( !ccmp(tag,"0") ) {
                        LPCC value = pv.left();
                        root->set(tag, value );
                    }
                    // qDebug()<<"parseXmlNode [cdata tag:"<<tag<<"]="<<KU(value);
                }
                return 2;
            } else {
                pv.start = sp;
                if( !pv.match("<!--","-->") ) {
                    return 11;
                }
            }
        } break;
        default: {
            LPCC ctag = pv.getNext();
            int tsp = pv.start;
            ep = pv.find(">");
            if( ep==-1 ) {
                return 12;
            }
            TreeNode* cur = root;

            fst = false;
            if( pv.CharAt(ep-1)=='/' ) {
                pv.start = ep+1;
                int tep = ep - 1;
                if( tsp<tep ) {
                    parseXmlSub(cur, pv.GetVar(), ctag, tsp, tep);
                }
                rtn = 3;
            } else {
                pv.start = sp;
                int ctype = toInteger(chkNode->GetVar(ctag));
                if( ctype==XML_LIST ) {
                    pv.start = ep+1;
                    if( parseXmlList(pv, root, chkNode, ctag, ctype, 0)>10 )
                        return 19;
                } else if( pv.match(gBuf.fmt("<%s",ctag), gBuf.fmt("</%s",ctag),FIND_FUNCSTR) ) {
                    if( ctype==XML_SUB ) {
                        ctag = gBuf.fmt("%s_%s",tag,ctag);
                    }
                    if( tsp<ep ) {
                        parseXmlSub(cur, pv.GetVar(), ctag, tsp, ep);
                    }
                    ep++;
                    parseXmlNode(cur, pv.GetVar(), ctag, XML_DATA, chkNode, ep, pv.cur);
                    rtn = 4;
                    pv.findEnd(">");
                } else {
                    LPCC buf = pv.get();
                    return 13;
                }
            }
        } break;
        }
    }
    return rtn;
}

int parseXmlList(XParseVar& pv, TreeNode* root, TreeNode* chkNode, LPCC tag, int ntype, U32 flag) {
    if( chkNode==NULL )
        return 0;
    char c;
    int sp=0, ep=0, rtn=0;
    bool chk = true;
    LPCC ctag = NULL;
    while( pv.valid() ) {
        sp = pv.findPos("<");
        if( sp==-1 ) {
            break;
        }
        pv.incr();
        c = pv.ch();
        switch( c) {
        case '/': {
            pv.incr();
            ctag = pv.getNext();
            bool ok = ccmp(tag, ctag) ? true :false;
            pv.findEnd(">");
            if( ok ) return 1;
        } break;
        case '?': {
            pv.findEnd("?>");
        } break;
        case '!': {
            pv.incr();
            if( pv.ch()=='D' && ccmp(pv.NextWord(),"DOCTYPE") ) {
                pv.findEnd(">");
            }
            else if( pv.ch()=='[' && pv.match("[CDATA[","]]") ) {
                LPCC value = pv.left();
                root->set(tag, value );
            } else {
                pv.start = sp;
                if( !pv.match("<!--","-->") ) {
                    return 11;
                }
            }
        } break;
        default: {
            sp = pv.start;
            // LPCC ctag = pv.getNext();
            pv.moveNext();
            if( pv.ch(1)=='-' ) {
                pv.incr().moveNext();
            }
            LPCC ctag = pv.GetChars(sp,pv.start);
            char ch = pv.ch(1);
            int tsp = pv.start;
            ep = pv.find(">");
            if( ep==-1 ) {
                return 12;
            }

            chk = true;
            if( pv.CharAt(ep-1)=='/' ) {
                pv.start = ep+1;
                int tep = ep-1;
                if( tsp<tep ) {
                    if( ntype==XML_LIST || ntype==XML_ROW ) {
                        TreeNode* cur = root->addNode();
                        cur->set("tag",ctag);
                        parseXmlSub(cur, pv.GetVar(), ctag, tsp, ep-1);
                    } else {
                        parseXmlSub(root, pv.GetVar(), ctag, tsp, ep-1);
                    }
                }
                chk = false;
                rtn = 2;
            }

            if( chk ) {
                TreeNode* cur = root;
                int ctype = toInteger(chkNode->GetVar(ctag));
                if( ctype==XML_LIST || ntype==XML_ROOT ) {
                    pv.start = ep+1;
                    if( parseXmlList(pv, cur, chkNode, ctag, ctype, 0)>10 )
                        return 19;
                } else  if( pv.match(gBuf.fmt("<%s",ctag),gBuf.fmt("</%s",ctag),FIND_FUNCSTR) ) {
                    if( ntype==XML_LIST || ntype==XML_ROW ) {
                        if( tag==NULL ) {
                            XParseVar sub(pv.GetVar(), ep+1,pv.cur);
                            if( parseXmlList(sub, root, chkNode, ctag, XML_ROW, 0)>10 )
                                return 19;
                            chk = false;
                        } else {
                            cur = root->addNode();
                            cur->set("tag",ctag);
                        }
                    }
                    if( tsp<ep ) {
                        parseXmlSub(cur, pv.GetVar(), ctag, tsp, ep);
                    }
                    if( chk )
                        parseXmlNode(cur, pv.GetVar(), ctag, XML_DATA, chkNode, ep+1, pv.cur);
                    rtn = 2;
                    pv.findEnd(">");
                } else {
                    return 13;
                }
            }
        } break;
        }
    }
    return rtn;
}

//#> etc
double rounding( double value, int pos ) {
      double temp;
      temp = value * pow( float(10), pos );
      temp = floor( temp + 0.5 );
      temp *= pow( float(10), -pos );
      return temp;
}

int HexPairValue(const char * code) {
  int value = 0;
  const char * pch = code;
  for (;;) {
    int digit = *pch++;
    if (digit >= '0' && digit <= '9') {
      value += digit - '0';
    }
    else if (digit >= 'A' && digit <= 'F') {
      value += digit - 'A' + 10;
    }
    else if (digit >= 'a' && digit <= 'f') {
      value += digit - 'a' + 10;
    }
    else {
      return -1;
    }
    if (pch == code + 2)
      return value;
    value <<= 4;
  }
}

LPCC UTF2C( LPCC str, int len, StrVar* rst, int chk ) {
    if( str==NULL || len==0 )
        return NULL;
#ifdef _WIN32
    int	size = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, NULL);
    if( size==0 )
        return NULL;
    int bsize = (size*sizeof(WCHAR))+4;
    // PWCHAR data = bsize>0xF000 ? (PWCHAR)GetOleString(bsize): (PWCHAR)gBuf.get(bsize);
    PWCHAR data = NULL;
    if( bsize>256 ) {
        StrVar* sv=getStrVarTemp();
        data=(PWCHAR)sv->alloc(bsize);
    } else {
        data=(PWCHAR)gBuf.alloc(bsize);
    }
    MultiByteToWideChar(CP_UTF8, 0, str, len, data, size);
    data[size] = 0;
    len = WideCharToMultiByte(CP_ACP, 0, data, -1, NULL, 0, NULL, NULL);
    if( chk )
        rst->reuse();
    char* rtn = rst->memalloc(len);
    WideCharToMultiByte(CP_ACP, 0, data, size, rtn, len, NULL, NULL);
    len--;
    if( len<0 )
        len=0;
    rst->movepos(len);
    return rtn;
#else
    return str;
#endif
}

char* C2UTF(char* cur, int size, StrVar* rst, int chk) {
#ifdef _WIN32
    int	nLength = MultiByteToWideChar(CP_ACP, 0, cur, size, NULL, NULL);
    int bsize = (nLength*sizeof(WCHAR))+4;
    PWCHAR buf = NULL;
    if( bsize>256 ) {
        StrVar* sv=getStrVarTemp();
        buf=(PWCHAR)sv->alloc(bsize);
    } else {
        buf=(PWCHAR)gBuf.alloc(bsize);
    }

    MultiByteToWideChar(CP_ACP, 0, cur, size, buf, nLength);
    buf[nLength] = 0;
    nLength = WideCharToMultiByte(CP_UTF8, 0, buf, -1, cur, 0, NULL, NULL);
    if( chk )
        rst->reuse();
    char* rtn = rst->memalloc(nLength);
    int num = WideCharToMultiByte(CP_UTF8, 0, buf, -1, rtn, nLength, NULL, NULL);
    num--;
    if( num<0 )
        num =0;
    rst->movepos(num);
    return rtn;
#else
        return cur;
#endif
}

inline int wlen( PCWCHAR data  )
{
    return (data) ? wcslen(data): 0;
}
LPCC W2C(PCWCHAR str ) {
#ifdef _WIN32
    int len = wlen(str) * sizeof(wchar_t);
    int size = len+1;
    int mod = size%4;
    if(mod!=0) size+=(4-mod);
    char* data = gBuf.get( size );
    WideCharToMultiByte(CP_ACP, 0, str, -1, data, size, NULL, NULL);
    data[len] = 0;
    return data;
#else
    return NULL;
#endif
}

void jsValueString(StrVar* sv, int sp, int ep, StrVar* rst) {
    int sz=ep-sp;
    if( sv==NULL || sz<=0 ) return;
    LPCC a = sv->get(sp);
    char ch = 0;
    for( int n=0;n<sz;n++) {
        ch = a[n];
        if( ch=='\n' || ch=='\r' || ch=='\t' || ch=='\"' || ch=='\\' ) {
            if( ch=='\"' )
                rst->add('\\').add(ch);
            else if( ch=='\\' )
                rst->add("\\\\");
            else
                rst->add('\\').add( ch=='\n' ? 'n' : ch=='\t' ? 't' : 'r');
        } else
            rst->add(ch);
    }
}
bool getFullRectPos(double cw, double ch, double pw, double ph, StrVar* rst) {
    /*
    QSizeF sz=getQRectF(sv).size();
    double cw=sz.width(), ch=sz.height();
    */
    if( cw==0 || ch==0 || pw==0 || ph==0 ) return false;
    if( pw > cw && ph > ch && pw/cw >= ph/ch || //both width and high are bigger, ratio at high is bigger or
        pw > cw && ph <= ch || //only the width is bigger or
        pw < cw && ph < ch && cw/pw < ch/ph //both width and height is smaller, ratio at width is smaller
     ) {
        ph=cw*ph/pw;
        pw=cw;
    } else if (pw > cw && ph > ch && pw/cw < ph/ch || //both width and high are bigger, ratio at width is bigger or
        ph > ch && pw <= cw || //only the height is bigger or
        pw < cw && ph < ch && cw/pw > ch/ph //both width and height is smaller, ratio at height is smaller
    ) {
        pw=ch*pw/ph;
        ph=ch;
    }
    double x=(cw-pw)/2.0, y=(ch-ph)/2.0;
    rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(pw).addDouble(ph);
    return true;
}

LPCC getPairKey(PARR arrs, int n, StrVar* rst) {
    StrVar* sv=arrs ? arrs->get(n): NULL;
    if( SVCHK('x',21) ) {
        int pos=sv->find("\f",FUNC_HEADER_POS,64);
        // qDebug("getPairKey (%d, pos:%d)", n, pos);
        if( pos>FUNC_HEADER_POS ) {
            LPCC key=NULL;
            pos-=FUNC_HEADER_POS;
            if( rst ) {
                rst->set(sv->get(FUNC_HEADER_POS),pos);
                key=rst->get();
            } else {
                key=gBuf.add(sv->get(FUNC_HEADER_POS),pos);
            }
            return key;
        }
    }
    return NULL;
}
bool getPairValue(PARR arrs, int n, StrVar* rst) {
    StrVar* sv=arrs ? arrs->get(n): NULL;
    if( SVCHK('x',21) && rst ) {
        int pos=sv->find("\f",FUNC_HEADER_POS,64);
        if( pos>FUNC_HEADER_POS ) {
            rst->set(sv,pos+1,sv->length() );
            return true;
        }
    }
    return false;
}
bool findPairKey(PARR arrs, LPCC key, StrVar* rst) {
    if( arrs==NULL ) return false;
    StrVar* sv=NULL;
    for( int n=0,sz=arrs->size(); n<sz; n++ ) {
        sv=arrs->get(n);
        if( SVCHK('x',21) ) {
            int pos=sv->find("\f",FUNC_HEADER_POS,64);
            if( pos>FUNC_HEADER_POS  ) {
                if( ccmpl(key,sv->get(FUNC_HEADER_POS),pos-FUNC_HEADER_POS) ) {
                    rst->set(sv,pos+1,sv->length() );
                    // qDebug("findPairKey (key:%s, pos:%d, val:%s)", key, pos, sv->get(pos+1) );
                    return true;
                }
            }
        }
    }
    return false;
}
int findPairIndex(PARR arrs, LPCC key ) {
    if( arrs==NULL ) return -1;
    StrVar* sv=NULL;
    for( int n=0,sz=arrs->size(); n<sz; n++ ) {
        sv=arrs->get(n);
        if( SVCHK('x',21) ) {
            int pos=sv->find("\f",FUNC_HEADER_POS,64);
            if( pos>FUNC_HEADER_POS ) {
                if( ccmpl(key,sv->get(FUNC_HEADER_POS),pos-FUNC_HEADER_POS) ) {
                    return n;
                }
            }
        }
    }
    return -1;
}
XListArr* findPairKeyStart(PARR arrs, LPCC key) {
    XListArr* parr=NULL;
    StrVar* sv=NULL;
    int len=slen(key);
    for( int n=0,sz=arrs->size(); n<sz; n++ ) {
        sv=arrs->get(n);
        if( SVCHK('x',21) ) continue;
        int pos=sv->find("\f",FUNC_HEADER_POS,64);
        pos-=FUNC_HEADER_POS;
        if( pos>0 ) {
            if( len==0 ) {
                if( parr==NULL ) parr=getListArrTemp();
                parr->add()->add(sv);
            } else if( pos>=len ) {
                if( ccmpl(key,len,sv->get(FUNC_HEADER_POS),len) ) {
                    if( parr==NULL ) parr=getListArrTemp();
                    parr->add()->add(sv);
                }
            }
        }
    }
    return parr;
}

void getDivArr(PARR a, PARR arrs, StrVar* rst) {
    if( a==NULL || arrs==NULL ) return;
    StrVar* sv=arrs->get(0);
    if( isNumberVar(sv) ) {
        double wid=0, num=0, step=0, xx=0;
        num=toDouble(sv);
        sv=arrs->get(1);
        if(!isNumberVar(sv) ) {
            LPCC ty=toString(sv,rst);
            double factor=0.25;
            sv=arrs->get(2);
            if(isNumberVar(sv) ) {
                wid=toDouble(sv);
                sv=arrs->get(3);
                if( isNumberVar(sv) ) {
                    xx=toDouble(sv);
                }
                sv=arrs->get(4);
                if( isNumberVar(sv) ) {
                    factor=toDouble(sv);
                }
            }
            XListArr* arr = getListArrTemp();
            double total=0,last=wid+xx, v=0;
            if( wid==0 ) wid=100;
            arr->add()->setVar('4',0).addDouble(xx);
            step=num>0? wid/num: wid;
            a->reuse();
            if(ccmp(ty,"fx")) {
                double ar=0.5, br=num/wid;
                int last=num-(br*10);
                for(int n=0;n<num;n++) {
                    if(n<last) {
                        v=step*(num/(wid*2));
                    } else {
                        v=step*br*(ar*n);
                        br+=0.1;
                    }
                    total+=v;
                    xx+=v;
                    a->add()->addDouble(xx,0);
                }
            } else if(ccmp(ty,"sin")) {
                double dt=(100*num)/160.0;
                for(int n=1;n<num;n++) {
                    v=step*sin(n/dt)*factor;
                    total+=v;
                    a->add()->addDouble(v,0);
                }
            } else if(ccmp(ty,"cos")) {
                double dt=(100*num)/160.0;
                for(int n=0;n<num;n++) {
                    v=step*cos(n/dt)*factor;
                    total+=v;
                    a->add()->addDouble(v,0);
                }
            }
            double dist=0;
            if(wid>total) {
                dist=wid-total;
                dist/=wid;
            } else {
                dist=total-wid;
                dist/=total;
            }
            qDebug("1>> %f %f", total, dist);
            for( int n=0,sz=a->size(); n<sz; n++ ) {
                v=a->get(n)->getDouble(0);
                xx+=v*dist;
                arr->add()->setVar('4',0).addDouble(xx);
            }
            a->reuse();
            arr->add()->setVar('4',0).addDouble(last);
            if( rst ) rst->setVar('a',0,(LPVOID)arr);
            return;
        }
        bool bBack=false;
        wid=toDouble(sv);
        if( wid==0 ) wid=100;
        sv=arrs->get(3);
        if( SVCHK('3',1) ) {
            bBack=true;
        }
        sv=arrs->get(2);
        if( isNumberVar(sv) ) {
            xx=toDouble(sv);
        } else if( bBack ) {
            xx=wid;
        }
        if( num>0 ) {
            step=wid/num;
        }
        if( step>0 ) {
            if( bBack ) {
                for(int n=num-1;n>=0;n-- ) {
                    a->add()->setVar('4',0).addDouble(xx);
                    xx-=step;
                }
                a->add()->setVar('4',0).addDouble(xx);
            } else {
                wid+=xx;
                for(int n=0;n<num;n++) {
                    a->add()->setVar('4',0).addDouble(xx);
                    xx+=step;
                }
                a->add()->setVar('4',0).addDouble(wid);
            }
        }
    } else {
        sv=arrs->get(1);
        if(isNumberVar(sv)) {
            double width=toDouble(sv), total=0, gab=0;
            bool bBack=false;
            int sp=0, ep=0, num=0;
            double xx=0;
            if( width==0 ) width=100;
            sv=arrs->get(3);
            if( SVCHK('3',1) ) {
                bBack=true;
            }
            sv=arrs->get(2);
            if( isNumberVar(sv) ) {
                xx=toDouble(sv);
            } else if( bBack ) {
                xx=width;
            }
            sv=getStrVarPointer(arrs->get(0), &sp, &ep);
            // qDebug("----getDivArr fields:%s ----", sv->get() );
            if( width>0 ) {
                XParseVar pv(sv, sp, ep);
                while( pv.ch() && pv.valid() ) {
                    LPCC val=pv.findEnd(",").v();
                    int len=slen(val);
                    if(len==0) continue;
                    sv=a->add();
                    if( ccmp(val,"*") ) {
                        num++;
                        continue;
                    }
                    gab=0;
                    if( val[len-1]=='%' ) {
                        val=gBuf.add(val,len-1);
                        if( isnum(val) ) {
                            double per=(double)atof(val);
                            gab=(width*per)/100.0;
                        } else {
                            gab=100;
                        }
                    } else if( isnum(val) ) {
                        gab=(double)atof(val);
                    } else {
                        gab=100;
                    }
                    sv->setVar('4',0).addDouble(gab);
                    total+=gab;
                }
                if( total>width ) {
                    if( num ) {
                        double dist=total/num;
                        for( int n=0,sz=a->size(); n<sz; n++ ) {
                            sv=a->get(n);
                            if( !isNumberVar(sv) ) {
                                sv->setVar('4',0).addDouble(dist);
                            }
                        }
                    }
                    for( int n=0,sz=a->size(); n<sz; n++ ) {
                        sv=a->get(n);
                        gab=(toDouble(sv)*width)/total;
                        sv->setVar('4',0).addDouble(gab);
                    }
                } else if( total!=width ) {
                    double dist=width-total;
                    if( num ) {
                        dist/=num;
                        for( int n=0,sz=a->size(); n<sz; n++ ) {
                            sv=a->get(n);
                            if( !isNumberVar(sv) ) {
                                sv->setVar('4',0).addDouble(dist);
                            }
                        }
                    } else {
                        for( int n=0,sz=a->size(); n<sz; n++ ) {
                            sv=a->get(n);
                            gab=(toDouble(sv)*width)/total;
                            sv->setVar('4',0).addDouble(gab);
                        }
                    }
                }
            }

            int sz=a->size();
            if( sz>0 ) {
                if( bBack ) {
                    for(int n=sz-1;n>=0;n-- ) {
                        sv=a->get(n);
                        gab=toDouble(sv);
                        sv->setVar('4',0).addDouble(xx);
                        xx-=gab;
                    }
                    a->add()->setVar('4',0).addDouble(xx);
                } else {
                    width+=xx;
                    for(int n=0;n<sz;n++) {
                        sv=a->get(n);
                        gab=toDouble(sv);
                        sv->setVar('4',0).addDouble(xx);
                        xx+=gab;
                    }
                    a->add()->setVar('4',0).addDouble(width);
                }
            }
        } else {
            LPCC ty=AS(0), sep=AS(2);
            if(ccmp(ty,"split")) {
                if(slen(sep) ) {
                    int sp=0, ep=0;
                    sv=arrs->get(1);
                    getStrVarPointer(sv,&sp,&ep);
                    XParseVar pv(sv,sp,ep);
                    a->reuse();
                    while(pv.valid()) {
                        a->add()->add(pv.findEnd(sep).v());
                    }
                } else {
                    LPCC val=AS(1);
                    QString s=KR(val);
                    for(int n=0;n<s.length();n++) {
                        QString v(s.at(n));
                        a->add()->add(Q2A(v));
                    }
                }
            } else {
                qDebug("#0#array split type not defined (type:%s) !!!", ty);
            }
        }
    }
    if( rst ) rst->setVar('a',0,(LPVOID)a);
}


double getTimeSec()
{
#ifdef WIN32
    return GetTickCount()/1000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
#endif
}

inline void quitHandler(int sig)
{
    getCf()->setBool("@modbusExample", false);
}
void setQuitHandler()
{
#ifdef WIN32
    signal(SIGINT, quitHandler);
#else
    struct sigaction sigHandler;
    sigHandler.sa_handler = quitHandler;
    sigemptyset(&sigHandler.sa_mask);
    sigHandler.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &sigHandler, NULL);
#endif
}
