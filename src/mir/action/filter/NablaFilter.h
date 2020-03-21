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


#ifndef mir_action_NablaFilter_h
#define mir_action_NablaFilter_h

#include <string>

#include "mir/action/plan/Action.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir {
namespace action {


class NablaFilter : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    NablaFilter(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~NablaFilter();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    util::MeshGeneratorParameters meshGeneratorParams_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


class NablaFilterFactory : public ActionFactory {
protected:
    NablaFilterFactory(const std::string& name);
    virtual ~NablaFilterFactory();

public:
    static void list(std::ostream&);
};


template <class T>
class NablaFilterBuilder : public NablaFilterFactory {
    virtual NablaFilter* make(const param::MIRParametrisation& param) { return new T(param); }

public:
    NablaFilterBuilder(const std::string& name) : NablaFilterFactory(name) {}
};


}  // namespace action
}  // namespace mir


#endif
