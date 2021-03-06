/**
 * @file GMRESTest.cpp
 *
 * @license
 * Copyright (c) 2009-2018
 * Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 * for Fraunhofer-Gesellschaft
 *
 * This file is part of the SCAI framework LAMA.
 *
 * LAMA is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * LAMA is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with LAMA. If not, see <http://www.gnu.org/licenses/>.
 * @endlicense
 *
 * @brief Specific tests for the solver class GMRES.
 * @author Malte Förster
 * @date 10.04.2012
 */

#include <boost/test/unit_test.hpp>

#include <scai/solver/test/TestMacros.hpp>

#include <scai/solver/GMRES.hpp>
#include <scai/solver/TrivialPreconditioner.hpp>
#include <scai/solver/criteria/IterationCount.hpp>
#include <scai/solver/logger/CommonLogger.hpp>

using namespace scai::solver;

// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( GMRESTest )

SCAI_LOG_DEF_LOGGER( logger, "Test.GMRESTest" )

// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE_TEMPLATE( ConstructorTest, ValueType, scai_numeric_test_types )
{
    LoggerPtr slogger( new CommonLogger( "<GMRES>: ", LogLevel::noLogging, LoggerWriteBehaviour::toConsoleOnly ) );
    GMRES<ValueType> cgSolver( "GMRESTestSolver", slogger );
    BOOST_CHECK_EQUAL( cgSolver.getId(), "GMRESTestSolver" );
    GMRES<ValueType> cgSolver2( "GMRESTestSolver2" );
    BOOST_CHECK_EQUAL( cgSolver2.getId(), "GMRESTestSolver2" );
    GMRES<ValueType> cgSolver3( cgSolver2 );
    BOOST_CHECK_EQUAL( cgSolver3.getId(), "GMRESTestSolver2" );
    BOOST_CHECK( cgSolver3.getPreconditioner() == 0 );
    GMRES<ValueType> cgSolver4( "cgSolver4" );
    SolverPtr<ValueType> preconditioner( new TrivialPreconditioner<ValueType>( "Trivial preconditioner" ) );
    cgSolver4.setPreconditioner( preconditioner );
    CriterionPtr<ValueType> criterion( new IterationCount<ValueType>( 10 ) );
    cgSolver4.setStoppingCriterion( criterion );
    GMRES<ValueType> cgSolver5( cgSolver4 );
    BOOST_CHECK_EQUAL( cgSolver5.getId(), cgSolver4.getId() );
    BOOST_CHECK_EQUAL( cgSolver5.getPreconditioner()->getId(), cgSolver4.getPreconditioner()->getId() );
}

// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END();
