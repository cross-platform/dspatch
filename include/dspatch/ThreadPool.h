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

#pragma once

#include <dspatch/Common.h>

#include <functional>

namespace DSPatch
{

namespace internal
{
class ThreadPool;
}  // namespace internal

/// A thread pool for enabling multi-buffering in components and circuits

/**
To boost component / circuit performance, multi-buffering can be enabled by passing a ThreadPool
object to its SetThreadPool() method. bufferCount sets the number of threads that should run
serially through the circuit, while, as the name suggests, threadsPerBuffer sets the number of
threads to use per buffer.
*/

class DLLEXPORT ThreadPool final
{
public:
    NONCOPYABLE( ThreadPool );

    using SPtr = std::shared_ptr<ThreadPool>;
    using SCPtr = std::shared_ptr<const ThreadPool>;

    explicit ThreadPool( int bufferCount, int threadsPerBuffer = 1 );
    ~ThreadPool();

    int GetBufferCount() const;
    int GetThreadsPerBuffer() const;

    void AddJob( int bufferNo, const std::function<void()>& job );

private:
    std::unique_ptr<internal::ThreadPool> p;
};

}  // namespace DSPatch
