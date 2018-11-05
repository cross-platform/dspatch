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
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

        outputs.SetValue( 0, _count++ );
    }

private:
    int _count;
};

}  // namespace DSPatch
