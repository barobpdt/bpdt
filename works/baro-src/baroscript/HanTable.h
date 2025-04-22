#pragma once

class CCodeTable
{
	/* 
		CodeTable[X][0] == ¿Ï¼ºÇü
		CodeTable[X][1] == Á¶ÇÕÇü
		CodeTable[X][2] == À¯´ÏÄÚµå
	*/
public:
	static unsigned short CodeTable[11172][3];	//°¡~ÆR
	static unsigned short CodeTable2[67][3];	//¤¡~¤Ó (¾ÆÁ÷ À¯´ÏÄÚµå ºÎºÐÀº ¸øÇßÀ½)
	static unsigned short GetWansungCodeFromJohab(unsigned short CODE);
};