/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date Jun 2014

#ifndef mir_Action_H
#define mir_Action_H

// #include "eckit/maths/Expression.h"

#include "mir/Params.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

/// @todo this class will become an eckit::maths::Expression

class Action : private eckit::NonCopyable {

public: // methods

    Action( const mir::Params::Ptr& );

    virtual ~Action();

    const mir::Params& params() const { return *params_; }
    mir::Params& params() { return *params_; }

private: // members

    mir::Params::Ptr params_;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
