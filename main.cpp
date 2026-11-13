#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <optional> // SFML 3 需要这个
#include <iostream>

// 窗口过程回调函数（用于拖拽）
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCHITTEST) {
        // 返回 HTCAPTION 让系统认为点在标题栏上，自动支持拖拽
        return HTCAPTION;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main() {
    // 1. 创建无边框窗口 (注意 SFML 3 的写法)
    sf::RenderWindow window(sf::VideoMode({512, 512}), "Pet", sf::Style::None);
    HWND hwnd = window.getNativeHandle(); // 获取 Windows 原生窗口句柄

    // 2. 设置扩展样式：透明、置顶、不占任务栏
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST ;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

    // 3. 设置颜色键透明（把纯黑色当作透明色抠掉）
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // 4. 子类化窗口，拦截拖拽消息
    WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    // 5. 加载宠物图片
    sf::Texture texture;
    if (!texture.loadFromFile("mmc.png"))
    {
        std::cerr << "Error: 找不到 mmc.png，请检查图片是否在正确的目录下！" << std::endl;
        return -1;
    }
    sf::Sprite pet(texture);
    pet.setPosition({0.f, 0.f});

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // 【关键】清屏颜色必须是纯黑，这样才会被上面的颜色键变成透明！
        window.clear(sf::Color::Black); 
        window.draw(pet);
        window.display();
    }

    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldProc);
    return 0;
}