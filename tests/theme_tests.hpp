#include "../examples/common/Neu_ThemeShowcase.hpp"
#include "../examples/common/Neu_LabelThemeGallery.hpp"

struct ThemeTestKeyboard {
    BYTE original[256]{};
    ThemeTestKeyboard() { check(GetKeyboardState(original)!=FALSE,"keyboard state saved for shortcut tests"); }
    ~ThemeTestKeyboard() { SetKeyboardState(original); }
    void modifiers(bool control,bool alt) {
        BYTE state[256]{};
        state[VK_CONTROL]=state[VK_LCONTROL]=control?0x80:0;
        state[VK_MENU]=state[VK_LMENU]=alt?0x80:0;
        check(SetKeyboardState(state)!=FALSE,"thread-local shortcut modifier state set");
    }
};
static bool sameThemeColor(const Neu_Color& a,const Neu_Color& b) { return a.r==b.r && a.g==b.g && a.b==b.b && a.a==b.a; }

static void testThemes(Neu_Application& app,Neu_Window& main,Neu_Window& secondary)
{
    const auto names=Neu_Theme::BuiltInThemeNames();
    check(names.size()==35,"six named themes added to built-in catalog");
    for(const auto& name:names) check(Neu_Theme::BuiltInThemeByName(name).name==name,"all built-in themes retain registry identity");
    const auto base=Neu_Theme::MaterialDark();
    const std::array<Neu_Theme,6> themes{{Neu_Theme::SupernovaRed1(),Neu_Theme::SupernovaRed2(),Neu_Theme::SupernovaRed3(),Neu_Theme::OffWhite1(),Neu_Theme::OffWhite2(),Neu_Theme::OffWhite3()}};
    const std::array<Neu_Color,6> highlights{{{195,0,51,255},{204,0,51,255},{255,30,81,255},{195,0,51,255},{204,0,51,255},{255,30,81,255}}};
    for(size_t i=0;i<themes.size();++i) {
        check(sameThemeColor(themes[i].highlight,highlights[i]),"requested HTML highlight color is exact");
        check(sameThemeColor(themes[i].accent,highlights[i]),"progress accent follows requested color");
        check(themes[i].topLeftCorner==base.topLeftCorner && themes[i].bottomRightCorner==base.bottomRightCorner && themes[i].edgeSize==base.edgeSize,"new themes preserve default six-edge shape");
        if(i<3) check(sameThemeColor(themes[i].glass,base.glass) && sameThemeColor(themes[i].background,base.background) && sameThemeColor(themes[i].border,base.border),"Supernova retains default grey surfaces");
        else check(sameThemeColor(themes[i].background,{242,240,239,255}) && sameThemeColor(themes[i].glass,{242,240,239,255}),"Off White uses requested off-white base");
    }
    check(Neu_Theme::BuiltInThemeByName("SUPERNOVA_RED_2").name=="supernova red 2","theme lookup accepts case and separator aliases");
    check(Neu_Theme::BuiltInThemeByName("Off White 3").name=="off-white-3","Off White theme alias lookup works");
    Neu_Window demo(app,1120,760,"Theme showcase regression tests"); check(demo.create(),"theme-only showcase window creates");
    neutrino_examples::ThemeShowcase showcase(demo); showcase.build();
    showcase.edit->setText("Keep this content");
    const std::array<const char*,6> captures{{"theme-supernova-red-1","theme-supernova-red-2","theme-supernova-red-3","theme-off-white-1","theme-off-white-2","theme-off-white-3"}};
    for(size_t i=0;i<themes.size();++i) {
        click(demo,44+static_cast<int>(i)*178,130);
        check(demo.themeName()==themes[i].name,"theme showcase button applies selected theme");
        check(showcase.current->text().find(themes[i].name)!=std::string::npos,"theme-changed callback updates theme label");
        check(showcase.edit->text()=="Keep this content" && std::abs(showcase.square->progress()-0.65f)<0.001f,"theme changes retain editable content and progress");
        mouse(demo,WM_MOUSEMOVE,400,570);
        ProgressTestSurface frame(1120,760); demo.paint(frame.dc);
        check(GetPixel(frame.dc,834,242)==RGB(highlights[i].r,highlights[i].g,highlights[i].b),"showcase progress renders exact selected theme accent");
        check(GetPixel(frame.dc,1090,80)==RGB(demo.theme().background.r,demo.theme().background.g,demo.theme().background.b),"transparent labels leave theme background readable");
        check(showcase.selector->selectedIndex()>=0 && showcase.selector->items()[static_cast<size_t>(showcase.selector->selectedIndex())]==themes[i].name,"showcase selector tracks active theme");
        capture(demo,captures[i]);
    }
    showcase.setProgress(25); check(std::abs(showcase.bar->progress()-0.25f)<0.001f && showcase.spinner->value()==25,"showcase adjustable progress samples synchronize");
    demo.setTheme(Neu_Theme::MaterialDark());
    const auto mainName=main.themeName(), secondaryName=secondary.themeName();
    ThemeTestKeyboard keyboard;
    keyboard.modifiers(true,true);
    click(demo,70,250); // Focus the text input; window shortcut must take priority.
    SendMessageW(demo.xid(),WM_SYSKEYDOWN,VK_RIGHT,0);
    check(demo.themeName()==names[1],"Ctrl+Alt+Right system-key message cycles forward from focused editor");
    check(showcase.current->text().find(names[1])!=std::string::npos,"shortcut updates showcase current-theme label");
    SendMessageW(demo.xid(),WM_KEYDOWN,VK_LEFT,0);
    check(demo.themeName()==names.front(),"Ctrl+Alt+Left cycles backwards");
    SendMessageW(demo.xid(),WM_SYSKEYDOWN,VK_LEFT,0);
    check(demo.themeName()==names.back(),"backwards shortcut wraps to final theme");
    SendMessageW(demo.xid(),WM_SYSKEYDOWN,VK_RIGHT,0);
    check(demo.themeName()==names.front(),"forward shortcut wraps to first theme");
    SendMessageW(demo.xid(),WM_SYSKEYDOWN,VK_RIGHT,static_cast<LPARAM>(1)<<30);
    check(demo.themeName()==names.front(),"holding shortcut key does not repeatedly cycle");
    demo.setTheme(Neu_Theme::SupernovaRed1()); SendMessageW(demo.xid(),WM_SYSKEYDOWN,VK_RIGHT,0);
    check(demo.themeName()=="supernova red 2","named theme identities determine cycling order");
    check(main.themeName()==mainName && secondary.themeName()==secondaryName,"cycling affects only the targeted active window");
    const auto before=demo.themeName();
    keyboard.modifiers(true,false); SendMessageW(demo.xid(),WM_KEYDOWN,VK_RIGHT,0);
    check(demo.themeName()==before,"Control without Alt does not cycle");
    keyboard.modifiers(false,true); SendMessageW(demo.xid(),WM_SYSKEYDOWN,VK_RIGHT,0);
    check(demo.themeName()==before,"Alt without Control does not cycle");
    keyboard.modifiers(false,false); SendMessageW(demo.xid(),WM_KEYDOWN,VK_RIGHT,0);
    check(demo.themeName()==before,"ordinary cursor key does not cycle");
    keyboard.modifiers(true,true); demo.setThemeCyclingShortcutsEnabled(false);
    SendMessageW(demo.xid(),WM_KEYDOWN,VK_RIGHT,0);
    check(demo.themeName()==before,"per-window option disables theme shortcut"); demo.setThemeCyclingShortcutsEnabled(true);
    for(size_t i=0;i<names.size();++i) demo.cycleTheme();
    check(demo.themeName()==before,"complete forward traversal wraps to same named theme");
    for(size_t i=0;i<names.size();++i) demo.cycleTheme(-1);
    check(demo.themeName()==before,"complete backward traversal wraps to same named theme");
    auto custom=Neu_Theme::MaterialDark(); custom.name="Custom"; demo.setTheme(custom); demo.cycleTheme();
    check(demo.themeName()==names.front(),"custom theme enters catalog at first theme when cycling forwards");
    auto menu=std::make_shared<Neu_PopWindowMenu>(); check(main.setNeutrinoMenu(menu),"main window menu attaches for theme shortcut test");
    main.openNeutrinoMenu(); SendMessageW(main.xid(),WM_SYSKEYDOWN,VK_RIGHT,0);
    check(main.neutrinoMenuOpen() && main.themeName()==names[1],"shortcut also works while menu overlays the window");
    main.closeNeutrinoMenu(); main.setNeutrinoMenu(nullptr); main.setTheme(Neu_Theme::MaterialDark());
    demo.close();
    keyboard.modifiers(false,false);
    Neu_Window galleryWindow(app,1180,760,"Label theme gallery regression");
    check(galleryWindow.create(),"label-theme gallery creates");
    neutrino_examples::LabelThemeGallery gallery; gallery.build(galleryWindow);
    check(gallery.choices.size()==names.size(),"label gallery includes all built-in and newly added themes");
    const auto status=gallery.status->text();
    for(size_t i=0;i<names.size();++i) {
        const int x=44+static_cast<int>(i%6)*170, y=94+static_cast<int>(i/6)*42;
        mouse(galleryWindow,WM_MOUSEMOVE,x,y);
        check(gallery.status->text()==status,"gallery hover does not misinterpret theme-choice payload as textbox");
        click(galleryWindow,x,y);
        check(galleryWindow.themeName()==names[i],"label gallery theme click remains responsive and selects correct theme");
    }
    ProgressTestSurface galleryFrame(1180,760); galleryWindow.paint(galleryFrame.dc);
    capture(galleryWindow,"label-theme-gallery");
    SendMessageW(galleryWindow.xid(),WM_CLOSE,0,0);
    check(!galleryWindow.xid(),"label gallery closes normally after all hover/click transitions");
}
