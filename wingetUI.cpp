// wingetUI.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "wingetUI.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	initgraph(640, 480);
	HWND hwnd = GetHWnd();  // EasyX 提供的函数，获取窗口句柄
	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~(WS_CAPTION | WS_THICKFRAME);  // 去掉标题栏和边框
	SetWindowLong(hwnd, GWL_STYLE, style);
	BeginBatchDraw();
	SetWindowPos(hwnd, NULL, 0, 0, 640, 480, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	HRGN rgn = CreateRoundRectRgn(0, 0, 640, 480, 10, 10); // 30=圆角半径
	SetWindowRgn(hwnd, rgn, TRUE);
	setaspectratio(0.25, 0.25);
	bool dragging = false;
	POINT dragStartScreen = { 0, 0 };
	RECT winStartRect = { 0,0,0,0 };
	while (1) {
		setbkcolor(RGB(250, 250, 250));
		cleardevice();
		int btnSize = 30;
		int btnX1 = 4*(640 - btnSize - 10), btnY1 = 4*5;
		int btnX2 = 4*(640 - 10), btnY2 = 4*35;

		setfillcolor(RGB(200, 200, 200));
		solidrectangle(0, 0, 640*4, 40*4);
		setbkcolor(RGB(200, 200, 200));
		settextcolor(BLACK);
		settextstyle(90, 0, _T("微软雅黑"));
		outtextxy(10, 10, _T("wingetUI"));
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		bool hover = (p.x >= btnX1/4 && p.x <= btnX2/4 && p.y >= btnY1/4 && p.y <= btnY2/4);
		if (hover) {
			setfillcolor(RGB(240, 100, 100));
			solidroundrect(btnX1, btnY1, btnX2, btnY2,20,20);
			setlinecolor(WHITE);
			setbkcolor(RGB(240, 100, 100));
		}
		else {
			setbkcolor(RGB(200, 200, 200));
			setlinecolor(BLACK);
		}
		setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 6); // 3 像素宽
		line(btnX1 + 25, btnY1 + 25, btnX2 - 25, btnY2 - 25); // 左上 → 右下
		line(btnX1 + 25, btnY2 - 25, btnX2 - 25, btnY1 + 25); // 左下 → 右上
		FlushBatchDraw();

		
		SHORT keyState = GetAsyncKeyState(VK_LBUTTON);
		if (keyState & 0x8000) { // 左键按下
			// --- 关闭按钮检测 ---
			if (p.x >= btnX1 / 4 && p.x <= btnX2 / 4 && p.y >= btnY1 / 4 && p.y <= btnY2 / 4) {
				EndBatchDraw();
				closegraph();
				return 0;
			}

			// --- 拖动检测 ---
			if (!dragging && p.y <= 40) {
				dragging = true;
				GetCursorPos(&dragStartScreen);
				GetWindowRect(hwnd, &winStartRect);
			}
			if (dragging) {
				POINT nowScreen;
				GetCursorPos(&nowScreen);
				int dx = nowScreen.x - dragStartScreen.x;
				int dy = nowScreen.y - dragStartScreen.y;

				int newLeft = winStartRect.left + dx;
				int newTop = winStartRect.top + dy;

				// 限制不移出屏幕工作区
				RECT work;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &work, 0);
				int winWpx = winStartRect.right - winStartRect.left;
				int winHpx = winStartRect.bottom - winStartRect.top;
				SetWindowPos(hwnd, NULL, newLeft, newTop, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		else {
			dragging = false;
		}

	}
	closegraph();
	return 0;
}
