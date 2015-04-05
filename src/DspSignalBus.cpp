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

#include <dspatch/DspSignalBus.h>

//=================================================================================================

DspSignalBus::~DspSignalBus()
{
}

//=================================================================================================

bool DspSignalBus::SetSignal(int signalIndex, DspSignal const* newSignal)
{
    if ((size_t)signalIndex < _signals.size() && newSignal != NULL)
    {
        return _signals[signalIndex].SetSignal(newSignal);
    }
    else
    {
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

bool DspSignalBus::SetSignal(std::string const& signalName, DspSignal const* newSignal)
{
    int signalIndex;

    if (FindSignal(signalName, signalIndex) && newSignal != NULL)
    {
        return _signals[signalIndex].SetSignal(newSignal);
    }
    else
    {
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

DspSignal* DspSignalBus::GetSignal(int signalIndex)
{
    if ((size_t)signalIndex < _signals.size())
    {
        return &_signals[signalIndex];
    }
    else
    {
        return NULL;
    }
}

//-------------------------------------------------------------------------------------------------

DspSignal* DspSignalBus::GetSignal(std::string const& signalName)
{
    int signalIndex;

    if (FindSignal(signalName, signalIndex))
    {
        return &_signals[signalIndex];
    }
    else
    {
        return NULL;
    }
}

//-------------------------------------------------------------------------------------------------

bool DspSignalBus::FindSignal(std::string const& signalName, int& returnIndex) const
{
    if (signalName == "")
    {
        return false;
    }

    for (size_t i = 0; i < _signals.size(); i++)
    {
        if (_signals[i].GetSignalName() == signalName)
        {
            returnIndex = i;
            return true;
        }
    }
    // if you get here the variable was not found.
    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspSignalBus::FindSignal(int signalIndex, int& returnIndex) const
{
    if ((size_t)signalIndex < _signals.size())
    {
        returnIndex = signalIndex;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

int DspSignalBus::GetSignalCount() const
{
    return _signals.size();
}

//-------------------------------------------------------------------------------------------------

void DspSignalBus::ClearValue(int signalIndex)
{
    if ((size_t)signalIndex < _signals.size())
    {
        return _signals[signalIndex].ClearValue();
    }
}

//-------------------------------------------------------------------------------------------------

void DspSignalBus::ClearValue(std::string const& signalName)
{
    int signalIndex;

    if (FindSignal(signalName, signalIndex))
    {
        return _signals[signalIndex].ClearValue();
    }
}

//-------------------------------------------------------------------------------------------------

void DspSignalBus::ClearAllValues()
{
    for (size_t i = 0; i < _signals.size(); i++)
    {
        _signals[i].ClearValue();
    }
}

//=================================================================================================

bool DspSignalBus::_AddSignal(std::string const& signalName)
{
    if (signalName != "")
    {
        int signalIndex;
        if (FindSignal(signalName, signalIndex))  // if the name already exists
        {
            return false;
        }
    }

    _signals.push_back(DspSignal(signalName));

    return true;
}

//-------------------------------------------------------------------------------------------------

bool DspSignalBus::_RemoveSignal()
{
    if (_signals.size() > 0)
    {
        _signals.pop_back();
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------

void DspSignalBus::_RemoveAllSignals()
{
    _signals.clear();
}

//=================================================================================================
