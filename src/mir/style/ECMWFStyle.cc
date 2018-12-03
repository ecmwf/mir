/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/style/ECMWFStyle.h"

#include <iostream>
#include <string>
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/types/FloatCompare.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRJob.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/style/Resol.h"
#include "mir/util/DeprecatedFunctionality.h"
#include "mir/util/Wind.h"


namespace mir {
namespace style {


namespace {

static MIRStyleBuilder<ECMWFStyle> __style("ecmwf");

struct DeprecatedStyle : ECMWFStyle, util::DeprecatedFunctionality {
    DeprecatedStyle(const param::MIRParametrisation& p) : ECMWFStyle(p), util::DeprecatedFunctionality("style 'dissemination' now known as 'ecmwf'") {}
};

static MIRStyleBuilder<DeprecatedStyle> __deprecated_style("dissemination");


struct KnownKey {

    KnownKey(const char* _key, const char* _target="", const bool supportsRotation=true) : key_(_key), target_(_target), supportsRotation_(supportsRotation) {}
    KnownKey(const KnownKey&) = delete;
    virtual ~KnownKey() = default;

    virtual bool sameKey(const param::MIRParametrisation&, const param::MIRParametrisation&) const = 0;
    virtual bool sameValue(const param::MIRParametrisation&, const param::MIRParametrisation&) const = 0;

    const std::string& key() {
        ASSERT(!key_.empty());
        return key_;
    }

    const std::string& target() {
        ASSERT(!target_.empty());
        return target_;
    }

    bool supportsRotation() const {
        return supportsRotation_;
    }

protected:
    const std::string key_;
    const std::string target_;
    const bool supportsRotation_;
};


struct Points : KnownKey {
    Points(const char* key) : KnownKey(key, "points", false) {}
    bool sameKey(const param::MIRParametrisation& p1, const param::MIRParametrisation&) const {
        return p1.has(key_);
    }
    bool sameValue(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
        return false;
    }
};


template< typename T >
struct KnownKeyT : KnownKey {
    KnownKeyT(const char* key, const char* target="", const bool supportsRotation=true) : KnownKey(key, target, supportsRotation) {}
    bool sameKey(const param::MIRParametrisation& p1, const param::MIRParametrisation& p2) const {
        return p1.has(key_) == p2.has(key_);
    }
    bool sameValue(const param::MIRParametrisation& p1, const param::MIRParametrisation& p2) const {

        T value1;
        T value2;
        ASSERT(p1.get(key_, value1));
        ASSERT(p2.get(key_, value2));

        return value1 == value2;
    }
};


template<>
bool KnownKeyT< double >::sameValue(const param::MIRParametrisation& p1, const param::MIRParametrisation& p2) const {

    double value1;
    double value2;
    ASSERT(p1.get(key_, value1));
    ASSERT(p2.get(key_, value2));

    return eckit::types::is_approximately_equal(value1, value2);
};


template<>
bool KnownKeyT< std::vector<double> >::sameValue(const param::MIRParametrisation& p1, const param::MIRParametrisation& p2) const {

    std::vector<double> value1;
    std::vector<double> value2;
    ASSERT(p1.get(key_, value1));
    ASSERT(p2.get(key_, value2));

    if (value1.size() != value2.size()) {
        return false;
    }

    for (auto v1 = value1.cbegin(), v2 = value2.cbegin(); v1 != value1.cend(); ++v1, ++v2) {
        if (!eckit::types::is_approximately_equal(*v1, *v2)) {
            return false;
        }
    }
    return true;
}


template< typename T >
struct KnownMultiKeyT : KnownKey {

    KnownMultiKeyT(const char* key, const char* fieldKey1, const char* fieldKey2, const char* target="", const bool supportsRotation=true) :
        KnownKey(key, target, supportsRotation),
        fieldKey1_(fieldKey1),
        fieldKey2_(fieldKey2) {
    }

    bool sameKey(const param::MIRParametrisation& p1, const param::MIRParametrisation& p2) const {
        return p1.has(key_) == p2.has(fieldKey1_) && p2.has(fieldKey2_);
    }

    bool sameValue(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
        std::ostringstream os;
        os << "KnownMultiKeyT<T>::sameValue() not implemented";
        throw eckit::SeriousBug(os.str());
    }

private:
    const std::string fieldKey1_;
    const std::string fieldKey2_;
};


template<>
bool KnownMultiKeyT< std::vector<double> >::sameValue(const param::MIRParametrisation& p1, const param::MIRParametrisation& p2) const {

    std::vector<double> value1;
    std::vector<double> value2(2);

    if(!p1.get(key_, value1)) {
        std::ostringstream oss;
        oss << "KnownMultiKeyT<std::vector<double>> cannot get key=" << key_;
        throw eckit::SeriousBug(oss.str());
    }

    if(!p2.get(fieldKey1_, value2[0])) {
        return false;
    }

    if(!p2.get(fieldKey2_, value2[1])) {
        return false;
    }


    if (value1.size() != value2.size()) {
        return false;
    }

    for (auto v1 = value1.cbegin(), v2 = value2.cbegin(); v1 != value1.cend(); ++v1, ++v2) {
        if (!eckit::types::is_approximately_equal(*v1, *v2)) {
            return false;
        }
    }
    return true;
}


static std::string target_gridded_from_parametrisation(const param::MIRParametrisation& parametrisation, bool checkRotation) {
    static const std::vector< KnownKey* > keys_targets = {
        new KnownMultiKeyT< std::vector<double> > ("grid", "west_east_increment", "south_north_increment", "regular-ll"),
        new KnownKeyT< size_t >            ("reduced",    "reduced-gg"),
        new KnownKeyT< size_t >            ("regular",    "regular-gg"),
        new KnownKeyT< size_t >            ("octahedral", "octahedral-gg"),
        new KnownKeyT< std::vector<long> > ("pl",         "reduced-gg-pl-given"),
        new KnownKeyT< std::string >       ("gridname",   "namedgrid"),
        new KnownKeyT< std::string >       ("griddef",    "griddef", false),
        new Points("latitudes"),
        new Points("longitudes"),
    };

    static const KnownMultiKeyT< std::vector<double> > south_pole("rotation", "south_pole_latitude", "south_pole_longitude");

    const param::MIRParametrisation& user = parametrisation.userParametrisation();
    const param::MIRParametrisation& field = parametrisation.fieldParametrisation();

    for (const auto& kt : keys_targets) {
        if (user.has(kt->key())) {

            // If user and field parametrisation have the same target_ key, and
            // its value is the same (and rotation, optionally) there's nothing to do
            if (!kt->sameKey(user, field) || !kt->sameValue(user, field)) {
                if (user.has("rotation") && !kt->supportsRotation()) {
                    throw eckit::UserError("ECMWFStyle: option 'rotation' is incompatible with '" + kt->target() + "'");
                }
                return (user.has("rotation") ? "rotated-" : "") + kt->target();
            }

            if (checkRotation && !south_pole.sameValue(user, field)) {
                return kt->target();
            }

            return "";
        }
    }

    eckit::Log::warning() << "ECMWFStyle: could not determine target from parametrisation" << std::endl;
    return "";
}

}  // (anonymous namespace)


ECMWFStyle::ECMWFStyle(const param::MIRParametrisation& parametrisation):
    MIRStyle(parametrisation) {
}


ECMWFStyle::~ECMWFStyle() = default;


void ECMWFStyle::prologue(action::ActionPlan& plan) const {

    std::string prologue;
    if (parametrisation_.get("prologue", prologue)) {
        plan.add(prologue);
    }

    if (parametrisation_.has("checkerboard")) {
        plan.add("misc.checkerboard");
    }

    if (parametrisation_.has("pattern")) {
        plan.add("misc.pattern");
    }

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.prologue", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.prologue.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }
}


void ECMWFStyle::sh2grid(action::ActionPlan& plan) const {

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.spectral", formula) ||
            parametrisation_.userParametrisation().get("formula.raw", formula)
       ) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.spectral.metadata", metadata);
        parametrisation_.userParametrisation().get("formula.raw.metadata", metadata);

        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    Resol resol(parametrisation_);

    bool rotation = parametrisation_.userParametrisation().has("rotation");

    bool vod2uv = false;
    bool uv2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);
    parametrisation_.userParametrisation().get("uv2uv", uv2uv);

    // completed later
    const std::string transform = "transform." + std::string(vod2uv ? "sh-vod-to-uv-" : "sh-scalar-to-");
    const std::string interpolate = "interpolate.grid2";
    const std::string target = target_gridded_from_parametrisation(parametrisation_, false);

    if (resol.resultIsSpectral()) {
        resol.prepare(plan);
    }

    if (!target.empty()) {
        if (resol.resultIsSpectral()) {

            plan.add(transform + target);

        } else {

            resol.prepare(plan);

            // if the intermediate grid is the same as the target grid, the interpolation to the
            // intermediate grid  is not followed by an additional interpolation
            std::string gridname;
            if (rotation || !parametrisation_.userParametrisation().get("gridname", gridname) || gridname != resol.gridname()) {
                plan.add(interpolate + target);
            }

        }

        if (vod2uv || uv2uv) {
            ASSERT(vod2uv != uv2uv);

            if (uv2uv) {
                plan.add("filter.adjust-winds-scale-cos-latitude");
            }

            if (rotation) {
                plan.add("filter.adjust-winds-directions");
            }
        }
    }

    if (parametrisation_.userParametrisation().get("formula.gridded", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.gridded.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }
}


void ECMWFStyle::sh2sh(action::ActionPlan& plan) const {

    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("intgrid", "none");

    Resol resol(runtime);
    eckit::Log::debug<LibMir>() << "ECMWFStyle: resol=" << resol << std::endl;

    // the runtime parametrisation above is needed to satisfy this assertion
    ASSERT(resol.resultIsSpectral());
    resol.prepare(plan);

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.spectral", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.spectral.metadata", metadata);

        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    bool vod2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.sh-vod-to-UV");
    }
}


void ECMWFStyle::grid2grid(action::ActionPlan& plan) const {

    bool rotation = parametrisation_.userParametrisation().has("rotation");

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.gridded", formula) ||
            parametrisation_.userParametrisation().get("formula.raw", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.gridded.metadata", metadata);
        parametrisation_.userParametrisation().get("formula.raw.metadata", metadata);

        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    bool vod2uv = false;
    bool uv2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);
    parametrisation_.userParametrisation().get("uv2uv", uv2uv);

    // completed later
    const std::string interpolate = "interpolate.grid2";
    const std::string target = target_gridded_from_parametrisation(parametrisation_, rotation);

    if (!target.empty()) {
        plan.add(interpolate + target);

        if (vod2uv || uv2uv) {
            ASSERT(vod2uv != uv2uv);

            if (rotation) {
                plan.add("filter.adjust-winds-directions");
            }
        }
    }
}


void ECMWFStyle::epilogue(action::ActionPlan& plan) const {
    auto& user = parametrisation_.userParametrisation();

    bool vod2uv = false;
    bool uv2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);
    parametrisation_.userParametrisation().get("uv2uv", uv2uv);

    if (vod2uv || uv2uv) {
        ASSERT(vod2uv != uv2uv);

        bool u_only = false;
        user.get("u-only", u_only);

        bool v_only = false;
        user.get("v-only", v_only);

        if (u_only) {
            ASSERT(!v_only);
            plan.add("select.field", "which", long(0));
        }

        if (v_only) {
            ASSERT(!u_only);
            plan.add("select.field", "which", long(1));
        }
    }

    std::string formula;
    if (user.get("formula.epilogue", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        user.get("formula.epilogue.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    std::string metadata;
    if (user.get("metadata", metadata)) {
        plan.add("set.metadata", "metadata", metadata);
    }

    std::string epilogue;
    if (parametrisation_.get("epilogue", epilogue)) {
        plan.add(epilogue);
    }
}


void ECMWFStyle::print(std::ostream& out) const {
    out << "ECMWFStyle[]";
}


void ECMWFStyle::prepare(action::ActionPlan& plan) const {

    // All the nasty logic goes there
    prologue(plan);

    size_t user_wants_gridded = 0;

    if (parametrisation_.userParametrisation().has("grid")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("reduced")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("regular")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("octahedral")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("pl")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("gridname")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("griddef")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("latitudes") ||
        parametrisation_.userParametrisation().has("longitudes")) {
        user_wants_gridded++;
    }

    ASSERT(user_wants_gridded <= 1);

    bool field_gridded  = parametrisation_.fieldParametrisation().has("gridded");
    bool field_spectral = parametrisation_.fieldParametrisation().has("spectral");

    ASSERT(field_gridded != field_spectral);


    if (field_spectral) {
        if (user_wants_gridded) {
            sh2grid(plan);
        } else {
            // "user wants spectral"
            sh2sh(plan);
        }
    }


    if (field_gridded) {

        std::string formula;
        if (parametrisation_.userParametrisation().get("formula.gridded", formula)) {
            std::string metadata;
            // paramId for the results of formulas
            parametrisation_.userParametrisation().get("formula.gridded.metadata", metadata);
            parametrisation_.userParametrisation().get("formula.raw.metadata", metadata);

            plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
        }
        grid2grid(plan);
    }


    if (field_gridded || user_wants_gridded) {

        bool globalise = false;
        parametrisation_.userParametrisation().get("globalise", globalise);

        if (globalise) {
            plan.add("filter.globalise");
        }

        if (parametrisation_.userParametrisation().has("area")) {
            plan.add("crop.area");
        }

        if (parametrisation_.userParametrisation().has("bitmap")) {
            plan.add("filter.bitmap");
        }

        if (parametrisation_.userParametrisation().has("frame")) {
            plan.add("filter.frame");
        }

    }


    epilogue(plan);
}


}  // namespace style
}  // namespace mir

