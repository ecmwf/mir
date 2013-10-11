#include <vector>
#include <algorithm> 
#include <iostream>

#include "utils.h"

#include <Tpetra_DefaultPlatform.hpp>
#include <Teuchos_DefaultSerialComm.hpp>
#include <Tpetra_Version.hpp>
#include <Teuchos_oblackholestream.hpp>

//-----------------------------------------------------------------------------------------

void
exampleRoutine (const Teuchos::RCP<const Teuchos::Comm<int> >& comm,
                std::ostream& out)
{
  // Print out the Tpetra software version information.
  out << Tpetra::version() << std::endl << std::endl;
}

//-----------------------------------------------------------------------------------------

int
main (int argc, char *argv[])
{
  using std::endl;
  using Teuchos::Comm;
  using Teuchos::SerialComm;
  using Teuchos::RCP;
  using Teuchos::rcp;

  // Make a "serial" (non-MPI) communicator.
  // It doesn't actually "communicate," because it only has one process.
  RCP<const Comm<int> > comm = rcp (new SerialComm<int>);

  // With a "serial" communicator, the rank is always 0,
  // and the number of processes is always 1.
  const int myRank = comm->getRank();
  const int numProcs = comm->getSize();

  Teuchos::oblackholestream blackHole;
  std::ostream& out = (myRank == 0) ? std::cout : blackHole;

  // We have a communicator and an output stream.
  // Let's do something with them!
  exampleRoutine (comm, out);

  return 0;
}
