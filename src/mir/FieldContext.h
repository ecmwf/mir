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
/// @date July 2014

#ifndef mir_FieldContext_H
#define mir_FieldContext_H

#include "atlas/FieldSet.h"

#include "mir/Params.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class FieldContext : public eckit::DispatchParams<FieldContext> {

public: // methods

	FieldContext( const atlas::FieldSet::Ptr& );

	virtual ~FieldContext();

private: // members

	atlas::FieldSet::Ptr fieldset_;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
