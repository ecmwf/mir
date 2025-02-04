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


#pragma once

#include <string>
#include <vector>


namespace eckit {
class Buffer;
class JSON;
namespace option {
class Option;
class CmdArgs;
}  // namespace option
}  // namespace eckit


namespace mir::compare {
class Field;
class FieldSet;
class MultiFile;
}  // namespace mir::compare


namespace mir::compare {

class WhiteLister {
public:
    WhiteLister() = default;

    WhiteLister(const WhiteLister&) = delete;
    WhiteLister(WhiteLister&&)      = delete;

    virtual ~WhiteLister();

    void operator=(const WhiteLister&) = delete;
    void operator=(WhiteLister&&)      = delete;

    virtual bool whiteListed(const MultiFile&, const Field&) const = 0;
    virtual bool ignoreError(const MultiFile&, const Field&) const = 0;
};

class DefaultWhiteLister : public WhiteLister {
    bool whiteListed(const MultiFile&, const Field&) const override { return false; }
    bool ignoreError(const MultiFile&, const Field&) const override { return false; }
    ~DefaultWhiteLister() override;

public:
    static const WhiteLister& instance();
};


class FieldComparator {
public:  // types
    using FieldSet  = compare::FieldSet;
    using MultiFile = compare::MultiFile;

public:  // methods
    FieldComparator(const eckit::option::CmdArgs&, const WhiteLister& = DefaultWhiteLister::instance());
    ~FieldComparator();

    void compare(const std::string& path1, const std::string& path2);

    void compare(const std::string& name, const MultiFile& multi1, const MultiFile& multi2);


    size_t list(const std::string& path);
    void json(eckit::JSON&, const std::string& path);

    static void addOptions(std::vector<eckit::option::Option*>&);

protected:  // members
    size_t count(const MultiFile& multi, FieldSet& fields);

    void compareCounts(const std::string& name, const MultiFile& multi1, const MultiFile& multi2, FieldSet& fields1,
                       FieldSet& fields2);

    void getField(const MultiFile& multi, eckit::Buffer& buffer, FieldSet& fields, const std::string& path,
                  off_t offset, size_t size, bool fail, size_t& duplicates);

    Field getField(eckit::Buffer& buffer, const std::string& path, off_t offset, size_t size);

    void getGribField(const MultiFile& multi, eckit::Buffer& buffer, FieldSet& fields, const std::string& path,
                      off_t offset, size_t size, bool fail);

    void getBufrField(const MultiFile& multi, eckit::Buffer& buffer, FieldSet& fields, const std::string& path,
                      off_t offset, size_t size, bool fail);

    void compareFields(const MultiFile& multi1, const MultiFile& multi2, const FieldSet& fields1,
                       const FieldSet& fields2, bool compareValues, bool compareMissingValues, bool compareStatistics);

    void compareFieldStatistics(const MultiFile& multi1, const MultiFile& multi2, const Field& field1,
                                const Field& field2);

    void compareFieldValues(const MultiFile& multi1, const MultiFile& multi2, const Field& field1, const Field& field2);

    void compareFieldMissingValues(const MultiFile& multi1, const MultiFile& multi2, const Field& field1,
                                   const Field& field2);

    void missingField(const MultiFile& multi1, const MultiFile& multi2, const Field& field, const FieldSet& fields,
                      bool& show);

    void error(const char* what);

    double normalised(double) const;
    double rounded(double) const;

protected:
    mutable size_t fatals_;
    mutable size_t warnings_;

private:
    const eckit::option::CmdArgs& args_;

    std::vector<std::string> ignore_;
    size_t maximumNumberOfErrors_;
    size_t saved_;

    const WhiteLister& whiteLister_;

    bool normaliseLongitudes_;
    bool ignoreWrappingAreas_;
    bool roundDegrees_;
    bool whiteListEntries_;
    bool saveFirstPossibleMatch_;
    bool saveDuplicates_;

    void whiteListEntries(const Field&, const MultiFile&) const;
};


}  // namespace mir::compare
