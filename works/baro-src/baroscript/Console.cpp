#include "Console.h"
#include <winuser.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#pragma execution_character_set( "utf-8" )

// macro for the Clear() function
#define ERR(bSuccess) { if(!bSuccess) return; }
// macro to check whether hConsole is valid
#define CHECK(hHandle) { if(hHandle == NULL) return; };


bool CConsole::Create(const char *szTitle, bool bNoClose)
{
	// Has console been already created?
	if(hConsole != NULL)
		return false;
	
	// Allocate a new console for our app
	if(!AllocConsole())
		return false;
	
	// Create the actual console
	hConsole = CreateFileA("CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
	if(hConsole == INVALID_HANDLE_VALUE)
		return false;
	
    if(SetConsoleMode(hConsole, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_MOUSE_INPUT ) == 0)
		return false;

	// if true, disable the [x] button of the console
	if(bNoClose)
		DisableClose();
				
	// set the console title
	if(szTitle != NULL)
		SetConsoleTitleA(szTitle);
    // _setmode(_fileno(hConsole), _O_U8TEXT);
    SetConsoleOutputCP( 65001 );
	return true;
}

void CConsole::Color(WORD wColor)
{
	CHECK(hConsole);

	// no color specified, reset to defaults (white font on black background)
	if( wColor != NULL)
		SetConsoleTextAttribute(hConsole, wColor );
	// change font and/or background color
	else
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // white text on black bg
}

void CConsole::print(const char* out, WORD color)
{
	CHECK(hConsole);

	DWORD		dwWritten;
	if( color != NULL)
		SetConsoleTextAttribute(hConsole, color );
	else
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // white text on black bg

	WriteConsoleA(hConsole,out,strlen(out),&dwWritten,0);
}

void CConsole::SetTitle(const char *title)
{
	// self-explanatory
	SetConsoleTitleA(title);
}

char* CConsole::GetTitle()
{
	// get the title of our console and return it
	static char szWindowTitle[256] = "";
	GetConsoleTitleA(szWindowTitle,sizeof(szWindowTitle));

	return szWindowTitle;
}


HWND CConsole::GetHWND()
{
	if(hConsole == NULL) 
		return NULL;

	// try to find our console window and return its HWND
    return FindWindowA("ConsoleWindowClass",GetTitle());
}

void CConsole::Show(bool bShow)
{
	CHECK(hConsole);

	// get out console's HWND and show/hide the console
	HWND hWnd = GetHWND();
	if(hWnd != NULL)
		ShowWindow(hWnd, SW_HIDE ? SW_SHOW : bShow);
}

void CConsole::DisableClose()
{
	CHECK(hConsole);

	HWND hWnd = GetHWND();
	
	// disable the [x] button if we found our console
	if(hWnd != NULL)
	{
		HMENU hMenu = GetSystemMenu(hWnd,0);
		if(hMenu != NULL)
		{
			DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
			DrawMenuBar(hWnd);
		}
	}
}


void CConsole::Clear()
{
	CHECK(hConsole);

	/***************************************/
	// This code is from one of Microsoft's
	// knowledge base articles, you can find it at 
    // http://support.microsoft.com/default.aspx?scid=KB;EN-US;q99261&
	/***************************************/

	COORD coordScreen = { 0, 0 };

	BOOL bSuccess;
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */ 
    DWORD dwConSize; 

	/* get the number of character cells in the current buffer */ 

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    ERR(bSuccess);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* fill the entire screen with blanks */ 

    bSuccess = FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten );
    ERR(bSuccess);

    /* get the current text attribute */ 

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    ERR(bSuccess);

    /* now set the buffer's attributes accordingly */ 

    bSuccess = FillConsoleOutputAttribute( hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten );
    ERR(bSuccess);

    /* put the cursor at (0, 0) */ 

    bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
    ERR(bSuccess);
}


HANDLE CConsole::GetHandle()
{
	// simply return the handle to our console
	return hConsole;
}

void CConsole::Close()
{
	// free the console, now it can't be used anymore until we Create() it again
	FreeConsole();
	hConsole = NULL;
}
