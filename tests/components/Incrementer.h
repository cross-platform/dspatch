#pragma once

namespace DSPatch
{

class Incrementer : public Component
{
public:
    Incrementer( int increment = 1 )
        : _increment( increment )
    {
        SetInputCount_( 1 );
        SetOutputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override
    {
        auto in = inputs.GetValue<int>( 0 );
        if ( in )
        {
            *in += _increment;
            outputs.SetValue( 0, inputs, 0 );  // pass the adjusted signal through (no copy)
        }
        // else set no output
    }

private:
    int _increment;
};

}  // namespace DSPatch
