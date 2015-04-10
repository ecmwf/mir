// File GribInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "GribInput.h"
#include "soyuz/util/MIRField.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/BufferedHandle.h"

#include "soyuz/util/Grib.h"


GribInput::GribInput() {
}

GribInput::~GribInput() {
}


const MIRParametrisation &GribInput::parametrisation() const {
    return *this;
}

MIRField *GribInput::field() const {
    ASSERT(grib_.get());

    size_t count;
    GRIB_CALL(grib_get_size(grib_.get(), "values", &count));

    size_t size = count;
    std::vector<double> values(count);
    GRIB_CALL(grib_get_double_array(grib_.get(), "values", &values[0], &size));
    ASSERT(count == size);

    long bitmap;
    GRIB_CALL(grib_get_long(grib_.get(), "bitmapPresent", &bitmap));

    double missing;
    GRIB_CALL(grib_get_double(grib_.get(), "missingValue", &missing));

    MIRField *field = new MIRField(bitmap != 0, missing);
    field->values(values);
    return field;
}


grib_handle *GribInput::gribHandle() const {
    return grib_.get();
}


static struct {
    const char *name;
    const char *key;
} mappings[] = {
    {"west_east_increment", "iDirectionIncrementInDegrees"},
    {"north_south_increment", "jDirectionIncrementInDegrees"},
    {"west", "longitudeOfFirstGridPointInDegrees"},
    {"east", "longitudeOfLastGridPointInDegrees"},
    {"north", "latitudeOfFirstGridPointInDegrees"},
    {"south", "latitudeOfLastGridPointInDegrees"},
    {"truncation", "pentagonalResolutionParameterJ"},// Assumes triangular truncation
    {0, 0},
};



bool GribInput::lowLevelGet(const std::string &name, std::string &value) const {

    // WARNING: Make sure the cache is cleared
    std::map<std::string, std::string>::const_iterator j = cache_.find(name);
    if (j != cache_.end()) {
        value = (*j).second;
        return true;
    }

    eckit::Log::info() << "GribInput::get " << name << std::endl;

    ASSERT(grib_.get());

    // Assumes LL grid, and scanning mode

    if (name == "area") {
        double latitudeOfFirstGridPointInDegrees;
        double longitudeOfFirstGridPointInDegrees;
        double latitudeOfLastGridPointInDegrees;
        double longitudeOfLastGridPointInDegrees;
        double jDirectionIncrementInDegrees;
        double iDirectionIncrementInDegrees;

        if (!GRIB_GET(grib_get_double(grib_.get(), "latitudeOfFirstGridPointInDegrees", &latitudeOfFirstGridPointInDegrees))) {
            return false;
        }

        if (!GRIB_GET(grib_get_double(grib_.get(), "longitudeOfFirstGridPointInDegrees", &longitudeOfFirstGridPointInDegrees))) {
            return false;
        }

        if (!GRIB_GET(grib_get_double(grib_.get(), "latitudeOfLastGridPointInDegrees", &latitudeOfLastGridPointInDegrees))) {
            return false;
        }

        if (!GRIB_GET(grib_get_double(grib_.get(), "longitudeOfLastGridPointInDegrees", &longitudeOfLastGridPointInDegrees))) {
            return false;
        }

        if (!GRIB_GET(grib_get_double(grib_.get(), "jDirectionIncrementInDegrees", &jDirectionIncrementInDegrees))) {
            return false;
        }

        if (!GRIB_GET(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees))) {
            return false;
        }


        double v = latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees;
        double h = (longitudeOfLastGridPointInDegrees + iDirectionIncrementInDegrees) - longitudeOfFirstGridPointInDegrees;

        if (v == 180 && h == 360) {
            value = "global";
        } else {
            eckit::StrStream os;
            os << latitudeOfFirstGridPointInDegrees
               << "/"
               << longitudeOfFirstGridPointInDegrees
               << "/"
               << latitudeOfLastGridPointInDegrees
               << "/"
               << longitudeOfLastGridPointInDegrees
               << eckit::StrStream::ends;

            value = std::string(os);
        }

        eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

        cache_[name] = value;
        return true;
    }

    if (name == "grid") {

        double jDirectionIncrementInDegrees;
        double iDirectionIncrementInDegrees;

        if (!GRIB_GET(grib_get_double(grib_.get(), "jDirectionIncrementInDegrees", &jDirectionIncrementInDegrees))) {
            return false;
        }

        if (!GRIB_GET(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees))) {
            return false;
        }


        eckit::StrStream os;
        os << iDirectionIncrementInDegrees
           << "/"
           << jDirectionIncrementInDegrees
           << eckit::StrStream::ends;

        value = std::string(os);

        eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

        cache_[name] = value;
        return true;
    }

    if (name == "regular") {
        std::string type;
        if (get("gridType", type)) {
            if (type == "regular_gg") {

                long N;

                GRIB_CALL(grib_get_long(grib_.get(), "N", &N));
                // GRIB_CALL(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees));

                eckit::StrStream os;
                os << N << eckit::StrStream::ends;

                value = std::string(os);

                eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

                cache_[name] = value;
                return true;
            }
        }
    }

    if (name == "reduced") {
        std::string type;
        if (get("gridType", type)) {
            if (type == "reduced_gg") {

                long N;

                GRIB_CALL(grib_get_long(grib_.get(), "N", &N));
                // GRIB_CALL(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees));

                eckit::StrStream os;
                os << N << eckit::StrStream::ends;

                value = std::string(os);

                eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

                cache_[name] = value;
                return true;
            }
        }
    }

    const char *key = name.c_str();
    size_t i = 0;
    while (mappings[i].name) {
        if (name == mappings[i].name) {
            key = mappings[i].key;
            break;
        }
        i++;
    }

    char buffer[1024];
    size_t size = sizeof(buffer);
    int err = grib_get_string(grib_.get(), key, buffer, &size);

    if (err == GRIB_SUCCESS) {
        value = buffer;
        eckit::Log::info() << "GribInput::get " << name << " is " << value << " (as " << key << ")" << std::endl;
        cache_[name] = value;
        return true;
    }

    if (err != GRIB_NOT_FOUND) {
        GRIB_ERROR(err, name.c_str());
    }

    return false;
}

bool GribInput::handle(grib_handle *h) {
    grib_.reset(h);
    cache_.clear();
    return h != 0;
}
