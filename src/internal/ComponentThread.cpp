/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2023, Marcus Tomlinson

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

#include <internal/ComponentThread.h>

#include <functional>
#include <queue>

using namespace DSPatch::internal;

template <class T>
class SafeQueue final
{
public:
    void push( const T& t )
    {
        std::lock_guard<std::mutex> lock( m );
        q.push( t );
        c.notify_one();
    }

    const T& front()
    {
        std::unique_lock<std::mutex> lock( m );
        while ( q.empty() )
        {
            c.wait( lock );
        }
        const T& f = q.front();
        q.pop();
        return f;
    }

private:
    std::queue<T> q;
    std::mutex m;
    std::condition_variable c;
};

class ThreadPool final
{
public:
    enum class JobType
    {
        Job,
        Stop
    };

    struct Job
    {
        // cppcheck-suppress unusedStructMember
        JobType type;
        std::function<void()> callback;
    };

    ThreadPool( int bufferCount, int threadsPerBuffer )
        : bufferCount( bufferCount )
        , threadsPerBuffer( threadsPerBuffer )
    {
        jobs.resize( bufferCount );
        bufferThreads.resize( bufferCount );
        for ( int i = 0; i < bufferCount; ++i )
        {
            for ( int j = 0; j < threadsPerBuffer; ++j )
            {
                bufferThreads[i].push_back( std::thread( &ThreadPool::Run, this, i ) );
            }
        }
    }

    ~ThreadPool()
    {
        for ( int i = 0; i < bufferCount; ++i )
        {
            for ( int j = 0; j < threadsPerBuffer; ++j )
            {
                jobs[i].push( { JobType::Stop, nullptr } );
            }
            for ( int j = 0; j < threadsPerBuffer; ++j )
            {
                bufferThreads[i][j].join();
            }
        }
    }

    void AddJob( int bufferNo, const std::function<void()>& callback )
    {
        jobs[bufferNo].push( { JobType::Job, callback } );
    }

    void Run( int bufferNo )
    {
        while ( true )
        {
            const auto& job = jobs[bufferNo].front();
            if ( job.type == JobType::Stop )
            {
                break;
            }
            job.callback();
        }
    }

    int bufferCount;
    int threadsPerBuffer;
    std::vector<std::vector<std::thread>> bufferThreads;
    std::deque<SafeQueue<Job>> jobs;
};

static ThreadPool threadPool( 8, 2 );

ComponentThread::ComponentThread() = default;

void ComponentThread::Sync()
{
    std::unique_lock<std::mutex> lock( _syncMutex );

    if ( !_gotSync )  // if haven't already got sync
    {
        _syncCondt.wait( lock );  // wait for sync
    }
}

void ComponentThread::Resume( int bufferNo, std::function<void()> const& tick )
{
    _gotSync = false;  // reset the sync flag
    _tick = tick;
    threadPool.AddJob( bufferNo, std::bind( &ComponentThread::_Run, this ) );
}

void ComponentThread::_Run()
{
    _tick();

    std::lock_guard<std::mutex> lock( _syncMutex );

    _gotSync = true;  // set the sync flag
    _syncCondt.notify_all();
}
