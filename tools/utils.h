#ifndef matrix_utils
#define matrix_utils

#include <sys/types.h>

#include <string>

//-------------------------------------------------------------------------------------------

double flops( size_t n, size_t k, size_t m) { return 2.0 * (double)m * (double)k * (double)n; }

//-------------------------------------------------------------------------------------------

double rand01() // returns double between 0.0 and 1.0
{
    return (double)rand() / (double)RAND_MAX;
}

//-------------------------------------------------------------------------------------------

template < typename M >
void init_rnd( M& m )
{
    for( size_t i = 0; i < m.rows(); ++i )
        for( size_t j = 0; j < m.cols(); ++j )
            m(i,j) = rand01();
}

//-------------------------------------------------------------------------------------------

template < typename M >
void init_ij( M& m )
{
    for( size_t i = 0; i < m.rows(); ++i )
        for( size_t j = 0; j < m.cols(); ++j )
            m(i,j) = 1 + i + j;
}

//-------------------------------------------------------------------------------------------

template < typename MC, typename MA, typename MB >
void dgemm( MC& C, const MA& A, const MB& B,
                   const size_t n, const size_t k, const size_t m )
{
    for(size_t i=0; i<n; ++i)
        for(size_t j=0; j<m; ++j)
            for(size_t l=0; l<k; ++l)
                C(i,j) +=  A(i,l) * B(l,j);
}

//-------------------------------------------------------------------------------------------

int verify(const std::string& s1, const std::string& s2 )
{
    if( s1 == s2 )
    {
        std::cout << "OK" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "FAIL" << std::endl;
        return -1;
    }
}

//-------------------------------------------------------------------------------------------

#endif // kronos_eigen_utils
