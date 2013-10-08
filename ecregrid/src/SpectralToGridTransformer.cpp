/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "SpectralToGridTransformer.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef LegendrePolynomials_H
#include "LegendrePolynomials.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef LegendreTransformSpectralToCoefficients_H
#include "LegendreTransformSpectralToCoefficients.h"
#endif

#ifndef InverseFastFourierTransform_H
#include "InverseFastFourierTransform.h"
#endif

#ifndef SpectralToSpectralTransformer_H
#include "SpectralToSpectralTransformer.h"
#endif

#ifndef Wind_H
#include "Wind.h"
#endif

#ifndef Timer_H
#include "Timer.h"
#endif

#include "Extraction.h"
#include <map>

class Prioritize {
	public:
		int operator() ( const pair<int, int>& p1,
				const pair<int, int>& p2 ) {
			return p1.first < p2.first;
		}
};

typedef map<int,int,less<int> > mapBlox;

SpectralToGridTransformer::SpectralToGridTransformer(const string& legendrePolynomialsMethod, int fftMax, bool auresol, bool conversion) :
    vdConversion_(conversion), auresol_(auresol), legendrePolynomialsMethod_(legendrePolynomialsMethod), fftMaxBlockSize_(fftMax)
{
}

SpectralToGridTransformer::~SpectralToGridTransformer()
{
}

Field* SpectralToGridTransformer::transform(const Field& in, const Field& out) const
{
	const SpectralField& input = dynamic_cast<const SpectralField&>(in);
	const GridField&    output = dynamic_cast<const GridField&>(out);

	int truncation = input.truncation();

	Grid* possibleGrid;
//	if(output.grid().isGlobal()){
//		possibleGrid = output.grid().getGlobalGrid();
//	}
//	else{
		auto_ptr<Grid>globalGrid( output.grid().getGlobalGrid() );
		possibleGrid =  output.grid().newGrid(*globalGrid);
//	}

    double missingValue = output.missingValue();

    Factory factory;
    auto_ptr<Extraction> extraction(factory.multiExtraction(output, missingValue));

	long calculatedLengthPossible = possibleGrid->calculatedNumberOfPoints();
	if(DEBUG)
		cout << "SpectralToGridTransformer::transform  output values length: " << calculatedLengthPossible <<  endl; 
    
    // CACHING OPPORTUNITY:
    vector<double> values(calculatedLengthPossible);

// Auresol
	if(auresol_){
		int truncationA = output.grid().truncate(truncation);
		if(DEBUG)
			cout << "SpectralToGridTransformer::transform AURESOL ON: " << truncationA << endl; 
		if(truncationA != truncation){
			truncation = truncationA;
		}
	}
	SpectralToSpectralTransformer sp2sp;
	vector<comp> dataComplex;
	sp2sp.transform(input,truncation,dataComplex);
	transform(truncation,input.wind(),dataComplex,*possibleGrid,*globalGrid,values);

    // check for frame extraction here
	bool bitmap = false;
    if(extraction.get()){
        bitmap = true;
        extraction->extract(*possibleGrid, values);
    }

	return new GridField( possibleGrid, input,output.bitsPerValue(),output.editionNumber(), output.scanningMode(), bitmap, values, missingValue);
}


void SpectralToGridTransformer::transform(int truncation, bool  windParameter, const vector<comp>& dataComplex, const Grid& output, const Grid& globalGrid, vector<double>& values) const
{
    // we assume that values has been allocated before it reaches us
    const size_t valuesLength = values.size();

	Factory factory;
	string legendreMethod = legendrePolynomialsMethod_;
    char* method = getenv("ECREGRID_LEGENDRE_POLYNOMIALS_METHOD");
    if(method){
		legendreMethod = string(method);
    }

	ref_counted_ptr<const LegendrePolynomials> legendrePolynomialsMethod = factory.polynomialsMethod(legendreMethod,truncation,output);

	if(DEBUG){
		cout << "LEGENDRE POLYNOMIALS METHOD: " << legendrePolynomialsMethod_ << endl; 
		cout << *legendrePolynomialsMethod << endl; 
	}

	double north = output.north();
	double south = output.south();
	if(DEBUG)
		cout << "SpectralToGridTransformer::transform  possibleGrid north: " << north << " south: " << south <<  endl; 

//	Northern hemisphere only
	double virtualNorth = north;
	double virtualSouth = south;

//  Southern hemisphere only
	if(north < 0) {
		virtualNorth = fabs(south);	
		virtualSouth = fabs(north);	
	}

	if(north > 0 && south < 0) {
		virtualSouth = 0;
		if(fabs(south) > north)
			virtualNorth = fabs(south);
	}

	if(DEBUG){
		cout << "SpectralToGridTransformer::transform  possibleGrid virtualNorth: " << virtualNorth << endl; 
		cout << "SpectralToGridTransformer::transform  possibleGrid virtualSouth: " << virtualSouth << endl; 
	}

	int virtualNorthIndex = globalGrid.northIndex(virtualNorth);
//	int virtualSouthIndex = globalGrid.northIndex(virtualSouth);

// ssp last
	int virtualSouthIndex = globalGrid.southIndex(virtualSouth);

	if(DEBUG)
		cout << "SpectralToGridTransformer::transform  possibleGrid virtualNorthIndex: " << virtualNorthIndex << " virtualSouthIndex: " << virtualSouthIndex <<  endl; 

	double west = output.west();
	double east = output.east();
	if(DEBUG)
		cout << "SpectralToGridTransformer::transform  possibleGrid west: " << west << " east: " << east <<  endl; 

//	int northIndex = globalGrid.northIndex(north);

	vector<double> globalLatitudes;
	globalGrid.latitudes(globalLatitudes);

	vector<long> gridDef;
	/*size_t latitudeNumberFromDef = */ output.getGridDefinition(gridDef);

	bool reduced        = output.reduced();

	long lonNumber = 0;
	mapBlox blox;
	if(reduced){
		long num = gridDef[virtualNorthIndex];
		int nn    = virtualNorthIndex;
		int ss    = virtualSouthIndex;
		for ( int j = virtualNorthIndex ; j <= virtualSouthIndex ;  j++ ){
			if(gridDef[j] != num){
				int ssin = j - 1;
				if(DEBUG)
					cout << "SpectralToGridTransformer::transform num: " << num << " nn: " << nn  <<  "  ss: " << ssin << endl; 
				blox.insert( pair<int,int> (nn,ssin) );
				num   = gridDef[j];
				nn    = j;
			}
		}
		blox.insert( pair<int,int> (nn,ss) );
	}
	else {
		blox.insert( pair<int,int> (virtualNorthIndex,virtualSouthIndex) );
		lonNumber = globalGrid.westEastNumberOfPoints();
	}

	
	LegendreTransformSpectralToCoefficients legendre(legendrePolynomialsMethod,truncation,globalLatitudes,windParameter,north,south);
	mapBlox::const_iterator iter = blox.begin(), stop = blox.end();
	unsigned long count = 0;
#if ECREGRID_TIMER
	Timer("SpectralToGridTransformer");
#endif
	for ( ; iter != stop; ++iter) {
		// Legendre
		mapLats legendreCoeff;
//		cout << "SpectralToGridTransformer::transform blox   first: " << iter->first << " second: " << iter->second << endl;
		int lot = legendre.transform(legendreCoeff,dataComplex,iter->first,iter->second);
		// FFT
		if(reduced)
			lonNumber = gridDef[iter->first];

			if(DEBUG)
				cout << "SpectralToGridTransformer::transform  lot: " << lot << " truncation " << truncation << " lonNumber " << lonNumber << " west " << west << " east " << east << endl; 
		InverseFastFourierTransform fourier(truncation, lonNumber, lot, west, east, windParameter);
		count += fourier.transform(values, legendreCoeff, output);
	}
	if(DEBUG)
		cout << "SpectralToGridTransformer::transform  NPTS from FFT: " << count << " calculated NPTS: " << valuesLength << endl; 

	ASSERT(count == valuesLength);

}

Wind* SpectralToGridTransformer::transformVector(const Field& inU, const Field& inV, const Field& req) const
{
	if(DEBUG)
		cout << "((((( SpectralToGridTransformer::transformVector ((((( U " << inU.number() << " V " << inV.number() << endl ;

	const SpectralField& inputU = dynamic_cast<const SpectralField&>(inU);
	const SpectralField& inputV = dynamic_cast<const SpectralField&>(inV);
    const GridField&     output = dynamic_cast<const GridField&>(req);

    int truncation = inputU.truncation();

    auto_ptr<Grid>globalGrid( output.grid().getGlobalGrid() );
    Grid* possibleGrid =  output.grid().newGrid(*globalGrid);
    Grid* possibleGridV =  output.grid().newGrid(*globalGrid);

    long calculatedLengthPossible = possibleGrid->calculatedNumberOfPoints();
    if(DEBUG)
        cout << "SpectralToGridTransformer::transform  output values length: " << calculatedLengthPossible <<  endl;
    
    // CACHING OPPORTUNITY:
    vector<double> valuesU(calculatedLengthPossible);
    vector<double> valuesV(calculatedLengthPossible);

	int truncationA = output.grid().truncate(truncation);
	SpectralToSpectralTransformer sp2sp;

	if(inU.number() == 138 && inV.number() == 155 && vdConversion_) {
		if(DEBUG){
			cout << "SpectralToGridTransformer::transformVector VORT,DIV TO U,V" << endl ;
		}
		int truncation = inputU.truncation();
		vector<comp> vort;
		vector<comp> dive;
		if(auresol_){
			if(DEBUG)
				cout << "SpectralToGridTransformer::transformVector AURESOL ON: " << truncationA << endl ;
			sp2sp.transform(inputU, truncationA - 1, vort);
			sp2sp.transform(inputV, truncationA - 1, dive);
			truncation = truncationA;
		}
		else{
			sp2sp.transform(inputU, truncation - 1, vort);
			sp2sp.transform(inputV, truncation - 1, dive);
		}
		vector<comp> ucomp;
        vector<comp> vcomp;
        sp2sp.vorticityDivergenceToUV(vort,dive,truncation,ucomp,vcomp);
		transform(truncation,inputU.wind(),ucomp,*possibleGrid,*globalGrid,valuesU);
		transform(truncation,inputV.wind(),vcomp,*possibleGrid,*globalGrid,valuesV);

	}
	else{
// Input is spectral u,v
		vector<comp> ucomp;
		vector<comp> vcomp;
		if(auresol_){
			sp2sp.transform(inputU, truncationA, ucomp);
			sp2sp.transform(inputV, truncationA, vcomp);
			truncation = truncationA;
		}
		else{
			sp2sp.transform(inputU, truncation, ucomp);
			sp2sp.transform(inputV, truncation, vcomp);
			ucomp.reserve(inputU.dataLengthComplex());
			vcomp.reserve(inputV.dataLengthComplex());
		}
		transform(truncation,inputU.wind(),ucomp,*possibleGrid,*globalGrid,valuesU);
		transform(truncation,inputV.wind(),vcomp,*possibleGrid,*globalGrid,valuesV);
	}

	GridField* uOut = new GridField(Parameter(131,128,inputU.levelType()), possibleGrid, inputU,output.bitsPerValue(),output.editionNumber(), output.scanningMode(),false, valuesU,output.missingValue());

	GridField* vOut = new GridField(Parameter(132,128,inputV.levelType()), possibleGridV, inputV,output.bitsPerValue(),output.editionNumber(), output.scanningMode(),false, valuesV,output.missingValue());

	return new Wind(uOut,vOut);

	throw UserError("SpectralToGridTransformer::transformVector vector treatment applied just for vort,div ");

	return 0;
}

void SpectralToGridTransformer::print(ostream& out) const
{
    out << "SpectralToGridTransformer{ Legendre Polynomials Method=[" << legendrePolynomialsMethod_ << "], fft Max Block Size[" << fftMaxBlockSize_ << "] }";
}
