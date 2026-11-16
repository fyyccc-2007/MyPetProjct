#define SFML_STATIC
#include <SFML/Graphics.hpp>

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif


#include <windows.h>
#include <optional> 
#include <iostream>
#include <vector>
#include <string>
#include <shellapi.h>
// 托盘点击消息
#define WM_TRAYICON (WM_APP + 1)
// 菜单项的 ID
#define ID_TRAY_EXIT 1001

NOTIFYICONDATAW g_nid = {};//托盘图标数据结构
RECT g_workArea;
// 窗口过程回调函数（用于拖拽）

void ShowMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);//获取鼠标坐标
    
    //创建右键菜单
    HMENU hMenu=CreatePopupMenu();
    AppendMenu(hMenu,MF_STRING,ID_TRAY_EXIT,L"退出桌宠");

    //显示菜单
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu,TPM_RIGHTALIGN,pt.x,pt.y,0,hwnd,NULL);
    PostMessage(hwnd,WM_NULL,0,0);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCHITTEST) {
        // 返回 HTCAPTION 让系统认为点在标题栏上，自动支持拖拽
        return HTCAPTION;
    }
    if (msg == WM_NCRBUTTONUP)
    {
        ShowMenu(hwnd); 
        return 0;              
    }
    // 监听拖拽结束消息
    if(msg==WM_EXITSIZEMOVE)
    {
        RECT winRect;
        GetWindowRect(hwnd,&winRect);
        int winHeight=winRect.bottom-winRect.top;
        int distance=abs(g_workArea.bottom-winRect.bottom),lim_distance=50;
        if(distance<=lim_distance)
        {
            int newY=g_workArea.bottom-winHeight;
            SetWindowPos(hwnd,NULL,winRect.left,newY,0,0, 
                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
    if(msg==WM_TRAYICON)
    {
        if(lParam==WM_RBUTTONUP)
        {
            ShowMenu(hwnd);
        }
    }
    if(msg==WM_COMMAND)
    {
        if(LOWORD(wParam)==ID_TRAY_EXIT)
        {
            Shell_NotifyIconW(NIM_DELETE, &g_nid); 
            ExitProcess(0); 
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
    
}

int main() {
    // 1. 创建无边框窗口 (注意 SFML 3 的写法)
    sf::RenderWindow window(sf::VideoMode({128, 128}), "Pet", sf::Style::None);
    HWND hwnd = window.getNativeHandle(); // 获取 Windows 原生窗口句柄

    // 2. 设置扩展样式：透明、置顶、不占任务栏
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
    //设置置顶
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    // 3. 设置颜色键透明（把纯黑色当作透明色抠掉）
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // 4. 子类化窗口，拦截拖拽消息
    WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    // 5. 加载宠物图片
    std::vector<sf::Texture> texture;
    int frameCount=10;
    for(int i=0;i<frameCount;i++)
    {
        sf::Texture tex;
        std::string fileName="res/mmc_write/mmc"+std::to_string(i)+".png";
        if (!tex.loadFromFile(fileName))
        {
            std::cerr << "Error: 找不到 "<<fileName<<"，请检查图片是否在正确的目录下！" << std::endl;
            return -1;
        }
        texture.push_back(std::move(tex));
    }
    sf::Sprite pet(texture[0]);
    pet.setPosition({0.f, 0.f});

    // 获取当前屏幕的工作区
    SystemParametersInfo(SPI_GETWORKAREA,0,&g_workArea,0);

    g_nid.cbSize = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd = hwnd;                    // 绑定到你的宠物窗口
    g_nid.uID = 1;                        // 图标 ID
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; 
    g_nid.uCallbackMessage = WM_TRAYICON; // 点击图标时，发送自定义的消息
    g_nid.hIcon = LoadIconW(NULL, IDI_APPLICATION); // 暂时用系统默认图标
    wcscpy_s(g_nid.szTip, L"桌宠"); // 鼠标悬停时的提示文字

    Shell_NotifyIconW(NIM_ADD, &g_nid);
    int currentFrame=0;
    float frameTime=0.1f;
    sf::Clock animClock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if(animClock.getElapsedTime().asSeconds()>=frameTime)
        {
            currentFrame++;
            if(currentFrame>=frameCount) currentFrame=0;
            pet.setTexture(texture[currentFrame]);
            animClock.restart();
        }

        window.clear(sf::Color::Black); 
        window.draw(pet);
        window.display();
    }

    Shell_NotifyIconW(NIM_DELETE, &g_nid);

    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldProc);
    return 0;
}