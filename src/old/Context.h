/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Florian Rathgeber
/// @date March 2015

#ifndef mir_Context_H
#define mir_Context_H

#include "mir/Params.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class MirContext {

public: // methods
    MirParams& params();

private: // members
    MirParams params_;
};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
