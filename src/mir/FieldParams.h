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

#ifndef mir_FieldParams_H
#define mir_FieldParams_H

#include "atlas/FieldSet.h"

#include "mir/Params.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class FieldParams : public eckit::DispatchParams<FieldParams> {

public: // methods

	FieldParams( const atlas::FieldSet::Ptr& );

	virtual ~FieldParams();

private: // members

	atlas::FieldSet::Ptr fieldset_;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
