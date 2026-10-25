# Neutrino

Independent Windows 10 and later package, split from
`Neutrino_Stage2_23_Tabbar_Positions_Fixed.zip`.

The original design, controls, themes, behavior, namespace `neutrino`, public
`Neu_` API, include path `<Neutrino/Neutrino.hpp>` and static-library name
`Neutrino` are preserved. Only platform branch selection and build/package files
change. This package contains its own headers, implementation, assets and demos;
it has no dependency on the other package. Keep the two installations separate.
`SOURCE_PROVENANCE.json` records hashes for auditing platform branch selection.
The original proprietary notice is retained. Original notes in `docs/history`
are historical and may describe the combined package; follow BUILDING.md here.

See [BUILDING.md](BUILDING.md) for build and installation commands.
# Main-window Neutrino Menu bar

Six new Supernova/Off White themes, built-in Ctrl+Alt+Left/Right theme cycling,
and a theme-only showcase are described in [the theme guide](docs/THEMES_AND_SHORTCUTS.md).

The square progress control now matches the themed button, with a doubled
10-pixel perimeter line and exact 6-pixel inner gap. See [the progress guide](docs/PROGRESS_SQUARE.md).

See [the wide-bar guide](docs/WIDE_BAR.md) for the optional main-window bar,
menu overlay, red circular X, new examples 23/24, and integration-test commands.

