/**
 * @file imageProcessing.cpp
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
 * @brief Example program to work on image data
 * @author Thomas Brandes
 * @date 04.05.2017
 */

#include <scai/lama/examples/image/ImageIO.hpp>

#include <scai/lama/examples/stencil/Stencil.hpp>
#include <scai/lama/examples/stencil/StencilMatrix.hpp>

#include <scai/lama/GridVector.hpp>
#include <scai/common/Settings.hpp>

#include <scai/lama.hpp>

// Matrix & vector related includes

using namespace scai;
using namespace lama;

int main( int argc, const char* argv[] )
{
    // relevant SCAI arguments: 
    //   SCAI_CONTEXT = ...    set default context
    //   SCAI_DEVICE  = ...    set default device

    common::Settings::parseArgs( argc, argv );

    // read in the image file, must be a png file

    GridVector<float> image;   // size will be ( width , height, ncolors )

    ImageIO::read( image, "input.bmp" );

    std::cout << "read image as grid vector : " << image << std::endl;

    SCAI_ASSERT_EQ_ERROR( image.nDims(), 3, "no color image data" )

    // apply stencil on the pixels, do not apply on the colors in 3-rd dimension 

    Stencil1D<float> stencil1( 3 );
    Stencil1D<float> stencilDummy( 1 );

    Stencil3D<float> stencil( stencil1, stencil1, stencilDummy );

    StencilMatrix<float> m( image.getDistributionPtr(), stencil );

    // apply the stencil

    GridVector<float> imageNew;

    imageNew = m * image;

    ImageIO::write( image, "output.bmp" );
}