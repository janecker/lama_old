set ( REQUIRED_FOUND FALSE )
if    ( MPI_ENABLED OR GPI_ENABLED )
  set ( REQUIRED_FOUND TRUE )
endif ( MPI_ENABLED OR GPI_ENABLED )

heading3 ( "Distributed" "REQUIRED_FOUND" )
    found_message ( "MPI" "MPI_FOUND" "OPTIONAL" "Version ${MPI_VERSION} at ${SCAI_MPI_INCLUDE_DIR}" )
    found_message ( "GPI" "GPI_FOUND" "OPTIONAL" "at ${SCAI_GPI_INCLUDE_DIR}" )