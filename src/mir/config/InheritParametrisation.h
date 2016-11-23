/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#ifndef mir_config_InheritParametrisation_h
#define mir_config_InheritParametrisation_h

#include <iosfwd>
#include <vector>
#include "eckit/value/Value.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace config {


class InheritParametrisation : protected param::SimpleParametrisation {
public:

    // -- Contructors

    InheritParametrisation();

    InheritParametrisation(const InheritParametrisation* parent, const std::string& key, const std::string& value);

    InheritParametrisation(const InheritParametrisation* parent, const std::vector<long>& paramIds);

    // -- Destructor

    ~InheritParametrisation();

    // -- Methods

    // Add a child
    InheritParametrisation& child(InheritParametrisation* who);

    // Fill contents provided a map
    void fill(const eckit::ValueMap&);

    /// Find best matching descendant according to paramId and metadata
    const InheritParametrisation& pick(const long& paramId, const param::MIRParametrisation& metadata) const;

    /// Find best matching descendant according to (key,value) pair
    const InheritParametrisation& pick(const std::string& key, const std::string& value) const;

    /// Collect all inherited traits, prioritizing younger/children traits
    void inherit(param::SimpleParametrisation& who) const;

    bool matches(const long& paramId, const param::MIRParametrisation& metadata) const;

    bool matches(const std::string& key, const std::string& value) const;

    const std::vector<long>& paramIds() const;

    bool empty() const;

    void print(std::ostream&) const;

    using SimpleParametrisation::has;
    using SimpleParametrisation::set;

private:

    // No copy allowed
    InheritParametrisation(const InheritParametrisation&);
    InheritParametrisation& operator=(const InheritParametrisation&);

    // -- Members

    const InheritParametrisation* parent_;
    std::vector< const InheritParametrisation* > children_;
    std::vector<long> paramIds_;
    const std::string key_;
    const std::string value_;
    const std::string fill_;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const InheritParametrisation& p) {
        p.print(s);
        return s;
    }

};


}  // namespace config
}  // namespace mir


#endif
