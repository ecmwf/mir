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


class InheritParametrisation : public param::SimpleParametrisation {
public:

    // -- Contructors

    InheritParametrisation();

    InheritParametrisation(const InheritParametrisation* parent, const std::vector<std::string>& labels);

    InheritParametrisation(const InheritParametrisation* parent, const std::vector<long>& ids);

    // -- Destructor

    ~InheritParametrisation();

    // -- Methods

    // Add a child
    InheritParametrisation& child(InheritParametrisation* who);

    // Fill contents provided a ValueMap (not overwriting)
    void fill(const eckit::ValueMap&);

    /// Find best matching descendant according to paramId and metadata
    const InheritParametrisation& pick(const long& paramId, const param::MIRParametrisation& metadata) const;

    /// Find best matching descendant according to label
    const InheritParametrisation& pick(const std::string& label) const;

    /// Check if ithis matches given paramId and metadata
    bool matches(const long& paramId, const param::MIRParametrisation& metadata) const;

    /// Check if ithis matches given label
    bool matches(const std::string& label) const;

    /// Collect all inherited traits, prioritizing younger/children traits
    void inherit(param::SimpleParametrisation& param) const;

    bool empty() const;

    void print(std::ostream&) const;

private:

    // No copy allowed
    InheritParametrisation(const InheritParametrisation&);
    InheritParametrisation& operator=(const InheritParametrisation&);

    // -- Methods
    // None

    // -- Members

    const InheritParametrisation* parent_;
    std::vector< const InheritParametrisation* > children_;
    std::vector< long >        paramIds_;
    std::vector< std::string > labels_;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const InheritParametrisation& p) {
        p.print(s);
        return s;
    }

};


}  // namespace config
}  // namespace mir


#endif
