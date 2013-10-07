/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendrePolynomialsCollection_H 
#define LegendrePolynomialsCollection_H

#include "ThreadSafeMap.h"
#include "ref_counted_ptr.h"
#include "LegendrePolynomials.h"
#include <string>

class Grid;

#define DEFAULT_MAX_COLLECTION_SIZE 12 

class LegendrePolynomialsCollection
{
public:
    
    LegendrePolynomialsCollection(unsigned int maxCollectionSize = DEFAULT_MAX_COLLECTION_SIZE);
    virtual ~LegendrePolynomialsCollection();

    ref_counted_ptr<const LegendrePolynomials> polynomials(int truncation, const Grid& grid) const;
    
    ref_counted_ptr<const LegendrePolynomials> addPolynomials(int truncation, const Grid& grid, LegendrePolynomials* newItem);

private:

    std::string generateKey(int truncation, const Grid& grid) const;

    ThreadSafeMap<LegendrePolynomials> collection_;
    unsigned int maxCollectionSize_;

};


#endif // LegendrePolynomialsCollection_H
