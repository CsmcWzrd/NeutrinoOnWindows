#pragma once
#include "Neu_TestCommon.hpp"

namespace neutrino_examples {
struct LabelThemeGallery {
    std::vector<std::pair<neutrino::Neu_Window*,int>> choices;
    std::shared_ptr<neutrino::Neu_Textbox> status;
    void build(neutrino::Neu_Window& win) {
        using namespace neutrino;
        using namespace neutrino_tests;
    status = add_status(win, "Stage2: label text offsets/insets, optional borders, wrapping/truncation, and theme gallery.");

    const auto names = Neu_Theme::BuiltInThemeNames();
    choices.clear();
    choices.reserve(std::max<size_t>(names.size(), 32));
    int x = 24;
    int y = 78;
    int index = 0;
    for (const auto& name : names) {
        auto swatch = std::make_shared<Neu_Button>(Neu_Layout{x, y, 160, 34});
        swatch->setText(name);
        swatch->setTextTruncation(true);
        swatch->setHintText("Click to apply this built-in Neutrino theme. Stage2 includes Windows-inspired themes such as win95, winxp, win10 and win11.");
        const int slot = index;
        Neu_Callbacks cb; // Hover callbacks must not interpret the choice payload as a textbox.
        cb.onClick = [](Neu_Control* sender, void* user_data) {
            auto* pair = static_cast<std::pair<Neu_Window*, int>*>(user_data);
            if (pair && pair->first) {
                const auto names = Neu_Theme::BuiltInThemeNames();
                pair->first->setTheme(Neu_Theme::BuiltInThemeByName(names[static_cast<size_t>(pair->second)]));
                pair->first->requestRedraw();
            } else if (sender) {
                sender->requestRedraw();
            }
        };
        choices.emplace_back(&win, slot);
        cb.userData = &choices.back();
        swatch->setCallbacks(cb);
        win.add(swatch);
        x += 170;
        if (x > 980) {
            x = 24;
            y += 42;
        }
        ++index;
    }

    const int labelsTop = 78 + static_cast<int>((names.size()+5)/6)*42 + 24;
    auto offsetLabel = std::make_shared<Neu_Label>(Neu_Layout{24, labelsTop, 520, 58});
    offsetLabel->setText("Offset label: top=10 right=20 bottom=6 left=30; text must not protrude outside left edge.");
    offsetLabel->setBorderVisible(true);
    offsetLabel->setTextOffset(10, 20, 6, 30);
    offsetLabel->setTextTruncation(true);
    offsetLabel->setHintText("This test is for Windows and Linux clipping. The text begins after the configured left offset.");
    win.add(offsetLabel);

    auto wrappedLabel = std::make_shared<Neu_Label>(Neu_Layout{570, labelsTop, 560, 88});
    wrappedLabel->setText("Word wrapped label: this intentionally long label should wrap inside the rectangular boundary and should never draw outside the control.");
    wrappedLabel->setBorderVisible(true);
    wrappedLabel->setWordWrap(true);
    wrappedLabel->setTextOffset(8, 12, 8, 14);
    win.add(wrappedLabel);

    auto clippedLabel = std::make_shared<Neu_Label>(Neu_Layout{24, labelsTop+110, 520, 42});
    clippedLabel->setText("Truncated label with a very long left-to-right sentence that must be clipped to the label width.");
    clippedLabel->setBorderVisible(true);
    clippedLabel->setWordWrap(false);
    clippedLabel->setTextTruncation(true);
    clippedLabel->setTextOffset(6, 12, 6, 12);
    win.add(clippedLabel);

    auto richLabel = std::make_shared<Neu_Label>(Neu_Layout{570, labelsTop+110, 560, 58});
    richLabel->setBorderVisible(true);
    richLabel->setTextOffset(6, 12, 6, 12);
    Neu_TextFragment a; a.text = "Bold "; a.bold = true;
    Neu_TextFragment b; b.text = "Italic "; b.italic = true; b.useFontColor = true; b.fontColor = {180, 60, 60, 255};
    Neu_TextFragment c; c.text = "Underline "; c.underline = true; c.useHighlightColor = true; c.highlightColor = {255, 240, 120, 180};
    Neu_TextFragment d; d.text = "Strike"; d.strikethrough = true;
    richLabel->addRichTextFragment(a);
    richLabel->addRichTextFragment(b);
    richLabel->addRichTextFragment(c);
    richLabel->addRichTextFragment(d);
    richLabel->setHintText("Minimum rich label formatting: bold, italic, underline, strike, font/highlight color.");
    win.add(richLabel);

    auto multi = std::make_shared<Neu_MultilineLabel>(Neu_Layout{24, labelsTop+190, 1106, 178});
    multi->setText("Multiline label wraps by default.\nThis long second line is meant to verify that word wrapping keeps text inside the control on both Windows GDI and Linux X11/Xft paths.\nBorder is enabled here only through a flag; labels remain borderless by default.");
    multi->setBorderVisible(true);
    multi->setWordWrap(true);
    multi->setTextOffset(10, 16, 10, 16);
    multi->setIconBmp("assets/icons/menu_icon.bmp");
    win.add(multi);

    }
};
}
