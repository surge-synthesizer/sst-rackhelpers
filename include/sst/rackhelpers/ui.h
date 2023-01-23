
#ifndef AIRWIN2RACK_UI_H
#define AIRWIN2RACK_UI_H

namespace sst::rackhelpers::ui
{

struct BufferedDrawFunctionWidget : virtual rack::FramebufferWidget
{
    typedef std::function<void(NVGcontext *)> drawfn_t;
    drawfn_t drawf;

    struct InternalBDW : rack::TransparentWidget
    {
        drawfn_t drawf;
        InternalBDW(rack::Rect box_, drawfn_t draw_) : drawf(draw_) { box = box_; }

        void draw(const DrawArgs &args) override { drawf(args.vg); }
    };

    InternalBDW *kid = nullptr;
    BufferedDrawFunctionWidget(rack::Vec pos, rack::Vec sz, drawfn_t draw_) : drawf(draw_)
    {
        box.pos = pos;
        box.size = sz;
        auto kidBox = rack::Rect(rack::Vec(0, 0), box.size);
        kid = new InternalBDW(kidBox, drawf);
        addChild(kid);
    }
};

struct BufferedDrawFunctionWidgetOnLayer : BufferedDrawFunctionWidget
{
    int layer{1};
    BufferedDrawFunctionWidgetOnLayer(rack::Vec pos, rack::Vec sz, drawfn_t draw_, int ly = 1)
        : BufferedDrawFunctionWidget(pos, sz, draw_), layer(ly)
    {
    }

    void draw(const DrawArgs &args) override { return; }
    void drawLayer(const DrawArgs &args, int dl) override
    {
        if (dl == layer)
        {
            BufferedDrawFunctionWidget::draw(args);
        }
    }
};

}

#endif // AIRWIN2RACK_UI_H
