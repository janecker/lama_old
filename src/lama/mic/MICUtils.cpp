/**
 * @file MICUtils.cpp
 *
 * @license
 * Copyright (c) 2009-2013
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
 * @brief Implementation of CSR utilities with MIC for the Inteface
 * @author Thomas Brandes
 * @date 02.07.2012
 * @since 1.1.0
 */

// hpp
#include <lama/mic/MICUtils.hpp>

// others
#include <lama/mic/MICContext.hpp>
#include <lama/LAMAInterfaceRegistry.hpp>
#include <lama/tracing.hpp>

namespace lama
{

LAMA_LOG_DEF_LOGGER( MICUtils::logger, "MIC.Utils" )

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void MICUtils::scale( ValueType array[], const ValueType value, const IndexType n )
{
    LAMA_LOG_INFO( logger, "scale, #n = " << n << ", value = " << value )

    if ( value == 1 )
    {
        return;
    }

    if ( value == 0 )
    {
        ValueType zero = 0;

        setVal( array, n, zero );
    }
    else
    {
        size_t arrayPtr = ( size_t ) array;

        int device = MICContext::getCurrentDevice();

        #pragma offload target( mic : device ) in( arrayPtr, value, n )
        {
           ValueType* array = ( ValueType* ) arrayPtr;

           #pragma omp parallel for 
           for ( IndexType i = 0; i < n; i++ )
           {
               array[i] *= value;
           }
        }
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType,typename OtherValueType>
void MICUtils::setScale( ValueType outValues[], 
                            const ValueType value,
                            const OtherValueType inValues[], 
                            const IndexType n )
{
    LAMA_LOG_INFO( logger, "setScale, #n = " << n << ", value = " << value )

    // alias of outValues == inValues is no problem

    if ( value == static_cast<ValueType>( 0 ) )
    {
        // Important : inValues might be undefined
        setVal( outValues, n, value );
        return;
    }

    if ( value == static_cast<ValueType>( 1 ) )
    {
        set( outValues, inValues, n );
        return;
    }

    size_t outPtr = ( size_t ) outValues;
    size_t inPtr = ( size_t ) inValues;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ) in( outPtr, inPtr, value, n )
    {
        ValueType* outValues = ( ValueType* ) outPtr;
        const OtherValueType* inValues = ( OtherValueType* ) inPtr;

        #pragma omp parallel for 
        for ( IndexType i = 0; i < n; i++ )
        {
            outValues[i] = static_cast<ValueType>( inValues[i] ) * value;
        }
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType MICUtils::sum( const ValueType array[], const IndexType n )
{
    LAMA_LOG_INFO( logger, "sum # array = " << array << ", n = " << n )

    ValueType val = 0;

    size_t arrayPtr = ( size_t ) array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ) in( arrayPtr, n ), out( val )
    { 
        val = 0;

        const ValueType* array = ( ValueType* ) arrayPtr;

        #pragma omp parallel for reduction( +:val )
        for ( IndexType i = 0; i < n; ++i )
        {
            val += array[i];
        }
    }

    return val;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void MICUtils::setVal( ValueType array[], const IndexType n, const ValueType val )
{
    LAMA_LOG_DEBUG( logger, "setVal<" << Scalar::getType<ValueType>() << ">: " << "array[" << n << "] = " << val )

    size_t arrayPtr = ( size_t ) array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ), in( arrayPtr, n, val )
    {
        ValueType* array = ( ValueType* ) arrayPtr;

        #pragma omp parallel for
        for ( IndexType i = 0; i < n; ++i )
        {
            array[i] = val;
        }
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void MICUtils::setOrder( ValueType array[], const IndexType n )
{
    LAMA_LOG_DEBUG( logger,
                    "setOrder<" << Scalar::getType<ValueType>() << ">: " 
                     << "array[" << n << "] = 0, 1, 2, ..., " << ( n - 1 ) )

    void* arrayPtr = array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device), in ( arrayPtr, n )
    {
        ValueType* array = static_cast<ValueType*>( arrayPtr );

        #pragma omp parallel for
        for ( IndexType i = 0; i < n; ++i )
        {
            array[i] = static_cast<ValueType>( i );
        }
    } 
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType MICUtils::getValue( const ValueType* array, const IndexType i )
{
    LAMA_LOG_DEBUG( logger, "getValue<" << Scalar::getType<ValueType>() << ">: i = " << i )

    ValueType val = 0;

    const void* arrayPtr = array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ), in( arrayPtr, i ), out( val )
    {
        const ValueType* array = static_cast<const ValueType*>( arrayPtr );
        val = array[i];
    }
    return val;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType MICUtils::maxval( const ValueType array[], const IndexType n )
{
    LAMA_LOG_INFO( logger, "maxval<" << Scalar::getType<ValueType>() << ">: " << "array[" << n << "]" )

    ValueType val = 0;

    size_t arrayPtr = ( size_t ) array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ) in( arrayPtr, n ), out( val )
    { 
        val = 0;

        ValueType* array = ( ValueType* ) arrayPtr;

        #pragma omp parallel
        {
    
            ValueType threadVal = 0;

            #pragma omp for 
            for ( IndexType i = 0; i < n; ++i )
            {
                if ( array[i] > threadVal )
                {
                    threadVal = array[i];
                }
            }
    
            #pragma omp critical
            {
                if ( threadVal > val )
                {
                    val = threadVal;
                }
            }
        }
    }

    return val;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType MICUtils::absMaxVal( const ValueType array[], const IndexType n )
{
    LAMA_LOG_INFO( logger, "absMaxVal<" << Scalar::getType<ValueType>() << ">: " << "array[" << n << "]" )
    LAMA_LOG_INFO( logger, "array = " << array )

    ValueType val = static_cast<ValueType>( 0.0 );

    // array is already on MIC device

    size_t ptr = ( size_t ) array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ) in( n, ptr ), inout( val )
    {
        ValueType* array = ( ValueType* ) ptr;

        #pragma omp parallel
        {
            ValueType threadVal = static_cast<ValueType>( 0.0 );

            #pragma omp for 
            for ( IndexType i = 0; i < n; ++i )
            {
                ValueType elem = std::abs( array[i] );
    
                if ( elem > threadVal )
                {
                    threadVal = elem;
                }
            }
    
            #pragma omp critical
            {
                if ( threadVal > val )
                {
                    val = threadVal;
                }
            }
        }
    }

    return val;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType MICUtils::absMaxDiffVal( const ValueType array1[], const ValueType array2[], const IndexType n )
{
    LAMA_LOG_DEBUG( logger, "absMaxDiffVal<" << Scalar::getType<ValueType>() << ">: " << "array[" << n << "]" )

    ValueType val = static_cast<ValueType>( 0.0 );

    #pragma omp parallel
    {
        ValueType threadVal = static_cast<ValueType>( 0.0 );

        #pragma omp for 
        for ( IndexType i = 0; i < n; ++i )
        {
            ValueType elem = std::abs( array1[i] - array2[i] );

            if ( elem > threadVal )
            {
                threadVal = elem;
            }
        }

        #pragma omp critical
        {
            LAMA_LOG_TRACE( logger, "max val of thread  = " << threadVal << ", global was " << val )

            if ( threadVal > val )
            {
                val = threadVal;
            }
        }
    }

    return val;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
bool MICUtils::isSorted( const ValueType array[], const IndexType n, bool ascending )
{
    LAMA_LOG_INFO( logger, "isSorted<" << Scalar::getType<ValueType>() 
                           << ">, n = " << n << ", ascending = " << ascending )

    bool sorted = true;   //!< will be set to false at violations

    const void* arrayPtr = array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ) in( arrayPtr, n, ascending ), out( sorted )
    {
        const ValueType* array = static_cast<const ValueType*>( arrayPtr );

        sorted = true;

        if ( ascending )
        {
            #pragma omp parallel for
            for ( IndexType i = 1; i < n; i++ )
            {
                if ( array[i-1] > array[i] && sorted )
                {
                    sorted = false;
                }
            }
        }
        else
        {
            #pragma omp parallel for
            for ( IndexType i = 1; i < n; i++ )
            {
                if ( array[i-1] < array[i] && sorted )
                {
                    sorted = false;
                }
            }
        }
    }

    return sorted;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType1,typename ValueType2>
void MICUtils::set( ValueType1 out[], const ValueType2 in[], const IndexType n )
{
    LAMA_LOG_INFO( logger, "set: out<" << Scalar::getType<ValueType1>() << "[" << n << "]" 
                            << " = in<" << Scalar::getType<ValueType2>() << ">[" << n << "]" )

    void* outPtr = out;
    const void* inPtr = in;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ), in( outPtr, inPtr, n )
    {
        ValueType1* out = static_cast<ValueType1*>( outPtr );
        const ValueType2* in = static_cast<const ValueType2*>( inPtr );

        #pragma omp parallel for
        for ( IndexType i = 0; i < n; i++ )
        {
            out[i] = static_cast<ValueType1>( in[i] );
        }
    }
}

/* --------------------------------------------------------------------------- */

bool MICUtils::validIndexes( const IndexType array[], const IndexType n, const IndexType size )
{
    LAMA_LOG_DEBUG( logger, "validIndexes: array[" << n << "], size " << size )

    bool validFlag = true;

    size_t arrayPtr = ( size_t ) array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ), in ( n, arrayPtr, size ), inout( validFlag )
    {
        const IndexType* array = ( IndexType* ) arrayPtr;

        #pragma omp parallel for reduction( & : validFlag )
        for ( IndexType i = 0; i < n; i++ )
        {
            if ( size <= array[i] || 0 > array[i] )
            {
                validFlag = false;
            }
        }
    }

    return validFlag;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType1,typename ValueType2>
void MICUtils::setGather( ValueType1 out[], const ValueType2 in[], const IndexType indexes[], const IndexType n )
{
    LAMA_LOG_DEBUG( logger, "setGather: out<" << Scalar::getType<ValueType1>() << ">[" << n << "]" 
                        << " = in<" << Scalar::getType<ValueType2>() << ">[ indexes[" << n << "] ]" )

    size_t outPtr = ( size_t ) out;
    size_t inPtr = ( size_t ) in;
    size_t indexesPtr = ( size_t ) indexes;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ) in( inPtr, outPtr, indexesPtr, n )
    {
        ValueType1* out = ( ValueType1* ) outPtr;
        const ValueType2* in = ( ValueType2* ) inPtr;
        const IndexType* indexes = ( IndexType* ) indexesPtr;

        #pragma omp parallel for 
        for ( IndexType i = 0; i < n; i++ )
        {
            out[i] = static_cast<ValueType1>( in[indexes[i]] );
        }
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType1,typename ValueType2>
void MICUtils::setScatter( ValueType1 out[], const IndexType indexes[], const ValueType2 in[], const IndexType n )
{
    LAMA_LOG_DEBUG( logger,
                    "setScatter: out<" << Scalar::getType<ValueType1>() << ">" 
                     << "[ indexes[" << n << "] ]" << " = in<" << Scalar::getType<ValueType2>() << ">[" << n << "]" )

    void* outPtr = out;
    const void* indexesPtr = indexes;
    const void* inPtr = in;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( mic : device ) in( outPtr, indexesPtr, inPtr, n )
    {
        ValueType1* out = static_cast<ValueType1*>( outPtr );
        const ValueType2* in = static_cast<const ValueType2*>( inPtr );
        const IndexType* indexes = static_cast<const IndexType*>( indexesPtr );

        #pragma omp parallel for
        for ( IndexType i = 0; i < n; i++ )
        {
            out[indexes[i]] = static_cast<ValueType1>( in[i] );
        }
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void MICUtils::invert( ValueType array[], const IndexType n )
{
    LAMA_REGION( "MIC.invert" )

    LAMA_LOG_INFO( logger, "invert array[ " << n << " ]" )

    size_t array_ptr = ( size_t ) array;

    int device = MICContext::getCurrentDevice();

    #pragma offload target( MIC : device ), in ( n, array_ptr )
    {
        ValueType* array = ( ValueType* ) array_ptr;

        ValueType one = 1;

        #pragma omp parallel for 
        for ( IndexType i = 0; i < n; ++i )
        {
            array[i] = one / array[i];
        }
    }
}

/* --------------------------------------------------------------------------- */
/*     Template instantiations via registration routine                        */
/* --------------------------------------------------------------------------- */

void MICUtils::setInterface( UtilsInterface& Utils )
{
    LAMA_LOG_INFO( logger, "set Utils routines for MIC in Interface" )

    LAMA_INTERFACE_REGISTER( Utils, validIndexes )

    LAMA_INTERFACE_REGISTER_T( Utils, scale, float )
    LAMA_INTERFACE_REGISTER_T( Utils, scale, double )

    LAMA_INTERFACE_REGISTER_TT( Utils, setScale, float, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, setScale, double, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, setScale, float, double )
    LAMA_INTERFACE_REGISTER_TT( Utils, setScale, double, double )

    LAMA_INTERFACE_REGISTER_T( Utils, sum, IndexType )
    LAMA_INTERFACE_REGISTER_T( Utils, sum, float )
    LAMA_INTERFACE_REGISTER_T( Utils, sum, double )

    LAMA_INTERFACE_REGISTER_T( Utils, setVal, IndexType )
    LAMA_INTERFACE_REGISTER_T( Utils, setVal, float )
    LAMA_INTERFACE_REGISTER_T( Utils, setVal, double )

    LAMA_INTERFACE_REGISTER_T( Utils, setOrder, IndexType )

    LAMA_INTERFACE_REGISTER_T( Utils, getValue, IndexType )
    LAMA_INTERFACE_REGISTER_T( Utils, getValue, float )
    LAMA_INTERFACE_REGISTER_T( Utils, getValue, double )

    LAMA_INTERFACE_REGISTER_T( Utils, maxval, IndexType )
    LAMA_INTERFACE_REGISTER_T( Utils, maxval, float )
    LAMA_INTERFACE_REGISTER_T( Utils, maxval, double )

    LAMA_INTERFACE_REGISTER_T( Utils, absMaxVal, float )
    LAMA_INTERFACE_REGISTER_T( Utils, absMaxVal, double )

    /*
    LAMA_INTERFACE_REGISTER_T( Utils, absMaxDiffVal, float )
    LAMA_INTERFACE_REGISTER_T( Utils, absMaxDiffVal, double )
    */

    LAMA_INTERFACE_REGISTER_T( Utils, isSorted, IndexType )
    LAMA_INTERFACE_REGISTER_T( Utils, isSorted, float )
    LAMA_INTERFACE_REGISTER_T( Utils, isSorted, double )

    LAMA_INTERFACE_REGISTER_TT( Utils, setScatter, int, int )

    LAMA_INTERFACE_REGISTER_TT( Utils, setScatter, float, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, setScatter, double, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, setScatter, float, double )
    LAMA_INTERFACE_REGISTER_TT( Utils, setScatter, double, double )

    LAMA_INTERFACE_REGISTER_TT( Utils, setGather, int, int )

    LAMA_INTERFACE_REGISTER_TT( Utils, setGather, float, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, setGather, float, double )
    LAMA_INTERFACE_REGISTER_TT( Utils, setGather, double, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, setGather, double, double )

    LAMA_INTERFACE_REGISTER_TT( Utils, set, int, int )

    LAMA_INTERFACE_REGISTER_TT( Utils, set, float, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, set, float, double )
    LAMA_INTERFACE_REGISTER_TT( Utils, set, double, float )
    LAMA_INTERFACE_REGISTER_TT( Utils, set, double, double )

    LAMA_INTERFACE_REGISTER_T( Utils, invert, float )
    LAMA_INTERFACE_REGISTER_T( Utils, invert, double )
}

/* --------------------------------------------------------------------------- */
/*    Static registration of the Utils routines                                */
/* --------------------------------------------------------------------------- */

bool MICUtils::registerInterface()
{
    LAMAInterface& interface = LAMAInterfaceRegistry::getRegistry().modifyInterface( Context::MIC );
    setInterface( interface.Utils );
    return true;
}

/* --------------------------------------------------------------------------- */
/*    Static initialiazion at program start                                    */
/* --------------------------------------------------------------------------- */

bool MICUtils::initialized = registerInterface();

} // namespace lama