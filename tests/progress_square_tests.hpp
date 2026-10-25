#include <limits>

// Included by the native integration suite, sharing check() and capture().
struct ProgressTestSurface {
    HDC dc{nullptr}; HBITMAP bitmap{nullptr}; HGDIOBJ previous{nullptr};
    ProgressTestSurface(int width=300, int height=240) {
        HDC screen=GetDC(nullptr); dc=CreateCompatibleDC(screen);
        bitmap=CreateCompatibleBitmap(screen,width,height); previous=SelectObject(dc,bitmap);
        ReleaseDC(nullptr,screen);
        RECT all{0,0,width,height}; HBRUSH background=CreateSolidBrush(RGB(1,2,3));
        FillRect(dc,&all,background); DeleteObject(background);
    }
    ~ProgressTestSurface() { SelectObject(dc,previous); DeleteObject(bitmap); DeleteDC(dc); }
};

static void testProgressSquare(Neu_Application& app)
{
    const COLORREF accent=RGB(250,0,200);
    const Neu_Layout layout{20,20,160,160};
    Neu_ProgressSquare progress(layout); Neu_Button button(layout);
    auto dark=Neu_Theme::MaterialDark(), light=Neu_Theme::MaterialLight();
    auto rounded=dark; rounded.setAllCorners(Neu_CornerStyle::RoundedCorner);
    auto beveled=dark; beveled.setAllCorners(Neu_CornerStyle::EdgeCorner);
    for (auto theme : {dark,light,rounded,beveled}) {
        theme.accent={250,0,200,255};
        Neu_SetCurrentDrawingTheme(theme);
        ProgressTestSurface reference, result;
        button.draw(nullptr,reference.dc,reference.dc,theme);
        progress.setProgress(0); progress.draw(nullptr,result.dc,result.dc,theme);
        bool match=true;
        for (int y=0;y<200;++y) for(int x=0;x<200;++x) {
            if(x>55 && x<145 && y>55 && y<145) continue; // Centered percentage text.
            match &= GetPixel(reference.dc,x,y)==GetPixel(result.dc,x,y);
        }
        check(match,"zero progress has exactly the button face and themed outline");
        int previousCount=-1;
        for(float value : {0.0f,0.01f,0.25f,0.5f,0.75f,0.99f,1.0f}) {
            ProgressTestSurface frame; progress.setProgress(value);
            progress.draw(nullptr,frame.dc,frame.dc,theme);
            int count=0;
            for(int y=20;y<=180;++y) for(int x=20;x<=180;++x) count+=GetPixel(frame.dc,x,y)==accent;
            check(count>=previousCount,"clockwise progress fills monotonically at boundary values");
            if(value==0) check(count==0,"zero progress draws no progress line");
            if(value>0) check(count>0,"positive progress uses the theme accent");
            previousCount=count;
            if(value==0.25f) {
                check(GetPixel(frame.dc,110,27)==accent && GetPixel(frame.dc,90,27)!=accent,
                      "progress starts top center and travels clockwise");
            }
        }
        progress.setProgress(1); progress.draw(nullptr,result.dc,result.dc,theme);
        // GDI RoundRect's right/bottom outline is one pixel inside its RECT;
        // the themed polygon uses the supplied endpoint coordinates directly.
        const bool polygon=theme.topLeftCorner==Neu_CornerStyle::EdgeCorner || theme.topRightCorner==Neu_CornerStyle::EdgeCorner ||
                           theme.bottomLeftCorner==Neu_CornerStyle::EdgeCorner || theme.bottomRightCorner==Neu_CornerStyle::EdgeCorner;
        const int farEdge=polygon ? 180 : 179;
        bool sides=true;
        for(int i=1;i<=6;++i) {
            sides &= GetPixel(result.dc,20+i,60)!=accent && GetPixel(result.dc,farEdge-i,60)!=accent;
            sides &= GetPixel(result.dc,60,20+i)!=accent && GetPixel(result.dc,60,farEdge-i)!=accent;
        }
        check(sides,"all four straight sides have exactly six clear pixels inside the border");
        bool thickness=true;
        for(int i=7;i<=16;++i) {
            thickness &= GetPixel(result.dc,20+i,60)==accent && GetPixel(result.dc,farEdge-i,60)==accent;
            thickness &= GetPixel(result.dc,60,20+i)==accent && GetPixel(result.dc,60,farEdge-i)==accent;
        }
        check(thickness,"progress line is exactly ten pixels wide on all four sides");
        check(GetPixel(result.dc,37,60)!=accent && GetPixel(result.dc,farEdge-17,60)!=accent,
              "ten-pixel line stops before the inner content");
        if(theme.topLeftCorner==Neu_CornerStyle::EdgeCorner && theme.topRightCorner!=Neu_CornerStyle::EdgeCorner) {
            bool correct=true; int diagonalPixels=0;
            const double e=theme.edgeSize;
            for(int y=20;y<=180;++y) for(int x=20;x<=180;++x) {
                if(GetPixel(result.dc,x,y)!=accent) continue;
                const double diagonalTL=(x-20+y-20-e)/std::sqrt(2.0);
                const double diagonalBR=(320-e-(x-20)-(y-20))/std::sqrt(2.0);
                const double distance=std::min({double(x-20),double(180-x),double(y-20),double(180-y),diagonalTL,diagonalBR});
                correct &= distance>=7-1e-6 && distance<17+1e-6;
                diagonalPixels += diagonalTL<17 || diagonalBR<17;
            }
            check(correct && diagonalPixels>0,"six-edge line respects parallel offsets along slanted edges too");
        }
    }
    progress.setProgress(-1); check(progress.progress()==0,"negative progress clamps to zero");
    progress.setProgress(2); check(progress.progress()==1,"excess progress clamps to one");
    progress.setProgress(std::numeric_limits<float>::quiet_NaN()); check(progress.progress()==0,"NaN progress is safe");
    for(const auto size : {Neu_Size{240,90},Neu_Size{90,180},Neu_Size{24,24},Neu_Size{1,1}}) {
        ProgressTestSurface frame; auto theme=dark; theme.accent={250,0,200,255};
        progress.setLayout(Neu_Layout{20,20,size.width,size.height}); progress.setProgress(1);
        progress.draw(nullptr,frame.dc,frame.dc,theme);
        check(GetPixel(frame.dc,19,19)==RGB(1,2,3),"rectangular and tiny control drawing leaves outside pixels untouched");
        if(size.width<35 || size.height<35) {
            int count=0;
            for(int y=20;y<20+size.height;++y) for(int x=20;x<20+size.width;++x) count+=GetPixel(frame.dc,x,y)==accent;
            check(count==0,"tiny controls omit the line instead of violating gap or thickness");
        } else check(GetPixel(frame.dc,20+size.width/2,27)==accent,"perimeter follows rectangular control rather than an inscribed square");
    }
    progress.setLayout(layout); progress.setProgress(0.65f);
    ProgressTestSurface frame; progress.draw(nullptr,frame.dc,frame.dc,dark);
    const DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(int i=0;i<25;++i) progress.draw(nullptr,frame.dc,frame.dc,dark);
    check(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=before,"progress repainting does not leak GDI resources");

    Neu_Window demo(app,840,300,"Themed progress rendering"); check(demo.create(),"progress preview native window created");
    auto reference=std::make_shared<Neu_Button>(Neu_Layout{20,80,160,160}); reference->setText("Themed button"); demo.add(reference);
    int x=220;
    for(float value : {0.0f,0.65f,1.0f}) {
        auto item=std::make_shared<Neu_ProgressSquare>(Neu_Layout{x,80,160,160}); item->setProgress(value); demo.add(item); x+=200;
    }
    demo.setTheme(dark); capture(demo,"progress-themed-dark");
    demo.setTheme(light); capture(demo,"progress-themed-light");
    demo.close();
}
