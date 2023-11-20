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

static double refEff;

TEST_CASE( "SignalBusTest" )
{
    SignalBus signalBus;

    signalBus.SetSignalCount( 4 );

    signalBus.SetValue( 0, 1.0 );
    REQUIRE( signalBus.HasValue( 0 ) );
    REQUIRE( *signalBus.GetValue<double>( 0 ) == 1.0 );

    signalBus.SetValue( 1, 1.0f );
    REQUIRE( signalBus.HasValue( 1 ) );
    REQUIRE( *signalBus.GetValue<float>( 1 ) == 1.0f );

    signalBus.SetValue( 2, 1u );
    REQUIRE( signalBus.HasValue( 2 ) );
    REQUIRE( *signalBus.GetValue<unsigned int>( 2 ) == 1u );

    signalBus.SetValue( 3, 1 );
    REQUIRE( signalBus.HasValue( 3 ) );
    REQUIRE( *signalBus.GetValue<int>( 3 ) == 1 );

    // no 5th input so should return false and nullptr
    REQUIRE( !signalBus.HasValue( 4 ) );
    REQUIRE( signalBus.GetValue<int>( 4 ) == nullptr );

    REQUIRE( signalBus.GetType( 0 ) != signalBus.GetType( 1 ) );
    REQUIRE( signalBus.GetType( 1 ) != signalBus.GetType( 2 ) );
    REQUIRE( signalBus.GetType( 2 ) != signalBus.GetType( 3 ) );
    // no 5th input so should return void type_info
    REQUIRE( signalBus.GetType( 3 ) != signalBus.GetType( 4 ) );
}

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

    REQUIRE( counter->GetInputCount() == 0 );
    REQUIRE( inc_p1->GetInputCount() == 1 );
    REQUIRE( probe->GetInputCount() == 5 );

    circuit->AddComponent( counter );
    circuit->AddComponent( inc_p1 );
    circuit->AddComponent( inc_p2 );
    circuit->AddComponent( inc_p3 );
    circuit->AddComponent( inc_p4 );
    circuit->AddComponent( inc_p5 );
    circuit->AddComponent( probe );

    REQUIRE( circuit->GetComponentCount() == 7 );

    REQUIRE( !circuit->AddComponent( counter ) );
    REQUIRE( !circuit->AddComponent( inc_p1 ) );
    REQUIRE( !circuit->AddComponent( probe ) );

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

    REQUIRE( counter->GetBufferCount() == 3 );
    REQUIRE( inc_p1->GetBufferCount() == 3 );
    REQUIRE( probe->GetBufferCount() == 3 );

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

TEST_CASE( "RefCountResetRegressionTest" )
{
    auto circuit = std::make_shared<Circuit>();
    auto feedback = std::make_shared<FeedbackTester>( 2 );

    circuit->AddComponent( feedback );
    circuit->SetBufferCount( 2 );

    feedback->ConnectInput( feedback, 0, 0 );
    feedback->SetValidInputs( 1 );

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->PauseAutoTick();

    feedback->ConnectInput( feedback, 0, 1 );
    feedback->ConnectInput( feedback, 0, 2 );
    feedback->ConnectInput( feedback, 0, 3 );
    feedback->SetValidInputs( 4 );

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->PauseAutoTick();

    feedback->ConnectInput( feedback, 0, 4 );
    feedback->ConnectInput( feedback, 0, 5 );
    feedback->ConnectInput( feedback, 0, 6 );
    feedback->ConnectInput( feedback, 0, 7 );
    feedback->ConnectInput( feedback, 0, 8 );
    feedback->ConnectInput( feedback, 0, 9 );
    feedback->SetValidInputs( 10 );

    circuit->StartAutoTick();
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

    // Calculate reference efficiency

    SignalBus testBus;
    auto measureRef = [&testBus]( std::shared_ptr<SlowCounter>& counter )
    {
        for ( int i = 0; i < 1000; ++i )
        {
            counter->Process_( testBus, testBus );
        }
    };

    auto begin = std::chrono::high_resolution_clock::now();
    auto t1 = std::thread( [&measureRef, &counter1]() { measureRef( counter1 ); } );
    auto t2 = std::thread( [&measureRef, &counter2]() { measureRef( counter2 ); } );
    auto t3 = std::thread( [&measureRef, &counter3]() { measureRef( counter3 ); } );
    auto t4 = std::thread( [&measureRef, &counter4]() { measureRef( counter4 ); } );
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    auto end = std::chrono::high_resolution_clock::now();
    refEff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

    counter1->ResetCount();
    counter2->ResetCount();
    counter3->ResetCount();
    counter4->ResetCount();

    std::cout << "Reference Efficiency: " << refEff << "%" << std::endl;

    // Tick the circuit with no threads
    begin = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000; ++i )
    {
        circuit->Tick();
    }
    end = std::chrono::high_resolution_clock::now();
    auto eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

    auto overhead = 100 - ( 100 * ( eff / refEff ) );
    std::cout << "0x Buffer Efficiency: " << eff << "% (-" << overhead << "%)" << std::endl;
    REQUIRE( eff >= refEff * 0.25 * 0.80 );

    // Tick the circuit with 1 thread, and check that no more ticks occurred
    if ( std::thread::hardware_concurrency() < 1 )
    {
        return;
    }
    circuit->SetBufferCount( 1 );

    begin = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000; ++i )
    {
        circuit->Tick();
    }
    end = std::chrono::high_resolution_clock::now();
    eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

    overhead = 100 - ( 100 * ( eff / refEff ) );
    std::cout << "1x Buffer Efficiency: " << eff << "% (-" << overhead << "%)" << std::endl;
    REQUIRE( eff >= refEff * 0.25 * 0.80 );

    // Tick the circuit with 2 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 2 )
    {
        return;
    }
    circuit->SetBufferCount( 2 );

    begin = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000; ++i )
    {
        circuit->Tick();
    }
    end = std::chrono::high_resolution_clock::now();
    eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

    overhead = 100 - ( 100 * ( eff / refEff ) );
    std::cout << "2x Buffer Efficiency: " << eff << "% (-" << overhead << "%)" << std::endl;
    REQUIRE( eff >= refEff * 0.5 * 0.80 );

    // Tick the circuit with 3 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 4 )
    {
        return;
    }
    circuit->SetBufferCount( 3 );

    begin = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000; ++i )
    {
        circuit->Tick();
    }
    end = std::chrono::high_resolution_clock::now();
    eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

    overhead = 100 - ( 100 * ( eff / refEff ) );
    std::cout << "3x Buffer Efficiency: " << eff << "% (-" << overhead << "%)" << std::endl;
    REQUIRE( eff >= refEff * 0.75 * 0.80 );

    // Tick the circuit with 4 threads, and check that more ticks occurred
    if ( std::thread::hardware_concurrency() < 4 )
    {
        return;
    }
    circuit->SetBufferCount( 4 );

    begin = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000; ++i )
    {
        circuit->Tick();
    }
    end = std::chrono::high_resolution_clock::now();
    eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

    overhead = 100 - ( 100 * ( eff / refEff ) );
    std::cout << "4x Buffer Efficiency: " << eff << "% (-" << overhead << "%)" << std::endl;
    REQUIRE( eff >= refEff * 0.80 );
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

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    circuit->StopAutoTick();
    circuit->RemoveComponent( counter1 );
    circuit->RemoveComponent( counter2 );
    circuit->RemoveComponent( counter3 );
    circuit->RemoveComponent( counter4 );
    circuit->RemoveComponent( probe );

    REQUIRE( !circuit->RemoveComponent( counter1 ) );
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

    circuit->ConnectOutToIn( counter, 0, probe, 0 );
    circuit->ConnectOutToIn( probe, 0, counter, 0 );

    // Tick the circuit for 100ms with 1 thread
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Re-wire

    auto pass_s1 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s1 );

    circuit->ConnectOutToIn( pass_s1, 0, probe, 0 );
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

    circuit->ConnectOutToIn( probe, 0, counter, 0 );
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
        circuit->Tick();
    }

    circuit->SetBufferCount( std::thread::hardware_concurrency() );

    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick();
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

    circuit->Tick();
}

TEST_CASE( "AutoTickOnCircuitDestructRegressionTest" )
{
    auto circuit = std::make_shared<Circuit>();
    auto counter = std::make_shared<Counter>();
    circuit->AddComponent( counter );
    circuit->SetBufferCount( 3 );

    REQUIRE( counter->Count() == 0 );
    circuit->Tick();
    circuit->Tick();
    circuit->Tick();
    circuit->Tick();

    circuit = nullptr;

    REQUIRE( counter->Count() == 4 );
}

TEST_CASE( "AutoTickOnBuffersUpdateRegressionTest" )
{
    auto circuit = std::make_shared<Circuit>();
    auto counter = std::make_shared<Counter>();
    circuit->AddComponent( counter );
    circuit->SetBufferCount( 3 );

    REQUIRE( counter->Count() == 0 );
    circuit->Tick();
    circuit->Tick();
    circuit->Tick();
    circuit->Tick();

    circuit->SetBufferCount( 2 );

    REQUIRE( counter->Count() == 4 );
}

TEST_CASE( "AddComponentAfterMultiBufferTickRegressionTest" )
{
    auto circuit = std::make_shared<Circuit>();
    auto counter = std::make_shared<Counter>();
    circuit->AddComponent( counter );
    circuit->SetBufferCount( 2 );

    REQUIRE( counter->Count() == 0 );
    circuit->Tick();
    circuit->Sync();

    REQUIRE( counter->Count() == 1 );

    circuit->RemoveComponent( counter );
    circuit->AddComponent( counter );

    circuit->Tick();
    circuit->Sync();

    REQUIRE( counter->Count() == 2 );
}

TEST_CASE( "TenThousandComponents" )
{
    auto begin = std::chrono::high_resolution_clock::now();

    auto circuit = std::make_shared<Circuit>();

    auto source = std::make_shared<Counter>();
    auto dest = std::make_shared<ThreadingProbe>( 500 );
    circuit->AddComponent( source );
    circuit->AddComponent( dest );

    for ( int i = 0; i < 500; i++ )
    {
        Component::SPtr last = source;
        for ( int j = 0; j < 20; j++ )
        {
            auto passthrough = std::make_shared<PassThrough>();
            circuit->AddComponent( passthrough );
            circuit->ConnectOutToIn( last, 0, passthrough, 0 );
            last = passthrough;
        }
        circuit->ConnectOutToIn( last, 0, dest, i );
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

    std::cout << "Construction, 10000 Components: " << diff_ms << "ms\n";

    {
        int iterationCount = 1000;

        begin = std::chrono::high_resolution_clock::now();

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        end = std::chrono::high_resolution_clock::now();

        diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "0x Buffer, 10000 Components: " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetBufferCount( 1 );

        int iterationCount = 1000;

        begin = std::chrono::high_resolution_clock::now();

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        end = std::chrono::high_resolution_clock::now();

        diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "1x Buffer, 10000 Components: " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetBufferCount( 2 );

        int iterationCount = 1000;

        begin = std::chrono::high_resolution_clock::now();

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        end = std::chrono::high_resolution_clock::now();

        diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "2x Buffer, 10000 Components: " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetBufferCount( 3 );

        int iterationCount = 1000;

        begin = std::chrono::high_resolution_clock::now();

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        end = std::chrono::high_resolution_clock::now();

        diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "3x Buffer, 10000 Components: " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetBufferCount( 4 );

        int iterationCount = 1000;

        begin = std::chrono::high_resolution_clock::now();

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        end = std::chrono::high_resolution_clock::now();

        diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "4x Buffer, 10000 Components: " << diff_ms / iterationCount << "ms\n";
    }

    begin = std::chrono::high_resolution_clock::now();

    circuit.reset();

    end = std::chrono::high_resolution_clock::now();

    diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

    std::cout << "Destruction, 10000 Components: " << diff_ms << "ms\n";
}
