/*
 * sst-rackhelpers - a Surge Synth Team product
 *
 * A set of header-only utilities we use when making stuff for VCV Rack
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * sst-rackhelpers is released under the MIT license, found in the file
 * "LICENSE.md" in this repository.
 *
 * All source for sst-rackhelpers is available at
 * https://github.com/surge-synthesizer/sst-rackhelpers
 */

#ifndef INCLUDE_SST_RACKHELPERS_MODULE_CONNECTOR_H
#define INCLUDE_SST_RACKHELPERS_MODULE_CONNECTOR_H

#include "neighbor_connectable.h"

#include <cassert>
#include <optional>

namespace sst::rackhelpers::module_connector
{

struct MultiColorMenuItem : rack::MenuItem
{
    static constexpr int colBoxSz{12};
    static constexpr size_t maxCircles{6};
    void draw(const DrawArgs &args) override
    {
        BNDwidgetState state = BND_DEFAULT;

        if (APP->event->hoveredWidget == this)
            state = BND_HOVER;

        // Set active state if this MenuItem
        rack::Menu *parentMenu = dynamic_cast<rack::Menu *>(parent);
        if (parentMenu && parentMenu->activeEntry == this)
            state = BND_ACTIVE;

        // Main text and background
        if (!disabled)
            bndMenuItem(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());
        else
            bndMenuLabel(args.vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());

        bool halfMoons = false, halfTop = false;
        if (rack::settings::cableColors.size() > maxCircles)
        {
            halfMoons = true;
        }
        auto bl =
            box.size.x - 3 - colBoxSz * std::min(rack::settings::cableColors.size(), maxCircles);
        hoverColor = baseColor;
        int idx = 0;
        for (auto &col : rack::settings::cableColors)
        {
            auto vg = args.vg;
            nvgSave(vg);
            if (idx >= maxCircles && !halfTop)
            {
                halfTop = true;
                bl = box.size.x - 3 -
                     colBoxSz * std::min(rack::settings::cableColors.size(), maxCircles);
            }
            else if (halfMoons && idx >= maxCircles * 2)
            {
                break;
            }

            idx++;

            nvgBeginPath(vg);
            if (halfMoons)
            {
                if (halfTop)
                {
                    nvgScissor(vg, bl, box.size.y * 0.5, colBoxSz, box.size.y * 0.5);
                }
                else
                {
                    nvgScissor(vg, bl, 0, colBoxSz, box.size.y * 0.5);
                }
            }
            nvgEllipse(vg, bl + colBoxSz * 0.5, box.size.y * 0.5, (colBoxSz - 2) * 0.5,
                       (colBoxSz - 2) * 0.5);
            nvgFillColor(vg, col);
            nvgFill(vg);

            if (hoverPos.x >= bl && hoverPos.x < bl + colBoxSz)
            {
                bool doDraw = true;
                if (halfMoons)
                {
                    doDraw = false;
                    if (halfTop && hoverPos.y > box.size.y * 0.5)
                    {
                        doDraw = true;
                    }
                    else if (!halfTop && hoverPos.y < box.size.y * 0.5)
                    {
                        doDraw = true;
                    }
                }
                if (doDraw)
                {
                    nvgStrokeColor(vg, nvgRGB(255, 255, 255));
                    nvgStrokeWidth(vg, 1);
                    nvgStroke(vg);

                    if (halfMoons)
                    {
                        nvgBeginPath(vg);
                        nvgMoveTo(vg, bl + 1, box.size.y * 0.5);
                        nvgLineTo(vg, bl + colBoxSz - 2, box.size.y * 0.5);
                        nvgStrokeColor(vg, nvgRGB(255, 255, 255));
                        nvgStrokeWidth(vg, 1.5);
                        nvgStroke(vg);
                    }
                    hoverColor = col;
                }
            }
            bl += colBoxSz;
            nvgRestore(vg);
        }
    }

    rack::Vec hoverPos{-1, -1};

    void onHover(const HoverEvent &e) override
    {
        hoverPos = e.pos;
        OpaqueWidget::onHover(e);
    }

    void step() override
    {
        rack::MenuItem::step();
        box.size.x += colBoxSz * std::min(rack::settings::cableColors.size(), maxCircles);
    }

    NVGcolor baseColor{APP->scene->rack->getNextCableColor()};
    NVGcolor hoverColor{baseColor};
    void onAction(const rack::event::Action &e) override
    {
        action(hoverColor);
        if (alwaysConsume)
            e.consume(this);
    }

    std::function<void(const NVGcolor &)> action = [](const auto &) {};
    bool alwaysConsume{false};

    static MultiColorMenuItem *create(std::string text, std::string rightText,
                                      std::function<void(const NVGcolor &)> action,
                                      bool disabled = false, bool alwaysConsume = false)
    {
        auto item = new MultiColorMenuItem();
        item->text = text;
        item->rightText = rightText;
        item->action = action;
        item->disabled = disabled;
        item->alwaysConsume = alwaysConsume;
        return item;
    }
};

inline std::vector<rack::Module *> findMixMasters()
{
    auto mids = rack::contextGet()->engine->getModuleIds();
    std::vector<rack::Module *> result;
    for (auto mid : mids)
    {
        auto mod = rack::contextGet()->engine->getModule(mid);
        if (mod)
        {
            auto nm = mod->getModel()->name;
            auto pn = mod->getModel()->plugin->name;
            if ((nm == "MixMaster" || nm == "MixMasterJr") && (pn == "MindMeld"))
            {
                result.push_back(mod);
            }
        }
    }
    return result;
}

inline int mixMasterNumInputs(rack::Module *mm)
{
    assert(mm->getModel()->plugin->name == "MindMeld");
    if (mm->getModel()->name == "MixMaster")
        return 16;
    if (mm->getModel()->name == "MixMasterJr")
        return 8;
    return 0;
}

inline int auxSpanderNumInputs(rack::Module *mm)
{
    assert(mm->getModel()->plugin->name == "MindMeld");
    if (mm->getModel()->name == "AuxSpander")
        return 4;
    if (mm->getModel()->name == "AuxSpanderJr")
        return 4;
    return 0;
}

inline std::pair<int, int> mixMasterInput(rack::Module *mm, int channel)
{
    return {channel * 2, channel * 2 + 1};
}

inline std::pair<int, int> auxSpanderReturn(rack::Module *mm, int channel)
{
    return {channel * 2, channel * 2 + 1};
}

inline std::pair<int, int> auxSpanderSend(rack::Module *mm, int channel)
{
    // oh marc why are your sends not interleaved like your returns?
    return {channel, channel + 4};
}

inline std::vector<rack::Module *> findAuxSpanders()
{
    auto mids = rack::contextGet()->engine->getModuleIds();
    std::vector<rack::Module *> result;
    for (auto mid : mids)
    {
        auto mod = rack::contextGet()->engine->getModule(mid);
        if (mod)
        {
            auto nm = mod->getModel()->name;
            auto pn = mod->getModel()->plugin->name;
            if ((nm == "AuxSpander" || nm == "AuxSpanderJr") && (pn == "MindMeld"))
            {
                result.push_back(mod);
            }
        }
    }
    return result;
}

inline void makeCableBetween(rack::Module *inModule, int inId, rack::Module *outModule, int outId,
                             NVGcolor col, rack::history::ComplexAction *complexAction = nullptr)
{
    // Create cable attached to cloned this->moduleWidget's input
    rack::engine::Cable *clonedCable = new rack::engine::Cable;
    clonedCable->inputModule = inModule;
    clonedCable->inputId = inId;
    clonedCable->outputModule = outModule;
    clonedCable->outputId = outId;
    APP->engine->addCable(clonedCable);

    rack::app::CableWidget *clonedCw = new rack::app::CableWidget;
    clonedCw->setCable(clonedCable);
    clonedCw->color = col;
    APP->scene->rack->addCable(clonedCw);

    // history::CableAdd
    rack::history::CableAdd *hca = new rack::history::CableAdd;
    hca->setCable(clonedCw);
    if (complexAction)
    {
        complexAction->push(hca);
    }
    else
    {
        APP->history->push(hca);
    }
}

inline void addOutputConnector(rack::Menu *menu, rack::Module *m, std::pair<int, int> cto,
                               rack::Module *source, int portL, int portR)
{
    auto nm = m->inputInfos[cto.first]->name;

    // MixMaster names are of form "-01- left" or "lbl left" so kill the "left"
    auto lpos = nm.find(" left");
    if (lpos != std::string::npos)
        nm = nm.substr(0, lpos);

    if (m->inputs[cto.first].isConnected() || m->inputs[cto.second].isConnected())
        menu->addChild(rack::createMenuLabel(nm + " (In Use)"));
    else
    {
        menu->addChild(MultiColorMenuItem::create(nm, "", [=](const auto &cableColor) {
            rack::history::ComplexAction *complexAction = new rack::history::ComplexAction;
            complexAction->name = "connect to " + nm;
            if (portL >= 0)
                makeCableBetween(m, cto.first, source, portL, cableColor, complexAction);
            if (portR >= 0)
                makeCableBetween(m, cto.second, source, portR, cableColor, complexAction);
            APP->history->push(complexAction);
        }));
    }
}

inline void addInputConnector(rack::Menu *menu, rack::Module *m, std::pair<int, int> cto,
                              rack::Module *source, int portL, int portR)
{
    if (portL < 0 && portR < 0)
        return;

    auto nm = m->outputInfos[cto.first]->name;

    // MixMaster names are of form "-01- left" or "lbl left" so kill the "left"
    auto lpos = nm.find(" left");
    if (lpos != std::string::npos)
        nm = nm.substr(0, lpos);

    menu->addChild(MultiColorMenuItem::create(nm, "", [=](const auto &cableColor) {
        rack::history::ComplexAction *complexAction = new rack::history::ComplexAction;
        complexAction->name = "connect to " + nm;
        if (portL >= 0)
            makeCableBetween(source, portL, m, cto.first, cableColor, complexAction);
        if (portR >= 0)
            makeCableBetween(source, portR, m, cto.second, cableColor, complexAction);
        APP->history->push(complexAction);
    }));
}

inline void outputsToMixMasterSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source,
                                      int portL, int portR)
{
    auto numIn = mixMasterNumInputs(m);
    if (numIn == 0)
        return;

    menu->addChild(rack::createMenuLabel("Connect to MixMaster Input"));
    menu->addChild(new rack::MenuSeparator());

    for (int i = 0; i < numIn; ++i)
    {
        auto cto = mixMasterInput(m, i);
        addOutputConnector(menu, m, cto, source, portL, portR);
    }
}

inline void outputsToAuxSpanderSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source,
                                       int portL, int portR)
{
    auto numIn = auxSpanderNumInputs(m);
    if (numIn == 0)
        return;

    menu->addChild(rack::createMenuLabel("Connect to AuxSpander Return"));
    menu->addChild(new rack::MenuSeparator());

    for (int i = 0; i < numIn; ++i)
    {
        auto cto = auxSpanderReturn(m, i);

        addOutputConnector(menu, m, cto, source, portL, portR);
    }
}

inline void inputsFromAuxSpanderSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source,
                                        int portL, int portR)
{
    auto numIn = auxSpanderNumInputs(m);
    if (numIn == 0)
        return;

    menu->addChild(rack::createMenuLabel("Connect to AuxSpander Return"));
    menu->addChild(new rack::MenuSeparator());

    for (int i = 0; i < numIn; ++i)
    {
        auto cto = auxSpanderSend(m, i);

        addInputConnector(menu, m, cto, source, portL, portR);
    }
}

inline std::vector<rack::Module *> findNeighborInputConnectablesInRow(const rack::Vec &pos)
{
    auto mids = rack::contextGet()->engine->getModuleIds();
    std::map<float, rack::Module *> modMap; // to sort it by xpos
    std::vector<rack::Module *> result;
    for (auto mid : mids)
    {
        auto wid = APP->scene->rack->getModule(mid);
        auto mod = rack::contextGet()->engine->getModule(mid);
        auto nmod = dynamic_cast<NeighborConnectable_V1 *>(mod);
        if (nmod && wid)
        {
            auto ip = nmod->getPrimaryInputs();
            if (ip.has_value() && wid->box.pos != pos && wid->box.pos.y == pos.y)
            {
                modMap[wid->box.pos.x] = mod;
            }
        }
    }
    for (const auto &[k, v] : modMap)
    {
        result.push_back(v);
    }
    return result;
}

inline void addConnectionMenu(rack::Menu *menu, rack::Module *source, rack::Module *neighbor,
                              const NeighborConnectable_V1::labeledStereoPort_t &from,
                              const NeighborConnectable_V1::labeledStereoPort_t &to)
{
    const auto [olab, meOutB] = from;
    const auto &[ilab, neInB] = to;
    auto me = source;
    std::string nm = "To " + neighbor->getModel()->name + " " + ilab;

    if (neighbor->inputs[neInB.first].isConnected() ||
        (neInB.second >= 0 && neighbor->inputs[neInB.second].isConnected()))
    {
        menu->addChild(rack::createMenuLabel(nm + " (In Use)"));
    }
    else
    {
        menu->addChild(MultiColorMenuItem::create(
            nm, "", [=, neIn = neInB, meOut = meOutB](const auto &cableColor) {
                rack::history::ComplexAction *complexAction = new rack::history::ComplexAction;
                complexAction->name = nm;
                if (neIn.first >= 0 && meOut.first >= 0)
                    makeCableBetween(neighbor, neIn.first, me, meOut.first, cableColor,
                                     complexAction);
                if (neIn.second >= 0 && meOut.second >= 0)
                    makeCableBetween(neighbor, neIn.second, me, meOut.second, cableColor,
                                     complexAction);
                APP->history->push(complexAction);
            }));
    }
}

inline void connectOutputToNeighorInput(rack::Menu *menu, rack::Module *me, bool useLeft,
                                        int portId)
{
    rack::Module *neighbor{nullptr};
    if (useLeft)
        neighbor = me->getLeftExpander().module;
    else
        neighbor = me->getRightExpander().module;

    auto meNC = dynamic_cast<NeighborConnectable_V1 *>(me);

    if (!meNC)
        return;

    if (!neighbor)
        return;

    auto neighNC = dynamic_cast<NeighborConnectable_V1 *>(neighbor);
    if (!neighNC)
        return;

    auto meOutVec = meNC->getPrimaryOutputs();
    auto neInVec = neighNC->getPrimaryInputs();

    if (!meOutVec.has_value() || !neInVec.has_value())
        return;

    if (neInVec->empty() || meOutVec->empty())
        return;

    for (const auto &from : *meOutVec)
    {
        const auto [olab, meOutB] = from;

        if (!((portId == meOutB.first) || (portId == meOutB.second)))
        {
            continue;
        }
        menu->addChild(new rack::MenuSeparator());
        for (const auto &to : *neInVec)
        {
            addConnectionMenu(menu, me, neighbor, from, to);
        }
    }
}

inline void connectOutputToInRowInputs(rack::Menu *menu, rack::Module *me, int portId)
{
    auto meNC = dynamic_cast<NeighborConnectable_V1 *>(me);

    if (!meNC)
        return;

    auto thisWid = APP->scene->rack->getModule(me->id);
    if (!thisWid)
        return;

    auto neighbors = findNeighborInputConnectablesInRow(thisWid->box.pos);
    if (neighbors.empty())
        return;

    auto meOutVec = meNC->getPrimaryOutputs();

    if (!meOutVec.has_value())
        return;

    menu->addChild(rack::createMenuLabel("Connectable Modules in This Row"));
    for (auto neighbor : neighbors)
    {
        auto neighNC = dynamic_cast<NeighborConnectable_V1 *>(neighbor);
        if (!neighNC)
            continue; // should never happen but hey

        auto neInVec = neighNC->getPrimaryInputs();

        if (!meOutVec.has_value() || !neInVec.has_value())
            continue;

        if (neInVec->empty() || meOutVec->empty())
            continue;

        for (const auto &from : *meOutVec)
        {
            const auto [olab, meOutB] = from;

            if (!((portId == meOutB.first) || (portId == meOutB.second)))
            {
                continue;
            }

            menu->addChild(new rack::MenuSeparator());
            for (const auto &to : *neInVec)
            {
                addConnectionMenu(menu, me, neighbor, from, to);
            }
        }
    }
}

template <typename T> struct PortConnectionMixin : public T
{
    bool connectAsOutputToMixmaster{false};
    bool connectAsInputFromMixmaster{false};
    int mixMasterStereoCompanion{-1};

    bool connectOutputToNeighbor{false};

    void appendContextMenu(rack::Menu *menu) override
    {
        // Get base class context menu before I add my goodies
        T::appendContextMenu(menu);

        if (connectOutputToNeighbor)
        {
            connectOutputToNeighorInput(menu, this->module, false, this->portId);

            auto thisWid = APP->scene->rack->getModule(this->module->id);
            if (!thisWid)
                return;
            auto nb = findNeighborInputConnectablesInRow(thisWid->box.pos);
            if (!nb.empty())
            {
                menu->addChild(new rack::MenuSeparator());
                menu->addChild(rack::createSubmenuItem("This Row", "", [this](auto *x) {
                    connectOutputToInRowInputs(x, this->module, this->portId);
                }));
            }
        }

        if (connectAsOutputToMixmaster)
        {
            auto mixM = findMixMasters();
            auto auxM = findAuxSpanders();

            auto lid = this->portId;
            auto rid = mixMasterStereoCompanion;
            if (rid >= 0 && lid > rid)
                std::swap(lid, rid);

            if (!mixM.empty() || !auxM.empty())
            {
                menu->addChild(new rack::MenuSeparator());
            }
            for (auto m : mixM)
            {
                menu->addChild(
                    rack::createSubmenuItem(m->getModel()->name, "", [m, this, lid, rid](auto *x) {
                        outputsToMixMasterSubMenu(x, m, this->module, lid, rid);
                    }));
            }

            for (auto m : auxM)
            {
                menu->addChild(
                    rack::createSubmenuItem(m->getModel()->name, "", [m, this, lid, rid](auto *x) {
                        outputsToAuxSpanderSubMenu(x, m, this->module, lid, rid);
                    }));
            }
        }

        if (connectAsInputFromMixmaster)
        {
            auto auxM = findAuxSpanders();

            auto lid = this->portId;
            auto rid = mixMasterStereoCompanion;
            if (rid >= 0 && lid > rid)
                std::swap(lid, rid);

            if (this->module->inputs[lid].isConnected() ||
                (rid >= 0 && this->module->inputs[rid].isConnected()))
            {
                // Don't show the menu
            }
            else
            {
                menu->addChild(new rack::MenuSeparator());

                for (auto m : auxM)
                {
                    menu->addChild(rack::createSubmenuItem(
                        m->getModel()->name, "", [m, this, lid, rid](auto *x) {
                            inputsFromAuxSpanderSubMenu(x, m, this->module, lid, rid);
                        }));
                }
            }
        }
    }
};
} // namespace sst::rackhelpers::module_connector

#endif // SURGEXTRACK_MIXMASTER_CONNECTOR_H
