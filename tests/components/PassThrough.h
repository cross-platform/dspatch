#pragma once

namespace DSPatch
{

class PassThrough : public Component
{
public:
    PassThrough()
        : Component( ProcessOrder::OutOfOrder )
    {
        SetInputCount_( 1 );
        SetOutputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus& inputs, SignalBus& outputs ) override
    {
        outputs.MoveSignal( 0, *inputs.GetSignal( 0 ) );  // pass the signal through (no copy)
    }
};

}  // namespace DSPatch
