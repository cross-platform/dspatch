#pragma once

namespace DSPatch
{

class Counter : public Component
{
public:
    explicit Counter( int increment = 1 )
        : _count( 0 )
        , _increment( increment )
    {
        SetOutputCount_( 1 );
    }

    int Count() const
    {
        return _count;
    }

protected:
    virtual void Process_( SignalBus&, SignalBus& outputs ) override
    {
        outputs.SetValue( 0, _count );
        _count += _increment;
    }

private:
    int _count;
    int _increment;
};

}  // namespace DSPatch
