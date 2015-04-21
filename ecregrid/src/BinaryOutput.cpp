/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "BinaryOutput.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldIdentity_H
#include "FieldIdentity.h"
#endif

#ifndef FieldFeatures_H
#include "FieldFeatures.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

#ifndef BinaryInput_H
#include "BinaryInput.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#include <fstream>

BinaryOutput::BinaryOutput():
    Output() {
}

BinaryOutput::BinaryOutput(const string& name):
    Output(name) {
}

BinaryOutput::~BinaryOutput() {
}


void BinaryOutput::write(const Field& field) const {
    /* Open output file */
    FILE* out = fopen(fileName_.c_str(),"w");
    if (! out) {
        throw CantOpenFile(fileName_);
    }

    size_t size = (size_t)field.dataLength()*sizeof(double);

    cout << "HandleRequest::writeToFile -- size = "  << size << endl;

    const double* pData = 0;
    if (field.data().size() > 0)
        pData = (const double*)&field.data()[0];

    /* Write the buffer in a file */
    if(fwrite(pData,1,size,out) != size) {
        fclose(out);
        throw WriteError("HandleRequest::writeToFile -- write error");
    }
}

void BinaryOutput::write(FILE* out, const Field& field) const {
    size_t size = (size_t)field.dataLength()*sizeof(double);

    cout << "HandleRequest::writeToFile -- size = "  << size << endl;
    return;

    const double* pData = 0;
    if (field.data().size() > 0)
        pData = (const double*)&field.data()[0];

    /* Write the buffer in a file */
    if(fwrite(pData,1,size,out) != size) {
        fclose(out);
        throw WriteError("HandleRequest::writeToFile -- write error");
    }
}

void BinaryOutput::write(FILE* out, const vector<double>& values) const {
    size_t size = (size_t)values.size()*sizeof(double);
    cout << "HandleRequest::writeToFile -- values Size = "  << values.size() << endl;

    const double* pData = 0;
    if (values.size() > 0)
        pData = (const double*)&values[0];

    /* Write the buffer in a file */
    if(fwrite(pData,1,size,out) != size) {
        fclose(out);
        throw WriteError("HandleRequest::write -- write error");
    }
}

void BinaryOutput::write(const char* buffer,long length) const {
    fstream  file;
    file.open(fileName_.c_str(), ios::out|ios::binary);

    if (!file.is_open())
        throw CantOpenFile(fileName_);

    file.write(buffer, length);
    if (file.fail())
        throw WriteError(fileName_);

    file.close();
}

