/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2013 Marcus Tomlinson

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

DspCircuit::DspCircuit( unsigned short threadCount )
: _currentThreadIndex( 0 ),
  _inToInWires( true ),
  _outToOutWires( false )
{
  SetThreadCount( threadCount );
}

//-------------------------------------------------------------------------------------------------

DspCircuit::~DspCircuit()
{
  StopAutoTick();
  RemoveAllComponents();
  SetThreadCount( 0 );
}

//=================================================================================================

void DspCircuit::PauseAutoTick()
{
  // pause auto tick
  DspComponent::PauseAutoTick();

  // manually tick until 0
  while( _currentThreadIndex != 0 )
  {
    Tick();
    Reset();
  }

  // sync all threads
  for( unsigned short i = 0; i < _circuitThreads.size(); i++ )
  {
    _circuitThreads[i].Sync();
  }
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::SetThreadCount( unsigned short threadCount )
{
  if( threadCount != _circuitThreads.size() )
  {
    PauseAutoTick();

    // stop all threads
    for( unsigned short i = 0; i < _circuitThreads.size(); i++ )
    {
      _circuitThreads[i].Stop();
    }

    // resize thread array
    _circuitThreads.resize( threadCount );

    // initialise and start all threads
    for( unsigned short i = 0; i < _circuitThreads.size(); i++ )
    {
      _circuitThreads[i].Initialise( &_components, i );
      _circuitThreads[i].Start();
    }

    // set all components to the new thread count
    for( unsigned short i = 0; i < _components.size(); i++ )
    {
      _components[i]->_SetBufferCount( threadCount );
    }

    ResumeAutoTick();
  }
}

//-------------------------------------------------------------------------------------------------

unsigned short DspCircuit::GetThreadCount() const
{
  return _circuitThreads.size();
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddComponent( DspComponent* component, std::string componentName )
{
  if( component != this )
  {
    // if the component has a name already
    if( component->GetComponentName() != "" && componentName == "" )
    {
      componentName = component->GetComponentName();
    }

    unsigned short componentIndex;

    if( component->_GetParentCircuit() != NULL )
    {
      return false; // if the component is already part of another circuit
    }
    if( _FindComponent( component, componentIndex ) )
    {
      return false; // if the component is already in the array
    }
    if( _FindComponent( componentName, componentIndex ) )
    {
      return false; // if the component name is already in the array
    }

    // components within the circuit need to have as many buffers as there are threads in the circuit
    component->_SetParentCircuit( this );
    component->_SetBufferCount( _circuitThreads.size() );
    component->SetComponentName( componentName );

    PauseAutoTick();
    _components.push_back( component );
    ResumeAutoTick();

    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddComponent( DspComponent& component, std::string componentName )
{
  return AddComponent( &component, componentName );
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveComponent( DspComponent* component )
{
  unsigned short componentIndex;

  if( _FindComponent( component, componentIndex ) )
  {
    PauseAutoTick();
    _RemoveComponent( componentIndex );
    ResumeAutoTick();
  }
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveComponent( DspComponent& component )
{
  RemoveComponent( &component );
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveComponent( std::string componentName )
{
  unsigned short componentIndex;

  if( _FindComponent( componentName, componentIndex ) )
  {
    PauseAutoTick();
    _RemoveComponent( componentIndex );
    ResumeAutoTick();
  }
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveAllComponents()
{
  for( unsigned short i = 0; i < _components.size(); i++ )
  {
    PauseAutoTick();
    _RemoveComponent( i-- ); // size drops as one is removed
    ResumeAutoTick();
  }
}

//-------------------------------------------------------------------------------------------------

unsigned short DspCircuit::GetComponentCount() const
{
  return _components.size();
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::DisconnectComponent( std::string component )
{
  unsigned short componentIndex;

  if( !_FindComponent( component, componentIndex ) ) // verify component exists
  {
    return;
  }

  PauseAutoTick();
  _DisconnectComponent( componentIndex );
  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddInput( std::string inputName )
{
  PauseAutoTick();
  bool result = AddInput_( inputName );
  ResumeAutoTick();

  return result;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::AddOutput( std::string outputName )
{
  PauseAutoTick();
  bool result = AddOutput_( outputName );
  ResumeAutoTick();

  return result;
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveInputs()
{
  RemoveInputs_();
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::RemoveOutputs()
{
  RemoveOutputs_();
}

//=================================================================================================

void DspCircuit::Process_( DspSignalBus& inputs, DspSignalBus& outputs )
{
  DspWire* wire;
  DspSignal* signal;

  // process in a single thread if this circuit has no threads
  // =========================================================
  if( _circuitThreads.size() == 0 )
  {
    // set all internal component inputs from connected circuit inputs
    for( unsigned short i = 0; i < _inToInWires.GetWireCount(); i++ )
    {
      wire = _inToInWires.GetWire( i );
      signal = inputs.GetSignal( wire->fromSignalIndex );
      wire->linkedComponent->_SetInputSignal( wire->toSignalIndex, signal );
    }

    // tick all internal components
    for( unsigned short i = 0; i < _components.size(); i++ )
    {
      _components[i]->Tick();
    }

    // reset all internal components
    for( unsigned short i = 0; i < _components.size(); i++ )
    {
      _components[i]->Reset();
    }

    // set all circuit outputs from connected internal component outputs
    for( unsigned short i = 0; i < _outToOutWires.GetWireCount(); i++ )
    {
      wire = _outToOutWires.GetWire( i );
      signal = wire->linkedComponent->_GetOutputSignal( wire->fromSignalIndex );
      outputs.SetSignal( wire->toSignalIndex, signal );
    }
  }
  // process in multiple thread if this circuit has threads
  // ======================================================
  else
  {
    _circuitThreads[_currentThreadIndex].Sync(); // sync with thread x

    // set all circuit outputs from connected internal component outputs
    for( unsigned short i = 0; i < _outToOutWires.GetWireCount(); i++ )
    {
      wire = _outToOutWires.GetWire( i );
      signal = wire->linkedComponent->_GetOutputSignal( wire->fromSignalIndex, _currentThreadIndex );
      outputs.SetSignal( wire->toSignalIndex, signal );
    }

    // set all internal component inputs from connected circuit inputs
    for( unsigned short i = 0; i < _inToInWires.GetWireCount(); i++ )
    {
      wire = _inToInWires.GetWire( i );
      signal = inputs.GetSignal( wire->fromSignalIndex );
      wire->linkedComponent->_SetInputSignal( wire->toSignalIndex, _currentThreadIndex, signal );
    }

    _circuitThreads[_currentThreadIndex].Resume(); // resume thread x

    if( ++_currentThreadIndex >= _circuitThreads.size() ) // shift to thread x+1
    {
      _currentThreadIndex = 0;
    }
  }
}

//=================================================================================================

bool DspCircuit::_FindComponent( DspComponent* component, unsigned short& returnIndex ) const
{
  for( unsigned short i = 0; i < _components.size(); i++ )
  {
    if( _components[i] == component )
    {
      returnIndex = i;
      return true;
    }
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::_FindComponent( DspComponent& component, unsigned short& returnIndex ) const
{
  return _FindComponent( &component, returnIndex );
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::_FindComponent( std::string componentName, unsigned short& returnIndex ) const
{
  for( unsigned short i = 0; i < _components.size(); i++ )
  {
    if( _components[i]->GetComponentName() != "" && _components[i]->GetComponentName() == componentName )
    {
      returnIndex = i;
      return true;
    }
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspCircuit::_FindComponent( unsigned short componentIndex, unsigned short& returnIndex ) const
{
  if( componentIndex < _components.size() )
  {
    returnIndex = componentIndex;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::_DisconnectComponent( unsigned short componentIndex )
{
  // remove component from _inputComponents and _inputWires
  _components[ componentIndex ]->DisconnectInputs();

  // remove component from _inToInWires
  DspWire* wire;
  for( unsigned short i = 0; i < _inToInWires.GetWireCount(); i++ )
  {
    wire = _inToInWires.GetWire( i );
    if( wire->linkedComponent == _components[ componentIndex ] )
    {
      _inToInWires.RemoveWire( i );
    }
  }

  // remove component from _outToOutWires
  for( unsigned short i = 0; i < _outToOutWires.GetWireCount(); i++ )
  {
    wire = _outToOutWires.GetWire( i );
    if( wire->linkedComponent == _components[ componentIndex ] )
    {
      _outToOutWires.RemoveWire( i );
    }
  }
}

//-------------------------------------------------------------------------------------------------

void DspCircuit::_RemoveComponent( unsigned short componentIndex )
{
  _DisconnectComponent( componentIndex );

  // set the removed component's parent circuit to NULL
  if( _components[componentIndex]->_GetParentCircuit() != NULL )
  {
    _components[componentIndex]->_SetParentCircuit( NULL );
  }
  // setting a component's parent to NULL (above) calls _RemoveComponent (hence the following code will run)
  else if( _components.size() != 0 )
  {
    for( unsigned short i = componentIndex; i < _components.size() - 1; i++ )
    {
      _components[i] = _components[i + 1]; // shift all lower elements up
    }

    _components.pop_back(); // remove end item
  }
}

//=================================================================================================
