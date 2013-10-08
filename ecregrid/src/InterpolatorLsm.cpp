/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "InterpolatorLsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef NearestNeigbourLsm_H
#include "NearestNeigbourLsm.h"
#endif

#ifndef Lsm_H
#include "Lsm.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef Timer_H
#include "Timer.h"
#endif

#ifndef DoubleLinear_H
#include "DoubleLinear.h"
#endif

#ifndef BiLinear_H
#include "BiLinear.h"
#endif


InterpolatorLsm::InterpolatorLsm(const Grid& input, const Grid& output, const string& lsmMethod) :
	Interpolator()
{
	lsm(input,output,lsmMethod);
}

InterpolatorLsm::InterpolatorLsm(int n, const Grid& input, const Grid& output, const string& lsmMethod) :
	Interpolator(n)
{
	lsm(input,output,lsmMethod);
}

InterpolatorLsm::InterpolatorLsm(bool w, bool a, double nPole, double sPole, int n, const Grid& input, const Grid& output, const string& lsmMethod) :
	Interpolator(w,a,nPole,sPole,n),emosTrigger_(false)
{
	lsm(input,output,lsmMethod);
}

InterpolatorLsm::~InterpolatorLsm()
{
}

double InterpolatorLsm::useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size) const
{
	ASSERT(size > 0);
    // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
    // them as class members. hence they are guaranteed to exist while this
    // call completes
    return NearestNeigbourLsm::calculateInterpolatedValue(where, nearests, *inLsmData_, *outLsmData_);
}

double InterpolatorLsm::useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size, double missingValue) const
{
	if(size == 0)
		return missingValue;

    // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
    // them as class members. hence they are guaranteed to exist while this
    // call completes
    return NearestNeigbourLsm::calculateInterpolatedValue(where, nearests, *inLsmData_, *outLsmData_);
}

void InterpolatorLsm::lsm(const Grid& input, const Grid& output, const string& lsmMethod)
{
    inLsmData_ = ref_counted_ptr<const vector<double> >(new vector<double>());
    outLsmData_ = ref_counted_ptr<const vector<double> >(new vector<double>());
	Factory factory;

	long inSize      = input.calculatedNumberOfPoints();
	string inLsmFile = input.predefinedLsmFileName();
	long outSize      = output.calculatedNumberOfPoints();
	string outLsmFile = output.predefinedLsmFileName();

	bool userInput = false;
	bool userOutput = false;
	string userInputPath  = "none";
	string userOutputPath = "none";
	char* pathAll = getenv("ECREGRID_LSM_PATH");
	char* pathIn  = getenv("ECREGRID_LSM_INPUT_PATH");
	char* pathOut = getenv("ECREGRID_LSM_OUTPUT_PATH");

	if(pathAll){
		userInputPath = string(pathAll);
		userInput = true;
		userOutputPath = string(pathAll);
		userOutput = true;
	}

	if(pathIn){
		userInputPath = string(pathIn);
		userInput = true;
	}
	if(pathOut){
		userOutputPath = string(pathOut);
		userOutput = true;
	}

	bool rotate = inLsmFilePreserve_ != inLsmFile || outLsmFilePreserve_ != outLsmFile;

	if(inLsmFilePreserve_ != inLsmFile || (inSize > (long)(*inLsmData_).size()) ){
		firstInput_        = false;
		inLsmFilePreserve_ = inLsmFile;
	}


	if(outLsmFilePreserve_ != outLsmFile || (outSize != (long)(*outLsmData_).size()) ){
		firstOutput_       = false;
		outLsmFilePreserve_ = outLsmFile;
	}

	if(!firstInput_){
		auto_ptr<Lsm>inLsm(factory.getLsm(inLsmFile,lsmMethod,input.lsmFileType(), userInput, userInputPath));
        inLsmData_ = inLsm->getLsmValuesDouble(input);
		firstInput_ = true;
	}

	if(output.rotated() || output.gridType() == "list"){
		if(rotate){
			vector<Point> outputPoints;
			output.generateGrid1D(outputPoints);
			size_t outSize = outputPoints.size();	


			if(DEBUG){
				cout << "InterpolatorLsm::lsm Output LSM is interpolated from Input LSM" << endl;
			}
// Emos hirlsm.F
			DoubleLinear bil;
//			BiLinear bil;
            // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
            // them as class members. hence they are guaranteed to exist while this
            // call completes
            vector<double>* newLsmData = new vector<double>(outSize);
			bil.interpolate(input,*inLsmData_,1,MISSING_VALUE,outputPoints, *newLsmData);

            // make a local lsm object 
            // and hold as a class-level member
            // (could potentially be added to the cache)
            outLsmData_ = ref_counted_ptr<const vector<double> >(newLsmData);
		}
		return;
	}

	if(!firstOutput_){
		auto_ptr<Lsm>outLsm(factory.getLsm(outLsmFile,lsmMethod,output.lsmFileType(),userOutput,userOutputPath));
        outLsmData_ = outLsm->getLsmValuesDouble(output);

		firstOutput_ = true;
	}

}


void InterpolatorLsm::print(ostream& out) const
{
	out << "Interpolation method is: " ;
}
