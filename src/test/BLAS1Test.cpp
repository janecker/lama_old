/**
 * @file BLAS1Test.cpp
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
 * @brief Contains tests for the class CUDABLAS1 and OpenMPBLAS1
 * @author: Bea Hornef
 * @date 5.7.2013
 * @since 1.0.0
 **/

// math for sqrt
#include <cmath>

// boost
#include <boost/test/unit_test.hpp>

// others
#include <lama/ContextAccess.hpp>
#include <lama/HostReadAccess.hpp>
#include <lama/LAMAArray.hpp>
#include <lama/LAMAInterface.hpp>
#include <lama/ReadAccess.hpp>
#include <lama/Scalar.hpp>
#include <lama/WriteAccess.hpp>

#include <test/TestMacros.hpp>

using namespace boost;
using namespace lama;

namespace lama
{
namespace BLAS1Test
{

template<typename ValueType>
void asumTest( ContextPtr loc )
{
    LAMA_INTERFACE_FN_T( asum, loc, BLAS, BLAS1, ValueType );

    {
        ValueType values[] =
        { 1.0, 2.0, -3.0, 4.0, 5.0, -6.0 };
        const IndexType nValues = sizeof( values ) / sizeof( ValueType );
        const IndexType incX1 = 1;
        const IndexType incX2 = 2;
        const ValueType result1 = 21.0;
        const ValueType result2 = 9.0;

        LAMAArray<ValueType> AValues( nValues, values );

        {
            LAMA_CONTEXT_ACCESS( loc );

            // std::cout << "test 1 (incX = 1)" << std::endl;
            ReadAccess<ValueType> rAValues( AValues, loc );
        	ValueType sum = asum( nValues / incX1, rAValues.get(), incX1, NULL );
            BOOST_CHECK_EQUAL( sum, result1);

            // std::cout << "test 2 (incX = 2)" << std::endl;
        	sum = asum( nValues / incX2, rAValues.get(), incX2, NULL );
        	BOOST_CHECK_EQUAL( sum, result2);
        }
    }
}  // asumTest

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void nrm2Test( ContextPtr loc )
{
    LAMA_INTERFACE_FN_T( nrm2, loc, BLAS, BLAS1, ValueType );

    {
        ValueType values[] =
        { 1, 2, 3, 4, 5, 6 };
        const IndexType nValues = sizeof( values ) / sizeof( ValueType );
        const IndexType incX1 = 1;
        const IndexType incX2 = 2;
        const ValueType result1 = 91.0;
        const ValueType result2 = 35.0;

        LAMAArray<ValueType> AValues( nValues, values );

        {
            LAMA_CONTEXT_ACCESS( loc );

            // std::cout << "test 1 (incX = 1)" << std::endl;
            ReadAccess<ValueType> rAValues( AValues, loc );
        	ValueType euclideanNorm = nrm2( nValues / incX1, rAValues.get(), incX1, NULL );
            BOOST_CHECK_CLOSE( euclideanNorm, ::sqrt(result1), 1e-4 );

            // std::cout << "test 2 (incX = 2)" << std::endl;
        	euclideanNorm = nrm2( nValues / incX2, rAValues.get(), incX2, NULL );
        	BOOST_CHECK_CLOSE( euclideanNorm, ::sqrt(result2), 1e-4 );
        }
    }
}  // nrm2Test

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void iamaxTest( ContextPtr loc )
{
    LAMA_INTERFACE_FN_T( iamax, loc, BLAS, BLAS1, ValueType );

    {
        ValueType values[] =
        { 1, 2, 3, 6, 5, 6 };
        const IndexType nValues = sizeof( values ) / sizeof( ValueType );
        const IndexType incX1 = 1;
        const IndexType incX2 = 2; // { 1, 3, 5}
        const IndexType result1 = 3;
        const IndexType result2 = 2;

        LAMAArray<ValueType> AValues( nValues, values );

        {
            LAMA_CONTEXT_ACCESS( loc );

            // std::cout << "test 1 (incX = 1) " << std::endl;
            ReadAccess<ValueType> rAValues( AValues, loc );
            IndexType smallestIndexOfMax = iamax( nValues / incX1, rAValues.get(), incX1, NULL );
            BOOST_CHECK_EQUAL( smallestIndexOfMax, result1 );

            // std::cout << "test 2 (incX = 2)" << std::endl;
            smallestIndexOfMax = iamax( nValues / incX2, rAValues.get(), incX2, NULL );
            BOOST_CHECK_EQUAL( smallestIndexOfMax, result2 );
        }
    }
}  // iamaxTest

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void swapTest( ContextPtr loc )
{
    LAMA_INTERFACE_FN_T( swap, loc, BLAS, BLAS1, ValueType );

    {
        ValueType values1[] = { 1, 2, 3, 4, 5};
        ValueType values2[] = { 7, 6, 5, 4, 3, 2, 1};

        const IndexType nValues = 3;
        const IndexType incX = 2;
        const IndexType incY = 3;


        LAMAArray<ValueType> AValues1( 5, values1 );
        LAMAArray<ValueType> AValues2( 7, values2 );

        {
            LAMA_CONTEXT_ACCESS( loc );

            WriteAccess<ValueType> wAValues1( AValues1, loc );
            WriteAccess<ValueType> wAValues2( AValues2, loc );

            swap( nValues, wAValues1.get(), incX, wAValues2.get(), incY, NULL );
        }

        {
            HostReadAccess<ValueType> rAValues1( AValues1 );
            HostReadAccess<ValueType> rAValues2( AValues2 );
            BOOST_CHECK_EQUAL( 7, rAValues1[0] );
            BOOST_CHECK_EQUAL( 1, rAValues2[0] );
            BOOST_CHECK_EQUAL( 4, rAValues1[2] );
            BOOST_CHECK_EQUAL( 3, rAValues2[3] );
            BOOST_CHECK_EQUAL( 1, rAValues1[4] );
            BOOST_CHECK_EQUAL( 5, rAValues2[6] );
        }

    }
}  // swapTest

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void viamaxTest( ContextPtr loc )
{
    LAMA_INTERFACE_FN_T( viamax, loc, BLAS, BLAS1, ValueType );

    {
        ValueType values[] =
        { 1, 2, 3, 6, 5, 6, 10, 10, -10, -10 };
        const IndexType incX1 = 1;
        const IndexType incX2 = 2;
        const ValueType result1 = 6.0;
        const ValueType result2 = 5.0;

        LAMAArray<ValueType> AValues( 10, values );

        {
            LAMA_CONTEXT_ACCESS( loc );

            // std::cout << "test 1 (incX = 1)" << std::endl;
            ReadAccess<ValueType> rAValues( AValues, loc );
            ValueType max = viamax( 6, rAValues.get(), incX1, NULL );
            BOOST_CHECK_EQUAL( max, result1 );

            // std::cout << "test 2 (incX = 2)" << std::endl;
            max = viamax( 3, rAValues.get(), incX2, NULL );
            BOOST_CHECK_EQUAL( max, result2 );
        }
    }
}  // viamaxTest

} // namespace BLAS1Test
} // namespace lama

/* ------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE( BLAS1Test );

LAMA_LOG_DEF_LOGGER( logger, "Test.BLAS1Test" );

LAMA_AUTO_TEST_CASE_T( asumTest, BLAS1Test );
LAMA_AUTO_TEST_CASE_T( nrm2Test, BLAS1Test );
LAMA_AUTO_TEST_CASE_T( iamaxTest, BLAS1Test );
LAMA_AUTO_TEST_CASE_T( swapTest, BLAS1Test );
LAMA_AUTO_TEST_CASE_T( viamaxTest, BLAS1Test );

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE_END();