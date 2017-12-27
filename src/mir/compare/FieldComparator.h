/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FieldComparator.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   May 2016

#ifndef mir_compare_FieldComparator_h
#define mir_compare_FieldComparator_h

#include <string>
#include <vector>


namespace eckit {
class Buffer;
namespace option {
class Option;
class CmdArgs;
}
}


namespace mir {
namespace compare {
class Field;
class FieldSet;
class MultiFile;
}
}


namespace mir {
namespace compare {

class WhiteLister {
public:
    virtual bool whiteListed(const MultiFile& name, const Field&) const = 0;
};

class DefaultWhiteLister : public WhiteLister {
    virtual bool whiteListed(const MultiFile& name, const Field&) const { return false; };
public:
    static const WhiteLister& instance();
};


class FieldComparator {
public: // types

  typedef mir::compare::FieldSet  FieldSet;
  typedef mir::compare::MultiFile MultiFile;

public: // methods

  FieldComparator(const eckit::option::CmdArgs &args, const WhiteLister& = DefaultWhiteLister::instance());
  ~FieldComparator();

  void compare(const std::string& path1,
               const std::string& path2);

  void compare(const std::string& name,
               const MultiFile& multi1,
               const MultiFile& multi2);


  size_t list(const std::string& path);

  static void addOptions(std::vector<eckit::option::Option*>&);

protected: // members

  size_t count(const MultiFile& multi,
               FieldSet& fields);


  void compareCounts(const std::string& name,
                     const MultiFile& multi1,
                     const MultiFile& multi2,
                     FieldSet& fields1,
                     FieldSet& fields2);

  void getField(const MultiFile& multi,
                eckit::Buffer& buffer,
                FieldSet& fields,
                const std::string& path,
                off_t offset,
                size_t size,
                bool fail);

  void compareFields(const MultiFile& multi1,
                     const MultiFile& multi2,
                     const FieldSet& fields1,
                     const FieldSet& fields2,
                     bool compareValues,
                     bool compareStatistics);

  void compareFieldStatistics(
    const MultiFile& multi1,
    const MultiFile& multi2,
    const Field& field1,
    const Field& field2);

  void compareFieldValues(
    const MultiFile& multi1,
    const MultiFile& multi2,
    const Field& field1,
    const Field& field2);

  void missingField(const MultiFile& multi1,
                    const MultiFile& multi2,
                    const Field& field,
                    const FieldSet& fields,
                    bool& show);

  void error(const char* string);

  double normalised(double) const;
  double rounded(double) const;

protected:

  mutable size_t fatals_;
  mutable size_t warnings_;

private:

  const eckit::option::CmdArgs &args_;
  bool normaliseLongitudes_;
  bool ignoreWrappingAreas_;
  bool whiteListEntries_;



  bool roundDegrees_;
  std::vector<std::string> ignore_;
  size_t maximumNumberOfErrors_;

  const WhiteLister& whiteLister_;

  void whiteListEntries(const Field & field, const MultiFile & multi) const;

};



}  // namespace compare
}  // namespace mir


#endif
