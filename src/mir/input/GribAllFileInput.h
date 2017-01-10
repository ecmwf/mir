/*
 * (C) Copyright 1996-2015 ECMWF.
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


#ifndef mir_input_GribAllFileInput_h
#define mir_input_GribAllFileInput_h


#include <string>
#include <vector>
#include "mir/input/MIRInput.h"


namespace mir {
namespace input {


class GribAllFileInput : public MIRInput {
public:

    // -- Exceptions
    // None

    // -- Contructors

    GribAllFileInput(const std::string& path);

    // -- Destructor

    virtual ~GribAllFileInput(); // Change to virtual if base class

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
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    GribAllFileInput(const GribAllFileInput &);
    GribAllFileInput &operator=(const GribAllFileInput &);

    // -- Members

    std::string path_;
    std::vector<MIRInput*> inputs_;
    size_t count_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual const param::MIRParametrisation& parametrisation(size_t which) const;
    virtual data::MIRField field() const;
    virtual bool next();

    virtual bool sameAs(const MIRInput &other) const ;
    virtual void print(std::ostream &out) const;

    virtual grib_handle* gribHandle(size_t which = 0) const;
    virtual size_t dimensions() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace input
}  // namespace mir
#endif

