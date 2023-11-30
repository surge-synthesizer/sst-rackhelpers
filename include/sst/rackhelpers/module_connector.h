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
            if (portL >= 0)
                makeCableBetween(m, cto.first, source, portL, cableColor, complexAction);
            if (portR >= 0)
                makeCableBetween(m, cto.second, source, portR, cableColor, complexAction);
            APP->history->push(complexAction);
        }));
    }
}

inline void addInputConnector(rack::Menu *menu, rack::Module *m, std::pair<int, int> cto,
                              rack::Module *source, int portL, int portR, NVGcolor cableColor)
{
    if (portL < 0 && portR < 0)
        return;

    auto nm = m->outputInfos[cto.first]->name;

    // MixMaster names are of form "-01- left" or "lbl left" so kill the "left"
    auto lpos = nm.find(" left");
    if (lpos != std::string::npos)
        nm = nm.substr(0, lpos);

    menu->addChild(rack::createMenuItem(nm, "", [=]() {
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
                                      int portL, int portR, NVGcolor cableColor)
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

inline void outputsToAuxSpanderSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source,
                                       int portL, int portR, NVGcolor cableColor)
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

inline void inputsFromAuxSpanderSubMenu(rack::Menu *menu, rack::Module *m, rack::Module *source,
                                        int portL, int portR, NVGcolor cableColor)
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

    for (const auto &[olab, meOutB] : *meOutVec)
    {
        menu->addChild(new rack::MenuSeparator());

        if (!((portId == meOutB.first) || (portId == meOutB.second)))
        {
            continue;
        }
        for (const auto &[ilab, neInB] : *neInVec)
        {
            std::string nm = "Connect " + olab + " to " + neighbor->getModel()->name + " " + ilab;

            if (neighbor->inputs[neInB.first].isConnected() ||
                (neInB.second >= 0 && neighbor->inputs[neInB.second].isConnected()))
            {
                menu->addChild(rack::createMenuLabel(nm + " (In Use)"));
            }
            else
            {
                auto cableColor = APP->scene->rack->getNextCableColor();

                menu->addChild(rack::createMenuItem(nm, "", [=, neIn = neInB, meOut = meOutB]() {
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
        if (connectOutputToNeighbor)
        {
            connectOutputToNeighorInput(menu, this->module, false, this->portId);
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
                        outputsToMixMasterSubMenu(x, m, this->module, lid, rid,
                                                  APP->scene->rack->getNextCableColor());
                    }));
            }

            for (auto m : auxM)
            {
                menu->addChild(
                    rack::createSubmenuItem(m->getModel()->name, "", [m, this, lid, rid](auto *x) {
                        outputsToAuxSpanderSubMenu(x, m, this->module, lid, rid,
                                                   APP->scene->rack->getNextCableColor());
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
                            inputsFromAuxSpanderSubMenu(x, m, this->module, lid, rid,
                                                        APP->scene->rack->getNextCableColor());
                        }));
                }
            }
        }
    }
};
} // namespace sst::rackhelpers::module_connector

#endif // SURGEXTRACK_MIXMASTER_CONNECTOR_H
