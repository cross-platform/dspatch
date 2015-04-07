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

#ifndef DSPOSCILLATOR_H
#define DSPOSCILLATOR_H

#include <DSPatch.h>

//=================================================================================================

class DspOscillator : public DspComponent
{
public:
    int pBufferSize;  // Int
    int pSampleRate;  // Int
    int pAmplitude;   // Float
    int pFrequency;   // Float

    DspOscillator(float startFreq = 1000.0, float startAmpl = 1.0);
    ~DspOscillator();

    void SetBufferSize(int bufferSize);
    void SetSampleRate(int sampleRate);
    void SetAmpl(float ampl);
    void SetFreq(float freq);

    int GetBufferSize() const;
    int GetSampleRate() const;
    float GetAmpl() const;
    float GetFreq() const;

protected:
    virtual void Process_(DspSignalBus& inputs, DspSignalBus& outputs);
    virtual bool ParameterUpdating_(int index, DspParameter const& param);

private:
    std::vector<float> _signalLookup;
    std::vector<float> _signal;

    int _lastPos;
    int _lookupLength;

    DspMutex _processMutex;

    void _BuildLookup();
};

//=================================================================================================

class DspOscillatorPlugin : public DspPlugin
{
    std::map<std::string, DspParameter> GetCreateParams() const
    {
        std::map<std::string, DspParameter> params;
        params["startFreq"] = DspParameter(DspParameter::Float);
        params["startAmpl"] = DspParameter(DspParameter::Float, 1.0f, std::make_pair(0.0f, 1.0f));
        return params;
    }

    DspComponent* Create(std::map<std::string, DspParameter>& params) const
    {
        float const* startFreq = params["startFreq"].GetFloat();
        float const* startAmpl = params["startAmpl"].GetFloat();

        if (startFreq && startAmpl)
        {
            return new DspOscillator(*startFreq, *startAmpl);
        }
        else if (startFreq && !startAmpl)
        {
            return new DspOscillator(*startFreq);
        }
        else
        {
            return new DspOscillator();
        }
    }
};

EXPORT_DSPPLUGIN(DspOscillatorPlugin)

//=================================================================================================

#endif /* DSPOSCILLATOR_H */
