# Main-window wide bar and Neutrino Menu

The Windows framework now supports an optional, full-client-width Neutrino Menu
bar on the application's main window. The first successfully created/registered
window is the main window. Existing secondary windows are never automatically
promoted when it closes.

The closed bar is exactly three times the menu font's pixel height: 48 pixels
with the current 16-pixel Segoe UI font. Its horizontal width follows the client
area. Toolbars, status bars, and native menubar metrics do not enlarge it.
`setWideBarReferenceHeight()` remains available for source compatibility but
no longer changes the height. This replaces the earlier conventional-bar minimum.

Click and release inside the bar to open the existing category/item menu above
the workspace. The bar is entirely absent while open. The menu's top strip has
a red circular button with a white X; click it or press Escape to close. The bar
returns to exactly the same location. Opening and closing do not change control
positions, text, selection, or scroll state in the workspace.

The menu covers the client area, intercepting mouse, wheel, and keyboard input.
The previous workspace focus is restored when it closes. Menu controls can use
normal callbacks, including commands that change the theme or close/remove the
menu. Opening dismisses an active workspace combo dropdown. The window frame
and other application windows remain usable.

```cpp
Neu_Window main(app, 960, 640, "My application");
main.create();
auto menu = std::make_shared<Neu_PopWindowMenu>();
menu->setCategories({"File", "Help"});
menu->setItems("File", {"Open", "Save"});
if (!main.setNeutrinoMenu(menu)) { /* handle invalid ownership/window role */ }
```

Attach after `create()`. The window owns the menu separately from its normal
controls: do not also call `add(menu)`. Attaching to a secondary window, attaching
another window's menu, or attaching a menu already added as a normal control
returns false. Passing `nullptr` removes both menu and bar. Existing applications
remain unchanged until they attach a menu.

`openNeutrinoMenu()`, `closeNeutrinoMenu()`, `neutrinoMenuOpen()`,
`wideBarVisible()`, `wideBarHeight()`, and `neutrinoMenuCloseBounds()` provide
programmatic control and geometry. Existing `menu->show()` / `hide()` also work;
the window synchronizes focus on the next paint or input event. The window owns
the attached menu's layout. Children use the framework's existing absolute
client coordinates; applications remain responsible for positioning their menu
children on resize. Existing category/item rendering and callbacks are retained.

Examples:

- `neutrino_test_23_wide_bar`: editable workspace, categories, resize, red X.
- `neutrino_test_24_main_window_menu`: 44-pixel toolbar, 48-pixel bar,
  secondary-window rejection, and a menu command that applies the light theme.

Build with the existing CMake, NMake, MinGW, or VS2022 solution instructions.
The new Visual Studio projects support Debug/Release with Win32, x64, and ARM64.

Run automated native integration tests:

```text
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Alternatively use `nmake /f Makefile.msvc test` or
`mingw32-make -f Makefile.mingw test`. The tests create native windows and use the
actual window message and rendering paths. Optional `NEUTRINO_TEST_CAPTURE_DIR`
points to an existing directory for three BMP render artifacts.

`SOURCE_PROVENANCE.json` and the original `VERIFICATION.md` describe the imported
split baseline. They are historical records; source checksums change with this
feature. See `WIDE_BAR_VERIFICATION.md` for this change's test results and limits.
