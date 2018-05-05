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


#ifndef mir_action_AreaCropper_h
#define mir_action_AreaCropper_h

#include "mir/action/plan/Action.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {
class Representation;
}
}


namespace mir {
namespace action {


class AreaCropper : public Action {
public:

    // -- Exceptions
    // None

    // -- Contructors

    AreaCropper(const param::MIRParametrisation&);
    AreaCropper(const param::MIRParametrisation&, const util::BoundingBox&);

    // -- Destructor

    virtual ~AreaCropper(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static util::BoundingBox croppingBoundingBox(const repres::Representation*, const util::BoundingBox&, bool caching);

    // -- Overridden methods

    virtual void execute(context::Context&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    util::BoundingBox bbox_;
    bool caching_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool sameAs(const Action& other) const;
    virtual const char* name() const;
    virtual bool isCropAction() const;
    virtual bool canCrop() const;
    virtual const util::BoundingBox& croppingBoundingBox() const;
    virtual util::BoundingBox extendedBoundingBox(const util::BoundingBox&, double angle) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const AreaCropper& p)
    //  { p.print(s); return s; }

};


}  // namespace action
}  // namespace mir


#endif

