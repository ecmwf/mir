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


#ifndef mir_param_InheritParametrisation_h
#define mir_param_InheritParametrisation_h

#include <iosfwd>
#include <string>
#include <vector>
#include "eckit/memory/NonCopyable.h"
#include "eckit/value/Value.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace param {


class InheritParametrisation : public SimpleParametrisation, private eckit::NonCopyable {
public:

    // -- Contructors

    InheritParametrisation();

    // -- Destructor

    ~InheritParametrisation();

    // -- Methods

    // Fill parametrisation provided a ValueMap (not overwriting)
    void fill(const eckit::ValueMap&);

    // Fill parametrisation provided a InheritParametrisation
    void fill(const InheritParametrisation&);

    /// Find best matching descendant according to paramId and metadata
    const InheritParametrisation& pick(const long& paramId, const MIRParametrisation& metadata) const;

    /// Find best matching descendant according to labels (separated by /)
    const InheritParametrisation& pick(const std::string& str) const;

    /// Find best matching descendant according to label hierarchy
    const InheritParametrisation& pick(const std::vector< std::string >& labels) const;

    // -- Overridden methods

    // From SimpleParametrisation
    bool empty() const;
    InheritParametrisation& clear(const std::string& name);
    bool has(const std::string& name) const;

    bool get(const std::string& name, std::string& value) const;
    bool get(const std::string& name, bool& value) const;
    bool get(const std::string& name, int& value) const;
    bool get(const std::string& name, long& value) const;
    bool get(const std::string& name, float& value) const;
    bool get(const std::string& name, double& value) const;

    bool get(const std::string& name, std::vector<int>& value) const;
    bool get(const std::string& name, std::vector<long>& value) const;
    bool get(const std::string& name, std::vector<float>& value) const;
    bool get(const std::string& name, std::vector<double>& value) const;
    bool get(const std::string& name, std::vector<std::string>& value) const;

private:

    // -- Constructors

    InheritParametrisation(const InheritParametrisation* parent, const std::string& label);

    InheritParametrisation(const InheritParametrisation* parent, const std::vector<long>& ids);

    // -- Methods

    // Generic getter
    template<typename T>
    bool _get(const std::string& name, T& value) const;

    // Check if this (or a parent node) matches requested paramId
    bool matchesId(long) const;

    // Check if this matches requested metadata
    bool matchesMetadata(const MIRParametrisation&) const;

    /// Check if this matches requested label
    bool matchesLabel(const std::string&) const;

    // Add a child
    InheritParametrisation& addChild(InheritParametrisation*);

    /// Remove traits from children
    InheritParametrisation& clearFromChildren(const std::string&);

    /// Collect all inherited traits, prioritizing younger/children traits
    void inherit(SimpleParametrisation&) const;

    /// Create a readable label hierarchy
    std::string labelHierarchy() const;

    // -- Overridden methods

    void print(std::ostream&) const;

    // -- Members

    const InheritParametrisation* parent_;
    std::vector< InheritParametrisation* > children_;
    std::vector< long > paramIds_;
    std::string label_;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const InheritParametrisation& p) {
        p.print(s);
        return s;
    }

};


}  // namespace param
}  // namespace mir


#endif
