/**
 * @file stencilDist.cpp
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
 * @brief Demo of distributed stencil
 * @author Thomas Brandes
 * @date 27.04.2017
 */

#include <scai/lama.hpp>

#include "Stencil.hpp"
#include "StencilStorage.hpp"
#include "StencilMatrix.hpp"

// Matrix & vector related includes
#include <scai/lama/DenseVector.hpp>
#include <scai/lama/SparseVector.hpp>
#include <scai/lama/expression/all.hpp>
#include <scai/lama/matrix/CSRSparseMatrix.hpp>
#include <scai/lama/matrix/DenseMatrix.hpp>
#include <scai/lama/storage/CSRStorage.hpp>
#include <scai/dmemo/BlockDistribution.hpp>
#include <scai/dmemo/GridDistribution.hpp>
#include <scai/dmemo/NoCommunicator.hpp>

// import common 
#include <scai/common/Walltime.hpp>
#include <scai/common/Settings.hpp>

#include <iostream>
#include <stdlib.h>

using namespace scai;
using namespace hmemo;
using namespace lama;
using namespace dmemo;


int main( int argc, const char* argv[] )
{
    // relevant SCAI arguments: 
    //   SCAI_CONTEXT = ...    set default context
    //   SCAI_DEVICE  = ...    set default device
    //   SCAI_NP      = ...    set the default processor grid for grid distribution

    common::Settings::parseArgs( argc, argv );

    Stencil1D<double> stencilFD8;

    stencilFD8.reserve( 8 );   // just for convenience, not mandatory

    stencilFD8.addPoint( -3, -5.0/7168.0 );
    stencilFD8.addPoint( -2, 49.0/5120.0 );
    stencilFD8.addPoint( -1, -245.0/3072.0 );
    stencilFD8.addPoint( 0, 1225.0/1024.0 );
    stencilFD8.addPoint( 1, -1225.0/1024.0 );
    stencilFD8.addPoint( 2, 245.0/3072.0 ) ;
    stencilFD8.addPoint( 3, -49.0/5120.0 );
    stencilFD8.addPoint( 4, 5.0/7168.0 );

    Stencil1D<double> stencilDummy( 1 );

    Stencil2D<double> stencilX( stencilFD8, stencilDummy );

    Stencil3D<double> stencil( 7 );

    const IndexType N = 4; 

    common::Grid3D grid( N, N, N );

    CommunicatorPtr comm = Communicator::getCommunicatorPtr(); 

    dmemo::DistributionPtr gridDistribution( new GridDistribution( grid, comm ) );

    StencilMatrix<double> distStencilMatrix( gridDistribution, stencil );
    CSRSparseMatrix<double> distCSRMatrix( distStencilMatrix );

    std::cout << "distributed stencilMatrix " << distStencilMatrix << std::endl;

    StencilMatrix<double> repStencilMatrix( grid, stencil );

    std::cout << "replicated stencilMatrix " << repStencilMatrix << std::endl;

    DenseVector<double> repX;
    repX.setRandom( repStencilMatrix.getColDistributionPtr(), 1.0f );

    DenseVector<double> distX( repX, distStencilMatrix.getColDistributionPtr() );

    std::cout << "max diff X = " << repX.maxDiffNorm( distX ) << std::endl;

    // replicated and distributed matrix-vector multiplication

    DenseVector<double> repY( repStencilMatrix * repX );
    DenseVector<double> distY( distStencilMatrix * distX );

    std::cout << "max diff Y = " << repY.maxDiffNorm( distY ) << std::endl;
}