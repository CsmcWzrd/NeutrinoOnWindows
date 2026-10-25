# Wide-bar verification — 26 October 2026

Source baseline: the user-provided `Neutrino.zip`, originally produced in
“Separate Neutrino Projects”. Only the Windows package was modified.

## Implemented behavior

- Optional full-width main-window bar, exactly three times the menu font pixel
  height (48 pixels for the current 16-pixel font). Other bar heights no longer
  enlarge it; the old reference-height setter is retained as a compatibility no-op.
- Bar click opens the existing category/item menu as a full-client overlay.
  Workspace controls retain their positions and state underneath.
- Open menu replaces the bar entirely. The default close button is a 16-pixel
  red circle with a white X, 40% of its original 40-pixel diameter. A boolean
  selects a normal themed Close Menu button instead. Escape also closes it.
- Categories switch and highlight on hover. Submenu items highlight on hover;
  commands still require a click. Items fill down columns then left to right.
  Inter-column dividers use the theme border color, starting at 10% of menu
  height and ending by 90%, no longer than 80% of its height.
- Underlying input is blocked while open, previous focus is restored on close,
  and drag cancellation/capture loss do not create accidental activations.
- Secondary-window attachment is rejected. Existing examples remain opt-in.

## Native Windows results

The current hover/columns/close-style revision was built and tested with CMake
x64 Debug/Release and the supplied Visual Studio solution Win32 Release. The
VS solution x64 Debug and NMake rows below are historical results from the
original feature revision, not reruns of this revision.

Host: Windows x64, reported version 10.0.26300. Windows SDK 10.0.26100.0,
MSVC v143 (19.44.35229), hosted by Visual Studio Community 2026.

| Build/test path | Result |
|---|---|
| CMake, x64 Debug | Library, all 32 examples, integration test built; CTest passed |
| CMake, x64 Release | Library, all 32 examples, integration test built; CTest passed |
| Supplied VS solution, x64 Debug (historical) | Original feature revision passed 97 checks |
| Supplied VS solution, Win32 Release | Current library, both updated examples, test built; 189 checks passed |
| NMake, x64 Debug (historical) | Original feature revision built all 32 examples and passed 97 checks |

The standalone CMake Debug/Release test executables also passed. The normal
suite performs 189 checks; enabling the six optional render captures adds six
file-write checks, yielding 195. Assertions remain active in Release builds.

Coverage includes native HWND/message dispatch; main-window ownership and
attachment errors; exact font-relative sizing, unaffected by native or custom bars; normal workspace
clicks with the feature attached; open/close states; click, keyboard, and wheel
isolation; focus restoration; menu-child callbacks; circular hit testing and
drag-off cancellation; unpaired button releases; Escape; direct show/hide;
resizing; tiny client areas; dark/light themes; buffering enabled/disabled;
200 open/close cycles; capture cancellation/loss; 20 repaint/open/close cycles
with GDI resource-count checks; safe menu replacement/removal, including removal
from a command's own callback; and main-window shutdown without secondary-window
promotion. New coverage verifies hover-only category switching and highlighting;
submenu highlighting without activating commands; pointer-leave cleanup; disabled
and empty menus; top-to-bottom/left-to-right ordering; automatic and configured
row limits; resize reflow; divider geometry and actual dark/light theme pixels;
16-pixel circle dimensions; both close-button styles, normal-button text rendering,
drag-off cancellation, live style changes, and the light theme. Drawing checks
verify the red circle and white X pixels.

Rendered previews in `validation/` were visually inspected: `wide-bar.png`,
`menu-overlay.png`, `bar-restored.png`, `hover-columns-circle.png`,
`hover-columns-normal.png`, and `hover-columns-normal-light.png`. They come from the actual GDI paint
path, rather than mockups. Build logs are included alongside these previews.

## Limits and existing warnings

- This is native testing on the installed Windows host, not testing across every
  Windows 10/11 edition, DPI setting, or display configuration.
- ARM64 project configurations and MinGW rules include the new targets, but were
  not compiled or executed in this run. x86 binaries were run on the x64 host.
- All existing examples were compiled; this report does not claim exhaustive
  interaction testing of every pre-existing control or manual launch of each demo.
- The original Win32 demo defines both `WinMain` and `wWinMain`; its existing
  ambiguous-entry-point linker warning remains. The new targets build cleanly.
- Existing callback-based command activation is retained; category navigation
  and highlighting now use hover. This is the existing two-level category/item
  menu model, not a new arbitrary-depth nested-menu model.
- Historical `VERIFICATION.md` / `SOURCE_PROVENANCE.json` refer to the imported
  baseline. `WIDE_BAR_SOURCE_CHANGES.json` records the modified/new package files.
