/**
 * @file BitmapIO.hpp
 *
 * @license
 * Copyright (c) 2009-2017
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
 * @brief Definition of routines to read/write image data
 * @author Thomas Brandes
 * @date 04.05.2017
 */
#pragma once

// for dll_import
#include <scai/common/config.hpp>

#include <scai/logging.hpp>
#include <scai/lama/io/ImageIO.hpp>

namespace scai
{

namespace lama
{

class COMMON_DLL_IMPORTEXPORT BitmapIO : public ImageIO
{

public:

    /** Implementation of virtual routine ImageIO::read for this format */

    void read( hmemo::_HArray& data, common::Grid& grid, const std::string& outputFileName );

    /** Implementation of virtual routine ImageIO::write for this format */

    void write( const hmemo::_HArray& data, const common::Grid& grid, const std::string& outputFileName );

    /** Typed version of BitmapIO::read */

    template<typename ValueType>
    void readImpl( hmemo::HArray<ValueType>& data, common::Grid& grid, const std::string& outputFileName );

    /** Typed version of BitmapIO::write */

    template<typename ValueType>
    void writeImpl( const hmemo::HArray<ValueType>& data, const common::Grid& grid, const std::string& outputFileName );

protected:

    SCAI_LOG_DECL_STATIC_LOGGER( logger )
};

} /* end namespace lama */

} /* end namespace scai */