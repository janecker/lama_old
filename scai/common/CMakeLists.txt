###
 # @file common/CMakeLists.txt
 #
 # @license
 # Copyright (c) 2009-2015
 # Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 # for Fraunhofer-Gesellschaft
 #
 # Permission is hereby granted, free of charge, to any person obtaining a copy
 # of this software and associated documentation files (the "Software"), to deal
 # in the Software without restriction, including without limitation the rights
 # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 # copies of the Software, and to permit persons to whom the Software is
 # furnished to do so, subject to the following conditions:
 #
 # The above copyright notice and this permission notice shall be included in
 # all copies or substantial portions of the Software.
 #
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 # SOFTWARE.
 # @endlicense
 #
 # @brief CMake configuration file for common library
 # @author Thomas Brandes
 # @date 09.06.2015
###

### Set required CMake version
cmake_minimum_required ( VERSION 2.8 )

### Set PROJECT NAME / supported LANGUAGES
project ( common CXX C )

### Set Module Path
set ( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../CMake/Modules" )

## Define all search packages
include ( SearchPackages/Common )

# Find required packages
foreach    ( PACKAGE_TO_FIND ${REQUIRED_PACKAGES_TO_FIND} )
    find_package ( ${PACKAGE_TO_FIND} ${LAMA_FIND_PACKAGE_FLAGS} REQUIRED )
endforeach ( PACKAGE_TO_FIND ${REQUIRED_PACKAGES_TO_FIND} )

# Find optional packages
foreach    ( PACKAGE_TO_FIND ${OPTIONAL_PACKAGES_TO_FIND} )
    find_package ( ${PACKAGE_TO_FIND} ${LAMA_FIND_PACKAGE_FLAGS} )
endforeach ( PACKAGE_TO_FIND ${OPTIONAL_PACKAGES_TO_FIND} )

include ( Functions/checkValue )
include ( Functions/setAndCheckCache )

include ( Package/CUDA )

### Include Compiler Flags
include ( SetBuildFlags )
include ( SetCPPFlags )
include ( SetNVCCFlags )

## Project System-Include
include_directories ( ../.. )

## add subdirectories
add_subdirectory ( examples )

## build library
file ( GLOB CPP_SOURCES "*.cpp" )
file ( GLOB HPP_HEADERS "*.hpp" )

if    ( WIN32 )
	add_definitions ( -DCOMMON_COMPILING_DLL )
endif ( WIN32 )

if    ( CUDA_FOUND AND LAMA_USE_CUDA )
    add_subdirectory ( cuda )
    include_directories ( ${CUDA_TOOLKIT_INCLUDE} )
endif ( CUDA_FOUND AND LAMA_USE_CUDA )

include_directories ( . )

### add library and put path to search path ###

add_library ( scai_common SHARED ${CPP_SOURCES} ${CUDA_SOURCES} )
target_link_libraries( scai_common ${PTHREADS_LIBRARY} )

### install ###

install ( TARGETS scai_common DESTINATION lib )
install ( FILES ${HPP_HEADERS} DESTINATION include/scai/common )

## install library.hpp
install ( FILES ../common.hpp DESTINATION include/scai )

## add custum target distClean
include ( DistClean )