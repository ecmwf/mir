/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Factory.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

#ifndef BinaryInput_H
#include "BinaryInput.h"
#endif

#ifndef AsciiInput_H
#include "AsciiInput.h"
#endif

#ifndef GribApiInput_H
#include "GribApiInput.h"
#endif

#ifndef Output_H
#include "Output.h"
#endif

#ifndef BinaryOutput_H
#include "BinaryOutput.h"
#endif

#ifndef AsciiOutput_H
#include "AsciiOutput.h"
#endif

#ifndef GribApiOutputGrid_H
#include "GribApiOutputGrid.h"
#endif

#ifndef GribApiOutputSpectral_H
#include "GribApiOutputSpectral.h"
#endif

#ifndef Lsm_H
#include "Lsm.h"
#endif

#ifndef LsmPreDefined_H
#include "LsmPreDefined.h"
#endif

#ifndef LsmEmos10minute_H
#include "LsmEmos10minute.h"
#endif

#ifndef LsmFromGrid_H
#include "LsmFromGrid.h"
#endif

#ifndef GTopo30_H
#include "GTopo30.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef Transformer_H
#include "Transformer.h"
#endif

#ifndef SpectralToSpectralTransformer_H
#include "SpectralToSpectralTransformer.h"
#endif

#ifndef GridToGridTransformer_H
#include "GridToGridTransformer.h"
#endif
#include "GridToGridMatrixTransformer.h"

#ifndef SpectralToGridTransformer_H
#include "SpectralToGridTransformer.h"
#endif

#ifndef SpectralToListOfPointsTransformer_H
#include "SpectralToListOfPointsTransformer.h"
#endif

#ifndef SpectralToRotatedGridTransformer_H
#include "SpectralToRotatedGridTransformer.h"
#endif

#ifndef GridToSpectralTransformer_H
#include "GridToSpectralTransformer.h"
#endif

#ifndef LegendrePolynomials_H
#include "LegendrePolynomials.h"
#endif

#ifndef LegendrePolynomialsReadFromFile_H
#include "LegendrePolynomialsReadFromFile.h"
#endif

#ifndef LegendrePolynomialsOnFly_H
#include "LegendrePolynomialsOnFly.h"
#endif

#ifndef LegendrePolynomialsMemoryMap_H
#include "LegendrePolynomialsMemoryMap.h"
#endif

#ifndef Frame_H
#include "Frame.h"
#endif

#ifndef Bitmap_H
#include "Bitmap.h"
#endif

#ifndef SubGrid_H
#include "SubGrid.h"
#endif

#ifndef SubArea_H
#include "SubArea.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif

#ifndef Anisotropy_H
#include "Anisotropy.h"
#endif

#ifndef Orientation_H
#include "Orientation.h"
#endif

#ifndef Slope_H
#include "Slope.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef RegularLatLon_H
#include "RegularLatLon.h"
#endif


#ifndef Interpolator_H
#include "Interpolator.h"
#endif

#ifndef BiLinear_H
#include "BiLinear.h"
#endif

#ifndef BiLinearPrecipitation_H
#include "BiLinearPrecipitation.h"
#endif

#ifndef BiLinearLsmInteger_H
#include "BiLinearLsmInteger.h"
#endif

#ifndef BiLinearBitmap_H
#include "BiLinearBitmap.h"
#endif

#ifndef NearestNeigbour_H
#include "NearestNeigbour.h"
#endif

#ifndef Linear_H
#include "Linear.h"
#endif

#ifndef LinearBitmap_H
#include "LinearBitmap.h"
#endif

#ifndef LinearFit3Times_H
#include "LinearFit3Times.h"
#endif

#ifndef LinearFit3TimesBitmap_H
#include "LinearFit3TimesBitmap.h"
#endif

#ifndef Cubic12pts_H
#include "Cubic12pts.h"
#endif

#ifndef Cubic12ptsBitmap_H
#include "Cubic12ptsBitmap.h"
#endif

#ifndef Average_H
#include "Average.h"
#endif


#ifndef AverageWeightedReduced_H
#include "AverageWeightedReduced.h"
#endif

#ifndef AverageWeightedRegular_H
#include "AverageWeightedRegular.h"
#endif

#ifndef AverageWeightedReducedLsm_H
#include "AverageWeightedReducedLsm.h"
#endif

#ifndef AverageWeightedRegularLsm_H
#include "AverageWeightedRegularLsm.h"
#endif


#ifndef InterpolatorLsm_H
#include "InterpolatorLsm.h"
#endif

#ifndef BiLinearLsm_H
#include "BiLinearLsm.h"
#endif

#ifndef BiLinearLsmBitmap_H
#include "BiLinearLsmBitmap.h"
#endif

#ifndef NearestNeigbourLsm_H
#include "NearestNeigbourLsm.h"
#endif

#ifndef LinearFit3TimesLsm_H
#include "LinearFit3TimesLsm.h"
#endif

#ifndef LinearFit3TimesLsmBitmap_H
#include "LinearFit3TimesLsmBitmap.h"
#endif

#ifndef Cubic12ptsLsm_H
#include "Cubic12ptsLsm.h"
#endif

#ifndef Cubic12ptsLsmBitmap_H
#include "Cubic12ptsLsmBitmap.h"
#endif

#ifndef AverageLsm_H
#include "AverageLsm.h"
#endif

#ifndef Conserving_H
#include "Conserving.h"
#endif

#ifndef DoubleLinear_H
#include "DoubleLinear.h"
#endif

#ifndef DoubleLinearBitmap_H
#include "DoubleLinearBitmap.h"
#endif

#ifndef DoubleLinearBitmapAdjusted_H
#include "DoubleLinearBitmapAdjusted.h"
#endif

#ifndef DoubleLinearLsm_H
#include "DoubleLinearLsm.h"
#endif

#ifndef DoubleLinearLsmBitmap_H
#include "DoubleLinearLsmBitmap.h"
#endif

#ifndef FluxConservingReduced_H
#include "FluxConservingReduced.h"
#endif

#ifndef FluxConservingRegular_H
#include "FluxConservingRegular.h"
#endif

#ifndef Extraction_H
#include "Extraction.h"
#endif

#include "LegendrePolynomialsSharedMemory.h"

/*static*/ LegendrePolynomialsCollection Factory::memoryMapCache_;
/*static*/ LegendrePolynomialsCollection Factory::sharedMemoryCache_;

Factory::Factory() {
}

Factory::~Factory() {
}


Interpolator* Factory::interpolationMethod(const string& method, int pointsForInterpolation, const GridField& input, const Grid& gridout, bool lsm, const string& lsmMethod, const string& extrapolate, double missingValue, bool bitmap) const {
    if(DEBUG)
        cout << "Factory::interpolationMethod => " << method << endl;
    if(method != "default") {
        if(method == "bilinear" || method == "BILINEAR") {
            if(extrapolate == "nearest") {
                if(bitmap) {
                    if(lsm)
                        return new BiLinearLsmBitmap(input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return new BiLinearBitmap(missingValue);
                } else {
                    if(lsm)
                        return new BiLinearLsm(input.grid(),gridout,lsmMethod);
                    else
                        return new BiLinear;
                }
            } else if(extrapolate == "linear") {
                if(bitmap) {
                    if(lsm)
                        return new BiLinearLsmBitmap(true,false,0,0,input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return new BiLinearBitmap(true,false,0,0,missingValue);
                } else {
                    if(lsm)
                        return new BiLinearLsm(true,false,0,0,input.grid(),gridout,lsmMethod);
                    else
                        return new BiLinear(true,false,0,0);
                }
            }

            double northPoleValue = 0;
            double southPoleValue = 0;
            bool extrapolateAverage = input.extrapolateAverageOnPole(northPoleValue,southPoleValue);
            bool extrapolateLinear  = input.extrapolateLinearOnPole();
            if(bitmap) {
                if(lsm)
                    return new BiLinearLsmBitmap(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,input.grid(),gridout,lsmMethod, missingValue);
                else
                    return new BiLinearBitmap(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue, missingValue);
            } else {
                if(lsm)
                    return new BiLinearLsm(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,input.grid(),gridout,lsmMethod);
                else
                    return new BiLinear(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue);
            }
        } else if(method == "bilinearinteger" || method == "BILINEARINTEGER") {
            return new BiLinearLsmInteger;
        } else if(method == "linear" || method == "LINEAR") {
            if(lsm)
                throw UserError("Factory::interpolationMethod This interpolation method is not supported -> Lsm +  ", method);
            if(extrapolate == "nearest") {
                if(bitmap)
                    return new LinearBitmap(missingValue);
                else
                    return new Linear;
            } else if(extrapolate == "linear") {
                if(bitmap)
                    return new LinearBitmap(true,false,0,0,missingValue);
                else
                    return new Linear(true,false,0,0);
            }
            double northPoleValue = 0;
            double southPoleValue = 0;
            bool extrapolateAverage = input.extrapolateAverageOnPole(northPoleValue,southPoleValue);
            bool extrapolateLinear  = input.extrapolateLinearOnPole();
            return  new Linear(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue);
        } else if(method == "linear-fit" || method == "LINEAR-FIT" || method == "LINEARFIT" || method == "linearfit") {
            if(extrapolate == "nearest") {
                if(bitmap) {
                    if(lsm)
                        return new LinearFit3TimesLsmBitmap(input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return new LinearFit3TimesBitmap(missingValue);
                } else {
                    if(lsm)
                        return new LinearFit3TimesLsm(input.grid(),gridout,lsmMethod);
                    else
                        return new LinearFit3Times;
                }
            } else if(extrapolate == "linear") {
                if(bitmap) {
                    if(lsm)
                        return new LinearFit3TimesLsmBitmap(true,false,0,0,input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return new LinearFit3TimesBitmap(true,false,0,0,missingValue);
                } else {
                    if(lsm)
                        return new LinearFit3TimesLsm(true,false,0,0,input.grid(),gridout,lsmMethod);
                    else
                        return new LinearFit3Times(true,false,0,0);
                }
            }
            double northPoleValue = 0;
            double southPoleValue = 0;
            bool extrapolateAverage = input.extrapolateAverageOnPole(northPoleValue,southPoleValue);
            bool extrapolateLinear  = input.extrapolateLinearOnPole();
            if(lsm)
                return  new LinearFit3TimesLsm(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,input.grid(),gridout,lsmMethod);
            else
                return  new LinearFit3Times(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue);
        } else if(method == "nearest-neighbour" || method == "NEAREST-NEIGHBOUR" ||
                  method == "nearestneighbour"  || method == "NEARESTNEIGHBOUR")
            if(lsm)
                return  new NearestNeigbourLsm(pointsForInterpolation,input.grid(),gridout,lsmMethod);
            else
                return  new NearestNeigbour(pointsForInterpolation);
        else if(method == "cubic") {
            if(extrapolate == "average") {
                double northPoleValue = 0;
                double southPoleValue = 0;
                bool extrapolateAverage = input.extrapolateAverageOnPole(northPoleValue,southPoleValue);
                bool extrapolateLinear  = input.extrapolateLinearOnPole();
                if(bitmap) {
                    if(lsm)
                        return  new Cubic12ptsLsmBitmap(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return  new Cubic12ptsBitmap(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,missingValue);
                } else {
                    if(lsm)
                        return  new Cubic12ptsLsm(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,input.grid(),gridout,lsmMethod);
                    else
                        return  new Cubic12pts(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue);
                }
            } else if(extrapolate == "linear") {
                if(bitmap) {
                    if(lsm)
                        return new Cubic12ptsLsmBitmap(true,false,0,0,input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return new Cubic12ptsBitmap(true,false,0,0,missingValue);
                } else {
                    if(lsm)
                        return new Cubic12ptsLsm(true,false,0,0,input.grid(),gridout,lsmMethod);
                    else
                        return new Cubic12pts(true,false,0,0);
                }
            }
            if(bitmap) {
                if(lsm)
                    return new Cubic12ptsLsmBitmap(input.grid(),gridout,lsmMethod,missingValue);
                else
                    return new Cubic12ptsBitmap(missingValue);
            } else {
                if(lsm)
                    return new Cubic12ptsLsm(input.grid(),gridout,lsmMethod);
                else
                    return new Cubic12pts;
            }
        } else if(method == "average" || method == "AVERAGE") {
// ssp to be finished for bitmap
            if(lsm)
                return new AverageLsm(pointsForInterpolation,input.grid(),gridout,lsmMethod);
            else
                return new Average(pointsForInterpolation);
        } else if(method == "double-linear" || method == "DOUBLE-LINEAR" || method == "DOUBLELINEAR" || method == "doublelinear") {
            if(bitmap) {
                if(lsm)
                    return new DoubleLinearLsmBitmap(input.grid(),gridout,lsmMethod,missingValue);
                else
                    return new DoubleLinearBitmap(missingValue);
            } else {
                if(lsm)
                    return new DoubleLinearLsm(input.grid(),gridout,lsmMethod);
                else
                    return new DoubleLinear;
            }
        } else if(method == "double-linear-adjusted" || method == "DOUBLE-LINEAR-ADJUSTED"
                  || method == "doublelinearadjusted" || method == "DOUBLELINEARADJUSTED") {
            return new DoubleLinearBitmapAdjusted(missingValue);
        } else if(method == "average-weighted" || method == "AVERAGE-WEIGHTED" ||
                  method == "averageweighted" || method == "AVERAGEWEIGHTED") {
            if(lsm) {
                if(gridout.reduced())
                    return new AverageWeightedReducedLsm(input.grid(),gridout,lsmMethod);
                else
                    return new AverageWeightedRegularLsm(input.grid(),gridout,lsmMethod);
            } else {
                if(gridout.reduced())
                    return new AverageWeightedReduced(input.grid(),gridout);
                else
                    return new AverageWeightedRegular(input.grid(),gridout);
            }
        } else if(method == "flux-conserving" || method == "FLUX-CONSERVING" ||
                  method == "fluxconserving" || method == "FLUXCONSERVING") {
            if(gridout.reduced())
                return new FluxConservingReduced(input.grid(),gridout);
            else
                return new FluxConservingRegular(input.grid(),gridout);
        } else if(method == "conserving") {
            return new Conserving();
        } else
            throw UserError("Factory::interpolationMethod This interpolation method is not supported -> ", method);
    } else {
        if (gridout.gridType() == "list") {
            if(lsm)
                return new BiLinearLsm(input.grid(),gridout,lsmMethod);
            else
                return new BiLinear;
        }

        if (input.conservation()) {
            return new BiLinearPrecipitation;
            /*
            if(lsm){
            	if(gridout.reduced())
            		return new AverageWeightedReducedLsm(input.grid(),gridout,lsmMethod);
            	else
            		return new AverageWeightedRegularLsm(input.grid(),gridout,lsmMethod);
            }
            else{
            	if(gridout.reduced())
            		return new AverageWeightedReduced(input.grid(),gridout);
            	else
            		return new AverageWeightedRegular(input.grid(),gridout);
            }
            */
        } else if (input.nearest())
            if(lsm)
                return  new NearestNeigbourLsm(pointsForInterpolation,input.grid(),gridout,lsmMethod);
            else
                return new NearestNeigbour;
        else {
            if(extrapolate == "nearest") {
                if(bitmap) {
                    if(lsm)
                        return new BiLinearLsmBitmap(input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return new BiLinearBitmap(missingValue);
                } else {
                    if(lsm)
                        return new BiLinearLsm(input.grid(),gridout,lsmMethod);
                    else
                        return new BiLinear;
                }
            } else if(extrapolate == "linear") {
                if(bitmap) {
                    if(lsm)
                        return new BiLinearLsmBitmap(true,false,0,0,input.grid(),gridout,lsmMethod,missingValue);
                    else
                        return new BiLinearBitmap(true,false,0,0,missingValue);
                } else {
                    if(lsm)
                        return new BiLinearLsm(true,false,0,0,input.grid(),gridout,lsmMethod);
                    else
                        return new BiLinear(true,false,0,0);
                }
            }
            double northPoleValue = 0;
            double southPoleValue = 0;
            bool extrapolateAverage = input.extrapolateAverageOnPole(northPoleValue,southPoleValue);
            bool extrapolateLinear  = input.extrapolateLinearOnPole();
            if(bitmap) {
                if(lsm)
                    return new BiLinearLsmBitmap(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,input.grid(),gridout,lsmMethod,missingValue);
                else
                    return new BiLinearBitmap(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,missingValue);
            } else {
                if(lsm)
                    return new BiLinearLsm(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue,input.grid(),gridout,lsmMethod);
                else
                    return new BiLinear(extrapolateLinear,extrapolateAverage,northPoleValue,southPoleValue);
            }
        }
    }

    throw UserError("Factory::interpolationMethod Unknown Method -> ", method);

    return 0;
}

DerivedSubgridParameters* Factory::selectDerivedSubgridParameter(const string& param) const {
    if(DEBUG)
        cout << "Factory::selectDerivedSubgridParameter => " << param << endl;

    if(param == "anisotropy")
        return new Anisotropy;
    else if(param == "orientation")
        return new Orientation;
    else if(param == "slope")
        return new Slope;

    throw UserError("Factory::selectDerivedSubgridParameter Unknown parameter -> ", param);

    return 0;
}

ref_counted_ptr<const LegendrePolynomials> Factory::polynomialsMethod(const string& method, int truncation, const Grid& grid) const {
    if(DEBUG)
        cout << "Factory::polynomialsMethod => " << method << endl;
    if(method == "fileio" ||  method == "FILEIO")
        return ref_counted_ptr<const LegendrePolynomials>(new LegendrePolynomialsReadFromFile(truncation,grid));
    else if(method == "mapped" || method == "MAPPED" || method == "default") {
        // check for cached value

        ref_counted_ptr<const LegendrePolynomials> ptr = Factory::memoryMapCache_.polynomials(truncation, grid);

        if (DEBUG)
            cout << "Factory::polynomialsMethod returning " << (ptr.get() ? "CACHED" : "NEW") << " polynomials for memmap" << endl;

        if (!ptr.get())
            ptr = Factory::memoryMapCache_.addPolynomials(truncation, grid, new LegendrePolynomialsMemoryMap(truncation, grid));

        return ptr;

    } else if(method == "shared" || method == "SHARED") {
        // check for cached value

        ref_counted_ptr<const LegendrePolynomials> ptr = Factory::sharedMemoryCache_.polynomials(truncation, grid);

        if (DEBUG)
            cout << "Factory::polynomialsMethod returning " << (ptr.get() ? "CACHED" : "NEW") << " polynomials for shared" << endl;

        if (!ptr.get())
            ptr = Factory::sharedMemoryCache_.addPolynomials(truncation, grid, new LegendrePolynomialsSharedMemory(truncation, grid));

        return ptr;

    } else if(method == "on_fly" || method == "ON_FLY" || method == "on-fly" || method == "ON-FLY")
        return ref_counted_ptr<const LegendrePolynomials>(new LegendrePolynomialsOnFly(truncation));
    else // unknown method
        throw UserError("Factory::polynomialsMethod Unknown method requested -> ", method);
}

Extraction* Factory::multiExtraction(const GridField& out, double missingValue)  const {
    if(DEBUG)
        cout << "Factory::multiExtraction frame " << out.frame() << "   bitmap " << out.bitmap() << endl;

    if(out.frame()) {
        return new Frame(out.frame(), missingValue);
    } else if(out.bitmap()) {
        return new Bitmap(out.bitmapFile(), missingValue);
    }
//	throw UserError("Factory::multiExtraction  there isn't frame or bitmap");
    return 0;
}

Extraction* Factory::multiExtractionWithoutSubArea(const GridField& out, double missingValue)  const {
    if(out.frame()) {
        return new Frame(out.frame(), missingValue);
    } else if(out.bitmap()) {
        return new Bitmap(out.bitmapFile(), missingValue);
    }
    //throw UserError("Factory::multiExtractionWithoutSubArea  there isn't frame or bitmap  ");
    return 0;

//ssp recursion ?
// multiExtractionWithoutSubArea(ext)
}


Transformer* Factory::getTransformer(const Field& in, const Field& out, const FieldDescription& fd) const {
    string legendrePolynomialsMethod = fd.ft_.legendrePolynomialsMethod_;
    int    fftMax      = fd.ft_.fftMax_;
    bool   auresol     = fd.ft_.auresol_;
    string intMethod   = fd.ft_.interpolationMethod_;
    string lsmMethod   = fd.ft_.lsmMethod_;
    bool   conversion  = fd.ft_.vdConversion_;
    int    numberOfNearestPoints = fd.ft_.numberOfNearestPoints_;
    string transType        = fd.ft_.grid2gridTransformationType_;
    string extrapolate      = fd.ft_.extrapolateOnPole_;

    string inName  = in.nameOfField();
    string outName = out.nameOfField();

    if(DEBUG)
        cout << "Factory::getTransformer from " << inName << " to " << outName << endl;

    if(inName == "sh" && outName == "sh") {
        return new SpectralToSpectralTransformer(conversion);
    } else if(inName != "sh" && outName != "sh") {
        if (IS_SET("ECREGRID_EXPERIMENTAL")) {
#ifdef EIGEN3_FOUND
            return new GridToGridMatrixTransformer(intMethod,lsmMethod,numberOfNearestPoints,transType,extrapolate);
#else
            ASSERT( "Eigen3 not found" != 0 );
#endif
        } else {
            return new GridToGridTransformer(intMethod,lsmMethod,numberOfNearestPoints,transType,extrapolate);
        }
    } else if(inName == "sh" && outName != "sh") {
        if(out.isRotated()) {
            return new SpectralToRotatedGridTransformer(legendrePolynomialsMethod,fftMax,auresol,conversion);
        } else {
            if(outName == "list") {
                return new SpectralToListOfPointsTransformer(legendrePolynomialsMethod,fftMax,auresol,conversion);
            } else {
                return new SpectralToGridTransformer(legendrePolynomialsMethod,fftMax,auresol,conversion);
            }
        }
    } else if(inName != "sh" && outName == "sh")
        return new GridToSpectralTransformer(legendrePolynomialsMethod,fftMax);


    throw UserError("Factory::getTransformer  Tranform from "+inName+" to "+outName," is not supported -> ");

}

Input* Factory::getInput(const string& fileName,const string& kind)  const {
    if(kind == "binary")
        return new BinaryInput(fileName);
    else if(kind == "grib")
        return new GribApiInput(fileName);
    else if(kind == "ascii")
        return new AsciiInput(fileName);
    else
        throw UserError("Factory::getInput This Input is not supported -> ", kind);

    return 0;
}

GribApiOutput* Factory::getGribApiOutput(const string& name)  const {
    if(name == "sh")
        return new GribApiOutputSpectral;
    else
        return new GribApiOutputGrid;

    return 0;
}

Output* Factory::getOutput(const string& fileName, const string& kind, const string& name)  const {
    if(kind == "binary")
        return  new BinaryOutput(fileName);
    else if(kind == "grib") {
        if(DEBUG)
            cout << "Factory::getOutput field name: " << name << endl;
        if(name == "sh")
            return new GribApiOutputSpectral(fileName);
        else
            return new GribApiOutputGrid(fileName);
    } else if(kind == "ascii")
        return new AsciiOutput(fileName);

    throw UserError("Factory::getOutput This Output is not supported -> ", kind);

    return 0;
}

Output* Factory::getOutputBinTxt(const string& fileName, const string& kind)  const {
    if(kind == "binary")
        return new BinaryOutput(fileName);
    else if(kind == "ascii")
        return new AsciiOutput(fileName);

    throw UserError("Factory::getOutput This Output is not supported -> ", kind);

    return 0;
}

GribApiOutput* Factory::getGribApiOutput(const string& fileName, const string& name)  const {
    if(DEBUG)
        cout << "Factory::getOutput field name: " << name << endl;
    if(name == "sh")
        return new GribApiOutputSpectral(fileName);
    else
        return new GribApiOutputGrid(fileName);

    throw UserError("Factory::getOutput This Output is not supported -> ", name);

    return 0;
}

Lsm* Factory::getLsm(const string& fileName, const string& lsmMethod, const string& lsmFileType, bool user, const string& userPath) const {
    if(DEBUG)
        cout << "Factory::getLsm lsmMethod: ((( " << lsmMethod << " ))) fileName: " << fileName << " lsmFileType: " << lsmFileType << endl;

    Input* in = getInput(fileName, lsmFileType);

    if(lsmMethod == "predefined" || lsmMethod == "PREDEFINED") {
        if(user)
            return new LsmPreDefined(in,userPath);
        else
            return new LsmPreDefined(in);
    } else if(lsmMethod == "10min" || lsmMethod == "10MIN")
        return new LsmEmos10minute(in);
    else if(lsmMethod == "ll1km" || lsmMethod == "LL1KM") {
        Interpolator* method = new NearestNeigbour(4);
        double ns = 180/21600;
        double we = 180/21600;
        Grid* grid = new RegularLatLon(ns,we);
        return new LsmFromGrid(in,grid,method);
    } else if(lsmMethod == "gtopo" || lsmMethod == "gtopo")
        return new GTopo30(in);
    else {
        if(user)
            return new LsmPreDefined(in,userPath);
        else
            return new LsmPreDefined(in);
    }

    throw UserError("Interpolator::getLsm This Lsm method is not supported -> ", lsmMethod);

    return 0;
}

