
#include <memory/memory.hpp>

#include <thrust/reduce.h>
#include <thrust/device_vector.h>

#include <logging/logging.hpp>

#include <cudamem/CUDAError.hpp>

#include <iostream>

using namespace memory;

LAMA_LOG_DEF_LOGGER( logger, "CudaExample" )

template<typename ValueType>
ValueType sum( const ValueType array[], const IndexType n )
{
    thrust::device_ptr<ValueType> data( const_cast<ValueType*>( array ) );

    ValueType zero = static_cast<ValueType>( 0 );

    ValueType result = thrust::reduce( data, data + n, zero, thrust::plus<ValueType>() );

    LAMA_CUDA_RT_CALL( cudaStreamSynchronize( 0 ), "cudaStreamSynchronize( 0 )" );

    LAMA_LOG_INFO( logger, "sum of " << n << " values = " << result )

    return result;
}

template<typename ValueType>
__global__
void add_kernel( ValueType* array, IndexType n )
{
    const IndexType i = blockIdx.x * blockDim.x + threadIdx.x;

    ValueType one = 1;

    if ( i < n )
    {
        array[i] += one;
    }
}

template<typename ValueType>
void add( ValueType array[], const IndexType n )
{
    const int blockSize = 256;
    const int nblocks   = ( n + blockSize - 1 ) / blockSize;

    dim3 block( blockSize, 1, 1 );
    dim3 grid( nblocks, 1, 1 );

    add_kernel<<<grid, block>>>( array, n );
}

int main()
{
    std::cout << "try to get " << context::CUDA << " context from factory" << std::endl;
    ContextPtr cudaContext = Context::getContext( context::CUDA );
    std::cout << "cudaContext = " << *cudaContext << std::endl;

    std::cout << "try to get " << context::CUDAHost << " context from factory" << std::endl;
    ContextPtr cudaHostContext = Context::getContext( context::CUDAHost );
    std::cout << "cudaHostContext = " << *cudaContext << std::endl;

    const IndexType N = 100;

    LAMAArray<double> data;
    
    std::cout << "data = " << data << std::endl;

    {
        LAMA_LOG_INFO( logger, "write only on cuda host" )
        WriteOnlyAccess<double> write( data, cudaHostContext, N );
        double* v = write.get();
        for ( IndexType i = 0; i < N; ++i )
        {
            v[i] = 1.0;
        }
    }

    {
        LAMA_LOG_INFO( logger, "read on cuda" )
        ReadAccess<double> read( data, cudaContext );
        LAMA_CONTEXT_ACCESS( cudaContext )
        double s = sum( read.get(), data.size() );
        std::cout << "sum = " << s << ", should be " << N  << std::endl;
    }

    {
        LAMA_LOG_INFO( logger, "write on cuda" )
        WriteAccess<double> write( data, cudaContext );
        LAMA_CONTEXT_ACCESS( cudaContext )
        add( write.get(), data.size() );
    }

    {
        LAMA_LOG_INFO( logger, "read on host" )
        HostReadAccess<double> read( data );
        for ( IndexType i = 0; i < N; ++i )
        {
            COMMON_ASSERT_EQUAL( read[i], 2.0, "wrong value after add" )
        }
    }
}
