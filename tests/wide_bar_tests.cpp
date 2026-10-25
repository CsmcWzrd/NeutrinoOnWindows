#include "Neutrino/Neutrino.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
using namespace neutrino;
static int checks = 0;
static void check(bool ok, const char* message)
{
    ++checks;
    if (!ok) throw std::runtime_error(message);
}
static void mouse(Neu_Window& w, UINT msg, int x, int y)
{
    SendMessageW(w.xid(), msg, msg == WM_LBUTTONDOWN ? MK_LBUTTON : 0, MAKELPARAM(x, y));
}
static void click(Neu_Window& w, int x, int y)
{
    mouse(w, WM_LBUTTONDOWN, x, y);
    mouse(w, WM_LBUTTONUP, x, y);
}
static void countClick(Neu_Control*, void* p) { ++*static_cast<int*>(p); }
static void closeMenu(Neu_Control* c, void*) { c->parent()->closeNeutrinoMenu(); }
static void removeMenu(Neu_Control* c, void*) { c->parent()->setNeutrinoMenu(nullptr); }
class InputProbe : public Neu_Button {
public:
    using Neu_Button::Neu_Button;
    int keys{0}, wheels{0};
    void handleXEvent(XEvent& ev) override {
        if (ev.message == WM_KEYDOWN || ev.message == WM_CHAR) ++keys;
        if (ev.message == WM_MOUSEWHEEL || ev.message == WM_MOUSEHWHEEL) ++wheels;
        Neu_Button::handleXEvent(ev);
    }
};
static void capture(Neu_Window& w, const char* name)
{
    const char* dir = std::getenv("NEUTRINO_TEST_CAPTURE_DIR");
    if (!dir) return;
    HDC screen = GetDC(w.xid());
    HDC dc = CreateCompatibleDC(screen);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = w.width();
    info.bmiHeader.biHeight = -w.height();
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* pixels = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    auto old = SelectObject(dc, bitmap);
    w.paint(dc);
    GdiFlush();
    BITMAPFILEHEADER header{};
    header.bfType = 0x4d42;
    header.bfOffBits = sizeof(header) + sizeof(info.bmiHeader);
    header.bfSize = header.bfOffBits + w.width()*w.height()*4;
    std::ofstream out(std::string(dir) + "/" + name + ".bmp", std::ios::binary);
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    out.write(reinterpret_cast<const char*>(&info.bmiHeader), sizeof(info.bmiHeader));
    out.write(static_cast<const char*>(pixels), w.width()*w.height()*4);
    check(static_cast<bool>(out), "render artifact saved");
    SelectObject(dc, old); DeleteObject(bitmap); DeleteDC(dc); ReleaseDC(w.xid(), screen);
}
static void render(Neu_Window& w, bool open)
{
    HDC screen = GetDC(w.xid());
    HDC dc = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateCompatibleBitmap(screen, w.width(), w.height());
    auto old = SelectObject(dc, bitmap);
    w.paint(dc);
    if (open) {
        const auto r = w.neutrinoMenuCloseBounds();
        check(GetPixel(dc, r.x + r.width / 2, r.y + 5) == RGB(220,35,45), "close button red circle");
        bool white = false;
        for (int y = r.y + r.height/3; y < r.y + 2*r.height/3; ++y)
            for (int x = r.x + r.width/3; x < r.x + 2*r.width/3; ++x)
                white |= GetPixel(dc, x, y) == RGB(255,255,255);
        check(white, "close button white X");
    }
    SelectObject(dc, old);
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(w.xid(), screen);
}
int main()
{
    try {
        Neu_Application app;
        check(app.open(), "application opens");
        Neu_Window w(app, 800, 600, "Wide bar integration tests");
        Neu_Window other(app, 400, 300, "Secondary integration tests");
        auto menu = std::make_shared<Neu_PopWindowMenu>();
        check(!w.setNeutrinoMenu(menu), "attach requires created main window");
        check(w.create() && other.create(), "native windows created");
        check(app.mainWindow() == &w, "first registered window is main");
        check(!other.setNeutrinoMenu(menu), "secondary window rejected");
        check(!other.wideBarVisible(), "secondary has no bar");
        check(w.setNeutrinoMenu(menu), "main accepts menu");
        menu->setCategories({"File", "Edit"});
        menu->setItems("File", {"Open", "Save"});
        check(w.wideBarVisible() && !w.neutrinoMenuOpen(), "initial closed state");
        check(w.wideBarHeight() == 48, "bar is exactly three times the 16-pixel menu font");
        auto toolbar = std::make_shared<Neu_ToolBar>(Neu_Layout{0, 100, 600, 50});
        w.add(toolbar);
        check(w.wideBarHeight() == 48, "toolbar does not enlarge font-sized bar");
        w.setWideBarReferenceHeight(60);
        check(w.wideBarHeight() == 48, "legacy reference does not enlarge font-sized bar");
        int clicks = 0;
        auto button = std::make_shared<InputProbe>(Neu_Layout{200, 200, 200, 50});
        button->setText("Underlying button");
        Neu_Callbacks cb; cb.onClick = countClick; cb.userData = &clicks;
        button->setCallbacks(cb); w.add(button);
        click(w, 250, 225);
        check(clicks == 1, "normal controls still click with bar attached");
        check(button->focused(), "underlying focus acquired");
        mouse(w, WM_LBUTTONDOWN, 250, 225);
        mouse(w, WM_LBUTTONUP, 20, 20);
        check(clicks == 1 && GetCapture() != w.xid(), "drag from workspace to bar releases capture without click");
        capture(w, "wide-bar");
        mouse(w, WM_LBUTTONDOWN, 20, 20);
        mouse(w, WM_LBUTTONUP, 20, 160);
        check(!w.neutrinoMenuOpen(), "drag off bar cancels activation");
        mouse(w, WM_LBUTTONUP, 20, 20);
        check(!w.neutrinoMenuOpen(), "unpaired release does not activate");
        click(w, 20, 20);
        check(w.neutrinoMenuOpen() && !w.wideBarVisible(), "bar click opens and fully hides bar");
        check(!button->focused(), "underlying keyboard focus suspended");
        click(w, 250, 225);
        SendMessageW(w.xid(), WM_CHAR, 'x', 0);
        SendMessageW(w.xid(), WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA), 0);
        check(button->keys == 0 && button->wheels == 0, "keyboard and wheel isolated from workspace");
        check(clicks == 1, "overlay blocks clicks and keyboard from underlying content");
        auto action = std::make_shared<Neu_MenuItem>(Neu_Layout{220, 320, 250, 44});
        action->setText("Close from command");
        Neu_Callbacks closeCb; closeCb.onClick = closeMenu;
        action->setCallbacks(closeCb); menu->add(action);
        render(w, true);
        capture(w, "menu-overlay");
        auto r = w.neutrinoMenuCloseBounds();
        click(w, r.x, r.y);
        check(w.neutrinoMenuOpen(), "outside circle corner does not close");
        mouse(w, WM_LBUTTONDOWN, r.x+r.width/2, r.y+r.height/2);
        mouse(w, WM_LBUTTONUP, 30, 180);
        check(w.neutrinoMenuOpen(), "drag off X cancels close");
        click(w, r.x+r.width/2, r.y+r.height/2);
        check(!w.neutrinoMenuOpen() && w.wideBarVisible(), "red X restores bar");
        check(button->focused() && clicks == 1, "focus restored without click through");
        capture(w, "bar-restored");
        w.openNeutrinoMenu();
        click(w, 250, 340);
        check(w.wideBarVisible(), "menu child callback closes overlay");
        w.openNeutrinoMenu();
        SendMessageW(w.xid(), WM_KEYDOWN, VK_ESCAPE, 0);
        check(w.wideBarVisible(), "Escape closes overlay");
        w.openNeutrinoMenu(); menu->hide(); render(w, false);
        check(w.wideBarVisible() && button->focused(), "direct menu hide restores focus and bar");
        menu->show(); render(w, true);
        check(w.neutrinoMenuOpen() && !button->focused(), "direct menu show enters overlay session");
        SetWindowPos(w.xid(), nullptr, 0, 0, 1030, 730, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        render(w, true);
        check(menu->bounds().width == w.width() && menu->bounds().height == w.height()-w.wideBarHeight(), "resize updates overlay geometry");
        for (const auto& theme : {Neu_Theme::MaterialDark(), Neu_Theme::MaterialLight()}) {
            w.setTheme(theme);
            for (bool buffered : {false, true}) {
                w.setMultiStageDoubleBuffering(buffered);
                render(w, true);
            }
        }
        w.closeNeutrinoMenu();
        const auto before = button->bounds();
        for (int i=0; i<200; ++i) { click(w, 20, 20); SendMessageW(w.xid(), WM_KEYDOWN, VK_ESCAPE, 0); }
        check(w.wideBarVisible() && button->bounds().x == before.x && button->bounds().y == before.y, "200 cycles preserve content layout");
        mouse(w, WM_LBUTTONDOWN, 20, 20);
        SendMessageW(w.xid(), WM_CANCELMODE, 0, 0);
        mouse(w, WM_LBUTTONUP, 20, 20);
        check(w.wideBarVisible(), "cancel mode clears pending bar click");
        mouse(w, WM_LBUTTONDOWN, 20, 20);
        SendMessageW(w.xid(), WM_CAPTURECHANGED, 0, 0);
        mouse(w, WM_LBUTTONUP, 20, 20);
        check(w.wideBarVisible(), "capture loss cancels pending bar click");
        const DWORD resourcesBefore = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
        for (int i=0; i<20; ++i) { w.openNeutrinoMenu(); render(w, true); w.closeNeutrinoMenu(); render(w, false); }
        check(GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS) <= resourcesBefore + 2, "repaint cycles do not leak GDI resources");
        // A command can remove its own owning menu while its callback is running.
        auto disposable = std::make_shared<Neu_PopWindowMenu>();
        auto remove = std::make_shared<Neu_MenuItem>(Neu_Layout{220, 320, 250, 44});
        Neu_Callbacks removeCb; removeCb.onClick = removeMenu; remove->setCallbacks(removeCb);
        disposable->add(remove);
        check(w.setNeutrinoMenu(disposable), "attach self-removing menu");
        disposable.reset(); remove.reset(); w.openNeutrinoMenu(); click(w, 250, 340);
        check(!w.wideBarVisible() && !w.neutrinoMenuOpen(), "self-removing callback finishes safely");
        check(w.setNeutrinoMenu(menu), "reattach original menu");
        w.openNeutrinoMenu();
        SendMessageW(w.xid(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(24, 24));
        render(w, true);
        check(w.neutrinoMenuCloseBounds().x >= 0, "tiny client area keeps close button in bounds");
        SendMessageW(w.xid(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(800, 600));
        auto replacement = std::make_shared<Neu_PopWindowMenu>();
        w.openNeutrinoMenu();
        check(w.setNeutrinoMenu(replacement) && !menu->parent() && w.wideBarVisible(), "replace open menu safely");
        check(w.setNeutrinoMenu(nullptr) && !replacement->parent() && !w.wideBarVisible(), "remove menu and bar");
        w.add(menu);
        check(!w.setNeutrinoMenu(menu), "reject duplicate normal control ownership");
        w.close();
        check(!app.mainWindow() && !other.setNeutrinoMenu(replacement), "secondary not promoted after main closes");
        other.close();
        std::cout << "PASS: " << checks << " integration checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}
