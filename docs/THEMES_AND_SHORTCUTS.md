# Supernova, Off White, and window theme cycling

The six new built-in themes use these exact highlight, accent, hover, and focus
colors. Supernova keeps the default MaterialDark grey surfaces, border, gradients,
and six-edge shape. Off White uses an `#F2F0EF` background and control face, dark
text, the default grey border, and subtle off-white gradients.

| Theme name | C++ factory | Highlight/accent |
|---|---|---|
| supernova red 1 | `Neu_Theme::SupernovaRed1()` | `#C30033` |
| supernova red 2 | `Neu_Theme::SupernovaRed2()` | `#CC0033` |
| supernova red 3 | `Neu_Theme::SupernovaRed3()` | `#FF1E51` |
| off-white-1 | `Neu_Theme::OffWhite1()` | `#C30033` |
| off-white-2 | `Neu_Theme::OffWhite2()` | `#CC0033` |
| off-white-3 | `Neu_Theme::OffWhite3()` | `#FF1E51` |

The catalog now contains 35 themes. `BuiltInThemeByName()` accepts the names
above and ignores case, spaces, hyphens, and underscores in lookups. Each
built-in factory sets `theme.name` so direct factory calls and registry lookups
retain their identity. Existing theme palettes are unchanged.

Every `Neu_Window` supports these shortcuts by default:

- **Ctrl+Alt+Right**: next built-in theme.
- **Ctrl+Alt+Left**: previous built-in theme.

The order is `Neu_Theme::BuiltInThemeNames()`, with wrapping at either end.
Only the window receiving the shortcut changes. This works while an editor or
other control has focus and while the Neutrino Menu overlays the window.
Holding the arrow does not race through themes: repeat messages are consumed
without another cycle. Other arrow combinations retain their existing behavior,
and unrelated Alt/system shortcuts continue through the normal Windows handler.

The feature belongs to the framework; applications do not need their own key
handlers. Applications may disable it per window or cycle programmatically:

```cpp
window.setThemeCyclingShortcutsEnabled(false); // defaults to true
window.cycleTheme();                           // next
window.cycleTheme(-1);                         // previous
auto currentName = window.themeName();
```

Unknown/custom theme names enter the catalog at its first theme when cycling
forward or its last theme when cycling backward. Modified built-in themes retain
their named position unless the application changes the name. The optional
`setOnThemeChanged(callback, userData)` callback runs after `setTheme()`, including
shortcut-driven changes, so applications can refresh their theme indicator.

## Theme-only showcase

Build/run `neutrino_theme_showcase` or the Visual Studio project
`NeutrinoThemeShowcase`. It includes six direct theme buttons, a dropdown for all
35 themes, previous/next controls, a current-theme/color indicator, editable and
password inputs, checkbox, radio, toggle, list, multiline editor, slider, spinner,
normal/disabled buttons, two square progress controls, and horizontal progress
bars. The slider/spinner update the adjustable progress samples. Theme changes
preserve control contents and values. An optional first command-line argument
selects the starting built-in theme by name.

The new target is included in CMake, NMake, MinGW, and the VS2022-compatible
solution for Debug/Release with Win32/x64/ARM64 configurations.

## Repaired label/theme gallery

`NeutrinoTest14LabelOffsetsThemes` had inherited focus/blur callbacks that treated
their `userData` as a status textbox, while its theme-click handler instead stored
a pointer to a `(window, theme-index)` pair in that field. Moving the pointer over
a theme button could corrupt memory and cause an apparent hang followed by an
access-violation crash. Those incompatible hover callbacks have been removed;
the ordinary control hover appearance remains active.

The gallery now includes all 35 themes, positions its label samples below the
expanded button grid, and uses the correct Test 14 window title. Its exact builder
is shared with the regression suite, which hovers and clicks every theme button,
checks the selected theme and status integrity, repaints, and closes the window.

Transparent labels also no longer receive an opaque filled shadow: this keeps
dark label text readable on light themes. The showcase selector follows the
currently active theme when shortcuts or buttons change it.

See `WIDE_BAR_VERIFICATION.md` for current native test results and limits.
