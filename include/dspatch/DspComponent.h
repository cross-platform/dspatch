/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2015 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LGPLv3.txt included in the packaging of this
file. Please review the following information to ensure the GNU Lesser
General Public License version 3.0 requirements will be met:
http://www.gnu.org/copyleft/lgpl.html.

Other Usage
Alternatively, this file may be used in accordance with the terms and
conditions contained in a signed written agreement between you and
Marcus Tomlinson.

DSPatch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
************************************************************************/

#ifndef DSPCOMPONENT_H
#define DSPCOMPONENT_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspSignalBus.h>
#include <dspatch/DspWireBus.h>
#include <dspatch/DspComponentThread.h>
#include <dspatch/DspParameter.h>

class DspCircuit;

//=================================================================================================
/// Abstract base class for all DSPatch components

/**
Classes derived from DspComponent can be added to a DspCircuit and routed to and from other
DspComponents.

On construction, derived classes must configure the component's IO buses by calling AddInput_() and
AddOutput_() respectively, as well as populate the component's parameter map via AddParameter_()
(see DspParameter).

Derived classes must also implement the virtual method: Process_(). The Process_() method is a
callback from the DSPatch engine that occurs when a new set of input signals is ready for
processing. The Process_() method has 2 arguments: the input bus, and the output bus. This
method's purpose is to pull its required inputs out of the input bus, process these inputs, and
populate the output bus with the results (see DspSignalBus).

Derived classes that expose parameters will also need to implement the virtual ParameterUpdating_()
method. The ParameterUpdating_() method is a callback from the DSPatch engine that occurs when an
update to a component parameter has been requested via the public SetParameter() method.
ParameterUpdating_() has 2 arguments: the parameter name, and the new parameter value to be set.
This method's purpose is to: 1. validate that the new value is legal, 2. make the necessary
internal changes associated with that parameter change, and 3. update the target parameter itself
by calling the protected SetParameter_() method. If the new parameter value is legal and the update
was successful, ParameterUpdating_() should return true, otherwise, it should return false.

In order for a component to do any work it must be ticked over. This is performed by repeatedly
calling the Tick() and Reset() methods. The Tick() method is responsible for acquiring the next set
of input signals from component input wires and populating the component's input bus. To insure
that these inputs are up-to-date, the dependent component first calls all of its input components'
Tick() methods -hence recursively called in all components going backward through the circuit (This
is what's classified as a "pull system"). The acquired input bus is then passed to the Process_()
method. The Reset() method then informs the component that the last circuit traversal has completed
and hence can execute the next Tick() request. A component's Tick() and Reset() methods can be
called in a loop from the main application thread, or alternatively, by calling StartAutoTick(), a
separate thread will spawn, automatically calling Tick() and Reset() methods continuously (This is
most commonly used to tick over an instance of DspCircuit).
*/

class DLLEXPORT DspComponent
{
public:
    enum CallbackType
    {
        InputAdded,
        InputRemoved,
        OutputAdded,
        OutputRemoved,
        ParameterAdded,
        ParameterRemoved,
        ParameterUpdated
    };
    typedef void (*Callback_t)(DspComponent* component, CallbackType const& callbackType, int index, void* userData);

    DspComponent();
    virtual ~DspComponent();

    void SetCallback(Callback_t const& callback, void* userData = NULL);

    void SetComponentName(std::string const& componentName);
    std::string GetComponentName() const;

    template <class FromOutputId, class ToInputId>
    bool ConnectInput(DspComponent* fromComponent, FromOutputId const& fromOutput, ToInputId const& toInput);

    template <class FromOutputId, class ToInputId>
    bool ConnectInput(DspComponent& fromComponent, FromOutputId const& fromOutput, ToInputId const& toInput);

    template <class FromOutputId, class ToInputId>
    void DisconnectInput(DspComponent const* fromComponent, FromOutputId const& fromOutput, ToInputId const& toInput);

    template <class FromOutputId, class ToInputId>
    void DisconnectInput(DspComponent const& fromComponent, FromOutputId const& fromOutput, ToInputId const& toInput);

    void DisconnectInput(int inputIndex);
    void DisconnectInput(std::string const& inputName);
    void DisconnectInput(DspComponent const* inputComponent);
    void DisconnectAllInputs();

    int GetInputCount();
    int GetOutputCount();
    int GetParameterCount();

    std::string GetInputName(int index);
    std::string GetOutputName(int index);
    std::string GetParameterName(int index);

    bool GetParameter(int index, DspParameter& param);
    DspParameter const* GetParameter(int index);
    bool SetParameter(int index, DspParameter const& param);

    void Tick();
    void Reset();

    void StartAutoTick();
    void StopAutoTick();
    void PauseAutoTick();
    void ResumeAutoTick();

protected:
    virtual void Process_(DspSignalBus&, DspSignalBus&);
    virtual bool ParameterUpdating_(int, DspParameter const&);

    bool AddInput_(std::string const& inputName = "");
    bool AddOutput_(std::string const& outputName = "");
    int AddParameter_(std::string const& paramName, DspParameter const& param);

    bool RemoveInput_();
    bool RemoveOutput_();
    bool RemoveParameter_();

    void RemoveAllInputs_();
    void RemoveAllOutputs_();
    void RemoveAllParameters_();

    int GetInputCount_();
    int GetOutputCount_();
    int GetParameterCount_();

    DspParameter const* GetParameter_(int index) const;
    bool SetParameter_(int index, DspParameter const& param);

private:
    virtual void _PauseAutoTick();

    void _SetParentCircuit(DspCircuit* parentCircuit);
    DspCircuit* _GetParentCircuit();

    bool _FindInput(std::string const& signalName, int& returnIndex) const;
    bool _FindInput(int signalIndex, int& returnIndex) const;
    bool _FindOutput(std::string const& signalName, int& returnIndex) const;
    bool _FindOutput(int signalIndex, int& returnIndex) const;

    void _SetBufferCount(int bufferCount);
    int _GetBufferCount() const;

    void _ThreadTick(int threadNo);
    void _ThreadReset(int threadNo);

    bool _SetInputSignal(int inputIndex, DspSignal const* newSignal);
    bool _SetInputSignal(int inputIndex, int threadIndex, DspSignal const* newSignal);
    DspSignal* _GetOutputSignal(int outputIndex);
    DspSignal* _GetOutputSignal(int outputIndex, int threadIndex);

    void _WaitForRelease(int threadNo);
    void _ReleaseThread(int threadNo);

private:
    friend class DspCircuit;
    friend class DspCircuitThread;

    DspCircuit* _parentCircuit;

    int _bufferCount;

    DspSignalBus _inputBus;
    DspSignalBus _outputBus;

    std::vector<DspSignalBus> _inputBuses;
    std::vector<DspSignalBus> _outputBuses;

    std::vector< std::pair<std::string, DspParameter> > _parameters;

    std::string _componentName;
    bool _isAutoTickRunning;
    bool _isAutoTickPaused;
    int _pauseCount;

    DspWireBus _inputWires;

    bool _hasTicked;

    DspComponentThread _componentThread;

    std::vector<bool*> _hasTickeds;  // bool pointers ensure that parallel threads will only read from this vector
    std::vector<bool> _gotReleases;  // bool pointers not used here as only 1 thread writes to this vector at a time
    std::vector<DspMutex> _releaseMutexes;
    std::vector<DspWaitCondition> _releaseCondts;

    Callback_t _callback;
    void* _userData;
};

//=================================================================================================

template <class FromOutputId, class ToInputId>
bool DspComponent::ConnectInput(DspComponent* fromComponent, FromOutputId const& fromOutput, ToInputId const& toInput)
{
    int fromOutputIndex;
    int toInputIndex;

    if (!fromComponent->_outputBus.FindSignal(fromOutput, fromOutputIndex) ||
        !_inputBus.FindSignal(toInput, toInputIndex))
    {
        return false;
    }

    PauseAutoTick();
    _inputWires.AddWire(fromComponent, fromOutputIndex, toInputIndex);
    ResumeAutoTick();

    return true;
}

//-------------------------------------------------------------------------------------------------

template <class FromOutputId, class ToInputId>
bool DspComponent::ConnectInput(DspComponent& fromComponent, FromOutputId const& fromOutput, ToInputId const& toInput)
{
    return ConnectInput(&fromComponent, fromOutput, toInput);
}

//-------------------------------------------------------------------------------------------------

template <class FromOutputId, class ToInputId>
void DspComponent::DisconnectInput(DspComponent const* fromComponent,
                                   FromOutputId const& fromOutput,
                                   ToInputId const& toInput)
{
    int fromOutputIndex;
    int toInputIndex;

    if (!fromComponent->_outputBus.FindSignal(fromOutput, fromOutputIndex) ||
        !_inputBus.FindSignal(toInput, toInputIndex))
    {
        return;
    }

    PauseAutoTick();
    _inputWires.RemoveWire(fromComponent, fromOutputIndex, toInputIndex);
    ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

template <class FromOutputId, class ToInputId>
void DspComponent::DisconnectInput(DspComponent const& fromComponent,
                                   FromOutputId const& fromOutput,
                                   ToInputId const& toInput)
{
    DisconnectInput(&fromComponent, fromOutput, toInput);
}

//=================================================================================================

#endif  // DSPCOMPONENT_H
