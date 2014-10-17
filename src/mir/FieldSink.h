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

#ifndef mir_FieldSink_H
#define mir_FieldSink_H

#include "atlas/FieldSet.h"

#include "mir/Action.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class FieldSink : public Action {

public: // methods

    FieldSink( const eckit::Params::Ptr& );

    virtual ~FieldSink();

	void eval( const atlas::FieldSet::Ptr& ) const;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
