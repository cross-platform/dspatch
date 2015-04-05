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

#include <dspatch/DspCircuit.h>
#include <dspatch/DspCircuitThread.h>
#include <dspatch/DspWire.h>

//=================================================================================================

DspCircuit::DspCircuit(int threadCount)
    : _currentThreadIndex(0)
    , _inToInWires(true)
    , _outToOutWires(false)
{
    SetThreadCount(threadCount);
}

//-------------------------------------------------------------------------------------------------

DspCircuit::~DspCircuit()
{
    StopAutoTick();
    RemoveAllComponents();
    SetThreadCount(0);
}

//=================================================================================================

void DspCircuit::SetThreadCount(int threadCount)
{
    if ((size_t)threadCount != _circuitThreads.size())
    {
        PauseAutoTick();

        // stop all threads
        for (size_t i = 0; i < _circuitThreads.size(); i++)
        {
            _circuitThreads[i].Stop();
        }

        // resize thread array
        _circuitThreads.resize(threadCount);

        // initialise and start all threads
        for (size_t i = 0; i < _circuitThreads.size(); i++)
        {
            _circuitThreads[i].Initialise(&_components, i);
            _circuitThreads[i].Start();
        }

        // set all components to the new thread count
        for (size_t i = 0; i < _components.size(); i++)
        {
            _components[i]->_SetBufferCount(threadCount);
        }

        ResumeAutoTick();
    }
}

//-------------------------------------------------------------------------------------------------

int DspCircuit::GetThreadCount() const
{
    return _circuitThreads.size();
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddComponent(DspComponent* component, std::string const& componentName)
{
    if (component != this && component != NULL)
    {
        std::string compName = componentName;

        // if the component has a name already
        if (component->GetComponentName() != "" && compName == "")
        {
            compName = component->GetComponentName();
        }

        int componentIndex;

        if (component->_GetParentCircuit() != NULL)
        {
            return false;  // if the component is already part of another circuit
        }
        if (_FindComponent(component, componentIndex))
        {
            return false;  // if the component is already in the array
        }
        if (_FindComponent(compName, componentIndex))
        {
            return false;  // if the component name is already in the array
        }

        // components within the circuit need to have as many buffers as there are threads in the circuit
        component->_SetParentCircuit(this);
        component->_SetBufferCount(_circuitThreads.size());
        component->SetComponentName(compName);

        PauseAutoTick();
        _components.push_back(component);
        ResumeAutoTick();

        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddComponent(DspComponent& component, std::string const& componentName)
{
    return AddComponent(&component, componentName);
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveComponent(DspComponent const* component)
{
    int componentIndex;

    if (_FindComponent(component, componentIndex))
    {
        PauseAutoTick();
        _RemoveComponent(componentIndex);
        ResumeAutoTick();
    }
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveComponent(DspComponent const& component)
{
    RemoveComponent(&component);
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveComponent(std::string const& componentName)
{
    int componentIndex;

    if (_FindComponent(componentName, componentIndex))
    {
        PauseAutoTick();
        _RemoveComponent(componentIndex);
        ResumeAutoTick();
    }
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveAllComponents()
{
    for (size_t i = 0; i < _components.size(); i++)
    {
        PauseAutoTick();
        _RemoveComponent(i--);  // size drops as one is removed
        ResumeAutoTick();
    }
}

//-------------------------------------------------------------------------------------------------

int DspCircuit::GetComponentCount() const
{
    return _components.size();
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::DisconnectComponent(std::string const& component)
{
    int componentIndex;

    if (!_FindComponent(component, componentIndex))  // verify component exists
    {
        return;
    }

    PauseAutoTick();
    _DisconnectComponent(componentIndex);
    ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddInput(std::string const& inputName)
{
    PauseAutoTick();
    bool result = AddInput_(inputName);
    ResumeAutoTick();
    return result;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddOutput(std::string const& outputName)
{
    PauseAutoTick();
    bool result = AddOutput_(outputName);
    ResumeAutoTick();
    return result;
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveInput()
{
    PauseAutoTick();
    RemoveInput_();
    ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveOutput()
{
    PauseAutoTick();
    RemoveOutput_();
    ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveAllInputs()
{
    PauseAutoTick();
    RemoveAllInputs_();
    ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveAllOutputs()
{
    PauseAutoTick();
    RemoveAllOutputs_();
    ResumeAutoTick();
}

//=================================================================================================

void DspCircuit::Process_(DspSignalBus& inputs, DspSignalBus& outputs)
{
    DspWire* wire;
    DspSignal* signal;

    // process in a single thread if this circuit has no threads
    // =========================================================
    if (_circuitThreads.size() == 0)
    {
        // set all internal component inputs from connected circuit inputs
        for (int i = 0; i < _inToInWires.GetWireCount(); i++)
        {
            wire = _inToInWires.GetWire(i);
            signal = inputs.GetSignal(wire->fromSignalIndex);
            wire->linkedComponent->_SetInputSignal(wire->toSignalIndex, signal);
        }

        // tick all internal components
        for (size_t i = 0; i < _components.size(); i++)
        {
            _components[i]->Tick();
        }

        // reset all internal components
        for (size_t i = 0; i < _components.size(); i++)
        {
            _components[i]->Reset();
        }

        // set all circuit outputs from connected internal component outputs
        for (int i = 0; i < _outToOutWires.GetWireCount(); i++)
        {
            wire = _outToOutWires.GetWire(i);
            signal = wire->linkedComponent->_GetOutputSignal(wire->fromSignalIndex);
            outputs.SetSignal(wire->toSignalIndex, signal);
        }
    }
    // process in multiple thread if this circuit has threads
    // ======================================================
    else
    {
        _circuitThreads[_currentThreadIndex].Sync();  // sync with thread x

        // set all circuit outputs from connected internal component outputs
        for (int i = 0; i < _outToOutWires.GetWireCount(); i++)
        {
            wire = _outToOutWires.GetWire(i);
            signal = wire->linkedComponent->_GetOutputSignal(wire->fromSignalIndex, _currentThreadIndex);
            outputs.SetSignal(wire->toSignalIndex, signal);
        }

        // set all internal component inputs from connected circuit inputs
        for (int i = 0; i < _inToInWires.GetWireCount(); i++)
        {
            wire = _inToInWires.GetWire(i);
            signal = inputs.GetSignal(wire->fromSignalIndex);
            wire->linkedComponent->_SetInputSignal(wire->toSignalIndex, _currentThreadIndex, signal);
        }

        _circuitThreads[_currentThreadIndex].Resume();  // resume thread x

        if ((size_t)++_currentThreadIndex >= _circuitThreads.size())  // shift to thread x+1
        {
            _currentThreadIndex = 0;
        }
    }
}

//=================================================================================================

void DspCircuit::_PauseAutoTick()
{
    // pause auto tick
    DspComponent::_PauseAutoTick();

    // manually tick until 0
    while (_currentThreadIndex != 0)
    {
        Tick();
        Reset();
    }

    // sync all threads
    for (size_t i = 0; i < _circuitThreads.size(); i++)
    {
        _circuitThreads[i].Sync();
    }
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::_FindComponent(DspComponent const* component, int& returnIndex) const
{
    for (size_t i = 0; i < _components.size(); i++)
    {
        if (_components[i] == component)
        {
            returnIndex = i;
            return true;
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::_FindComponent(DspComponent const& component, int& returnIndex) const
{
    return _FindComponent(&component, returnIndex);
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::_FindComponent(std::string const& componentName, int& returnIndex) const
{
    for (size_t i = 0; i < _components.size(); i++)
    {
        if (_components[i]->GetComponentName() != "" && _components[i]->GetComponentName() == componentName)
        {
            returnIndex = i;
            return true;
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::_FindComponent(int componentIndex, int& returnIndex) const
{
    if ((size_t)componentIndex < _components.size())
    {
        returnIndex = componentIndex;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::_DisconnectComponent(int componentIndex)
{
    // remove component from _inputComponents and _inputWires
    _components[componentIndex]->DisconnectAllInputs();

    // remove any connections this component has to other components
    for (size_t i = 0; i < _components.size(); ++i)
    {
        _components[i]->DisconnectInput(_components[componentIndex]);
    }

    // remove component from _inToInWires
    DspWire* wire;
    for (int i = 0; i < _inToInWires.GetWireCount(); i++)
    {
        wire = _inToInWires.GetWire(i);
        if (wire->linkedComponent == _components[componentIndex])
        {
            _inToInWires.RemoveWire(i);
        }
    }

    // remove component from _outToOutWires
    for (int i = 0; i < _outToOutWires.GetWireCount(); i++)
    {
        wire = _outToOutWires.GetWire(i);
        if (wire->linkedComponent == _components[componentIndex])
        {
            _outToOutWires.RemoveWire(i);
        }
    }
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::_RemoveComponent(int componentIndex)
{
    _DisconnectComponent(componentIndex);

    // set the removed component's parent circuit to NULL
    if (_components[componentIndex]->_GetParentCircuit() != NULL)
    {
        _components[componentIndex]->_SetParentCircuit(NULL);
    }
    // setting a component's parent to NULL (above) calls _RemoveComponent (hence the following code will run)
    else if (_components.size() != 0)
    {
        _components.erase(_components.begin() + componentIndex);
    }
}

//=================================================================================================
