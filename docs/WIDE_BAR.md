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
a 16-pixel red circular button with a white X (40% of its original 40-pixel
diameter); click it or press Escape to close. The bar
returns to exactly the same location. Opening and closing do not change control
positions, text, selection, or scroll state in the workspace.

The menu covers the client area, intercepting mouse, wheel, and keyboard input.
The previous workspace focus is restored when it closes. Menu controls can use
normal callbacks, including commands that change the theme or close/remove the
menu. Opening dismisses an active workspace combo dropdown. The window frame
and other application windows remain usable.

Hover over a category to display its submenu. Category and submenu-item
highlights follow the pointer and clear when it leaves; clicking a category does
not switch it. Commands still activate by clicking, never merely by hovering.
The last displayed category remains open while the pointer enters its submenu.

Submenu items fill vertically, then continue in columns from left to right.
By default each column fits as many 24-pixel rows as the available menu height
allows. `menu->setSubmenuRowsPerColumn(5)` optionally limits columns to five
rows; `0` restores automatic sizing. A shorter window reduces that limit to fit.
The columns share the available width, and long labels are truncated. Between
submenu columns, one-pixel vertical lines use the active theme's border color.
They start at 10% of the menu height and end at 90%, with a maximum length of 80%.

Choose the close-button style with a boolean:

```cpp
main.setNeutrinoMenuUseNormalCloseButton(true);  // themed "Close Menu" button
main.setNeutrinoMenuUseNormalCloseButton(false); // small red circle (default)
```

This setting can change while the menu is open. The normal button is a standard
`Neu_Button` using the window's current theme, including its hover styling.
`neutrinoMenuUsesNormalCloseButton()` reports the current boolean value.

```cpp
Neu_Window main(app, 960, 640, "My application");
main.create();
auto menu = std::make_shared<Neu_PopWindowMenu>();
menu->setCategories({"File", "Help"});
menu->setItems("File", {"Open", "Save"});
menu->setSubmenuRowsPerColumn(5); // optional; 0 uses available height
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
children on resize. Menu children are drawn above the generated submenu rows.
`selectedCategoryIndex()`, `highlightedCategoryIndex()`, and
`highlightedItemIndex()` expose navigation state (highlight indices are -1 when
none). `submenuItemBounds(index)` and `submenuColumnDividers()` expose the same
geometry used for drawing and hover hit testing.

Examples:

- `neutrino_test_23_wide_bar`: editable workspace, hover navigation, a three-column
  File submenu, theme-colored dividers, and the small red circular X.
- `neutrino_test_24_main_window_menu`: 44-pixel toolbar, 48-pixel bar,
  secondary-window rejection, three-column submenu, themed "Close Menu" button,
  and a menu command that applies the light theme.

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
points to an existing directory for fifteen BMP render artifacts, including both
close styles, themed progress controls, the six new themes, and the label gallery.

`SOURCE_PROVENANCE.json` and the original `VERIFICATION.md` describe the imported
split baseline. They are historical records; source checksums change with this
feature. See `WIDE_BAR_VERIFICATION.md` for this change's test results and limits.
