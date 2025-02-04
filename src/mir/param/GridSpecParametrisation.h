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


#pragma once

#include <memory>

#include "eckit/geo/Grid.h"
#include "eckit/geo/Spec.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Mutex.h"


namespace mir::param {


class GridSpecParametrisation final : public MIRParametrisation {
public:
    // -- Constructors

    explicit GridSpecParametrisation(const std::string& gridspec);
    explicit GridSpecParametrisation(const eckit::geo::Grid*);

    // -- Types

    struct Mapping {
        Mapping(const Mapping&) = delete;
        Mapping(Mapping&&)      = delete;

        virtual ~Mapping() = default;

        Mapping& operator=(const Mapping&) = delete;
        Mapping& operator=(Mapping&&)      = delete;

        virtual void fill(SimpleParametrisation& param) const {}

        virtual bool get(const std::string& name, std::vector<int>&) const { return false; }
        virtual bool get(const std::string& name, std::vector<long>&) const { return false; }
        virtual bool get(const std::string& name, std::vector<float>&) const { return false; }
        virtual bool get(const std::string& name, std::vector<double>&) const { return false; }
        virtual bool get(const std::string& name, std::vector<std::string>&) const { return false; }

    protected:
        Mapping() = default;
    };

    // -- Methods

    const eckit::geo::Grid& grid() const;
    const eckit::geo::Spec& spec() const { return spec_; }

private:
    // -- Members

    std::unique_ptr<const eckit::geo::Grid> grid_;
    const eckit::geo::Spec& spec_;

    mutable SimpleParametrisation cache_;
    mutable util::recursive_mutex mutex_;  // to protect the cache

    std::unique_ptr<const Mapping> mapping_;

    // -- Methods

    template <typename T>
    void _set(const std::string& name, const T& value) const {
        util::lock_guard<util::recursive_mutex> lock(mutex_);

        cache_.set(name, value);
    }

    template <typename T>
    bool _get(const std::string& name, T& value) const {
        util::lock_guard<util::recursive_mutex> lock(mutex_);

        if (cache_.get(name, value)) {
            return true;
        }

        if (spec_.get(name, value)) {
            cache_.set(name, value);
            return true;
        }

        return false;
    }

    // -- Overridden methods

    // From MIRParametrisation
    bool has(const std::string& name) const override;

    bool get(const std::string& name, std::string&) const override;
    bool get(const std::string& name, bool&) const override;
    bool get(const std::string& name, int&) const override;
    bool get(const std::string& name, long&) const override;
    bool get(const std::string& name, float&) const override;
    bool get(const std::string& name, double&) const override;

    bool get(const std::string& name, std::vector<int>&) const override;
    bool get(const std::string& name, std::vector<long>&) const override;
    bool get(const std::string& name, std::vector<float>&) const override;
    bool get(const std::string& name, std::vector<double>&) const override;
    bool get(const std::string& name, std::vector<std::string>&) const override;

    void print(std::ostream&) const override;
};


}  // namespace mir::param
