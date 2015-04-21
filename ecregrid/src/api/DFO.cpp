#include "DFO.h"
#include <stdexcept>

namespace ecregrid {

DFO::DFO(Representation rep)
    : representation_(rep),
      northSouthIncrement_(0.0),
      westEastIncrement_(0.0),
      cellCentred_(false),
      shifted_(false),
      scanMode_(NSEW),
      gaussianNumber_(0),
      truncation_(0),
      southPoleOfRotation_(std::make_pair(0.0, 0.0)),
      bitmapFile_(""),
      missingValue_(9999),
      frameNumber_(0) {

    // Add any additional implementation according to representation
    //
    switch (representation_) {
    case RegularLatLon:
        break;
    case RotatedRegularLatLon:
        break;
    case ReducedLatLon:
        break;
    case RegularGaussian:
        break;
    case ReducedGaussian:
        break;
    case SphericalHarmonic:
        break;
    case PolarStereographic:
        break;
    case ListOfPoints:
        break;
    default:
        break;
    }

}

void DFO::setGaussianNumber(int number) {
    if (RegularGaussian != representation_ && ReducedGaussian != representation_ )
        throw std::runtime_error("Unknown Gaussian representation");

    gaussianNumber_ = number;
}

void DFO::setTruncation(int truncation) {
    if (SphericalHarmonic != representation_)
        throw std::runtime_error("Setting truncation on a non-spherical representation");

    truncation_ = truncation;
}

void DFO::setScanMode(ScanMode mode) {
    // TODO work out which grids this is applicable to
    scanMode_ = mode;
}

void DFO::setReducedGridDefinition(const std::vector<long>& def) {
    if (ReducedLatLon != representation_ && ReducedGaussian != representation_)
        throw std::runtime_error("Setting reduced grid definition on non-reduced representation");

    reducedGridDefinition_ = std::vector<long>(def);
}

void DFO::setSouthPoleOfRotation(const std::pair<double, double>& pt) {
    if (RotatedRegularLatLon != representation_)
        throw std::runtime_error("Setting rotation on non-rotated representation");

    southPoleOfRotation_ = std::pair<double, double>(pt);
}

void DFO::setSubArea(const eckit::Area& area) {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Setting subarea on spherical representation");

    if (ReducedLatLon == representation_ || ReducedGaussian == representation_)
        throw std::runtime_error("Setting subarea on reduced representation");

    subArea_ = eckit::Area(area);
}

void DFO::clearSubArea() {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Clearing subarea on spherical representation");

    subArea_ = eckit::Area();  // take default value
}

void DFO::setFrameNumber(int frameNumber) {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Setting frame on spherical representation");

    if (ReducedLatLon == representation_ || ReducedGaussian == representation_)
        throw std::runtime_error("Setting frame on reduced representation");

    frameNumber_ = frameNumber;
}

void DFO::setBitmapFile(const std::string& file) {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Setting bitmap on spherical representation");

    bitmapFile_ = file;
};

// combine the following to a single method?
void DFO::setMissingValue(double val) {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Setting missing value on spherical representation");

    missingValue_ = val;
}

void DFO::clearBitmapFile() {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Clearing bitmap on spherical representation");

    bitmapFile_ = "";
}

void DFO::setIncrements(double northSouth, double westEast) {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Setting increments on spherical representation");

    if (RegularLatLon != representation_ && RotatedRegularLatLon != representation_)
        throw std::runtime_error("Setting increments on non-regular LatLon representation");

    northSouthIncrement_ = northSouth;
    westEastIncrement_ = westEast;
}

void DFO::setCellCentred(bool cellCentred) {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Setting cell-centred on spherical representation");

    cellCentred_ = cellCentred;
}

void DFO::setShifted(bool shifted) {
    if (SphericalHarmonic == representation_)
        throw std::runtime_error("Setting shifted on spherical representation");

    shifted_ = shifted;
}

// DFOReader

double DFOReader::westEastIncrement() const {
    return dfo_.westEastIncrement_;
}

double DFOReader::northSouthIncrement() const {
    return dfo_.northSouthIncrement_;
}

void DFOReader::reducedGridDefinition(std::vector<long>& grid) const {
    grid = std::vector<long>(dfo_.reducedGridDefinition_);
}

bool DFOReader::cellCentred() const {
    return dfo_.cellCentred_;
}

bool DFOReader::shifted() const {
    return dfo_.shifted_;
}

DFO::ScanMode DFOReader::scanMode() const {
    return dfo_.scanMode_;
}

DFO::Representation DFOReader::representation() const {
    return dfo_.representation_;
}

int DFOReader::gaussianNumber() const {
    return dfo_.gaussianNumber_;
}

int DFOReader::truncation() const {
    return dfo_.truncation_;
}

void DFOReader::subArea(eckit::Area& area) const {
    area = dfo_.subArea_;
}

void DFOReader::southPoleOfRotation(std::pair<double, double>& pole) const {
    pole = std::pair<double, double>(dfo_.southPoleOfRotation_);
}

bool DFOReader::usingBitmap() const {
    return dfo_.bitmapFile_.size() > 0;
}

void DFOReader::bitmapInfo(std::string& fileName, double& missingValue) const {
    fileName = dfo_.bitmapFile_;
    missingValue = dfo_.missingValue_;
}

int DFOReader::frameNumber() const {
    return dfo_.frameNumber_;
}

}
