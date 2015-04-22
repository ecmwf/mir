/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <istream>

#include "eckit/utils/Translator.h"
#include "eckit/io/DataHandle.h"

#include "soyuz/data/MIRField.h"
#include "soyuz/input/MIRInput.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/repres/Representation.h"
#include "soyuz/util/Grib.h"

#include "soyuz/output/GribOutput.h"


namespace mir {
namespace output {


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


void GribOutput::copy(const param::MIRParametrisation &param, input::MIRInput &input) { // Not iterpolation performed

    grib_handle *h = input.gribHandle(); // Base class will throw an exception is input cannot provide a grib_handle

    const void *message;
    size_t size;

    GRIB_CALL(grib_get_message(h, &message, &size));

    out(message, size, false);

}


void GribOutput::save(const param::MIRParametrisation &param, input::MIRInput &input, data::MIRField &field) {

    grib_handle *h = input.gribHandle(); // Base class will throw an exception is input cannot provide a grib_handle


    grib_info info = {{0},};



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


#undef X


}  // namespace output
}  // namespace mir

