#include "ecTransformer.h"
#include "ITransformer.h"
#include <ecImpl.h>
#include <RegularGaussian.h>
#include <ReducedGaussian.h>

namespace ecregrid {

// Contains logic to create the correct transformer according to the input and
// output representations and the options passed.
ITransformer* TransformerFactory::make(const IFieldRepresentation& in, IFieldRepresentation& out, const TransformerOptions& opts) {
    return new TransformerImpl(in, out, opts);
}

// Returns a particular field representation according to the struct
// containing the set of options for the field
IFieldRepresentation* FieldRepresentationFactory::make(const ecregrid::DFO& opts) {
    return new FieldRepresentationImpl(opts);
}

TransformerOptions::TransformerOptions(InterpolationMethod intMethod)
    : interpolationMethod_(intMethod),
      lsmMethod_(Predefined),
      legendrePolynomialsMethod_(FileIO),
      poleExtrapolationMethod_(AveragePole),
      autoResolution_(false) {
    //  set up with defaults according to method
    switch (interpolationMethod_) {
    case NoInterpolation:
        break;
    case Bilinear:
        break;
    case NearestNeighbour:
        break;
    case Cubic:
        break;
    case Linear:
        break;
    case LinearFit:
        break;
    case Average:
        break;
    case AverageWeighted:
        break;
    default:
        break;


    }
}

void TransformerOptions::setInterpolationMethod(InterpolationMethod method) {
    interpolationMethod_ = method;
}

void TransformerOptions::setLSMMethod(LSMMethod method) {
    lsmMethod_ = method;
}

void TransformerOptions::setLegendrePolynomialsMethod(LegendrePolynomialsMethod method) {
    legendrePolynomialsMethod_ = method;
}

void TransformerOptions::setPoleExtrapolationMethod(PoleExtrapolationMethod method) {
    poleExtrapolationMethod_ = method;
}

void TransformerOptions::setAutoResolution(bool set) {
    autoResolution_ = set;
}


// TransformerOptionsReader
//
TransformerOptions::InterpolationMethod TransformerOptionsReader::interpolationMethod() const {
    return opts_.interpolationMethod_;
}

TransformerOptions::LSMMethod TransformerOptionsReader::lsmMethod() const {
    return opts_.lsmMethod_;
}

TransformerOptions::LegendrePolynomialsMethod TransformerOptionsReader::legendrePolynomialsMethod() const {
    return opts_.legendrePolynomialsMethod_;
}

TransformerOptions::PoleExtrapolationMethod TransformerOptionsReader::poleExtrapolationMethod() const {
    return opts_.poleExtrapolationMethod_;
}

bool TransformerOptionsReader::autoResolution() const {
    return opts_.autoResolution_;
}


/*static */ void TransformerUtils::getReducedGaussianRepresentation(unsigned int gaussian_number, std::vector<long>& grid_spec) {
    try {
        auto_ptr<ReducedGaussian> grid( new ReducedGaussian(gaussian_number));
        const std::vector<long>& defn = grid->gridDefinition();
        grid_spec = defn;
    } catch (...) {
        throw std::runtime_error("Unknown Reduced Gaussian representation");
    }

}

}
