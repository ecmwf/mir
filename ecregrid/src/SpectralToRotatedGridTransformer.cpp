/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "SpectralToRotatedGridTransformer.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef GridToGridTransformer_H
#include "GridToGridTransformer.h"
#endif

#ifndef SpectralToSpectralTransformer_H
#include "SpectralToSpectralTransformer.h"
#endif

#ifndef ReducedGaussian_H
#include "ReducedGaussian.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#ifndef Wind_H
#include "Wind.h"
#endif


SpectralToRotatedGridTransformer::SpectralToRotatedGridTransformer(const string& coeffMethod, int fftMax, bool auresol, bool conversion) :
    SpectralToGridTransformer(coeffMethod,fftMax,auresol,conversion)
{
}

SpectralToRotatedGridTransformer::~SpectralToRotatedGridTransformer()
{
}

Field* SpectralToRotatedGridTransformer::transform(const Field& in, const Field& out) const
{
	const SpectralField& input = dynamic_cast<const SpectralField&>(in);
	const GridField&    output = dynamic_cast<const GridField&>(out);

	int reqGauss = input.matchGaussian();
	if(auresol_)
		reqGauss = output.grid().matchGaussian();
	auto_ptr<ReducedGaussian> bridge( new ReducedGaussian(reqGauss));
	Grid* globalReducedGaussian = bridge->getGlobalGrid();

    long calculatedLength   = globalReducedGaussian->calculatedNumberOfPoints();
    if(DEBUG)
        cout << "SpectralToGridTransformer::transform  output values length: " << calculatedLength <<  endl;
    
    // CACHING OPPORTUNITY:
    vector<double> values(calculatedLength);

	int truncation = input.truncation();
	if(auresol_)
		truncation = globalReducedGaussian->truncate(truncation);

	SpectralToSpectralTransformer sp2sp;
	vector<comp> newComp;
    sp2sp.transform(input,truncation,newComp);

    SpectralToGridTransformer::transform(truncation,input.wind(),newComp,*globalReducedGaussian,*globalReducedGaussian,values);

	bool bitmap = false;
	GridField bridgeField(globalReducedGaussian,input,output.bitsPerValue(),output.editionNumber(), output.scanningMode(),bitmap, values,output.missingValue());

//	cout << "SpectralToRotatedGridTransformer::transform " <<  *reducedGaussian << endl;
//	cout << "SpectralToRotatedGridTransformer::transform " <<  output << endl;
//	reducedGaussian->dump2file("./values");

	GridToGridTransformer gr2gr("cubic");

	return gr2gr.transform(bridgeField,output);
}

Wind* SpectralToRotatedGridTransformer::transformVector(const Field& inU, const Field& inV, const Field& req) const
{
	if(DEBUG)
		cout << "((((( SpectralToRotatedGridTransformer::transformVector ((((( U " << inU.number() << " V " << inV.number() << endl ;

	const SpectralField& inputU = dynamic_cast<const SpectralField&>(inU);
	const SpectralField& inputV = dynamic_cast<const SpectralField&>(inV);
    const GridField&     output = dynamic_cast<const GridField&>(req);
//	cout <<"SpectralToRotatedGridTransformer::transformVector input U " << inputU << endl;

	int reqGauss = inputU.matchGaussian();
	if(auresol_)
		reqGauss  = output.grid().matchGaussian();
    auto_ptr<ReducedGaussian> bridge( new ReducedGaussian(reqGauss));
	Grid* globalReducedGaussian = bridge->getGlobalGrid();
	Grid* globalReducedGaussianV = bridge->getGlobalGrid();

    long calculatedLengthPossible = globalReducedGaussian->calculatedNumberOfPoints();
    if(DEBUG)
        cout << "SpectralToRotatedGridTransformer::transform  output values length: " << calculatedLengthPossible <<  endl;
    
    // CACHING OPPORTUNITY:
    vector<double> valuesU(calculatedLengthPossible);
    vector<double> valuesV(calculatedLengthPossible);

	
	int truncation = inputU.truncation();
	if(auresol_)
		truncation = output.grid().truncate(truncation);
	SpectralToSpectralTransformer sp2sp;

	if(inU.number() == 138 && inV.number() == 155 && vdConversion_) {
		if(DEBUG){
			cout << "SpectralToRotatedGridTransformer::transformVector VORT,DIV TO U,V" << endl ;
		}
		vector<comp> vort;
		vector<comp> dive;
		sp2sp.transform(inputU, truncation - 1, vort);
		sp2sp.transform(inputV, truncation - 1, dive);
		vector<comp> ucomp;
        vector<comp> vcomp;
        sp2sp.vorticityDivergenceToUV(vort,dive,truncation,ucomp,vcomp);
		SpectralToGridTransformer::transform(truncation,inputU.wind(),ucomp,*globalReducedGaussian,*globalReducedGaussian,valuesU);
		SpectralToGridTransformer::transform(truncation,inputV.wind(),vcomp,*globalReducedGaussian,*globalReducedGaussian,valuesV);

	}
	else{
// Input is spectral u,v
		vector<comp> ucomp;
		vector<comp> vcomp;
		sp2sp.transform(inputU, truncation, ucomp);
		sp2sp.transform(inputV, truncation, vcomp);
		SpectralToGridTransformer::transform(truncation,inputU.wind(),ucomp,*globalReducedGaussian,*globalReducedGaussian,valuesU);
		SpectralToGridTransformer::transform(truncation,inputV.wind(),vcomp,*globalReducedGaussian,*globalReducedGaussian,valuesV);
	}

	GridField bridgeU(Parameter(131,128,inputU.levelType()), globalReducedGaussian, inputU,inputU.bitsPerValue(),inputU.editionNumber(), 1,false, valuesU,output.missingValue());
//	cout <<"SpectralToRotatedGridTransformer::transformVector bridge(Reduced Gaussian) U " << bridgeU << endl;
	GridField bridgeV(Parameter(132,128,inputV.levelType()), globalReducedGaussianV, inputV,inputV.bitsPerValue(),inputV.editionNumber(), 1,false, valuesV,output.missingValue());

	GridToGridTransformer gr2gr("cubic");

	return gr2gr.transformVector(bridgeU,bridgeV,output);
}
