/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_fe_Tessellation_H
#define mir_method_fe_Tessellation_H

#include "mir/method/fe/FiniteElement.h"


namespace mir {
namespace method {
namespace fe {


class Tessellation: public FiniteElement {
public:

    Tessellation(const param::MIRParametrisation&);

    virtual ~Tessellation();

protected:

    virtual void hash(eckit::MD5&) const;
    virtual bool sameAs(const Method& other) const;
    virtual void print(std::ostream&) const;

private:

    virtual const char* name() const;

};


}  // namespace fe
}  // namespace method
}  // namespace mir


#endif

