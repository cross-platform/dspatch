[![Build Status](https://travis-ci.org/MarcusTomlinson/DSPatch.svg?branch=master)](https://travis-ci.org/MarcusTomlinson/DSPatch)
[![Build status](https://ci.appveyor.com/api/projects/status/kqh1el01gnaarga8/branch/master?svg=true)](https://ci.appveyor.com/project/MarcusTomlinson/dspatch/branch/master)

# DSPatch

The Refreshingly Simple C++ Dataflow Framework

Webite: http://flowbasedprogramming.com

DSPatch, pronounced "dispatch", is a powerful C++ dataflow framework. DSPatch is not limited to any particular domain or data type, from reactive programming to stream processing, DSPatch's generic, object-oriented API allows you to create virtually any dataflow system imaginable.


## Build

```
git clone https://github.com/cross-platform/dspatch.git
cd DSPatch
mkdir build
cd build
cmake ..
make
```

- *`cmake ..` will auto-detect your IDE / compiler. To manually select one, use `cmake -G`.*
- *When building for an IDE, instead of `make`, simply open the cmake generated project file.*


### See also:

DSPatchables (https://github.com/cross-platform/dspatchables): A DSPatch component repository.

DSPatcher (https://github.com/cross-platform/dspatcher): A cross-platform graphical tool for building DSPatch circuits.
