#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#include <cstdio>
#include <iostream>
#include <sstream>

#include <Eigen/Dense>

#include "utils.h"

//-------------------------------------------------------------------------------------------

std::string sep = "\n----------------------------------------\n";

Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
Eigen::IOFormat InitFmt(4, 0, ", ", ",\n", "", "");

using namespace Eigen;

//-----------------------------------------------------------------------------------------

#define COLS 6
#define ROWS 6

#define DATA "m.dat"

//-----------------------------------------------------------------------------------------

std::string construct()
{
    std::ostringstream os;

    const size_t rows = ROWS;
    const size_t cols = COLS;

    const size_t size = sizeof(double)*rows*cols;

    double* mem = new double[ rows*cols ];

    Map<MatrixXd> A (mem,rows,cols);

    init_rnd(A);

    // dump matrix to file

    int fd = 0;
    if( (fd = ::open(DATA,O_RDWR|O_CREAT,0777)) < 0 )
        ::close(fd), ::perror("Error opening file"), ::exit(EXIT_FAILURE);

    if( ::write(fd, mem, size ) == -1 )
        ::close(fd), ::perror("Error writing to file"), ::exit(EXIT_FAILURE);

    if( ::close(fd) < 0 )
        ::perror("Error closing to file"), ::exit(EXIT_FAILURE);

    // output matrix

    os << sep << A.format(CleanFmt) << sep << std::endl;

    // delete matrix

    delete[](mem);

    return os.str();
}

//-----------------------------------------------------------------------------------------

std::string reconstruct()
{
    std::ostringstream os;

    const size_t rows = ROWS;
    const size_t cols = COLS;

    const size_t size = sizeof(double)*rows*cols;

    // open file and mmap region

    int fd = 0;
    if( (fd = ::open(DATA,O_RDONLY)) < 0 )
        ::close(fd), ::perror("Error opening file"), ::exit(EXIT_FAILURE);

    double* map = 0;
    if( (map = (double*) ::mmap(0, size, PROT_READ, MAP_SHARED, fd, 0 )) ==  MAP_FAILED)
        ::close(fd), ::perror("Error mmapping the file"), ::exit(EXIT_FAILURE);

    if( ::close(fd) < 0 )
        ::perror("Error closing to file"), ::exit(EXIT_FAILURE);

    // bind to a matrix

    Map<MatrixXd> A(map,rows,cols);

    // output matrix

    os << sep << A.format(CleanFmt) << sep << std::endl;

    // unmap memory

    if( ::munmap(map, size) == -1 )
      ::perror("Error un-mmapping the file"), ::exit(EXIT_FAILURE);

    return os.str();
}

int main()
{
    ::unlink( DATA );

    std::string s1 = construct();
    std::string s2 = reconstruct();

    return verify( s1, s2 );
}
