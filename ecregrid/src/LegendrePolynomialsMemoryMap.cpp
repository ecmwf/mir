/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendrePolynomialsMemoryMap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#include <sstream>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>


LegendrePolynomialsMemoryMap::LegendrePolynomialsMemoryMap(int truncation, const Grid& grid) :
    LegendrePolynomialsRead(truncation,grid) {
//	checkAndPossiblyCreate(grid);

    openCoefficientsFile();

    auto_ptr<Grid>globalGrid( grid.getGlobalGrid() );
    int nshalf = (1 + globalGrid->northSouthNumberOfPoints())/2;

    length_ = latLength_ * nshalf * sizeof(double);

    addr_ = mmap(0, length_, PROT_READ, MAP_SHARED, fd_, 0);

    if(addr_ == MAP_FAILED)
        throw ReadError("LegendrePolynomialsMemoryMap::LegendrePolynomialsMemoryMap" + constructCoefficientsFilename());

    next_ = (double*)addr_;


    if(DEBUG)
        cout << "LegendrePolynomialsMemoryMap::LegendrePolynomialsMemoryMap ||||||||" << latLength_ * nshalf << " " << length_ <<  endl;

    closeFile();
}


LegendrePolynomialsMemoryMap::~LegendrePolynomialsMemoryMap() {
// Unmap here...

    if (DEBUG)
        cout << "LegendrePolynomialsMemoryMap::~LegendrePolynomialsMemoryMap destructor called" << endl;

    munmap(addr_,length_);

    closeFile();

}

const double* LegendrePolynomialsMemoryMap::getOneLatitude(double lat, int rowOffset) const {
    return 0;
}


void LegendrePolynomialsMemoryMap::print(ostream& out) const {
    LegendrePolynomialsRead::print(out);
    out << "Memory Map";
}
