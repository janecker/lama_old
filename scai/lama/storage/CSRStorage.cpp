/**
 * @file CSRStorage.cpp
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
 * @brief Implementation and instantiation for template class CSRStorage.
 * @author Thomas Brandes
 * @date 04.06.2011
 * @since 1.0.0
 */

// hpp
#include <scai/lama/storage/CSRStorage.hpp>

// others
#include <scai/lama/LAMAInterface.hpp>

#include <scai/lama/storage/StorageMethods.hpp>

#include <scai/lama/LAMAArrayUtils.hpp>

#include <scai/lama/distribution/Redistributor.hpp>
#include <scai/memory.hpp>

#include <scai/tasking/TaskSyncToken.hpp>

#include <scai/lama/openmp/OpenMPUtils.hpp>
#include <scai/lama/openmp/OpenMPCSRUtils.hpp>

// assert
#include <scai/lama/exception/LAMAAssert.hpp>

// boost
#include <scai/common/bind.hpp>
#include <boost/preprocessor.hpp>
#include <scai/tracing.hpp>

#include <cmath>

namespace scai
{

namespace lama
{

using std::abs;
using scai::common::unique_ptr;
using scai::common::shared_ptr;

using scai::tasking::TaskSyncToken;

using namespace scai::memory;

/* --------------------------------------------------------------------------- */

SCAI_LOG_DEF_TEMPLATE_LOGGER( template<typename ValueType>, CSRStorage<ValueType>::logger, "MatrixStorage.CSRStorage" )

/* --------------------------------------------------------------------------- */

template<typename ValueType>
CSRStorage<ValueType>::CSRStorage()
    : CRTPMatrixStorage<CSRStorage<ValueType>,ValueType>( 0, 0 ), mNumValues( 0 ), mSortedRows( false )
{
    allocate( 0, 0 ); // creates at least mIa
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
CSRStorage<ValueType>::CSRStorage(
    const IndexType numRows,
    const IndexType numColumns,
    const IndexType numValues,
    const LAMAArray<IndexType>& ia,
    const LAMAArray<IndexType>& ja,
    const ContextArray& values )

    : CRTPMatrixStorage<CSRStorage<ValueType>,ValueType>()
{
    this->setCSRData( numRows, numColumns, numValues, ia, ja, values );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::print() const
{
    using std::cout;
    using std::endl;

    cout << "CSRStorage " << mNumRows << " x " << mNumColumns << ", #values = " << mNumValues << endl;

    ReadAccess<IndexType> ia( mIa );
    ReadAccess<IndexType> ja( mJa );
    ReadAccess<ValueType> values( mValues );

    for( IndexType i = 0; i < mNumRows; i++ )
    {
        cout << "Row " << i << " ( " << ia[i] << " - " << ia[i + 1] << " ) :";

        for( IndexType jj = ia[i]; jj < ia[i + 1]; ++jj )
        {
            cout << " " << ja[jj] << ":" << values[jj];
        }

        cout << endl;
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::clear()
{
    mNumRows = 0;
    mNumColumns = 0;

    mIa.clear();
    mJa.clear();
    mValues.clear();

    mDiagonalProperty = checkDiagonalProperty();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
MatrixStorageFormat CSRStorage<ValueType>::getFormat() const
{
    return Format::CSR;
}

/* --------------------------------------------------------------------------- */

#ifndef SCAI_ASSERT_LEVEL_DEBUG
template<typename ValueType>
void CSRStorage<ValueType>::check( const char* ) const
{}
#else
template<typename ValueType>
void CSRStorage<ValueType>::check( const char* msg ) const
{
    SCAI_ASSERT_EQUAL_ERROR( mNumRows + 1, mIa.size() )
    SCAI_ASSERT_EQUAL_ERROR( mNumValues, mJa.size() )
    SCAI_ASSERT_EQUAL_ERROR( mNumValues, mValues.size() )

    // check ascending values in offset array mIa

    {
        ContextPtr loc = getContextPtr();

        LAMA_INTERFACE_FN_DEFAULT_T( isSorted, loc, Utils, Reductions, IndexType )
        LAMA_INTERFACE_FN_T( getValue, loc, Utils, Getter, IndexType )

        ReadAccess<IndexType> csrIA( mIa, loc );

        SCAI_CONTEXT_ACCESS( loc )

        bool ascending = true; // check for ascending

        IndexType numValues = getValue( csrIA.get(), mNumRows );

        SCAI_ASSERT_ERROR(
            numValues == mNumValues,
            "ia[" << mNumRows << "] = " << numValues << ", expected " << mNumValues << ", msg = " << msg )

        SCAI_ASSERT_ERROR( isSorted( csrIA.get(), mNumRows + 1, ascending ),
                           *this << " @ " << msg << ": IA is illegal offset array" )
    }

    // check column indexes in JA

    {
        ContextPtr loc = getContextPtr();

        LAMA_INTERFACE_FN_DEFAULT( validIndexes, loc, Utils, Indexes )

        ReadAccess<IndexType> rJA( mJa, loc );

        SCAI_CONTEXT_ACCESS( loc )

        SCAI_ASSERT_ERROR( validIndexes( rJA.get(), mNumValues, mNumColumns ),
                           *this << " @ " << msg << ": illegel indexes in JA" )
    }
}
#endif

/* --------------------------------------------------------------------------- */

template<typename ValueType>
bool CSRStorage<ValueType>::checkDiagonalProperty() const
{
    // diagonal property is given if size of matrix is 0

    if( mNumRows == 0 || mNumColumns == 0 )
    {
        return true;
    }

    // non-zero sized matrix with no values has not diagonal property

    if( mNumValues == 0 )
    {
        return false;
    }

    ContextPtr loc = getContextPtr(); // there we do the checks

    //get function pointer
    LAMA_INTERFACE_FN( hasDiagonalProperty, loc, CSRUtils, Offsets )

    //get read access
    ReadAccess<IndexType> csrIA( mIa, loc );
    ReadAccess<IndexType> csrJA( mJa, loc );
    SCAI_CONTEXT_ACCESS( loc )

    IndexType numDiagonals = std::min( mNumRows, mNumColumns );

    bool diagonalProperty = hasDiagonalProperty( numDiagonals, csrIA.get(), csrJA.get() );

    SCAI_LOG_DEBUG( logger, *this << ": diagonalProperty = " << diagonalProperty );

    return diagonalProperty;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::setIdentity( const IndexType size )
{
    SCAI_LOG_DEBUG( logger, "set identity, size = " << size )

    _MatrixStorage::setDimension( size, size );

    mNumValues = mNumRows;

    WriteOnlyAccess<IndexType> ia( mIa, mNumRows + 1 );
    WriteOnlyAccess<IndexType> ja( mJa, mNumValues );
    WriteOnlyAccess<ValueType> values( mValues, mNumValues );

    ValueType one = static_cast<ValueType>( 1.0 );

    OpenMPUtils::setOrder( ia.get(), mNumRows + 1 );
    OpenMPUtils::setOrder( ja.get(), mNumRows );
    OpenMPUtils::setVal( values.get(), mNumRows, one );

    mDiagonalProperty = true; // obviously given for identity matrix
    mSortedRows = true; // obviously given for identity matrix

    // Note: we do not build row indexes, no row is empty

    SCAI_LOG_INFO( logger, *this << ": identity matrix" )
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
template<typename OtherValueType>
void CSRStorage<ValueType>::setCSRDataImpl(
    const IndexType numRows,
    const IndexType numColumns,
    const IndexType numValues,
    const LAMAArray<IndexType>& ia,
    const LAMAArray<IndexType>& ja,
    const LAMAArray<OtherValueType>& values,
    const ContextPtr /* loc */)
{
    ContextPtr loc = this->getContextPtr();

    if( ia.size() == numRows )
    {
        // checking is done where ia is already valid, preferred is loc

        ContextPtr loc1 = ia.getValidContext( loc->getType() );

        // we assume that ia contains the sizes, verify it by summing up

        LAMA_INTERFACE_FN_DEFAULT_T( sum, loc1, Utils, Reductions, IndexType )

        ReadAccess<IndexType> csrIA( ia, loc1 );

        SCAI_CONTEXT_ACCESS( loc1 )

        IndexType n = sum( csrIA.get(), numRows );

        if( n != numValues )
        {
            COMMON_THROWEXCEPTION( "ia is invalid size array" )
        }
    }
    else if( ia.size() == numRows + 1 )
    {
        // checking is done where ia is already valid

        ContextPtr loc1 = ia.getValidContext( loc->getType() );

        LAMA_INTERFACE_FN_DEFAULT( validOffsets, loc1, CSRUtils, Offsets )

        ReadAccess<IndexType> csrIA( ia, loc1 );

        SCAI_CONTEXT_ACCESS( loc1 )

        if( !validOffsets( csrIA.get(), numRows, numValues ) )
        {
            COMMON_THROWEXCEPTION( "ia is invalid offset array" )
        }
    }
    else
    {
        COMMON_THROWEXCEPTION( "ia array with size = " << ia.size() << " illegal, #rows = " << numRows )
    }

    SCAI_ASSERT_EQUAL_ERROR( numValues, ja.size() );
    SCAI_ASSERT_EQUAL_ERROR( numValues, values.size() );

    {
        LAMA_INTERFACE_FN( validIndexes, loc, Utils, Indexes )

        // make sure that column indexes in JA are all valid

        ReadAccess<IndexType> csrJA( ja, loc );

        SCAI_CONTEXT_ACCESS( loc )

        if( !validIndexes( csrJA.get(), numValues, numColumns ) )
        {
            COMMON_THROWEXCEPTION( "invalid column indexes in ja = " << ja << ", #columns = " << numColumns )
        }
    }

    // now we can copy all data

    mNumRows = numRows;
    mNumColumns = numColumns;
    mNumValues = numValues;

    SCAI_LOG_DEBUG( logger, "fill " << *this << " with csr data, " << numValues << " non-zero values" )

    // storage data will be directly allocated on the location

    if( ia.size() == numRows )
    {
        {
            // reserve enough memory for mIa

            WriteOnlyAccess<IndexType> myIA( mIa, loc, mNumRows + 1 );
        }

        LAMAArrayUtils::assign( mIa, ia, loc );

        {
            ContextPtr loc1 = loc; // loc might change if sizes2offsets is not available

            LAMA_INTERFACE_FN_DEFAULT( sizes2offsets, loc1, CSRUtils, Offsets )

            WriteAccess<IndexType> myIA( mIa, loc1 );

            myIA.resize( mNumRows + 1 ); // no realloc as capacity is sufficient

            sizes2offsets( myIA.get(), numRows );
        }
    }
    else
    {
        LAMAArrayUtils::assign( mIa, ia, loc );
    }

    LAMAArrayUtils::assign( mValues, values, loc );
    LAMAArrayUtils::assign( mJa, ja, loc );

    /* do not sort rows, destroys diagonal property during redistribute

     OpenMPCSRUtils::sortRowElements( myJA.get(), myValues.get(), myIA.get(),
     mNumRows, mDiagonalProperty );

     */

    mDiagonalProperty = checkDiagonalProperty();

    buildRowIndexes();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::sortRows( bool diagonalProperty )
{
    {
        ReadAccess<IndexType> csrIA( mIa );
        WriteAccess<IndexType> csrJA( mJa );
        WriteAccess<ValueType> csrValues( mValues );

        OpenMPCSRUtils::sortRowElements( csrJA.get(), csrValues.get(), csrIA.get(), mNumRows, diagonalProperty );
    }

    mDiagonalProperty = checkDiagonalProperty();
}

//this version avoids copying the ia, ja, and value arrays, but instead swaps them
//also it does not check their validity
//this is much faster of course, but destroys the input ia, ja and value arrays
template<typename ValueType>
template<typename OtherValueType>
void CSRStorage<ValueType>::setCSRDataSwap(
    const IndexType numRows,
    const IndexType numColumns,
    const IndexType numValues,
    LAMAArray<IndexType>& ia,
    LAMAArray<IndexType>& ja,
    LAMAArray<OtherValueType>& values,
    const ContextPtr /* loc */)
{
    //set necessary information
    mNumRows = numRows;
    mNumColumns = numColumns;
    mNumValues = numValues;

    SCAI_LOG_DEBUG( logger, "fill " << *this << " with csr data, " << numValues << " non-zero values" )

    //swap arrays

    mIa.swap( ia );
    mJa.swap( ja );
    mValues.swap( values );

    mDiagonalProperty = checkDiagonalProperty();

    // this builds only row indices if context is on host

    buildRowIndexes();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::buildRowIndexes()
{
    SCAI_LOG_INFO( logger, "buildRowIndexes is temporarily disabled." );
    return;

    mRowIndexes.clear();

    if( mNumRows == 0 )
    {
        return;
    }

    if( getContext().getType() != context::Host )
    {
        SCAI_LOG_INFO( logger, "CSRStorage: build row indices is currently only implemented on host" )
    }

    // This routine is only available on the Host

    ContextPtr loc = Context::getContextPtr( context::Host );

    ReadAccess<IndexType> csrIA( mIa, loc );

    IndexType nonZeroRows = OpenMPCSRUtils::countNonEmptyRowsByOffsets( csrIA.get(), mNumRows );

    float usage = float( nonZeroRows ) / float( mNumRows );

    if( usage >= mCompressThreshold )
    {
        SCAI_LOG_INFO( logger, "CSRStorage: do not build row indexes, usage = " << usage )
        return;
    }

    SCAI_LOG_INFO( logger, "CSRStorage: build row indexes, #entries = " << nonZeroRows )

    WriteOnlyAccess<IndexType> rowIndexes( mRowIndexes, loc, nonZeroRows );

    OpenMPCSRUtils::setNonEmptyRowsByOffsets( rowIndexes.get(), nonZeroRows, csrIA.get(), mNumRows );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::redistributeCSR( const CSRStorage<ValueType>& other, const Redistributor& redistributor )
{
    SCAI_REGION( "Storage.redistributeCSR" )

    const Distribution& sourceDistribution = *redistributor.getSourceDistributionPtr();
    const Distribution& targetDistribution = *redistributor.getTargetDistributionPtr();

    SCAI_LOG_INFO( logger,
                   other << ": redistribute of CSR<" << other.getValueType() << "> to CSR<" << this->getValueType() << " via " << redistributor )

    bool sameDist = false;

    // check for same distribution, either equal or both replicated

    if( sourceDistribution.isReplicated() && targetDistribution.isReplicated() )
    {
        sameDist = true;
    }
    else if( &sourceDistribution == &targetDistribution )
    {
        sameDist = true;
    }

    if( sameDist )
    {
        SCAI_LOG_INFO( logger, "redistributor with same source/target distribution" )

        assign( other );

        return; // so we are done
    }

    // check that source distribution fits with storage

    SCAI_ASSERT_EQUAL_ERROR( other.getNumRows(), sourceDistribution.getLocalSize() )

    if( &other == this )
    {
        // due to alias we need temporary array

        LAMAArray<IndexType> targetIA;
        LAMAArray<IndexType> targetJA;
        LAMAArray<ValueType> targetValues;

        StorageMethods<ValueType>::redistributeCSR( targetIA, targetJA, targetValues, other.getIA(), other.getJA(),
                other.getValues(), redistributor );

        // we can swap the new arrays

        mIa.swap( targetIA );
        mJa.swap( targetJA );
        mValues.swap( targetValues );
    }
    else
    {
        StorageMethods<ValueType>::redistributeCSR( mIa, mJa, mValues, other.getIA(), other.getJA(), other.getValues(),
                redistributor );
    }

    // it is not necessary to convert the other storage to CSR

    mNumColumns = other.getNumColumns();
    mNumRows = mIa.size() - 1;
    mNumValues = mJa.size();

    mDiagonalProperty = checkDiagonalProperty();

    buildRowIndexes();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
CSRStorage<ValueType>::~CSRStorage()
{
    SCAI_LOG_DEBUG( logger,
                    "~CSRStorage for maxtrix " << mNumRows << " x " << mNumColumns << ", # non-zeros = " << mNumValues )
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
IndexType CSRStorage<ValueType>::getNumValues() const
{
    return mNumValues;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::purge()
{
    mNumColumns = 0;
    mNumRows = 0;
    mNumValues = 0;

    mIa.purge();
    mJa.purge();
    mValues.purge();

    mDiagonalProperty = checkDiagonalProperty();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::allocate( IndexType numRows, IndexType numColumns )
{
    SCAI_LOG_INFO( logger,
                   "allocate CSR sparse matrix of size " << numRows << " x " << numColumns << ", numValues = 0" )

    _MatrixStorage::setDimension( numRows, numColumns );

    mNumValues = 0;

    mJa.clear();
    mValues.clear();

    WriteOnlyAccess<IndexType> ia( mIa, mNumRows + 1 );

    // make a correct initialization for the offset array

    OpenMPUtils::setVal( ia.get(), mNumRows + 1, 0 );

    mDiagonalProperty = false;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::compress( const ValueType eps /* = 0.0 */)
{
    WriteAccess<IndexType> ia( mIa );
    ReadAccess<IndexType> ja( mJa );
    ReadAccess<ValueType> values( mValues );

    IndexType nonDiagZeros = 0;

    for( IndexType i = 0; i < mNumRows; ++i )
    {
        for( IndexType jj = ia[i]; jj < ia[i + 1]; ++jj )
        {
            if( ja[jj] == i )
            {
                continue;
            }

            if( abs( values[jj] ) <= eps )
            {
                ++nonDiagZeros;
            }
        }
    }

    SCAI_LOG_INFO( logger, "compress: " << nonDiagZeros << " non-diagonal zero elements" )

    if( nonDiagZeros == 0 )
    {
        return;
    }

    const IndexType newNumValues = mJa.size() - nonDiagZeros;

    LAMAArray<ValueType> newValuesArray;
    LAMAArray<IndexType> newJaArray;

    WriteOnlyAccess<ValueType> newValues( newValuesArray, newNumValues );
    WriteOnlyAccess<IndexType> newJa( newJaArray, newNumValues );

    IndexType gap = 0;

    for( IndexType i = 0; i < mNumRows; ++i )
    {
        for( IndexType jj = ia[i] + gap; jj < ia[i + 1]; ++jj )
        {
            if( abs( values[jj] ) <= eps && ja[jj] != i )
            {
                ++gap;
                continue;
            }

            newValues[jj - gap] = values[jj];
            newJa[jj - gap] = ja[jj];
        }

        ia[i + 1] -= gap;
    }

    SCAI_ASSERT_EQUAL_DEBUG( gap, nonDiagZeros )

    ia.release();
    ja.release();
    values.release();
    newJa.release();
    newValues.release();

    mJa.swap( newJaArray );
    mValues.swap( newValuesArray );
    mNumValues = newNumValues;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::swap( CSRStorage<ValueType>& other )
{
    std::swap( mNumValues, other.mNumValues );
    mIa.swap( other.mIa );
    mJa.swap( other.mJa );
    mValues.swap( other.mValues );

    // swap sizes and row indexes

    MatrixStorage<ValueType>::swap( other );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::swap( LAMAArray<IndexType>& ia, LAMAArray<IndexType>& ja, LAMAArray<ValueType>& values )
{
    SCAI_ASSERT_EQUAL_ERROR( ia.size(), mNumRows + 1 )

    IndexType numValues = 0;

    {
        ReadAccess<IndexType> csrIA( ia );
        numValues = csrIA[mNumRows];
    }

    SCAI_ASSERT_EQUAL_ERROR( numValues, ja.size() )
    SCAI_ASSERT_EQUAL_ERROR( numValues, values.size() )

    mNumValues = numValues;

    mIa.swap( ia );
    mJa.swap( ja );
    mValues.swap( values );

    mDiagonalProperty = checkDiagonalProperty();

    // build new array of row indexes

    buildRowIndexes();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
size_t CSRStorage<ValueType>::getMemoryUsageImpl() const
{
    size_t memoryUsage = 0;
    memoryUsage += sizeof(IndexType);
    memoryUsage += sizeof(IndexType) * mIa.size();
    memoryUsage += sizeof(IndexType) * mJa.size();
    memoryUsage += sizeof(ValueType) * mValues.size();
    return memoryUsage;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::writeAt( std::ostream& stream ) const
{
    using ::operator<<;   // ToDo: still other operators in this namespace
    stream << "CSRStorage<" << common::getScalarType<ValueType>() << ">("
           << " size = " << mNumRows << " x " << mNumColumns
           << ", nnz = " << mNumValues << ", diag = " << mDiagonalProperty << ", sorted = " << mSortedRows << " )";
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType CSRStorage<ValueType>::getValue( const IndexType i, const IndexType j ) const
{
    SCAI_LOG_TRACE( logger, "get value (" << i << ", " << j << ")" )
    const ReadAccess<IndexType> ia( mIa );
    const ReadAccess<IndexType> ja( mJa );
    const ReadAccess<ValueType> values( mValues );
    ValueType myValue = 0;

    SCAI_LOG_TRACE( logger, "search column in ja from " << ia[i] << ":" << ia[i + 1] )

    for( IndexType jj = ia[i]; jj < ia[i + 1]; ++jj )
    {
        IndexType col = ja[jj];

        SCAI_ASSERT_DEBUG( 0 <= col && col < mNumColumns,
                           "column index at pos " << jj << " = " << col << " out of range" )

        if( col == j )
        {
            SCAI_LOG_TRACE( logger, "found column j = " << j << " at " << jj << ", value = " << values[jj] )
            myValue = values[jj];
            break;
        }
    }

    return myValue;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::prefetch( const ContextPtr location ) const
{
    mIa.prefetch( location );
    mJa.prefetch( location );
    mValues.prefetch( location );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
LAMAArray<IndexType>& CSRStorage<ValueType>::getIA()
{
    return mIa;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
LAMAArray<IndexType>& CSRStorage<ValueType>::getJA()
{
    return mJa;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
LAMAArray<ValueType>& CSRStorage<ValueType>::getValues()
{
    return mValues;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
const LAMAArray<IndexType>& CSRStorage<ValueType>::getIA() const
{
    return mIa;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
const LAMAArray<IndexType>& CSRStorage<ValueType>::getJA() const
{
    return mJa;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
const LAMAArray<ValueType>& CSRStorage<ValueType>::getValues() const
{
    return mValues;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::setDiagonalImpl( const Scalar value )
{
    const IndexType numDiagonalElements = std::min( mNumColumns, mNumRows );

    if( !mDiagonalProperty )
    {
        COMMON_THROWEXCEPTION( "setDiagonal: matrix storage has not diagonal property." )
    }

    ValueType val = value.getValue<ValueType>();

    ReadAccess<IndexType> wIa( mIa );
    WriteAccess<ValueType> wValues( mValues );

    for( IndexType i = 0; i < numDiagonalElements; ++i )
    {
        wValues[wIa[i]] = val;
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
template<typename OtherValueType>
void CSRStorage<ValueType>::setDiagonalImpl( const LAMAArray<OtherValueType>& diagonal )
{
    IndexType numDiagonalElements = diagonal.size();

    {
        ReadAccess<OtherValueType> rDiagonal( diagonal );
        ReadAccess<IndexType> csrIA( mIa );

        WriteAccess<ValueType> wValues( mValues ); // partial setting

        //  wValues[ wIa[ i ] ] = rDiagonal[ i ];

        OpenMPUtils::setScatter( wValues.get(), csrIA.get(), rDiagonal.get(), numDiagonalElements );
    }

    if( SCAI_LOG_TRACE_ON( logger ) )
    {
        SCAI_LOG_TRACE( logger, "CSR after setDiagonal" )
        print();
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
template<typename OtherType>
void CSRStorage<ValueType>::getRowImpl( LAMAArray<OtherType>& row, const IndexType i ) const
{
    SCAI_ASSERT_DEBUG( i >= 0 && i < mNumRows, "row index " << i << " out of range" )

    WriteOnlyAccess<OtherType> wRow( row, mNumColumns );

    const ReadAccess<IndexType> ia( mIa );
    const ReadAccess<IndexType> ja( mJa );
    const ReadAccess<ValueType> values( mValues );

    for( IndexType j = 0; j < mNumColumns; ++j )
    {
        wRow[j] = 0.0;
    }

    for( IndexType jj = ia[i]; jj < ia[i + 1]; ++jj )
    {
        wRow[ja[jj]] = static_cast<OtherType>( values[jj] );
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
template<typename OtherValueType>
void CSRStorage<ValueType>::getDiagonalImpl( LAMAArray<OtherValueType>& diagonal ) const
{
    const IndexType numDiagonalElements = std::min( mNumColumns, mNumRows );

    ContextPtr loc = getContextPtr();
    WriteOnlyAccess<OtherValueType> wDiagonal( diagonal, loc, numDiagonalElements );
    ReadAccess<IndexType> csrIA( mIa, loc );
    ReadAccess<ValueType> rValues( mValues, loc );

    SCAI_CONTEXT_ACCESS( loc )
    LAMA_INTERFACE_FN_TT( setGather, loc, Utils, Copy, OtherValueType, ValueType )

    setGather( wDiagonal.get(), rValues.get(), csrIA.get(), numDiagonalElements );

    /*
     const IndexType numDiagonalElements = std::min( mNumColumns, mNumRows );

     WriteOnlyAccess<OtherValueType> wDiagonal( diagonal, numDiagonalElements );

     ReadAccess<IndexType> csrIA( mIa );
     ReadAccess<ValueType> rValues( mValues );

     //  diagonal[ i ] = rValues[ csrIA[ i ] ], diagonal values are at the offsets

     OpenMPUtils::setGather( wDiagonal.get(), rValues.get(), csrIA.get(), numDiagonalElements );
     */
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::scaleImpl( const Scalar scalar )
{
    ContextPtr loc = getContextPtr();

    LAMA_INTERFACE_FN_DEFAULT_T( scale, loc, Utils, Transform, ValueType )

    WriteAccess<ValueType> csrValues( mValues, loc );

	SCAI_CONTEXT_ACCESS( loc )

    ValueType value = scalar.getValue<ValueType>();

    scale( csrValues.get(), value, mNumValues );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
template<typename OtherValueType>
void CSRStorage<ValueType>::scaleImpl( const LAMAArray<OtherValueType>& diagonal )
{
    IndexType n = std::min( mNumRows, diagonal.size() );

    ContextPtr loc = getContextPtr();

    LAMA_INTERFACE_FN_TT( scaleRows, loc, CSRUtils, Scale, ValueType, OtherValueType )

    SCAI_CONTEXT_ACCESS( loc )

    {
        ReadAccess<OtherValueType> rDiagonal( diagonal, loc );
        ReadAccess<IndexType> csrIA( mIa, loc );
        WriteAccess<ValueType> csrValues( mValues, loc ); // updateAccess

        scaleRows( csrValues.get(), csrIA.get(), n, rDiagonal.get() );
    }

    if( SCAI_LOG_TRACE_ON( logger ) )
    {
        SCAI_LOG_TRACE( logger, "CSR after scale diagonal" )
        print();
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::wait() const
{
    mIa.wait();
    mJa.wait();
    mValues.wait();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::assign( const _MatrixStorage& other )
{
    if( &other == this )
    {
        // this special case avoids copying of data but is also
        // mandatory to avoid conflicting read/write accesses on LAMA arrays

        SCAI_LOG_INFO( logger, typeName() << ": self assign, skipped, matrix = " << other )

        return;
    }

    SCAI_LOG_INFO( logger, typeName() << ": assign " << other )

    // Nearly the same routine as MatrixStorage::assign but here we
    // do not need any temporary data for ia, ja, and values

    other.buildCSRData( mIa, mJa, mValues );

    // actualize my member variables (class CSRStorage)

    _MatrixStorage::_assign( other ); // copy sizes

    mNumValues = mJa.size();

    mDiagonalProperty = checkDiagonalProperty();

    buildRowIndexes();

    check( "assign" );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::assignTranspose( const MatrixStorage<ValueType>& other )
{
    SCAI_LOG_INFO( logger, *this << ": (CSR) assign transpose " << other )

    _MatrixStorage::_assignTranspose( other );

    // pass LAMAArrays of this storage to build the values in it

    if( &other == this )
    {
        LAMAArray<IndexType> tmpIA;
        LAMAArray<IndexType> tmpJA;
        LAMAArray<ValueType> tmpValues;

        other.buildCSCData( tmpIA, tmpJA, tmpValues );
        swap( tmpIA, tmpJA, tmpValues );
    }
    else
    {
        other.buildCSCData( mIa, mJa, mValues );
        mNumValues = mJa.size();
        mDiagonalProperty = checkDiagonalProperty();
        buildRowIndexes();
    }

    // actualize my member variables (class CSRStorage)

    check( "assignTranspose" );

}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::copyTo( _MatrixStorage& other ) const
{
    // Compressed sparse column data can be set directly to other matrix

    other.setCSRData( mNumRows, mNumColumns, mNumValues, mIa, mJa, mValues );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
template<typename OtherValueType>
void CSRStorage<ValueType>::buildCSR(
    LAMAArray<IndexType>& ia,
    LAMAArray<IndexType>* ja,
    LAMAArray<OtherValueType>* values,
    const ContextPtr loc ) const
{
    ReadAccess<IndexType> inIA( mIa, loc );

    //build number of values per row into ia
    if( ja == NULL || values == NULL )
    {
        WriteOnlyAccess<IndexType> csrIA( ia, loc, mNumRows );

        LAMA_INTERFACE_FN( offsets2sizes, loc, CSRUtils, Offsets )
        SCAI_CONTEXT_ACCESS( loc )

        offsets2sizes( csrIA.get(), inIA.get(), mNumRows );
        return;
    }

    // copy the offset array ia and ja
    {
        ReadAccess<IndexType> inJA( mJa, loc );
        WriteOnlyAccess<IndexType> csrIA( ia, loc, mNumRows + 1 );
        WriteOnlyAccess<IndexType> csrJA( *ja, loc, mNumValues );

        SCAI_CONTEXT_ACCESS( loc )
        LAMA_INTERFACE_FN_TT( set, loc, Utils, Copy, IndexType, IndexType )
        set( csrIA.get(), inIA.get(), mNumRows + 1 );
        set( csrJA.get(), inJA.get(), mNumValues );
    }

    // copy values
    {
        ReadAccess<ValueType> inValues( mValues, loc );
        WriteOnlyAccess<OtherValueType> csrValues( *values, loc, mNumValues );

        SCAI_CONTEXT_ACCESS( loc )
        LAMA_INTERFACE_FN_TT( set, loc, Utils, Copy, OtherValueType, ValueType )
        set( csrValues.get(), inValues.get(), mNumValues );
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::buildCSCData(
    LAMAArray<IndexType>& colIA,
    LAMAArray<IndexType>& colJA,
    LAMAArray<ValueType>& colValues ) const
{
    SCAI_LOG_INFO( logger, *this << ": buildCSCData by call of CSR2CSC" )

    // build the CSC data directly on the device where this matrix is located.

    this->convertCSR2CSC( colIA, colJA, colValues, mNumColumns, mIa, mJa, mValues, this->getContextPtr() );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::splitHalo(
    MatrixStorage<ValueType>& localData,
    MatrixStorage<ValueType>& haloData,
    Halo& halo,
    const Distribution& colDist,
    const Distribution* rowDist ) const
{
    SCAI_REGION( "Storage.splitHalo" )

    SCAI_LOG_INFO( logger, *this << ": split CSR according to column distribution " << colDist )

    SCAI_ASSERT_EQUAL( mNumColumns, colDist.getGlobalSize() )

    if( colDist.isReplicated() )
    {
        // if there is no column distribution, halo is not needed

        if( rowDist )
        {
            localData.localize( *this, *rowDist );
        }
        else
        {
            localData.assign( *this );
        }

        haloData.allocate( mNumRows, 0 );

        halo = Halo(); // empty halo schedule

        return;
    }

    IndexType numRows = mNumRows;

    // check optional row distribution if specified

    if( rowDist )
    {
        SCAI_LOG_INFO( logger, *this << ": split also localizes for " << *rowDist )
        SCAI_ASSERT_EQUAL( mNumRows, rowDist->getGlobalSize() )
        numRows = rowDist->getLocalSize();
    }

    LAMAArray<IndexType> localIA;
    LAMAArray<IndexType> localJA;
    LAMAArray<ValueType> localValues;

    LAMAArray<IndexType> haloIA;
    LAMAArray<IndexType> haloJA;
    LAMAArray<ValueType> haloValues;

    StorageMethods<ValueType>::splitCSR( localIA, localJA, localValues, haloIA, haloJA, haloValues, mIa, mJa, mValues,
                                         colDist, rowDist );

    SCAI_ASSERT_EQUAL_DEBUG( localIA.size(), numRows + 1 )
    SCAI_ASSERT_EQUAL_DEBUG( haloIA.size(), numRows + 1 )

    const IndexType haloNumValues = haloJA.size();
    const IndexType localNumValues = localJA.size();

    SCAI_LOG_INFO( logger,
                   *this << ": split into " << localNumValues << " local non-zeros " " and " << haloNumValues << " halo non-zeros" )

    const IndexType localNumColumns = colDist.getLocalSize();

    IndexType haloNumColumns; // will be available after remap

    // build the halo by the non-local indexes

    _StorageMethods::buildHalo( halo, haloJA, haloNumColumns, colDist );

    SCAI_LOG_INFO( logger, "build halo: " << halo )

    localData.setCSRData( numRows, localNumColumns, localNumValues, localIA, localJA, localValues );

    localData.check( "local part after split" );

    // halo data is expected to have many empty rows, so enable compressing with row indexes

    haloData.setCompressThreshold( 0.5 );

    haloData.setCSRData( numRows, haloNumColumns, haloNumValues, haloIA, haloJA, haloValues );

    haloData.check( "halo part after split" );

    SCAI_LOG_INFO( logger,
                   "Result of split: local storage = " << localData << ", halo storage = " << haloData << ", halo = " << halo )
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::matrixTimesVector(
    LAMAArray<ValueType>& result,
    const ValueType alpha,
    const LAMAArray<ValueType>& x,
    const ValueType beta,
    const LAMAArray<ValueType>& y ) const
{
    SCAI_LOG_INFO( logger,
                   *this << ": matrixTimesVector, result = " << result << ", alpha = " << alpha << ", x = " << x << ", beta = " << beta << ", y = " << y )

    SCAI_REGION( "Storage.CSR.timesVector" )

    SCAI_ASSERT_EQUAL_ERROR( x.size(), mNumColumns )
    SCAI_ASSERT_EQUAL_ERROR( result.size(), mNumRows )

    if( ( beta != 0.0 ) && ( &result != &y ) )
    {
        SCAI_ASSERT_EQUAL_ERROR( y.size(), mNumRows )
    }

    ContextPtr loc = getContextPtr();

    SCAI_LOG_INFO( logger, *this << ": matrixTimesVector on " << *loc )

    LAMA_INTERFACE_FN_T( sparseGEMV, loc, CSRUtils, Mult, ValueType )
    LAMA_INTERFACE_FN_T( normalGEMV, loc, CSRUtils, Mult, ValueType )

    ReadAccess<IndexType> csrIA( mIa, loc );
    ReadAccess<IndexType> csrJA( mJa, loc );
    ReadAccess<ValueType> csrValues( mValues, loc );

    ReadAccess<ValueType> rX( x, loc );

    // Possible alias of result and y must be handled by coressponding accesses

    if( &result == &y )
    {
        // only write access for y, no read access for result

        WriteAccess<ValueType> wResult( result, loc );

        if( mRowIndexes.size() > 0 && ( beta == 1.0 ) )
        {
            // y += alpha * thisMatrix * x, can take advantage of row indexes

            IndexType numNonZeroRows = mRowIndexes.size();
            ReadAccess<IndexType> rows( mRowIndexes, loc );

            SCAI_CONTEXT_ACCESS( loc )
            sparseGEMV( wResult.get(), alpha, rX.get(), numNonZeroRows, rows.get(), csrIA.get(), csrJA.get(),
                        csrValues.get(), NULL );
        }
        else
        {
            // we assume that normalGEMV can deal with the alias of result, y

            SCAI_CONTEXT_ACCESS( loc )
            normalGEMV( wResult.get(), alpha, rX.get(), beta, wResult.get(), mNumRows, mNumColumns, mNumValues,
                        csrIA.get(), csrJA.get(), csrValues.get(), NULL );
        }
    }
    else
    {
        WriteOnlyAccess<ValueType> wResult( result, loc, mNumRows );
        ReadAccess<ValueType> rY( y, loc );

        SCAI_CONTEXT_ACCESS( loc )
        normalGEMV( wResult.get(), alpha, rX.get(), beta, rY.get(), mNumRows, mNumColumns, mNumValues, csrIA.get(),
                    csrJA.get(), csrValues.get(), NULL );
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::vectorTimesMatrix(
    LAMAArray<ValueType>& result,
    const ValueType alpha,
    const LAMAArray<ValueType>& x,
    const ValueType beta,
    const LAMAArray<ValueType>& y ) const
{
    SCAI_LOG_INFO( logger,
                   *this << ": vectorTimesMatrix, result = " << result << ", alpha = " << alpha << ", x = " << x << ", beta = " << beta << ", y = " << y )

    SCAI_REGION( "Storage.CSR.VectorTimesMatrix" )

    SCAI_ASSERT_EQUAL_ERROR( x.size(), mNumRows )
    SCAI_ASSERT_EQUAL_ERROR( result.size(), mNumColumns )

    if( ( beta != 0.0 ) && ( &result != &y ) )
    {
        SCAI_ASSERT_EQUAL_ERROR( y.size(), mNumColumns )
    }

    ContextPtr loc = getContextPtr();

    SCAI_LOG_INFO( logger, *this << ": vectorTimesMatrix on " << *loc )

    LAMA_INTERFACE_FN_T( sparseGEVM, loc, CSRUtils, Mult, ValueType )
    LAMA_INTERFACE_FN_T( normalGEVM, loc, CSRUtils, Mult, ValueType )

    ReadAccess<IndexType> csrIA( mIa, loc );
    ReadAccess<IndexType> csrJA( mJa, loc );
    ReadAccess<ValueType> csrValues( mValues, loc );

    ReadAccess<ValueType> rX( x, loc );

    // Possible alias of result and y must be handled by coressponding accesses

    if( &result == &y )
    {
        // only write access for y, no read access for result

        WriteAccess<ValueType> wResult( result, loc );

        if( mRowIndexes.size() > 0 && ( beta == 1.0 ) )
        {
            // y += alpha * thisMatrix * x, can take advantage of row indexes

            IndexType numNonZeroRows = mRowIndexes.size();
            ReadAccess<IndexType> rows( mRowIndexes, loc );

            SCAI_CONTEXT_ACCESS( loc )
            sparseGEVM( wResult.get(), alpha, rX.get(), mNumColumns, numNonZeroRows, rows.get(), csrIA.get(),
                        csrJA.get(), csrValues.get(), NULL );
        }
        else
        {
            // we assume that normalGEMV can deal with the alias of result, y

            SCAI_CONTEXT_ACCESS( loc )
            normalGEVM( wResult.get(), alpha, rX.get(), beta, wResult.get(), mNumRows, mNumColumns, csrIA.get(),
                        csrJA.get(), csrValues.get(), NULL );
        }
    }
    else
    {
        WriteOnlyAccess<ValueType> wResult( result, loc, mNumColumns );
        ReadAccess<ValueType> rY( y, loc );

        SCAI_CONTEXT_ACCESS( loc )
        normalGEVM( wResult.get(), alpha, rX.get(), beta, rY.get(), mNumRows, mNumColumns, csrIA.get(), csrJA.get(),
                    csrValues.get(), NULL );
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::matrixTimesVectorN(
    LAMAArray<ValueType>& result,
    const IndexType n,
    const ValueType alpha,
    const LAMAArray<ValueType>& x,
    const ValueType beta,
    const LAMAArray<ValueType>& y ) const
{
    SCAI_LOG_INFO( logger,
                   *this << ": matrixTimesVector, result = " << result << ", alpha = " << alpha << ", x = " << x << ", beta = " << beta << ", y = " << y )

    SCAI_REGION( "Storage.CSR.timesVectorN" )

    SCAI_ASSERT_EQUAL_ERROR( x.size(), n * mNumColumns )
    SCAI_ASSERT_EQUAL_ERROR( result.size(), n * mNumRows )

    if( ( beta != 0.0 ) && ( &result != &y ) )
    {
        SCAI_ASSERT_EQUAL_ERROR( y.size(), n * mNumRows )
    }

    ContextPtr loc = getContextPtr();

    SCAI_LOG_INFO( logger, *this << ": matrixTimesVectorN on " << *loc )

    LAMA_INTERFACE_FN_T( gemm, loc, CSRUtils, Mult, ValueType )

    ReadAccess<IndexType> csrIA( mIa, loc );
    ReadAccess<IndexType> csrJA( mJa, loc );
    ReadAccess<ValueType> csrValues( mValues, loc );

    ReadAccess<ValueType> rX( x, loc );

    // Possible alias of result and y must be handled by coressponding accesses

    if( &result == &y )
    {
        // only write access for y, no read access for result

        WriteAccess<ValueType> wResult( result, loc );

        // we assume that normalGEMV can deal with the alias of result, y

        SCAI_CONTEXT_ACCESS( loc )
        gemm( wResult.get(), alpha, rX.get(), beta, wResult.get(), mNumRows, n, mNumColumns, csrIA.get(), csrJA.get(),
              csrValues.get(), NULL );
    }
    else
    {
        WriteOnlyAccess<ValueType> wResult( result, loc, mNumRows );
        ReadAccess<ValueType> rY( y, loc );

        SCAI_CONTEXT_ACCESS( loc )
        gemm( wResult.get(), alpha, rX.get(), beta, rY.get(), mNumRows, n, mNumColumns, csrIA.get(), csrJA.get(),
              csrValues.get(), NULL );
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
SyncToken* CSRStorage<ValueType>::matrixTimesVectorAsync(
    LAMAArray<ValueType>& result,
    const ValueType alpha,
    const LAMAArray<ValueType>& x,
    const ValueType beta,
    const LAMAArray<ValueType>& y ) const
{
    SCAI_LOG_INFO( logger,
                   *this << ": matrixTimesVectorAsync, result = " << result << ", alpha = " << alpha << ", x = " << x << ", beta = " << beta << ", y = " << y )

    SCAI_REGION( "Storage.CSR.timesVectorAsync" )

    ContextPtr loc = getContextPtr();

    // Note: checks will be done by asynchronous task in any case
    //       and exception in tasks are handled correctly

    SCAI_LOG_INFO( logger, *this << ": matrixTimesVectorAsync on " << *loc )

    if( loc->getType() == context::Host )
    {
        // execution as separate thread

        void (CSRStorage::*pf)(
            LAMAArray<ValueType>&,
            const ValueType,
            const LAMAArray<ValueType>&,
            const ValueType,
            const LAMAArray<ValueType>& ) const

            = &CSRStorage<ValueType>::matrixTimesVector;

        using scai::common::bind;
        using scai::common::ref;
        using scai::common::cref;

        SCAI_LOG_INFO( logger, *this << ": matrixTimesVectorAsync on Host by own thread" )

        return new TaskSyncToken( bind( pf, this, ref( result ), alpha, cref( x ), beta, cref( y ) ) );
    }

    SCAI_ASSERT_EQUAL_ERROR( x.size(), mNumColumns )
    SCAI_ASSERT_EQUAL_ERROR( result.size(), mNumRows )

    if( ( beta != 0.0 ) && ( &result != &y ) )
    {
        SCAI_ASSERT_EQUAL_ERROR( y.size(), mNumRows )
    }

    LAMA_INTERFACE_FN_T( sparseGEMV, loc, CSRUtils, Mult, ValueType )
    LAMA_INTERFACE_FN_T( normalGEMV, loc, CSRUtils, Mult, ValueType )

    unique_ptr<SyncToken> syncToken( loc->getSyncToken() );

    // all accesses will be pushed to the sync token as LAMA arrays have to be protected up
    // to the end of the computations.

    shared_ptr<ReadAccess<IndexType> > csrIA( new ReadAccess<IndexType>( mIa, loc ) );
    shared_ptr<ReadAccess<IndexType> > csrJA( new ReadAccess<IndexType>( mJa, loc ) );
    shared_ptr<ReadAccess<ValueType> > csrValues( new ReadAccess<ValueType>( mValues, loc ) );
    shared_ptr<ReadAccess<ValueType> > rX( new ReadAccess<ValueType>( x, loc ) );

    // Possible alias of result and y must be handled by coressponding accesses

    if( &result == &y )
    {
        // only write access for y, no read access for result

        shared_ptr<WriteAccess<ValueType> > wResult( new WriteAccess<ValueType>( result, loc ) );

        if( mRowIndexes.size() > 0 && ( beta == 1.0 ) )
        {
            // y += alpha * thisMatrix * x, can take advantage of row indexes

            IndexType numNonZeroRows = mRowIndexes.size();

            shared_ptr<ReadAccess<IndexType> > rows( new ReadAccess<IndexType>( mRowIndexes, loc ) );

            syncToken->pushToken( rows );

            SCAI_CONTEXT_ACCESS( loc )

            sparseGEMV( wResult->get(), alpha, rX->get(), numNonZeroRows, rows->get(), csrIA->get(), csrJA->get(),
                        csrValues->get(), syncToken.get() );
        }
        else
        {
            // we assume that normalGEMV can deal with the alias of result, y

            SCAI_CONTEXT_ACCESS( loc )

            normalGEMV( wResult->get(), alpha, rX->get(), beta, wResult->get(), mNumRows, mNumColumns, mNumValues,
                        csrIA->get(), csrJA->get(), csrValues->get(), syncToken.get() );
        }

        syncToken->pushToken( wResult );
    }
    else
    {
        shared_ptr<WriteAccess<ValueType> > wResult( new WriteOnlyAccess<ValueType>( result, loc, mNumRows ) );
        shared_ptr<ReadAccess<ValueType> > rY( new ReadAccess<ValueType>( y, loc ) );

        SCAI_CONTEXT_ACCESS( loc )

        normalGEMV( wResult->get(), alpha, rX->get(), beta, rY->get(), mNumRows, mNumColumns, mNumValues, csrIA->get(),
                    csrJA->get(), csrValues->get(), syncToken.get() );

        syncToken->pushToken( wResult );
        syncToken->pushToken( rY );
    }

    syncToken->pushToken( csrIA );
    syncToken->pushToken( csrJA );
    syncToken->pushToken( csrValues );
    syncToken->pushToken( rX );

    return syncToken.release();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
SyncToken* CSRStorage<ValueType>::vectorTimesMatrixAsync(
    LAMAArray<ValueType>& result,
    const ValueType alpha,
    const LAMAArray<ValueType>& x,
    const ValueType beta,
    const LAMAArray<ValueType>& y ) const
{
    SCAI_LOG_INFO( logger,
                   *this << ": vectorTimesMatrixAsync, result = " << result << ", alpha = " << alpha << ", x = " << x << ", beta = " << beta << ", y = " << y )

    SCAI_REGION( "Storage.CSR.vectorTimesMatrixAsync" )

    ContextPtr loc = getContextPtr();

    // Note: checks will be done by asynchronous task in any case
    //       and exception in tasks are handled correctly

    SCAI_LOG_INFO( logger, *this << ": vectorTimesMatrixAsync on " << *loc )

    if( loc->getType() == context::Host )
    {
        // execution as separate thread

        void (CSRStorage::*pf)(
            LAMAArray<ValueType>&,
            const ValueType,
            const LAMAArray<ValueType>&,
            const ValueType,
            const LAMAArray<ValueType>& ) const

            = &CSRStorage<ValueType>::vectorTimesMatrix;

        using scai::common::bind;
        using scai::common::ref;
        using scai::common::cref;

        SCAI_LOG_INFO( logger, *this << ": vectorTimesMatrixAsync on Host by own thread" )

        return new TaskSyncToken( bind( pf, this, ref( result ), alpha, cref( x ), beta, cref( y ) ) );
    }

    SCAI_ASSERT_EQUAL_ERROR( x.size(), mNumRows )
    SCAI_ASSERT_EQUAL_ERROR( result.size(), mNumColumns )

    if( ( beta != 0.0 ) && ( &result != &y ) )
    {
        SCAI_ASSERT_EQUAL_ERROR( y.size(), mNumColumns )
    }

    LAMA_INTERFACE_FN_T( sparseGEVM, loc, CSRUtils, Mult, ValueType )
    LAMA_INTERFACE_FN_T( normalGEVM, loc, CSRUtils, Mult, ValueType )

    unique_ptr<SyncToken> syncToken( loc->getSyncToken() );

    // all accesses will be pushed to the sync token as LAMA arrays have to be protected up
    // to the end of the computations.

    shared_ptr<ReadAccess<IndexType> > csrIA( new ReadAccess<IndexType>( mIa, loc ) );
    shared_ptr<ReadAccess<IndexType> > csrJA( new ReadAccess<IndexType>( mJa, loc ) );
    shared_ptr<ReadAccess<ValueType> > csrValues( new ReadAccess<ValueType>( mValues, loc ) );
    shared_ptr<ReadAccess<ValueType> > rX( new ReadAccess<ValueType>( x, loc ) );

    // Possible alias of result and y must be handled by coressponding accesses

    if( &result == &y )
    {
        // only write access for y, no read access for result

        shared_ptr<WriteAccess<ValueType> > wResult( new WriteAccess<ValueType>( result, loc ) );

        if( mRowIndexes.size() > 0 && ( beta == 1.0 ) )
        {
            // y += alpha * thisMatrix * x, can take advantage of row indexes

            IndexType numNonZeroRows = mRowIndexes.size();

            shared_ptr<ReadAccess<IndexType> > rows( new ReadAccess<IndexType>( mRowIndexes, loc ) );

            syncToken->pushToken( rows );

            SCAI_CONTEXT_ACCESS( loc )

            sparseGEVM( wResult->get(), alpha, rX->get(), mNumColumns, numNonZeroRows, rows->get(), csrIA->get(),
                        csrJA->get(), csrValues->get(), syncToken.get() );
        }
        else
        {
            // we assume that normalGEMV can deal with the alias of result, y

            SCAI_CONTEXT_ACCESS( loc )

            normalGEVM( wResult->get(), alpha, rX->get(), beta, wResult->get(), mNumRows, mNumColumns, csrIA->get(),
                        csrJA->get(), csrValues->get(), syncToken.get() );
        }

        syncToken->pushToken( wResult );
    }
    else
    {
        shared_ptr<WriteAccess<ValueType> > wResult( new WriteOnlyAccess<ValueType>( result, loc, mNumColumns ) );
        shared_ptr<ReadAccess<ValueType> > rY( new ReadAccess<ValueType>( y, loc ) );

        SCAI_CONTEXT_ACCESS( loc )

        normalGEVM( wResult->get(), alpha, rX->get(), beta, rY->get(), mNumRows, mNumColumns, csrIA->get(),
                    csrJA->get(), csrValues->get(), syncToken.get() );

        syncToken->pushToken( wResult );
        syncToken->pushToken( rY );
    }

    syncToken->pushToken( csrIA );
    syncToken->pushToken( csrJA );
    syncToken->pushToken( csrValues );
    syncToken->pushToken( rX );

    return syncToken.release();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::jacobiIterate(
    LAMAArray<ValueType>& solution,
    const LAMAArray<ValueType>& oldSolution,
    const LAMAArray<ValueType>& rhs,
    const ValueType omega ) const
{
    SCAI_REGION( "Storage.CSR.jacobiIterate" )

    SCAI_LOG_INFO( logger, *this << ": Jacobi iteration for local matrix data." )

    SCAI_ASSERT_ERROR( mDiagonalProperty, *this << ": jacobiIterate requires diagonal property" )

    if( &solution == &oldSolution )
    {
        COMMON_THROWEXCEPTION( "alias of solution and oldSolution unsupported" )
    }

    SCAI_ASSERT_EQUAL_DEBUG( mNumRows, oldSolution.size() )
    SCAI_ASSERT_EQUAL_DEBUG( mNumRows, solution.size() )
    SCAI_ASSERT_EQUAL_DEBUG( mNumRows, mNumColumns )
    // matrix must be square

    ContextPtr loc = getContextPtr();

    // loc = ContextFactory::getContext( context::Host );  // does not run on other devices

    LAMA_INTERFACE_FN_T( jacobi, loc, CSRUtils, Solver, ValueType )

    WriteAccess<ValueType> wSolution( solution, loc );
    ReadAccess<IndexType> csrIA( mIa, loc );
    ReadAccess<IndexType> csrJA( mJa, loc );
    ReadAccess<ValueType> csrValues( mValues, loc );
    ReadAccess<ValueType> rOldSolution( oldSolution, loc );
    ReadAccess<ValueType> rRhs( rhs, loc );

    // Due to diagonal property there is no advantage by taking row indexes

    SCAI_CONTEXT_ACCESS( loc )

    jacobi( wSolution.get(), csrIA.get(), csrJA.get(), csrValues.get(), rOldSolution.get(), rRhs.get(), omega, mNumRows,
            NULL );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::jacobiIterateHalo(
    LAMAArray<ValueType>& localSolution,
    const MatrixStorage<ValueType>& localStorage,
    const LAMAArray<ValueType>& oldHaloSolution,
    const ValueType omega ) const
{
    SCAI_REGION( "Storage.CSR.jacobiIterateHalo" )

    SCAI_LOG_INFO( logger, *this << ": Jacobi iteration for halo matrix data." )

    SCAI_ASSERT_EQUAL_DEBUG( mNumRows, localSolution.size() )
    SCAI_ASSERT_EQUAL_DEBUG( mNumRows, localStorage.getNumRows() )
    SCAI_ASSERT_EQUAL_DEBUG( mNumRows, localStorage.getNumColumns() )
    SCAI_ASSERT_DEBUG( localStorage.hasDiagonalProperty(), localStorage << ": has not diagonal property" )
    SCAI_ASSERT_EQUAL_DEBUG( mNumColumns, oldHaloSolution.size() )

    const CSRStorage<ValueType>* csrLocal;

    if( localStorage.getFormat() == Format::CSR )
    {
        csrLocal = dynamic_cast<const CSRStorage<ValueType>*>( &localStorage );
        SCAI_ASSERT_DEBUG( csrLocal, "could not cast to CSRStorage " << localStorage )
    }
    else
    {
        // either copy localStorage to CSR (not recommended) or
        // just get the diagonal in localValues and set order in localIA
        COMMON_THROWEXCEPTION( "local stroage is not CSR" )
    }

    ContextPtr loc = getContextPtr();

    LAMA_INTERFACE_FN_T( jacobiHalo, loc, CSRUtils, Solver, ValueType )

    {
        WriteAccess<ValueType> wSolution( localSolution, loc ); // will be updated
        ReadAccess<IndexType> localIA( csrLocal->mIa, loc );
        ReadAccess<ValueType> localValues( csrLocal->mValues, loc );
        ReadAccess<IndexType> haloIA( mIa, loc );
        ReadAccess<IndexType> haloJA( mJa, loc );
        ReadAccess<ValueType> haloValues( mValues, loc );
        ReadAccess<ValueType> rOldHaloSolution( oldHaloSolution, loc );

        const IndexType numNonEmptyRows = mRowIndexes.size();

        SCAI_LOG_INFO( logger, "#row indexes = " << numNonEmptyRows )

        if( numNonEmptyRows != 0 )
        {
            ReadAccess<IndexType> haloRowIndexes( mRowIndexes, loc );

            SCAI_CONTEXT_ACCESS( loc )

            jacobiHalo( wSolution.get(), localIA.get(), localValues.get(), haloIA.get(), haloJA.get(), haloValues.get(),
                        haloRowIndexes.get(), rOldHaloSolution.get(), omega, numNonEmptyRows );
        }
        else
        {
            SCAI_CONTEXT_ACCESS( loc )

            jacobiHalo( wSolution.get(), localIA.get(), localValues.get(), haloIA.get(), haloJA.get(), haloValues.get(),
                        NULL,
                        rOldHaloSolution.get(), omega, mNumRows );
        }
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::jacobiIterateHalo(
    LAMAArray<ValueType>& localSolution,
    const LAMAArray<ValueType>& localDiagonal,
    const LAMAArray<ValueType>& oldHaloSolution,
    const ValueType omega ) const
{
    SCAI_REGION( "Storage.CSR.jacobiIterateHalo" )

    SCAI_LOG_INFO( logger, *this << ": Jacobi iteration for halo matrix data." )

    SCAI_ASSERT_EQUAL_DEBUG( mNumRows, localSolution.size() )
    SCAI_ASSERT_EQUAL_DEBUG( mNumColumns, oldHaloSolution.size() )

    ContextPtr loc = getContextPtr();

    LAMA_INTERFACE_FN_T( jacobiHaloWithDiag, loc, CSRUtils, Solver, ValueType )

    {
        WriteAccess<ValueType> wSolution( localSolution, loc ); // will be updated
        ReadAccess<ValueType> localDiagValues( localDiagonal, loc );
        ReadAccess<IndexType> haloIA( mIa, loc );
        ReadAccess<IndexType> haloJA( mJa, loc );
        ReadAccess<ValueType> haloValues( mValues, loc );
        ReadAccess<ValueType> rOldHaloSolution( oldHaloSolution, loc );

        const IndexType numNonEmptyRows = mRowIndexes.size();

        SCAI_LOG_INFO( logger, "#row indexes = " << numNonEmptyRows )

        if( numNonEmptyRows != 0 )
        {
            ReadAccess<IndexType> haloRowIndexes( mRowIndexes, loc );

            SCAI_CONTEXT_ACCESS( loc )

            jacobiHaloWithDiag( wSolution.get(), localDiagValues.get(), haloIA.get(), haloJA.get(), haloValues.get(),
                                haloRowIndexes.get(), rOldHaloSolution.get(), omega, numNonEmptyRows );
        }
        else
        {
            SCAI_CONTEXT_ACCESS( loc )

            jacobiHaloWithDiag( wSolution.get(), localDiagValues.get(), haloIA.get(), haloJA.get(), haloValues.get(),
                                NULL,
                                rOldHaloSolution.get(), omega, mNumRows );
        }
    }
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::matrixPlusMatrix(
    const ValueType alpha,
    const MatrixStorage<ValueType>& a,
    const ValueType beta,
    const MatrixStorage<ValueType>& b )
{
    SCAI_LOG_INFO( logger, "this = " << alpha << " * A + " << beta << " * B" << ", with A = " << a << ", B = " << b )

    SCAI_REGION( "Storage.CSR.plusMatrix" )

    // a and b have to be CSR storages, otherwise create temporaries.

    const CSRStorage<ValueType>* csrA = NULL;
    const CSRStorage<ValueType>* csrB = NULL;

    // Define shared pointers in case we need temporaries

    common::shared_ptr<CSRStorage<ValueType> > tmpA;
    common::shared_ptr<CSRStorage<ValueType> > tmpB;

    if( a.getFormat() == Format::CSR )
    {
        csrA = dynamic_cast<const CSRStorage<ValueType>*>( &a );
        SCAI_ASSERT_DEBUG( csrA, "could not cast to CSRStorage " << a )
    }
    else
    {
        LAMA_UNSUPPORTED( a << ": will be converted to CSR for matrix multiply" )
        tmpA = common::shared_ptr<CSRStorage<ValueType> >( new CSRStorage<ValueType>( a ) );
        csrA = tmpA.get();
    }

    if( b.getFormat() == Format::CSR )
    {
        csrB = dynamic_cast<const CSRStorage<ValueType>*>( &b );
        SCAI_ASSERT_DEBUG( csrB, "could not cast to CSRStorage " << b )
    }
    else
    {
        LAMA_UNSUPPORTED( b << ": will be converted to CSR for matrix multiply" )
        tmpB = common::shared_ptr<CSRStorage<ValueType> >( new CSRStorage<ValueType>( b ) );
        csrB = tmpB.get();
    }

    // compute where target data will be

    ContextPtr loc = this->getContextPtr();

    matrixAddMatrixCSR( alpha, *csrA, beta, *csrB, loc );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::matrixTimesMatrix(
    const ValueType alpha,
    const MatrixStorage<ValueType>& a,
    const MatrixStorage<ValueType>& b,
    const ValueType beta,
    const MatrixStorage<ValueType>& c )
{
    SCAI_LOG_INFO( logger,
                   "this = " << alpha << " +* A * B + " << beta << " * C, with " << "A = " << a << ", B = " << b << ", C = " << c )

    SCAI_REGION( "Storage.CSR.timesMatrix" )

    // a and b have to be CSR storages, otherwise create temporaries.

    const CSRStorage<ValueType>* csrA = NULL;
    const CSRStorage<ValueType>* csrB = NULL;
    const CSRStorage<ValueType>* csrC = NULL;

    // Define two shared pointers in case we need temporaries

    common::shared_ptr<CSRStorage<ValueType> > tmpA;
    common::shared_ptr<CSRStorage<ValueType> > tmpB;
    common::shared_ptr<CSRStorage<ValueType> > tmpC;

    if( a.getFormat() == Format::CSR )
    {
        csrA = dynamic_cast<const CSRStorage<ValueType>*>( &a );
        SCAI_ASSERT_DEBUG( csrA, "could not cast to CSRStorage " << a )
    }
    else
    {
        LAMA_UNSUPPORTED( a << ": will be converted to CSR for matrix multiply" )
        tmpA = common::shared_ptr<CSRStorage<ValueType> >( new CSRStorage<ValueType>( a ) );
        csrA = tmpA.get();
    }

    if( b.getFormat() == Format::CSR )
    {
        csrB = dynamic_cast<const CSRStorage<ValueType>*>( &b );
        SCAI_ASSERT_DEBUG( csrB, "could not cast to CSRStorage " << b )
    }
    else
    {
        LAMA_UNSUPPORTED( b << ": will be converted to CSR for matrix multiply" )
        tmpB = common::shared_ptr<CSRStorage<ValueType> >( new CSRStorage<ValueType>( b ) );
        csrB = tmpB.get();
    }

    if( beta != 0.0 )
    {
        // c temporary needed if not correct format/type or aliased to this

        if( ( c.getFormat() == Format::CSR ) && ( &c != this ) )
        {
            csrC = dynamic_cast<const CSRStorage<ValueType>*>( &c );
            SCAI_ASSERT_DEBUG( csrC, "could not cast to CSRStorage " << c )
        }
        else
        {
            LAMA_UNSUPPORTED( c << ": CSR temporary required for matrix add" )
            tmpC = common::shared_ptr<CSRStorage<ValueType> >( new CSRStorage<ValueType>( c ) );
            csrC = tmpC.get();
        }

    }

    // now we have in any case all arguments as CSR Storage

    ContextPtr loc = Context::getContextPtr( context::Host );

    if( a.getContext().getType() == b.getContext().getType() )
    {
        loc = a.getContextPtr();
    }

    ContextPtr saveContext = getContextPtr();

    CSRStorage<ValueType> tmp1;
    tmp1.matrixTimesMatrixCSR( alpha, *csrA, *csrB, loc );
    tmp1.setContext( loc );

    if( beta != 0 )
    {
        CSRStorage<ValueType> tmp2;
        tmp2.matrixAddMatrixCSR( 1.0, tmp1, beta, *csrC, loc );
        swap( tmp2 );
    }
    else
    {
        swap( tmp1 );
    }

    this->setContext( saveContext );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::matrixAddMatrixCSR(
    const ValueType alpha,
    const CSRStorage<ValueType>& a,
    const ValueType beta,
    const CSRStorage<ValueType>& b,
    const ContextPtr loc )
{
    SCAI_LOG_INFO( logger,
                   "this = " << alpha << " * A + " << beta << " * B, with " << "A = " << a << ", B = " << b << ", all are CSR" )

//    // TODO: just temporary, MAKE loc const again!
//    loc = Context::getContextPtr( context::Host );

    LAMA_INTERFACE_FN( matrixAddSizes, loc, CSRUtils, Offsets )
    LAMA_INTERFACE_FN_T( matrixAdd, loc, CSRUtils, Mult, ValueType )

    if( &a == this || &b == this )
    {
        // due to alias we would get problems with Write/Read access, so use a temporary

        CSRStorage<ValueType> tmp;

        tmp.matrixAddMatrixCSR( alpha, a, beta, b, loc );

        swap( tmp ); // safe as tmp will be destroyed afterwards

        return;
    }

    SCAI_REGION( "Storage.CSR.addMatrixCSR" )

    allocate( a.getNumRows(), a.getNumColumns() );

    SCAI_ASSERT_EQUAL_ERROR( mNumRows, b.getNumRows() )
    SCAI_ASSERT_EQUAL_ERROR( mNumColumns, b.getNumColumns() )

    mDiagonalProperty = ( mNumRows == mNumColumns );

    {
        ReadAccess<IndexType> aIa( a.getIA(), loc );
        ReadAccess<IndexType> aJa( a.getJA(), loc );
        ReadAccess<ValueType> aValues( a.getValues(), loc );

        ReadAccess<IndexType> bIa( b.getIA(), loc );
        ReadAccess<IndexType> bJa( b.getJA(), loc );
        ReadAccess<ValueType> bValues( b.getValues(), loc );

        // Step 1: compute row sizes of C, build offsets
        SCAI_LOG_DEBUG( logger, "Determing sizes of result matrix C" )

        WriteOnlyAccess<IndexType> cIa( mIa, loc, mNumRows + 1 );

        SCAI_CONTEXT_ACCESS( loc )

        mNumValues = matrixAddSizes( cIa.get(), mNumRows, mNumColumns, mDiagonalProperty, aIa.get(), aJa.get(),
                                     bIa.get(), bJa.get() );

        // Step 2: fill in ja, values

        SCAI_LOG_DEBUG( logger, "Compute the sparse values, # = " << mNumValues )

        WriteOnlyAccess<IndexType> cJa( mJa, loc, mNumValues );
        WriteOnlyAccess<ValueType> cValues( mValues, loc, mNumValues );

        matrixAdd( cJa.get(), cValues.get(), cIa.get(), mNumRows, mNumColumns, mDiagonalProperty, alpha, aIa.get(),
                   aJa.get(), aValues.get(), beta, bIa.get(), bJa.get(), bValues.get() );
    }

    SCAI_LOG_DEBUG( logger, *this << ": compress by removing zero elements" )

    // Computation of C might have produced some zero elements

    //compress();

    check( "result of matrix + matrix" ); // just verify for a correct matrix
}

/* --------------------------------------------------------------------------- */

//TODO: just temporary
template<typename ValueType>
void CSRStorage<ValueType>::setNumValues( const IndexType numValues )
{
    mNumValues = numValues;
}

template<typename ValueType>
void CSRStorage<ValueType>::matrixTimesMatrixCSR(
    const ValueType alpha,
    const CSRStorage<ValueType>& a,
    const CSRStorage<ValueType>& b,
    const ContextPtr loc )
{
    SCAI_LOG_INFO( logger,
                   *this << ": = " << alpha << " * A * B, with " << "A = " << a << ", B = " << b << ", all are CSR" << ", Context = " << loc->getType() )

    LAMA_INTERFACE_FN( matrixMultiplySizes, loc, CSRUtils, Offsets )
    LAMA_INTERFACE_FN_T( matrixMultiply, loc, CSRUtils, Mult, ValueType )

    SCAI_ASSERT_ERROR( &a != this, "matrixTimesMatrix: alias of a with this result matrix" )
    SCAI_ASSERT_ERROR( &b != this, "matrixTimesMatrix: alias of b with this result matrix" )

    SCAI_ASSERT_EQUAL_ERROR( a.getNumColumns(), b.getNumRows() )

    IndexType k = a.getNumColumns();

    SCAI_REGION( "Storage.CSR.timesMatrixCSR" )

    allocate( a.getNumRows(), b.getNumColumns() );

    mDiagonalProperty = ( mNumRows == mNumColumns );

    {
        ReadAccess<IndexType> aIA( a.getIA(), loc );
        ReadAccess<IndexType> aJA( a.getJA(), loc );
        ReadAccess<ValueType> aValues( a.getValues(), loc );

        ReadAccess<IndexType> bIA( b.getIA(), loc );
        ReadAccess<IndexType> bJA( b.getJA(), loc );
        ReadAccess<ValueType> bValues( b.getValues(), loc );

        WriteOnlyAccess<IndexType> cIA( mIa, loc, mNumRows + 1 );

        SCAI_CONTEXT_ACCESS( loc )

        mNumValues = matrixMultiplySizes( cIA.get(), mNumRows, mNumColumns, k, mDiagonalProperty, aIA.get(), aJA.get(),
                                          bIA.get(), bJA.get() );

        WriteOnlyAccess<IndexType> cJa( mJa, loc, mNumValues );
        WriteOnlyAccess<ValueType> cValues( mValues, loc, mNumValues );

        matrixMultiply( cIA.get(), cJa.get(), cValues.get(), mNumRows, mNumColumns, k, alpha, mDiagonalProperty,
                        aIA.get(), aJA.get(), aValues.get(), bIA.get(), bJA.get(), bValues.get() );
    }

    // TODO: check this!
//    compress();
    buildRowIndexes();
//    check( "result of matrix x matrix" ); // just verify for a correct matrix
//    mDiagonalProperty = checkDiagonalProperty();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType CSRStorage<ValueType>::l1Norm() const
{
	SCAI_LOG_INFO( logger, *this << ": l1Norm()" )

    if( mNumValues == 0 )
    {
        return 0.0f;
    }

	ContextPtr loc = getContextPtr();

    LAMA_INTERFACE_FN_T( asum, loc, BLAS, BLAS1, ValueType )

	ReadAccess<ValueType> data( mValues, loc );

	SCAI_CONTEXT_ACCESS( loc );

	return asum( mNumValues, data.get(), 1, NULL );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType CSRStorage<ValueType>::l2Norm() const
{
	SCAI_LOG_INFO( logger, *this << ": l2Norm()" )

    if( mNumValues == 0 )
    {
        return 0.0f;
    }

	ContextPtr loc = getContextPtr();

    LAMA_INTERFACE_FN_T( dot, loc, BLAS, BLAS1, ValueType )

	ReadAccess<ValueType> data( mValues, loc );

	SCAI_CONTEXT_ACCESS( loc );

	return sqrt(dot( mNumValues, data.get(), 1, data.get(), 1, NULL ));
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType CSRStorage<ValueType>::maxNorm() const
{
    // no more checks needed here

    SCAI_LOG_INFO( logger, *this << ": maxNorm()" )

    if( mNumValues == 0 )
    {
        return 0.0f;
    }

    ContextPtr loc = getContextPtr();

    LAMA_INTERFACE_FN_DEFAULT_T( absMaxVal, loc, Utils, Reductions, ValueType )

    ReadAccess<ValueType> csrValues( mValues, loc );

    SCAI_CONTEXT_ACCESS( loc )

    ValueType maxval = absMaxVal( csrValues.get(), mNumValues );

    return maxval;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType CSRStorage<ValueType>::maxDiffNorm( const MatrixStorage<ValueType>& other ) const
{
    SCAI_REGION( "Storage.CSR.maxDiffNorm" )

    SCAI_ASSERT_EQUAL_ERROR( mNumRows, other.getNumRows() )
    SCAI_ASSERT_EQUAL_ERROR( mNumColumns, other.getNumColumns() )

    SCAI_LOG_INFO( logger, *this << ": maxDiffNorm( " << other << " )" )

    common::shared_ptr<CSRStorage<ValueType> > tmpOtherCSR;

    const CSRStorage<ValueType>* otherCSR;

    if( other.getValueType() == this->getValueType() && ( other.getFormat() == Format::CSR ) )
    {
        otherCSR = dynamic_cast<const CSRStorage<ValueType>*>( &other );
        SCAI_ASSERT_ERROR( otherCSR, other << ": could not cast to " << typeName() )
    }
    else
    {
        LAMA_UNSUPPORTED( other << ": converted to " << typeName() << " for maxDiffNorm" )
        tmpOtherCSR.reset( new CSRStorage<ValueType>( other ) );
        otherCSR = tmpOtherCSR.get();
    }

    return maxDiffNormImpl( *otherCSR );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
ValueType CSRStorage<ValueType>::maxDiffNormImpl( const CSRStorage<ValueType>& other ) const
{
    // no more checks needed here

    SCAI_LOG_INFO( logger, *this << ": maxDiffNormImpl( " << other << " )" )

    if( mNumRows == 0 )
    {
        return 0.0f;
    }

    ContextPtr loc = getContextPtr();

    bool sorted = mSortedRows && other.mSortedRows && ( mDiagonalProperty == other.mDiagonalProperty );

    LAMA_INTERFACE_FN_DEFAULT_T( absMaxDiffVal, loc, CSRUtils, Reductions, ValueType )

    ReadAccess<IndexType> csrIA1( mIa, loc );
    ReadAccess<IndexType> csrJA1( mJa, loc );
    ReadAccess<ValueType> csrValues1( mValues, loc );

    ReadAccess<IndexType> csrIA2( other.mIa, loc );
    ReadAccess<IndexType> csrJA2( other.mJa, loc );
    ReadAccess<ValueType> csrValues2( other.mValues, loc );

    SCAI_CONTEXT_ACCESS( loc )

    ValueType maxval = absMaxDiffVal( mNumRows, sorted, csrIA1.get(), csrJA1.get(), csrValues1.get(), csrIA2.get(),
                                      csrJA2.get(), csrValues2.get() );

    return maxval;
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
CSRStorage<ValueType>* CSRStorage<ValueType>::clone() const
{
    return new CSRStorage<ValueType>();
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
CSRStorage<ValueType>* CSRStorage<ValueType>::copy() const
{
    return new CSRStorage<ValueType>( *this );
}

template<typename ValueType>
void CSRStorage<ValueType>::buildSparseRowSizes( LAMAArray<IndexType>& rowSizes ) const
{
    SCAI_LOG_DEBUG( logger, "copy nnz for each row in LAMAArray" );

    WriteOnlyAccess<IndexType> writeRowSizes( rowSizes, mNumRows );
    ReadAccess<IndexType> csrIA( mIa );

    OpenMPCSRUtils::offsets2sizes( writeRowSizes.get(), csrIA.get(), mNumRows );
}

/* --------------------------------------------------------------------------- */

template<typename ValueType>
void CSRStorage<ValueType>::buildSparseRowData(
    LAMAArray<IndexType>& sparseJA,
    LAMAArray<ValueType>& sparseValues ) const
{
    SCAI_LOG_INFO( logger, *this << ": build sparse row data" );

    // for CSR format we can just copy arrays with column indexes and data values

    sparseJA = mJa;
    sparseValues = mValues;
}

/* ========================================================================= */
/*       Template Instantiations                                             */
/* ========================================================================= */

#define LAMA_CSR_STORAGE_INSTANTIATE(z, I, _)                                  \
    template<>                                                                 \
    const char* CSRStorage<ARITHMETIC_TYPE##I>::typeName()                     \
    {                                                                          \
        return "CSRStorage<ARITHMETIC_TYPE##I>";                               \
    }                                                                          \
                                                                               \
    template class COMMON_DLL_IMPORTEXPORT CSRStorage<ARITHMETIC_TYPE##I> ;    \
                                                                               \
    template void CSRStorage<ARITHMETIC_TYPE##I>::setCSRDataSwap(              \
            const IndexType numRows,                                           \
            const IndexType numColumns,                                        \
            const IndexType numValues,                                         \
            LAMAArray<IndexType>& ia,                                          \
            LAMAArray<IndexType>& ja,                                          \
            LAMAArray<ARITHMETIC_TYPE##I>& values,                             \
            const ContextPtr loc );                                            \

    BOOST_PP_REPEAT( ARITHMETIC_TYPE_CNT, LAMA_CSR_STORAGE_INSTANTIATE, _ )

#undef LAMA_CSR_STORAGE_INSTANTIATE

} /* end namespace lama */

} /* end namespace scai */