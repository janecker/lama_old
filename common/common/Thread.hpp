/**
 * @file common/Thread.hpp
 *
 * @license
 * Copyright (c) 2009-2015
 * Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 * for Fraunhofer-Gesellschaft
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * @endlicense
 *
 * @brief Definition of the class Thread to manage thread ids and names
 * @author Thomas Brandes
 * @date 11.06.2015
 */
#pragma once

// for dll_import

#include <common/config.hpp>

#include <pthread.h>

namespace common
{

/** This class helps to deal with thread ids and to name threads.
 *  It also supports critical regions by mutex and scoped locks.
 */

class COMMON_DLL_IMPORTEXPORT Thread
{
public:

    typedef pthread_t Id;

    static Id getSelf();

    /** Set a name for the current thread. */

    static void defineCurrentThreadId( const char* name );

    /** Query the name of the current thread. */

    static const char* getCurrentThreadId();

    /** Own mutex class for synchronization of threads */

    class Mutex
    {

    friend class ScopedLock;  // allow access to the mutex

    public:

        /** Constructor creates and initalizes the mutex. 
         *
         *  @param[in] isRecursive if true the mutex will be recursive
         *
         *  A recursive mutex might be locked by the same thread several times.
         */

        Mutex( const bool isRecursive = false );

        /** Destructor frees and releases the mutex. */

        ~Mutex();

        /** Lock the mutex when entering a critical section. */

        void lock();

        /** Unlocks the mutex when leaving a critical section. */

        void unlock();

        pthread_mutex_t p_mutex;

        pthread_mutexattr_t p_mutexattr;
    };

    /** Locking of a mutex within a scope, unlock by destructor. */

    class ScopedLock
    {

    friend class Condition;  // allow access to the mutex

    public:

        ScopedLock( Mutex& mutex );

        ~ScopedLock();

        Mutex& mMutex;
    };

    /** Own condition class for synchronization of threads */

    class Condition
    {
    public:

        /** Constructor creates and initalizes the mutex. */

        Condition();

        /** Destructor frees and releases the mutex. */

        ~Condition();

        /** Notifiy one thread waiting a the condition */

        void notify_one();

        /** Notifiy all threds waiting on the condition. */

        void notify_all();

        void wait( ScopedLock& lock );

    private:

        pthread_cond_t p_condition;
    };


};


} //namespace lama