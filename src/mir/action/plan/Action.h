// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <memory>
#include <string>


namespace eckit {
class JSON;
}

namespace mir {
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
namespace util {
class BoundingBox;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::action {


class Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Action(const param::MIRParametrisation&);

    Action(const Action&) = delete;
    Action(Action&&)      = delete;

    // -- Destructor

    virtual ~Action();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Action&) = delete;
    void operator=(Action&&)      = delete;

    // -- Methods

    void perform(context::Context&) const;

    virtual bool sameAs(const Action&) const = 0;
    virtual void custom(std::ostream&) const;
    virtual const char* name() const = 0;

    // For optimising plans
    virtual bool mergeWithNext(const Action&);
    virtual bool deleteWithNext(const Action&);
    virtual bool isEndAction() const;
    virtual bool isCropAction() const;
    virtual bool isRegridAction() const;

    virtual bool canCrop() const;
    virtual bool getGriddedTargetName(std::string&) const;

    virtual util::BoundingBox outputBoundingBox() const;

    /// Spec (see ActionFactory::make_from_string), as a known action name if built from it alone
    void json(eckit::JSON&, bool lookupKnownActions) const;
    std::string json_str(bool lookupKnownActions = true) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    virtual void print(std::ostream&) const = 0;
    const param::MIRParametrisation& parametrisation() const { return parametrisation_; }

    /// Spec {type: name, ...}, as ActionFactory::make_from_string (not supported by default)
    virtual void json(eckit::JSON&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const param::MIRParametrisation& parametrisation_;

    /// Parametrisation, if built from a spec (see ActionFactory::make_from_string)
    std::unique_ptr<const param::MIRParametrisation> spec_;

    // -- Methods

    // call perform() for the public interface
    virtual void execute(context::Context&) const = 0;

    bool getKnownName(std::string&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Action& p) {
        p.print(s);
        return s;
    }

    /// The same action, as sameAs (all actions, a spec being optional)
    friend bool operator==(const Action& a, const Action& b) { return a.sameAs(b) && b.sameAs(a); }
    friend bool operator!=(const Action& a, const Action& b) { return !(a == b); }

    friend class ActionFactory;
};


class ActionFactory {
    std::string name_;

    virtual Action* make(const param::MIRParametrisation&) = 0;

protected:
    ActionFactory(const std::string&);

    virtual ~ActionFactory();

public:
    ActionFactory(const ActionFactory&)            = delete;
    ActionFactory(ActionFactory&&)                 = delete;
    ActionFactory& operator=(const ActionFactory&) = delete;
    ActionFactory& operator=(ActionFactory&&)      = delete;

    static Action* build(const std::string&, const param::MIRParametrisation&, bool exact = true);
    static void list(std::ostream&);

    /// Action from a spec: a name, or {type: name, a_b: value, ...} (as Action::json_str), owning its parametrisation
    [[nodiscard]] static Action* make_from_string(const std::string&);
};


template <class T>
class ActionBuilder : public ActionFactory {
    Action* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    ActionBuilder(const std::string& name) : ActionFactory(name) {}
};


}  // namespace mir::action
