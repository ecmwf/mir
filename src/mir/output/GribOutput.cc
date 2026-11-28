// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/GribOutput.h"

#include <memory>
#include <ostream>
#include <sstream>

#include "eckit/config/Resource.h"
#include "eckit/log/JSON.h"

#include "mir/action/context/Context.h"
#include "mir/action/io/Copy.h"
#include "mir/action/io/Save.h"
#include "mir/action/io/Set.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/grib/BasicAngle.h"
#include "mir/grib/Compatibility.h"
#include "mir/grib/Config.h"
#include "mir/grib/OutputKeys.h"
#include "mir/grib/Packing.h"
#include "mir/input/MIRInput.h"
#include "mir/key/Area.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Mutex.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir::output {


static util::recursive_mutex local_mutex;


#define X(a) Log::debug() << "  GRIB encoding: " << #a << " = " << (a) << std::endl
#define Y(a) oss << " " << #a << "=" << a


[[noreturn]] void eccodes_assertion(const char* message) {
    throw exception::SeriousBug(message);
}


namespace {


void check_message_is_valid(grib_handle* h) {
    if (grib_check_is_message_valid()) {
        long valid = 1;
        ASSERT(codes_get_long(h, "isMessageValid", &valid) == CODES_SUCCESS);

        if (valid != 1) {
            throw exception::WriteError("GribOutput: invalid GRIB message");
        }
    }
}


/// Geometry encoded by ecCodes from the gridSpec, instead of by GRIB keys; not cached, so it can be switched in-process
bool is_save_with_gridspec(const param::MIRParametrisation& param) {
    if (!eckit::Resource<bool>("$MIR_GRIDSPEC_GRIB_OUTPUT", false)) {
        return false;
    }

    // requests only the GRIB keys of the legacy encoding can honour
    std::string basicAngle = "decimal";
    param.get("basic-angle", basicAngle);

    return basicAngle == "decimal";
}


bool delete_local_definition(const param::MIRParametrisation& param) {
    bool remove = false;
    return param.userParametrisation().get("delete-local-definition", remove) && remove;
}


/// The keys other than geometry and values, of both encodings (the edition dependent ones aren't known yet)
grib::OutputKeys output_keys(const param::MIRParametrisation& param, const data::MIRField& field, size_t i) {
    grib::OutputKeys keys;

    for (const auto& [key, value] : field.metadata(i)) {
        keys.add_after(key, value);
    }

    if (delete_local_definition(param)) {
        keys.add_after("deleteLocalDefinition", 1L);
    }

    return keys;
}


// The packing type (if of the same kind, gridded or spectral, as encoded later) and bits per value of a message
struct EncodedPacking {
    explicit EncodedPacking(grib_handle* h) {
        char buffer[64];
        size_t len = sizeof(buffer);
        GRIB_CALL(codes_get_string(h, "packingType", buffer, &len));
        type_ = buffer;

        GRIB_CALL(codes_get_long(h, "bitsPerValue", &bitsPerValue_));
    }

    void set(grib_handle* h) const {
        auto kind = [](const std::string& type) { return type.substr(0, type.find('_')); };

        char buffer[64];
        size_t len = sizeof(buffer);
        GRIB_CALL(codes_get_string(h, "packingType", buffer, &len));

        if (const std::string type(buffer); type != type_ && kind(type) == kind(type_)) {
            len = type_.length();
            GRIB_CALL(codes_set_string(h, "packingType", type_.c_str(), &len));
        }

        GRIB_CALL(codes_set_long(h, "bitsPerValue", bitsPerValue_));
    }

private:
    std::string type_;
    long bitsPerValue_ = 0;
};


void set_missing_values(grib_handle* h, const data::MIRField& field) {
    GRIB_CALL(codes_set_double(h, "missingValue", field.missingValue()));
    GRIB_CALL(codes_set_long(h, "bitmapPresent", field.hasMissing() ? 1L : 0L));
}


}  // namespace


GribOutput::GribOutput() : interpolated_(0), saved_(0) {}


GribOutput::~GribOutput() = default;


size_t GribOutput::interpolated() const {
    return interpolated_;
}


size_t GribOutput::saved() const {
    return saved_;
}


size_t GribOutput::copy(const param::MIRParametrisation& /*unused*/, context::Context& ctx) {
    saved_++;

    const auto& input = ctx.input();

    size_t total = 0;
    for (size_t i = 0; i < input.dimensions(); i++) {
        auto* h = input.gribHandle(i);  // Base class throws if input cannot provide handle
        ASSERT(h);

        check_message_is_valid(h);

        const void* message = nullptr;
        size_t size         = 0;

        GRIB_CALL(codes_get_message(h, &message, &size));

        out(message, size, false);
        total += size;
    }

    return total;
}


bool GribOutput::printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
    std::unique_ptr<grib::Packing> pack(grib::Packing::build(param));
    ASSERT(pack);

    bool ok = pack->printParametrisation(out);

    std::string compatibility;
    if (param.userParametrisation().get("compatibility", compatibility)) {
        out << (ok ? "," : "") << "compatibility=" << compatibility;
        ok = true;
    }

    if (delete_local_definition(param)) {
        out << (ok ? "," : "") << "delete-local-definition=1";
        ok = true;
    }

    return ok;
}


void GribOutput::jsonParametrisation(eckit::JSON& j, const param::MIRParametrisation& param) const {
    std::unique_ptr<grib::Packing> pack(grib::Packing::build(param));
    ASSERT(pack);
    pack->json(j);

    if (std::string compatibility; param.userParametrisation().get("compatibility", compatibility)) {
        j << "compatibility" << compatibility;
    }

    if (delete_local_definition(param)) {
        j << "delete-local-definition" << true;
    }
}


void GribOutput::prepare(const param::MIRParametrisation& param, action::ActionPlan& plan, MIROutput& output) {
    if (plan.ended()) {
        return;
    }

    auto compatibility_empty = [&param]() {
        std::string compatibility;
        param.userParametrisation().get("compatibility", compatibility);
        return compatibility.empty();
    };

    auto packing_empty = [&param]() {
        std::unique_ptr<grib::Packing> pack(grib::Packing::build(param));
        ASSERT(pack);
        return pack->empty();
    };

    // encoding anew, unless only setting the packing or copying as is
    const auto save = !plan.empty() || !compatibility_empty() || delete_local_definition(param);

    plan.add(save               ? new action::io::Save(param, output)
             : !packing_empty() ? static_cast<action::Action*>(new action::io::Set(param, output))
                                : new action::io::Copy(param, output));
}


bool GribOutput::sameParametrisation(const param::MIRParametrisation& param1,
                                     const param::MIRParametrisation& param2) const {
    std::unique_ptr<grib::Packing> packing1(grib::Packing::build(param1));
    std::unique_ptr<grib::Packing> packing2(grib::Packing::build(param2));

    if (!packing1->sameAs(packing2.get())) {
        return false;
    }

    std::string compatibility1;
    std::string compatibility2;

    param1.userParametrisation().get("compatibility", compatibility1);
    param2.userParametrisation().get("compatibility", compatibility2);

    return compatibility1 == compatibility2 && delete_local_definition(param1) == delete_local_definition(param2);
}


size_t GribOutput::save(const param::MIRParametrisation& param, context::Context& ctx) {
    trace::ResourceUsage usage("GribOutput::save");

    interpolated_++;

    const auto& field = ctx.field();
    const auto& input = ctx.input();

    field.validate();

    size_t total = 0;

    util::MIRStatistics::Timing saveTimer;
    auto timer(ctx.statistics().gribEncodingTimer());

    std::unique_ptr<grib::Packing> pack(grib::Packing::build(param));
    ASSERT(pack);

    for (size_t i = 0; i < field.dimensions(); i++) {

        // Protect ecCodes and set error callback handling (throws)
        util::lock_guard<util::recursive_mutex> lock(local_mutex);
        codes_set_codes_assertion_failed_proc(&eccodes_assertion);

        // Special case where only values are changing; handle is cloned, and new values are set
        if (param.userParametrisation().has("filter")) {

            // Make sure handle deleted even in case of exception
            auto* h = codes_handle_clone(input.gribHandle(i));
            HandleDeleter hf(h);

            long n;
            GRIB_CALL(codes_get_long(h, "numberOfDataPoints", &n));
            if (static_cast<size_t>(n) != field.values(i).size()) {
                throw exception::UserError("Using 'filter' requires preserving the number of points from input");
            }

            set_missing_values(h, field);
            GRIB_CALL(codes_set_double_array(h, "values", field.values(i).data(), field.values(i).size()));

            total += write(h, saveTimer);

            continue;
        }

        if (is_save_with_gridspec(param)) {
            auto* h = codes_handle_clone(input.gribHandle(field.handle(i)));
            HandleDeleter hf(h);

            repres::RepresentationHandle repres(field.representation());

            // the geometry is encoded for the edition it finds, and the packing for the geometry it finds
            pack->setEdition(h);

            // setting the geometry resets the values (a constant field) and so their packing, which is as encoded
            const EncodedPacking encoded(h);

            auto keys = output_keys(param, field, i);
            keys.set_before(h);

            const auto spec = repres->spec().str();
            auto length     = spec.length();
            GRIB_CALL(codes_set_string(h, "gridSpec", spec.c_str(), &length));

            encoded.set(h);

            // the edition as encoded (the geometry can need a higher)
            long edition = 0;
            GRIB_CALL(codes_get_long(h, "edition", &edition));
            grib::Compatibility(param).keys(edition, keys);

            keys.set_after(h);

            set_missing_values(h, field);
            pack->set(repres, h, field.values(i));

            total += write(h, saveTimer);

            continue;
        }

        auto* h = input.gribHandle(field.handle(i));  // Base class throws if input cannot provide handle

        grib_info info;

        // missing values
        if (field.hasMissing() && field.missingValue() == 0.) {
            info.grid.bitmapPresent = 1L;
            info.extra_set("missingValue", 0.);  // ecCodes workaround for missingValue==0
        }
        else {
            info.grid.bitmapPresent = field.hasMissing() ? 1L : 0L;
            info.grid.missingValue  = field.missingValue();
        }

        // Ask representation to update info (as encoded in this edition)
        pack->fillEdition(info);

        repres::RepresentationHandle repres(field.representation());
        repres->fillGrib(info);

        // Packing, accuracy, edition
        pack->fill(repres, info);

        // Basic angle (after representation), support only for gridType=regular_ll
        std::string basicAngle = "decimal";
        param.get("basic-angle", basicAngle);

        if (basicAngle == "as-input") {
            std::vector<long> fraction(2);
            GRIB_CALL(codes_get_long(h, "basicAngleOfTheInitialProductionDomain", &fraction[0]));
            GRIB_CALL(codes_get_long(h, "subdivisionsOfBasicAngle", &fraction[1]));

            grib::BasicAngle basic(fraction[0], fraction[1]);
            basic.fillGrib(info);
        }
        else if (basicAngle == "fraction") {
            grib::BasicAngle basic(info);
            basic.fillGrib(info);
        }
        else {
            // codes_grib_util_set_spec does not need anything here (GRIB standard)
            ASSERT(basicAngle == "decimal");
        }

        // Extra settings (paramId comes from here), the edition as requested, otherwise the input's
        auto keys = output_keys(param, field, i);

        long edition = info.packing.editionNumber;
        if (edition == 0) {
            param.get("edition", edition);
        }
        grib::Compatibility(param).keys(edition, keys);

        keys.fill(info);

        // Give a chance to a sub-class to modify info
        fill(h, info);

        if (Log::debug_active()) {
            auto p(Log::debug().precision(12));
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
            for (long j = 0; j < info.grid.pl_size && j < 4; j++) {
                X(info.grid.pl[j]);
            }

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
            for (long j = 0; j < info.packing.extra_settings_count; j++) {
                X(info.packing.extra_settings[j].name);
                switch (info.packing.extra_settings[j].type) {
                    case CODES_TYPE_LONG:
                        X(info.packing.extra_settings[j].long_value);
                        break;
                    case CODES_TYPE_DOUBLE:
                        X(info.packing.extra_settings[j].double_value);
                        break;
                    case CODES_TYPE_STRING:
                        X(info.packing.extra_settings[j].string_value);
                        break;
                }
            }
            Log::debug().precision(p);
        }


        // ecCodes sets the keys before converting to a higher edition, failing on the keys of that edition only
        std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> converted(nullptr, &codes_handle_delete);
        if (long edition = 0; info.packing.editionNumber > 0 &&
                              codes_get_long(h, "edition", &edition) == CODES_SUCCESS &&
                              edition < info.packing.editionNumber) {
            converted.reset(codes_handle_clone(h));
            ASSERT(converted);
            GRIB_CALL(codes_set_long(converted.get(), "edition", info.packing.editionNumber));
            h = converted.get();
        }

        const auto& values = field.values(i);
        int flags          = 0;
        int err            = 0;

        auto* result =
            codes_grib_util_set_spec(h, &info.grid, &info.packing, flags, values.data(), values.size(), &err);
        HandleDeleter hf(result);  // Make sure handle deleted even in case of exception


        if (err == CODES_WRONG_GRID) {
            std::ostringstream oss;

            oss << "CODES_WRONG_GRID: ";

            Y(info.grid.grid_type);
            Y(info.grid.Ni);
            Y(info.grid.Nj);
            Y(info.grid.iDirectionIncrementInDegrees);
            Y(info.grid.jDirectionIncrementInDegrees);
            Y(info.grid.longitudeOfFirstGridPointInDegrees);
            Y(info.grid.longitudeOfLastGridPointInDegrees);
            Y(info.grid.latitudeOfFirstGridPointInDegrees);
            Y(info.grid.latitudeOfLastGridPointInDegrees);

            throw exception::SeriousBug(oss.str());
        }

        GRIB_CALL(err);

        total += write(result, saveTimer);


        static bool checkArea = eckit::Resource<bool>("$MIR_CHECK_AREA", false);
        if (checkArea) {
            util::BoundingBox user;
            if (key::Area::get(param.userParametrisation(), user)) {

                util::BoundingBox before(
                    info.grid.latitudeOfFirstGridPointInDegrees, info.grid.longitudeOfFirstGridPointInDegrees,
                    info.grid.latitudeOfLastGridPointInDegrees, info.grid.longitudeOfLastGridPointInDegrees);

                // input::GribMemoryInput g(message, size);
                // util::BoundingBox after(g);

                if (user != before /*|| user != after || before != after*/) {
                    Log::info() << "MIR_CHECK_AREA:"
                                << " request=" << user << " result="
                                << before
                                // << " grib=" << after
                                << std::endl;
                }
            }
        }
    }

    ctx.statistics().gribEncodingTiming() -= saveTimer;

    return total;
}


size_t GribOutput::set(const param::MIRParametrisation& param, context::Context& ctx) {
    trace::ResourceUsage usage("GribOutput::set");

    interpolated_++;

    const auto& field = ctx.field();
    const auto& input = ctx.input();

    field.validate();

    size_t total = 0;

    util::MIRStatistics::Timing saveTimer;
    auto timer(ctx.statistics().gribEncodingTimer());

    std::unique_ptr<grib::Packing> pack(grib::Packing::build(param));
    ASSERT(pack);

    ASSERT(field.dimensions() == 1);

    for (size_t i = 0; i < field.dimensions(); i++) {

        // Protect ecCodes and set error callback handling (throws)
        util::lock_guard<util::recursive_mutex> lock(local_mutex);
        codes_set_codes_assertion_failed_proc(&eccodes_assertion);

        // Make sure handle deleted even in case of exception
        auto* h = codes_handle_clone(input.gribHandle(field.handle(i)));
        HandleDeleter hf(h);

        repres::RepresentationHandle repres(field.representation());

        // Packing, accuracy, edition
        set_missing_values(h, field);
        pack->set(repres, h, field.values(i));

        total += write(h, saveTimer);
    }

    ctx.statistics().gribEncodingTiming() -= saveTimer;

    return total;
}


void GribOutput::fill(grib_handle* /*unused*/, grib_info& /*unused*/) const {}


size_t GribOutput::write(grib_handle* h, eckit::Timing& timer) {
    ASSERT(h != nullptr);
    check_message_is_valid(h);

    const void* message = nullptr;
    size_t size         = 0;
    GRIB_CALL(codes_get_message(h, &message, &size));

    GRIB_CALL(codes_check_message_header(message, size, PRODUCT_GRIB));
    GRIB_CALL(codes_check_message_footer(message, size, PRODUCT_GRIB));

    util::MIRStatistics::AutoTiming timing(timer);
    out(message, size, true);

    return size;
}


const grib::Config& GribOutput::config() {
    static const grib::Config cfg(LibMir::configFile(LibMir::config_file::GRIB_OUTPUT), true);
    return cfg;
}


param::MIRParametrisation* GribOutput::make_parametrised_config(const param::MIRParametrisation& param) {
    auto* ptr = new param::CombinedParametrisation(param.userParametrisation(), param.fieldParametrisation(),
                                                   config().find(param));
    ASSERT(ptr != nullptr);
    return ptr;
}


#undef Y
#undef X


}  // namespace mir::output
