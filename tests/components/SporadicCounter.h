#pragma once

namespace DSPatch
{

class SporadicCounter : public Component
{
public:
    SporadicCounter( int increment = 1 )
        : _count( 0 )
        , _increment( increment )
    {
        srand( (unsigned int)time( nullptr ) );

        SetOutputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const&, SignalBus& outputs ) override
    {
        if ( rand() % 2 == 1 )
        {
            outputs.SetValue( 0, _count );
            _count += _increment;
        }
    }

private:
    int _count;
    int _increment;
};

}  // namespace DSPatch
