// File GribOutput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "GribOutput.h"
#include "GribInput.h"
#include "MIRField.h"
#include "MIRParametrisation.h"

#include <istream>
#include "Grib.h"
#include "Representation.h"

#include "eckit/utils/Translator.h"
#include "eckit/io/DataHandle.h"


#define X(a) std::cout << #a << " = " << a << std::endl

class HandleFree {
    grib_handle *h_;
  public:
    HandleFree(grib_handle *h): h_(h) {}
    ~HandleFree() {
        if (h_) grib_handle_delete(h_);
    }
};

GribOutput::GribOutput() {
}

GribOutput::~GribOutput() {
}


void GribOutput::copy(const MIRParametrisation &param, MIRInput &input) { // Not iterpolation performed

    grib_handle *h = input.gribHandle(); // Base class will throw an exception is input cannot provide a grib_handle

    const void *message;
    size_t size;

    GRIB_CALL(grib_get_message(h, &message, &size));

    dataHandle().write(message, size);
}

void GribOutput::save(const MIRParametrisation &param, MIRInput &input, MIRField &field) {

    grib_handle *h = input.gribHandle(); // Base class will throw an exception is input cannot provide a grib_handle


    grib_spec spec = {0,};



    /* bitmap */
    spec.grid.bitmapPresent = field.hasMissing() ? 1 : 0;
    spec.grid.missingValue = field.missingValue();

    /* Packing options */

    spec.packing.packing = GRIB_UTIL_PACKING_SAME_AS_INPUT;
    spec.packing.accuracy = GRIB_UTIL_ACCURACY_SAME_BITS_PER_VALUES_AS_INPUT;

    std::string value;
    if (param.get("accuracy", value)) {
        long bits = eckit::Translator<std::string, long>()(value);
        spec.packing.accuracy = GRIB_UTIL_ACCURACY_USE_PROVIDED_BITS_PER_VALUES;
        spec.packing.bitsPerValue = bits;
    }

    // Ask last representation to update spec

    field.representation()->fill(spec);


    X(spec.grid.grid_type);
    X(spec.grid.Ni);
    X(spec.grid.Nj);
    X(spec.grid.iDirectionIncrementInDegrees);
    X(spec.grid.jDirectionIncrementInDegrees);
    X(spec.grid.longitudeOfFirstGridPointInDegrees);
    X(spec.grid.longitudeOfLastGridPointInDegrees);
    X(spec.grid.latitudeOfFirstGridPointInDegrees);
    X(spec.grid.latitudeOfLastGridPointInDegrees);
    X(spec.grid.uvRelativeToGrid);
    X(spec.grid.latitudeOfSouthernPoleInDegrees);
    X(spec.grid.longitudeOfSouthernPoleInDegrees);
    X(spec.grid.iScansNegatively);
    X(spec.grid.jScansPositively);
    X(spec.grid.N);
    X(spec.grid.bitmapPresent);
    X(spec.grid.missingValue);
    X(spec.grid.pl_size);
    X(spec.grid.truncation);
    X(spec.grid.orientationOfTheGridInDegrees);
    X(spec.grid.DyInMetres);
    X(spec.grid.DxInMetres);
    X(spec.packing.packing_type);
    X(spec.packing.packing);
    X(spec.packing.boustrophedonic);
    X(spec.packing.editionNumber);
    X(spec.packing.accuracy);
    X(spec.packing.bitsPerValue);
    X(spec.packing.decimalScaleFactor);
    X(spec.packing.computeLaplacianOperator);
    X(spec.packing.truncateLaplacian);
    X(spec.packing.laplacianOperator);
    X(spec.packing.deleteLocalDefinition);
    // X(spec.packing.extra_settings);
    X(spec.packing.extra_settings_count);



    int flags = 0;
    int err = 0;

    const std::vector<double> values = field.values();

    grib_handle *result = grib_util_set_spec(h, &spec.grid, &spec.packing, flags, &values[0], values.size(), &err);
    HandleFree hf(result); // Make sure handle deleted even in case of exception

    GRIB_CALL(err);

    const void *message;
    size_t size;

    GRIB_CALL(grib_get_message(result, &message, &size));

    dataHandle().write(message, size);

}
