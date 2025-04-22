////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// R-fi �ѱ��Է±�ҽ� Ver 1.02
//
// ����:RB
//----------------------------------------------------------------
//
//	2007-02-08 Ver 0.1
//		�̱�������, �齺���̽�, Ư��������� 
//  2007-02-13 Ver 1.0
//		�齺���̽�, Ư������ �����Ϸ�	//		
//		���ۿϷ�
//	2007-02-14 Ver 1.01
//		�������� ������ ������ ���� ��� ����� ���׼���
//  2007-02-14 Ver 1.02
//		�� ���� ��û�� A�� ��µǴ� ���� '��'�� ��µǵ��� ���� (�����̿ܿ��� 8���� ������ 9������ ������)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "KorAutomata.h"
#include <string.h>
#include <stdio.h>


namespace RBAUTOMATA
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CKoreanAutomata::CKoreanAutomata() 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	CKoreanAutomata::CKoreanAutomata() 
		: m_InStackIndex(0),
		m_KeyInputMode (KOR_INPUT),
		m_TempHanState(HS_START),
		m_KeyCode(0),
		m_OutStackIndex(0),
		
		m_RealKey(0),
		m_Key(1),		
		m_StrIndex(0)
	{
		ClearAll();
	}
	void CKoreanAutomata::ClearAll() {
		StringBufferClear();		
		ClearOutStack();
		Clear();
	
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CKoreanAutomata::~CKoreanAutomata() 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	CKoreanAutomata::~CKoreanAutomata()
	{
		ClearAll();
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void CKoreanAutomata::Clear()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	void CKoreanAutomata::Clear()
	{
		m_InStackIndex = 0;
		m_KeyInputMode = KOR_INPUT;
		m_TempHanState = HS_START;
		m_KeyCode = 0;	

		for(int i=0;i<INSTACK_SIZE;i++)
		{
			m_InStack[i].CurrentHanState = HS_START;
			m_InStack[i].UnionCode = 0;
			m_InStack[i].Key = 0;
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void CKoreanAutomata::ClearOutStack()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	void CKoreanAutomata::ClearOutStack()
	{
		m_OutStackIndex = 0;
		for(int i=0;i<OUTSTACK_SIZE;i++)
		{
			m_OutStack[i]=0;
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//unsigned short	CKoreanAutomata::GetUnionCode()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	unsigned short	CKoreanAutomata::GetUnionCode()
	{
		if(m_InStackIndex)
		{
			return m_InStack[m_InStackIndex-1].UnionCode;
		}
		return 0;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//unsigned short	CKoreanAutomata::GetConvertedUnionCode()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	unsigned short	CKoreanAutomata::GetConvertedUnionCode()//�ϼ������� ����Ʈ �� �ڵ带 ��´�.
	{
		unsigned short JCode = GetUnionCode();						//���յ� �������ڵ带 ���´�.
		unsigned short WCode = CCodeTable::GetWansungCodeFromJohab(JCode);	//���� ���ڴ� �������̹Ƿ� �ϼ������� ��ȯ�Ѵ�.
		unsigned short RealCode = (WCode<<8) | (WCode>>8);					//���� ����Ʈ ���Ĵ�� ����Ʈ���� �ٲ۴�
		return RealCode;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void CKoreanAutomata::PrintStack()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	void CKoreanAutomata::PrintStack()
	{
		//�ν��û��� ǥ��
		if(m_InStackIndex)
		for(int i=0;i<m_InStackIndex;i++)
		{
				unsigned short JCode = m_InStack[i].UnionCode;				//���յ� �������ڵ带 ���´�.
				unsigned short WCode = CCodeTable::GetWansungCodeFromJohab(JCode);	//���� ���ڴ� �������̹Ƿ� �ϼ������� ��ȯ�Ѵ�.
				unsigned short RealCode = (WCode<<8) | (WCode>>8);					//���� ����Ʈ ���Ĵ�� ����Ʈ���� �ٲ۴�
			char buff[3] = {RealCode & 0x00ff,RealCode>>8, 0};					//����� ���� char�� ������ �����Ѵ�.
			printf("%s-", buff);
		}

	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//unsigned char* CKoreanAutomata::GetCodeTable()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	unsigned char* CKoreanAutomata::GetCodeTable()
	{
		static unsigned char HanKeyboardTable[] = {
				//0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,  /*  !"#$%&' */ 
				//0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,  /* ()*+,-./ */ 
				//0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,  /* 01234567 */ 
				//0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,  /* 89:;<=>? */ 
				//0x40, 0x41, 0x42, 0x43, 0x44, 0x86, 0x46, 0x47,  /* @ABCDEFG */ 
				//0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0xA6,  /* HIJKLMNO */ 
				//0xAC, 0x8A, 0x83, 0x53, 0x8C, 0x55, 0x56, 0x8F,  /* PQRSTUVW */ 
				//0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,  /* XYZ[\]^_ */ 
				//0x60, 0x88, 0xBA, 0x90, 0x8D, 0x85, 0x87, 0x94,  /* `abcdefg */ 
				//0xAD, 0xA5, 0xA7, 0xA3, 0xBD, 0xBB, 0xB4, 0xA4,  /* hijklmno */ 
				//0xAA, 0x89, 0x82, 0x84, 0x8B, 0xAB, 0x93, 0x8E,  /* pqrstuvw */ 
				//0x92, 0xB3, 0x91, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F   /* xyz{|}~  */ 

				//�ֹ����� ��û�ϸ� A�� �����ִ°� �ƴ϶� '��'�� �����ֵ��� ����
				0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,  /*  !"#$%&' */ 
				0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,  /* ()*+,-./ */ 
				0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,  /* 01234567 */ 
				0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,  /* 89:;<=>? */ 
				0x40, 0x88, 0xBA, 0x90, 0x8D, 0x86, 0x87, 0x94,  /* `ABCDEFG */ 
				0xAD, 0xA5, 0xA7, 0xA3, 0xBD, 0xBB, 0xB4, 0xA6,  /* HIJKLMNO */ 
				0xAC, 0x8A, 0x83, 0x84, 0x8C, 0xAB, 0x93, 0x8F,  /* PQRSTUVW */ 
				0x92, 0xB3, 0x91, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,  /* XYZ[\]^_ */ 
				0x60, 0x88, 0xBA, 0x90, 0x8D, 0x85, 0x87, 0x94,  /* `abcdefg */ 
				0xAD, 0xA5, 0xA7, 0xA3, 0xBD, 0xBB, 0xB4, 0xA4,  /* hijklmno */ 
				0xAA, 0x89, 0x82, 0x84, 0x8B, 0xAB, 0x93, 0x8E,  /* pqrstuvw */ 
				0x92, 0xB3, 0x91, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F   /* xyz{|}~  */ 
		}; 
		return HanKeyboardTable;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//unsigned short CKoreanAutomata::ConvertKey(unsigned short Key)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	unsigned short CKoreanAutomata::ConvertKey(unsigned short Key)
	{
		unsigned char* CodeTable = GetCodeTable();
		if(GetNowMode() == ENG_INPUT || Key <= 32 || Key > 127)
			return Key;
		else
			return CodeTable[Key-32];
	};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//unsigned short CKoreanAutomata::JoongsungPair(unsigned char Old, unsigned char New)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned short CKoreanAutomata::JoongsungPair(unsigned char Old, unsigned char New)
	{
		static unsigned char dJoongTable[7][3] = { 
			{ 0xad, 0xa3, 0xae },   /* ��, ��, �� */ 
			{ 0xad, 0xa4, 0xaf },   /* ��, ��, �� */ 
			{ 0xad, 0xbd, 0xb2 },   /* ��, ��, �� */ 
			{ 0xb4, 0xa7, 0xb5 },   /* ��, ��, �� */ 
			{ 0xb4, 0xaa, 0xb6 },   /* ��, ��, �� */ 
			{ 0xb4, 0xbd, 0xb7 },	/* ��, ��, �� */ 
			{ 0xbb, 0xbd, 0xbc }	/* ��, ��, �� */ 
		};

		int i ;
		for(i=0; i<7; i++)
		{
			if(dJoongTable[i][0] == Old && dJoongTable[i][1] == New)
				return m_KeyCode = dJoongTable[i][2];
		}
		return 0;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//unsigned short CKoreanAutomata::JongsungPair(unsigned char Old, unsigned char New)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned short CKoreanAutomata::JongsungPair(unsigned char Old, unsigned char New)
	{
		static unsigned char dJongsungTable[11][3] = { 
			{ 0x82, 0x8b, 0xc4 },    /* ��, ��*/ 
			{ 0x84, 0x8e, 0xc6 },    /* ��, ��*/ 
			{ 0x84, 0x94, 0xc7 },    /* ��, ��*/ 
			{ 0x87, 0x82, 0xca },    /* ��, ��*/ 
			{ 0x87, 0x88, 0xcb },    /* ��, ��*/ 
			{ 0x87, 0x89, 0xcc },    /* ��, ��*/ 
			{ 0x87, 0x8b, 0xcd },    /* ��, ��*/ 
			{ 0x87, 0x92, 0xce },    /* ��, ��*/ 
			{ 0x87, 0x93, 0xcf },    /* ��, ��*/ 
			{ 0x87, 0x94, 0xd0 },    /* ��, ��*/ 
			{ 0x89, 0x8b, 0xd4 }     /* ��, ��*/ 
		}; 
		int i ;
		for(i=0; i<11; i++)
		{
			if(dJongsungTable[i][0] == Old && dJongsungTable[i][1] == New)
				return m_KeyCode = dJongsungTable[i][2];
		}	
		return 0;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int CKoreanAutomata::hanAutomata2(unsigned short Key, unsigned short UnconvertedKey)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int CKoreanAutomata::hanAutomata2(unsigned short Key, unsigned short UnconvertedKey)
	{	
		/*-------------------------------------------------------------------- 
		�μ�	: key = �Էµ� Ű �ڵ� 
		��ȯ��	: �� ������ ������ ������ 1��, ��� �������̸� 0��, �ϼ��� ������ �ڵ�� �Է� ������ ���� ���������� ���� �� �ִ�.  
		����	:  
		--------------------------------------------------------------------*/ 	

		CHARKIND chKind;
		bool bCanBeJongsung = false;
		/* �ʼ��ڵ忡 �����ϴ� ���� �ڵ� */ 
		static unsigned char Cho2Jong[] = {   
			0xc2,  /*  �⿪                 */  
				0xc3,  /*  �ֱ⿪               */ 
				0xc5,  /*  ����                 */ 
				0xc8,  /*  ���                 */ 
				0x00,  /*  �ֵ�� (�ش� ����)   */ 
				0xc9,  /*  ����                 */ 
				0xd1,  /*  ����                 */ 
				0xd3,  /*  ����                 */ 
				0x00,  /*  ����� (�ش� ����)   */ 
				0xd5,  /*  �ÿ�                 */ 
				0xd6,  /*  �ֽÿ�               */ 
				0xd7,  /*  ����                 */ 
				0xd8,  /*  ����                 */ 
				0x00,  /*  ������ (�ش� ����)   */ 
				0xd9,  /*  ġ��                 */ 
				0xda,  /*  Ű��                 */   
				0xdb,  /*  Ƽ��                 */ 
				0xdc,  /*  ����                 */ 
				0xdd   /*  ����                 */ 
		}; 	

		unsigned short	TempUnionCode;
		unsigned short	OldKeyCode;	
		unsigned short	UC_OldKeyCode;	// UC is Unconverted


		//�������� �������� �Ǵ�
		if((Key & 0x60) == 0x20)
		{
			chKind = VOWEL; //����
			bCanBeJongsung = false;
		}
		else
		{
			chKind = CONSONANT;
			if(!(Key == 0x86 || Key == 0x8A || Key == 0x8F)) // ��, ��, ��,�� ��ħ���� �� �� ����. 		
				bCanBeJongsung = true;
		}

		//���� �ѱ��� �Էµǰ� �ִٸ� ������ ����� ���ÿ��� ���յ� �ڵ�� �Էµ� �ۼ踦 �����´�.
		if(m_TempHanState)
		{
			TempUnionCode	=	m_InStack[m_InStackIndex-1].UnionCode;
			OldKeyCode		=	m_InStack[m_InStackIndex-1].Key;	
			UC_OldKeyCode	=	m_InStack[m_InStackIndex-1].UnConvertedKey;
		}
		else
		{
			TempUnionCode = 0x8441;
			OldKeyCode = 0;
			UC_OldKeyCode = 0;
		}
		m_KeyCode = Key;

		//���丶Ÿ �ھ� �κ�
		HAN_STATUS OldState = State();
		switch(m_TempHanState)
		{
		case HS_START:
			if(chKind == CONSONANT)
				m_TempHanState = HS_CHOSUNG;
			else
				m_TempHanState = HS_JOONGSUNG;
			break;
		case HS_CHOSUNG:
			if(chKind == VOWEL)
				m_TempHanState = HS_JOONGSUNG;
			else
				m_TempHanState = HS_END1;
			break;
		case HS_JOONGSUNG:
			if(bCanBeJongsung)
				m_TempHanState = HS_JONGSUNG;
			else if(JoongsungPair(OldKeyCode, m_KeyCode))
				m_TempHanState = HS_DJOONGSUNG;
			else
				m_TempHanState = HS_END1;
			break;
		case HS_DJOONGSUNG:
			if(bCanBeJongsung)
				m_TempHanState = HS_JONGSUNG;
			else
				m_TempHanState = HS_END1;
			break;
		case HS_JONGSUNG:
			if(chKind == CONSONANT && JongsungPair(OldKeyCode, m_KeyCode))
				m_TempHanState = HS_DJONGSUNG;
			else if(chKind == VOWEL)
				m_TempHanState = HS_END2;
			else
				m_TempHanState = HS_END1;
			break;
		case HS_DJONGSUNG:
			if(chKind == VOWEL)
				m_TempHanState = HS_END2;
			else
				m_TempHanState = HS_END1;
			break;
		}

		//���丶Ÿ�� ���¸� ���� �ڵ带 �����Ѵ�.
		switch(m_TempHanState)
		{
		case HS_CHOSUNG:
			TempUnionCode = (TempUnionCode & 0x83FF) | ((m_KeyCode-0x80)<<10);
			break;
		case HS_JOONGSUNG:
		case HS_DJOONGSUNG:
			TempUnionCode = (TempUnionCode & 0xFC1F) | ((m_KeyCode-0xA0)<<5);
			if(OldState == HS_START)
			{
				m_InStack[m_InStackIndex].CurrentHanState = m_TempHanState;
				m_InStack[m_InStackIndex].UnionCode	= TempUnionCode;
				m_InStack[m_InStackIndex].UnConvertedKey= UnconvertedKey;
				m_InStack[m_InStackIndex++].Key		= Key;
				m_TempHanState = HS_START;			
				return true;
			}
			break;
		case HS_JONGSUNG:
			m_KeyCode = Cho2Jong[m_KeyCode-0x82];
		case HS_DJONGSUNG:
			TempUnionCode = (TempUnionCode & 0xFFE0) | (m_KeyCode-0xC0);
			break;
		case HS_END1:
			m_TempHanState = HS_START;
			//hanAutomata2(Key);
			m_OutStack[m_OutStackIndex++] = UnconvertedKey;
			return true;
		case HS_END2:
			m_TempHanState = HS_START;
			m_OutStack[m_OutStackIndex++] = UnconvertedKey;
			m_OutStack[m_OutStackIndex++] = UC_OldKeyCode;
			m_InStackIndex--;		
			return true;
		}
		m_InStack[m_InStackIndex].CurrentHanState = m_TempHanState;
		m_InStack[m_InStackIndex].UnionCode	= TempUnionCode;
		m_InStack[m_InStackIndex].UnConvertedKey= UnconvertedKey;
		m_InStack[m_InStackIndex++].Key		= Key;
		return false;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int	CKoreanAutomata::BackSpace()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int	CKoreanAutomata::BackSpace()
	{
		switch(m_TempHanState)
		{
		case HS_START:
			return 0;
		case HS_CHOSUNG:					
			m_TempHanState = HS_START;
			break;
		case HS_JOONGSUNG:					
			m_TempHanState = HS_CHOSUNG;			
			break;
		case HS_DJOONGSUNG:			
			m_TempHanState = HS_JOONGSUNG;			
			break;
		case HS_JONGSUNG:
			m_TempHanState = HS_JOONGSUNG;
			break;
		case HS_DJONGSUNG:
			m_TempHanState = HS_JONGSUNG;
			break;
		}	
		m_InStackIndex--;
		return m_InStack[m_InStackIndex].UnionCode;	
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int	CKoreanAutomata::AnotherKey()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int	CKoreanAutomata::AnotherKey()
	{
		switch(m_TempHanState)
		{
		case HS_START:
			return 0;
		case HS_CHOSUNG:								
		case HS_JOONGSUNG:										
		case HS_DJOONGSUNG:			
		case HS_JONGSUNG:					
		case HS_DJONGSUNG:			
			m_TempHanState = HS_START;				
		}	
		for(int i=0;i<INSTACK_SIZE;i++)
		{
			m_InStack[i].CurrentHanState = HS_START;
			m_InStack[i].UnionCode = 0;
			m_InStack[i].Key = 0;
		}
		m_InStackIndex = 0;		
		m_KeyCode = 0;	
		return true;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int IsHangul(char* Str, int Index)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	int IsHangul(char* Str, int Index)
	{
		int State; // Eng = 0, Han 1st =1, Han 2nd =2
		int inMode;
		unsigned short Key;
		int i = 0;
		if(strlen(Str) < Index+1)
			return -1;
		State = 0;
		while((Key = *(Str+i)) != '\0')
		{
			if(Key<0xa4a1)	
				inMode = 0;
			else			
				inMode = 1;

			switch(State)
			{
			case 0: //Eng
				if(inMode)	State = 1;
				else		State = 0;
				break;
			case 1:
				State = 2;				
				break;
			case 2:
				if(inMode)	State = 1;
				else		State = 0;
				break;
			}

			if(i++==Index)
				break;
		}
		return State;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void	CKoreanAutomata::StringBufferClear()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void	CKoreanAutomata::StringBufferClear()
{
	memset(m_DestStrBuffer, 0, STRINGBUFFER_SIZE);
	m_StrIndex = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int CKoreanAutomata::KeyPush(int KeyInput)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int CKoreanAutomata::KeyPush(int KeyInput)
	{	
		int RealKey = KeyInput;
		int ConvertedKey = ConvertKey(RealKey);							//ó���Ұ� ������ ���ο� Ű�� �Է¹޴´�.
		bool bUnionComplated = false;

		if(RealKey == ConvertedKey)										// �ѱ��Է� �̿��� Ű�� �Է¹޾Ҵ�.
		{
			if(RealKey == 8)											//�齺���̽� �� ���
			{
				if(BackSpace() == false)								//�� ���� ������ ����. (���丶Ÿ�� HS_START ���°� ��)
				{
					if(m_StrIndex)
					{
						if(RBAUTOMATA::IsHangul(m_DestStrBuffer, m_StrIndex-1))
							m_DestStrBuffer[--m_StrIndex] = 0;
							m_DestStrBuffer[--m_StrIndex] = 0;
					}
				}
			}
			else														//��� ��Ÿ Ư�����ڰ� ������ ���
			{	
				if(m_TempHanState)										//�������� ���ڰ� �־��ٸ�..
				{
					unsigned short RealCode = GetConvertedUnionCode();
					m_DestStrBuffer[m_StrIndex++] = RealCode & 0x00ff;
					m_DestStrBuffer[m_StrIndex++] = RealCode>>8;
				}
				m_DestStrBuffer[m_StrIndex++] = ConvertedKey;
				AnotherKey();				
			}
		}
		else if(hanAutomata2(ConvertedKey, RealKey))					//���ڰ� �ϼ��� �� ���� True�� �����Ѵ�.
		{			
			bUnionComplated = true;
			unsigned short RealCode = GetConvertedUnionCode();
			m_DestStrBuffer[m_StrIndex++] = RealCode & 0x00ff;
			m_DestStrBuffer[m_StrIndex++] = RealCode>>8;
			Clear();
		}
		if(m_OutStackIndex)												//�ֱ� �Է¿��� �ʰ��Ǿ� ó���ؾ��� Ű�� �����ִٸ� ���ȣ��
		{			
			RealKey = m_OutStack[--m_OutStackIndex];					//���� �Է¹������� �װ����� �Ѵ�.			
			return KeyPush(RealKey);
		}

		return bUnionComplated;
	}
}

