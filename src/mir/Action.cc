/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/log/Log.h"
#include "Action.h"
#include "Interpolator.h" 

//-----------------------------------------------------------------------------

namespace mir {

//-----------------------------------------------------------------------------

Action::Action()
{
    eckit::Log::info() << "Build a Action" << std::endl;
}

Action::~Action()
{
}

//-----------------------------------------------------------------------------

Interpolate::Interpolate(eckit::StringDict& config)
: interpolator_(0)
{
    for (eckit::StringDict::const_iterator it = config.begin(); it != config.end(); ++it)
        eckit::Log::info() << "Interpolate config " << it->first << " = " << it->second << std::endl;
    
    /// @todo react to config parameters
    interpolator_ = new mir::Bilinear(); 
}

Interpolate::~Interpolate()
{
    if (interpolator_)
    {
        delete interpolator_;
        interpolator_ = 0;
    }
}


void Interpolate::operator() (const eckit::grid::FieldSet& input, eckit::grid::FieldSet& output)
{
    ASSERT(interpolator_);
    interpolator_->interpolate(input, output);
}

} // namespace mir
