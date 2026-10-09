// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/param/FieldParametrisation.h"

#include <sstream>

#include "eckit/geo/Grid.h"
#include "eckit/spec/Spec.h"
#include "eckit/utils/StringTools.h"

#include "mir/config/LibMir.h"
#include "mir/grib/Config.h"
#include "mir/param/Rules.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
// #include "mir/util/Log.h"


namespace mir::param {


namespace detail {


static const SimpleParametrisation EMPTY;


class FieldInfo {
public:
    FieldInfo() = default;

    explicit FieldInfo(const MIRParametrisation& field) {
        static const Rules param_rules;
        if (const auto* rules = param_rules.find(field); rules != nullptr) {
            param_ = rules;
        }

        if (std::string type; field.get("gridType", type) && !type.empty()) {
            static const grib::Config grid_type_rules(LibMir::configFile(LibMir::config_file::GRID_TYPE), true);

            if (std::string uid; field.get("uid", uid) && eckit::geo::GridSpecByUID::instance().exists(uid)) {
                catalog_.reset(eckit::geo::GridSpecByUID::instance().get(uid).spec());
                ASSERT(catalog_);

                type = eckit::StringTools::lower(catalog_->get_string("type", type));
            }

            SimpleParametrisation id;
            id.set("type", type);
            gridType_ = &grid_type_rules.find(id);
        }
    }

    template <class T>
    bool get(const std::string& name, T& value) const {
        return (catalog_ && catalog_->get(name, value)) || gridType_->get(name, value) || param_->get(name, value);
    }

private:
    std::unique_ptr<const eckit::spec::Spec> catalog_;
    const MIRParametrisation* gridType_ = &EMPTY;
    const MIRParametrisation* param_    = &EMPTY;
};


}  // namespace detail


FieldParametrisation::FieldParametrisation() = default;


FieldParametrisation::~FieldParametrisation() = default;


bool FieldParametrisation::has(const std::string& /*name*/) const {
    // Log::debug() << "FieldParametrisation::has(" << name << ") " << *this << std::endl;
    return false;
}


bool FieldParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<double>& value) const {

    // Check if this is in the Rules
    if (_get(name, value)) {
        return true;
    }

    // Special cases
    if (name == "grid") {
        std::vector<double> grid(2, 0.);
        if (get("west_east_increment", grid[0]) && get("south_north_increment", grid[1])) {
            value.swap(grid);
            return true;
        }
    }

    if (name == "area") {
        std::vector<double> area(4, 0.);
        if (get("north", area[0]) && get("west", area[1]) && get("south", area[2]) && get("east", area[3])) {
            value.swap(area);
            return true;
        }
    }

    if (name == "latitudes") {
        latitudes(value);
        return !value.empty();
    }

    if (name == "longitudes") {
        longitudes(value);
        return !value.empty();
    }

    return false;
}


bool FieldParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


void FieldParametrisation::reset() {
    // Reset cached values
    info_.reset();
}


template <class T>
bool FieldParametrisation::_get(const std::string& name, T& value) const {
    if (!info_) {
        // empty while finding (finding queries this parametrisation)
        info_ = std::make_unique<detail::FieldInfo>();
        info_ = std::make_unique<detail::FieldInfo>(*this);
    }

    return info_->get(name, value);
}


void FieldParametrisation::latitudes(std::vector<double>& /*unused*/) const {
    std::ostringstream os;
    os << "FieldParametrisation::latitudes() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void FieldParametrisation::longitudes(std::vector<double>& /*unused*/) const {
    std::ostringstream os;
    os << "FieldParametrisation::longitudes() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


}  // namespace mir::param
