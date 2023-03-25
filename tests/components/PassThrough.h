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
    virtual void Process_( SignalBus& inputs, SignalBus& outputs ) override
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto in = inputs.GetValue<int>( 0 );
        if ( in )
        {
            outputs.MoveSignal( 0, inputs.GetSignal( 0 ) );  // pass the signal through (no copy)
        }
        // else set no output

        std::chrono::duration<double, std::micro> elapsedMs;
        do
        {
            elapsedMs = std::chrono::high_resolution_clock::now() - start;
        } while ( elapsedMs.count() < 1.0 );
    }
};

}  // namespace DSPatch
