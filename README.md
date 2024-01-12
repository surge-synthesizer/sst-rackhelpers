# Rack Helpers for SST properties

This is a collection of header-only C++-17 utilities that we find useful when
writing rack modules. This README serves as documentation.

Core features are

* neighbor and mixmaster connect menus
* Handy UI things we use often
* Some JSON read/write protection code which makes a bit less boilerplate

You are probably here for the first 

# How to use this generally

- add it as a submodule or copy it
- if you use CMake just `add_subdirectory` it
- if you use rack make, add the `include` directory into your include path with
  `-Ilibs/sst-rackhelpers/include` or whatnot and turn on C++17 by adding
  `-std=c++17` to your `CXXFLAGS` before you include `plugin.mk` 
   and after you include `plugin.mk` add `CXXFLAGS := $(filter-out -std=c++11,$(CXXFLAGS))`
- Include the header you want and use the functions

# Neighbor Connectors

The point of "NeighborConnectors" and so on is three fold

- A module can advertise specific ports as input- or output- with a label for audio 
  connection
- You can take a port and augment it so it has a right mouse menu to connect to
  other neighbor connectors. To use this, you need to advertise a
  port as an output port using the neighbor API.
- The same API allows you to add automatic connections to instances of MixMaster or
  AuxMater in your session.

## Advertising your ports

`sst::rackhelpers::mdoule_connector::NeighborConnectable_V1` is an ABI-stable
  exportable interface. If your module inherits this, you will participate in
  neighbor connectivity.

It has two typedefs and two virtual methods. The virtual methods return a list
  of primary inputs and primary outputs as a given pair. If you don't override the
inputs or outputs function, it returns an optional without a value.

For the simplest case of stereo in and stereo out, you can have an implementation 
such as this from the Surge VCF:

```cpp
    std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs() override
    {
        return {{std::make_pair("Input", std::make_pair(INPUT_L, INPUT_R))}};
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs() override
    {
        return {{std::make_pair("Output", std::make_pair(OUTPUT_L, OUTPUT_R))}};
    }
```

whereas a more complicated module, like the surge mixer with 3 inputs, has an
implementation like this

```cpp
    std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs() override
    {
        return {{std::make_pair("Input 1", std::make_pair(INPUT_OSC1_L, INPUT_OSC1_R)),
                 std::make_pair("Input 2", std::make_pair(INPUT_OSC2_L, INPUT_OSC2_R)),
                 std::make_pair("Input 3", std::make_pair(INPUT_OSC3_L, INPUT_OSC3_R))}};
    }

    std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs() override
    {
        return {{std::make_pair("Output", std::make_pair(OUTPUT_L, OUTPUT_R))}};
    }
```

If you have mono i/o simply return `-1` as the second item of the pair for the 
right channel, like this example from BaconPlugs Bitulator:

```cpp
    std::optional<std::vector<labeledStereoPort_t>> getPrimaryInputs() override
    {
        return {{std::make_pair("Input", std::make_pair(SIGNAL_INPUT, -1))}};
    }
    std::optional<std::vector<labeledStereoPort_t>> getPrimaryOutputs() override
    {
        return {{std::make_pair("Outputs", std::make_pair(CRUNCHED_OUTPUT, -1))}};
    }
```

Once you inherit the base class and add implementations of getInputs, you should
be able to auto-connect with a surge module or an airwindows module. Just touch side by
side (with 2.2.1) or put in the same row (at head) and you will see a connector.

In the future if we add other features, we will add `NeighborConnectable_V2` which
  inherits V1, like windows does basically.

## Adding connectivity menu to your port

Once you are a connectable, you want to add the right mouse menu items to your port.
To do this you can use `sst::rackhelpers::module_connector::PortConnectionMixin`
mixin class. This class uses CRTP to inherit from whatever port base class you want
and add an expanded `appendContextMenu` method which adds all the fun menu items.

To use it you need to change your `addInput` or `addOutput` widget code. 

Typical rack code will have idiomatic code like this

```cpp
addOutput(rack::createOutputCentered(pos, mod, Module::OUTPUT_L));
```

or perhaps

```cpp
addOutput(rack::createOutputCentered<MyCustomPort>(pos, mod, Module::OUTPUT_L));
```

with the first defaulting to `rack::SvgPort` as the port. To use the module
connector, we usually split this into a create phase and an add phase, and the
create phase uses the mixin to wrap the underlying port. So for instance, the 
second would change to

```cpp
auto outl = rack::createOutputCenetered<
        sst::rackhelpers::module_connector::PortConnectorMixin<MyCustomPort>
    >(pos, mod, Module::OUTPUT_L);

outl->connectAsOutputToNeighbor = true;
outl->connectAsOutputToMixmaster = true;
outl->mixmasterStereoCompnaion = Module::OUTPUT_R;

addOutput(outl);
```

so the changes are

1. Rather than using `MyCustomPort` you wrap it in a `PortConnectorMixin`. (You can 
also this mixin in your own custom port class of course)
2. You keep a reference for a little bit so you can set up the features
3. And then you add it

The features are

- `connectAsOutputToNeighbor`. If this is true and you impelment `NeighborConnectable_V1` and 
   your left or right neighbor or row mate also implements the API, then add a connection menu
- `connectAsOutputToMixmaster` (or `connectAsInputToMixmaster` in the input case) generates 
  a connection menu to MixMaster and AuxMaster. To do this you need to specifiy a stereo
  companion to the port if you have one. Note that by specifying the stereo compnaion it
  makes it possible to add mixmaster connectivity without implementing the Neighbor api.

# UI Helpers

- `sst::rackhelpers::ui::BufferedDrawFunctionWidget` is a FrameBufferWidget
   constructed with a lambda to do the drawing. Surge, BaconPlugs and AirWinRack
   use this literally everywhere. There's a version for having a layer also.

# JSON read/write

- `sst::rackhelpers::json::jsonSafeGet<T>` looks for a key in a json root and
  if the key both exists and is of the right type, returns the value, signaling
  error using `std::optional`
- `sst::rackhelpers::json::convertFromJson` is a templated thing which returns a 
  `std::optional<T>` based on reading a json object and it having the correct type.

