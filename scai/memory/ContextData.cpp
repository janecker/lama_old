/**
 * @file ContextData.cpp
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
 * @brief Implementation of methods for Contextdata.
 * @author Thomas Brandes
 * @date 11.07.2011
 * @since 1.0.0
 */

// hpp
#include <scai/memory/ContextData.hpp>
#include <scai/memory/Context.hpp>

#include <scai/common/Exception.hpp>
#include <scai/logging.hpp>

using scai::tasking::SyncToken;

namespace scai
{

namespace memory
{

/* ---------------------------------------------------------------------------------*/

SCAI_LOG_DEF_LOGGER( ContextData::logger, "ContextData" )

/* ---------------------------------------------------------------------------------*/

ContextData::ContextData( MemoryPtr memory ) :

    size( 0 ),
    mMemory( memory ),
    pointer( 0 ),
    valid( false ),
    allocated( false )
{
}

ContextData::ContextData() :

    size( 0 ),
    mMemory(),
    pointer( 0 ),
    valid( false ), 
    allocated( false )
{
}

/* ---------------------------------------------------------------------------------*/

ContextData::~ContextData()
{
    SCAI_LOG_DEBUG( logger, "~ContextData @ " << *mMemory << ", size = " << size )

    free(); 
}

/* ---------------------------------------------------------------------------------*/

void ContextData::allocate( const size_t size )
{
    COMMON_ASSERT( 0 == pointer, "ContextData data already given at " << *mMemory )

    pointer = mMemory->allocate( size );

    if ( !pointer )
    {
        COMMON_THROWEXCEPTION( "Could not allocate ContextData of size = " << size << " on " << *mMemory )
    }

    this->size = size;
    allocated = true;
    SCAI_LOG_DEBUG( logger, "allocated " << size << " bytes" )
}

/* ---------------------------------------------------------------------------------*/

void ContextData::setRef( void* reference, const size_t size )
{
    COMMON_ASSERT( 0 == pointer, "ContextData data already given at " << *mMemory )
    pointer = reference;
    this->size = size;
    allocated = false;
    valid     = true;   // we assume it as valid data

    if ( !pointer && size )
    {
        COMMON_THROWEXCEPTION( "NULL pointer cannot set be as reference, size = " << size )
    }

    SCAI_LOG_DEBUG( logger, "set ref for " << size << " bytes" )
}

/* ---------------------------------------------------------------------------------*/

void ContextData::free()
{
    SCAI_LOG_TRACE( logger, "free for " << *mMemory )

    if ( mMemory && pointer )
    {
        if ( allocated )
        {
            mMemory->free( pointer, size );
        }
    }

    pointer = 0;
    size = 0;
    valid = false;
    // we do not delete the mMemory pointers as output will cause runtime errors
}

/* ---------------------------------------------------------------------------------*/

void ContextData::realloc( const size_t newSize, const size_t validSize )
{
    // Note: realloc can also be used to shrink the array size

    COMMON_ASSERT( allocated, "Cannot realloc data set by reference" )
    COMMON_ASSERT( mMemory, "no mMemory available for realloc" )

    COMMON_ASSERT_LE( validSize, size, "size of valid data is more than actual size" )
    COMMON_ASSERT_LE( validSize, newSize, "size of valid data is more than new size" )

    void* oldPointer = pointer;

    size_t oldSize = size;

    if ( validSize <= 0 )
    {
        // existent data is no more needed
        mMemory->free( pointer, size );
    }

    // allocate new memory

    pointer = mMemory->allocate( newSize );
    size = newSize;

    if ( validSize > 0 )
    {
        // copy the old entries in the new memory befree free of old memory

        mMemory->memcpy( pointer, oldPointer, validSize );
        mMemory->free( oldPointer, oldSize );
    }
}

void ContextData::reserve( const size_t newSize, const size_t validSize )
{
    if ( newSize <= size )
    {
        COMMON_ASSERT_LE( validSize, newSize, "size of valid data is more than new size" )

        // current capacity is sufficient
        return;
    }

    if ( size == 0 )
    {
        COMMON_ASSERT_LE( validSize, size, "size of valid data is more than actual size" )

        // first allocation of the data, validSize is also 0

        pointer = mMemory->allocate( newSize );
        size = newSize;
        allocated = true;
        return;
    }

    realloc( newSize, validSize );
}

void ContextData::writeAt( std::ostream& stream ) const
{
    stream << "ContextData ( size = " << size 
           << ", valid = " << valid << ", capacity = " << size 
           << " ) @ " << *mMemory;
}

void ContextData::copyFrom( const ContextData& other, size_t size )
{
    SCAI_LOG_INFO( logger, "copyFrom " << *other.mMemory << " to " << *mMemory << ", size = " << size )

    if ( mMemory.get() == other.mMemory.get() )
    {
        SCAI_LOG_INFO( logger, "copy on same context" )
        mMemory->memcpy( pointer, other.pointer, size );
    }
    else if ( mMemory->canCopyFrom( *other.mMemory ) )
    {
        SCAI_LOG_INFO( logger, "copy from" )
        mMemory->memcpyFrom( pointer, *other.mMemory, other.pointer, size );
    }
    else if ( other.mMemory->canCopyTo( *mMemory ) )
    {
        SCAI_LOG_INFO( logger, "copy to" )
        other.mMemory->memcpyTo( *mMemory, pointer, other.pointer, size );
    }
    else
    {
        SCAI_LOG_WARN( logger, "copyFrom " << *other.mMemory << " to " << *mMemory << " UNSUPPORTED" )

        COMMON_THROWEXCEPTION( "copyFrom  "
                               << *other.mMemory << " to " << *mMemory << ", size = " << size  << " UNSUPPORTED" )
        // Note: calling routine can deal with it by involving ContextData available on host
    }
}

SyncToken* ContextData::copyFromAsync( const ContextData& other, size_t size )
{
    SCAI_LOG_INFO( logger, "copyFrom " << *other.mMemory << " to " << *mMemory << ", size = " << size )

    if ( mMemory.get() == other.mMemory.get() )
    {
        // pointer equality implies it is the same context

        return mMemory->memcpyAsync( pointer, other.pointer, size );
    }
    else if ( mMemory->canCopyFrom( *other.mMemory ) )
    {
        return mMemory->memcpyFromAsync( pointer, *other.mMemory, other.pointer, size );
    }
    else if ( other.mMemory->canCopyTo( *mMemory ) )
    {
        return other.mMemory->memcpyToAsync( *mMemory, pointer, other.pointer, size );
    }
    else
    {
        COMMON_THROWEXCEPTION( "copyFrom  "
                               << *other.mMemory << " to " << *mMemory << ", size = " << size  << " NOT SUPPORTED" )

        // Note: calling routine can deal with it by involving ContextData available on host

        return NULL;  // dead code, but avoids warning
    }
}

} /* end namespace memory */

} /* end namespace scai */