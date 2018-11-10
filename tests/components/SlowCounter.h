#pragma once

#include <thread>

namespace DSPatch
{

class SlowCounter : public Component
{
public:
    SlowCounter()
        : _count( 0 )
    {
        SetOutputCount_( 1 );
    }

    void ResetCount()
    {
        _count = 0;
    }

protected:
    virtual void Process_( SignalBus const&, SignalBus& outputs ) override
    {
        auto start = std::chrono::high_resolution_clock::now();

        outputs.SetValue( 0, _count++ );

        std::chrono::duration<double, std::micro> elapsedMs;
        do
        {
            elapsedMs = std::chrono::high_resolution_clock::now() - start;
        } while ( elapsedMs.count() < _waitMs );

        _waitMs = 1000 - ( elapsedMs.count() - _waitMs );
    }

private:
    int _count;
    int _waitMs = 1000;
};

}  // namespace DSPatch
