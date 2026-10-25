#include "Neutrino/Neutrino.hpp"
using namespace neutrino;

static void changeTheme(Neu_Control* sender, void*)
{
    sender->parent()->setTheme(Neu_Theme::MaterialLight());
    sender->parent()->closeNeutrinoMenu();
}

int main()
{
    Neu_Application app;
    if (!app.open()) return 1;
    Neu_Window main(app, 1000, 680, "Neutrino - Main Window Menu with Toolbar");
    Neu_Window secondary(app, 520, 320, "Neutrino - Secondary Window (no wide bar)");
    if (!main.create() || !secondary.create()) return 1;
    auto toolbar = std::make_shared<Neu_ToolBar>(Neu_Layout{16, 104, 800, 44});
    main.add(toolbar);
    auto menu = std::make_shared<Neu_PopWindowMenu>();
    menu->setCategories({"Workspace", "Appearance"});
    menu->setItems("Workspace", {"Main-window-only overlay", "Secondary windows retain their normal content"});
    menu->setItems("Appearance", {"Use the button below to apply the light theme"});
    auto action = std::make_shared<Neu_MenuItem>(Neu_Layout{220, 240, 300, 44});
    action->setText("Apply light theme and close menu");
    Neu_Callbacks cb;
    cb.onClick = changeTheme;
    action->setCallbacks(cb);
    menu->add(action);
    if (!main.setNeutrinoMenu(menu)) return 1;
    // Deliberately demonstrate rejection on a secondary window.
    if (secondary.setNeutrinoMenu(std::make_shared<Neu_PopWindowMenu>())) return 2;
    auto note = std::make_shared<Neu_Label>(Neu_Layout{20, 30, 470, 40});
    note->setText("Only the main application window has the wide bar.");
    secondary.add(note);
    auto editor = std::make_shared<Neu_Multilinetextbox>(Neu_Layout{16, 170, 800, 380});
    editor->setText("Workspace content is preserved while Neutrino Menu is open.\nTry the Appearance menu button, resizing, and Escape.");
    main.add(editor);
    main.show();
    secondary.show();
    app.run();
    return 0;
}
