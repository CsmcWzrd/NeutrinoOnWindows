#include "common/Neu_LabelThemeGallery.hpp"

int main(int argc, char** argv)
{
    neutrino::Neu_Application app;
    if (!neutrino_tests::start_app(app,argc,argv)) return 1;
    neutrino::Neu_Window win(app,1180,760,"Neutrino Test 14 - Label Offsets and Themes");
    neutrino_tests::apply_test_window_defaults(win);
    if (!win.create()) return 1;
    neutrino_examples::LabelThemeGallery gallery;
    gallery.build(win);
    win.show();
    app.run();
    return 0;
}
