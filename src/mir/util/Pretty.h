/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_Pretty_h
#define mir_util_Pretty_h

#include <iosfwd>
#include <string>

#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"


namespace mir {


class Pretty {
public:
    // -- Types

    struct Plural {
        Plural(std::string one) : Plural(one, one + "s") {}
        Plural(std::string one, std::string notOne) : s_{one, notOne} {}
        Plural(const Plural& other) : s_{other.s_[0], other.s_[1]} {}
        ~Plural() = default;

        const std::string& operator()(int count) const { return s_[count != 1]; }
        const std::string& operator()(size_t count) const { return s_[count != 1]; }

        operator bool() const { return !s_[0].empty(); }
        Plural& operator=(const Plural&) = delete;

    private:
        const std::string s_[2];
    };

    struct PrettyProgress : public eckit::Timer {
        PrettyProgress(const std::string& name, size_t limit, const Plural& units, std::ostream&);
        virtual ~PrettyProgress() = default;
        bool operator++();

    protected:
        double lastTime_;
        size_t counter_;

    private:
        virtual bool hasOutput() = 0;
        const Plural units_;
        const size_t limit_;
    };

    struct ProgressTimer : PrettyProgress {

        /// @param name of the timer
        /// @param limit counter maximum value
        /// @param units unit/units
        /// @param time how often to output progress, based on elapsed time
        /// @param o output stream
        ProgressTimer(const std::string& name, size_t limit, const Pretty::Plural& units,
                      std::ostream& o = eckit::Log::info(), double time = 5.);

    private:
        bool hasOutput();
        const double time_;
    };

    struct ProgressCounter : PrettyProgress {

        /// @param name of the timer
        /// @param limit counter maximum value
        /// @param units unit/units
        /// @param count how often to output progress, based on total counter
        /// @param o output stream
        ProgressCounter(const std::string& name, size_t limit, const Pretty::Plural& units,
                        std::ostream& o = eckit::Log::info(), size_t count = 10000);

    private:
        bool hasOutput();
        const size_t count_;
    };


    // -- Exceptions
    // None

    // -- Constructors

    Pretty(int count);
    Pretty(size_t count);
    Pretty(int count, const Plural& plural) : plural_(plural), count_(count) {}
    Pretty(size_t count, const Plural& plural) : plural_(plural), count_(static_cast<int>(count)) {}
    Pretty(const Pretty&) = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    Pretty& operator=(const Pretty&) = delete;

    // -- Methods
    // None

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const Plural& plural_;
    int count_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Pretty& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir


#endif
