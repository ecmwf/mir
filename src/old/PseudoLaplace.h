/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef mir_PseudoLaplace_H
#define mir_PseudoLaplace_H

#include "mir/KNearest.h"

//-----------------------------------------------------------------------------

namespace atlas {
namespace grid { class Point2; }
}

namespace mir {

//-----------------------------------------------------------------------------

class PseudoLaplace: public KNearest {

public:

    PseudoLaplace();

    virtual ~PseudoLaplace();

	virtual void compute( atlas::Grid& in, atlas::Grid& out, Weights::Matrix& W ) const;

    virtual std::string classname() const;

};

//-----------------------------------------------------------------------------

} // namespace mir

#endif
