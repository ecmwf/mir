/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "VerticalTransformer.h"

VerticalTransformer::VerticalTransformer(const string& typeOfLevel, int level) :
	typeOfLevel_(typeOfLevel), level_(level)
{
}

VerticalTransformer::~VerticalTransformer()
{
}
