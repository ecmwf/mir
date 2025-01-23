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


#include "mir/param/MIRParametrisation.h"

#include <memory>

#include "eckit/geo/spec/Custom.h"


namespace eckit::geo {
class Grid;
}


namespace mir::param {


class GridSpecParametrisation final : public MIRParametrisation {
public:
    explicit GridSpecParametrisation(const eckit::geo::Grid&);

private:
    bool has(const std::string& name) const override { return custom_->has(name); }

    bool get(const std::string& name, std::string& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, bool& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, int& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, long& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, float& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, double& v) const override { return custom_->get(name, v); }

    bool get(const std::string& name, std::vector<int>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<long>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<float>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<double>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<std::string>& v) const override { return custom_->get(name, v); }

    bool get(const std::string& name, size_t& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<size_t>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, long long& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<long long>& v) const override { return custom_->get(name, v); }

    void print(std::ostream&) const override;

    std::unique_ptr<eckit::geo::spec::Custom> custom_;
};


}  // namespace mir::param
