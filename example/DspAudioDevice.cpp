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

#include <DspAudioDevice.h>

#include <RtAudio.h>

#include <iostream>
#include <string.h>
#include <cstdlib>

//=================================================================================================

struct RtAudioMembers
{
    std::vector<RtAudio::DeviceInfo> deviceList;

    RtAudio audioStream;
    RtAudio::StreamParameters outputParams;
    RtAudio::StreamParameters inputParams;
};

//=================================================================================================

DspAudioDevice::DspAudioDevice()
    : _rtAudio(new RtAudioMembers())
    , _gotWaitReady(false)
    , _gotSyncReady(true)
{
    _outputChannels.resize(20);
    for (int i = 0; i < 20; i++)
    {
        AddInput_();
    }

    AddInput_("Sample Rate");

    _inputChannels.resize(20);
    for (int i = 0; i < 20; i++)
    {
        AddOutput_();
    }

    std::vector<std::string> deviceNameList;

    for (unsigned int i = 0; i < _rtAudio->audioStream.getDeviceCount(); i++)
    {
        _rtAudio->deviceList.push_back(_rtAudio->audioStream.getDeviceInfo(i));
        deviceNameList.push_back(_rtAudio->audioStream.getDeviceInfo(i).name);
    }

    pDeviceList = AddParameter_("deviceList", DspParameter(DspParameter::List, deviceNameList));
    pIsStreaming = AddParameter_("isStreaming", DspParameter(DspParameter::Bool, false));
    pBufferSize = AddParameter_("bufferSize", DspParameter(DspParameter::Int, 256));
    pSampleRate = AddParameter_("sampleRate", DspParameter(DspParameter::Int, 44100));

    SetDevice(_rtAudio->audioStream.getDefaultOutputDevice());
    SetBufferSize(GetBufferSize());
    SetSampleRate(GetSampleRate());
}

//-------------------------------------------------------------------------------------------------

DspAudioDevice::~DspAudioDevice()
{
    _StopStream();

    delete _rtAudio;
}

//-------------------------------------------------------------------------------------------------

bool DspAudioDevice::SetDevice(int deviceIndex)
{
    if (deviceIndex >= 0 && deviceIndex < GetDeviceCount())
    {
        _StopStream();

        SetParameter_(pDeviceList, DspParameter(DspParameter::Int, deviceIndex));

        _rtAudio->inputParams.nChannels = _rtAudio->deviceList[deviceIndex].inputChannels;
        _rtAudio->inputParams.deviceId = deviceIndex;

        _rtAudio->outputParams.nChannels = _rtAudio->deviceList[deviceIndex].outputChannels;
        _rtAudio->outputParams.deviceId = deviceIndex;

        _StartStream();

        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

std::string DspAudioDevice::GetDeviceName(int deviceIndex) const
{
    if (deviceIndex >= 0 && deviceIndex < GetDeviceCount())
    {
        return _rtAudio->deviceList[deviceIndex].name;
    }

    return "";
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::GetDeviceInputCount(int deviceIndex) const
{
    return _rtAudio->deviceList[deviceIndex].inputChannels;
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::GetDeviceOutputCount(int deviceIndex) const
{
    return _rtAudio->deviceList[deviceIndex].outputChannels;
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::GetCurrentDevice() const
{
    return *GetParameter_(pDeviceList)->GetInt();
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::GetDeviceCount() const
{
    return GetParameter_(pDeviceList)->GetList()->size();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::SetBufferSize(int bufferSize)
{
    _StopStream();

    SetParameter_(pBufferSize, DspParameter(DspParameter::Int, bufferSize));
    for (size_t i = 0; i < _inputChannels.size(); i++)
    {
        _inputChannels[i].resize(bufferSize);
    }

    _StartStream();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::SetSampleRate(int sampleRate)
{
    _StopStream();
    SetParameter_(pSampleRate, DspParameter(DspParameter::Int, sampleRate));
    _StartStream();
}

//-------------------------------------------------------------------------------------------------

bool DspAudioDevice::IsStreaming() const
{
    return *GetParameter_(pIsStreaming)->GetBool();
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::GetBufferSize() const
{
    return *GetParameter_(pBufferSize)->GetInt();
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::GetSampleRate() const
{
    return *GetParameter_(pSampleRate)->GetInt();
}

//=================================================================================================

void DspAudioDevice::Process_(DspSignalBus& inputs, DspSignalBus& outputs)
{
    // Wait until the sound card is ready for the next set of buffers
    // ==============================================================
    _syncMutex.Lock();
    if (!_gotSyncReady)  // if haven't already got the release
    {
        _syncCondt.Wait(_syncMutex);  // wait for sync
    }
    _gotSyncReady = false;  // reset the release flag
    _syncMutex.Unlock();

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
    if (inputs.GetValue(0, _outputChannels[0]))
    {
        if (GetBufferSize() != (int)_outputChannels[0].size() && _outputChannels[0].size() != 0)
        {
            SetBufferSize(_outputChannels[0].size());
        }
    }

    // Retrieve incoming component buffers for the sound card to output
    // ================================================================
    for (size_t i = 0; i < _outputChannels.size(); i++)
    {
        if (!inputs.GetValue(i, _outputChannels[i]))
        {
            _outputChannels[i].assign(_outputChannels[i].size(), 0);
        }
    }

    // Retrieve incoming sound card buffers for the component to output
    // ================================================================
    for (size_t i = 0; i < _inputChannels.size(); i++)
    {
        outputs.SetValue(i, _inputChannels[i]);
    }

    // Inform the sound card that buffers are now ready
    // ================================================
    _buffersMutex.Lock();
    _gotWaitReady = true;  // set release flag
    _waitCondt.WakeAll();  // release sync
    _buffersMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

bool DspAudioDevice::ParameterUpdating_(int index, DspParameter const& param)
{
    if (index == pDeviceList)
    {
        return SetDevice(*param.GetInt());
    }
    else if (index == pBufferSize)
    {
        SetBufferSize(*param.GetInt());
        return true;
    }
    else if (index == pSampleRate)
    {
        SetSampleRate(*param.GetInt());
        return true;
    }

    return false;
}

//=================================================================================================

void DspAudioDevice::_SetIsStreaming(bool isStreaming)
{
    SetParameter_(pIsStreaming, DspParameter(DspParameter::Bool, isStreaming));
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::_WaitForBuffer()
{
    _buffersMutex.Lock();
    if (!_gotWaitReady)  // if haven't already got the release
    {
        _waitCondt.Wait(_buffersMutex);  // wait for sync
    }
    _gotWaitReady = false;  // reset the release flag
    _buffersMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::_SyncBuffer()
{
    _syncMutex.Lock();
    _gotSyncReady = true;  // set release flag
    _syncCondt.WakeAll();  // release sync
    _syncMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::_StopStream()
{
    _SetIsStreaming(false);

    _buffersMutex.Lock();
    _gotWaitReady = true;  // set release flag
    _waitCondt.WakeAll();  // release sync
    _buffersMutex.Unlock();

    if (_rtAudio->audioStream.isStreamOpen())
    {
        _rtAudio->audioStream.closeStream();
    }
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::_StartStream()
{
    RtAudio::StreamParameters* inputParams = NULL;
    RtAudio::StreamParameters* outputParams = NULL;

    if (_rtAudio->inputParams.nChannels != 0)
    {
        inputParams = &_rtAudio->inputParams;
    }

    if (_rtAudio->outputParams.nChannels != 0)
    {
        outputParams = &_rtAudio->outputParams;
    }

    RtAudio::StreamOptions options;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;
    options.flags |= RTAUDIO_NONINTERLEAVED;

    _rtAudio->audioStream.openStream(outputParams,
                                     inputParams,
                                     RTAUDIO_FLOAT32,
                                     GetSampleRate(),
                                     (unsigned int*)const_cast<int*>(GetParameter_(pBufferSize)->GetInt()),
                                     &_StaticCallback,
                                     this,
                                     &options);

    _rtAudio->audioStream.startStream();

    _SetIsStreaming(true);
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::_StaticCallback(
    void* outputBuffer, void* inputBuffer, unsigned int, double, unsigned int, void* userData)
{
    return (reinterpret_cast<DspAudioDevice*>(userData))->_DynamicCallback(inputBuffer, outputBuffer);
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::_DynamicCallback(void* inputBuffer, void* outputBuffer)
{
    _WaitForBuffer();

    if (IsStreaming())
    {
        float* floatOutput = (float*)outputBuffer;
        float* floatInput = (float*)inputBuffer;

        if (outputBuffer != NULL)
        {
            for (size_t i = 0; i < _outputChannels.size(); i++)
            {
                if (_rtAudio->deviceList[GetCurrentDevice()].outputChannels >= (i + 1))
                {
                    for (size_t j = 0; j < _outputChannels[i].size(); j++)
                    {
                        *floatOutput++ = _outputChannels[i][j];
                    }
                }
            }
        }

        if (inputBuffer != NULL)
        {
            for (size_t i = 0; i < _inputChannels.size(); i++)
            {
                if (_rtAudio->deviceList[GetCurrentDevice()].inputChannels >= (i + 1))
                {
                    for (size_t j = 0; j < _inputChannels[i].size(); j++)
                    {
                        _inputChannels[i][j] = *floatInput++;
                    }
                }
            }
        }
    }
    else
    {
        _SyncBuffer();
        return 1;
    }

    _SyncBuffer();
    return 0;
}

//=================================================================================================
