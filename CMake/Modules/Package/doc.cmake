###
 # @file doc.cmake
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
 # @brief everthing for building sphinx and doxygen documentation
 # @author Lauretta Schubert
 # @date 12.02.2016
 # @since 2.0.0
###

include ( Package/Sphinx )
include ( Package/Doxygen)

if    ( SPHINX_FOUND OR DOXYGEN_FOUND )
	set ( DOC_FOUND TRUE )
else  ( SPHINX_FOUND OR DOXYGEN_FOUND )
	set ( DOC_FOUND FALSE )
endif ( SPHINX_FOUND OR DOXYGEN_FOUND )

set ( BUILD_DOC ${DOC_FOUND} CACHE BOOL "Enable / Disable building of doc" )

if    ( BUILD_DOC AND NOT DOC_FOUND )
    message( FATAL_ERROR "Build of documentation enabled, but configuration is incomplete!")
endif ( BUILD_DOC AND NOT DOC_FOUND )