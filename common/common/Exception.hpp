/**
 * @file Exception.hpp
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
 * @brief Interface of the class Exception.
 * @author Thomas Brandes
 * @date 11.06.2015
 * @since 1.0.0
 */

#pragma once

// for dll_import
#include <common/config.hpp>

#include <exception>
#include <string>
#include <sstream>

/**
 * @brief The namespace common holds common stuff useful for different C++ projects
 */
namespace common
{

/**
 * @brief The class Exception represents a general exception in common library.
 */
class COMMON_DLL_IMPORTEXPORT Exception: public std::exception
{
public:

    /**
     * @brief The default constructor creates an Exception with no message.
     */
    Exception()
    {
    }

    /**
     * @brief This constructor creates an Exception with the passed message.
     *
     * @param[in] message  the message to assign to this.
     */
    Exception( const std::string& message ) : mMessage( message )
    {
    }

    /**
     * @brief The destructor destroys this Exception.
     */
    virtual ~Exception() throw ()
    {
    }

    /**
     * @brief what() returns the message of this Exception.
     *
     * @return the message of this Exception.
     */
    virtual const char* what() const throw ()
    {
        return mMessage.c_str();
    }

protected:

    std::string mMessage;
};

}  // namespace common

#define COMMON_THROWEXCEPTION( msg )                                               \
    {                                                                              \
        std::ostringstream errorStr;                                               \
        errorStr<<"Exception in line "<<__LINE__<<" of file "<<__FILE__<<"\n";     \
        errorStr<<"    Message: "<<msg<<"\n";                                      \
        throw common::Exception( errorStr.str() );                                 \
    }