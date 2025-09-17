
#include <tchar.h>
#include "framework.h"
#include "wingetUI.h"
#include <thread>

#pragma comment(lib,"Kernel32.lib")
// 在文件顶部添加以下宏定义，确保 GetTickCount64 可用
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

void solidgradientrect(int x1, int y1, int x2, int y2,
    COLORREF c1, COLORREF c2, bool vertical = true)
{
    if (vertical) {
        int height = y2 - y1;
        for (int i = 0; i < height; i++) {
            double t = (double)(i + 0.5) / height;
            COLORREF color = RGB(
                (BYTE)(GetRValue(c1) + (GetRValue(c2) - GetRValue(c1)) * t + 0.5),
                (BYTE)(GetGValue(c1) + (GetGValue(c2) - GetGValue(c1)) * t + 0.5),
                (BYTE)(GetBValue(c1) + (GetBValue(c2) - GetBValue(c1)) * t + 0.5)
            );
            setlinecolor(color);
            solidrectangle(x1, y1 + i, x2, y1 + i);
        }
    }
    else { // 水平渐变
        int width = x2 - x1;
        for (int i = 0; i < width; i++) {
            double t = (double)(i + 0.5) / width;
            COLORREF color = RGB(
                (BYTE)(GetRValue(c1) + (GetRValue(c2) - GetRValue(c1)) * t + 0.5),
                (BYTE)(GetGValue(c1) + (GetGValue(c2) - GetGValue(c1)) * t + 0.5),
                (BYTE)(GetBValue(c1) + (GetBValue(c2) - GetBValue(c1)) * t + 0.5)
            );
            setlinecolor(color);
            line(x1 + i, y1, x1 + i, y2);
        }
    }
}



int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{

    const int winW = 640;
    const int winH = 480;
    const int scale = 1; // 恢复缩放因子

    // 初始化 EasyX 窗口
    initgraph(winW, winH);
    HWND hwnd = GetHWnd();


    bool dragging = false;


    // 去掉标题栏和调整样式
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME);
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, NULL, 0, 0, winW, winH, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);


    // 设定窗口 region（圆角），同时我们也会在客户区绘制圆角背景以避免黑角
    HRGN rgn = CreateRoundRectRgn(0, 0, winW, winH, 20, 20);
    SetWindowRgn(hwnd, rgn, TRUE);

    // 恢复你之前的缩放（坐标系映射）
    setaspectratio(1.0 / scale, 1.0 / scale);


    IMAGE taskBar = IMAGE(winW * scale, 40 * scale);

    SetWorkingImage(&taskBar);
    solidgradientrect(0, 0, winW * scale, 40 * scale, RGB(180, 180, 180), RGB(220, 220, 220), false);
    SetWorkingImage(NULL);

    BeginBatchDraw();


    POINT dragStartScreen = { 0, 0 };
    RECT winStartRect = { 0,0,0,0 };

    const int titleH = 40;   // 基坐标（像素）
    const int btnSize = 30;  // 基坐标（像素）
    const int margin = 5;   // 基坐标（像素）


    // --- 关闭按钮（基坐标 & 缩放坐标） ---
    int base_btnX1 = winW - btnSize - margin;
    int base_btnY1 = margin;
    int base_btnX2 = winW - margin;
    int base_btnY2 = base_btnY1 + btnSize;

    int s_btnX1 = base_btnX1 * scale;
    int s_btnY1 = base_btnY1 * scale;
    int s_btnX2 = base_btnX2 * scale;
    int s_btnY2 = base_btnY2 * scale;

    DWORD  lastTime = GetTickCount();

    while (1) {
        ExMessage msg;
        getmessage(&msg);
        switch (msg.message)
        {
        case WM_MOVE:
        case WM_MOUSEMOVE: {

            //--- 绘制（使用 缩放后坐标 * scale） ---
            cleardevice();

            // 背景（用圆角矩形覆盖黑角）
            setfillcolor(RGB(250, 250, 250));
            solidroundrect(0, 0, winW * scale, winH * scale, 20 * scale, 20 * scale);

            // 标题栏（绘制到缩放坐标）
            putimage(0, 0, &taskBar);

            // --- 设置清晰字体（LOGFONT + ClearType） ---
            LOGFONT f;
            gettextstyle(&f);
            f.lfHeight = 20;
            _tcscpy_s(f.lfFaceName, LF_FACESIZE, _T("微软雅黑"));
#ifdef CLEARTYPE_QUALITY
            f.lfQuality = CLEARTYPE_QUALITY;
#else
            f.lfQuality = ANTIALIASED_QUALITY;
#endif
            settextstyle(&f);
            setbkmode(TRANSPARENT);
            settextcolor(BLACK);

            outtextxy(10 * scale, 10 * scale, _T("wingetUI"));


            // 鼠标位置（屏幕->客户区，单位为基像素）
            POINT p;
            GetCursorPos(&p);
            ScreenToClient(hwnd, &p);

            // 使用基坐标进行 hit test（p 已经是客户区像素坐标）
            bool hover = (p.x >= base_btnX1 && p.x <= base_btnX2 && p.y >= base_btnY1 && p.y <= base_btnY2);

            // 绘制关闭按钮（使用缩放坐标）
            setfillcolor(hover ? RGB(240, 100, 100) : RGB(220, 50, 50));
            solidroundrect(s_btnX1, s_btnY1, s_btnX2, s_btnY2, 8 * scale, 8 * scale);

            setlinecolor(WHITE);
            setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 3); // 线宽随缩放放大
            int offset = 6 * scale;
            line(s_btnX1 + offset, s_btnY1 + offset, s_btnX2 - offset, s_btnY2 - offset);
            line(s_btnX1 + offset, s_btnY2 - offset, s_btnX2 - offset, s_btnY1 + offset);

            FlushBatchDraw();
            break;
        }
        case WM_LBUTTONDOWN:
            if (msg.y <= titleH) { // 标题栏区域
                dragging = true;
                dragStartScreen.x = msg.x;
                dragStartScreen.y = msg.y;
                std::thread([hwnd]() {
                    PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                    }).detach();
            }
            if (msg.x >= base_btnX1 && msg.x <= base_btnX2 && msg.y >= base_btnY1 && msg.y <= base_btnY2) {

                EndBatchDraw();
                closegraph();
                exit(0);
            }
            break;
        case WM_LBUTTONUP:
            dragging = false;
            break;
        }
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
