/**
 * @file common/Thread.hpp
 *
 * @license
 * Copyright (c) 2009-2017
 * Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 * for Fraunhofer-Gesellschaft
 *
 * This file is part of the SCAI framework LAMA.
 *
 * LAMA is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * LAMA is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with LAMA. If not, see <http://www.gnu.org/licenses/>.
 *
 * Other Usage
 * Alternatively, this file may be used in accordance with the terms and
 * conditions contained in a signed written agreement between you and
 * Fraunhofer SCAI. Please contact our distributor via info[at]scapos.com.
 * @endlicense
 *
 * @brief Definition of the class Thread to manage thread ids and names
 * @author Thomas Brandes
 * @date 11.06.2015
 */
#pragma once

// for dll_import
#include <scai/common/config.hpp>

// local library
#include <scai/common/NonCopyable.hpp>
#include <scai/common/macros/throw.hpp>

// C++11 features

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace scai
{

namespace common
{

/** This class helps to deal with thread ids and to name threads.
 *  It also supports critical regions by mutex and scoped locks.
 */

class COMMON_DLL_IMPORTEXPORT Thread : common::NonCopyable
{
public:

    typedef std::thread::id Id;

    static Id getSelf();

    /** Set a name for the current thread. */

    static void defineCurrentThreadName( const char* name );

    /** Query the name of a thread. */

    static const char* getThreadName( Id id );

    /** Query the name of the current thread. */

    static const char* getCurrentThreadName();

    /** Own mutex class for synchronization of threads */

    class COMMON_DLL_IMPORTEXPORT Mutex
    {

        friend class ScopedLock;  // allow access to the mutex

    public:

        /** Constructor creates and initalizes the mutex.
         *
         *  @param[in] isRecursive if true the mutex will be recursive
         *
         *  A recursive mutex might be locked by the same thread several times.
         */

        Mutex( bool isRecursive = false );

        /** Destructor frees and releases the mutex. */

        ~Mutex();

        /** Lock the mutex when entering a critical section. */

        void lock();

        /** Unlocks the mutex when leaving a critical section. */

        void unlock();

        std::unique_ptr<std::mutex> mMutex;
        std::unique_ptr<std::recursive_mutex> mRecursiveMutex;

        bool mIsRecursive;  // false: mMutex != NULL, true: mRecursiveMutex != NULL

    };

    /** Add derived class which is a recursive mutex by default constructor. */

    class COMMON_DLL_IMPORTEXPORT RecursiveMutex : public Mutex
    {
    public:

        RecursiveMutex() : Mutex( true )
        {
        }

        ~RecursiveMutex()
        {
        }
    };

    /** Locking of a mutex within a scope, unlock by destructor. */

    class COMMON_DLL_IMPORTEXPORT ScopedLock
    {

        friend class Condition;  // allow access to the mutex

    public:

        ScopedLock( Mutex& mutex );

        ~ScopedLock();

        Mutex& mMutex;
    };

    /** Own condition class for synchronization of threads */

    class COMMON_DLL_IMPORTEXPORT Condition : std::condition_variable_any
    {
    public:

        /** Constructor creates and initalizes the mutex. */

        Condition();

        /** Destructor frees and releases the mutex. */

        ~Condition();

        /** Notifiy one thread waiting a the condition */

        void notifyOne();

        /** Notifiy all threds waiting on the condition. */

        void notifyAll();

        void wait( ScopedLock& lock );

    };

    /** Only supported signature, void* might be replaced by any pointer type. */

    typedef void* ( *ThreadFunction ) ( void* );

    Thread()
    {
    }

    /** Constructor of a new thread that executes a void routine with one reference argument. */

    template<typename T>
    Thread( void ( *work ) ( T& ), T& arg )
    {
        run( work, arg );
    }

    /** Run can also be called after thread has been created. */

    template<typename T>
    void run( void ( *work ) ( T& ), T& arg )
    {
        void* parg = ( void* ) ( &arg );
        ThreadFunction routine = ( ThreadFunction ) work;
        start( routine, parg );
    }

    void start( ThreadFunction start_routine, void* arg );

    void join();

    Id getId()
    {
        return mId;
    }

    /** Destructor of thread, waits for its termination. */

    ~Thread();

private:

    std::thread* mThread;  // contructed by start

    Id mId;
};

} /* end namespace common */

} /* end namespace scai */
