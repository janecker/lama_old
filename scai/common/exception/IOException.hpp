/**
 * @file IOException.hpp
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
 * @brief Definition of exception class that is thrown when an assertion fails.
 * @author Thomas Brandes
 * @date 11.06.2015
 */
#pragma once

// base class
#include <scai/common/exception/Exception.hpp>

// std
#include <string>

namespace scai
{

namespace common
{

/** Derived exception class for an exception that is thrown if any assertion fails. */

class COMMON_DLL_IMPORTEXPORT IOException : public Exception
{
public:
    /**
     * @brief The default constructor creates an IOException with no message.
     */
    IOException( );

    /**
     * @brief This constructor creates an IOException with the passed message.
     *
     * @param[in] message  the message to assign to this.
     */
    IOException( const std::string& message );

    /**
     * @brief The destructor destroys this IOException.
     */
    virtual ~IOException() throw ();

    /**
     * @brief Override virtual routine with class specific method.
     */
    virtual const char* what() const throw();

protected:

    std::string mMessage;
};

} /* end namespace common */

} /* end namespace scai */
