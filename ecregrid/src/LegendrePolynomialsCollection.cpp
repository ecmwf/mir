#include "LegendrePolynomialsCollection.h"
#include "LegendrePolynomials.h"
#include "Grid.h"
#include <sstream>

using namespace std;

LegendrePolynomialsCollection::LegendrePolynomialsCollection(unsigned int maxCollectionSize /*=DEFAULT_MAX_COLLECTION_SIZE*/)
: maxCollectionSize_(maxCollectionSize)
{

}

LegendrePolynomialsCollection::~LegendrePolynomialsCollection()
{

}

ref_counted_ptr<const LegendrePolynomials> LegendrePolynomialsCollection::polynomials(int truncation, const Grid& grid) const
{
    return collection_.getItem(generateKey(truncation, grid));
}

ref_counted_ptr<const LegendrePolynomials> LegendrePolynomialsCollection::addPolynomials(int truncation, const Grid& grid, LegendrePolynomials* newItem)
{
    return collection_.addItem(generateKey(truncation, grid), newItem);    
}

string LegendrePolynomialsCollection::generateKey(int truncation, const Grid& grid) const
{
    stringstream ss;
    ss << "Truncation=" << truncation << "&Grid=" << grid.coeffInfo();
    return ss.str();
}

