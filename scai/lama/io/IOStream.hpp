/**
 * @file IOStream.hpp
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
 * @brief Extension of the std::fstream class
 * @author Jan Ecker, Thomas Brandes
 * @date 16.03.2016
 */
#pragma once

// for dll_import
#include <scai/common/config.hpp>

// local library

// scai internal libraries
#include <scai/common/SCAITypes.hpp>
#include <scai/common/ScalarType.hpp>
#include <scai/common/unique_ptr.hpp>
#include <scai/common/TypeTraits.hpp>
#include <scai/common/mepr/TemplateSpecifier.hpp>
#include <scai/common/macros/loop.hpp>
#include <scai/common/exception/IOException.hpp>

#include <scai/hmemo/WriteOnlyAccess.hpp>
#include <scai/hmemo/ReadAccess.hpp>

#include <scai/hmemo.hpp>

#include <scai/utilskernel/LArray.hpp>

#include <fstream>
#include <string>
#include <typeinfo>
#include <iostream>
#include <iomanip>

namespace scai
{

namespace lama
{

/** Extension of class fstream for LAMA IO.
 *
 *  These are the extensions
 *
 *  - binary write/read routines with type conversions for heterogeneous arrays
 *  - keep the file name for error messages
 *  - flag to convert between big/little endian
 */

class COMMON_DLL_IMPORTEXPORT IOStream : public std::fstream
{
public:

    typedef enum
    {
        BIG,             //!< forces the use of big endian for binary data
        LITTLE,          //!< forces the use of little endian for binary data
        MACHINE_ENDIAN   //!< use the same endian as provided on this machie
    } Endian;

    /** Constructor of a new stream for any mode and opens it */

    IOStream( const std::string& filename, std::ios_base::openmode mode, Endian usedEndian = MACHINE_ENDIAN );

    /** Open the stream file */

    void open( const std::string& filename, std::ios_base::openmode mode, Endian usedEndian = MACHINE_ENDIAN );

    /** Write array data binary in the opened file
     * 
     *  @param[in] data is the array, size can be queried
     *  @param[in] type specifies the data type used in the file
     *
     *  - no conversion if type stands for ValueType or if type is INTERNAL
     *  - conversion of complex (ValueType) to real(type) deletes the imaginary part
     */

    template<typename ValueType>
    inline void writeBinary( const hmemo::HArray<ValueType>& data,
                             const common::scalar::ScalarType type );

    template<typename ValueType>
    inline void readBinary( hmemo::HArray<ValueType>& data,
                            const IndexType size,
                            const common::scalar::ScalarType type );

    template<typename ValueType>
    void writeFormatted( const hmemo::HArray<ValueType>& val, int prec );

    template<typename ValueType1, typename ValueType2>
    void writeFormatted(
        const hmemo::HArray<ValueType1>& val1, int prec1,
        const hmemo::HArray<ValueType2>& val2, int prec2 );

    template<typename ValueType1, typename ValueType2, typename ValueType3>
    void writeFormatted(
        const hmemo::HArray<ValueType1>& val1, int prec1,
        const hmemo::HArray<ValueType2>& val2, int prec2,
        const hmemo::HArray<ValueType3>& val3, int prec3 );

    template<typename ValueType>
    void readFormatted( hmemo::HArray<ValueType>& val,
                        const IndexType nlines );

    template<typename ValueType1, typename ValueType2>
    void readFormatted( hmemo::HArray<ValueType1>& val,
                        hmemo::HArray<ValueType2>& val2,
                        const IndexType nlines );

    template<typename ValueType1, typename ValueType2, typename ValueType3>
    void readFormatted( hmemo::HArray<ValueType1>& val,
                        hmemo::HArray<ValueType2>& val2,
                        hmemo::HArray<ValueType3>& val3,
                        const IndexType nlines );

    /** Same as close but gives a warning if not all data has been read */

    void closeCheck();

    /** current file name, file must be open. */

    const std::string& getFileName();
    
private:

    /** Write binary array without conversion */

    template<typename FileType>
    inline void writeBinDirect( const hmemo::HArray<FileType>& data );

    /** Write binary array with conversion 
     *
     * @param data is array to be written, will be converted to FileType before
     * @tparam FileType is type used in file
     * @tparam Dataype is type of the array to be written
     */

    template<typename FileType, typename DataType>
    inline void writeBinConverted( const hmemo::HArray<DataType>& data );

    template<typename FileType>
    inline void readBinDirect( hmemo::HArray<FileType>& data, const IndexType size );

    template<typename FileType, typename DataType>
    inline void readBinConverted(  hmemo::HArray<DataType>& data, const IndexType size );

    Endian mUsedEndian;

    /** Logger for this class */

    SCAI_LOG_DECL_STATIC_LOGGER( logger )

    static Endian mMachineEndian;

    static Endian _determineMachineEndian();

    template<typename ValueType, typename TList> struct Wrapper;

    std::string mFileName;    //!< save the name for error messages

    void endianConvert( char* out, const char* in, const IndexType n, const IndexType size );

}; // class IOStream

/* --------------------------------------------------------------------------------- */

template<typename ValueType>
struct IOStream::Wrapper<ValueType, common::mepr::NullType>
{
    static void writeBinResolved( IOStream&, const hmemo::HArray<ValueType>&, const common::scalar::ScalarType fileType )
    {
        COMMON_THROWEXCEPTION( "unsupported file data type " << fileType )
    }

    static void readBinResolved( IOStream&, hmemo::HArray<ValueType>&, const IndexType, const common::scalar::ScalarType fileType )
    {
        COMMON_THROWEXCEPTION( "unsupported file data type " << fileType )
    }
};

/* --------------------------------------------------------------------------------- */

template<typename ValueType, typename H, typename T>
struct IOStream::Wrapper<ValueType, common::mepr::TypeList<H, T> >
{
    static void writeBinResolved( IOStream& fs, const hmemo::HArray<ValueType>& data, const common::scalar::ScalarType fileType )
    {
        if ( fileType == common::TypeTraits<H>::stype )
        {
            fs.writeBinConverted<H, ValueType>( data );
        }
        else
        {
            Wrapper<ValueType, T>::writeBinResolved( fs, data, fileType );
        }
    }

    static void readBinResolved( IOStream& fs, hmemo::HArray<ValueType>& data, const IndexType size, const common::scalar::ScalarType fileType )
    {
        if ( fileType == common::TypeTraits<H>::stype )
        {
            fs.readBinConverted<H, ValueType>( data, size );
        }
        else
        {
            Wrapper<ValueType, T>::readBinResolved( fs, data, size, fileType );
        }
    }
};

/* --------------------------------------------------------------------------------- */

template<typename ValueType>
inline void IOStream::writeBinary( const hmemo::HArray<ValueType>& data,
                                   const common::scalar::ScalarType type )
{
    if ( type == common::scalar::INTERNAL || common::scalar::INDEX_TYPE == type || type == common::TypeTraits<ValueType>::stype )
    {
        // no type conversion needed

        writeBinDirect<ValueType>( data );
    }
    else
    {
        Wrapper < ValueType, SCAI_TYPELIST( SCAI_ARITHMETIC_ARRAY_HOST ) >::writeBinResolved( *this, data, type );
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType>
inline void IOStream::readBinary( hmemo::HArray<ValueType>& data,
                                  const IndexType size,
                                  const common::scalar::ScalarType type )
{
    if ( common::scalar::INTERNAL == type || common::scalar::INDEX_TYPE == type || type == common::TypeTraits<ValueType>::stype )
    {
        // no type conversion needed

        readBinDirect<ValueType>( data, size );
    }
    else
    {
        // use meta programming to call the template routine belonging to type

        Wrapper < ValueType, SCAI_TYPELIST( SCAI_ARITHMETIC_ARRAY_HOST ) >::readBinResolved( *this, data, size, type );
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType>
inline void IOStream::writeBinDirect( const hmemo::HArray<ValueType>& data )
{
    SCAI_LOG_INFO( logger, "writeBinDirect<" << common::TypeTraits<ValueType>::id() << 
                            ">, size = " << data.size() << " * " << sizeof( ValueType) ) 

    hmemo::ReadAccess<ValueType> dataRead( data );

    if ( mUsedEndian == MACHINE_ENDIAN || ( mUsedEndian == mMachineEndian ) )
    {
        std::fstream::write( reinterpret_cast<const char*>( dataRead.get() ), sizeof( ValueType ) * data.size() );
    }
    else
    {
        scai::common::scoped_array<ValueType> tmp( new ValueType[ data.size() ] );

        if ( scai::common::isComplex( scai::common::TypeTraits<ValueType>::stype ) )
        {
            // consider complex value type as 2 values that must be converted

            endianConvert( reinterpret_cast<char*>( tmp.get() ),
                           reinterpret_cast<const char*>( dataRead.get() ),
                           data.size() * 2,
                           sizeof( ValueType ) / 2 );
        }
        else
        {
            endianConvert( reinterpret_cast<char*>( tmp.get() ),
                           reinterpret_cast<const char*>( dataRead.get() ),
                           data.size(),
                           sizeof( ValueType ) );
        }

        std::fstream::write( reinterpret_cast<const char*>( tmp.get() ), sizeof( ValueType ) * data.size() );
    }
}

/* --------------------------------------------------------------------------------- */

template<typename FileType, typename DataType>
inline void IOStream::writeBinConverted( const hmemo::HArray<DataType>& data )
{
    SCAI_LOG_INFO( logger, "writeBinConverted<" << common::TypeTraits<FileType>::id() << ", " << common::TypeTraits<DataType>::id() << ">" )

    SCAI_ASSERT( is_open(), "IOStream is not opened" );

    if ( typeid( FileType ) == typeid( DataType ) )
    {
        writeBinDirect( data );
    }
    else
    {
        utilskernel::LArray<FileType> buffer( data );
        writeBinDirect( buffer );
    }

    std::fstream::flush();
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType>
inline void IOStream::readBinDirect( hmemo::HArray<ValueType>& data,
                                     const IndexType size )
{
    SCAI_LOG_INFO( logger, "readBinDirect<" << common::TypeTraits<ValueType>::id() << ">, size = " << size )

    size_t ndata = sizeof( ValueType ) * size;

    hmemo::WriteOnlyAccess<ValueType> dataWrite( data, size );

    if ( mUsedEndian == MACHINE_ENDIAN || ( mUsedEndian == mMachineEndian ) )
    {
        std::fstream::read( reinterpret_cast<char*>( dataWrite.get() ), ndata);

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, "binary read: could not read " << ndata << " bytes" )
        }
    }
    else
    {
        scai::common::scoped_array<ValueType> tmp( new ValueType[ data.size() ] );

        std::fstream::read( reinterpret_cast<char*>( tmp.get() ), ndata );

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, "binary read: could not read " << ndata << " bytes" )
        }


        if ( scai::common::isComplex( scai::common::TypeTraits<ValueType>::stype ) )
        {
            // consider complex value type as 2 values that must be converted

            endianConvert( reinterpret_cast<char*>( dataWrite.get() ),
                           reinterpret_cast<const char*>( tmp.get() ),
                           size * 2, sizeof( ValueType ) / 2 );
        }
        else
        {
            endianConvert( reinterpret_cast<char*>( dataWrite.get() ),
                           reinterpret_cast<const char*>( tmp.get() ),
                           size, sizeof( ValueType ) );
        }

    }
}

/* --------------------------------------------------------------------------------- */

template<typename FileType, typename DataType>
inline void IOStream::readBinConverted( hmemo::HArray<DataType>& data,
                                        const IndexType size )
{
    SCAI_LOG_INFO( logger, "read array data <" << common::TypeTraits<FileType>::id() << "> to <"
                   << common::TypeTraits<DataType>::id() << ">, size = " << size )

    if ( typeid( FileType ) == typeid( DataType ) )
    {
        readBinDirect( data, size );
    }
    else
    {
        hmemo::HArray<FileType> buffer;
        readBinDirect( buffer, size );
        utilskernel::LArray<DataType>& lData = reinterpret_cast<utilskernel::LArray<DataType>& >( data );
        lData = buffer;
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType>
void IOStream::writeFormatted( const hmemo::HArray<ValueType>& val, int prec )
{
    int n = val.size();

    SCAI_LOG_INFO( logger, "writeFormatted<" 
                           << common::TypeTraits<ValueType>::id() << ":" << prec 
                           << ">, n = " << n )

    if ( n == 0 )
    {  
        return;
    }

    hmemo::ContextPtr ctx = hmemo::Context::getHostPtr();

    hmemo::ReadAccess<ValueType> rVal( val, ctx );

    for ( IndexType k = 0; k < n; ++k )
    {
        *this << std::setprecision( prec ) << rVal[k] << std::endl;

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, "Error reading file " << mFileName )
        }
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType1, typename ValueType2, typename ValueType3>
void IOStream::writeFormatted( 
    const hmemo::HArray<ValueType1>& val1, int prec1,
    const hmemo::HArray<ValueType2>& val2, int prec2,
    const hmemo::HArray<ValueType3>& val3, int prec3 )
{
    int n = val1.size();

    SCAI_ASSERT_EQUAL( n, val2.size(), "size mismatch" );
    SCAI_ASSERT_EQUAL( n, val3.size(), "size mismatch" );

    SCAI_LOG_INFO( logger, "writeFormatted<" 
                           << common::TypeTraits<ValueType1>::id() << ":" << prec1
                           << ", " << common::TypeTraits<ValueType2>::id() << ":" << prec2
                           << ", " << common::TypeTraits<ValueType3>::id() << ":" << prec3
                           << ">, n = " << n )

    hmemo::ContextPtr ctx = hmemo::Context::getHostPtr();

    // use two times the size if Value type is not complex

    hmemo::ReadAccess<ValueType1> rVal1( val1, ctx );
    hmemo::ReadAccess<ValueType2> rVal2( val2, ctx );
    hmemo::ReadAccess<ValueType3> rVal3( val3, ctx );

    for ( IndexType k = 0; k < n; ++k )
    {
        *this << std::setprecision( prec1 ) << rVal1[k] << ' ';
        *this << std::setprecision( prec2 ) << rVal2[k] << ' ';
        *this << std::setprecision( prec3 ) << rVal3[k] << std::endl;

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, "Error reading file " << mFileName )
        }
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType1, typename ValueType2>
void IOStream::writeFormatted( 
    const hmemo::HArray<ValueType1>& val1, int prec1,
    const hmemo::HArray<ValueType2>& val2, int prec2 )
{
    int n = val1.size();

    SCAI_ASSERT_EQUAL( n, val2.size(), "size mismatch" );

    SCAI_LOG_INFO( logger, "writeFormatted<" 
                           << common::TypeTraits<ValueType1>::id() << ":" << prec1
                           << ", " << common::TypeTraits<ValueType2>::id() << ":" << prec2
                           << ">, n = " << n )

    hmemo::ContextPtr ctx = hmemo::Context::getHostPtr();

    // use two times the size if Value type is not complex

    hmemo::ReadAccess<ValueType1> rVal1( val1, ctx );
    hmemo::ReadAccess<ValueType2> rVal2( val2, ctx );

    for ( IndexType k = 0; k < n; ++k )
    {
        *this << std::setprecision( prec1 ) << rVal1[k] << ' ';
        *this << std::setprecision( prec2 ) << rVal2[k] << std::endl;

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, "Error reading file " << mFileName )
        }
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType>
void IOStream::readFormatted( hmemo::HArray<ValueType>& val,
                              const IndexType nlines )
{
    SCAI_LOG_INFO( logger, "readFormatted<" 
                           << common::TypeTraits<ValueType>::id()
                           << ">, nlines = " << nlines )

    hmemo::ContextPtr ctx = hmemo::Context::getHostPtr();

    hmemo::WriteOnlyAccess<ValueType> wVal( val, ctx, nlines );

    std::string line;

    for ( IndexType k = 0; k < nlines; ++k )
    {
        std::getline( *this, line );

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, 
                                 "readFormatted<" << common::TypeTraits<ValueType>::id() 
                                 << "> from " << mFileName 
                                 << " at entry line " << k << " of " << nlines )
        }

        std::istringstream iss( line );

        iss >> wVal[k];
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType1, typename ValueType2>
void IOStream::readFormatted( hmemo::HArray<ValueType1>& val1,
                              hmemo::HArray<ValueType2>& val2,
                              const IndexType nlines )
{
    SCAI_LOG_INFO( logger, "readFormatted<" 
                           << common::TypeTraits<ValueType1>::id() 
                           << common::TypeTraits<ValueType2>::id() 
                           << ">, nlines = " << nlines )

    hmemo::ContextPtr ctx = hmemo::Context::getHostPtr();

    hmemo::WriteOnlyAccess<ValueType1> wVal1( val1, ctx, nlines );
    hmemo::WriteOnlyAccess<ValueType2> wVal2( val2, ctx, nlines );

    std::string line;

    for ( IndexType k = 0; k < nlines; ++k )
    {
        std::getline( *this, line );

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, 
                                 "readFormatted<" << common::TypeTraits<ValueType1>::id() 
                                 << ", " << common::TypeTraits<ValueType2>::id() 
                                 << "> from " << mFileName 
                                 << " at entry line " << k << " of " << nlines )
        }

        std::istringstream iss( line );

        iss >> wVal1[k];
        iss >> wVal2[k];

        if ( iss.fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, "Error reading file " << mFileName 
                                 << ", entry line " << k << " of " << nlines
                                 << ", line = " << line )
        }
    }
}

/* --------------------------------------------------------------------------------- */

template<typename ValueType1, typename ValueType2, typename ValueType3>
void IOStream::readFormatted( hmemo::HArray<ValueType1>& val1,
                              hmemo::HArray<ValueType2>& val2,
                              hmemo::HArray<ValueType3>& val3,
                              const IndexType nlines )
{
    SCAI_LOG_INFO( logger, "readFormatted<" 
                           << common::TypeTraits<ValueType1>::id() 
                           << common::TypeTraits<ValueType2>::id() 
                           << common::TypeTraits<ValueType3>::id() 
                           << ">, nlines = " << nlines )

    hmemo::ContextPtr ctx = hmemo::Context::getHostPtr();

    hmemo::WriteOnlyAccess<ValueType1> wVal1( val1, ctx, nlines );
    hmemo::WriteOnlyAccess<ValueType2> wVal2( val2, ctx, nlines );
    hmemo::WriteOnlyAccess<ValueType3> wVal3( val3, ctx, nlines );

    std::string line;

    for ( IndexType k = 0; k < nlines; ++k )
    {
        std::getline( *this, line );

        if ( this->fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, 
                                 "readFormatted<" << common::TypeTraits<ValueType1>::id() 
                                 << ", " << common::TypeTraits<ValueType2>::id() 
                                 << ", " << common::TypeTraits<ValueType3>::id() 
                                 << "> from " << mFileName 
                                 << " at entry line " << k << " of " << nlines )
        }

        std::istringstream iss( line );

        iss >> wVal1[k];
        iss >> wVal2[k];
        iss >> wVal3[k];

        if ( iss.fail() )
        {
            SCAI_THROWEXCEPTION( common::IOException, "Error reading file " << mFileName 
                                 << ", entry line " << k << " of " << nlines
                                 << ", line = " << line )
        }
    }
}

/* --------------------------------------------------------------------------------- */

} /* end namespace lama */

} /* end namespace scai */