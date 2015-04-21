#ifndef DFO_H
#define DFO_H

#include <vector>
#include <string>

#include "ecField.h"

// DFO == Description Factory Object

// Description of the field object passed to factory
// Possibly reusing code from grib api
//
// Would contain all relevant data taken
// from grib files but can be used by
// non-grib users also

namespace ecregrid {

class DFOReader;

class DFO {

  public:
    enum Representation { RegularLatLon,   RotatedRegularLatLon, ReducedLatLon,
                          RegularGaussian, ReducedGaussian,
                          SphericalHarmonic,
                          PolarStereographic,
                          ListOfPoints
                        };

    enum ScanMode {     NSEW,      // i scans positively, j scans negatively
                        SNEW,      // i scans positively, j scans positively
                        NSWE,      // i scans negatively, j scans negatively
                        SNWE       // i scans negatively, j scans positively
                  };

    // constructors to give self-consistent objects with our chosen defaults
    // that can be modified using methods below
    DFO(Representation rep);


    // The following methods will throw an exception should the
    // parameter being set be irrelevant to the Representation specified
    // in the constructor (e.g. setting Gaussian Number of SphericalHarmonic
    // representation)

    // for spherical representations
    void setTruncation(int truncation);

    // for gaussian grids
    void setGaussianNumber(int number);

    // for lat/lon grids
    void setIncrements(double northSouth, double westEast);

    // for reduced grids
    void setReducedGridDefinition(const std::vector<long>& def);

    // for all grids
    void setScanMode(ScanMode mode);
    void setSouthPoleOfRotation(const std::pair<double, double>& pt);

    void setSubArea(const eckit::Area& area);
    void clearSubArea();

    void setFrameNumber(int frameNumber); // frameNumber=0 indicates no framing

    void setBitmapFile(const std::string& file);
    void clearBitmapFile();

    void setMissingValue(double val);

    void setCellCentred(bool cellCentred);
    void setShifted(bool shifted);

  private:

    Representation representation_;

    // Grid
    double northSouthIncrement_;
    double westEastIncrement_;

    std::vector<long> reducedGridDefinition_;

    bool cellCentred_;
    bool shifted_;

    ScanMode scanMode_;

    int gaussianNumber_;

    // spherical harmonics
    int truncation_;

    // Sub-area and Rotation
    eckit::Area subArea_;
    std::pair<double, double> southPoleOfRotation_;

    // Bitmap
    std::string    bitmapFile_;
    double    missingValue_;

    // Frame
    int       frameNumber_;

    friend class DFOReader;
};


class DFOReader {
  public:
    DFOReader(const DFO& dfo) : dfo_(dfo) {};

    long numberOfPointsNorthSouth() const;
    long numberOfPointsWestEast() const;

    double westEastIncrement() const;
    double northSouthIncrement() const;

    void reducedGridDefinition(std::vector<long>& grid) const;

    bool cellCentred() const;
    bool shifted() const;

    ecregrid::DFO::ScanMode scanMode() const;
    DFO::Representation representation() const;

    int gaussianNumber() const;
    int truncation() const;

    void subArea(eckit::Area& area) const;
    void southPoleOfRotation(std::pair<double, double>& pole) const;

    void bitmapInfo(std::string& fileName, double& missingValue) const;
    bool usingBitmap() const;
    int frameNumber() const;

  private:
    const DFO& dfo_;
};


}
#endif // DFO_H
