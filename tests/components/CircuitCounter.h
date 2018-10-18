#pragma once

namespace DSPatch
{

class CircuitCounter : public Component
{
public:
    CircuitCounter()
        : _count( 0 )
    {
        SetInputCount_( 1 );
        SetOutputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override
    {
        auto in = inputs.GetValue<bool>( 0 );
        if ( in )
        {
            // Count only when the circuit is closed
            ++_count;
        }

        outputs.SetValue( 0, _count );
    }

private:
    int _count;
};

}  // namespace DSPatch
