#pragma once

class CCodeTable
{
	/* 
		CodeTable[X][0] == �ϼ���
		CodeTable[X][1] == ������
		CodeTable[X][2] == �����ڵ�
	*/
public:
	static unsigned short CodeTable[11172][3];	//��~�R
	static unsigned short CodeTable2[67][3];	//��~�� (���� �����ڵ� �κ��� ������)
	static unsigned short GetWansungCodeFromJohab(unsigned short CODE);
};