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

#include "atlas/grid/FieldSet.h"

#include "mir/Params.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class FieldContext : public eckit::DispatchParams<FieldContext> {

public: // types

    typedef atlas::grid::FieldSet FieldSet;

public: // methods

    FieldContext( const FieldSet::Ptr& );
    virtual ~FieldContext();

protected: // methods

    value_t getBoundBox( const key_t& k ) const;

private: // members

    FieldSet::Ptr fieldset_;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
