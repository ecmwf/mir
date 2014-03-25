/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @date Oct 2013

#ifndef mir_Action_H
#define mir_Action_H

#include <map>
#include <string>

#include "eckit/memory/NonCopyable.h"
#include "eckit/types/Types.h"

#include "mir/mir.h"

/// @todo move me
#include "mir/Interpolator.h"

//-----------------------------------------------------------------------------

namespace eckit {
namespace grid {

//-----------------------------------------------------------------------------

class FieldSet;

//-----------------------------------------------------------------------------

} // namespace grid
} // namespace eckit

namespace mir {

/// @note possibly rename to operation.. fieldop?
class Action : private eckit::NonCopyable {

public: // methods

    Action();

    virtual ~Action();

    virtual void operator() (const atlas::grid::FieldSet& input, atlas::grid::FieldSet& output) = 0;

protected:

};


class Interpolate : public Action {

public:
    Interpolate(eckit::StringDict& config);
    virtual ~Interpolate();

    virtual void operator() (const atlas::grid::FieldSet& input, atlas::grid::FieldSet& output) ;
protected:
    mir::Interpolator* interpolator_;

};

#if 0

class Var : public boost::any
{

};

class Op {
public:
    Op( Var& in ) { *this = in; }
    Op& operator= (Var& in);
    virtual Var result() = 0;
};

struct WrapVar : public Op
{
    WrapVar( Var in ) : store_(in) {}
    Var store_;

    Var result() { return store_; }
};

class Add : public Op
{
public:

    Op operator() ( Var a, Var b )
    {
        a_ = a;
        b_ = b;
        return this;
    }

    Op operator() ( Op a, Var b )
    {
        a_ = a;
        b_ = b;
        return this;
    }

    Op operator() ( Var a, Op b );
    {
        a_ = a;
        b_ = b;
        return this;
    }

    Op operator() ( Op a, Op b );
    {
        a_ = a;
        b_ = b;
        return this;
    }

    virtual Var result()
    {
        return a_.result() + b_.result();
    }

private:

    Op a_;
    Op b_;

};

class Not : public Op
{
public:
    Op& operator() ( Var a )
    {
        a_ = a;
        return this;
    }

    virtual Var result()
    {
        return ! a_.result();
    }

private:

    Op a_;
};

Var r = Add( Add( 2, 3) , Not( 0 ) ).result();

#endif

/// @todo implement sublcasses for functionality intepolate / extract etc 
//
//e.g. Interpolate : public Action etc

//-----------------------------------------------------------------------------

} // namespace mir

#endif
