/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2024, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

#include <dspatch/SignalBus.h>

#include <string>
#include <vector>

namespace DSPatch
{

namespace internal
{
class Component;
}  // namespace internal

/// Abstract base class for DSPatch components

/**
Classes derived from Component can be added to a Circuit and routed to and from other Components.

On construction, derived classes must configure the component's IO buses by calling SetInputCount_() and SetOutputCount_()
respectively.

Derived classes must also implement the virtual method: Process_(). The Process_() method is a callback from the DSPatch engine
that occurs when a new set of input signals is ready for processing. The Process_() method has 2 arguments: the input bus, and the
output bus. This method's purpose is to pull its required inputs out of the input bus, process these inputs, and populate the
output bus with the results (see SignalBus).

In order for a component to do any work it must be ticked. This is performed by repeatedly calling the Tick() method. This method
is responsible for acquiring the next set of input signals from its input wires and populating the component's input bus. The
acquired input bus is then passed to the Process_() method.

<b>PERFORMANCE TIP:</b> If a component is capable of processing its buffers out-of-order within a stream processing circuit,
consider initialising its base with ProcessOrder::OutOfOrder to improve performance. Note however that Process_() must be
thread-safe to operate in this mode.
*/

class DLLEXPORT Component
{
public:
    NONCOPYABLE( Component );

    using SPtr = std::shared_ptr<Component>;

    enum class ProcessOrder
    {
        InOrder,
        OutOfOrder
    };

    Component( ProcessOrder processOrder = ProcessOrder::InOrder );
    virtual ~Component();

    bool ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput );

    void DisconnectInput( int inputNo );
    void DisconnectInput( const Component::SPtr& fromComponent );
    void DisconnectAllInputs();

    int GetInputCount() const;
    int GetOutputCount() const;

    std::string GetInputName( int inputNo ) const;
    std::string GetOutputName( int outputNo ) const;

    void SetBufferCount( int bufferCount, int startBuffer );
    int GetBufferCount() const;

    void TickSeries( int bufferNo );
    void TickParallel( int bufferNo );

    void ScanSeries( std::vector<Component*>& components );
    void ScanParallel( std::vector<std::vector<DSPatch::Component*>>& componentsMap, int& scanPosition );
    void EndScan();

protected:
    virtual void Process_( SignalBus&, SignalBus& ) = 0;

    void SetInputCount_( int inputCount, const std::vector<std::string>& inputNames = {} );
    void SetOutputCount_( int outputCount, const std::vector<std::string>& outputNames = {} );

private:
    internal::Component* p;
};

}  // namespace DSPatch
