#pragma once

namespace DSPatch
{

class ThreadingProbe : public Component
{
public:
    explicit ThreadingProbe( int inputCount = 4 )
        : _inputCount( inputCount )
        , _count( 0 )
    {
        SetInputCount_( _inputCount );
    }

    int GetCount() const
    {
        return _count;
    }

    void ResetCount()
    {
        _count = 0;
    }

protected:
    virtual void Process_( SignalBus& inputs, SignalBus& ) override
    {
        for ( int i = 0; i < _inputCount; ++i )
        {
            auto in = inputs.GetValue<int>( i );
            REQUIRE( in );
            REQUIRE( *in == _count );
        }

        ++_count;
    }

private:
    const int _inputCount;
    int _count;
};

}  // namespace DSPatch
