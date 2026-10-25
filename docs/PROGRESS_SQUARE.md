# Themed square progress control

`Neu_ProgressSquare` now draws its face using the same themed control renderer
as `Neu_Button`. The default theme's opposing chamfered corners form a six-edge
outline; custom rounded or all-chamfered themes use their button outline too.
Gradient, border, hover appearance, icon support, and focus rendering remain
consistent with the button. Percentage text stays centered inside the progress
line. The control follows its complete bounds, including rectangular layouts.

The perimeter line is now **10 pixels thick**, twice its former 5-pixel thickness.
Its outer edge leaves **6 clear pixels after the one-pixel button border**.
Parallel offsets maintain that separation along the slanted edges as well as
the horizontal and vertical sides. This measures the gap from the outside edge
of the progress line, rather than from its center. Native GDI pixel coordinates
are accounted for on both sides of the control.

The line starts at the top center and advances clockwise by perimeter length.
It uses the theme's accent color through completion. At 100% the entire inset
band is filled without a seam. At 0% only the button face and percentage remain.
Percentage text is clipped inside the line so it cannot overwrite the band.
If a control is too small to accommodate the exact gap and thickness, the band
is omitted; it is never squeezed into an incorrectly spaced miniature version.

The existing API is unchanged:

```cpp
auto progress = std::make_shared<Neu_ProgressSquare>(Neu_Layout{20, 20, 160, 160});
progress->setProgress(0.65f);
window.add(progress);
```

Example 10 (`neutrino_test_10_images_progress_labels`) includes a matching-size
button below the progress control for visual comparison. The progress stepping
button still cycles through values. `validation/progress-themed-dark.png` and
`validation/progress-themed-light.png` show a button alongside 0%, 65%, and 100%.

The existing native regression suite now also checks button-face equality,
six clear gap pixels and ten band pixels on every straight side, normal-distance
constraints at the diagonal sides, clockwise and monotonic filling, theme accent
colors, rounded/all-chamfered themes, rectangular/tiny bounds, clamping and NaN,
and GDI resource counts across repaint cycles. Run it with the existing CTest or
NMake `test` commands. See `WIDE_BAR_VERIFICATION.md` for current results.
