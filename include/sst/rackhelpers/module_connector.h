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

void makeCableBetween(rack::Module *inModule, int inId, rack::Module *outModule, int outId,
                      NVGcolor col, rack::history::ComplexAction *complexAction = nullptr)
{
    // Create cable attached to cloned ModuleWidget's input
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

void addOutputConnector(rack::Menu *menu, rack::Module *m, std::pair<int, int> cto,
                        rack::Module *source, int portL, int portR, NVGcolor cableColor)
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
        menu->addChild(rack::createMenuItem(nm, "", [=]() {
            rack::history::ComplexAction *complexAction = new rack::history::ComplexAction;
            complexAction->name = "connect to " + nm;
            makeCableBetween(m, cto.first, source, portL, cableColor, complexAction);
            makeCableBetween(m, cto.second, source, portR, cableColor, complexAction);
            APP->history->push(complexAction);
        }));
    }
}

void addInputConnector(rack::Menu *menu, rack::Module *m, std::pair<int, int> cto,
                       rack::Module *source, int portL, int portR, NVGcolor cableColor)
{
    auto nm = m->outputInfos[cto.first]->name;

    // MixMaster names are of form "-01- left" or "lbl left" so kill the "left"
    auto lpos = nm.find(" left");
    if (lpos != std::string::npos)
        nm = nm.substr(0, lpos);

    menu->addChild(rack::createMenuItem(nm, "", [=]() {
        rack::history::ComplexAction *complexAction = new rack::history::ComplexAction;
        complexAction->name = "connect to " + nm;
        makeCableBetween(source, portL, m, cto.first, cableColor, complexAction);
        makeCableBetween(source, portR, m, cto.second, cableColor, complexAction);
        APP->history->push(complexAction);
    }));
}

void outputsToMixMasterSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source, int portL,
                               int portR, NVGcolor cableColor = nvgRGB(0xFF, 0x90, 0x00))
{
    auto numIn = mixMasterNumInputs(m);
    if (numIn == 0)
        return;

    menu->addChild(rack::createMenuLabel("Connect to MixMaster Input"));
    menu->addChild(new rack::MenuSeparator());

    for (int i = 0; i < numIn; ++i)
    {
        auto cto = mixMasterInput(m, i);
        addOutputConnector(menu, m, cto, source, portL, portR, cableColor);
    }
}

void outputsToAuxSpanderSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source, int portL,
                                int portR, NVGcolor cableColor = nvgRGB(0xFF, 0x90, 0x00))
{
    auto numIn = auxSpanderNumInputs(m);
    if (numIn == 0)
        return;

    menu->addChild(rack::createMenuLabel("Connect to AuxSpander Return"));
    menu->addChild(new rack::MenuSeparator());

    for (int i = 0; i < numIn; ++i)
    {
        auto cto = auxSpanderReturn(m, i);

        addOutputConnector(menu, m, cto, source, portL, portR, cableColor);
    }
}

void inputsFromAuxSpanderSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source, int portL,
                                 int portR, NVGcolor cableColor = nvgRGB(0xFF, 0x90, 0x00))
{
    auto numIn = auxSpanderNumInputs(m);
    if (numIn == 0)
        return;

    menu->addChild(rack::createMenuLabel("Connect to AuxSpander Return"));
    menu->addChild(new rack::MenuSeparator());

    for (int i = 0; i < numIn; ++i)
    {
        auto cto = auxSpanderSend(m, i);

        addInputConnector(menu, m, cto, source, portL, portR, cableColor);
    }
}

inline void connectOutputToNeighorInput(rack::Menu *menu, rack::Module *me, bool useLeft)
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

    auto meOut = meNC->getPrimaryOutputs();
    auto neIn = neighNC->getPrimaryInputs();

    if (!meOut.has_value() || !neIn.has_value())
        return;

    std::string nm = "Connect Output to " + neighbor->getModel()->name;
    auto cableColor = nvgRGB(0xFF, 0x90, 0x00);

    menu->addChild(new rack::MenuSeparator());
    menu->addChild(rack::createMenuItem(nm, "", [=]() {
        rack::history::ComplexAction *complexAction = new rack::history::ComplexAction;
        complexAction->name = nm;
        makeCableBetween(neighbor, neIn->first, me, meOut->first, cableColor, complexAction);
        makeCableBetween(neighbor, neIn->second, me, meOut->second, cableColor, complexAction);
        APP->history->push(complexAction);
    }));
}

} // namespace sst::rackhelpers::module_connector

#endif // SURGEXTRACK_MIXMASTER_CONNECTOR_H
