// wingetUI.cpp
#include <graphics.h>   // EasyX
#include <windows.h>
#include <tchar.h>
#include "framework.h"

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

void solidgradientrect(int x1, int y1, int x2, int y2,
    COLORREF c1, COLORREF c2, bool vertical = true)
{
    if (vertical) {
        int height = y2 - y1;
        for (int i = 0; i < height; i++) {
            double t = (double)i / height;
            int r = GetRValue(c1) + (int)((GetRValue(c2) - GetRValue(c1)) * t);
            int g = GetGValue(c1) + (int)((GetRValue(c2) - GetRValue(c1)) * t);
            int b = GetBValue(c1) + (int)((GetRValue(c2) - GetRValue(c1)) * t);
            setlinecolor(RGB(r, g, b));
            line(x1, y1 + i, x2, y1 + i);
        }
    }
    else { // 水平渐变
        int width = x2 - x1;
        for (int i = 0; i < width; i++) {
            double t = (double)i / width;
            int r = GetRValue(c1) + (int)((GetRValue(c2) - GetRValue(c1)) * t);
            int g = GetGValue(c1) + (int)((GetRValue(c2) - GetRValue(c1)) * t);
            int b = GetBValue(c1) + (int)((GetRValue(c2) - GetRValue(c1)) * t);
            setlinecolor(RGB(r, g, b));
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
    const int scale = 4; // 恢复缩放因子

    // 初始化 EasyX 窗口
    initgraph(winW, winH);
    HWND hwnd = GetHWnd();

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

    BeginBatchDraw();

    // 拖动状态
    bool dragging = false;
    POINT dragStartScreen = { 0, 0 };
    RECT winStartRect = { 0,0,0,0 };

    const int titleH = 40;   // 基坐标（像素）
    const int btnSize = 30;  // 基坐标（像素）
    const int margin = 5;   // 基坐标（像素）

    while (1) {
        // --- 绘制（使用 缩放后坐标 * scale） ---
        cleardevice();

        // 背景（用圆角矩形覆盖黑角）
        setfillcolor(RGB(250, 250, 250));
        solidroundrect(0, 0, winW * scale, winH * scale, 20 * scale, 20 * scale);

        // 标题栏（绘制到缩放坐标）
        setfillcolor(RGB(200, 200, 200));
        solidrectangle(0, 0, winW * scale, titleH * scale);

        // --- 设置清晰字体（LOGFONT + ClearType） ---
        LOGFONT f;
        gettextstyle(&f);
        f.lfHeight = 90; // 使用你原来的大字号（恢复为较大值以配合缩放）
        _tcscpy_s(f.lfFaceName, LF_FACESIZE, _T("微软雅黑"));
#ifdef CLEARTYPE_QUALITY
        f.lfQuality = CLEARTYPE_QUALITY;
#else
        f.lfQuality = ANTIALIASED_QUALITY;
#endif
        settextstyle(&f);
        setbkmode(TRANSPARENT);
        settextcolor(BLACK);

        // 注意：这里 outtextxy 使用的是“缩放后的坐标系单位”，
        // 你原先的代码写 outtextxy(10,10,...) 配合 setaspectratio 是可行的，
        // 为保持原样我也用 10*scale,10*scale 来绘制文字位置更稳定。
        outtextxy(10 * scale, 10 * scale, _T("wingetUI"));

        // --- 关闭按钮（基坐标 & 缩放坐标） ---
        int base_btnX1 = winW - btnSize - margin;
        int base_btnY1 = margin;
        int base_btnX2 = winW - margin;
        int base_btnY2 = base_btnY1 + btnSize;

        int s_btnX1 = base_btnX1 * scale;
        int s_btnY1 = base_btnY1 * scale;
        int s_btnX2 = base_btnX2 * scale;
        int s_btnY2 = base_btnY2 * scale;

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
        setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 4 * (scale / 2)); // 线宽随缩放放大
        int offset = 6 * scale;
        line(s_btnX1 + offset, s_btnY1 + offset, s_btnX2 - offset, s_btnY2 - offset);
        line(s_btnX1 + offset, s_btnY2 - offset, s_btnX2 - offset, s_btnY1 + offset);

        FlushBatchDraw();

        // --- 鼠标按下处理：关闭按钮 / 自绘拖动（限制工作区） ---
        SHORT keyState = GetAsyncKeyState(VK_LBUTTON);
        if (keyState & 0x8000) {
            // 点击关闭按钮（基坐标判断）
            if (hover && !dragging) {
                EndBatchDraw();
                closegraph();
                return 0;
            }

            // 开始拖动（仅在标题栏区域，基坐标判断）
            if (!dragging && p.y <= titleH) {
                dragging = true;
                GetCursorPos(&dragStartScreen);
                GetWindowRect(hwnd, &winStartRect);
            }

            // 正在拖动时移动窗口（使用屏幕坐标，不受缩放影响）
            if (dragging) {
                POINT now;
                GetCursorPos(&now);
                int dx = now.x - dragStartScreen.x;
                int dy = now.y - dragStartScreen.y;

                int newLeft = winStartRect.left + dx;
                int newTop = winStartRect.top + dy;

                RECT work;
                SystemParametersInfo(SPI_GETWORKAREA, 0, &work, 0);
                int winWpx = winStartRect.right - winStartRect.left;
                int winHpx = winStartRect.bottom - winStartRect.top;

                SetWindowPos(hwnd, NULL, newLeft, newTop, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        else {
            // 松开鼠标，结束拖动
            dragging = false;
        }

        Sleep(10);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
