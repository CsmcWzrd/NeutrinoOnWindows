#pragma once
#include "Neutrino/Neutrino.hpp"
#include <array>
#include <iomanip>

namespace neutrino_examples {
using namespace neutrino;

class ThemeShowcaseSelector : public Neu_ComboBox {
public:
    using Neu_ComboBox::Neu_ComboBox;
    void selectTheme(const std::string& name) {
        auto item=std::find(items_.begin(),items_.end(),name);
        selected_=item==items_.end()?-1:static_cast<int>(item-items_.begin());
        selectedIndices_.clear(); if(selected_>=0) selectedIndices_.insert(selected_);
        setText(name);
    }
};

struct ThemeShowcase {
    struct Choice { ThemeShowcase* showcase{}; std::string name; };
    Neu_Window& window;
    std::array<Choice,6> choices;
    std::shared_ptr<Neu_Label> current;
    std::shared_ptr<Neu_Textbox> edit;
    std::shared_ptr<ThemeShowcaseSelector> selector;
    std::shared_ptr<Neu_Slider> slider;
    std::shared_ptr<Neu_Spinner> spinner;
    std::shared_ptr<Neu_ProgressSquare> square;
    std::shared_ptr<Neu_ProgressBar> bar;

    explicit ThemeShowcase(Neu_Window& win) : window(win) {}
    ~ThemeShowcase() { window.setOnThemeChanged(nullptr,nullptr); }

    template<class T> std::shared_ptr<T> add(Neu_Layout layout, const std::string& text={}) {
        auto control=std::make_shared<T>(layout); control->setText(text); window.add(control); return control;
    }
    static void changed(Neu_Window* win, void* data) {
        auto& self=*static_cast<ThemeShowcase*>(data);
        const auto c=win->theme().highlight;
        std::ostringstream label;
        label << "Current theme: " << win->themeName() << "     Highlight: #" << std::hex << std::uppercase << std::setfill('0')
              << std::setw(2) << int(c.r) << std::setw(2) << int(c.g) << std::setw(2) << int(c.b);
        self.current->setText(label.str());
        self.selector->selectTheme(win->themeName());
    }
    void setProgress(int value) {
        square->setProgress(value/100.0f); bar->setProgress(value/100.0f);
        bar->setText("Progress " + std::to_string(value) + "%");
        slider->setValue(value); spinner->setValue(value);
    }
    void build() {
        add<Neu_Label>({24,16,1072,36},"Neutrino Theme Showcase");
        current=add<Neu_Label>({24,60,1072,32});
        const std::array<const char*,6> names{{"supernova red 1","supernova red 2","supernova red 3","off-white-1","off-white-2","off-white-3"}};
        for(size_t i=0;i<names.size();++i) {
            choices[i]={this,names[i]};
            auto b=add<Neu_Button>({24+static_cast<int>(i)*178,110,170,40},names[i]);
            Neu_Callbacks cb; cb.userData=&choices[i];
            cb.onClick=[](Neu_Control*,void* data) { auto& c=*static_cast<Choice*>(data); c.showcase->window.setTheme(Neu_Theme::BuiltInThemeByName(c.name)); };
            b->setCallbacks(cb);
        }
        auto previous=add<Neu_Button>({368,168,166,36},"Previous theme");
        auto next=add<Neu_Button>({550,168,166,36},"Next theme");
        Neu_Callbacks cycle; cycle.userData=this;
        cycle.onClick=[](Neu_Control*,void* data) { static_cast<ThemeShowcase*>(data)->window.cycleTheme(-1); }; previous->setCallbacks(cycle);
        cycle.onClick=[](Neu_Control*,void* data) { static_cast<ThemeShowcase*>(data)->window.cycleTheme(1); }; next->setCallbacks(cycle);
        add<Neu_Label>({744,168,352,36},"Ctrl+Alt+Left / Right also cycle themes");
        edit=add<Neu_Textbox>({24,235,320,38},"Editable text: type here");
        add<Neu_Passwordbox>({24,285,320,38},"password");
        auto check=add<Neu_CheckBox>({24,340,154,38},"Checked"); check->setChecked(true);
        auto radio=add<Neu_RadioButton>({194,340,150,38},"Radio"); radio->setChecked(true);
        auto toggle=add<Neu_ToggleButton>({24,390,320,40},"Toggle sample"); toggle->setChecked(true);
        auto list=add<Neu_Listbox>({24,460,320,190}); list->setItems({"Buttons and inputs","Hover and focus colors","Selected list items","Checkboxes and toggles","Progress controls"});
        add<Neu_Multilinetextbox>({368,235,348,180},"Theme preview\nTry editing this text.\n\nHover over controls to see highlights.\nUse the theme buttons or keyboard shortcut.");
        slider=add<Neu_Slider>({368,430,348,40}); slider->setRange(0,100);
        spinner=add<Neu_Spinner>({368,490,348,40}); spinner->setRange(0,100);
        add<Neu_Button>({368,550,166,44},"Normal button");
        auto disabled=add<Neu_Button>({550,550,166,44},"Disabled button"); disabled->setEnabled(false);
        add<Neu_Label>({744,210,160,22},"Adjustable progress");
        add<Neu_Label>({932,210,160,22},"Completed progress");
        square=add<Neu_ProgressSquare>({744,235,160,160});
        auto full=add<Neu_ProgressSquare>({932,235,160,160}); full->setProgress(1);
        bar=add<Neu_ProgressBar>({744,430,348,36});
        auto quarter=add<Neu_ProgressBar>({744,490,348,36},"Progress 25%"); quarter->setProgress(0.25f);
        add<Neu_MultilineLabel>({744,550,348,100},"Perimeter line: 10px\nInside gap: 6px\nThemed six-edge outline");
        add<Neu_Label>({24,690,1072,42},"Select a theme above. All controls keep their state when the active window changes theme.");
        // Added last so the open dropdown renders over the other sample controls.
        selector=add<ThemeShowcaseSelector>({24,168,320,36}); selector->setItems(Neu_Theme::BuiltInThemeNames()); selector->setMultiSelect(false);
        Neu_Callbacks select; select.userData=this;
        select.onSelectionChanged=[](Neu_Control*,int,int,const char* value,void* data) { if(value) static_cast<ThemeShowcase*>(data)->window.setTheme(Neu_Theme::BuiltInThemeByName(value)); };
        selector->setCallbacks(select);
        Neu_Callbacks slide; slide.userData=this;
        slide.onClick=[](Neu_Control* c,void* data) { static_cast<ThemeShowcase*>(data)->setProgress(static_cast<Neu_Slider*>(c)->value()); }; slider->setCallbacks(slide);
        Neu_Callbacks spin; spin.userData=this;
        spin.onTextChanged=[](Neu_Control* c,const char*,void* data) {
            auto& self=*static_cast<ThemeShowcase*>(data); const int value=static_cast<Neu_Spinner*>(c)->value();
            self.square->setProgress(value/100.0f); self.bar->setProgress(value/100.0f);
            self.bar->setText("Progress "+std::to_string(value)+"%"); self.slider->setValue(value);
        }; spinner->setCallbacks(spin);
        setProgress(65);
        window.setOnThemeChanged(changed,this);
        window.setTheme(Neu_Theme::SupernovaRed1());
    }
};
} // namespace neutrino_examples
