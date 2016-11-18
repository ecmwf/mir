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


#ifndef mir_config_InheritParam_h
#define mir_config_InheritParam_h

#include <algorithm>
#include <iosfwd>
#include <vector>
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace config {


class InheritParam : protected param::SimpleParametrisation {
public:

    // -- Contructors

    InheritParam();

    InheritParam(const InheritParam* parent, const std::string& key, const std::string& value);

    InheritParam(const InheritParam* parent, const std::vector<long>& paramIds);

    // -- Destructor

    ~InheritParam();

    // -- Methods

    // Add a child
    void child(const InheritParam* who);

    /// Find best matching descendant according to paramId and metadata
    bool pick(const InheritParam* who, const long& paramId, const param::MIRParametrisation& metadata) const;

    /// Collect all inherited traits, prioritizing younger/children traits
    void inherit(param::SimpleParametrisation& who) const;

    bool matches(const long& paramId, const param::MIRParametrisation& metadata) const;

    const std::vector<long>& paramIds() const;

    bool empty() const;

    void print(std::ostream&) const;

    using SimpleParametrisation::has;
    using SimpleParametrisation::set;

private:

    // No copy allowed
    InheritParam(const InheritParam&);
    InheritParam& operator=(const InheritParam&);

    // -- Members

    const InheritParam* parent_;
    std::vector< const InheritParam* > children_;
    std::vector<long> paramIds_;
    const std::string key_;
    const std::string value_;
    const std::string fill_;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const InheritParam& p) {
        p.print(s);
        return s;
    }

};


}  // namespace config
}  // namespace mir


#endif
