/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/grib/Packing.h"

#include <memory>
#include <ostream>

#include "eckit/filesystem/PathName.h"

#include "mir/config/LibMir.h"
#include "mir/grib/Config.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::grib {


void check(bool ok, const std::string& message) {
    if (!ok) {
        Log::error() << message << std::endl;
        throw exception::UserError(message);
    }
};


Packing::Packing(const std::string& name, const param::MIRParametrisation& param) :
    bitsPerValue_(0),
    precision_(0),
    definePrecision_(false),
    gridded_(param.userParametrisation().has("grid") || param.fieldParametrisation().has("gridded")) {
    const auto& user  = param.userParametrisation();
    const auto& field = param.fieldParametrisation();

    ASSERT(!name.empty());
    packing_ = name;
    std::string packing;

    definePacking_ = !field.get("packing", packing) || packing_ != packing || gridded_ != field.has("gridded");
    defineBitsPerValueBeforePacking_ = definePacking_ && packing == "ieee";

    defineBitsPerValue_ = false;
    if (defineBitsPerValueBeforePacking_) {
        ASSERT(param.get("accuracy", bitsPerValue_));
        defineBitsPerValue_ = true;
    }
    else if (user.get("accuracy", bitsPerValue_)) {
        long accuracy       = 0;
        defineBitsPerValue_ = !field.get("accuracy", accuracy) || bitsPerValue_ != accuracy;
    }

    long edition = 0;
    param.get("edition", edition_ = field.get("edition", edition) ? 0 : 2);

    defineEdition_ = edition_ > 0 && edition_ != edition;
}


bool Packing::sameAs(const Packing* other) const {
    if (definePacking_ != other->definePacking_ || defineBitsPerValue_ != other->defineBitsPerValue_ ||
        defineEdition_ != other->defineEdition_ || definePrecision_ != other->definePrecision_) {
        return false;
    }

    bool samePacking   = !definePacking_ || packing_ == other->packing_;
    bool sameAccuracy  = !defineBitsPerValue_ || bitsPerValue_ == other->bitsPerValue_;
    bool sameEdition   = !defineEdition_ || edition_ == other->edition_;
    bool sameprecision = !definePrecision_ || precision_ == other->precision_;

    return samePacking && sameAccuracy && sameEdition && sameprecision;
}


bool Packing::printParametrisation(std::ostream& out) const {
    std::string sep;

    if (definePacking_) {
        out << sep << "packing=" << packing_;
        sep = ",";
    }

    if (defineEdition_) {
        out << sep << "edition=" << edition_;
        sep = ",";
    }

    if (defineBitsPerValue_) {
        out << sep << "accuracy=" << bitsPerValue_;
        sep = ",";
    }

    if (definePrecision_) {
        out << sep << "precision=" << precision_;
        sep = ",";
    }

    return !sep.empty();
}


bool Packing::empty() const {
    return !definePacking_ && !defineBitsPerValue_ && !defineEdition_ && !definePrecision_;
}


void Packing::fill(grib_info& info, long pack) const {
    info.packing.packing  = CODES_UTIL_PACKING_SAME_AS_INPUT;
    info.packing.accuracy = CODES_UTIL_ACCURACY_SAME_BITS_PER_VALUES_AS_INPUT;
    // (Representation can set edition, so it isn't reset)

    if (definePacking_) {
        info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
        info.packing.packing_type = pack;
    }

    if (defineBitsPerValue_ && !definePrecision_) {
        info.packing.accuracy     = CODES_UTIL_ACCURACY_USE_PROVIDED_BITS_PER_VALUES;
        info.packing.bitsPerValue = bitsPerValue_;
    }

    if (defineEdition_) {
        info.packing.editionNumber = edition_;
    }

    if (definePrecision_) {
        info.extra_set("precision", precision_);
    }
}


void Packing::set(grib_handle* h, const std::string& type) const {
    // Note: order is important, it is not applicable to all packing's.

    if (defineEdition_) {
        GRIB_CALL(codes_set_long(h, "edition", edition_));
    }

    if (defineBitsPerValueBeforePacking_) {
        GRIB_CALL(codes_set_long(h, "bitsPerValue", bitsPerValue_));
    }

    if (definePacking_) {
        auto len = type.length();
        GRIB_CALL(codes_set_string(h, "packingType", type.c_str(), &len));
    }

    if (definePrecision_) {
        GRIB_CALL(codes_set_long(h, "precision", precision_));
    }
    else if (defineBitsPerValue_) {
        GRIB_CALL(codes_set_long(h, "bitsPerValue", bitsPerValue_));
    }
}


namespace packing {


struct ArchivedValue : Packing {
    ArchivedValue(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        ASSERT(!definePacking_);
    }

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, CODES_UTIL_PACKING_SAME_AS_INPUT);
    }

    void set(const repres::Representation*, grib_handle* handle) const override { Packing::set(handle, ""); }
};


struct CCSDS : Packing {
    CCSDS(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        edition_       = 2;
        long edition   = 0;
        defineEdition_ = !param.fieldParametrisation().get("edition", edition) || edition_ != edition;
    }

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, CODES_UTIL_PACKING_TYPE_CCSDS);
    }

    void set(const repres::Representation*, grib_handle* handle) const override { Packing::set(handle, "grid_ccsds"); }
};


struct Complex : Packing {
    using Packing::Packing;

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
    }

    void set(const repres::Representation*, grib_handle* handle) const override {
        Packing::set(handle, "spectral_complex");
    }
};


struct IEEE : Packing {
    IEEE(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        constexpr long L32  = 32;
        constexpr long L64  = 64;
        constexpr long L128 = 128;

        // Accuracy set by user, otherwise by field (rounded up to a supported precision)
        long bits = L32;
        param.fieldParametrisation().get("accuracy", bits);

        if (!param.userParametrisation().get("accuracy", bitsPerValue_)) {
            bitsPerValue_ = bits <= L32 ? L32 : bits <= L64 ? L64 : L128;
        }

        definePrecision_ = bitsPerValue_ != bits || definePacking_ || !param.fieldParametrisation().has("accuracy");
        precision_       = bitsPerValue_ == L32 ? 1 : bitsPerValue_ == L64 ? 2 : bitsPerValue_ == L128 ? 3 : 0;

        if (precision_ == 0) {
            std::string msg = "packing=ieee: only supports accuracy=32, 64 and 128";
            Log::error() << msg << std::endl;
            throw exception::UserError(msg);
        }
    }

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, CODES_UTIL_PACKING_TYPE_IEEE);
    }

    void set(const repres::Representation*, grib_handle* handle) const override {
        Packing::set(handle, gridded() ? "grid_ieee" : "spectral_ieee");
    }
};


struct Simple : Packing {
    Simple(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        if (!gridded()) {
            Log::warning() << "packing=simple: only supports gridded data" << std::endl;
        }
    }

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, gridded() ? CODES_UTIL_PACKING_TYPE_GRID_SIMPLE : CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE);
    }

    void set(const repres::Representation*, grib_handle* handle) const override {
        Packing::set(handle, gridded() ? "grid_simple" : "spectral_simple");
    }
};


struct SecondOrder : Packing {
    SecondOrder(const std::string& name, const param::MIRParametrisation& param) :
        Packing(name, param), simple_(name, param) {}

    Simple simple_;

    static bool check(const repres::Representation* repres) {
        ASSERT(repres != nullptr);

        auto n = repres->numberOfPoints();
        if (n < 4) {
            Log::warning() << "packing=second-order: does not support less than 4 values, using packing=simple"
                           << std::endl;
            return false;
        }
        return true;
    }

    void fill(const repres::Representation* repres, grib_info& info) const override {
        if (!check(repres)) {
            simple_.fill(repres, info);
            return;
        }

        Packing::fill(info, CODES_UTIL_PACKING_TYPE_GRID_SECOND_ORDER);
    }

    void set(const repres::Representation* repres, grib_handle* handle) const override {
        if (!check(repres)) {
            simple_.set(repres, handle);
            return;
        }

        Packing::set(handle, "grid_second_order");
    }
};


}  // namespace packing


Packing* Packing::build(const param::MIRParametrisation& param) {
    const auto& user  = param.userParametrisation();
    const auto& field = param.fieldParametrisation();


    // Defaults
    long edition = 2;
    param.get("edition", edition);

    static const grib::Config config(LibMir::configFile(LibMir::config_file::GRIB_OUTPUT), true);
    std::unique_ptr<param::MIRParametrisation> grib_config(
        new param::CombinedParametrisation(user, field, config.find(param)));


    // Check edition conversion
    bool edition_conversion = false;
    grib_config->get("grib-edition-conversion", edition_conversion);

    if (!edition_conversion && !user.has("edition")) {
        long field_edition = 0;
        field.get("edition", field_edition);
        check(field_edition == 0 || field_edition == edition, "GRIB edition conversion is disabled)");
    }


    // Packing
    std::string packing_spectral = "complex";
    std::string packing_gridded  = "ccsds";
    bool packing_always_set      = false;
    grib_config->get("grib-packing-gridded", packing_gridded);
    grib_config->get("grib-packing-spectral", packing_spectral);
    grib_config->get("grib-packing-always-set", packing_always_set);

    ASSERT(field.has("spectral") != field.has("gridded"));
    auto gridded = user.has("grid") || (field.has("gridded"));
    auto packing = packing_always_set                          ? (gridded ? packing_gridded : packing_spectral)
                   : field.has("spectral") && user.has("grid") ? packing_gridded
                                                               : "av";
    user.get("packing", packing);


    // Aliasing
    auto av = packing == "av" || packing == "archived-value";
    if (av) {
        packing = field.has("spectral") ? packing_spectral : packing_gridded;
        field.get("packing", packing);
    }
    else if (packing == "co") {
        packing = "complex";
    }
    else if (packing == "so") {
        packing = "second-order";
    }


    // Instantiate packing method
    if (packing == "ccsds") {
        check(edition_conversion || edition == 2, "GRIB packing=ccsds requires edition conversion (disabled)");
        check(gridded, "GRIB packing=ccsds requires gridded data");
        return new packing::CCSDS(packing, param);
    }

    if (packing == "complex") {
        check(!gridded, "GRIB packing=complex requires spectral data");
        return new packing::Complex(packing, param);
    }

    if (packing == "ieee") {
        return new packing::IEEE(packing, param);
    }

    if (packing.compare(0, 12, "second-order") == 0) {
        check(gridded, "GRIB packing=second-order requires gridded data");
        return new packing::SecondOrder(packing, param);
    }

    if (packing.compare(0, 6, "simple") == 0) {
        return new packing::Simple(packing, param);
    }

    if (av) {
        return new packing::ArchivedValue(packing, param);
    }


    list(Log::error() << "Packing: unknown packing '" << packing << "', choices are: ");
    throw exception::UserError("Packing: unknown packing '" + packing + "'");
}


void Packing::list(std::ostream& out) {
    out << "archived-value, av, ccsds, co, complex, ieee, second-order, simple, so" << std::endl;
}


}  // namespace mir::grib
