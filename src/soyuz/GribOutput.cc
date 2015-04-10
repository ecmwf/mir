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

    out(message, size, false);

}

void GribOutput::save(const MIRParametrisation &param, MIRInput &input, MIRField &field) {

    grib_handle *h = input.gribHandle(); // Base class will throw an exception is input cannot provide a grib_handle


    grib_info info = {0,};



    /* bitmap */
    info.grid.bitmapPresent = field.hasMissing() ? 1 : 0;
    info.grid.missingValue = field.missingValue();

    /* Packing options */

    info.packing.packing = GRIB_UTIL_PACKING_SAME_AS_INPUT;
    info.packing.accuracy = GRIB_UTIL_ACCURACY_SAME_BITS_PER_VALUES_AS_INPUT;

    std::string value;
    if (param.get("accuracy", value)) {
        long bits = eckit::Translator<std::string, long>()(value);
        info.packing.accuracy = GRIB_UTIL_ACCURACY_USE_PROVIDED_BITS_PER_VALUES;
        info.packing.bitsPerValue = bits;
    }

    // Ask last representation to update info

    field.representation()->fill(info);


    X(info.grid.grid_type);
    X(info.grid.Ni);
    X(info.grid.Nj);
    X(info.grid.iDirectionIncrementInDegrees);
    X(info.grid.jDirectionIncrementInDegrees);
    X(info.grid.longitudeOfFirstGridPointInDegrees);
    X(info.grid.longitudeOfLastGridPointInDegrees);
    X(info.grid.latitudeOfFirstGridPointInDegrees);
    X(info.grid.latitudeOfLastGridPointInDegrees);
    X(info.grid.uvRelativeToGrid);
    X(info.grid.latitudeOfSouthernPoleInDegrees);
    X(info.grid.longitudeOfSouthernPoleInDegrees);
    X(info.grid.iScansNegatively);
    X(info.grid.jScansPositively);
    X(info.grid.N);
    X(info.grid.bitmapPresent);
    X(info.grid.missingValue);
    X(info.grid.pl_size);
    X(info.grid.truncation);
    X(info.grid.orientationOfTheGridInDegrees);
    X(info.grid.DyInMetres);
    X(info.grid.DxInMetres);
    X(info.packing.packing_type);
    X(info.packing.packing);
    X(info.packing.boustrophedonic);
    X(info.packing.editionNumber);
    X(info.packing.accuracy);
    X(info.packing.bitsPerValue);
    X(info.packing.decimalScaleFactor);
    X(info.packing.computeLaplacianOperator);
    X(info.packing.truncateLaplacian);
    X(info.packing.laplacianOperator);
    X(info.packing.deleteLocalDefinition);
    // X(info.packing.extra_settings);
    X(info.packing.extra_settings_count);



    int flags = 0;
    int err = 0;

    const std::vector<double> values = field.values();

    grib_handle *result = grib_util_set_spec(h, &info.grid, &info.packing, flags, &values[0], values.size(), &err);
    HandleFree hf(result); // Make sure handle deleted even in case of exception

    GRIB_CALL(err);

    const void *message;
    size_t size;

    GRIB_CALL(grib_get_message(result, &message, &size));

    out(message, size, true);

}
