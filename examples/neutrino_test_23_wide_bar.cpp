#include "Neutrino/Neutrino.hpp"
using namespace neutrino;

int main()
{
    Neu_Application app;
    if (!app.open()) return 1;
    Neu_Window win(app, 960, 640, "Neutrino - Wide Bar and Menu Overlay");
    if (!win.create()) return 1;
    auto menu = std::make_shared<Neu_PopWindowMenu>();
    menu->setCategories({"File", "Edit", "View", "Help"});
    menu->setItems("File", {"New document", "Open document", "Save document"});
    menu->setItems("Edit", {"Undo", "Redo", "Copy", "Paste"});
    menu->setItems("View", {"Workspace", "Preview", "Properties"});
    menu->setItems("Help", {"Close with the red X or Escape", "The workspace stays in place underneath"});
    if (!win.setNeutrinoMenu(menu)) return 1;
    auto text = std::make_shared<Neu_Textbox>(Neu_Layout{24, 110, 620, 42});
    text->setText("Edit this text, then open and close the Neutrino Menu.");
    win.add(text);
    auto label = std::make_shared<Neu_MultilineLabel>(Neu_Layout{24, 180, 820, 180});
    label->setText("Click the full-width bar above to open Neutrino Menu.\nThe bar disappears entirely while the menu is open.\nThe red circular X restores it without moving or resetting this workspace.\nResize the window with the menu open to see it follow the client area.");
    win.add(label);
    win.show();
    app.run();
    return 0;
}
