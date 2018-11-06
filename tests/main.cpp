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
#include <components/Incrementer.h>
#include <components/NoOutputProbe.h>
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
        circuit->Tick();
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
    circuit->SetThreadCount( 3 );
    circuit->StartAutoTick();
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
        circuit->Tick();
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
        circuit->Tick();
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
        counter->Tick();
        adder->Tick();
        passthrough->Tick();
        probe->Tick();

        counter->Reset();
        adder->Reset();
        passthrough->Reset();
        probe->Reset();
    }
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
        circuit->Tick();
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
        circuit->Tick();
    }
}

TEST_CASE( "ThreadPerformanceTest" )
{
    int const efficiencyThreshold = 90;  // expect at least 90% efficiency

    // Configure a circuit made up of 3 parallel counters, then adjust the thread count
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
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    circuit->PauseAutoTick();

    int count = probe->GetCount();
    std::cout << "0x Thread Efficiency: " << count / 5 << "%" << std::endl;
    REQUIRE( count / 5 > efficiencyThreshold );

    // Tick the circuit with 1 thread, and check that no more ticks occurred
    if ( std::thread::hardware_concurrency() < 1 )
    {
        return;
    }
    circuit->SetThreadCount( 1 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "1x Thread Efficiency: " << count / 5 << "%" << std::endl;
    REQUIRE( count / 5 > efficiencyThreshold );

    // Tick the circuit with 2 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 2 )
    {
        return;
    }
    circuit->SetThreadCount( 2 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "2x Thread Efficiency: " << count / 10 << "%" << std::endl;
    REQUIRE( count / 10 > efficiencyThreshold );

    // Tick the circuit with 3 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 3 )
    {
        return;
    }
    circuit->SetThreadCount( 3 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "3x Thread Efficiency: " << count / 15 << "%" << std::endl;
    REQUIRE( count / 15 > efficiencyThreshold );

    // Tick the circuit with 4 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 4 )
    {
        return;
    }
    circuit->SetThreadCount( 4 );

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();
    probe->ResetCount();

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    circuit->PauseAutoTick();

    count = probe->GetCount();
    std::cout << "4x Thread Efficiency: " << count / 20 << "%" << std::endl;
    REQUIRE( count / 20 > efficiencyThreshold );
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
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Adjust the thread count while the circuit is running
    circuit->SetThreadCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 4 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 3 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->StopAutoTick();

    REQUIRE( circuit->GetThreadCount() == 3 );
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
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Re-wire

    auto pass_s1 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s1 );

    circuit->ConnectOutToIn( pass_s1, 0, probe_id, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->ConnectOutToIn( counter, 0, pass_s1, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Disconnect a component

    probe->DisconnectInput( 0 );
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
