// File SphericalHarmonics.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "SphericalHarmonics.h"

#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"
#include "eckit/log/Log.h"


#include <iostream>
#include "Grib.h"

SphericalHarmonics::SphericalHarmonics(const MIRParametrisation &parametrisation) {

    eckit::Translator<std::string, size_t> s2d;
    std::string value;

    ASSERT(parametrisation.get("truncation", value));
    truncation_ = s2d(value);

}

SphericalHarmonics::SphericalHarmonics(size_t truncation):
    truncation_(truncation) {
}


SphericalHarmonics::~SphericalHarmonics() {
}


void SphericalHarmonics::print(std::ostream &out) const {
    out << "SphericalHarmonics["
        << "truncation=" << truncation_
        << "]";
}


void SphericalHarmonics::fill(grib_info &info) const  {
    info.grid.grid_type = GRIB_UTIL_GRID_SPEC_SH;
    info.grid.truncation = truncation_;
    // info.packing.computeLaplacianOperator = 1;
    // info.packing.truncateLaplacian = 1;
    // info.packing.laplacianOperator = 0;
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX; // Check if this is needed, why does grib_api not copy input?
}

Representation *SphericalHarmonics::truncate(size_t truncation,
        const std::vector<double> &in, std::vector<double> &out) const {

    // WARNING: Untested code!

    if (truncation == truncation_) {
        return 0;

    }

    size_t insize = (truncation_ + 1) * (truncation_ + 2);
    ASSERT(in.size() == insize);

    SphericalHarmonics *sh = new SphericalHarmonics(truncation);

    size_t outsize = (truncation + 1) * (truncation + 2);
    out = std::vector<double>(outsize);

    int delta = truncation_ - truncation;
    size_t i = 0;
    size_t j = 0;

    if (delta > 0) {
        size_t t1 = truncation + 1;
        for (size_t m = 0; m < t1; m++) {
            for (size_t n = m ; n < t1; n++) {
                out[i++] = in[j++];
                out[i++] = in[j++];
            }
            j += delta;
            j += delta;
        }
        ASSERT(i == outsize);
    } else {
        // Pad with zeros
        size_t t1 = truncation + 1;
        size_t t_ = truncation_ ;

        for (size_t m = 0; m < t1; m++) {
            for (size_t n = m ; n < t1; n++) {
                if (m > t_ || n > t_)
                {
                    out[i++] = 0;
                    out[i++] = 0;
                } else {
                    out[i++] = in[j++];
                    out[i++] = in[j++];
                }
            }
        }
        // std::cout << j << " " << insize << std::endl;
        ASSERT(j == insize);
    }

    // std::cout << i << " " << outsize << std::endl;
    // std::cout << j << " " << insize << std::endl;

    // ASSERT(i == outsize);
    // ASSERT(j == insize);

    return sh;
}


static RepresentationBuilder<SphericalHarmonics> sphericalHarmonics("sh"); // Name is what is returned by grib_api
