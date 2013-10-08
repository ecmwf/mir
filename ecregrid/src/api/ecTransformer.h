#ifndef ECTRANSFORMER_H
#define ECTRANSFORMER_H

// Factory classes to produce concrete instantiations of interfaces 
#include <vector>

namespace ecregrid {
    class IFieldRepresentation;
    class ITransformer;
    class DFO;
}

namespace ecregrid {

// Holds options for LSM, Legendre, interpolation methods etc
// relating to the transformation process rather than the 
// field descriptions themselves	


class TransformerOptions {
public:

    enum InterpolationMethod { NoInterpolation, Bilinear, NearestNeighbour, Cubic, Linear, LinearFit, Average, AverageWeighted };
    enum LSMMethod { NoLSM, Predefined, TenMin, GTOPO };
    enum LegendrePolynomialsMethod { NoPolynomials, OnFly, FileIO, SharedMemory, MemoryMapped };
    enum PoleExtrapolationMethod { NoPole, AveragePole, LinearPole };

    TransformerOptions(InterpolationMethod intMethod); // with defaults according to method selected

    void setInterpolationMethod(InterpolationMethod method);
    void setLSMMethod(LSMMethod method);
    void setLegendrePolynomialsMethod(LegendrePolynomialsMethod method);
    void setPoleExtrapolationMethod(PoleExtrapolationMethod method);
    void setAutoResolution(bool set);

private:

    InterpolationMethod interpolationMethod_;
    LSMMethod lsmMethod_;
    LegendrePolynomialsMethod legendrePolynomialsMethod_;
    PoleExtrapolationMethod poleExtrapolationMethod_;

    bool      autoResolution_;

    friend class TransformerOptionsReader;
};


class TransformerOptionsReader
{
public:
    TransformerOptionsReader(const TransformerOptions& opts) : opts_(opts) { };

    TransformerOptions::InterpolationMethod interpolationMethod() const;
    TransformerOptions::LSMMethod lsmMethod() const;
    TransformerOptions::LegendrePolynomialsMethod legendrePolynomialsMethod() const;
    TransformerOptions::PoleExtrapolationMethod poleExtrapolationMethod() const;

    bool autoResolution() const;

private:
    const TransformerOptions& opts_;

};

// Returns interface to object performing transformations between fields
class TransformerFactory {
public:
    // Output field description deliberately left non-const
    // as it may need to be adjusted by the interpolator (area etc)
	ITransformer* make(const IFieldRepresentation& in, IFieldRepresentation& out, const TransformerOptions& opts);
};

// Returns interface to field representation objects
class FieldRepresentationFactory {
public:	
    // Have included methods for both C struct and string arguments
    // NB it is likely that string will not contain sufficient information
    // for us to proceed
	IFieldRepresentation* make(const ecregrid::DFO& opts); 
};

class TransformerUtils {
public:

    // Have included a helper function to return the reduced gaussian grid
    // definition for a given resolution.
    //
    // For input value N, a vector is filled containing the number of grid
    // points along each of N * 2 latitudes according to the grid definition
    // (N values for the upper hemisphere, N for the lower).
    // The second N values are the same as the first N in reverse order.
    //
    // Where the reduced grid definition is not known, the function throws
    // an exception and the vector is untouched.
    //
    static void getReducedGaussianRepresentation(unsigned int gaussian_number, std::vector<long>& grid_spec);



};

}

#endif //ECTRANSFORMER_H
