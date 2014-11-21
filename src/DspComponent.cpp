/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2014 Marcus Tomlinson

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

#include <DSPatch.h>

#include <dspatch/DspCircuit.h>
#include <dspatch/DspComponent.h>
#include <dspatch/DspComponentThread.h>
#include <dspatch/DspWire.h>

//=================================================================================================

DspComponent::DspComponent()
: _parentCircuit( NULL ),
  _bufferCount( 0 ),
  _componentName( "" ),
  _isAutoTickRunning( false ),
  _isAutoTickPaused( false ),
  _hasTicked( false ),
  _callback( NULL ),
  _userData( NULL )
{
  _componentThread.Initialise( this );
}

//-------------------------------------------------------------------------------------------------

DspComponent::~DspComponent()
{
  if( _parentCircuit != NULL )
  {
    _parentCircuit->RemoveComponent( this );
  }

  StopAutoTick();
  _SetBufferCount( 0 );
  DisconnectAllInputs();
}

//=================================================================================================

void DspComponent::SetCallback( Callback_t const& callback, void* userData )
{
  PauseAutoTick();
  _callback = callback;
  _userData = userData;
  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

void DspComponent::SetComponentName( std::string const& componentName )
{
  _componentName = componentName;
}

//-------------------------------------------------------------------------------------------------

std::string DspComponent::GetComponentName() const
{
  return _componentName;
}

//-------------------------------------------------------------------------------------------------

void DspComponent::DisconnectInput( unsigned short inputIndex )
{
  PauseAutoTick();

  // remove inputComponent from _inputWires
  for( unsigned short i = 0; i < _inputWires.GetWireCount(); i++ )
  {
    DspWire* wire = _inputWires.GetWire( i );
    if( wire->toSignalIndex == inputIndex )
    {
      _inputWires.RemoveWire( i );
      break;
    }
  }

  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

void DspComponent::DisconnectInput( std::string const& inputName )
{
  unsigned short inputIndex;

  PauseAutoTick();

  if( _FindInput( inputName, inputIndex ) )
  {
    DisconnectInput( inputIndex );
  }

  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

void DspComponent::DisconnectInput( DspComponent const* inputComponent )
{
  PauseAutoTick();

  // remove inputComponent from _inputWires
  for( unsigned short i = 0; i < _inputWires.GetWireCount(); i++ )
  {
    DspWire* wire = _inputWires.GetWire( i );
    if( wire->linkedComponent == inputComponent )
    {
      _inputWires.RemoveWire( i );
    }
  }

  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

void DspComponent::DisconnectAllInputs()
{
  PauseAutoTick();
  _inputWires.RemoveAllWires();
  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

unsigned short DspComponent::GetInputCount()
{
  PauseAutoTick();
  unsigned short inputCount = GetInputCount_();
  ResumeAutoTick();
  return inputCount;
}

//-------------------------------------------------------------------------------------------------

unsigned short DspComponent::GetOutputCount()
{
  PauseAutoTick();
  unsigned short outputCount = GetOutputCount_();
  ResumeAutoTick();
  return outputCount;
}

//-------------------------------------------------------------------------------------------------

unsigned short DspComponent::GetParameterCount()
{
  PauseAutoTick();
  unsigned short parameterCount = GetParameterCount_();
  ResumeAutoTick();
  return parameterCount;
}

//-------------------------------------------------------------------------------------------------

std::string DspComponent::GetInputName( unsigned short index )
{
  std::string inputName;

  PauseAutoTick();

  if( index < _inputBus.GetSignalCount() )
  {
    inputName = _inputBus.GetSignal( index )->GetSignalName();
  }

  ResumeAutoTick();
  return inputName;
}

//-------------------------------------------------------------------------------------------------

std::string DspComponent::GetOutputName( unsigned short index )
{
  std::string outputName;

  PauseAutoTick();

  if( index < _outputBus.GetSignalCount() )
  {
    outputName = _outputBus.GetSignal( index )->GetSignalName();
  }

  ResumeAutoTick();
  return outputName;
}

//-------------------------------------------------------------------------------------------------

std::string DspComponent::GetParameterName( unsigned short index )
{
  std::string parameterName;

  PauseAutoTick();

  if( index < _parameters.size() )
  {
    std::map< std::string, DspParameter >::const_iterator it = _parameters.begin();
    std::advance( it, index );
    parameterName = it->first;
  }

  ResumeAutoTick();
  return parameterName;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::GetParameter( std::string const& paramName, DspParameter& param )
{
  bool result = false;
  PauseAutoTick();

  DspParameter const* paramPtr = GetParameter_( paramName );
  if( paramPtr )
  {
    result = param.SetParam( *paramPtr );
  }

  ResumeAutoTick();
  return result;
}

//-------------------------------------------------------------------------------------------------

DspParameter const* DspComponent::GetParameter( std::string const& paramName )
{
    PauseAutoTick();
    DspParameter const* result = GetParameter_( paramName );
    ResumeAutoTick();
    return result;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::SetParameter( std::string const& paramName, DspParameter const& param )
{
  PauseAutoTick();
  bool result = ParameterUpdating_( paramName, param );
  ResumeAutoTick();
  return result;
}

//-------------------------------------------------------------------------------------------------

void DspComponent::Tick()
{
  // continue only if this component has not already been ticked
  if( !_hasTicked )
  {
    // 1. set _hasTicked flag
    _hasTicked = true;

    // 2. get outputs required from input components
    for( unsigned short i = 0; i < _inputWires.GetWireCount(); i++ )
    {
      DspWire* wire = _inputWires.GetWire( i );
      wire->linkedComponent->Tick();

      DspSignal* signal = wire->linkedComponent->_outputBus.GetSignal( wire->fromSignalIndex );
      _inputBus.SetSignal( wire->toSignalIndex, signal );
    }

    // 3. clear all outputs
    _outputBus.ClearAllValues();

    // 4. call Process_() with newly aquired inputs
    Process_( _inputBus, _outputBus );
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponent::Reset()
{
  // clear all inputs
  _inputBus.ClearAllValues();

  // reset _hasTicked flag
  _hasTicked = false;
}

//-------------------------------------------------------------------------------------------------

void DspComponent::StartAutoTick()
{
  // Global scoped components (components not within a circuit) are added to the "global circuit" in
  // order to be auto-ticked. Technically it is only the global circuit that auto-ticks -This in turn
  // auto-ticks all components contained.

  // if this is the global circuit
  if( DSPatch::_IsThisGlobalCircuit( this ) )
  {
    if( _componentThread.IsStopped() )
    {
      _componentThread.Start();

      _isAutoTickRunning = true;
      _isAutoTickPaused = false;
    }
    else
    {
      ResumeAutoTick();
    }
  }
  // else if this component has no parent or it's parent is the global circuit
  else if( _parentCircuit == NULL || DSPatch::_IsThisGlobalCircuit( _parentCircuit ) )
  {
    DSPatch::_AddGlobalComponent( this );
    DSPatch::_StartGlobalAutoTick();
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponent::StopAutoTick()
{
  // If a component is part of the global circuit, a call to StopAutoTick() removes it from the
  // global circuit as to stop it from being auto-ticked. When all components are removed, the
  // global circuit auto-ticking is stopped.

  // if this is the global circuit
  if( DSPatch::_IsThisGlobalCircuit( this ) && !_componentThread.IsStopped() )
  {
    _componentThread.Stop();

    _isAutoTickRunning = false;
    _isAutoTickPaused = false;
  }
  // else if this component's parent is the global circuit
  else if( DSPatch::_IsThisGlobalCircuit( _parentCircuit ) )
  {
    DSPatch::_RemoveGlobalComponent( this );

    if( DSPatch::_GetGlobalComponentCount() == 0 )
    {
      DSPatch::_StopGlobalAutoTick();
    }
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponent::PauseAutoTick()
{
  // A call to PauseAutoTick() recursively traverses it's parent circuits until it reaches the
  // global circuit. When the global circuit is reached, it's auto-tick is paused.

  // if this is the global circuit
  if( DSPatch::_IsThisGlobalCircuit( this ) && !_componentThread.IsStopped() )
  {
    if( _isAutoTickRunning )
    {
      _componentThread.Pause();
      _isAutoTickPaused = true;
      _isAutoTickRunning = false;
    }
  }
  else if( _parentCircuit != NULL )
  {
    _parentCircuit->PauseAutoTick(); // recursive call to find the global circuit
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponent::ResumeAutoTick()
{
  // A call to ResumeAutoTick() recursively traverses it's parent circuits until it reaches the
  // global circuit. When the global circuit is reached, it's auto-tick is resumed.

  // if this is the global circuit
  if( DSPatch::_IsThisGlobalCircuit( this ) && _isAutoTickPaused )
  {
    _componentThread.Resume();
    _isAutoTickPaused = false;
    _isAutoTickRunning = true;
  }
  else if( _parentCircuit != NULL )
  {
    _parentCircuit->ResumeAutoTick(); // recursive call to find the global circuit
  }
}

//=================================================================================================

bool DspComponent::AddInput_( std::string const& inputName )
{
  for( unsigned short i = 0; i < _inputBuses.size(); i++ )
  {
    _inputBuses[i]._AddSignal( inputName );
  }
  if( _inputBus._AddSignal( inputName ) )
  {
    if( _callback )
    {
      _callback( this, InputAdded, _inputBus.GetSignalCount() - 1, _userData );
    }
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::AddOutput_( std::string const& outputName )
{
  for( unsigned short i = 0; i < _outputBuses.size(); i++ )
  {
    _outputBuses[i]._AddSignal( outputName );
  }
  if( _outputBus._AddSignal( outputName ) )
  {
    if( _callback )
    {
      _callback( this, OutputAdded, _outputBus.GetSignalCount() - 1, _userData );
    }
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::AddParameter_( std::string const& paramName, DspParameter const& param )
{
  if( _parameters.find( paramName ) == _parameters.end() )
  {
    _parameters.insert( std::make_pair( paramName, param ) );
    if( _callback )
    {
      _callback( this, ParameterAdded, _parameters.size() - 1, _userData );
    }
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::RemoveInput_()
{
  if( _inputBus._RemoveSignal() )
  {
    if( _callback )
    {
      _callback( this, InputRemoved, _inputBus.GetSignalCount(), _userData );
    }
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::RemoveOutput_()
{
  if( _outputBus._RemoveSignal() )
  {
    if( _callback )
    {
      _callback( this, OutputRemoved, _outputBus.GetSignalCount(), _userData );
    }
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::RemoveParameter_()
{
  if( !_parameters.empty() )
  {
    _parameters.erase( _parameters.rbegin()->first );
    if( _callback )
    {
      _callback( this, ParameterRemoved, _parameters.size(), _userData );
    }
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------------------

void DspComponent::RemoveAllInputs_()
{
  for( unsigned short i = 0; i < _inputBuses.size(); i++ )
  {
    _inputBuses[i]._RemoveAllSignals();
  }
  _inputBus._RemoveAllSignals();
  if( _callback )
  {
    _callback( this, InputRemoved, -1, _userData );
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponent::RemoveAllOutputs_()
{
  for( unsigned short i = 0; i < _outputBuses.size(); i++ )
  {
    _outputBuses[i]._RemoveAllSignals();
  }
  _outputBus._RemoveAllSignals();
  if( _callback )
  {
    _callback( this, OutputRemoved, -1, _userData );
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponent::RemoveAllParameters_()
{
  _parameters.clear();
  if( _callback )
  {
    _callback( this, ParameterRemoved, -1, _userData );
  }
}

//-------------------------------------------------------------------------------------------------

unsigned short DspComponent::GetInputCount_()
{
  return _inputBus.GetSignalCount();
}

//-------------------------------------------------------------------------------------------------

unsigned short DspComponent::GetOutputCount_()
{
  return _outputBus.GetSignalCount();
}

//-------------------------------------------------------------------------------------------------

unsigned short DspComponent::GetParameterCount_()
{
  return _parameters.size();
}

//-------------------------------------------------------------------------------------------------

DspParameter const* DspComponent::GetParameter_( std::string const& paramName ) const
{
  std::map< std::string, DspParameter >::const_iterator it = _parameters.find( paramName );
  if( it != _parameters.end() )
  {
    return &it->second;
  }
  return NULL;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::SetParameter_( std::string const& paramName, DspParameter const& param )
{
  std::map< std::string, DspParameter >::iterator it = _parameters.find( paramName );
  if( it != _parameters.end() )
  {
    if( it->second.SetParam( param ) )
    {
      if( _callback )
      {
        _callback( this, ParameterUpdated, std::distance( _parameters.begin(), _parameters.find( paramName ) ), _userData );
      }
      return true;
    }
  }
  return false;
}

//=================================================================================================

void DspComponent::_SetParentCircuit( DspCircuit* parentCircuit )
{
  if( _parentCircuit != parentCircuit && parentCircuit != this )
  {
    DspCircuit* currentParent = _parentCircuit;
    _parentCircuit = NULL;

    // if this component is part of another circuit, remove it from that circuit first
    if( currentParent != NULL )
    {
      currentParent->RemoveComponent( this );
    }

    _parentCircuit = parentCircuit;

    // this method is called from within AddComponent() so don't call AddComponent() here
  }
}

//-------------------------------------------------------------------------------------------------

DspCircuit* DspComponent::_GetParentCircuit()
{
  return _parentCircuit;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::_FindInput( std::string const& signalName, unsigned short& returnIndex ) const
{
  return _inputBus.FindSignal( signalName, returnIndex );
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::_FindInput( unsigned short signalIndex, unsigned short& returnIndex ) const
{
  if( signalIndex < _inputBus.GetSignalCount() )
  {
    returnIndex = signalIndex;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::_FindOutput( std::string const& signalName, unsigned short& returnIndex ) const
{
  return _outputBus.FindSignal( signalName, returnIndex );
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::_FindOutput( unsigned short signalIndex, unsigned short& returnIndex ) const
{
  if( signalIndex < _outputBus.GetSignalCount() )
  {
    returnIndex = signalIndex;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

void DspComponent::_SetBufferCount( unsigned short bufferCount )
{
  // _bufferCount is the current thread count / bufferCount is new thread count

  // delete excess _hasTickeds (if new buffer count is less than current)
  for( long i = _bufferCount - 1; i >= ( long ) bufferCount; i-- )
  {
    delete _hasTickeds[i];
  }

  // resize local buffer array
  _hasTickeds.resize( bufferCount );

  // create excess _hasTickeds (if new buffer count is more than current)
  for( unsigned short i = _bufferCount; i < bufferCount; i++ )
  {
    _hasTickeds[i] = new bool();
  }

  _inputBuses.resize( bufferCount );
  _outputBuses.resize( bufferCount );

  _gotReleases.resize( bufferCount );
  _releaseMutexes.resize( bufferCount );
  _releaseCondts.resize( bufferCount );

  for( unsigned short i = _bufferCount; i < bufferCount; i++ )
  {
    *_hasTickeds[i] = false;
    _gotReleases[i] = false;

    for( unsigned short j = 0; j < _inputBus.GetSignalCount(); j++ )
    {
      _inputBuses[i]._AddSignal( _inputBus.GetSignal( j )->GetSignalName() );
    }

    for( unsigned short j = 0; j < _outputBus.GetSignalCount(); j++ )
    {
      _outputBuses[i]._AddSignal( _outputBus.GetSignal( j )->GetSignalName() );
    }
  }

  if( bufferCount > 0 )
  {
    _gotReleases[0] = true;
  }

  _bufferCount = bufferCount;
}

//-------------------------------------------------------------------------------------------------

unsigned short DspComponent::_GetBufferCount() const
{
  return _bufferCount;
}

//-------------------------------------------------------------------------------------------------

void DspComponent::_ThreadTick( unsigned short threadNo )
{
  // continue only if this component has not already been ticked
  if( *_hasTickeds[threadNo] == false )
  {
    // 1. set _hasTicked flag
    *_hasTickeds[threadNo] = true;

    // 2. get outputs required from input components
    for( unsigned short i = 0; i < _inputWires.GetWireCount(); i++ )
    {
      DspWire* wire = _inputWires.GetWire( i );
      wire->linkedComponent->_ThreadTick( threadNo );

      DspSignal* signal = wire->linkedComponent->_outputBuses[threadNo].GetSignal( wire->fromSignalIndex );
      _inputBuses[threadNo].SetSignal( wire->toSignalIndex, signal );
    }

    // 3. clear all outputs
    _outputBuses[threadNo].ClearAllValues();

    // 4. wait for your turn to process.
    _WaitForRelease( threadNo );

    // 5. call Process_() with newly aquired inputs
    Process_( _inputBuses[threadNo], _outputBuses[threadNo] );

    // 6. signal that you're done processing.
    _ReleaseThread( threadNo );
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponent::_ThreadReset( unsigned short threadNo )
{
  // clear all inputs
  _inputBuses[threadNo].ClearAllValues();

  // reset _hasTicked flag
  *_hasTickeds[threadNo] = false;
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::_SetInputSignal( unsigned short inputIndex, DspSignal const* newSignal )
{
  return _inputBus.SetSignal( inputIndex, newSignal );
}

//-------------------------------------------------------------------------------------------------

bool DspComponent::_SetInputSignal( unsigned short inputIndex, unsigned short threadIndex, DspSignal const* newSignal )
{
  return _inputBuses[threadIndex].SetSignal( inputIndex, newSignal );
}

//-------------------------------------------------------------------------------------------------

DspSignal* DspComponent::_GetOutputSignal( unsigned short outputIndex )
{
  return _outputBus.GetSignal( outputIndex );
}

//-------------------------------------------------------------------------------------------------

DspSignal* DspComponent::_GetOutputSignal( unsigned short outputIndex, unsigned short threadIndex )
{
  return _outputBuses[threadIndex].GetSignal( outputIndex );
}

//-------------------------------------------------------------------------------------------------

void DspComponent::_WaitForRelease( unsigned short threadNo )
{
  _releaseMutexes[threadNo].Lock();
  if( !_gotReleases[threadNo] )
  {
    _releaseCondts[threadNo].Wait( _releaseMutexes[threadNo] ); // wait for resume
  }
  _gotReleases[threadNo] = false; // reset the release flag
  _releaseMutexes[threadNo].Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspComponent::_ReleaseThread( unsigned short threadNo )
{
  unsigned short nextThread = threadNo + 1;

  if( nextThread >= _bufferCount )
    nextThread = 0;

  _releaseMutexes[nextThread].Lock();
  _gotReleases[nextThread] = true;
  _releaseCondts[nextThread].WakeAll();
  _releaseMutexes[nextThread].Unlock();
}

//=================================================================================================
