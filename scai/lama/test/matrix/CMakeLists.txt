###
 # @file scai/lama/test/matrix/CMakeLists.txt
 #
 # @license
 # Copyright (c) 2009-2018
 # Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 # for Fraunhofer-Gesellschaft
 #
 # This file is part of the SCAI framework LAMA.
 #
 # LAMA is free software: you can redistribute it and/or modify it under the
 # terms of the GNU Lesser General Public License as published by the Free
 # Software Foundation, either version 3 of the License, or (at your option)
 # any later version.
 #
 # LAMA is distributed in the hope that it will be useful, but WITHOUT ANY
 # WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 # FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 # more details.
 #
 # You should have received a copy of the GNU Lesser General Public License
 # along with LAMA. If not, see <http://www.gnu.org/licenses/>.
 # @endlicense
 #
 # @brief Build configuration for test of LAMA matrix classes
 # @author Jan Ecker
 # @date 03.04.2013
###

scai_project ( 

    CLASSES
    
    HEADERS

        Matrices
        
    SOURCES

        AllMatrixTest
        Redistribution
        Creator

        MatrixConstructorTest
        SparseMatrixTest
        StencilMatrixTest
        DenseMatrixTest

        MatrixAssemblyTest

        GramianMatrixTest
        HybridMatrixTest

        lamaMatrixTest
    )

### checks ####
if ( ${SCAI_ASSERT_LEVEL} STREQUAL "OFF" )
   message ( STATUS "WARNING: tests will not check for exceptions (assertions are switched off)" )
endif ( ${SCAI_ASSERT_LEVEL} STREQUAL "OFF" )

### add definitions ###
add_definitions ( -DLAMA_TESTFILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../../res/testfiles" )

### define unit test (executable) ###
scai_test ( EXECUTABLE lamaMatrixTest 
            FILES      ${CXX_SOURCES} 
            RUN UNIT_TEST )
