#include <vector>
#include <algorithm> 
#include <iostream>

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include "utils.h"

//-------------------------------------------------------------------------------------------

std::string sep = "\n----------------------------------------\n";

Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
Eigen::IOFormat InitFmt(4, 0, ", ", ",\n", "", "");

using namespace Eigen;

//-----------------------------------------------------------------------------------------

template<typename Scalar, typename Index = size_t >
class Tri
{
public:
  Tri() : m_row(0), m_col(0), m_value(0) {}

  Tri(const Index& i, const Index& j, const Scalar& v = Scalar(0))
    : m_row(i), m_col(j), m_value(v)
  {}

  /** \returns the row index of the element */
  const Index& row() const { return m_row; }
  void row( const Index& ) const { return m_row; }

  /** \returns the column index of the element */
  const Index& col() const { return m_col; }

  /** \returns the value of the element */
  const Scalar& value() const { return m_value; }

  Index m_row, m_col;
  Scalar m_value;

};

//-----------------------------------------------------------------------------------------

typedef std::vector< Tri<double> > Coef_t;

//-----------------------------------------------------------------------------------------

int main()
{
    std::ostringstream osi;
    std::ostringstream osf;

    const size_t n = 10;
    const size_t k = 10;
    const size_t m = 20;

    // build coefficients of sparse matrix

    Coef_t coefs;
    coefs.resize( 3*n );
    size_t i = 0;
    Coef_t::iterator it = coefs.begin();
    for( ; it != coefs.end(); ++it, ++i )
    {
        const size_t r = std::min( i / 3 , n-1 );
        const size_t c = std::min( i / 3 + i % 3 , k-1 );
        it->m_row   = r ;
        it->m_col   = c;
        it->m_value = i+1;
    }

//    for( size_t i = 0; i < coefs.size(); ++i )
//        std::cout << "row " << coefs[i].row()  << " col " << coefs[i].col() << " value " << coefs[i].value() << std::endl;

    // assemble sparse matrix

    SparseMatrix<double> A(n,k);
    A.setFromTriplets(coefs.begin(),coefs.end());

//    std::cout << sep << A << sep << std::endl;

    // make dense matrix

    MatrixXd B( k, m);

    init_ij(B);

    // compute result

    MatrixXd C( n, m);

    C = A * B;

    osi << C << std::endl;

//    std::cout << sep << C.format(InitFmt) << sep << std::endl;

    // verification

    MatrixXd Cr(n,m);

    Cr <<   14,   20,   26,   32,   38,   44,   50,   56,   62,   68,   74,   80,   86,   92,   98,  104,  110,  116,  122,  128,
            47,   62,   77,   92,  107,  122,  137,  152,  167,  182,  197,  212,  227,  242,  257,  272,  287,  302,  317,  332,
            98,  122,  146,  170,  194,  218,  242,  266,  290,  314,  338,  362,  386,  410,  434,  458,  482,  506,  530,  554,
           167,  200,  233,  266,  299,  332,  365,  398,  431,  464,  497,  530,  563,  596,  629,  662,  695,  728,  761,  794,
           254,  296,  338,  380,  422,  464,  506,  548,  590,  632,  674,  716,  758,  800,  842,  884,  926,  968, 1010, 1052,
           359,  410,  461,  512,  563,  614,  665,  716,  767,  818,  869,  920,  971, 1022, 1073, 1124, 1175, 1226, 1277, 1328,
           482,  542,  602,  662,  722,  782,  842,  902,  962, 1022, 1082, 1142, 1202, 1262, 1322, 1382, 1442, 1502, 1562, 1622,
           623,  692,  761,  830,  899,  968, 1037, 1106, 1175, 1244, 1313, 1382, 1451, 1520, 1589, 1658, 1727, 1796, 1865, 1934,
           755,  833,  911,  989, 1067, 1145, 1223, 1301, 1379, 1457, 1535, 1613, 1691, 1769, 1847, 1925, 2003, 2081, 2159, 2237,
           870,  957, 1044, 1131, 1218, 1305, 1392, 1479, 1566, 1653, 1740, 1827, 1914, 2001, 2088, 2175, 2262, 2349, 2436, 2523;

    osf << Cr << std::endl;

    return verify( osi.str(),osf.str() );
}
