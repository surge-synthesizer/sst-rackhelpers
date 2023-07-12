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

#ifndef INCLUDE_SST_RACKHELPERS_NEIGHBOR_CONNECTABLE_H
#define INCLUDE_SST_RACKHELPERS_NEIGHBOR_CONNECTABLE_H

#include <optional>
#include <vector>
#include <utility>

namespace sst::rackhelpers::module_connector
{
struct NeighborConnectable_V1
{
    virtual ~NeighborConnectable_V1() = default;

    typedef std::pair<int, int> stereoPort_t;
    typedef std::pair<std::string, stereoPort_t> labeledStereoPort_t;

    virtual std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs()
    {
        return std::nullopt;
    }
    virtual std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs()
    {
        return std::nullopt;
    }
};
} // namespace sst::rackhelpers::module_connector
#endif // SURGEXTRACK_NEIGHBOR_CONNECTABLE_H
