/**
 * @file AMGSetupManager.hpp
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
 * @brief AMGSetupManager.hpp
 * @author Thomas Brandes
 * @date 15.03.2013
 * @since 1.0.0
 */

#pragma once

// for dll_import
#include <scai/common/config.hpp>

// base classes
#include <scai/common/NonCopyable.hpp>

// others
#include <scai/lama/solver/AMGSetup.hpp>

// logging
#include <scai/logging.hpp>

namespace scai
{

namespace lama
{

/** @brief Base class for a AMG setup manager.
 *
 *  For each AMG setup type a AMG setup manager must be available.
 *  The AMG setup manager registers in the AMGSetupFactory and
 *  delivers a shared pointer for a AMG setup.
 *
 */

class AMGSetupManager: private common::NonCopyable
{
public:

    virtual ~AMGSetupManager();

    /** @brief Method that returns a AMGSetup.  */

    virtual AMGSetupPtr getAMGSetup() = 0;

protected:

    /** Constructor of a AMGSetupManager, type must be specified. */

    AMGSetupManager( const char* type );

    std::string mAMGSetupType; //!< type of AMGSetup managed

    SCAI_LOG_DECL_STATIC_LOGGER( logger )
};

} /* end namespace lama */

} /* end namespace scai */