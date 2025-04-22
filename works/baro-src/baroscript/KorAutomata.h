#include "HanTable.h"

namespace RBAUTOMATA
{

	#define INSTACK_SIZE		1000
	#define OUTSTACK_SIZE		1000
	#define STRINGBUFFER_SIZE	1000


	enum KEYINPUTMODE {ENG_INPUT, KOR_INPUT};
	enum CHARKIND {CONSONANT, VOWEL}; //모음, 자음


	enum HAN_STATUS
	{
		HS_START,
		HS_CHOSUNG,
		HS_JOONGSUNG, HS_DJOONGSUNG,
		HS_JONGSUNG, HS_DJONGSUNG,
		HS_END1, HS_END2
	};


	struct INSTACKNODE
	{
		HAN_STATUS	CurrentHanState;
		unsigned short			UnionCode;
		unsigned short			Key;
		unsigned short			UnConvertedKey;
	};


	int IsHangul(char* Str, int Index);

	class CKoreanAutomata
	{
	public:
		CKoreanAutomata();
		~CKoreanAutomata();

		int				KeyPush(int KeyInput);	
		const char*		GetStringBuffer(void) const {return m_DestStrBuffer;}				//문자열이 저장된 버퍼를 얻어온다. (완성형으로 저장되어 있음)
		void			StringBufferClear();												//스트링 버퍼를 지운다.
		KEYINPUTMODE	ChangeKeyInputMode(KEYINPUTMODE Mode){return m_KeyInputMode = Mode;}//한/영 전환		
		int				GetBufferSize() {return STRINGBUFFER_SIZE;}							//스트링 버퍼 크기를 알 수 있다.
		HAN_STATUS		State() const {return m_TempHanState; }								//현재 조합중인 상태를 알 수 있다.
		
		unsigned short	GetConvertedUnionCode();											//조합중인 글자를 완성형으로 얻어옴
		unsigned short	GetUnionCode();														//조합중인 글자를 조합형으로 얻어옴

		int				GetStringSize() { return m_StrIndex; }
		void PrintStack();																	//표준출력으로 현재 상태를 출력한다.
		KEYINPUTMODE GetNowMode(void){ return m_KeyInputMode; }	
		void ClearAll();
	private:
		void Clear();
		void ClearOutStack();		
		
		KEYINPUTMODE	m_KeyInputMode;		
		HAN_STATUS		m_TempHanState;	

		int				m_InStackIndex;	
		INSTACKNODE		m_InStack[INSTACK_SIZE];											//조합중인 글자를 저장하는 Stack
		int				m_OutStackIndex;								
		unsigned short	m_OutStack[OUTSTACK_SIZE];
		
		unsigned short	m_KeyCode;				
		int				m_RealKey, m_Key;
		char			m_DestStrBuffer[STRINGBUFFER_SIZE];									// 입력된 자료가 저장될 버퍼	
		int				m_StrIndex;															// 입력 포인터		

		unsigned short	ConvertKey(unsigned short key);		
		int	BackSpace();
		int	AnotherKey();			
		
		unsigned char* GetCodeTable();		
		unsigned short JoongsungPair(unsigned char Old, unsigned char New);
		unsigned short JongsungPair(unsigned char Old, unsigned char New);

		//Return 값
		//	0 : 조합중
		//	1 : 한글이 완성되었다
		//	2 : 한글이 이외의 문자가 조합됐다.
		//	3 : 백스페이스
		int hanAutomata2(unsigned short Key, unsigned short UnconvertedKey);
	};	
}
