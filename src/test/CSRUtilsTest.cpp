/**
 * @file CSRUtilsTest.cpp
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
 * @brief Contains tests for the CSRUtils interface to be tested on different devices
 * @author: Thomas Brandes
 * @date 05.07.2013
 * @since 1.0.1
 **/

// boost
#include <boost/test/unit_test.hpp>

// others
#include <lama/ContextAccess.hpp>
#include <lama/LAMAArray.hpp>
#include <lama/LAMAInterface.hpp>
#include <lama/ReadAccess.hpp>
#include <lama/WriteAccess.hpp>

#include <test/TestMacros.hpp>

using namespace boost;
using namespace lama;

/* ------------------------------------------------------------------------------------------------------------------ */

// Dummy type, needed to use the lama interface
typedef bool NoType;

/* ------------------------------------------------------------------------------------------------------------------ */

namespace lama
{
namespace CSRUtilsTest
{

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType, typename OtherValueType>
void absMaxDiffValTest( ContextPtr loc )
{
    try
    {

       LAMA_INTERFACE_FN_T( absMaxDiffVal, loc, CSRUtils, Reductions, ValueType );

        // input arrays
     
        //    Array1             Array2
        //
        //    1 2 3 0 0          1 2 0 0 0
        //    0 0 1 1 2          1 0 2 2 1

        const IndexType ia1[] = { 0, 3, 6 };
        const IndexType ja1[] = { 0, 1, 2, 2, 3, 4 };
        const IndexType ia2[] = { 0, 2, 6 };
        const IndexType ja2[] = { 0, 1, 0, 2, 3, 4 };
        const ValueType values1[] = { 1, 2, 3, 1, 1, 2 };
        const ValueType values2[] = { 1, 2, 1, 2, 2, 1 };

        const IndexType numRows = 2;
        // const IndexType numColumns = 5;
        const IndexType numValues1 = sizeof( ja1 ) / sizeof( IndexType );
        const IndexType numValues2 = sizeof( ja2 ) / sizeof( IndexType );

        LAMAArray<IndexType> csrIA1( numRows + 1, ia1 );
        LAMAArray<IndexType> csrJA1( numValues1, ja1 );
        LAMAArray<ValueType> csrValues1( numValues1, values1 );

        LAMAArray<IndexType> csrIA2( numRows + 1, ia2 );
        LAMAArray<IndexType> csrJA2( numValues2, ja2 );
        LAMAArray<ValueType> csrValues2( numValues2, values2 );

        ReadAccess<IndexType> rCSRIA1( csrIA1, loc );
        ReadAccess<IndexType> rCSRJA1( csrJA1, loc );
        ReadAccess<ValueType> rCSRValues1( csrValues1, loc );

        ReadAccess<IndexType> rCSRIA2( csrIA2, loc );
        ReadAccess<IndexType> rCSRJA2( csrJA2, loc );
        ReadAccess<ValueType> rCSRValues2( csrValues2, loc );

        LAMA_CONTEXT_ACCESS( loc );

        ValueType maxVal = absMaxDiffVal( numRows, false,
                                          rCSRIA1.get(), rCSRJA1.get(), rCSRValues1.get(),
                                          rCSRIA2.get(), rCSRJA2.get(), rCSRValues2.get() );

        BOOST_CHECK_EQUAL( 3, maxVal );

        // rows are sorted, so we can alsoy apply sortFlag = true

        maxVal = absMaxDiffVal( numRows, true,
                                rCSRIA1.get(), rCSRJA1.get(), rCSRValues1.get(),
                                rCSRIA2.get(), rCSRJA2.get(), rCSRValues2.get() );

        BOOST_CHECK_EQUAL( 3, maxVal );
    }
    catch( Exception )
    {
        std::cout <<  "WARN: CSRUtils::absMaxDiffVal not available on " << *loc << ", not tested" << std::endl;
        return;
    }

}

}//namespace CSRUtilsTest
} //namespace lama

/* ------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE( CSRUtilsTest );

LAMA_LOG_DEF_LOGGER( logger, "Test.CSRUtilsTest" );

LAMA_AUTO_TEST_CASE_TT( absMaxDiffValTest, CSRUtilsTest );

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE_END();