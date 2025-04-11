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


#include <iosfwd>
#include <string>
#include <vector>


namespace eckit {
class JSON;
}


namespace mir::util {


class Reorder {
public:
    // -- Types

    struct Builder {
        virtual Reorder* build(const std::string& name, size_t size) const = 0;

    protected:
        static void register_builder(const std::string& name, Builder*);
    };


    template <typename T>
    struct BuilderT : Builder {
        explicit BuilderT(const std::string& name) { register_builder(name, this); }

        Reorder* build(const std::string& name, size_t size) const override { return new T(name, size); }
    };

    // -- Constructors

    explicit Reorder(const std::string _name, size_t _size) : name(_name), size(_size) {}

    Reorder(const Reorder&) = delete;
    Reorder(Reorder&&)      = delete;

    // -- Destructors

    virtual ~Reorder() = default;

    // -- Operators

    Reorder& operator=(const Reorder&) = delete;
    Reorder& operator=(Reorder&&)      = delete;

    // -- Methods

    virtual std::vector<size_t> reorder() = 0;

    // -- Members

    const std::string name;
    const size_t size;

    // -- Class methods

    static Reorder* build(const std::string& name, size_t _size);
    static void list(std::ostream&);

private:
    // -- Methods

    virtual void print(std::ostream&) const = 0;

    void json(eckit::JSON&) const;

    // Friends

    friend std::ostream& operator<<(std::ostream& s, const Reorder& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& j, const Reorder& p) {
        p.json(j);
        return j;
    }
};


}  // namespace mir::util
