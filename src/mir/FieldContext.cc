/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "atlas/grid/Grid.h"

#include "mir/FieldContext.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace atlas::grid;

namespace mir {

//------------------------------------------------------------------------------------------------------

FieldContext::FieldContext(const atlas::grid::FieldSet::Ptr& f) :
    fieldset_(f)
{
    dispatch_["BoundBox"] = &FieldContext::getBoundBox;
}

FieldContext::~FieldContext()
{
}

Params::value_t FieldContext::getBoundBox(const Params::key_t& k, Params* r) const
{
    Grid::BoundBox bbox = fieldset_->grid().boundingBox();

    return Value( bbox );
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
