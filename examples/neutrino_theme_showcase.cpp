#include "common/Neu_ThemeShowcase.hpp"

int main(int argc, char** argv)
{
    neutrino::Neu_Application app;
    if(!app.open()) return 1;
    neutrino::Neu_Window window(app,1120,760,"Neutrino Theme Showcase");
    if(!window.create()) return 1;
    neutrino_examples::ThemeShowcase showcase(window);
    showcase.build();
    if(argc>1) window.setTheme(neutrino::Neu_Theme::BuiltInThemeByName(argv[1]));
    window.show();
    app.run();
    return 0;
}
