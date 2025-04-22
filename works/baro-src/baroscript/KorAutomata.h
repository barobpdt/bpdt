#include "HanTable.h"

namespace RBAUTOMATA
{

	#define INSTACK_SIZE		1000
	#define OUTSTACK_SIZE		1000
	#define STRINGBUFFER_SIZE	1000


	enum KEYINPUTMODE {ENG_INPUT, KOR_INPUT};
	enum CHARKIND {CONSONANT, VOWEL}; //����, ����


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
		const char*		GetStringBuffer(void) const {return m_DestStrBuffer;}				//���ڿ��� ����� ���۸� ���´�. (�ϼ������� ����Ǿ� ����)
		void			StringBufferClear();												//��Ʈ�� ���۸� �����.
		KEYINPUTMODE	ChangeKeyInputMode(KEYINPUTMODE Mode){return m_KeyInputMode = Mode;}//��/�� ��ȯ		
		int				GetBufferSize() {return STRINGBUFFER_SIZE;}							//��Ʈ�� ���� ũ�⸦ �� �� �ִ�.
		HAN_STATUS		State() const {return m_TempHanState; }								//���� �������� ���¸� �� �� �ִ�.
		
		unsigned short	GetConvertedUnionCode();											//�������� ���ڸ� �ϼ������� ����
		unsigned short	GetUnionCode();														//�������� ���ڸ� ���������� ����

		int				GetStringSize() { return m_StrIndex; }
		void PrintStack();																	//ǥ��������� ���� ���¸� ����Ѵ�.
		KEYINPUTMODE GetNowMode(void){ return m_KeyInputMode; }	
		void ClearAll();
	private:
		void Clear();
		void ClearOutStack();		
		
		KEYINPUTMODE	m_KeyInputMode;		
		HAN_STATUS		m_TempHanState;	

		int				m_InStackIndex;	
		INSTACKNODE		m_InStack[INSTACK_SIZE];											//�������� ���ڸ� �����ϴ� Stack
		int				m_OutStackIndex;								
		unsigned short	m_OutStack[OUTSTACK_SIZE];
		
		unsigned short	m_KeyCode;				
		int				m_RealKey, m_Key;
		char			m_DestStrBuffer[STRINGBUFFER_SIZE];									// �Էµ� �ڷᰡ ����� ����	
		int				m_StrIndex;															// �Է� ������		

		unsigned short	ConvertKey(unsigned short key);		
		int	BackSpace();
		int	AnotherKey();			
		
		unsigned char* GetCodeTable();		
		unsigned short JoongsungPair(unsigned char Old, unsigned char New);
		unsigned short JongsungPair(unsigned char Old, unsigned char New);

		//Return ��
		//	0 : ������
		//	1 : �ѱ��� �ϼ��Ǿ���
		//	2 : �ѱ��� �̿��� ���ڰ� ���յƴ�.
		//	3 : �齺���̽�
		int hanAutomata2(unsigned short Key, unsigned short UnconvertedKey);
	};	
}
