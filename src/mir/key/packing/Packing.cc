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


#include "mir/key/packing/Packing.h"

#include <map>
#include <ostream>
#include <set>

#include "eckit/config/Resource.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::key::packing {


static util::once_flag once;
static util::recursive_mutex* local_mutex         = nullptr;
static std::map<std::string, PackingFactory*>* ms = nullptr;
static std::map<std::string, PackingFactory*>* mg = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    ms          = new std::map<std::string, PackingFactory*>();
    mg          = new std::map<std::string, PackingFactory*>();
}


Packing::Packing(const std::string& name, const param::MIRParametrisation& param) :
    accuracy_(0),
    edition_(0),
    gridded_(param.userParametrisation().has("grid") || param.fieldParametrisation().has("gridded")) {
    const auto& user  = param.userParametrisation();
    const auto& field = param.fieldParametrisation();

    ASSERT(!name.empty());
    packing_ = name;
    std::string packing;

    bool gridded = false;
    field.get("gridded", gridded);

    definePacking_               = !field.get("packing", packing) || packing_ != packing || gridded_ != gridded;
    defineAccuracyBeforePacking_ = definePacking_ && packing == "ieee";

    defineAccuracy_ = false;
    if (defineAccuracyBeforePacking_) {
        ASSERT(param.get("accuracy", accuracy_));
        defineAccuracy_ = true;
    }
    else if (user.get("accuracy", accuracy_)) {
        long accuracy   = 0;
        defineAccuracy_ = !field.get("accuracy", accuracy) || accuracy_ != accuracy;
    }

    defineEdition_ = false;
    if (user.get("edition", edition_)) {
        long edition   = 0;
        defineEdition_ = !field.get("edition", edition) || edition_ != edition;
    }
}


Packing::~Packing() = default;


bool Packing::sameAs(const Packing* other) const {
    if (definePacking_ != other->definePacking_ || defineAccuracy_ != other->defineAccuracy_ ||
        defineEdition_ != other->defineEdition_) {
        return false;
    }
    bool samePacking  = !definePacking_ || packing_ == other->packing_;
    bool sameAccuracy = !defineAccuracy_ || accuracy_ == other->accuracy_;
    bool sameEdition  = !defineEdition_ || edition_ == other->edition_;
    return samePacking && sameAccuracy && sameEdition;
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

    if (defineAccuracy_) {
        out << sep << "accuracy=" << accuracy_;
        sep = ",";
    }

    return !sep.empty();
}


bool Packing::empty() const {
    return !definePacking_ && !defineAccuracy_ && !defineEdition_;
}


void Packing::fill(grib_info& info, long pack) const {
    info.packing.packing  = CODES_UTIL_PACKING_SAME_AS_INPUT;
    info.packing.accuracy = CODES_UTIL_ACCURACY_SAME_BITS_PER_VALUES_AS_INPUT;
    // (Representation can set edition, so it isn't reset)

    if (definePacking_) {
        info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
        info.packing.packing_type = pack;
    }

    if (defineAccuracy_) {
        info.packing.accuracy     = CODES_UTIL_ACCURACY_USE_PROVIDED_BITS_PER_VALUES;
        info.packing.bitsPerValue = accuracy_;
    }

    if (defineEdition_) {
        info.packing.editionNumber = edition_;
    }
}


void Packing::set(grib_handle* h, const std::string& type) const {
    // Note: order is important, it is not applicable to all packing's.

    if (defineEdition_) {
        GRIB_CALL(codes_set_long(h, "edition", edition_));
    }

    if (defineAccuracyBeforePacking_) {
        GRIB_CALL(codes_set_long(h, "bitsPerValue", accuracy_));
    }

    if (definePacking_) {
        auto len = type.length();
        GRIB_CALL(codes_set_string(h, "packingType", type.c_str(), &len));
    }

    if (defineAccuracy_) {
        GRIB_CALL(codes_set_long(h, "bitsPerValue", accuracy_));
    }
}


PackingFactory::PackingFactory(const std::string& name, const std::string& alias, bool spectral, bool gridded) :
    name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(gridded || spectral);

    if (gridded) {
        ASSERT_MSG(mg->insert({name, this}).second, "PackingFactory: duplicate gridded packing");
        if (!alias.empty()) {
            ASSERT_MSG(mg->insert({alias, this}).second, "PackingFactory: duplicate gridded packing");
        }
    }

    if (spectral) {
        ASSERT_MSG(ms->insert({name, this}).second, "PackingFactory: duplicate spectral packing");
        if (!alias.empty()) {
            ASSERT_MSG(ms->insert({alias, this}).second, "PackingFactory: duplicate spectral packing");
        }
    }
}


PackingFactory::~PackingFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    mg->erase(name_);
    ms->erase(name_);
}


Packing* PackingFactory::build(const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    long edition = 2;
    param.get("edition", edition);

    std::string default_spectral = "complex";
    std::string default_gridded  = "ccsds";
    param.get("default-spectral-packing", default_spectral);
    param.get(edition <= 1 ? "default-grib1-gridded-packing" : "default-grib2-gridded-packing", default_gridded);

    const auto& user  = param.userParametrisation();
    const auto& field = param.fieldParametrisation();


    // When converting from spectral to gridded...
    auto name = user.has("grid") && field.has("spectral") ? default_gridded : "av";
    user.get("packing", name);


    // When converting formats, field packing needs a sensible default
    std::string packing = field.has("spectral") ? default_spectral : default_gridded;
    field.get("packing", packing);


    bool av       = name == "av" || name == "archived-value";
    bool gridded  = user.has("grid") || field.has("gridded");
    std::string t = gridded ? "gridded" : "spectral";
    const auto& m = gridded ? *mg : *ms;


    // In case of packing=av, try instantiating specific packing
    if (av) {
        auto j = m.find(packing);
        if (j != m.end()) {
            return j->second->make(packing, param);
        }
    }

    auto j = m.find(name);
    if (j != m.end()) {
        return j->second->make(av ? packing : j->second->name_, param);
    }

    Log::error() << "PackingFactory: unknown " << t << " packing '" << name << "', choices are: ";
    for (const auto& j : m) {
        Log::error() << ", " << j.first;
    }

    throw exception::SeriousBug("PackingFactory: unknown " + t + " packing '" + name + "'");
}


void PackingFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::set<std::string> p;
    for (const auto& j : *ms) {
        p.insert(j.first);
    }
    for (const auto& j : *mg) {
        p.insert(j.first);
    }

    const char* sep = "";
    for (const auto& j : p) {
        out << sep << j;
        sep = ", ";
    }
}


class ArchivedValue : public Packing {
public:
    // -- Constructors

    ArchivedValue(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        ASSERT(!definePacking_);
    }

private:
    // -- Methods

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, 0 /* dummy, protected by ASSERT */);
    }

    void set(const repres::Representation*, grib_handle* handle) const override {
        Packing::set(handle, "" /* dummy, protected by ASSERT */);
    }
};


class CCSDS : public Packing {
public:
    // -- Constructors

    CCSDS(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        if (!gridded()) {
            std::string msg = "packing=ccsds: only supports gridded fields";
            Log::error() << msg << std::endl;
            throw exception::UserError(msg);
        }

        long required = 2;
        long edition  = 0;
        if (param.get("edition", edition) && edition != required) {
            static const bool grib_edition_conversion_default =
                eckit::Resource<bool>("$MIR_GRIB_EDITION_CONVERSION;mirGribEditionConversion", false);
            bool grib_edition_conversion = grib_edition_conversion_default;
            param.get("grib-edition-conversion", grib_edition_conversion);

            if (!grib_edition_conversion) {
                throw exception::UserError("Packing: edition conversion is required, but disabled");
            }
        }

        if (edition != required) {
            edition_       = required;
            defineEdition_ = true;
        }
    }

private:
    // -- Overridden methods

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, CODES_UTIL_PACKING_TYPE_CCSDS);
    }
    void set(const repres::Representation*, grib_handle* handle) const override { Packing::set(handle, "grid_ccsds"); }
};


class Complex : public Packing {
public:
    // -- Constructors

    Complex(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        ASSERT(!gridded());
    }

private:
    // -- Overridden methods

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
    }

    void set(const repres::Representation*, grib_handle* handle) const override {
        Packing::set(handle, "spectral_complex");
    }
};


class IEEE : public Packing {
public:
    // -- Constructors

    IEEE(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
        const auto& user  = param.userParametrisation();
        const auto& field = param.fieldParametrisation();

        constexpr long L32  = 32;
        constexpr long L64  = 64;
        constexpr long L128 = 128;

        // Accuracy set by user, otherwise by field (rounded up to a supported precision)
        long bits = L32;
        field.get("accuracy", bits);

        if (!user.get("accuracy", accuracy_)) {
            accuracy_ = bits <= L32 ? L32 : bits <= L64 ? L64 : L128;
        }

        definePrecision_ = accuracy_ != bits || definePacking_ || !field.has("accuracy");
        precision_       = accuracy_ == L32 ? 1 : accuracy_ == L64 ? 2 : accuracy_ == L128 ? 3 : 0;

        if (precision_ == 0) {
            std::string msg = "packing=ieee: only supports accuracy=32, 64 and 128";
            Log::error() << msg << std::endl;
            throw exception::UserError(msg);
        }
    }

private:
    // -- Members

    long precision_;
    bool definePrecision_;

    // -- Overridden methods

    void fill(const repres::Representation*, grib_info& info) const override {
        info.packing.packing = CODES_UTIL_PACKING_SAME_AS_INPUT;
        // (Representation can set edition, so it isn't reset)

        if (definePacking_) {
            info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
            info.packing.packing_type = CODES_UTIL_PACKING_TYPE_IEEE;
        }

        if (defineEdition_) {
            info.packing.editionNumber = edition_;
        }

        if (definePrecision_) {
            info.extra_set("precision", precision_);
        }
    }
    void set(const repres::Representation*, grib_handle* handle) const override {
        Packing::set(handle, gridded() ? "grid_ieee" : "spectral_ieee");

        if (definePrecision_) {
            GRIB_CALL(codes_set_long(handle, "precision", precision_));
        }
    }
    bool printParametrisation(std::ostream& out) const override {
        const auto* sep = Packing::printParametrisation(out) ? "," : "";
        if (definePrecision_) {
            out << sep << "precision=" << precision_;
        }
        return true;
    }
    bool empty() const override { return Packing::empty() && !definePrecision_; }
};


class Simple : public Packing {
public:
    // -- Constructors

    using Packing::Packing;

private:
    // -- Overridden methods

    void fill(const repres::Representation*, grib_info& info) const override {
        Packing::fill(info, gridded() ? CODES_UTIL_PACKING_TYPE_GRID_SIMPLE : CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE);
    }
    void set(const repres::Representation*, grib_handle* handle) const override {
        Packing::set(handle, gridded() ? "grid_simple" : "spectral_simple");
    }

    friend class SecondOrder;
};


class SecondOrder : public Packing {
public:
    // -- Constructors

    SecondOrder(const std::string& name, const param::MIRParametrisation& param) :
        Packing(name, param), simple_(name, param) {}

private:
    // -- Members

    Simple simple_;

    // -- Methods

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

    // -- Overridden methods

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


static const PackingBuilder<ArchivedValue> __packing1("archived-value", "av", true, true);
static const PackingBuilder<CCSDS> __packing2("ccsds", false, true);
static const PackingBuilder<Complex> __packing3("complex", "co", true, false);
static const PackingBuilder<IEEE> __packing4("ieee", true, true);
static const PackingBuilder<SecondOrder> __packing5("second-order", "so", false, true);
static const PackingBuilder<Simple> __packing6("simple", true, true);


}  // namespace mir::key::packing
