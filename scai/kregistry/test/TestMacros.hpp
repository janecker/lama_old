/**
 * @file scai/lama/test/TestMacros.hpp
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
 * @brief Additional Macros used for testing of LAMA with Boost Test.
 * @author Jiri Kraus
 * @date 06.04.2011
 * @since 1.0.0
 */

#pragma once

#include <scai/common/test/TestMacros.hpp>
#include <scai/hmemo/test/TestMacros.hpp>

#include <scai/kregistry/exception/KernelRegistryException.hpp>

/*
 * Redefinition of SCAI_CHECK_CLOSE
 * works even if ValueType is unknown
 */

#define LAMA_RUN_TESTL(z, I, method )                                                                  			\
    try                                                                                                			\
    {                                                                                                  			\
        method<ARITHMETIC_HOST_TYPE_##I>( context, logger );                                           			\
    }                                                                                                  			\
    catch ( scai::kregistry::KernelRegistryException& )                                                			\
    {                                                                                                  			\
        SCAI_LOG_WARN( logger, #method << "<" << PRINT_STRING( ARITHMETIC_HOST_TYPE_##I ) << "> cannot run on " \
                       << context->getType() << ", corresponding function not implemented yet." );     			\
        return;                                                                                        			\
    }
