/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "AsciiOutput.h"

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

#ifndef Input_H
#include "Input.h"
#endif

#ifndef AsciiInput_H
#include "AsciiInput.h"
#endif

#include <fstream>

AsciiOutput::AsciiOutput(const string& name):
	Output(name)
{
}

AsciiOutput::AsciiOutput():
	Output()
{
}

AsciiOutput::~AsciiOutput()
{
}

void AsciiOutput::write(const Field& f) const
{
	f.dump2file(fileName_);
}

void AsciiOutput::write(FILE* out, const vector<double>& values) const
{
	for(unsigned long i = 0; i < values.size(); i++){
		if(SMALL_NUMBERS)
			fprintf(out,"%8.15f \n",values[i]);
		else
			fprintf(out,"%f \n",values[i]);
	}
}

void AsciiOutput::write(FILE* out, const Field& f) const
{
	f.dump2file(fileName_);
}
