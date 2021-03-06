###
 # @file CMakeLists.txt
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
 # @brief CMake configuration file for scai_utilskernel tests
 # @author Thomas Brandes
 # @date 18.07.2017
###


set ( TEST_SOURCES 
          UtilsKernelTest
          HArrayUtilsTest
          SectionTest
          UtilsTest
          TransferUtilsTest
          FFTTest
    )

scai_project ( 

    CLASSES

    HEADERS
        
        HArrays

    SOURCES

        ${TEST_SOURCES}
)

scai_test ( EXECUTABLE utilskernelTest 
            FILES      ${CXX_SOURCES} 
            UNIT_TEST
            RUN 
) 

###  copy xmltest.sh script

scai_test_scripts( SCRIPTS       xmltest.sh )

###  configure code_coverage.sh (if USE_CODE_COVERAGE ) in build direcotry

scai_test_code_coverage( ${USE_CODE_COVERAGE} )

