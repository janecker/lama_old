/**
 * @file CUDAStreamSyncToken.cpp
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
 * @brief Implementation of methods for class CUDAStreamSyncToken.
 * @author Jiri Kraus
 * @date 20.05.2011
 */

// hpp
#include <scai/tasking/cuda/CUDAStreamSyncToken.hpp>

#include <scai/tasking/cuda/CUDAStreamPool.hpp>

// internal scai libraries

#include <scai/common/macros/assert.hpp>
#include <scai/common/cuda/CUDAAccess.hpp>
#include <scai/common/cuda/CUDAError.hpp>

namespace scai
{

using common::CUDACtx;
using common::CUDAAccess;

namespace tasking
{

CUDAStreamSyncToken::CUDAStreamSyncToken( const CUDACtx& cuda, const StreamType type ) : mCUDA( cuda )
{
    // take a CUDA stream from a pool, that might be allocated at first use

    mStream = CUDAStreamPool::getPool( mCUDA ).reserveStream( type );
    mEvent = 0;
    SCAI_LOG_DEBUG( logger, "StreamSyncToken for " << mCUDA.getDeviceNr() << " generated, stream = " << mStream )
}

CUDAStreamSyncToken::~CUDAStreamSyncToken()
{
    wait();

    CUDAStreamPool::getPool( mCUDA ).releaseStream( mStream );
}

void CUDAStreamSyncToken::wait()
{
    if ( isSynchronized() )
    {
        return;
    }

    SCAI_LOG_DEBUG( logger, "wait on CUDA stream synchronization" )

    {
        common::CUDAAccess tmpAccess( mCUDA );

        if ( mEvent != 0 )
        {
            SCAI_CUDA_DRV_CALL( cuEventSynchronize( mEvent ), "cuEventSynchronize( " << mEvent << " ) failed." )
            SCAI_CUDA_DRV_CALL( cuEventDestroy( mEvent ), "cuEventDestroy( " << mEvent << " ) failed." );
            mEvent = 0;
        }
        else
        {
            SCAI_LOG_DEBUG( logger, "synchronize with stream " << mStream );
            SCAI_CUDA_DRV_CALL( cuStreamSynchronize( mStream ), "cuStreamSynchronize( " << mStream << " ) failed." );
            SCAI_LOG_DEBUG( logger, "synchronized with stream " << mStream );
        }

        // finally called functions might also need the context, e.g. unbindTexture

        setSynchronized();
    }
}

bool CUDAStreamSyncToken::probe() const
{
    if ( isSynchronized() )
    {
        return true;
    }

    return probeEvent( mEvent );
}

cudaStream_t CUDAStreamSyncToken::getCUDAStream() const
{
    return ( cudaStream_t ) mStream;
}

bool CUDAStreamSyncToken::probeEvent( const CUevent& stopEvent ) const
{
    SCAI_ASSERT( stopEvent != 0, "probe on invalid event" )

    common::CUDAAccess tmpAccess ( mCUDA );

    CUresult result = cuEventQuery( stopEvent );

    if ( result != CUDA_SUCCESS && result != CUDA_ERROR_NOT_READY )
    {
        SCAI_CUDA_DRV_CALL( result, "cuEventQuery failed for CUevent " << stopEvent << '.' );
    }

    return result == CUDA_SUCCESS;
}

bool CUDAStreamSyncToken::queryEvent( const CUevent event ) const
{
    common::CUDAAccess cudaAccess ( mCUDA );

    CUresult result = cuEventQuery( event );

    if ( result != CUDA_SUCCESS || result != CUDA_ERROR_NOT_READY )
    {
        SCAI_CUDA_DRV_CALL( result, "cuEventQuery failed for CUevent " << event << '.' );
    }

    return result == CUDA_SUCCESS;
}

void CUDAStreamSyncToken::synchronizeEvent( const CUevent event ) const
{
    common::CUDAAccess cudaAccess ( mCUDA );

    SCAI_CUDA_DRV_CALL( cuEventSynchronize( event ), "cuEventSynchronize failed for CUevent " << event << '.' )
}

CUDAStreamSyncToken* CUDAStreamSyncToken::getCurrentSyncToken()
{
    SyncToken* syncToken = SyncToken::getCurrentSyncToken();

    if ( syncToken == NULL )
    {
        return NULL;
    }

    // make a dynamic CAST

    CUDAStreamSyncToken* cudaStreamSyncToken = dynamic_cast<CUDAStreamSyncToken*>( syncToken );

    // If the current sync token is not a CUDA stream token it is very likely an error

    if ( cudaStreamSyncToken == NULL )
    {
        SCAI_LOG_ERROR( logger, "Current sync token = " << *syncToken << " not CUDAStreamSyncToken as expected" )
    }

    // But might not be too serious so probably NULL results in synchronous execution

    return cudaStreamSyncToken;
}

} /* end namespace tasking */

} /* end namespace scai */