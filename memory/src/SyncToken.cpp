/**
 * @file SyncToken.cpp
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
 * @brief Implementation of non-pure methods of abstract class SyncToken.
 * @author Thomas Brandes, Jiri Kraus
 * @date 22.03.2011
 * @since 1.0.0
 */

// hpp
#include <memory/SyncToken.hpp>

// others
#include <memory/Access.hpp>
#include <memory/LAMAArray.hpp>

#include <common/Exception.hpp>

using boost::shared_ptr;

namespace memory
{

LAMA_LOG_DEF_LOGGER( SyncToken::logger, "SyncToken" )

/* ------------------------------------------------------------------------ */

SyncToken::CGuard::CGuard()
{
}

SyncToken::CGuard::~CGuard()
{
    // this destructor is called at the end of the program
    // Give an error message if not all SyncTokens have been deleted

    if( countSyncToken )
    {
        LAMA_LOG_ERROR( logger, "Remaining SyncToken (undeleted) = " << countSyncToken );
    }
}

/* ------------------------------------------------------------------------ */

int SyncToken::countSyncToken = 0;

SyncToken::CGuard SyncToken::cguard;

/* ------------------------------------------------------------------------ */

SyncToken::SyncToken()
                : mSynchronized( false )
{
    LAMA_LOG_DEBUG( logger, "SyncToken constructed" )

    countSyncToken++;
}

SyncToken::~SyncToken()
{
    LAMA_LOG_DEBUG( logger, "~SyncToken" )

    if( !mSynchronized )
    {
        LAMA_LOG_WARN( logger, "no synchronization called on SyncToken" )
    }

    countSyncToken--;
}

/* ------------------------------------------------------------------------ */

void SyncToken::writeAt( std::ostream& stream ) const
{
    stream << "SyncToken( synchronized = " << mSynchronized << " )";
}

/* ------------------------------------------------------------------------ */

void SyncToken::pushAccess( shared_ptr<Access> access )
{
    COMMON_ASSERT( access.get(), "NULL access cannot be pushed for synchronization." )

    if( mSynchronized )
    {
        LAMA_LOG_DEBUG( logger, *this << ": push access not done, already synchronized" )
    }
    else
    {
        LAMA_LOG_DEBUG( logger, *this << ": push access, will be freed at synchronization" )

        // take ownership of the access so it is not deleted before synchronization

        mAccesses.push_back( access );
    }
}

/* ------------------------------------------------------------------------ */

void SyncToken::pushArray( shared_ptr<_LAMAArray> array )
{
    COMMON_ASSERT( array.get(), "NULL array cannot be pushed for synchronization." )

    if( mSynchronized )
    {
        LAMA_LOG_DEBUG( logger, *this << ": push array not done, already synchronized" )

    }
    else
    {
        LAMA_LOG_DEBUG( logger, *this << ": push array, will be freed at synchronization" )

        // take ownership of the pointer, have to be deleted at synchronization

        mArrays.push_back( array );
    }
}

/* ----------------------------------------------------------------------- */

void SyncToken::pushRoutine( boost::function<void()> function )
{
    mSynchronizedFunctions.push_back( function );
}

/* ----------------------------------------------------------------------- */

bool SyncToken::isSynchronized() const
{
    return mSynchronized;
}

/* ----------------------------------------------------------------------- */

void SyncToken::setSynchronized()
{
    if( mSynchronized )
    {
        COMMON_THROWEXCEPTION( *this << " is already synchronized" )
    }

    mSynchronized = true;

    // after synchronization we can give up ownership of accesses, arrays, childs

    mAccesses.clear();
    mArrays.clear();

    for( size_t i = 0; i < mSynchronizedFunctions.size(); ++i )
    {
        mSynchronizedFunctions[i]();
    }
}

}