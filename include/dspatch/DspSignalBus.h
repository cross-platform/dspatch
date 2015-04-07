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

#ifndef DSPSIGNALBUS_H
#define DSPSIGNALBUS_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspSignal.h>

//=================================================================================================
/// DspSignal container

/**
A DspSignalBus contains DspSignals (see DspSignal). Via the Process_() method, a DspComponent
receives signals into it's "inputs" DspSignalBus and provides signals to it's "outputs"
DspSignalBus. Although DspSignals can be acquired from a DspSignalBus, the DspSignalBus class
provides public getters and setters for manipulating it's internal DspSignal values directly,
abstracting the need to retrieve and interface with the contained DspSignals themself.
*/

class DLLEXPORT DspSignalBus
{
public:
    virtual ~DspSignalBus();

    bool SetSignal(int signalIndex, DspSignal const* newSignal);
    bool SetSignal(std::string const& signalName, DspSignal const* newSignal);

    DspSignal* GetSignal(int signalIndex);
    DspSignal* GetSignal(std::string const& signalName);

    bool FindSignal(std::string const& signalName, int& returnIndex) const;
    bool FindSignal(int signalIndex, int& returnIndex) const;

    int GetSignalCount() const;

    template <class ValueType>
    bool SetValue(int signalIndex, ValueType const& newValue);

    template <class ValueType>
    bool SetValue(std::string const& signalName, ValueType const& newValue);

    template <class ValueType>
    bool GetValue(int signalIndex, ValueType& returnValue) const;

    template <class ValueType>
    bool GetValue(std::string const& signalName, ValueType& returnValue) const;

    template <class ValueType>
    ValueType const* GetValue(int signalIndex) const;

    template <class ValueType>
    ValueType const* GetValue(std::string const& signalName) const;

    void ClearValue(int signalIndex);
    void ClearValue(std::string const& signalName);

    void ClearAllValues();

private:
    bool _AddSignal(std::string const& signalName = "");

    bool _RemoveSignal();
    void _RemoveAllSignals();

private:
    friend class DspComponent;

    std::vector<DspSignal> _signals;
};

//=================================================================================================

template <class ValueType>
bool DspSignalBus::SetValue(int signalIndex, ValueType const& newValue)
{
    if ((size_t)signalIndex < _signals.size())
    {
        return _signals[signalIndex].SetValue(newValue);
    }
    else
    {
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

template <class ValueType>
bool DspSignalBus::SetValue(std::string const& signalName, ValueType const& newValue)
{
    int signalIndex;

    if (FindSignal(signalName, signalIndex))
    {
        return _signals[signalIndex].SetValue(newValue);
    }
    else
    {
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

template <class ValueType>
bool DspSignalBus::GetValue(int signalIndex, ValueType& returnValue) const
{
    if ((size_t)signalIndex < _signals.size())
    {
        return _signals[signalIndex].GetValue(returnValue);
    }
    else
    {
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

template <class ValueType>
bool DspSignalBus::GetValue(std::string const& signalName, ValueType& returnValue) const
{
    int signalIndex;

    if (FindSignal(signalName, signalIndex))
    {
        return _signals[signalIndex].GetValue(returnValue);
    }
    else
    {
        return false;
    }
}

//-------------------------------------------------------------------------------------------------

template <class ValueType>
ValueType const* DspSignalBus::GetValue(int signalIndex) const
{
    if (signalIndex < _signals.size())
    {
        return _signals[signalIndex].GetValue<ValueType>();
    }
    else
    {
        return NULL;
    }
}

//-------------------------------------------------------------------------------------------------

template <class ValueType>
ValueType const* DspSignalBus::GetValue(std::string const& signalName) const
{
    int signalIndex;

    if (FindSignal(signalName, signalIndex))
    {
        return _signals[signalIndex].GetValue<ValueType>();
    }
    else
    {
        return NULL;
    }
}

//=================================================================================================

#endif  // DSPSIGNALBUS_H
