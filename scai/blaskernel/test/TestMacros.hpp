/**
 * @file scai/blaskernel/test/TestMacros.hpp
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
 * @brief Additional Macros used for testing of blaskernel with Boost Test.
 * @author Eric Schricker
 * @date 20.11.2015
 * @since 2.0.0
 */

#pragma once

#include <scai/common/test/TestMacros.hpp>
#include <scai/hmemo/test/TestMacros.hpp>

#include <scai/hmemo.hpp>

template<typename ValueType>
void initArray( scai::hmemo::HArray<ValueType>& dst, const ValueType src[], const IndexType size)
{
	scai::hmemo::ContextPtr loc = scai::hmemo::Context::getHostPtr();

	scai::hmemo::WriteAccess<ValueType> wDst( dst );

	for( IndexType i = 0; i < size; ++i)
	{
		wDst[i] = src[i];
	}
}