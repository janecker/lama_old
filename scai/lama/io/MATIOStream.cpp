/**
 * @file lama/io/MATIOStream.cpp
 *
 * @license
 * Copyright (c) 2009-2016
 * Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 * for Fraunhofer-Gesellschaft
 *
 * This file is part of the SCAI framework LAMA.
 *
 * LAMA is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * LAMA is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with LAMA. If not, see <http://www.gnu.org/licenses/>.
 *
 * Other Usage
 * Alternatively, this file may be used in accordance with the terms and
 * conditions contained in a signed written agreement between you and
 * Fraunhofer SCAI. Please contact our distributor via info[at]scapos.com.
 * @endlicense
 *
 * @brief Implementation of methods for MATIOstream
 * @author Thomas Brandes
 * @date 23.11.2016
 */

#include "scai/lama/io/MATIOStream.hpp"

#include <zlib.h>

using namespace std;

namespace scai
{

namespace lama
{

/* --------------------------------------------------------------------------------- */

common::scalar::ScalarType MATIOStream::matlabType2ScalarType( uint32_t dataType )
{
    switch ( dataType )
    {
        case  MAT_INT8   :
            return common::scalar::CHAR;          // INT8 same as CHAR
        case  MAT_UINT8  :
            return common::scalar::UNKNOWN;       // UINT8 not supported yet
        case  MAT_INT16  :
            return common::scalar::UNKNOWN;       // INT16 not supported yet
        case  MAT_UINT16 :
            return common::scalar::UNKNOWN;       // UINT16 not supported yet
        case  MAT_INT32  :
            return common::scalar::INT;           // INT32
        case  MAT_UINT32 :
            return common::scalar::UNSIGNED_INT;  // UINT32 not supported yet
        case  MAT_FLOAT  :
            return common::scalar::FLOAT;         // single precision
        case  MAT_DOUBLE :
            return common::scalar::DOUBLE;        // double precision
        case  MAT_LDOUBLE:
            return common::scalar::LONG_DOUBLE;   // long double precision
        case  MAT_INT64  :
            return common::scalar::LONG;          // INT64
        case  MAT_UINT64 :
            return common::scalar::UNSIGNED_LONG; // UINT64
        default          :
            return common::scalar::UNKNOWN;       //
    }
}

/* --------------------------------------------------------------------------------- */

uint32_t MATIOStream::scalarType2MatlabType( common::scalar::ScalarType stype )
{
    switch ( stype )
    {
        case  common::scalar::CHAR                :
            return MAT_INT8;
        case  common::scalar::INT                 :
            return MAT_INT32;
        case  common::scalar::UNSIGNED_INT        :
            return MAT_UINT32;
        case  common::scalar::LONG                :
            return MAT_INT64;
        case  common::scalar::UNSIGNED_LONG       :
            return MAT_UINT64;
        case  common::scalar::FLOAT               :
            return MAT_FLOAT;
        case  common::scalar::DOUBLE              :
            return MAT_DOUBLE;
        case  common::scalar::LONG_DOUBLE         :
            return MAT_LDOUBLE;
        case  common::scalar::COMPLEX             :
            return MAT_FLOAT;
        case  common::scalar::DOUBLE_COMPLEX      :
            return MAT_DOUBLE;
        case  common::scalar::LONG_DOUBLE_COMPLEX :
            return MAT_LDOUBLE;

        default :
            COMMON_THROWEXCEPTION( "no Matlab type for " << stype )
    }

    return 0;
}

/* --------------------------------------------------------------------------------- */

common::scalar::ScalarType MATIOStream::class2ScalarType( uint32_t dataType )
{
    switch ( dataType )
    {
        case  MAT_INT8_CLASS   :
            return common::scalar::CHAR;          // INT8 same as CHAR
        case  MAT_UINT8_CLASS  :
            return common::scalar::UNKNOWN;       // UINT8 not supported yet
        case  MAT_INT16_CLASS  :
            return common::scalar::UNKNOWN;       // INT16 not supported yet
        case  MAT_UINT16_CLASS :
            return common::scalar::UNKNOWN;       // UINT16 not supported yet
        case  MAT_INT32_CLASS  :
            return common::scalar::INT;           // INT32
        case  MAT_UINT32_CLASS :
            return common::scalar::UNSIGNED_INT;  // UINT32 not supported yet
        case  MAT_FLOAT_CLASS  :
            return common::scalar::FLOAT;         // single precision
        case  MAT_DOUBLE_CLASS :
            return common::scalar::DOUBLE;        // double precision
        case  MAT_INT64_CLASS  :
            return common::scalar::LONG;          // INT64
        case  MAT_UINT64_CLASS :
            return common::scalar::UNSIGNED_LONG; // UINT64
        default                :
            return common::scalar::UNKNOWN;       //
    }
}

/* --------------------------------------------------------------------------------- */

MATIOStream::MATClass MATIOStream::scalarType2Class( common::scalar::ScalarType stype )
{
    switch ( stype )
    {
        case  common::scalar::CHAR                :
            return MAT_INT8_CLASS;
        case  common::scalar::INT                 :
            return MAT_INT32_CLASS;
        case  common::scalar::UNSIGNED_INT        :
            return MAT_UINT32_CLASS;
        case  common::scalar::LONG                :
            return MAT_INT64_CLASS;
        case  common::scalar::UNSIGNED_LONG       :
            return MAT_UINT64_CLASS;
        case  common::scalar::FLOAT               :
            return MAT_FLOAT_CLASS;
        case  common::scalar::DOUBLE              :
            return MAT_DOUBLE_CLASS;
        case  common::scalar::LONG_DOUBLE         :
            return MAT_DOUBLE_CLASS;
        case  common::scalar::COMPLEX             :
            return MAT_FLOAT_CLASS;
        case  common::scalar::DOUBLE_COMPLEX      :
            return MAT_DOUBLE_CLASS;
        case  common::scalar::LONG_DOUBLE_COMPLEX :
            return MAT_DOUBLE_CLASS;

        default :
            COMMON_THROWEXCEPTION( "no Matlab class for " << stype )
    }

    return MAT_CELL_CLASS;  // just dummy
}

/* --------------------------------------------------------------------------------- */

MATIOStream::MATIOStream( const std::string& filename, ios_base::openmode mode ) : IOStream( filename, mode )
{
}

/* --------------------------------------------------------------------------------- */

void MATIOStream::readMATFileHeader( int& version, IOStream::Endian& indicator )
{
    // MATFile header has 128 bytes

    char header[128];

    fstream::read( header, 128 );

    header[116] = '\0';

    const short int* versionPtr   = reinterpret_cast<short int*>( &header[124] );

    // const short int* indicatorPtr = reinterpret_cast<short int*>( &header[126] );

    SCAI_LOG_DEBUG( logger, "Header txt = " << header )

    indicator = IOStream::MACHINE_ENDIAN;
    version = *versionPtr;

    SCAI_LOG_DEBUG( logger, "Version = " << version )
}

/* --------------------------------------------------------------------------------- */

void MATIOStream::writeMATFileHeader()
{
    char header[128];

    memset( header, 0, 128 );

    sprintf( header, "This is the output of LAMA" );

    short int* versionPtr   = reinterpret_cast<short int*>( &header[124] );

    *versionPtr = 256;

    header[126] = 'I';
    header[127] = 'M';

    fstream::write( header, 128 );
}

/* --------------------------------------------------------------------------------- */

const char* MATIOStream::readDataElementHeader( uint32_t& dataType, uint32_t& nBytes, uint32_t& wBytes, const char buffer[] )
{
    const uint32_t* data = reinterpret_cast<const uint32_t*>( buffer );

    bool small = false;

    dataType = data[0];

    if ( ( dataType >> 16 ) != 0 )
    {
        small    = true;
        nBytes   = dataType >> 16;
        dataType = ( dataType << 16 ) >> 16;
        SCAI_LOG_DEBUG( logger, "small element: nBytes = " << nBytes << ", dataType = " << dataType )
    }
    else
    {
        nBytes   = data[1];
    }

    if ( small )
    {
        wBytes = 8;
        return buffer + 4;   // the next 4 bytes are just the data
    }
    else
    {
        wBytes = 8 + nBytes + ( 8 - nBytes ) % 8;
        SCAI_LOG_DEBUG( logger, "data element: nBytes = " << nBytes << " / " << wBytes << ", dataType = " << dataType )
        return buffer + 8;
    }
}

/* --------------------------------------------------------------------------------- */

uint32_t MATIOStream::writeDataElementHeader( const uint32_t dataType, const uint32_t nBytes, bool dryRun )
{
    // Note: currently we do not support the short data element format for writing

    SCAI_LOG_DEBUG( logger, "Write: DataElementHeader, type = " << dataType << " " << matlabType2ScalarType( dataType )
                             << ", #bytes = " << nBytes )

    char buffer[8];

    uint32_t size = sizeof( buffer );

    uint32_t* data = reinterpret_cast<uint32_t*>( buffer );

    data[0] = dataType;
    data[1] = nBytes;

    if ( !dryRun )
    {
        fstream::write( buffer, size );
    }

    return size;
}

/* --------------------------------------------------------------------------------- */

static void uncompress( char out[], int out_len, const char in[], int len, int flush )
{
    z_stream infstream;

    infstream.zalloc = Z_NULL;
    infstream.zfree  = Z_NULL;
    infstream.opaque = Z_NULL;

    int ok = inflateInit( &infstream );

    SCAI_ASSERT_EQ_ERROR( ok, Z_OK, "Unable to inflateInit" )

    // inflate the variable head

    infstream.avail_in = len;
    infstream.next_in = ( unsigned char* ) in;
    infstream.avail_out = out_len;
    infstream.next_out = ( unsigned char* ) out;

    ok = inflate( &infstream, flush );

    if ( flush == Z_NO_FLUSH )
    {
        SCAI_ASSERT_EQ_ERROR( ok, Z_OK, "Unable to inflate" )
    }
    else
    {
        SCAI_ASSERT_EQ_ERROR( ok, Z_STREAM_END, "Unable to inflate" )
    }

    // get the headers

    // inflate(&infstream, Z_FINISH);
    inflateEnd( &infstream );
}

/* --------------------------------------------------------------------------------- */

uint32_t MATIOStream::readDataElement( common::scoped_array<char>& dataElement )
{
    char buffer[8];

    uint32_t dataType;
    uint32_t nBytes;
    uint32_t wBytes;

    fstream::read( buffer, 8 );

    if ( fstream::fail() )
    {
        COMMON_THROWEXCEPTION( "No element found in input file" );
    }

    readDataElementHeader( dataType, nBytes, wBytes, buffer );

    if ( dataType != MAT_COMPRESSED )
    {
        // uncompressed data, copy header and the other data

        SCAI_LOG_DEBUG( logger, "uncompressed elem, width = " << wBytes << ", #bytes = " << nBytes )

        dataElement.reset( new char[ wBytes ] );
        memcpy( dataElement.get(), buffer, 8 );
        fstream::read( dataElement.get() + 8, wBytes - 8 );

        return wBytes;
    }
    else
    {
        // allocate temporary data for the compressed data and read it

        common::scoped_array<char> compressedData( new char[nBytes] );

        fstream::read( compressedData.get(), nBytes );

        // uncompress header info only to find out the size of the uncommpressed data

        uncompress( buffer, 8, compressedData.get(), nBytes, Z_NO_FLUSH );

        uint32_t uncompressedNBytes;
        uint32_t uncompressedWBytes;

        readDataElementHeader( dataType, uncompressedNBytes, uncompressedWBytes, buffer );

        SCAI_LOG_DEBUG( logger, "uncompressed dataType = " << dataType << ", uncompressed nBytes = " << uncompressedNBytes )

        dataElement.reset( new char[uncompressedWBytes] );

        uncompress( dataElement.get(), uncompressedWBytes, compressedData.get(), nBytes, Z_FINISH );

        return uncompressedWBytes;
    }
}

/* --------------------------------------------------------------------------------- */

static inline uint32_t paddingBytes( const uint32_t nBytes )
{
    uint32_t padSize = nBytes % 8;

    if ( padSize )
    {
        padSize = 8 - padSize;
    }

    return padSize;
}

/* --------------------------------------------------------------------------------- */

uint32_t MATIOStream::writePadding( uint32_t size, bool dryRun )
{
    uint32_t padSize = paddingBytes( size );

    static char padData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    if ( !dryRun )
    {
        fstream::write( padData, padSize );
    }

    return padSize;
}

/* --------------------------------------------------------------------------------- */

uint32_t MATIOStream::writeSparseHeader(
    const IndexType m,
    const IndexType n,
    const IndexType nnz,
    const uint32_t nBytes,
    bool isComplex,
    bool dryRun )
{
    uint32_t dataType = MAT_MATRIX;

    uint32_t wBytes = writeDataElementHeader( dataType, nBytes, dryRun );

    uint32_t header[2]   = { 0, static_cast<uint32_t>( nnz ) };
    char*    headerBytes = reinterpret_cast<char*>( header );

    headerBytes[0] = static_cast<char>( MAT_SPARSE_CLASS );              // class
    headerBytes[1] = static_cast<char>( isComplex ? ( 1 << 3 ) : 0 ) ;   // array flags

    wBytes += writeData( header, 2, dryRun );

    int dims[2] = { static_cast<int>( m ), static_cast<int>( n ) };

    wBytes += writeData( dims, 2, dryRun );
    wBytes += writeString( "LAMA", dryRun );

    return wBytes;
}

/* --------------------------------------------------------------------------------- */

uint32_t MATIOStream::writeDenseHeader(
    const IndexType m,
    const IndexType n,
    const uint32_t nBytes,
    common::scalar::ScalarType stype,
    bool dryRun )
{
    uint8_t matClass  = MATIOStream::scalarType2Class( stype );
    bool    isComplex = common::isComplex( stype );

    SCAI_LOG_INFO( logger, "writeDenseHeader, nBytes = " << nBytes << ", complex = " << isComplex )

    uint32_t dataType = MAT_MATRIX;
    uint32_t wBytes   = writeDataElementHeader( dataType, nBytes, dryRun );

    uint32_t header[2];
    char*    headerBytes = reinterpret_cast<char*>( header );

    memset( headerBytes, 0, 8 );

    headerBytes[0] = static_cast<char>( matClass );                      // class
    headerBytes[1] = static_cast<char>( isComplex ? ( 1 << 3 ) : 0 ) ;   // array flags

    SCAI_LOG_INFO( logger, "Array flags, class = " << ( int ) headerBytes[0] 
                            << ", flags = " << ( int ) headerBytes[1] )

    wBytes += writeData( header, 2, dryRun );

    int dims[2] = { static_cast<int>( m ), static_cast<int>( n ) };

    wBytes += writeData( dims, 2, dryRun );
    wBytes += writeString( "LAMA", dryRun );

    return wBytes;
}

/* --------------------------------------------------------------------------------- */

uint32_t MATIOStream::getMatrixInfo( MATClass& matClass, IndexType dims[2], IndexType& nnz, bool& isComplex, const char* data, bool isCell )
{
    uint32_t dataType;
    uint32_t nBytes;
    uint32_t wBytes;

    const char* elementPtr = readDataElementHeader( dataType, nBytes, wBytes, data );

    SCAI_ASSERT_EQ_ERROR( dataType, MATIOStream::MAT_MATRIX, "can only read array as MATRIX - MATLAB array" )

    uint32_t header[2];
    int32_t  mdims[2];
   
    uint8_t* headerFlags = reinterpret_cast<uint8_t*>( header );

    elementPtr += getData( header, 2, elementPtr );
    elementPtr += getData( mdims, 2, elementPtr );

    dims[0] = static_cast<IndexType>( mdims[0] );
    dims[1] = static_cast<IndexType>( mdims[1] );

    matClass  = MATClass( headerFlags[0] );
    isComplex = headerFlags[1] & ( 1 << 3 );

    nnz = header[1];

    char nameData[128];

    if ( isCell )
    {
        SCAI_LOG_INFO( logger, "read info of cell " << dims[0] << " x " << dims[1] 
                                << ", nnz = " << nnz << ", isComplex = " << isComplex << ", class = " << matClass )
    }
    else
    {
        elementPtr += getString( nameData, 128, elementPtr );

        SCAI_LOG_INFO( logger, "read info of matrix " << nameData << ", " << dims[0] << " x " << dims[1] 
                                << ", nnz = " << nnz << ", isComplex = " << isComplex << ", class = " << matClass )
    }

    return elementPtr - data;      // read bytes
}

/* --------------------------------------------------------------------------------- */

}  // namespace lama

}  // namespace scai