#ifndef CRONTHREAD_H
#define CRONTHREAD_H
#include "../baroscript/XFunc.h"
#include "../fobject/FThread.h"


#define SECONDS_PER_MINUTE 1
#define     FIRST_SEC			0
#define     LAST_SEC			59
#define     SEC_COUNT			(LAST_SEC - FIRST_SEC + 1)

#define     FIRST_MINUTE		0
#define     LAST_MINUTE			59
#define     MINUTE_COUNT		(LAST_MINUTE - FIRST_MINUTE + 1)

#define     FIRST_HOUR			0
#define     LAST_HOUR			23
#define     HOUR_COUNT			(LAST_HOUR - FIRST_HOUR + 1)

#define     FIRST_DOM			1
#define     LAST_DOM			31
#define     DOM_COUNT			(LAST_DOM - FIRST_DOM + 1)

#define     FIRST_MONTH			1
#define     LAST_MONTH			12
#define     MONTH_COUNT			(LAST_MONTH - FIRST_MONTH + 1)

/* note on DOW: 0 and 7 are both Sunday, for compatibility reasons. */
#define     FIRST_DOW			0
#define     LAST_DOW			7
#define     DOW_COUNT			(LAST_DOW - FIRST_DOW + 1)


#define DOM_STAR		0x01
#define DOW_STAR		0x02
#define WHEN_REBOOT		0x04
#define MIN_STAR		0x08
#define HR_STAR         0x10
#define SEC_STAR		0x20

#define     _bit_byte(bit) ((bit) >> 3)
#define     _bit_mask(bit)  (1 << ((bit)&0x7))
#define     bitstr_size(nbits)  ((((nbits) - 1) >> 3) + 1)
#define     bit_alloc(nbits) (char *)malloc(1, (unsigned int)bitstr_size(nbits) * sizeof(char))
#define     bit_decl(name, nbits) (name)[bitstr_size(nbits)]
#define     bit_test(name, bit) ((name)[_bit_byte(bit)] & _bit_mask(bit))
#define     bit_set(name, bit) (name)[_bit_byte(bit)] |= _bit_mask(bit)
#define     bit_clear(name, bit) (name)[_bit_byte(bit)] &= ~_bit_mask(bit)

/* clear bits start ... stop in bitstring */
#define     bit_nclear(name, start, stop) { \
      register unsigned char *_name = name; \
      register int _start = start, _stop = stop; \
      register int _startbyte = _bit_byte(_start); \
      register int _stopbyte = _bit_byte(_stop); \
      if (_startbyte == _stopbyte) { \
            _name[_startbyte] &= ((0xff >> (8 - (_start&0x7))) | \
                              (0xff << ((_stop&0x7) + 1))); \
      } else { \
            _name[_startbyte] &= 0xff >> (8 - (_start&0x7)); \
            while (++_startbyte < _stopbyte) \
                  _name[_startbyte] = 0; \
            _name[_stopbyte] &= 0xff << ((_stop&0x7) + 1); \
      } \
}
/* set bits start ... stop in bitstring */
#define     bit_nset(name, start, stop) { \
      register unsigned char *_name = name; \
      register int _start = start, _stop = stop; \
      register int _startbyte = _bit_byte(_start); \
      register int _stopbyte = _bit_byte(_stop); \
      if (_startbyte == _stopbyte) { \
            _name[_startbyte] |= ((0xff << (_start&0x7)) & \
                            (0xff >> (7 - (_stop&0x7)))); \
      } else { \
            _name[_startbyte] |= 0xff << ((_start)&0x7); \
            while (++_startbyte < _stopbyte) \
                  _name[_startbyte] = 0xff; \
            _name[_stopbyte] |= 0xff >> (7 - (_stop&0x7)); \
      } \
}
/* find first bit clear in name */
#define     bit_ffc(name, nbits, value) { \
      register unsigned char *_name = name; \
      register int _byte, _nbits = nbits; \
      register int _stopbyte = _bit_byte(_nbits), _value = -1; \
      for (_byte = 0; _byte <= _stopbyte; ++_byte) \
            if (_name[_byte] != 0xff) { \
                  _value = _byte << 3; \
                  for (_stopbyte = _name[_byte]; (_stopbyte&0x1); \
                      ++_value, _stopbyte >>= 1); \
                  break; \
            } \
      *(value) = _value; \
}
/* find first bit set in name */
#define     bit_ffs(name, nbits, value) { \
      register unsigned char *_name = name; \
      register int _byte, _nbits = nbits; \
      register int _stopbyte = _bit_byte(_nbits), _value = -1; \
      for (_byte = 0; _byte <= _stopbyte; ++_byte) \
            if (_name[_byte]) { \
                  _value = _byte << 3; \
                  for (_stopbyte = _name[_byte]; !(_stopbyte&0x1); \
                      ++_value, _stopbyte >>= 1); \
                  break; \
            } \
      *(value) = _value; \
}


class CronEntry {
public:
    CronEntry() : flags(0), cmd(NULL) {}
    char        *cmd;
    U8    bit_decl(sec,		SEC_COUNT);
    U8    bit_decl(minute,	MINUTE_COUNT);
    U8    bit_decl(hour,	HOUR_COUNT);
    U8    bit_decl(dom,		DOM_COUNT);
    U8    bit_decl(month,	MONTH_COUNT);
    U8    bit_decl(dow,		DOW_COUNT);
    U32 flags;
};

class CronThread : public FThread {
public:
    CronThread( TreeNode* cf );
    ~CronThread();

public:
    void Run();
    void Final();
    bool exec();

    TreeNode*		xconfig;
    TreeNode*		xroot;
    XListArr		xarr;
    U32				xflag;
    U16				xtype, xstate;

    XListArr* getWorker() { return &xarr; }
    void	push(StrVar* sv);

    U32 flag() { return xflag; }
    U32 type() { return xtype; }
    TreeNode* config() { return xconfig; }
    TreeNode* root();
};

void cronSetTime(TreeNode* root);
void cronSleep(long target);

StrVar* cronLineAdd(TreeNode* cur );
long cronRun( TreeNode* root );

#endif // CRONTHREAD_H
