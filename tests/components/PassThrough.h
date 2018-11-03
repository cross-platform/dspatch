#pragma once

namespace DSPatch
{

class PassThrough : public Component
{
public:
    PassThrough()
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
            outputs.MoveSignal( 0, inputs.GetSignal( 0 ) );  // pass the signal through (no copy)
        }
        // else set no output
    }
};

}  // namespace DSPatch
