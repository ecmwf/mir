// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "eckit/geo/Grid.h"
#include "eckit/spec/Spec.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Mutex.h"


namespace mir::param {


class GridSpecParametrisation final : public MIRParametrisation {
public:
    // -- Constructors

    explicit GridSpecParametrisation(const std::string& gridspec);
    explicit GridSpecParametrisation(const eckit::geo::Grid*);

    // -- Methods

    const eckit::geo::Grid& grid() const;
    const eckit::spec::Spec& spec() const;

private:
    // -- Members

    std::unique_ptr<const eckit::geo::Grid> grid_;

    mutable SimpleParametrisation cache_;
    mutable util::recursive_mutex mutex_;  // to protect the cache

    // -- Methods

    /// If the value is known, computing (and caching) it on first request
    bool cached(const std::string& name) const;

    template <typename T>
    bool _get(const std::string& name, T& value) const {
        util::lock_guard<util::recursive_mutex> lock(mutex_);
        return cached(name) && cache_.get(name, value);
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
