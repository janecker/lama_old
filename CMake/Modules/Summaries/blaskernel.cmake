###
 # @file Summaries/blaskernel.cmake
 #
 # @license
 # Copyright (c) 2009-2013
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
 # @brief SCAI blaskernel Configuration Summary
 # @author Eric Schricker
 # @date 11.01.2016
 # @since 2.0.0
###

include ( Functions/scaiStatusMessage )
include ( Functions/scaiSummaryMessage )

message ( STATUS "" )
message ( STATUS "Summary of SCAI blaskernel Configuration:" )
message ( STATUS "=====================================" )
message ( STATUS "" )

scai_status_message ( HEADLINE "Compiler:" )
# C++ Compiler
scai_summary_message ( "FOUND"
                       "CMAKE_CXX_COMPILER"
                       "C++ Compiler"
                       "${CMAKE_CXX_COMPILER_ID} ${${CMAKE_CXX_COMPILER_ID}CXX_COMPILER_VERSION}" )

message ( STATUS "" )

if    ( CXX_SUPPORTS_C11 OR BOOST_INCLUDE_DIR )
    set( REQUIRED_FOUND TRUE )
else  ( CXX_SUPPORTS_C11 OR BOOST_INCLUDE_DIR )
	set( REQUIRED_FOUND FALSE )
endif ( CXX_SUPPORTS_C11 OR BOOST_INCLUDE_DIR )

scai_summary_message ( "STATIC"
                       "REQUIRED_FOUND"
                       "blaskernel"
                       "Needs compiler supporting C++11 or Boost" )

scai_summary_message ( "FOUND"
					             "CXX_SUPPORTS_C11"
					             "C++11 support"
					             "" )
				
if    ( NOT CXX_SUPPORTS_C11 )
scai_summary_message ( "FOUND"
                       "BOOST_INCLUDE_DIR"
                       "Boost"
                       "Version ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}, add include dir ${BOOST_INCLUDE_DIR} to compile your sources" )
endif ( NOT CXX_SUPPORTS_C11 )

# LAMA (core)
message ( STATUS "" )
scai_status_message ( HEADLINE "LIBRARIES:" )

if    ( SCAI_BLAS_FOUND )
    set( REQUIRED_FOUND TRUE )
    if ( SCAI_BLAS_NAME MATCHES "BLAS" AND NOT LAPACK_FOUND )
        set( REQUIRED_FOUND FALSE )
    endif ( SCAI_BLAS_NAME MATCHES "BLAS" AND NOT LAPACK_FOUND )
else  ( SCAI_BLAS_FOUND )
    set( REQUIRED_FOUND FALSE )
endif ( SCAI_BLAS_FOUND )

scai_summary_message ( "STATIC"
                       "REQUIRED_FOUND"
                       "BlasKernel (core)"
                       "" )

   # BLAS
    scai_summary_message ( "FOUND"
                           "SCAI_BLAS_FOUND"
                           "BLAS"
                           "(${SCAI_BLAS_NAME}) with libraries: ${SCAI_SCAI_BLAS_LIBRARIES}" )
    if    ( SCAI_BLAS_NAME MATCHES "BLAS" )
      message ( STATUS "" )
      scai_summary_message ( "FOUND"
                             "LAPACK_FOUND"
                             "LAPACK"
                             "" )
    endif ( SCAI_BLAS_NAME MATCHES "BLAS" )

# LAMA CUDA
message ( STATUS "" )
scai_summary_message ( "USE"
                       "USE_CUDA"
                       "CUDA"
                       "" )

    # CUDA
    scai_summary_message ( "FOUND"
                           "CUDA_FOUND"
                           "CUDA"
                           "${CUDA_VERSION} at ${CUDA_INCLUDE_DIRS}" )
                           
    # CUDA Compute Capability
    scai_summary_message ( "FOUND"
                           "CUDA_HAVE_GPU"
                           "Compute Capability"
                           "${CUDA_COMPUTE_CAPABILITY}" )
                           
# LAMA MIC
message ( STATUS "" )
scai_summary_message ( "USE"
                       "USE_MIC"
                       "MIC"
                       "" )   

message ( STATUS "" )
scai_summary_message ( "FOUND"
                       "SCAI_COMMON_FOUND"
                       "SCAI Common"
                       "" )
                       
scai_summary_message ( "FOUND"
                       "SCAI_LOGGING_FOUND"
                       "SCAI Logging"
                       "" )
                       
scai_summary_message ( "FOUND"
                       "SCAI_TRACING_FOUND"
                       "SCAI Tracing"
                       "" )
                       
scai_summary_message ( "FOUND"
                       "SCAI_TASKING_FOUND"
                       "SCAI Tasking"
                       "" )
                       
scai_summary_message ( "FOUND"
                       "SCAI_KREGISTRY_FOUND"
                       "SCAI Kregistry"
                       "" )

# LAMA TEST
message ( STATUS "" )
scai_status_message ( HEADLINE "TESTING:" )

scai_summary_message ( "USE"
                       "BUILD_TEST"
                       "TEST"
                       "" )

    # Boost Test-Framework
    scai_summary_message ( "FOUND"
                           "Boost_UNIT_TEST_FRAMEWORK_FOUND"
                           "Boost Unit Test"
                           "" )
                           
    # Boost Regex
    scai_summary_message ( "FOUND"
                           "Boost_REGEX_FOUND"
                           "Boost Regex"
                           "" )  
                       
message ( STATUS "" )

scai_status_message ( HEADLINE "INFO:" )

message ( STATUS "LAMA Version : ${LAMA_VERSION} ${LAMA_VERSION_NAME}" )
message ( STATUS "Build Type   : ${CMAKE_BUILD_TYPE}" )
message ( STATUS "Library Type : ${SCAI_LIBRARY_TYPE}" )
message ( STATUS "ASSERT Level : ${SCAI_ASSERT_LEVEL} ( -DSCAI_ASSERT_LEVEL_${SCAI_ASSERT_LEVEL} )" )
message ( STATUS "LOG Level    : ${SCAI_LOGGING_LEVEL} ( -D${SCAI_LOGGING_FLAG} )" )
message ( STATUS "TRACING      : ${SCAI_TRACING} ( -D${SCAI_TRACING_FLAG} )" )
if    ( USE_CODE_COVERAGE )
	message ( STATUS "CODE COVERAGE: ${USE_CODE_COVERAGE}" )
endif ( USE_CODE_COVERAGE )
message ( STATUS "" )

if    ( USE_CUDA AND NOT CUDA_FOUND )
    message( FATAL_ERROR "Build of LAMA Cuda enabled, but configuration is incomplete!")
endif ( USE_CUDA AND NOT CUDA_FOUND )