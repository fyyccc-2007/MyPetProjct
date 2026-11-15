#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <optional> // SFML 3 需要这个
#include <iostream>
#include <vector>
#include <string>

RECT g_workArea;
// 窗口过程回调函数（用于拖拽）
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCHITTEST) {
        // 返回 HTCAPTION 让系统认为点在标题栏上，自动支持拖拽
        return HTCAPTION;
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
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main() {
    // 1. 创建无边框窗口 (注意 SFML 3 的写法)
    sf::RenderWindow window(sf::VideoMode({128, 128}), "Pet", sf::Style::None);
    HWND hwnd = window.getNativeHandle(); // 获取 Windows 原生窗口句柄

    // 2. 设置扩展样式：透明、置顶、不占任务栏
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST ;
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

    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldProc);
    return 0;
}