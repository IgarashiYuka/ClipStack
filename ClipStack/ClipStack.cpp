// ClipStack.cpp : 定义应用程序的入口点。
//
//#define STORE_AS_QUEUE

#include "framework.h"
#include "ClipStack.h"
#include "mshtmcid.h"
#include "shellapi.h"
#include "WinUser.h"
#include <string>
#include <vector>

#ifdef STORE_AS_QUEUE
#include <queue>
#else
#include <stack>
#endif

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                     // 当前实例
WCHAR szTitle[MAX_LOADSTRING];       // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名
// 此代码模块中包含的函数的前向声明:

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
std::vector<wchar_t> last_item = {L' '};
#ifdef STORE_AS_QUEUE
std::queue
#else 
std::stack
#endif
<std::vector<wchar_t>> cliper_storage;
bool is_Registered = false;
uint32_t color = 0xf1f288;

#define APPWM_ICONNOTIFY (WM_APP + 1)
static GUID tray_guid = {
    0x88f1f2c0, 0x1707, 0x1505, {
        0x88, 0xf1, 0xf2, 0xc0, 0x88, 0xf1, 0xf2, 0xa1
    }
};
UINT s_uTaskbarRestart;
NOTIFYICONDATAW trayicon = {};

uint32_t hashCode(char* input) {
    uint32_t hash = 0;
    size_t i = strlen(input);
    while (i--) hash = (hash << 5) - hash + *(input++);
    return hash & 0XFFFFFFFF;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    char path[_MAX_PATH + 1];
    GetModuleFileNameA(NULL, (char*)path, _MAX_PATH);
    tray_guid.Data1 = hashCode(path);
    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIPSTACK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIPSTACK));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIPSTACK));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIPSTACK);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
/* CreateSmallIcon: https://stackoverflow.com/a/457432*/
HICON CreateSmallIcon( HWND hWnd , std::wstring value, size_t font_size, size_t x, size_t y, uint32_t color)
{
    const wchar_t* szText = value.c_str();
    HDC hdc, hdcMem;
    HBITMAP hBitmap = NULL;
    HBITMAP hOldBitMap = NULL;
    HBITMAP hBitmapMask = NULL;
    ICONINFO iconInfo;
    HFONT hFont;
    HICON hIcon;
    hdc = GetDC ( hWnd );
    hdcMem = CreateCompatibleDC ( hdc );
    SetBkMode(hdcMem, TRANSPARENT); // VERY IMPORTANT
    hBitmap = CreateCompatibleBitmap ( hdc, 16, 16 );
    hBitmapMask = CreateCompatibleBitmap ( hdc, 16, 16 );
    ReleaseDC ( hWnd, hdc );
    hOldBitMap = (HBITMAP) SelectObject ( hdcMem, hBitmap );
    PatBlt(hdcMem, 0, 0, 16, 16, WHITENESS);

    // Draw percentage
    hFont = CreateFont (font_size, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    TEXT ("Arial"));
    hFont = (HFONT) SelectObject ( hdcMem, hFont );
    SetTextColor(hdcMem, (~color)&0x00ffffff); // 0x00bbggrr, not rrggbb !!
    TextOut ( hdcMem, x, y, szText, lstrlen (szText) );
    PatBlt(hdcMem, 0, 0, 16, 16, DSTINVERT);

    SelectObject(hdcMem, hBitmapMask);

    SelectObject ( hdc, hOldBitMap );
    hOldBitMap = NULL;

    iconInfo.fIcon = TRUE;
    iconInfo.xHotspot = 0;
    iconInfo.yHotspot = 0;
    iconInfo.hbmMask = hBitmapMask;
    iconInfo.hbmColor = hBitmap;

    hIcon = CreateIconIndirect ( &iconInfo );

    DeleteObject ( SelectObject ( hdcMem, hFont ) );
    DeleteDC ( hdcMem );
    DeleteDC ( hdc );
    DeleteObject ( hBitmap );
    DeleteObject ( hBitmapMask );

    return hIcon;
}
void RefreshBuffer(size_t value, HWND hWnd)
{
    size_t font_size, x, y;
    if (value >= 1000) value = 999;
    if (value < 10) {
        font_size = 24;
        x = 1; 
        y = -3;
    }
    else if (value < 100) {
        font_size = 18;
        x = -1;
        y = -1;
    }
    else if (value < 1000) {
        font_size = 12;
        x = 0;
        y = 3;
    }
    trayicon.uFlags = NIF_MESSAGE | NIF_TIP | NIF_GUID | NIF_SHOWTIP | NIF_ICON;
    trayicon.hIcon = CreateSmallIcon(hWnd, std::to_wstring(value), font_size, x, y, color);//CreateIconIndirect(&oldIconInfo);
    Shell_NotifyIconW(NIM_MODIFY, &trayicon);
    DestroyIcon(trayicon.hIcon);
}
void AddTaskbarIcons(HWND hWnd) {
    trayicon.cbSize = sizeof(trayicon);
    trayicon.hWnd = hWnd;
    trayicon.uFlags = NIF_MESSAGE | NIF_TIP | NIF_GUID | NIF_SHOWTIP;
    trayicon.guidItem = tray_guid;
    trayicon.uCallbackMessage = APPWM_ICONNOTIFY;
    wcscpy_s(trayicon.szTip, L"Right Click to EXIT, <C-S-V> to paste ALL stack items to editor");
    Shell_NotifyIconW(NIM_ADD, &trayicon);
    trayicon.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &trayicon);
    RefreshBuffer(cliper_storage.size(), hWnd);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    //ShowWindow(hWnd, nCmdShow);
    //UpdateWindow(hWnd);
    //ShowWindow(hWnd, SW_HIDE);
    AddClipboardFormatListener(hWnd);
    RegisterHotKey(hWnd, 2, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'A');
    RegisterHotKey(hWnd, 3, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'V');
    
    AddTaskbarIcons(hWnd);
    s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
    while (!cliper_storage.empty())
        cliper_storage.pop();
    return TRUE;
}
//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
bool skipNext = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD tmp;
    std::vector<wchar_t> tmp2;
    size_t tmp3;
    std::vector<wchar_t> *tmp4;
    LPWSTR tmp5;
    std::wstring tmp6;
    HGLOBAL hglb;
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        
        // 分析菜单选择:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            do{
                NOTIFYICONDATAW trayicon = {};
                trayicon.cbSize = sizeof(trayicon);
                trayicon.hWnd = hWnd;
                trayicon.uFlags = NIF_GUID;
                trayicon.guidItem = tray_guid;
                Shell_NotifyIconW(NIM_DELETE, &trayicon);
            }while(0);
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_HOTKEY:
        switch (wParam)
        {
        case 1:
            if (cliper_storage.empty())
            {
                tmp4 = &last_item;
                color = 0x0080FF;
            }
            else
            {
                tmp4 = &(cliper_storage.
#ifdef STORE_AS_QUEUE           
                    front()
#else
                    top()
#endif
                );
                color = 0xf1f288;
            }
            //GlobalFree(hMem);
            hglb = GlobalAlloc(GMEM_MOVEABLE, tmp4->size() * sizeof(wchar_t));
            if (hglb == NULL)
                break;
            tmp5 = (LPWSTR)GlobalLock(hglb);
            tmp = GlobalSize(hglb);
            if (tmp5 == NULL)
                break;
            tmp4->push_back(L'\0');
            for (size_t i = 0; i < tmp4->size(); i++)
            {
                tmp5[i] = tmp4->operator[](i);
            }
            tmp4->pop_back();
            GlobalUnlock(hglb);
            if (!cliper_storage.empty())
                cliper_storage.pop();
            skipNext = true;
            OpenClipboard(0);
            EmptyClipboard();
            SetClipboardData(CF_UNICODETEXT, hglb);
            CloseClipboard();

            UnregisterHotKey(hWnd, 1);
            // Simulate a key release
            keybd_event(0x56,
                        0,
                        KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                        0);
            // Simulate a key press
            keybd_event(0x56,
                        0,
                        KEYEVENTF_EXTENDEDKEY | 0,
                        0);
            RegisterHotKey(hWnd, 1, MOD_CONTROL | MOD_NOREPEAT, 'V');
            RefreshBuffer(cliper_storage.size(), hWnd);
            break;
        case 2:
            while (!cliper_storage.empty())
                cliper_storage.pop();
            color = 0x0080ff;
            RefreshBuffer(cliper_storage.size(), hWnd);
            break;
        case 3:
            if (!cliper_storage.empty())
            {
                last_item.clear();
                while (!cliper_storage.empty())
                {
                    for (auto i : cliper_storage.
#ifdef STORE_AS_QUEUE
                        front()
#else
                        top()
#endif
                        )
                    {
                        last_item.push_back(i);
                    }
                    last_item.pop_back();
                    last_item.push_back(L'\r');
                    last_item.push_back(L'\n');
                    cliper_storage.pop();
                }
                last_item.pop_back();
                last_item.pop_back();
                last_item.push_back(L'\0');
                skipNext = true;
                hglb = GlobalAlloc(GMEM_MOVEABLE, last_item.size() * sizeof(wchar_t));
                if (hglb == NULL)
                    break;
                tmp5 = (LPWSTR)GlobalLock(hglb);
                if (tmp5 == NULL)
                    break;
                for (size_t i = 0; i < last_item.size(); i++)
                {
                    tmp5[i] = last_item[i];
                }
                last_item.pop_back();
                OpenClipboard(0);
                EmptyClipboard();
                SetClipboardData(CF_UNICODETEXT, hglb);
                CloseClipboard();

                UnregisterHotKey(hWnd, 3);
                keybd_event('V',
                            0,
                            KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                            0);
                // Simulate a key press
                keybd_event('V',
                            0,
                            KEYEVENTF_EXTENDEDKEY | 0,
                            0);
                RegisterHotKey(hWnd, 3, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'V');
                color = 0x0080ff;
                RefreshBuffer(cliper_storage.size(), hWnd);
            }
            break;
        }
        break;

    case APPWM_ICONNOTIFY:
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
        case WM_CONTEXTMENU:
            SendMessageA(hWnd, WM_CLOSE, NULL, NULL);
            break;
        default:
            break;
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CLIPBOARDUPDATE:
        if (IsClipboardFormatAvailable(CF_UNICODETEXT))
        {
            if (skipNext)
            {
                skipNext = false;
                break;
            }
            if (!OpenClipboard(hWnd))
            {
                tmp = GetLastError();
                tmp = tmp;
            }
            hglb = GetClipboardData(CF_UNICODETEXT);
            if (hglb != NULL)
            {
                tmp5 = (LPWSTR)GlobalLock(hglb);
                if (tmp5 == NULL)
                    break;
                tmp3 = wcslen(tmp5);
                if (!tmp3)
                    break;
                last_item.reserve(tmp3 + 1);
                last_item.resize(tmp3);
                for (size_t i = 0; i < tmp3; i++)
                {
                    last_item[i] = tmp5[i];
                }
                last_item.push_back(L'\0');
                cliper_storage.push(last_item);
                if (!is_Registered)
                    RegisterHotKey(hWnd, 1, MOD_CONTROL | MOD_NOREPEAT, 0x56);
                is_Registered = true;
                color = 0xf1f288;
                RefreshBuffer(cliper_storage.size(), hWnd);
            }
            else
            {
                tmp = GetLastError();
                tmp = tmp;
            }
            CloseClipboard();
        }
        else
        {
            if (is_Registered)
                UnregisterHotKey(hWnd, 1);
            color = 0x0080ff;
            is_Registered = false;
            RefreshBuffer(cliper_storage.size(), hWnd);
        }
    default:
        if (message == s_uTaskbarRestart) {
            AddTaskbarIcons(hWnd);
            break;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
