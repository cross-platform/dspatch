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

#include <DspOscillator.h>

#include <math.h>

//=================================================================================================

static const float TWOPI = 6.283185307179586476925286766559f;

//=================================================================================================

DspOscillator::DspOscillator(float startFreq, float startAmpl)
    : _lastPos(0)
    , _lookupLength(0)
{
    AddInput_("Sample Rate");
    AddInput_("Buffer Size");

    AddOutput_();

    pBufferSize = AddParameter_("bufferSize", DspParameter(DspParameter::Int, 256));
    pSampleRate = AddParameter_("sampleRate", DspParameter(DspParameter::Int, 44100));
    pAmplitude = AddParameter_("amplitude", DspParameter(DspParameter::Float, startAmpl));
    pFrequency = AddParameter_("frequency", DspParameter(DspParameter::Float, startFreq));

    _BuildLookup();
}

//-------------------------------------------------------------------------------------------------

DspOscillator::~DspOscillator()
{
}

//=================================================================================================

void DspOscillator::SetBufferSize(int bufferSize)
{
    SetParameter_(pBufferSize, DspParameter(DspParameter::Int, bufferSize));

    _processMutex.Lock();
    _BuildLookup();
    _processMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspOscillator::SetSampleRate(int sampleRate)
{
    SetParameter_(pSampleRate, DspParameter(DspParameter::Int, sampleRate));

    _processMutex.Lock();
    _BuildLookup();
    _processMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspOscillator::SetAmpl(float ampl)
{
    SetParameter_(pAmplitude, DspParameter(DspParameter::Float, ampl));

    _processMutex.Lock();
    _BuildLookup();
    _processMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspOscillator::SetFreq(float freq)
{
    SetParameter_(pFrequency, DspParameter(DspParameter::Float, freq));

    _processMutex.Lock();
    _BuildLookup();
    _processMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

int DspOscillator::GetBufferSize() const
{
    return *GetParameter_(pBufferSize)->GetInt();
}

//-------------------------------------------------------------------------------------------------

int DspOscillator::GetSampleRate() const
{
    return *GetParameter_(pSampleRate)->GetInt();
}

//-------------------------------------------------------------------------------------------------

float DspOscillator::GetAmpl() const
{
    return *GetParameter_(pAmplitude)->GetFloat();
}

//-------------------------------------------------------------------------------------------------

float DspOscillator::GetFreq() const
{
    return *GetParameter_(pFrequency)->GetFloat();
}

//=================================================================================================

void DspOscillator::Process_(DspSignalBus& inputs, DspSignalBus& outputs)
{
    // Synchronise sample rate with the "Sample Rate" input feed
    // =========================================================
    int sampleRate;
    if (inputs.GetValue("Sample Rate", sampleRate))
    {
        if (sampleRate != GetSampleRate())
        {
            SetSampleRate(sampleRate);
        }
    }

    // Synchronise buffer size with the size of incoming buffers
    // =========================================================
    if (inputs.GetValue("Buffer Size", _signal))
    {
        if (GetBufferSize() != (int)_signal.size())
        {
            SetBufferSize(_signal.size());
        }
    }

    _processMutex.Lock();

    if (_signalLookup.size() != 0)
    {
        for (size_t i = 0; i < _signal.size(); i++)
        {
            if (_lastPos >= _lookupLength)
            {
                _lastPos = 0;
            }
            _signal[i] = _signalLookup[_lastPos++];
        }

        outputs.SetValue(0, _signal);
    }

    _processMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

bool DspOscillator::ParameterUpdating_(int index, DspParameter const& param)
{
    if (index == pBufferSize)
    {
        SetBufferSize(*param.GetInt());
        return true;
    }
    else if (index == pSampleRate)
    {
        SetSampleRate(*param.GetInt());
        return true;
    }
    else if (index == pAmplitude)
    {
        SetAmpl(*param.GetFloat());
        return true;
    }
    else if (index == pFrequency)
    {
        SetFreq(*param.GetFloat());
        return true;
    }

    return false;
}

//=================================================================================================

void DspOscillator::_BuildLookup()
{
    float posFrac = _lookupLength <= 0 ? 0 : (float)_lastPos / (float)_lookupLength;
    float angleInc = TWOPI * GetFreq() / GetSampleRate();

    _lookupLength = (int)((float)GetSampleRate() / GetFreq());

    _signal.resize(GetBufferSize());
    _signalLookup.resize(_lookupLength);

    for (int i = 0; i < _lookupLength; i++)
    {
        _signalLookup[i] = sin(angleInc * i) * GetAmpl();
    }

    _lastPos = (int)(posFrac * (float)_lookupLength + 0.5f);  // calculate new position (round up)
}

//=================================================================================================
