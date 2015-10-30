#include <Windows.h>
#include <random>
#include <functional>
#include <ctime>
#include <list>
#include "resource.h"
#pragma comment (lib, "Msimg32.lib")

VOID CALLBACK Move(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime);
VOID CALLBACK PlayerMove(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime);
VOID CALLBACK Humanize(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Initialize();

class Meteor
{
private:
	int x, y;
	int dx, dy;
	static std::function<int()> GetPos;
	static std::function<int()> GetVel;
public:
	Meteor(){Reset();}
	void Reset()
	{
		switch (rand() % 4)
		{
		case 0:
			x = GetPos(), y = 0, dx = GetVel(), dy = GetVel(); break;
		case 1:
			x = 0, y = GetPos(), dx = GetVel(), dy = GetVel(); break;
		case 2:
			x = GetPos(), y = 1000, dx = GetVel(), dy = -GetVel(); break;
		case 3:
			x = 1000, y = GetPos(), dx = -GetVel(), dy = GetVel(); break;
		}
	};
	friend VOID CALLBACK Move(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime);
};
struct tag_keyState
{
	char left : 1;
	char up : 1;
	char right : 1;
	char down : 1;
} keyState = { 0, 0, 0, 0 };


std::function<int()> Meteor::GetPos = std::bind(std::uniform_int_distribution < int > {0, 1000}, std::default_random_engine((UINT)time(0)));
std::function<int()> Meteor::GetVel = std::bind(std::uniform_int_distribution < int > {1, 30}, std::default_random_engine((UINT)time(0)));

HINSTANCE g_hInst;
HWND hMainWnd;
LPCTSTR Title = L"METEOR";

HBITMAP EraseBit;
HBITMAP DrawBit;
HBITMAP InvBit;
HBITMAP OldErase;
HBITMAP OldDraw;
HBITMAP OldInv;
HDC EraseDC;
HDC DrawDC;
HDC InvDC;


Meteor Meteors[100];

int myX, myY;
int myDX, myDY;
DWORD StartTime;
static TCHAR timebuf[25];
bool invincible;



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	g_hInst = hInstance;
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = Title;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_VREDRAW | CS_HREDRAW;

	RegisterClass(&WndClass);

	hMainWnd = CreateWindow(Title, Title, WS_OVERLAPPEDWINDOW^WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000, NULL, NULL, hInstance, NULL);
	ShowWindow(hMainWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
	{
		hMainWnd = hWnd;
		DrawBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		EraseBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		InvBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		HDC hdc = GetDC(hWnd);

		DrawDC = CreateCompatibleDC(hdc);
		EraseDC = CreateCompatibleDC(hdc);
		InvDC = CreateCompatibleDC(hdc);

		OldErase = (HBITMAP)SelectObject(EraseDC, EraseBit);
		OldDraw = (HBITMAP)SelectObject(DrawDC, DrawBit);
		OldInv = (HBITMAP)SelectObject(InvDC, InvBit);
		ReleaseDC(hWnd, hdc);

		Initialize();
		break;
	}
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT: keyState.left = 1; break;
		case VK_UP: keyState.up = 1; break;
		case VK_RIGHT: keyState.right = 1; break;
		case VK_DOWN: keyState.down = 1; break;
		}
		break;
	case WM_LBUTTONDOWN:
	{
		static DWORD prevPos;
		static LONG prevTime;
		static int dx = GetSystemMetrics(SM_CXDOUBLECLK);
		static int dy = GetSystemMetrics(SM_CYDOUBLECLK);
		static int dt = GetDoubleClickTime();

		DWORD curPos = GetMessagePos();
		LONG curTime = GetMessageTime();
		static int cnt = 0;

		if (abs(LOWORD(curPos) - LOWORD(prevPos)) < dx && abs(HIWORD(curPos) - HIWORD(prevPos)) < dy
			&& curTime - prevTime < dt) ++cnt;
		else cnt = 0;

		if (cnt == 2)
		{
			invincible = true;
			SetTimer(hWnd, 4, 5000, Humanize);
			cnt = 0;
		}


		prevPos = curPos;
		prevTime = curTime;
	}


	case WM_KEYUP:
		switch (wParam)
		{
		case VK_LEFT: keyState.left = 0; break;
		case VK_UP: keyState.up = 0; break;
		case VK_RIGHT: keyState.right = 0; break;
		case VK_DOWN: keyState.down = 0; break;
		}
		break;
	case WM_DESTROY:
		SelectObject(DrawDC, OldDraw);
		SelectObject(EraseDC, OldErase);
		SelectObject(InvDC, OldInv);
		DeleteDC(DrawDC);
		DeleteDC(EraseDC);
		DeleteDC(InvDC);
		DeleteObject(DrawBit);
		DeleteObject(EraseBit);
		DeleteObject(InvBit);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

VOID CALLBACK Move(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime)
{
	HDC hdc = GetDC(hWnd);
	for (auto &M : Meteors)
	{
		SetPixel(hdc, M.x, M.y, RGB(0,0,0)); // "Erase Meteor"
		M.x += M.dx;
		M.y += M.dy;
		if (M.x<0||M.x>1000||M.y<0||M.y>1000) M.Reset();
		else if (abs(myX - M.x) < 16 && abs(myY - M.y) < 16 && !invincible)
		{
			InvalidateRect(hWnd, NULL, TRUE);
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 1);
			KillTimer(hWnd, 2);
			lstrcat(timebuf, L"\nContinue?");
			if (MessageBox(hWnd, timebuf, L"GAME OVER", MB_OKCANCEL) == IDOK)
			{
				Initialize();
				return;
			}
			else PostQuitMessage(0);
		}
		SetPixel(hdc, M.x, M.y, RGB(255,255,255)); // "Draw Meteor"
	}

	ReleaseDC(hWnd, hdc);
}

VOID CALLBACK PlayerMove(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime)
{
	HDC hdc = GetDC(hWnd);

	RECT R = { myX, myY, myX + 48, myY + 48 };
	TransparentBlt(hdc, myX-24, myY-24, 48, 48,EraseDC, 0, 0, 48, 48, RGB(255, 255, 255));

	myDX -= 3 * (keyState.right - keyState.left);
	myDY -= 3 * (keyState.down - keyState.up);
	
	myDX *= 0.9;
	myDY *= 0.9;

	myX += myDX;
	myY += myDY;

	//myX -= 10 * (keyState.right - keyState.left);
	//myY -= 10 * (keyState.down - keyState.up);

	//BitBlt(hdc, myX, myY, 48, 48, MemDC, 0, 0, SRCCOPY);
	TransparentBlt(hdc, myX-24, myY-24, 48, 48, invincible?InvDC:DrawDC, 0, 0, 48, 48, RGB(255, 255, 255));

	ReleaseDC(hWnd, hdc);

}

VOID CALLBACK Timing(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime)
{
	HDC hdc = GetDC(hWnd);
	SetBkColor(hdc, 0);
	SetTextColor(hdc, RGB(255,255,255));
	swprintf_s(timebuf, 25, TEXT("%.2f Sec"), (GetTickCount() - StartTime) / 1000.0);
	TextOut(hdc, 900, 0, timebuf, lstrlen(timebuf));
	ReleaseDC(hWnd, hdc);
}
void Initialize()
{
	myX = rand() % 1000;
	myY = rand() % 1000;
	myDX = 0;
	myDY = 0;
	keyState.down = 0;
	keyState.left = 0;
	keyState.right = 0;
	keyState.up = 0;
	SetTimer(hMainWnd, 0, 100, Move);
	SetTimer(hMainWnd, 1, 20, PlayerMove);
	SetTimer(hMainWnd, 2, 100, Timing);
	StartTime = GetTickCount();
	for (auto &x : Meteors) x.Reset();
}

VOID CALLBACK Humanize(HWND hWnd, UINT iMsg, UINT CallerID, DWORD dwTime)
{
	invincible = false;
	KillTimer(hWnd, CallerID);
}