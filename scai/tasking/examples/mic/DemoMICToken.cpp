/**
 *  @brief Example of using a MIC sync token
 */

#include <scai/logging.hpp>
#include <scai/tasking/mic/MICSyncToken.hpp>
#include <scai/common/Walltime.hpp>

#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace scai;
using namespace scai::common;
using namespace scai::tasking;

const int NSIZE = 700;

__declspec( target(mic, host) )
extern void compute( size_t NSIZE );

void start_mic_compute( MICSyncToken& token )
{
    int signal_value = 1;

    int device = token.getDevice();

    int n = NSIZE;

    #pragma offload target( mic : device ), in( n ), signal(signal_value)
    {
        compute( n );
    }

    token.setSignal( signal_value );
}

int main()
{
    double t;

    int device = 0;
    int n = NSIZE;

    // measure coprocessor

    t = Walltime::get();
    #pragma offload target(mic : device)
    {
        compute( n );
    }
    t = Walltime::get() - t;

    std::cout << "MIC run (1st): " << t << " seconds" << std::endl;

    // measure coprocessor again, no init device

    t = Walltime::get();
    #pragma offload target(mic : device)
    {
        compute( n );
    }
    t = Walltime::get() - t;

    std::cout << "MIC run (2nd): " << t << " seconds" << std::endl;


    t = Walltime::get();
    compute( n );
    t = Walltime::get() - t;

    std::cout << "Host activity = " << t << " seconds" << std::endl;

    // LAMA approach using SyncToken

    t = Walltime::get();

    {
        MICSyncToken token( device );
        start_mic_compute( token );
        compute( n );
        // implicit wait at end of this scope
    }

    t = Walltime::get() - t;

    std::cout << "Host + Device activity = " << t << " seconds" << std::endl;
}