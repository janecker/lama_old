/**
 * @file JDSStorageTest.cpp
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
 * @brief Test cases for JDSStorage( only specific ones )
 * @author Thomas Brandes
 * @date 31.08.2012
 * @since 1.0.0
 */

#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include <scai/lama/storage/JDSStorage.hpp>

#include <scai/utilskernel/HArrayUtils.hpp>
#include <scai/utilskernel/LArray.hpp>

#include <scai/common/test/TestMacros.hpp>
#include <scai/common/TypeTraits.hpp>

#include <scai/logging.hpp>

using namespace scai;
using namespace scai::lama;
using namespace scai::utilskernel;
using namespace scai::hmemo;
using scai::common::Exception;

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE( JDSStorageTest )

SCAI_LOG_DEF_LOGGER( logger, "Test.JDSStorageTest" )

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( checkTest, ValueType, scai_arithmetic_test_types )
{
    ContextPtr context = Context::getContextPtr();

    // This routine tests the check method of JDSStorage individually for this class

    JDSStorage<ValueType> jdsStorage;

    jdsStorage.setContextPtr( context );

    SCAI_LOG_INFO( logger, "checkTest for JDSStorage<" << common::TypeTraits<ValueType>::id() << "> @ " << *context )

    for ( int icase = 0; icase < 6; ++icase )
    {
        // build up a correct JDS storage
        IndexType valuesJa[] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        IndexType valuesDlg[] =
        { 3, 3, 3 };
        const IndexType nDlg = sizeof( valuesDlg ) / sizeof( IndexType );
        IndexType valuesIlg[] =
        { 3, 3, 3 };
        const IndexType nIlg = sizeof( valuesIlg ) / sizeof( IndexType );
        IndexType valuesPerm[] =
        { 2, 0, 1 };
        const IndexType nPerm = sizeof( valuesPerm ) / sizeof( IndexType );
        const IndexType numRows = nIlg;
        const IndexType numValues = nJa;
        const IndexType numColumns = 10;
        const IndexType numDiagonals = nDlg;
        HArrayRef<IndexType> jdsJA( nJa, valuesJa );
        HArray<ValueType> jdsValues( nJa, 1.0 ); // only need to build JDS storage
        HArrayRef<IndexType> jdsDLG( nDlg, valuesDlg );
        HArrayRef<IndexType> jdsILG( nIlg, valuesIlg );
        HArrayRef<IndexType> jdsPerm( nPerm, valuesPerm );
        // setJDSData will copy/convert values up to the needed context
        jdsStorage.setJDSData( numRows, numColumns, numValues, numDiagonals, jdsDLG, jdsILG, jdsPerm, jdsJA,
                               jdsValues );

        if ( icase == 0 )
        {
            jdsStorage.check( "test with correct values" );
        }
        else if ( icase == 1 )
        {
            //  -> invalid ja     { 0, 1, 2, 3, 15, 5, 6, 7, 8 }
            HArray<IndexType>& jdsJA = const_cast<HArray<IndexType>&>( jdsStorage.getJA() );
            HArrayUtils::setVal( jdsJA, 5, 15 );
            BOOST_CHECK_THROW( { jdsStorage.check( "Expect illegal index in JA" ); }, Exception );
        }
        else if ( icase == 2 )
        {
            //  -> invalid ilg    { 3, 3, 4 }
            HArray<IndexType>& jdsILG = const_cast<HArray<IndexType>&>( jdsStorage.getIlg() );
            HArrayUtils::setVal( jdsILG, 2, 4 );
            BOOST_CHECK_THROW( { jdsStorage.check( "Expect illegal ilg" ); }, Exception );
        }
        else if ( icase == 3 )
        {
            //  -> invalid dlg    { 3, 3, 4 }
            HArray<IndexType>& jdsDLG = const_cast<HArray<IndexType>&>( jdsStorage.getDlg() );
            HArrayUtils::setVal( jdsDLG, 2, 4 );
            BOOST_CHECK_THROW( { jdsStorage.check( "Expect illegal dlg" ); }, Exception );
        }
        else if ( icase == 4 )
        {
            //  -> invalid perm   { 5, 0, 2 }
            HArray<IndexType>& jdsPerm = const_cast<HArray<IndexType>&>( jdsStorage.getPerm() );
            HArrayUtils::setVal( jdsPerm, 0, 5 );
            BOOST_CHECK_THROW( { jdsStorage.check( "Expect illegal perm" ); }, Exception );
        }
        else if ( icase == 5 )
        {
            //  -> invalid perm   { 0, 0, 2 }
            HArray<IndexType>& jdsPerm = const_cast<HArray<IndexType>&>( jdsStorage.getPerm() );
            HArrayUtils::setVal( jdsPerm, 0, 0 );
            BOOST_CHECK_THROW( { jdsStorage.check( "Expect illegal perm" ); }, Exception );
        }

    } // CASE_LOOP
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( constructorTest, ValueType, scai_arithmetic_test_types )
{
    ContextPtr context = Context::getContextPtr();

    SCAI_LOG_INFO( logger, "constructorTest for JDSStorage<" << common::TypeTraits<ValueType>::id() << "> @ " << *context )

    const IndexType numRows = 10;
    const IndexType numColumns = 15;

    JDSStorage<ValueType> jdsStorage( numRows, numColumns );

    BOOST_REQUIRE_EQUAL( numRows, jdsStorage.getNumRows() );
    BOOST_REQUIRE_EQUAL( numColumns, jdsStorage.getNumColumns() );
    BOOST_REQUIRE_EQUAL( 0, jdsStorage.getNumValues() );

    for ( IndexType i = 0; i < numRows; ++i )
    {
        for ( IndexType j = 0; j < numColumns; ++j )
        {
            float v = static_cast<float> ( jdsStorage.getValue( i, j ) );
            BOOST_CHECK_SMALL( v , 1.0e-5f );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( constructor1Test, ValueType, scai_arithmetic_test_types )
{
    ContextPtr context = Context::getContextPtr();

    SCAI_LOG_INFO( logger, "constructor1Test for JDSStorage<" << common::TypeTraits<ValueType>::id() << "> @ " << *context )

    const IndexType numRows = 3;
    const IndexType numColumns = 3;
// Note: ja, values are stored column-major order
    const IndexType ja[] =
    {   2, 0, 1, 2,};
    const ValueType values[] =
    {   0.3f, 0.5f, 0.5f, 0.2f};
    const IndexType perm[] =
    {   2, 0, 1};
    const IndexType ilg[] =
    {   2, 1, 1};
    const IndexType dlg[] =
    {   3, 1};
    const IndexType numDiagonals = sizeof( dlg ) / sizeof( IndexType );
    const IndexType numValues = sizeof( ja ) / sizeof( IndexType );
    const IndexType sizeValues = sizeof( values ) / sizeof( ValueType );
    const IndexType sizePerm = sizeof( perm ) / sizeof( IndexType );
    const IndexType sizeILG = sizeof( ilg ) / sizeof( IndexType );
    BOOST_CHECK_EQUAL( numValues, sizeValues );
    BOOST_CHECK_EQUAL( numRows, sizePerm );
    BOOST_CHECK_EQUAL( numRows, sizeILG );
    LArray<IndexType> jdsILG( numRows, ilg );
    LArray<IndexType> jdsDLG( numDiagonals, dlg );
    LArray<IndexType> jdsPerm( numRows, perm );
    LArray<IndexType> jdsJA( numValues, ja );
    LArray<ValueType> jdsValues( numValues, values );
    // Call the specific constructor for JDS storage
    JDSStorage<ValueType> jdsStorage( numRows, numColumns, numValues, numDiagonals,
                                      jdsDLG, jdsILG, jdsPerm, jdsJA, jdsValues );
    // Test all the getter routines, includes the specific ones for JDSStorage
    BOOST_REQUIRE_EQUAL( numRows, jdsStorage.getNumRows() );
    BOOST_REQUIRE_EQUAL( numColumns, jdsStorage.getNumColumns() );
    BOOST_REQUIRE_EQUAL( numDiagonals, jdsStorage.getNumDiagonals() );
    {
        ReadAccess<IndexType> jdsILG( jdsStorage.getIlg() );
        ReadAccess<IndexType> jdsPerm( jdsStorage.getPerm() );

        for ( IndexType i = 0; i < numRows; ++i )
        {
            BOOST_CHECK_EQUAL( ilg[i], jdsILG[i] );
            BOOST_CHECK_EQUAL( perm[i], jdsPerm[i] );
        }

        ReadAccess<IndexType> jdsDLG( jdsStorage.getDlg() );

        for ( IndexType i = 0; i < numDiagonals; ++i )
        {
            BOOST_CHECK_EQUAL( dlg[i], jdsDLG[i] );
        }

        ReadAccess<IndexType> jdsJA( jdsStorage.getJA() );
        ReadAccess<ValueType> jdsValues( jdsStorage.getValues() );

        // JDS keeps values in same order

        for ( IndexType i = 0; i < numValues; ++i )
        {
            BOOST_CHECK_EQUAL( ja[i], jdsJA[i] );
            BOOST_CHECK_EQUAL( values[i], jdsValues[i] );
        }
    }
    // copy constructor on all available locations
    JDSStorage<ValueType> jdsStorageCopy( jdsStorage, context );
    BOOST_REQUIRE_EQUAL( numRows, jdsStorageCopy.getNumRows() );
    BOOST_REQUIRE_EQUAL( numColumns, jdsStorageCopy.getNumColumns() );
    BOOST_REQUIRE_EQUAL( numDiagonals, jdsStorageCopy.getNumDiagonals() );
    {
        ReadAccess<IndexType> jdsILG( jdsStorageCopy.getIlg() );
        ReadAccess<IndexType> jdsPerm( jdsStorageCopy.getPerm() );

        for ( IndexType i = 0; i < numRows; ++i )
        {
            BOOST_CHECK_EQUAL( ilg[i], jdsILG[i] );
            BOOST_CHECK_EQUAL( perm[i], jdsPerm[i] );
        }

        ReadAccess<IndexType> jdsDLG( jdsStorageCopy.getDlg() );

        for ( IndexType i = 0; i < numDiagonals; ++i )
        {
            BOOST_CHECK_EQUAL( dlg[i], jdsDLG[i] );
        }

        ReadAccess<IndexType> jdsJA( jdsStorageCopy.getJA() );
        ReadAccess<ValueType> jdsValues( jdsStorageCopy.getValues() );

        // JDS keeps values in same order

        for ( IndexType i = 0; i < numValues; ++i )
        {
            BOOST_CHECK_EQUAL( ja[i], jdsJA[i] );
            BOOST_CHECK_EQUAL( values[i], jdsValues[i] );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( typenameTest, ValueType, scai_arithmetic_test_types )
{
    SCAI_LOG_INFO( logger, "typeNameTest for JDSStorage<" << common::TypeTraits<ValueType>::id() << ">" )

    // context does not matter here, so runs for every context

    std::string s = JDSStorage<ValueType>::typeName();

    BOOST_CHECK( s.length() > 0 );
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE_END();