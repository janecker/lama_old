/**
 * @file Config.hpp
 *
 * @brief Structure that contains configuration for label propagation
 * @author Thomas Brandes
 * @date 07.06.2013
 * @since 1.0.1
 */

#include <scai/lama.hpp>

#include <scai/memory/Context.hpp>
#include <scai/common/Printable.hpp>
#include <scai/lama/matrix/Matrix.hpp>
#include <scai/lama/Communicator.hpp>

#include <cstring>

/** Parameters to define LAMA behavior */

class Config : public Printable
{

public:

    Config()
    {
        // overlap communication with local computation

        mCommunicationKind = scai::lama::Matrix::SYNCHRONOUS;
        mComm              = scai::lama::Communicator::get();
        mContext           = memory::Context::getContextPtr( memory::context::Host );
        mMaxIters          = 1000;
    }

    ~Config()
    {
        // give up ownership for communicator and context

        mComm.reset();
        mContext.reset();
    }

    void setArg( const char* arg )
    {
        std::string val = arg;

        // make upper string for more convenience, e.g. Host is same as host or HOST

        for ( std::string::iterator p = val.begin(); val.end() != p; ++p )
        {
            *p = toupper( *p );
        }

        // make it upper case

        if (   ( "CSR" == val ) || ( "Dense" == val ) )
        { 
            mMatrixFormat = val;
        }
        else if ( "HOST" == val )
        { 
            mContext = memory::Context::getContextPtr( memory::context::Host );
        }
        else if ( ( "CUDA" == val ) || ( "GPU" == val ) )
        { 
            // int device = mComm->getNodeRank();
            int device = 0;
            mContext = memory::Context::getContextPtr( memory::context::CUDA, device );
        }
        else if ( "SYNC" == val )
        {
            mCommunicationKind = scai::lama::Matrix::SYNCHRONOUS;
        }
        else if ( "ASYNC" == val )
        {
            mCommunicationKind = scai::lama::Matrix::ASYNCHRONOUS;
        }
        else if ( isNumber( val.c_str() ) )
        {
            sscanf( val.c_str(), "%d", &mMaxIters );
        }
        else
        {
            std::cout << "Illegal argument: " << arg << std::endl;
        }
    }

    const char* getFormat( ) const
    {
        if ( mMatrixFormat == "" )
        {
            // choose default format by context: Host -> CSR, CUDA -> ELL

            if ( mContext->getType() == memory::context::CUDA )
            {
                return "ELL";
            }
            else
            {
                return "CSR";
            }
        }
        else
        {
            return mMatrixFormat.c_str();
        }
    }

    memory::ContextPtr getContextPtr() const
    {
        return mContext;
    }

    const memory::Context& getContext() const
    {
        return *mContext;
    }

    scai::lama::CommunicatorPtr getCommunicatorPtr() const
    {
        return mComm;
    }

    const scai::lama::Communicator& getCommunicator() const
    {
        return *mComm;
    }

    std::string            mMatrixFormat;
    memory::ContextPtr     mContext;
    scai::lama::Matrix::SyncKind mCommunicationKind;
    int                    mMaxIters;

    void writeAt( std::ostream& stream ) const
    {
        stream << "LAMA configuration" << std::endl;
        stream << "==================" << std::endl;
        stream << "Context       = " << *mContext << std::endl;
        stream << "Communicator  = " << *mComm << std::endl;
        stream << "Matrix format = " << getFormat() << std::endl;
        stream << "CommKind      = " << mCommunicationKind << std::endl;
    }

private:

    scai::lama::CommunicatorPtr  mComm;

    inline bool isNumber( const char* arg )
    {
        int len = strlen( arg );
 
        for ( int i = 0; i < len; ++i )
        {
            if ( !isdigit( arg[i] ) )
            {
                return false;
            }
        }

        return true;
    }
};
