/**
 * @file JDSUtilsTest.cpp
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
 * @brief Contains tests for the class CUDAJDSUtils and OpenMPJDSUtils
 * @author: Jan Ecker
 * @date 16.10.2012
 * @since 1.0.0
 **/

// boost
#include <boost/test/unit_test.hpp>

// others
#include <scai/kregistry.hpp>
#include <scai/sparsekernel/JDSKernelTrait.hpp>
#include <scai/hmemo.hpp>
#include <scai/sparsekernel/test/TestMacros.hpp>

/*--------------------------------------------------------------------- */

using namespace scai;
using namespace hmemo;
using namespace sparsekernel;
using namespace kregistry;
using common::TypeTraits;
using common::Exception;

/* --------------------------------------------------------------------- */

BOOST_AUTO_TEST_SUITE( JDSUtilsTest )

/* --------------------------------------------------------------------- */

SCAI_LOG_DEF_LOGGER( logger, "Test.JDSUtilsTest" )

/* ------------------------------------------------------------------------------------- */

BOOST_AUTO_TEST_CASE_TEMPLATE( getRowTest, ValueType, scai_arithmetic_test_types )
{
    typedef float OtherValueType;

    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::getRow<ValueType, OtherValueType> > getRow;

    ContextPtr loc = Context::getContextPtr( getRow.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    ValueType valuesValues[] =
    { 1, 7, 12, 2, 8, 13, 3, 9, 14, 4, 10, 15, 5, 11, 6 };
    const IndexType nValues = sizeof( valuesValues ) / sizeof( ValueType );
    IndexType valuesJa[] =
    { 0, 1, 5, 2, 3, 7, 4, 5, 12, 6, 7, 15, 8, 9, 10 };
    const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
    IndexType valuesDlg[] =
    { 3, 3, 3, 3, 2, 1 };
    const IndexType nDlg = sizeof( valuesDlg ) / sizeof( IndexType );
    IndexType valuesIlg[] =
    { 6, 5, 4 };
    const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
    IndexType valuesPerm[] =
    { 1, 2, 0 };
    const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
    OtherValueType expectedValues[] =
    { 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 0, 0, 0, 0, 0 };
    const IndexType i = 2;
    const IndexType numColumns = 16;
    const IndexType numRows = 3;
    HArray<ValueType> values( nValues );
    HArray<IndexType> ja( nJa );
    HArray<IndexType> dlg( nDlg );
    HArray<IndexType> ilg( nIlg );
    HArray<IndexType> perm( nPerm );
    HArray<OtherValueType> row( numColumns );

    initArray( values, valuesValues, nValues );
    initArray( ja, valuesJa, nJa );
    initArray( dlg, valuesDlg, nDlg );
    initArray( ilg, valuesIlg, nIlg );
    initArray( perm, valuesPerm, nPerm );
    initArray( row, OtherValueType(0), numColumns );

    ReadAccess<ValueType> rValues( values, loc );
    ReadAccess<IndexType> rJa( ja, loc );
    ReadAccess<IndexType> rDlg( dlg, loc );
    ReadAccess<IndexType> rIlg( ilg, loc );
    ReadAccess<IndexType> rPerm( perm, loc );
    {
        WriteOnlyAccess<OtherValueType> wRow( row, loc, numColumns );
        SCAI_CONTEXT_ACCESS( loc );
        getRow[loc->getType()]( wRow.get(), i, numColumns, numRows, rPerm.get(), rIlg.get(), rDlg.get(), rJa.get(), rValues.get() );
    }
    ReadAccess<OtherValueType> rRow( row );

    for ( IndexType i = 0; i < numColumns; i++ )
    {
        BOOST_CHECK_EQUAL( expectedValues[i], rRow[i] );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( getValueTest, ValueType, scai_arithmetic_test_types )
{
    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::getValue<ValueType> > getValue;

    ContextPtr loc = Context::getContextPtr( getValue.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    ValueType valuesValues[] =
    { 1, 5, 4, 3, 1, 3, 2, 2, 2, 8, 4, 9, 9, 7, 8, 7, 2 };
    const IndexType nValues = sizeof( valuesValues ) / sizeof( ValueType );
    IndexType valuesJa[] =
    { 0, 0, 2, 2, 0, 1, 3, 5, 4, 1, 3, 7, 6, 6, 9, 9, 9 };
    const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
    IndexType valuesDlg[] =
    { 5, 5, 3, 3, 1 };
    const IndexType nDlg = sizeof( valuesDlg ) / sizeof( IndexType );
    IndexType valuesIlg[] =
    { 5, 4, 4, 2, 2 };
    const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
    IndexType valuesPerm[] =
    { 0, 2, 3, 1, 4 };
    const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
    ValueType expectedValues[5][10] =
    {
        { 1, 3, 0, 4, 0, 0, 7, 0, 0, 2 },
        { 0, 0, 3, 0, 2, 0, 0, 0, 0, 0 },
        { 5, 0, 0, 2, 0, 0, 0, 9, 0, 8 },
        { 0, 0, 4, 0, 0, 2, 9, 0, 0, 7 },
        { 1, 8, 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    const IndexType numColumns = 10;
    const IndexType numRows = 5;
    HArray<ValueType> values( nValues );
    HArray<IndexType> ja( nJa );
    HArray<IndexType> dlg( nDlg );
    HArray<IndexType> ilg( nIlg );
    HArray<IndexType> perm( nPerm );

    initArray( values, valuesValues, nValues );
    initArray( ja, valuesJa, nJa );
    initArray( dlg, valuesDlg, nDlg );
    initArray( ilg, valuesIlg, nIlg );
    initArray( perm, valuesPerm, nPerm );

    ReadAccess<ValueType> rValues( values, loc );
    ReadAccess<IndexType> rJa( ja, loc );
    ReadAccess<IndexType> rDlg( dlg, loc );
    ReadAccess<IndexType> rIlg( ilg, loc );
    ReadAccess<IndexType> rPerm( perm, loc );

    for ( IndexType i = 0; i < numRows; i++ )
    {
        for ( IndexType j = 0; j < numColumns; j++ )
        {
            SCAI_CONTEXT_ACCESS( loc );
            ValueType value = getValue[loc->getType()]( i, j, numRows, rDlg.get(), rIlg.get(), rPerm.get(), rJa.get(), rValues.get() );
            BOOST_CHECK_EQUAL( expectedValues[i][j], value );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( scaleValueTest, ValueType, scai_arithmetic_test_types )
{
    typedef float OtherValueType;

    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::scaleValue<ValueType, OtherValueType> > scaleValue;

    ContextPtr loc = Context::getContextPtr( scaleValue.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    ValueType valuesValues[] =
    { 1, 7, 12, 2, 8, 13, 3, 9, 14, 4, 10, 15, 5, 11, 6 };
    const IndexType nValues = sizeof( valuesValues ) / sizeof( ValueType );
    IndexType valuesDlg[] =
    { 3, 3, 3, 3, 2, 1 };
    const IndexType nDlg = sizeof( valuesDlg ) / sizeof( IndexType );
    IndexType valuesIlg[] =
    { 6, 5, 4 };
    const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
    IndexType valuesPerm[] =
    { 1, 2, 0 };
    const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
    OtherValueType valuesDiagonal[] =
    { 3, 1, 2 };
    const IndexType nDiagonal = sizeof( valuesDiagonal ) / sizeof( OtherValueType );
    ValueType expectedValues[] =
    { 1, 14, 36, 2, 16, 39, 3, 18, 42, 4, 20, 45, 5, 22, 6 };
    const IndexType numRows = 3;
    HArray<ValueType> values( nValues );
    HArray<IndexType> dlg( nDlg );
    HArray<IndexType> ilg( nIlg );
    HArray<IndexType> perm( nPerm );
    HArray<OtherValueType> diagonal( nDiagonal );

    initArray( values, valuesValues, nValues );
    initArray( dlg, valuesDlg, nDlg );
    initArray( ilg, valuesIlg, nIlg );
    initArray( perm, valuesPerm, nPerm );
    initArray( diagonal, valuesDiagonal, nDiagonal );

    ReadAccess<IndexType> rDlg( dlg, loc );
    ReadAccess<IndexType> rIlg( ilg, loc );
    ReadAccess<IndexType> rPerm( perm, loc );
    ReadAccess<OtherValueType> rDiagonal( diagonal, loc );
    {
        WriteAccess<ValueType> wValues( values, loc );
        SCAI_CONTEXT_ACCESS( loc );
        scaleValue[loc->getType()]( numRows, rPerm.get(), rIlg.get(), rDlg.get(), wValues.get(), rDiagonal.get() );
    }
    ReadAccess<ValueType> rValues( values );

    for ( IndexType i = 0; i < nValues; i++ )
    {
        BOOST_CHECK_EQUAL( expectedValues[i], rValues[i] );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE( checkDiagonalPropertyTest )
{
    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::checkDiagonalProperty> checkDiagonalProperty;

    ContextPtr loc = Context::getContextPtr( checkDiagonalProperty.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    // check with matrix without diagonal property
    {
        IndexType valuesJa[] =
        { 0, 1, 5, 2, 3, 7, 4, 5, 12, 6, 7, 15, 8, 9, 10 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        IndexType valuesDlg[] =
        { 3, 3, 3, 3, 2, 1 };
        const IndexType nDlg = sizeof( valuesDlg ) / sizeof( IndexType );
        IndexType valuesIlg[] =
        { 6, 5, 4 };
        const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
        IndexType valuesPerm[] =
        { 1, 2, 0 };
        const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
        const IndexType numRows = 3;
        const IndexType numColumns = 16;
        const IndexType numDiagonals = 3;
        HArray<IndexType> ja( nJa );
        HArray<IndexType> dlg( nDlg );
        HArray<IndexType> ilg( nIlg );
        HArray<IndexType> perm( nPerm );

        initArray( ja, valuesJa, nJa );
        initArray( dlg, valuesDlg, nDlg );
        initArray( ilg, valuesIlg, nIlg );
        initArray( perm, valuesPerm, nPerm );

        ReadAccess<IndexType> rJa( ja, loc );
        ReadAccess<IndexType> rDlg( dlg, loc );
        ReadAccess<IndexType> rIlg( ilg, loc );
        ReadAccess<IndexType> rPerm( perm, loc );
        SCAI_CONTEXT_ACCESS( loc );
        bool diagonalProperty;
        diagonalProperty = checkDiagonalProperty[loc->getType()]( numDiagonals, numRows, numColumns,
                                                       rPerm.get(), rJa.get(), rDlg.get() );
        BOOST_CHECK_EQUAL( false, diagonalProperty );
    }
    // check with matrix with diagonal property
    {
        IndexType valuesJa[] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        IndexType valuesDlg[] =
        { 3, 3, 3 };
        const IndexType nDlg = sizeof( valuesDlg ) / sizeof( IndexType );
        IndexType valuesIlg[] =
        { 3, 3, 3 };
        const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
        IndexType valuesPerm[] =
        { 0, 1, 2 };
        const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
        const IndexType numRows = 3;
        const IndexType numColumns = 3;
        const IndexType numDiagonals = 3;
        HArray<IndexType> ja( nJa );
        HArray<IndexType> dlg( nDlg );
        HArray<IndexType> ilg( nIlg );
        HArray<IndexType> perm( nPerm );

        initArray( ja, valuesJa, nJa );
        initArray( dlg, valuesDlg, nDlg );
        initArray( ilg, valuesIlg, nIlg );
        initArray( perm, valuesPerm, nPerm );

        ReadAccess<IndexType> rJa( ja, loc );
        ReadAccess<IndexType> rDlg( dlg, loc );
        ReadAccess<IndexType> rIlg( ilg, loc );
        ReadAccess<IndexType> rPerm( perm, loc );
        SCAI_CONTEXT_ACCESS( loc );
        bool diagonalProperty;
        diagonalProperty = checkDiagonalProperty[loc->getType()]( numDiagonals, numRows, numColumns,
                                                       rPerm.get(), rJa.get(), rDlg.get() );
        BOOST_CHECK_EQUAL( true, diagonalProperty );
    }
    // check with empty matrix
    {
        const IndexType numRows = 0;
        const IndexType numColumns = 0;
        const IndexType numDiagonals = 0;
        HArray<IndexType> ja;
        HArray<IndexType> dlg;
        HArray<IndexType> ilg;
        HArray<IndexType> perm;
        ReadAccess<IndexType> rJa( ja, loc );
        ReadAccess<IndexType> rDlg( dlg, loc );
        ReadAccess<IndexType> rIlg( ilg, loc );
        ReadAccess<IndexType> rPerm( perm, loc );
        SCAI_CONTEXT_ACCESS( loc );
        bool diagonalProperty = checkDiagonalProperty[loc->getType()]( numDiagonals, numRows, numColumns,
                                                            rPerm.get(), rJa.get(), rDlg.get() );
        BOOST_CHECK_EQUAL( false, diagonalProperty );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE( ilg2dlgTest )
{
    typedef float OtherValueType;

    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::ilg2dlg> ilg2dlg;

    ContextPtr loc = Context::getContextPtr( ilg2dlg.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    {
        IndexType valuesIlg[] =
        { 7, 7, 5, 4, 4, 1 };
        const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
        IndexType expectedValues[] =
        { 6, 5, 5, 5, 3, 2, 2 };
        const IndexType numRows = 6;
        const IndexType numDiagonals = 7;
        HArray<IndexType> dlg( numDiagonals );
        HArray<IndexType> ilg( nIlg );

        initArray( ilg, valuesIlg, nIlg );
        {
            WriteOnlyAccess<IndexType> wDlg( dlg, loc, numDiagonals );
            ReadAccess<IndexType> rIlg( ilg, loc );
            SCAI_CONTEXT_ACCESS( loc );
            ilg2dlg[loc->getType()]( wDlg.get(), numDiagonals, rIlg.get(), numRows );
        }
        ReadAccess<IndexType> rDlg( dlg );

        for ( IndexType i = 0; i < numDiagonals; i++ )
        {
            BOOST_CHECK_EQUAL( expectedValues[i], rDlg.get()[i] );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE( sortRowsTest )
{
    typedef float OtherValueType;

    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::sortRows> sortRows;

    ContextPtr loc = Context::getContextPtr( sortRows.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    {
        IndexType valuesIlg[] =
        { 5, 2, 4, 4, 2, 7 };
        const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
        IndexType valuesPerm[] =
        { 0, 1, 2, 3, 4, 5 };
        const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
        IndexType expectedIlg[] =
        { 7, 5, 4, 4, 2, 2 };
        IndexType expectedPerm[] =
        { 5, 0, 2, 3, 1, 4 };
        const IndexType numRows = 6;

        HArray<IndexType> perm( nPerm, valuesPerm, testContext );
        HArray<IndexType> ilg( nIlg, valuesIlg, testContext );

        {
            WriteAccess<IndexType> wPerm( perm, loc );
            WriteAccess<IndexType> wIlg( ilg, loc );
            SCAI_CONTEXT_ACCESS( loc );
            sortRows[loc->getType()]( wIlg.get(), wPerm.get(), numRows );
        }

        ReadAccess<IndexType> rIlg( ilg );
        ReadAccess<IndexType> rPerm( perm );

        for ( IndexType i = 0; i < numRows; i++ )
        {
            BOOST_CHECK_EQUAL( expectedIlg[i], rIlg.get()[i] );
            BOOST_CHECK_EQUAL( expectedPerm[i], rPerm.get()[i] );
        }
    }
    {
        const IndexType numRows = 0;
        HArray<IndexType> perm( numRows );
        HArray<IndexType> ilg( numRows );
        {
            WriteOnlyAccess<IndexType> wPerm( perm, loc, numRows );
            WriteOnlyAccess<IndexType> wIlg( ilg, loc, numRows );
            SCAI_CONTEXT_ACCESS( loc );
            sortRows[loc->getType()]( wIlg.get(), wPerm.get(), numRows );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE( setInversePermTest )
{
    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::setInversePerm> setInversePerm;

    ContextPtr loc = Context::getContextPtr( setInversePerm.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    {
        IndexType valuesPerm[] =
        { 5, 0, 2, 3, 1, 4 };
        const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
        IndexType expectedPerm[] =
        { 1, 4, 2, 3, 5, 0 };
        const IndexType numRows = 6;

        HArray<IndexType> perm( nPerm, valuesPerm, testContext );
        HArray<IndexType> inversePerm;  // will be allocated/used on loc

        {
            ReadAccess<IndexType> rPerm( perm, loc );
            WriteOnlyAccess<IndexType> wInversePerm( inversePerm, loc, numRows );
            SCAI_CONTEXT_ACCESS( loc );
            setInversePerm[loc->getType()]( wInversePerm.get(), rPerm.get(), numRows );
        }

        ReadAccess<IndexType> rInversePerm( inversePerm );

        for ( IndexType i = 0; i < numRows; i++ )
        {
            BOOST_CHECK_EQUAL( expectedPerm[i], rInversePerm.get()[i] );
        }
    }
    {
        const IndexType numRows = 0;
        HArray<IndexType> perm( numRows );
        HArray<IndexType> inversePerm( numRows );
        {
            ReadAccess<IndexType> rPerm( perm, loc );
            WriteOnlyAccess<IndexType> wInversePerm( inversePerm, loc, numRows );
            SCAI_CONTEXT_ACCESS( loc );
            setInversePerm[loc->getType()]( wInversePerm.get(), rPerm.get(), numRows );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( setCSRValuesTest, ValueType, scai_arithmetic_test_types )
{
    typedef float OtherValueType;

    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::setCSRValues<ValueType, OtherValueType> > setCSRValues;

    ContextPtr loc = Context::getContextPtr( setCSRValues.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    /*
     * Testmatrix:
     * 0 0 5 3 0 0 4 0
     * 3 0 4 0 3 5 0 0
     * 0 2 0 8 7 9 0 5
     * 2 0 3 0 0 0 0 0
     * 5 0 0 7 0 0 0 9
     */
    IndexType valuesJDSDlg[] =
    { 5, 5, 4, 2, 1 };
    const IndexType nJDSDlg = sizeof( valuesJDSDlg ) / sizeof( IndexType );
    IndexType valuesJDSIlg[] =
    { 5, 4, 3, 3, 2 };
    const IndexType nJDSIlg = sizeof( valuesJDSIlg ) / sizeof( IndexType );
    IndexType valuesJDSPerm[] =
    { 2, 1, 0, 4, 3 };
    const IndexType nJDSPerm = sizeof( valuesJDSPerm ) / sizeof( IndexType );
    IndexType valuesCSRIa[] =
    { 0, 3, 7, 12, 14, 17 };
    const IndexType nCSRIa = sizeof( valuesCSRIa ) / sizeof( IndexType );
    IndexType valuesCSRJa[] =
    { 2, 3, 6, 0, 2, 4, 5, 1, 3, 4, 5, 7, 0, 2, 0, 3, 7 };
    const IndexType nCSRJa = sizeof( valuesCSRJa ) / sizeof( IndexType );
    OtherValueType valuesCSRValues[] =
    { 5, 3, 4, 3, 4, 3, 5, 2, 8, 7, 9, 5, 2, 3, 5, 7, 9 };
    const IndexType nCSRValues = sizeof( valuesCSRValues ) / sizeof( OtherValueType );
    IndexType expectedJDSJa[] =
    { 1, 0, 2, 0, 0, 3, 2, 3, 3, 2, 4, 4, 6, 7, 5, 5, 7 };
    ValueType expectedJDSValues[] =
    { 2, 3, 5, 5, 2, 8, 4, 3, 7, 3, 7, 3, 4, 9, 9, 5, 5 };
    const IndexType numRows = 5;
    const IndexType nJDS = nCSRValues;

    HArray<IndexType> JDSDlg( nJDSDlg, valuesJDSDlg );
    HArray<IndexType> JDSIlg( nJDSIlg, valuesJDSIlg );
    HArray<IndexType> JDSPerm( nJDSPerm, valuesJDSPerm );
    HArray<IndexType> CSRIa( nCSRIa, valuesCSRIa );
    HArray<IndexType> CSRJa( nCSRJa, valuesCSRJa );
    HArray<OtherValueType> CSRValues( nCSRValues, valuesCSRValues );

    // output arrays

    HArray<IndexType> JDSJa;
    HArray<ValueType> JDSValues;

    {
        WriteOnlyAccess<IndexType> wJDSJa( JDSJa, loc, nJDS );
        WriteOnlyAccess<ValueType> wJDSValues( JDSValues, loc, nJDS );
        ReadAccess<IndexType> rJDSDlg( JDSDlg, loc );
        ReadAccess<IndexType> rJDSIlg( JDSIlg, loc );
        ReadAccess<IndexType> rJDSPerm( JDSPerm, loc );
        ReadAccess<IndexType> rCSRIa( CSRIa, loc );
        ReadAccess<IndexType> rCSRJa( CSRJa, loc );
        ReadAccess<OtherValueType> rCSRValues( CSRValues, loc );
        SCAI_CONTEXT_ACCESS( loc );
        setCSRValues[loc->getType()]( wJDSJa.get(), wJDSValues.get(), numRows, rJDSPerm.get(), rJDSIlg.get(), nJDSDlg, rJDSDlg.get(),
                           rCSRIa.get(), rCSRJa.get(), rCSRValues.get() );
    }

    ReadAccess<IndexType> rJDSJa( JDSJa );
    ReadAccess<ValueType> rJDSValues( JDSValues );

    for ( IndexType i = 0; i < nJDS; i++ )
    {
        BOOST_CHECK_EQUAL( expectedJDSJa[i], rJDSJa.get()[i] );
        BOOST_CHECK_EQUAL( expectedJDSValues[i], rJDSValues.get()[i] );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( getCSRValuesTest, ValueType, scai_arithmetic_test_types )
{
    typedef float OtherValueType;

    ContextPtr testContext = ContextFix::testContext;

    KernelTraitContextFunction<JDSKernelTrait::getCSRValues<ValueType, OtherValueType> > getCSRValues;

    ContextPtr loc = Context::getContextPtr( getCSRValues.validContext( testContext->getType() ) );

    BOOST_WARN_EQUAL( loc->getType(), testContext->getType() );

    /*
     * Testmatrix:
     * 0 0 5 3 0 0 4 0
     * 3 0 4 0 3 5 0 0
     * 0 2 0 8 7 9 0 5
     * 2 0 3 0 0 0 0 0
     * 5 0 0 7 0 0 0 9
     */
    IndexType valuesJDSDlg[] =
    { 5, 5, 4, 2, 1 };
    const IndexType nJDSDlg = sizeof( valuesJDSDlg ) / sizeof( IndexType );
    IndexType valuesJDSIlg[] =
    { 5, 4, 3, 3, 2 };
    const IndexType nJDSIlg = sizeof( valuesJDSIlg ) / sizeof( IndexType );
    IndexType valuesJDSPerm[] =
    { 2, 1, 0, 4, 3 };
    const IndexType nJDSPerm = sizeof( valuesJDSPerm ) / sizeof( IndexType );
    IndexType valuesCSRIa[] =
    { 0, 3, 7, 12, 14, 17 };
    const IndexType nCSRIa = sizeof( valuesCSRIa ) / sizeof( IndexType );
    IndexType valuesJDSJa[] =
    { 1, 0, 2, 0, 0, 3, 2, 3, 3, 2, 4, 4, 6, 7, 5, 5, 7 };
    const IndexType nJDSJa = sizeof( valuesJDSJa ) / sizeof( IndexType );
    ValueType valuesJDSValues[] =
    { 2, 3, 5, 5, 2, 8, 4, 3, 7, 3, 7, 3, 4, 9, 9, 5, 5 };
    const IndexType nJDSValues = sizeof( valuesJDSValues ) / sizeof( ValueType );
    IndexType expectedCSRJa[] =
    { 2, 3, 6, 0, 2, 4, 5, 1, 3, 4, 5, 7, 0, 2, 0, 3, 7 };
    OtherValueType expectedCSRValues[] =
    { 5, 3, 4, 3, 4, 3, 5, 2, 8, 7, 9, 5, 2, 3, 5, 7, 9 };
    const IndexType numRows = 5;
    const IndexType nJDS = nJDSValues;
    HArray<IndexType> JDSJa( nJDSJa );
    HArray<ValueType> JDSValues( nJDSValues );
    HArray<IndexType> JDSDlg( nJDSDlg );
    HArray<IndexType> JDSIlg( nJDSIlg );
    HArray<IndexType> JDSPerm( nJDSPerm );
    HArray<IndexType> CSRIa( nCSRIa );
    HArray<IndexType> CSRJa( nJDS );
    HArray<OtherValueType> CSRValues( nJDS );

    initArray( JDSJa, valuesJDSJa, nJDSJa );
    initArray( JDSValues, valuesJDSValues, nJDSValues );
    initArray( JDSDlg, valuesJDSDlg, nJDSDlg );
    initArray( JDSIlg, valuesJDSIlg, nJDSIlg );
    initArray( JDSPerm, valuesJDSPerm, nJDSPerm );
    initArray( CSRIa, valuesCSRIa, nCSRIa );
    {
        ReadAccess<IndexType> rJDSJa( JDSJa, loc );
        ReadAccess<ValueType> rJDSValues( JDSValues, loc );
        ReadAccess<IndexType> rJDSDlg( JDSDlg, loc );
        ReadAccess<IndexType> rJDSIlg( JDSIlg, loc );
        ReadAccess<IndexType> rJDSPerm( JDSPerm, loc );
        ReadAccess<IndexType> rCSRIa( CSRIa, loc );
        WriteOnlyAccess<IndexType> wCSRJa( CSRJa, loc, nJDS );
        WriteOnlyAccess<OtherValueType> wCSRValues( CSRValues, loc, nJDS );
        SCAI_CONTEXT_ACCESS( loc );
        getCSRValues[loc->getType()]( wCSRJa.get(), wCSRValues.get(), rCSRIa.get(), numRows, rJDSPerm.get(), rJDSIlg.get(),
                           rJDSDlg.get(), rJDSJa.get(), rJDSValues.get() );
    }
    ReadAccess<IndexType> rCSRJa( CSRJa );
    ReadAccess<OtherValueType> rCSRValues( CSRValues );

    for ( IndexType i = 0; i < nJDS; i++ )
    {
        BOOST_CHECK_EQUAL( expectedCSRJa[i], rCSRJa.get()[i] );
        BOOST_CHECK_EQUAL( expectedCSRValues[i], rCSRValues.get()[i] );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE_END()