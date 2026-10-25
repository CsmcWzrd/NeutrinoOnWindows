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
    if (open && !w.neutrinoMenuUsesNormalCloseButton()) {
        const auto r = w.neutrinoMenuCloseBounds();
        check(GetPixel(dc, r.x + r.width / 2, r.y + 5) == RGB(220,35,45), "close button red circle");
        bool white = false;
        for (int y = r.y + r.height/3; y < r.y + 2*r.height/3; ++y)
            for (int x = r.x + r.width/3; x < r.x + 2*r.width/3; ++x)
                white |= GetPixel(dc, x, y) == RGB(255,255,255);
        check(white, "close button white X");
    } else if (open) {
        const auto r = w.neutrinoMenuCloseBounds();
        check(GetPixel(dc, r.x + r.width/2, r.y + 5) != RGB(220,35,45), "normal button uses theme instead of red circle");
        bool text = false;
        const auto color = w.theme().text;
        for (int y = r.y + 6; y < r.y + r.height - 6; ++y)
            for (int x = r.x + 6; x < r.x + r.width - 6; ++x)
                text |= GetPixel(dc, x, y) == RGB(color.r, color.g, color.b);
        check(text, "normal Close Menu button renders themed text");
    }
    SelectObject(dc, old);
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(w.xid(), screen);
}
static void testHoverColumns(Neu_Window& w, const std::shared_ptr<Neu_PopWindowMenu>& menu)
{
    menu->setCategories({"File", "Edit"});
    menu->setItems("File", {"A", "B", "C", "D", "E", "F", "G"});
    menu->setItems("Edit", {"Undo", "Redo"});
    menu->setSubmenuRowsPerColumn(3);
    w.openNeutrinoMenu();
    const auto m = menu->bounds();
    check(!w.neutrinoMenuUsesNormalCloseButton() && w.neutrinoMenuCloseBounds().width == 16, "default circle is 40 percent of original diameter");
    click(w, 20, m.y + 44);
    check(menu->selectedCategoryIndex() == 0, "category click does not switch submenu");
    check(menu->highlightedCategoryIndex() == -1, "category click does not highlight a row");
    int callbacks = 0;
    Neu_Callbacks cb; cb.onClick = countClick; cb.userData = &callbacks; menu->setCallbacks(cb);
    mouse(w, WM_MOUSEMOVE, 20, m.y + 44);
    check(menu->selectedCategoryIndex() == 1, "hover switches to Edit submenu without click");
    check(menu->highlightedCategoryIndex() == 1, "category highlight follows hover");
    mouse(w, WM_MOUSEMOVE, 20, m.y + 2);
    check(menu->selectedCategoryIndex() == 1, "sidebar top padding does not select first category");
    mouse(w, WM_MOUSEMOVE, 20, m.y + 20);
    check(menu->selectedCategoryIndex() == 0, "hover switches back to File");
    const auto a = menu->submenuItemBounds(0), b = menu->submenuItemBounds(1);
    const auto d = menu->submenuItemBounds(3), g = menu->submenuItemBounds(6);
    check(a.x == b.x && b.y == a.y + 24, "items fill down a column");
    check(d.x > a.x && d.y == a.y && g.x > d.x && g.y == a.y, "next columns continue left to right at top");
    check(menu->submenuItemBounds(7).width == 0, "out of range item has empty bounds");
    const auto dividers = menu->submenuColumnDividers();
    check(dividers.size() == 2, "three columns have two separators");
    for (const auto& line : dividers) {
        check(line.y >= m.y + m.height * 0.1 && line.height <= m.height * 0.8,
              "divider starts at ten percent and is no longer than eighty percent");
        check(line.y + line.height <= m.y + m.height * 0.9, "divider ends by ninety percent");
    }
    mouse(w, WM_MOUSEMOVE, b.x + 10, b.y + 12);
    check(menu->highlightedItemIndex() == 1 && callbacks == 0, "submenu hover highlights without activating command");
    check(menu->highlightedCategoryIndex() == -1, "category highlight clears when pointer enters submenu");
    mouse(w, WM_LBUTTONDOWN, a.x + 10, a.y + 12);
    check(menu->highlightedItemIndex() == 1, "button down does not change hover highlight");
    mouse(w, WM_LBUTTONUP, a.x + 10, a.y + 12);
    const auto e = menu->submenuItemBounds(4);
    mouse(w, WM_MOUSEMOVE, e.x + 10, e.y + 12);
    check(menu->highlightedItemIndex() == 4, "hover works in subsequent columns");
    mouse(w, WM_MOUSEMOVE, dividers[0].x, dividers[0].y + 5);
    check(menu->highlightedItemIndex() == -1, "divider gap has no item highlight");
    mouse(w, WM_MOUSEMOVE, b.x + 10, b.y + 12);
    SendMessageW(w.xid(), WM_MOUSELEAVE, 0, 0);
    check(menu->highlightedItemIndex() == -1, "leaving window clears submenu highlight");
    menu->setEnabled(false);
    mouse(w, WM_MOUSEMOVE, 20, m.y + 44);
    check(menu->selectedCategoryIndex() == 0, "disabled menu ignores category hover");
    menu->setEnabled(true);
    for (const auto& theme : {Neu_Theme::MaterialDark(), Neu_Theme::MaterialLight()}) {
        w.setTheme(theme);
        HDC screen = GetDC(w.xid()), dc = CreateCompatibleDC(screen);
        HBITMAP bitmap = CreateCompatibleBitmap(screen, w.width(), w.height());
        auto old = SelectObject(dc, bitmap); w.paint(dc);
        const auto line = dividers.front();
        check(GetPixel(dc, line.x, line.y + 1) == RGB(theme.border.r, theme.border.g, theme.border.b), "divider uses active theme border color");
        check(GetPixel(dc, line.x, line.y - 1) != RGB(theme.border.r, theme.border.g, theme.border.b), "divider has top inset");
        SelectObject(dc, old); DeleteObject(bitmap); DeleteDC(dc); ReleaseDC(w.xid(), screen);
    }
    w.setTheme(Neu_Theme::MaterialDark());
    mouse(w, WM_MOUSEMOVE, e.x + 10, e.y + 12);
    capture(w, "hover-columns-circle");
    w.setNeutrinoMenuUseNormalCloseButton(true);
    check(w.neutrinoMenuUsesNormalCloseButton() && w.neutrinoMenuCloseBounds().width == 120, "boolean selects normal Close Menu button");
    render(w, true); capture(w, "hover-columns-normal");
    auto close = w.neutrinoMenuCloseBounds();
    mouse(w, WM_LBUTTONDOWN, close.x + 20, close.y + 15);
    mouse(w, WM_LBUTTONUP, 20, m.y + 100);
    check(w.neutrinoMenuOpen(), "drag off normal button cancels close");
    click(w, close.x + 20, close.y + 15);
    check(w.wideBarVisible(), "normal button closes menu and restores bar");
    w.openNeutrinoMenu();
    w.setTheme(Neu_Theme::MaterialLight());
    render(w, true); capture(w, "hover-columns-normal-light");
    w.setTheme(Neu_Theme::MaterialDark());
    close = w.neutrinoMenuCloseBounds();
    mouse(w, WM_LBUTTONDOWN, close.x + 20, close.y + 15);
    w.setNeutrinoMenuUseNormalCloseButton(false);
    mouse(w, WM_LBUTTONUP, close.x + 20, close.y + 15);
    check(w.neutrinoMenuOpen(), "switching button style cancels pending close");
    std::vector<std::string> many(55, "Item"); menu->setItems("File", many);
    menu->setSubmenuRowsPerColumn(0);
    const int rows = std::max(1, (m.height-20)/24);
    check(menu->submenuItemBounds(rows).x > menu->submenuItemBounds(0).x, "automatic row count wraps to next column");
    for (size_t i=0; i<many.size(); ++i) {
        const auto item = menu->submenuItemBounds(i);
        check(item.y >= m.y && item.y + item.height <= m.y + m.height && item.x + item.width <= m.x + m.width, "automatic grid stays inside menu");
    }
    SendMessageW(w.xid(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(320, 180));
    mouse(w, WM_MOUSEMOVE, 20, 80);
    check(menu->submenuItemBounds(4).x > menu->submenuItemBounds(0).x, "shorter window reflows columns on resize");
    SendMessageW(w.xid(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(800, 600));
    menu->setCategories({});
    check(menu->selectedCategoryIndex() == -1 && menu->submenuColumnDividers().empty(), "empty categories have no selection or dividers");
    w.closeNeutrinoMenu();
    menu->setCategories({"File", "Edit"}); menu->setItems("File", {"Open", "Save"}); menu->setSubmenuRowsPerColumn(0);
    menu->setCallbacks(Neu_Callbacks{});
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
        testHoverColumns(w, menu);
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
