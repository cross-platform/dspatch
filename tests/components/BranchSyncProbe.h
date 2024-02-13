/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2024, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

namespace DSPatch
{

class BranchSyncProbe final : public Component
{
public:
    BranchSyncProbe( int p1, int p2, int p3 )
        : _p1( p1 )
        , _p2( p2 )
        , _p3( p3 )
        , _count( 0 )
    {
        SetInputCount_( 3 );
    }

protected:
    void Process_( SignalBus& inputs, SignalBus& ) override
    {
        auto in0 = inputs.GetValue<int>( 0 );
        REQUIRE( in0 );

        auto in1 = inputs.GetValue<int>( 1 );
        REQUIRE( in1 );

        auto in2 = inputs.GetValue<int>( 2 );
        REQUIRE( in2 );

        REQUIRE( *in0 == _p1 + _count );
        REQUIRE( *in1 == _p2 + _count );
        REQUIRE( *in2 == _p3 + _count++ );
    }

private:
    int _p1;
    int _p2;
    int _p3;
    int _count;
};

}  // namespace DSPatch
