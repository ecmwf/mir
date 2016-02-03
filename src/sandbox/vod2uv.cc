/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/runtime/Tool.h"
// #include "eckit/runtime/Context.h"
// #include "eckit/parser/Tokenizer.h"

// #include "mir/api/MIRJob.h"
// #include "mir/action/VOD2UVTransform.h"
// #include "mir/action/Sh2ShTransform.h"

// #include "mir/input/GribFileInput.h"
// #include "mir/output/GribFileOutput.h"
// #include "mir/data/MIRField.h"
// #include "mir/repres/Representation.h"

#include "mir/api/emoslib.h"
#include "eckit/io/StdFile.h"
#include "mir/util/Grib.h"
#include "eckit/io/Buffer.h"


class VOD2UVTool : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    VOD2UVTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void VOD2UVTool::run() {
    int err;
    size_t size_vo, size_d;
    const void *vo_buffer, *d_buffer;

    eckit::StdFile fvo("/tmp/vo.grib");
    grib_handle *hvo = grib_handle_new_from_file(0, fvo, &err);
    ASSERT(hvo);
    grib_get_message(hvo, &vo_buffer, &size_vo);

    eckit::StdFile fd("/tmp/d.grib");
    grib_handle *hd = grib_handle_new_from_file(0, fd, &err);
    ASSERT(hd);
    grib_get_message(hd, &d_buffer, &size_d);

    ASSERT(size_d == size_vo);

    int size = size_d;

    eckit::Log::info() << "Size " << size << std::endl;

    eckit::Buffer ub(size);
    eckit::Buffer vb(size);

    int ints[] = {1,};
    double reals[] = { 5, 5};

    intout_("autoresol", ints, reals, 0, 0, 0);
    intout_("grid", ints, reals, 0, 0, 0);

    intuvp2_(vo_buffer, d_buffer, size, ub, vb, size);
    eckit::StdFile fu("/tmp/uv.grib", "w");
    fwrite(ub, 1, size, fu);
    fwrite(vb, 1, size, fu);
}


int main( int argc, char **argv ) {
    VOD2UVTool tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    return 0;
#else
    return tool.start();
#endif
}

