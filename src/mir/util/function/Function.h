/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_function_Function_h
#define mir_util_function_Function_h

#include <functional>
#include <string>
#include <vector>
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace util {
namespace function {


/// Function base class (tree nodes)
class Function {
public:
    virtual ~Function() {}
    virtual double eval(const param::MIRParametrisation& p) const = 0;
};


/// Constant value (function tree leaf)
class Constant : public Function {
public:
    Constant(double value) : value_(value) {}
    double eval(const param::MIRParametrisation&) const;
private:
    const double value_;
};


/// Variable (runtime) value (function tree leaf)
class Variable : public Function {
public:
    Variable(const char* varname) : variable_(varname) {}
    Variable(const std::string& varname) : variable_(varname) {}
    double eval(const param::MIRParametrisation& p) const;
private:
    std::string variable_;
};


// --


class FunctionFactory {
protected:
    const std::string name_;
    FunctionFactory(const std::string& name);
    virtual ~FunctionFactory();
public:
    virtual Function* make(std::vector<Function*>&) = 0;
    static Function* build(const std::string& name, std::vector<Function*>);
    static void list(std::ostream&);
};


template<class T>
class UnaryOperationBuilder : public FunctionFactory {
    Function* make(std::vector<Function*>& args) {
        if (args.size() != 1) {
            throw eckit::UserError("FunctionFactory: '" + name_ + "' takes one argument");
        }
        return new T(args[0]);
    }
public:
    UnaryOperationBuilder(const std::string& name) : FunctionFactory(name) {}
};


template<class T>
class BinaryOperationBuilder : public FunctionFactory {
    Function* make(std::vector<Function*>& args) {
        if (args.size() != 2) {
            throw eckit::UserError("FunctionFactory: '" + name_ + "' takes two arguments");
        }
        return new T(args[0], args[1]);
    }
public:
    BinaryOperationBuilder(const std::string& name) : FunctionFactory(name) {}
};


template<class T>
class FunctionBuilder : public FunctionFactory {
    Function* make(std::vector<Function*>& args) {
        return new T(args);
    }
public:
    FunctionBuilder(const std::string& name) : FunctionFactory(name) {}
};


}  // namespace function
}  // namespace util
}  // namespace mir


#endif
