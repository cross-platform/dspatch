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

#include <dspatch/DspParameter.h>

//=================================================================================================

DspParameter::DspParameter()
    : _type(Null)
    , _isSet(false)
    , _isRangeSet(false)
{
}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter(ParamType const& type)
    : _type(type)
    , _isSet(false)
    , _isRangeSet(false)
{
}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter(ParamType const& type, int const& initValue, std::pair<int, int> const& valueRange)
    : _type(type)
    , _isSet(false)
    , _isRangeSet(false)
{
    if (type == Bool)
    {
        if (!SetBool(initValue != 0))
        {
            _type = Null;
        }
    }
    else if (type == Float)
    {
        if (!SetFloatRange(std::make_pair((float)valueRange.first, (float)valueRange.second)) ||
            !SetFloat((float)initValue))
        {
            _type = Null;
        }
    }
    else
    {
        if (!SetIntRange(valueRange) || !SetInt(initValue))
        {
            _type = Null;
        }
    }
}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter(ParamType const& type, float const& initValue, std::pair<float, float> const& valueRange)
    : _type(type)
    , _isSet(false)
    , _isRangeSet(false)
{
    if (!SetFloatRange(valueRange) || !SetFloat(initValue))
    {
        _type = Null;
    }
}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter(ParamType const& type, std::string const& initValue)
    : _type(type)
    , _isSet(false)
    , _isRangeSet(false)
{
    if (!SetString(initValue))
    {
        _type = Null;
    }
}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter(ParamType const& type, std::vector<std::string> const& initValue)
    : _type(type)
    , _isSet(false)
    , _isRangeSet(false)
{
    if (!SetList(initValue))
    {
        _type = Null;
    }
}

//=================================================================================================

DspParameter::ParamType DspParameter::Type() const
{
    return _type;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::IsSet() const
{
    return _isSet;
}

//-------------------------------------------------------------------------------------------------

bool const* DspParameter::GetBool() const
{
    if (!_isSet)
    {
        return NULL;
    }

    if (_type == Bool)
    {
        return &_boolValue;
    }

    return NULL;
}

//-------------------------------------------------------------------------------------------------

int const* DspParameter::GetInt() const
{
    if (!_isSet)
    {
        return NULL;
    }

    if (_type == Int || _type == List)
    {
        return &_intValue;
    }

    return NULL;
}

//-------------------------------------------------------------------------------------------------

std::pair<int, int> const* DspParameter::GetIntRange() const
{
    if (!_isRangeSet)
    {
        return NULL;
    }

    if (_type == Int || _type == List)
    {
        return &_intRange;
    }

    return NULL;
}

//-------------------------------------------------------------------------------------------------

float const* DspParameter::GetFloat() const
{
    if (!_isSet)
    {
        return NULL;
    }

    if (_type == Float)
    {
        return &_floatValue;
    }

    return NULL;
}

//-------------------------------------------------------------------------------------------------

std::pair<float, float> const* DspParameter::GetFloatRange() const
{
    if (!_isRangeSet)
    {
        return NULL;
    }

    if (_type == Float)
    {
        return &_floatRange;
    }

    return NULL;
}

//-------------------------------------------------------------------------------------------------

std::string const* DspParameter::GetString() const
{
    if (!_isSet)
    {
        return NULL;
    }

    if (_type == String || _type == FilePath)
    {
        return &_stringValue;
    }
    else if (_type == List)
    {
        return &_listValue[_intValue];
    }

    return NULL;
}

//-------------------------------------------------------------------------------------------------

std::vector<std::string> const* DspParameter::GetList() const
{
    if (!_isSet)
    {
        return NULL;
    }

    if (_type == List)
    {
        return &_listValue;
    }

    return NULL;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetBool(bool const& value)
{
    if (_type == Bool)
    {
        _boolValue = value;
        _isSet = true;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetInt(int const& value)
{
    if (_type == Int || _type == List)
    {
        if (_isRangeSet)
        {
            _intValue = value < _intRange.first ? _intRange.first : value;
            _intValue = value > _intRange.second ? _intRange.second : value;
        }
        else
        {
            _intValue = value;
        }
        _isSet = true;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetIntRange(std::pair<int, int> const& intRange)
{
    if (_type == Int)
    {
        _intRange = intRange;

        if (intRange.first == intRange.second && intRange.first == -1)
        {
            _isRangeSet = false;
        }
        else
        {
            if (_isSet)
            {
                _intValue = _intValue < intRange.first ? intRange.first : _intValue;
                _intValue = _intValue > intRange.second ? intRange.second : _intValue;
            }
            _isRangeSet = true;
        }
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetFloat(float const& value)
{
    if (_type == Float)
    {
        if (_isRangeSet)
        {
            _floatValue = value < _floatRange.first ? _floatRange.first : value;
            _floatValue = value > _floatRange.second ? _floatRange.second : value;
        }
        else
        {
            _floatValue = value;
        }
        _isSet = true;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetFloatRange(std::pair<float, float> const& floatRange)
{
    if (_type == Float)
    {
        _floatRange = floatRange;

        if (floatRange.first == floatRange.second && floatRange.first == -1)
        {
            _isRangeSet = false;
        }
        else
        {
            if (_isSet)
            {
                _floatValue = _floatValue < floatRange.first ? floatRange.first : _floatValue;
                _floatValue = _floatValue > floatRange.second ? floatRange.second : _floatValue;
            }
            _isRangeSet = true;
        }
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetString(std::string const& value)
{
    if (_type == String || _type == FilePath)
    {
        _stringValue = value;
        _isSet = true;
        return true;
    }
    else if (_type == List)
    {
        for (size_t i = 0; i < _listValue.size(); ++i)
        {
            if (_listValue[i] == value)
            {
                _intValue = i;
                _isSet = true;
                return true;
            }
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetList(std::vector<std::string> const& value)
{
    if (_type == List)
    {
        _listValue = value;

        _intRange.first = 0;
        _intRange.second = value.size() - 1;
        _isRangeSet = true;

        _intValue = 0;
        _isSet = true;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetParam(DspParameter const& param)
{
    if (_type == Null)
    {
        _type = param.Type();
    }

    if (param.Type() == Bool)
    {
        if (param.GetBool())
        {
            return SetBool(*param.GetBool());
        }
    }
    else if (param.Type() == Int)
    {
        if (param.GetIntRange() && param.GetInt())
        {
            return SetIntRange(*param.GetIntRange()) && SetInt(*param.GetInt());
        }
        else if (param.GetInt())
        {
            return SetInt(*param.GetInt());
        }
    }
    else if (param.Type() == Float)
    {
        if (param.GetFloatRange() && param.GetFloat())
        {
            return SetFloatRange(*param.GetFloatRange()) && SetFloat(*param.GetFloat());
        }
        if (param.GetFloat())
        {
            return SetFloat(*param.GetFloat());
        }
    }
    else if (param.Type() == String || param.Type() == FilePath)
    {
        if (param.GetString())
        {
            return SetString(*param.GetString());
        }
    }
    else if (param.Type() == Trigger)
    {
        if (_type == Trigger)
        {
            return true;
        }
    }
    else if (param.Type() == List)
    {
        if (param.GetList())
        {
            return SetList(*param.GetList());
        }
    }

    return false;
}

//=================================================================================================
