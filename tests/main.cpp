#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <DSPatch.h>

#include <components/Adder.h>
#include <components/BranchSyncProbe.h>
#include <components/ChangingCounter.h>
#include <components/ChangingProbe.h>
#include <components/CircuitCounter.h>
#include <components/CircuitProbe.h>
#include <components/Counter.h>
#include <components/FeedbackProbe.h>
#include <components/FeedbackTester.h>
#include <components/Incrementer.h>
#include <components/NoOutputProbe.h>
#include <components/NullInputProbe.h>
#include <components/ParallelProbe.h>
#include <components/PassThrough.h>
#include <components/SerialProbe.h>
#include <components/SlowCounter.h>
#include <components/SporadicCounter.h>
#include <components/ThreadingProbe.h>

#include <thread>

using namespace DSPatch;

TEST_CASE( "SerialTest" )
{
    // Configure a circuit made up of a counter and 5 incrementers in series
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_s1 = std::make_shared<Incrementer>( 1 );
    auto inc_s2 = std::make_shared<Incrementer>( 2 );
    auto inc_s3 = std::make_shared<Incrementer>( 3 );
    auto inc_s4 = std::make_shared<Incrementer>( 4 );
    auto inc_s5 = std::make_shared<Incrementer>( 5 );
    auto probe = std::make_shared<SerialProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( inc_s1 );
    circuit->AddComponent( inc_s2 );
    circuit->AddComponent( inc_s3 );
    circuit->AddComponent( inc_s4 );
    circuit->AddComponent( inc_s5 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, inc_s1, 0 );
    circuit->ConnectOutToIn( inc_s1, 0, inc_s2, 0 );
    circuit->ConnectOutToIn( inc_s2, 0, inc_s3, 0 );
    circuit->ConnectOutToIn( inc_s3, 0, inc_s4, 0 );
    circuit->ConnectOutToIn( inc_s4, 0, inc_s5, 0 );
    circuit->ConnectOutToIn( inc_s5, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Series );
    }
}

TEST_CASE( "ParallelTest" )
{
    // Configure a circuit made up of a counter and 5 incrementers in parallel
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1 = std::make_shared<Incrementer>( 1 );
    auto inc_p2 = std::make_shared<Incrementer>( 2 );
    auto inc_p3 = std::make_shared<Incrementer>( 3 );
    auto inc_p4 = std::make_shared<Incrementer>( 4 );
    auto inc_p5 = std::make_shared<Incrementer>( 5 );
    auto probe = std::make_shared<ParallelProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( inc_p1 );
    circuit->AddComponent( inc_p2 );
    circuit->AddComponent( inc_p3 );
    circuit->AddComponent( inc_p4 );
    circuit->AddComponent( inc_p5 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, inc_p1, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p2, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p3, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p4, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p5, 0 );
    circuit->ConnectOutToIn( inc_p1, 0, probe, 0 );
    circuit->ConnectOutToIn( inc_p2, 0, probe, 1 );
    circuit->ConnectOutToIn( inc_p3, 0, probe, 2 );
    circuit->ConnectOutToIn( inc_p4, 0, probe, 3 );
    circuit->ConnectOutToIn( inc_p5, 0, probe, 4 );

    // Tick the circuit for 100ms with 3 threads
    circuit->SetBufferCount( 3 );
    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->StopAutoTick();
}

TEST_CASE( "BranchSyncTest" )
{
    // Configure a circuit made up of 3 parallel branches of 4, 2, and 1 component(s) respectively
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1_s1 = std::make_shared<Incrementer>();
    auto inc_p1_s2 = std::make_shared<Incrementer>();
    auto inc_p1_s3 = std::make_shared<Incrementer>();
    auto inc_p1_s4 = std::make_shared<Incrementer>();
    auto inc_p2_s1 = std::make_shared<Incrementer>();
    auto inc_p2_s2 = std::make_shared<Incrementer>();
    auto inc_p3_s1 = std::make_shared<Incrementer>();
    auto probe = std::make_shared<BranchSyncProbe>();

    circuit->AddComponent( counter );

    circuit->AddComponent( inc_p1_s1 );
    circuit->AddComponent( inc_p1_s2 );
    circuit->AddComponent( inc_p1_s3 );
    circuit->AddComponent( inc_p1_s4 );

    circuit->AddComponent( inc_p2_s1 );
    circuit->AddComponent( inc_p2_s2 );

    circuit->AddComponent( inc_p3_s1 );

    circuit->AddComponent( probe );

    // Wire branch 1
    circuit->ConnectOutToIn( counter, 0, inc_p1_s1, 0 );
    circuit->ConnectOutToIn( inc_p1_s1, 0, inc_p1_s2, 0 );
    circuit->ConnectOutToIn( inc_p1_s2, 0, inc_p1_s3, 0 );
    circuit->ConnectOutToIn( inc_p1_s3, 0, inc_p1_s4, 0 );
    circuit->ConnectOutToIn( inc_p1_s4, 0, probe, 0 );

    // Wire branch 2
    circuit->ConnectOutToIn( counter, 0, inc_p2_s1, 0 );
    circuit->ConnectOutToIn( inc_p2_s1, 0, inc_p2_s2, 0 );
    circuit->ConnectOutToIn( inc_p2_s2, 0, probe, 1 );

    // Wire branch 3
    circuit->ConnectOutToIn( counter, 0, inc_p3_s1, 0 );
    circuit->ConnectOutToIn( inc_p3_s1, 0, probe, 2 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Series );
    }
}

TEST_CASE( "FeedbackTest" )
{
    // Configure a circuit made up of an adder that adds a counter to its own previous output
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto adder = std::make_shared<Adder>();
    auto passthrough = std::make_shared<PassThrough>();
    auto probe = std::make_shared<FeedbackProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( adder );
    circuit->AddComponent( passthrough );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, adder, 0 );
    circuit->ConnectOutToIn( adder, 0, passthrough, 0 );

    circuit->ConnectOutToIn( passthrough, 0, adder, 1 );

    circuit->ConnectOutToIn( adder, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Series );
    }
}

TEST_CASE( "FeedbackTestNoCircuit" )
{
    auto counter = std::make_shared<Counter>();
    auto adder = std::make_shared<Adder>();
    auto passthrough = std::make_shared<PassThrough>();
    auto probe = std::make_shared<FeedbackProbe>();

    adder->ConnectInput( counter, 0, 0 );
    passthrough->ConnectInput( adder, 0, 0 );

    adder->ConnectInput( passthrough, 0, 1 );

    probe->ConnectInput( adder, 0, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        counter->Tick( Component::TickMode::Series );
        adder->Tick( Component::TickMode::Series );
        passthrough->Tick( Component::TickMode::Series );
        probe->Tick( Component::TickMode::Series );

        counter->Reset();
        adder->Reset();
        passthrough->Reset();
        probe->Reset();
    }
}

TEST_CASE( "RefCountResetRegressionTest" )
{
    auto circuit = std::make_shared<Circuit>();
    auto feedback = std::make_shared<FeedbackTester>( 2 );

    circuit->AddComponent( feedback );
    circuit->SetBufferCount( 2 );

    feedback->ConnectInput( feedback, 0, 0 );
    feedback->SetValidInputs( 1 );

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->PauseAutoTick();

    feedback->ConnectInput( feedback, 0, 1 );
    feedback->ConnectInput( feedback, 0, 2 );
    feedback->ConnectInput( feedback, 0, 3 );
    feedback->SetValidInputs( 4 );

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->PauseAutoTick();

    feedback->ConnectInput( feedback, 0, 4 );
    feedback->ConnectInput( feedback, 0, 5 );
    feedback->ConnectInput( feedback, 0, 6 );
    feedback->ConnectInput( feedback, 0, 7 );
    feedback->ConnectInput( feedback, 0, 8 );
    feedback->ConnectInput( feedback, 0, 9 );
    feedback->SetValidInputs( 10 );

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->StopAutoTick();
}

TEST_CASE( "NoOutputTest" )
{
    // Configure a circuit where the counter responds sporadically
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<SporadicCounter>();
    auto probe = std::make_shared<NoOutputProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Series );
    }
}

TEST_CASE( "ChangingOutputTest" )
{
    // Configure a circuit whereby a component outputs varying types (int, float, string, vector)
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<ChangingCounter>();
    auto probe = std::make_shared<ChangingProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Series );
    }
}

TEST_CASE( "ThreadPerformanceTest" )
{
    int const efficiencyThreshold = 80;  // expect at least 80% efficiency

    // Configure a circuit made up of 4 parallel counters, then adjust the thread count
    auto circuit = std::make_shared<Circuit>();

    auto counter1 = std::make_shared<SlowCounter>();
    auto counter2 = std::make_shared<SlowCounter>();
    auto counter3 = std::make_shared<SlowCounter>();
    auto counter4 = std::make_shared<SlowCounter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter1 );
    circuit->AddComponent( counter2 );
    circuit->AddComponent( counter3 );
    circuit->AddComponent( counter4 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter1, 0, probe, 0 );
    circuit->ConnectOutToIn( counter2, 0, probe, 1 );
    circuit->ConnectOutToIn( counter3, 0, probe, 2 );
    circuit->ConnectOutToIn( counter4, 0, probe, 3 );

    // Tick the circuit with no threads
    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    int count = probe->GetCount();
    std::cout << "0x Buffer Efficiency (Series Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold * 0.25 );

    // Tick the circuit with 1 thread, and check that no more ticks occurred
    if ( std::thread::hardware_concurrency() < 1 )
    {
        return;
    }
    circuit->SetBufferCount( 1 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "1x Buffer Efficiency (Series Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold * 0.25 );

    // Tick the circuit with 2 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 2 )
    {
        return;
    }
    circuit->SetBufferCount( 2 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "2x Buffer Efficiency (Series Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold * 0.5 );

    // Tick the circuit with 3 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 3 )
    {
        return;
    }
    circuit->SetBufferCount( 3 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "3x Buffer Efficiency (Series Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold * 0.75 );

    // Tick the circuit with 4 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 4 )
    {
        return;
    }
    circuit->SetBufferCount( 4 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "4x Buffer Efficiency (Series Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold );
}

TEST_CASE( "StopAutoTickRegressionTest" )
{
    auto circuit = std::make_shared<Circuit>();

    auto counter1 = std::make_shared<SlowCounter>();
    auto counter2 = std::make_shared<SlowCounter>();
    auto counter3 = std::make_shared<SlowCounter>();
    auto counter4 = std::make_shared<SlowCounter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter1 );
    circuit->AddComponent( counter2 );
    circuit->AddComponent( counter3 );
    circuit->AddComponent( counter4 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter1, 0, probe, 0 );
    circuit->ConnectOutToIn( counter2, 0, probe, 1 );
    circuit->ConnectOutToIn( counter3, 0, probe, 2 );
    circuit->ConnectOutToIn( counter4, 0, probe, 3 );

    circuit->SetBufferCount( std::thread::hardware_concurrency() );

    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->StopAutoTick();
    circuit->RemoveComponent( counter1 );
    circuit->RemoveComponent( counter2 );
    circuit->RemoveComponent( counter3 );
    circuit->RemoveComponent( counter4 );
    circuit->RemoveComponent( probe );
}

TEST_CASE( "ThreadAdjustmentTest" )
{
    // Configure a counter circuit, then adjust the thread count while it's running
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );
    circuit->ConnectOutToIn( counter, 0, probe, 1 );
    circuit->ConnectOutToIn( counter, 0, probe, 2 );
    circuit->ConnectOutToIn( counter, 0, probe, 3 );

    // Tick the circuit for 100ms with 1 thread
    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Adjust the thread count while the circuit is running
    circuit->SetBufferCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 4 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 3 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->StopAutoTick();

    REQUIRE( circuit->GetBufferCount() == 3 );
}

TEST_CASE( "WiringTest" )
{
    // Configure a counter circuit, then re-wire it while it's running
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<CircuitCounter>();
    auto probe = std::make_shared<CircuitProbe>();

    auto counter_id = circuit->AddComponent( counter );
    auto probe_id = circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter_id, 0, probe, 0 );
    circuit->ConnectOutToIn( probe, 0, counter_id, 0 );

    // Tick the circuit for 100ms with 1 thread
    circuit->StartAutoTick( Component::TickMode::Series );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Re-wire

    auto pass_s1 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s1 );

    circuit->ConnectOutToIn( pass_s1, 0, probe_id, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->ConnectOutToIn( counter, 0, pass_s1, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Disconnect a component

    circuit->PauseAutoTick();
    probe->DisconnectInput( 0 );
    circuit->ResumeAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->DisconnectComponent( probe );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Wire in a new component

    auto pass_s2 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s2 );

    circuit->ConnectOutToIn( probe, 0, counter_id, 0 );
    circuit->ConnectOutToIn( pass_s2, 0, probe, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->ConnectOutToIn( pass_s1, 0, pass_s2, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->StopAutoTick();
}

//=================================================================================================

TEST_CASE( "SerialTest2" )
{
    // Configure a circuit made up of a counter and 5 incrementers in series
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_s1 = std::make_shared<Incrementer>( 1 );
    auto inc_s2 = std::make_shared<Incrementer>( 2 );
    auto inc_s3 = std::make_shared<Incrementer>( 3 );
    auto inc_s4 = std::make_shared<Incrementer>( 4 );
    auto inc_s5 = std::make_shared<Incrementer>( 5 );
    auto probe = std::make_shared<SerialProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( inc_s1 );
    circuit->AddComponent( inc_s2 );
    circuit->AddComponent( inc_s3 );
    circuit->AddComponent( inc_s4 );
    circuit->AddComponent( inc_s5 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, inc_s1, 0 );
    circuit->ConnectOutToIn( inc_s1, 0, inc_s2, 0 );
    circuit->ConnectOutToIn( inc_s2, 0, inc_s3, 0 );
    circuit->ConnectOutToIn( inc_s3, 0, inc_s4, 0 );
    circuit->ConnectOutToIn( inc_s4, 0, inc_s5, 0 );
    circuit->ConnectOutToIn( inc_s5, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Parallel );
    }
}

TEST_CASE( "ParallelTest2" )
{
    // Configure a circuit made up of a counter and 5 incrementers in parallel
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1 = std::make_shared<Incrementer>( 1 );
    auto inc_p2 = std::make_shared<Incrementer>( 2 );
    auto inc_p3 = std::make_shared<Incrementer>( 3 );
    auto inc_p4 = std::make_shared<Incrementer>( 4 );
    auto inc_p5 = std::make_shared<Incrementer>( 5 );
    auto probe = std::make_shared<ParallelProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( inc_p1 );
    circuit->AddComponent( inc_p2 );
    circuit->AddComponent( inc_p3 );
    circuit->AddComponent( inc_p4 );
    circuit->AddComponent( inc_p5 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, inc_p1, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p2, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p3, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p4, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p5, 0 );
    circuit->ConnectOutToIn( inc_p1, 0, probe, 0 );
    circuit->ConnectOutToIn( inc_p2, 0, probe, 1 );
    circuit->ConnectOutToIn( inc_p3, 0, probe, 2 );
    circuit->ConnectOutToIn( inc_p4, 0, probe, 3 );
    circuit->ConnectOutToIn( inc_p5, 0, probe, 4 );

    // Tick the circuit for 100ms with 3 threads
    circuit->SetBufferCount( 3 );
    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->StopAutoTick();
}

TEST_CASE( "BranchSyncTest2" )
{
    // Configure a circuit made up of 3 parallel branches of 4, 2, and 1 component(s) respectively
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1_s1 = std::make_shared<Incrementer>();
    auto inc_p1_s2 = std::make_shared<Incrementer>();
    auto inc_p1_s3 = std::make_shared<Incrementer>();
    auto inc_p1_s4 = std::make_shared<Incrementer>();
    auto inc_p2_s1 = std::make_shared<Incrementer>();
    auto inc_p2_s2 = std::make_shared<Incrementer>();
    auto inc_p3_s1 = std::make_shared<Incrementer>();
    auto probe = std::make_shared<BranchSyncProbe>();

    circuit->AddComponent( counter );

    circuit->AddComponent( inc_p1_s1 );
    circuit->AddComponent( inc_p1_s2 );
    circuit->AddComponent( inc_p1_s3 );
    circuit->AddComponent( inc_p1_s4 );

    circuit->AddComponent( inc_p2_s1 );
    circuit->AddComponent( inc_p2_s2 );

    circuit->AddComponent( inc_p3_s1 );

    circuit->AddComponent( probe );

    // Wire branch 1
    circuit->ConnectOutToIn( counter, 0, inc_p1_s1, 0 );
    circuit->ConnectOutToIn( inc_p1_s1, 0, inc_p1_s2, 0 );
    circuit->ConnectOutToIn( inc_p1_s2, 0, inc_p1_s3, 0 );
    circuit->ConnectOutToIn( inc_p1_s3, 0, inc_p1_s4, 0 );
    circuit->ConnectOutToIn( inc_p1_s4, 0, probe, 0 );

    // Wire branch 2
    circuit->ConnectOutToIn( counter, 0, inc_p2_s1, 0 );
    circuit->ConnectOutToIn( inc_p2_s1, 0, inc_p2_s2, 0 );
    circuit->ConnectOutToIn( inc_p2_s2, 0, probe, 1 );

    // Wire branch 3
    circuit->ConnectOutToIn( counter, 0, inc_p3_s1, 0 );
    circuit->ConnectOutToIn( inc_p3_s1, 0, probe, 2 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Parallel );
    }
}

TEST_CASE( "FeedbackTest2" )
{
    // Configure a circuit made up of an adder that adds a counter to its own previous output
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto adder = std::make_shared<Adder>();
    auto passthrough = std::make_shared<PassThrough>();
    auto probe = std::make_shared<FeedbackProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( adder );
    circuit->AddComponent( passthrough );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, adder, 0 );
    circuit->ConnectOutToIn( adder, 0, passthrough, 0 );

    circuit->ConnectOutToIn( passthrough, 0, adder, 1 );

    circuit->ConnectOutToIn( adder, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Parallel );
    }
}

TEST_CASE( "FeedbackTestNoCircuit2" )
{
    auto counter = std::make_shared<Counter>();
    auto adder = std::make_shared<Adder>();
    auto passthrough = std::make_shared<PassThrough>();
    auto probe = std::make_shared<FeedbackProbe>();

    adder->ConnectInput( counter, 0, 0 );
    passthrough->ConnectInput( adder, 0, 0 );

    adder->ConnectInput( passthrough, 0, 1 );

    probe->ConnectInput( adder, 0, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        counter->Tick( Component::TickMode::Parallel );
        adder->Tick( Component::TickMode::Parallel );
        passthrough->Tick( Component::TickMode::Parallel );
        probe->Tick( Component::TickMode::Parallel );

        counter->Reset();
        adder->Reset();
        passthrough->Reset();
        probe->Reset();
    }
}

TEST_CASE( "RefCountResetRegressionTest2" )
{
    auto circuit = std::make_shared<Circuit>();
    auto feedback = std::make_shared<FeedbackTester>( 2 );

    circuit->AddComponent( feedback );
    circuit->SetBufferCount( 2 );

    feedback->ConnectInput( feedback, 0, 0 );
    feedback->SetValidInputs( 1 );

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->PauseAutoTick();

    feedback->ConnectInput( feedback, 0, 1 );
    feedback->ConnectInput( feedback, 0, 2 );
    feedback->ConnectInput( feedback, 0, 3 );
    feedback->SetValidInputs( 4 );

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->PauseAutoTick();

    feedback->ConnectInput( feedback, 0, 4 );
    feedback->ConnectInput( feedback, 0, 5 );
    feedback->ConnectInput( feedback, 0, 6 );
    feedback->ConnectInput( feedback, 0, 7 );
    feedback->ConnectInput( feedback, 0, 8 );
    feedback->ConnectInput( feedback, 0, 9 );
    feedback->SetValidInputs( 10 );

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->StopAutoTick();
}

TEST_CASE( "NoOutputTest2" )
{
    // Configure a circuit where the counter responds sporadically
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<SporadicCounter>();
    auto probe = std::make_shared<NoOutputProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Parallel );
    }
}

TEST_CASE( "ChangingOutputTest2" )
{
    // Configure a circuit whereby a component outputs varying types (int, float, string, vector)
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<ChangingCounter>();
    auto probe = std::make_shared<ChangingProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Parallel );
    }
}

TEST_CASE( "ThreadPerformanceTest2" )
{
    int efficiencyThreshold = 80;  // expect at least 80% efficiency with 4+ cores
    if ( std::thread::hardware_concurrency() < 4 )
    {
        float fraction = (float)std::thread::hardware_concurrency() / 4;
        efficiencyThreshold *= fraction;
    }

    // Configure a circuit made up of 4 parallel counters, then adjust the thread count
    auto circuit = std::make_shared<Circuit>();

    auto counter1 = std::make_shared<SlowCounter>();
    auto counter2 = std::make_shared<SlowCounter>();
    auto counter3 = std::make_shared<SlowCounter>();
    auto counter4 = std::make_shared<SlowCounter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter1 );
    circuit->AddComponent( counter2 );
    circuit->AddComponent( counter3 );
    circuit->AddComponent( counter4 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter1, 0, probe, 0 );
    circuit->ConnectOutToIn( counter2, 0, probe, 1 );
    circuit->ConnectOutToIn( counter3, 0, probe, 2 );
    circuit->ConnectOutToIn( counter4, 0, probe, 3 );

    // Tick the circuit with no threads
    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    int count = probe->GetCount();
    std::cout << "0x Buffer Efficiency (Parallel Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold * 0.7 );

    // Tick the circuit with 1 thread, and check that no more ticks occurred
    if ( std::thread::hardware_concurrency() < 1 )
    {
        return;
    }
    circuit->SetBufferCount( 1 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "1x Buffer Efficiency (Parallel Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold * 0.7 );

    // Tick the circuit with 2 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 2 )
    {
        return;
    }
    circuit->SetBufferCount( 2 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "2x Buffer Efficiency (Parallel Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold );

    // Tick the circuit with 3 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 3 )
    {
        return;
    }
    circuit->SetBufferCount( 3 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "3x Buffer Efficiency (Parallel Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold );

    // Tick the circuit with 4 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 4 )
    {
        return;
    }
    circuit->SetBufferCount( 4 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "4x Buffer Efficiency (Parallel Mode): " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 >= efficiencyThreshold );
}

TEST_CASE( "StopAutoTickRegressionTest2" )
{
    auto circuit = std::make_shared<Circuit>();

    auto counter1 = std::make_shared<SlowCounter>();
    auto counter2 = std::make_shared<SlowCounter>();
    auto counter3 = std::make_shared<SlowCounter>();
    auto counter4 = std::make_shared<SlowCounter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter1 );
    circuit->AddComponent( counter2 );
    circuit->AddComponent( counter3 );
    circuit->AddComponent( counter4 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter1, 0, probe, 0 );
    circuit->ConnectOutToIn( counter2, 0, probe, 1 );
    circuit->ConnectOutToIn( counter3, 0, probe, 2 );
    circuit->ConnectOutToIn( counter4, 0, probe, 3 );

    circuit->SetBufferCount( std::thread::hardware_concurrency() );

    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->StopAutoTick();
    circuit->RemoveComponent( counter1 );
    circuit->RemoveComponent( counter2 );
    circuit->RemoveComponent( counter3 );
    circuit->RemoveComponent( counter4 );
    circuit->RemoveComponent( probe );
}

TEST_CASE( "ThreadAdjustmentTest2" )
{
    // Configure a counter circuit, then adjust the thread count while it's running
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );
    circuit->ConnectOutToIn( counter, 0, probe, 1 );
    circuit->ConnectOutToIn( counter, 0, probe, 2 );
    circuit->ConnectOutToIn( counter, 0, probe, 3 );

    // Tick the circuit for 100ms with 1 thread
    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Adjust the thread count while the circuit is running
    circuit->SetBufferCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 4 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetBufferCount( 3 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->StopAutoTick();

    REQUIRE( circuit->GetBufferCount() == 3 );
}

TEST_CASE( "WiringTest2" )
{
    // Configure a counter circuit, then re-wire it while it's running
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<CircuitCounter>();
    auto probe = std::make_shared<CircuitProbe>();

    auto counter_id = circuit->AddComponent( counter );
    auto probe_id = circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter_id, 0, probe, 0 );
    circuit->ConnectOutToIn( probe, 0, counter_id, 0 );

    // Tick the circuit for 100ms with 1 thread
    circuit->StartAutoTick( Component::TickMode::Parallel );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Re-wire

    auto pass_s1 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s1 );

    circuit->ConnectOutToIn( pass_s1, 0, probe_id, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->ConnectOutToIn( counter, 0, pass_s1, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Disconnect a component

    circuit->PauseAutoTick();
    probe->DisconnectInput( 0 );
    circuit->ResumeAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->DisconnectComponent( probe );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Wire in a new component

    auto pass_s2 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s2 );

    circuit->ConnectOutToIn( probe, 0, counter_id, 0 );
    circuit->ConnectOutToIn( pass_s2, 0, probe, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->ConnectOutToIn( pass_s1, 0, pass_s2, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->StopAutoTick();
}

//=================================================================================================

TEST_CASE( "ThreadStopRegressionTest" )
{
    // Configure a circuit made up of 3 parallel branches of 4, 2, and 1 component(s) respectively
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1_s1 = std::make_shared<Incrementer>();
    auto inc_p1_s2 = std::make_shared<Incrementer>();
    auto inc_p1_s3 = std::make_shared<Incrementer>();
    auto inc_p1_s4 = std::make_shared<Incrementer>();
    auto inc_p2_s1 = std::make_shared<Incrementer>();
    auto inc_p2_s2 = std::make_shared<Incrementer>();
    auto inc_p3_s1 = std::make_shared<Incrementer>();
    auto probe = std::make_shared<BranchSyncProbe>();

    circuit->AddComponent( counter );

    circuit->AddComponent( inc_p1_s1 );
    circuit->AddComponent( inc_p1_s2 );
    circuit->AddComponent( inc_p1_s3 );
    circuit->AddComponent( inc_p1_s4 );

    circuit->AddComponent( inc_p2_s1 );
    circuit->AddComponent( inc_p2_s2 );

    circuit->AddComponent( inc_p3_s1 );

    circuit->AddComponent( probe );

    // Wire branch 1
    circuit->ConnectOutToIn( counter, 0, inc_p1_s1, 0 );
    circuit->ConnectOutToIn( inc_p1_s1, 0, inc_p1_s2, 0 );
    circuit->ConnectOutToIn( inc_p1_s2, 0, inc_p1_s3, 0 );
    circuit->ConnectOutToIn( inc_p1_s3, 0, inc_p1_s4, 0 );
    circuit->ConnectOutToIn( inc_p1_s4, 0, probe, 0 );

    // Wire branch 2
    circuit->ConnectOutToIn( counter, 0, inc_p2_s1, 0 );
    circuit->ConnectOutToIn( inc_p2_s1, 0, inc_p2_s2, 0 );
    circuit->ConnectOutToIn( inc_p2_s2, 0, probe, 1 );

    // Wire branch 3
    circuit->ConnectOutToIn( counter, 0, inc_p3_s1, 0 );
    circuit->ConnectOutToIn( inc_p3_s1, 0, probe, 2 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Series );
        circuit->Tick( Component::TickMode::Parallel );
    }

    circuit->SetBufferCount( 30 );

    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick( Component::TickMode::Series );
        circuit->Tick( Component::TickMode::Parallel );
    }
}

TEST_CASE( "DisconnectComponentRegressionTest" )
{
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto probe = std::make_shared<NullInputProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );
    circuit->ConnectOutToIn( counter, 0, probe, 1 );

    circuit->DisconnectComponent( counter );

    circuit->Tick( Component::TickMode::Series );
}
