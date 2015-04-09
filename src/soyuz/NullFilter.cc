// File NullFilter.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "NullFilter.h"

#include <iostream>

NullFilter::NullFilter(const MIRParametrisation& parametrisation):
    Action(parametrisation)
{
}

NullFilter::~NullFilter()
{
}

void NullFilter::print(std::ostream& out) const
{
    out << "NullFilter[]";
}

void NullFilter::execute(MIRField&) const
{
    // Nothing to do
}

static ActionBuilder<NullFilter> nullFilter("filter.noop");
