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

#include "mir/api/emoslib.h"


#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/runtime/LibBehavior.h"
#include "eckit/runtime/Context.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/output/GribMemoryOutput.h"

#include "mir/input/VODInput.h"
#include "mir/output/UVOutput.h"

namespace mir {
namespace api {
namespace {


typedef int fortint;
typedef double fortfloat;


std::auto_ptr<MIRJob> job(0);


extern "C" fortint intout_(char *name, fortint *ints, fortfloat *reals, const char *value, fortint, fortint) {

    eckit::Log::info() << "++++++ intout " << name << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        if (strcasecmp(name, "grid") == 0) {
            job->set("grid", reals[0], reals[1]);
            return 0;
        }

        if (strcasecmp(name, "area") == 0) {
            job->set("area", reals[0] ,  reals[1] , reals[2] , reals[3]);
            return 0;
        }

        // if(strcasecmp(name, "gaussian") == 0) {
        //      // TODO:
        //     return 0;
        // }

        if (strcasecmp(name, "reduced") == 0) {
            job->set("reduced", long(ints[0]));
            return 0;
        }

        if (strcasecmp(name, "truncation") == 0) {
            job->set("truncation", long(ints[0]));
            return 0;
        }

        if (strcasecmp(name, "regular") == 0) {
            job->set("regular", long(ints[0]));
            return 0;
        }

        // TODO: Check that gaussian == regular in all cases
        if (strcasecmp(name, "gaussian") == 0) {
            job->set("regular", long(ints[0]));
            return 0;
        }

        if (strcasecmp(name, "rotation") == 0) {
            job->set("rotation", double(reals[0]) , double(reals[1]));
            return 0;
        }

        if (strcasecmp(name, "autoresol") == 0) {
            job->set("autoresol", ints[0] != 0);
            return 0;
        }

        // if(strcasecmp(name, "resol") == 0) {
        //     job->set("resol", value);
        //     return 0;
        // }

        if (strcasecmp(name, "style") == 0) {
            job->set("style", value);
            return 0;
        }

        if (strcasecmp(name, "bitmap") == 0) {
            job->set("bitmap", value);
            return 0;
        }

        if (strcasecmp(name, "accuracy") == 0) {
            job->set("accuracy", value);
            return 0;
        }

        if (strcasecmp(name, "frame") == 0) {
            job->set("frame", long(ints[0]));
            return 0;
        }

        if (strcasecmp(name, "intermediate_gaussian") == 0) {
            job->set("intermediate_gaussian", long(ints[0]));
            return 0;
        }

        if (strcasecmp(name, "interpolation") == 0) {
            std::string low;
            const char *p = value;
            while (*p) {
                low += tolower(*p);
                p++;
            }
            job->set("interpolation", low);
            return 0;
        }

        throw eckit::SeriousBug(std::string("Unexpected name in INTOUT: ") + name);
        // job->set(

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intin_(char *name, fortint *ints, fortfloat *reals, const char *value, fortint, fortint) {

    eckit::Log::info() << "++++++ intin " << name << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        eckit::Log::warning() << "INTIN not implemenent (ignored), name=" << name << std::endl;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intf_(char *, fortint *, fortfloat *, char *, fortint *, fortfloat *) {

    eckit::Log::info() << "++++++ intf" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        NOTIMP;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intf2(char *grib_in, fortint *length_in, char *grib_out, fortint *length_out) {

    eckit::Log::info() << "++++++ intf2" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        mir::input::GribMemoryInput input(grib_in, *length_in);
        mir::output::GribMemoryOutput output(grib_out, *length_out);

        job->execute(input, output);

        ASSERT(output.interpolated() + output.saved() == 1);

        if (output.saved() == 1) {
            *length_out = 0; // Not interpolation performed
        } else {
            *length_out = output.length();
        }

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intuvs2_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {

    eckit::Log::info() << "++++++ intuvs2" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        NOTIMP;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intuvp2_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {

    eckit::Log::info() << "++++++ intuvp2" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        mir::input::GribMemoryInput vort_input(vort_grib_in, *length_in);
        mir::input::GribMemoryInput div_input(div_grib_in, *length_in);

        mir::output::GribMemoryOutput u_output(u_grib_out, *length_out);
        mir::output::GribMemoryOutput v_output(v_grib_out, *length_out);

        mir::input::VODInput input(vort_input, div_input);
        mir::output::UVOutput output(u_output, v_output);

        job->set("vod2uv", true);

        job->execute(input, output);

        job->clear("vod2uv");

        // ASSERT(output.interpolated() + output.saved() == 1);

        // if (output.saved() == 1) {
        //     *length_out = 0; // Not interpolation performed
        // } else {
        //     *length_out = output.length();
        // }

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intvect2_(char *u_grib_in, char *v_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {

    eckit::Log::info() << "++++++ intvect2" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        if (!job.get()) {
            job.reset(new MIRJob());
        }

        mir::input::GribMemoryInput vort_input(u_grib_in, *length_in);
        mir::input::GribMemoryInput div_input(v_grib_in, *length_in);

        mir::output::GribMemoryOutput u_output(u_grib_out, *length_out);
        mir::output::GribMemoryOutput v_output(v_grib_out, *length_out);

        mir::input::VODInput input(vort_input, div_input);
        mir::output::UVOutput output(u_output, v_output);

        job->execute(input, output);

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intuvs_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {

    eckit::Log::info() << "++++++ intuvs" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        NOTIMP;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intuvp_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {

    eckit::Log::info() << "++++++ intuvp" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        NOTIMP;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intvect_(char *u_grib_in, char *v_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {

    eckit::Log::info() << "++++++ intvect" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        NOTIMP;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint iscrsz_() {

    eckit::Log::info() << "++++++ iscrsz" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        NOTIMP;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint ibasini_(fortint *force) {

    eckit::Log::info() << "++++++ ibasini" << std::endl;

    // Init interpolation package
    job.reset(0);
    return 0;
}

extern "C" void intlogm_(fortint (*)(char *, fortint)) {

    eckit::Log::info() << "++++++ intlogm" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        NOTIMP;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
    }
#endif
}

typedef void (*emos_cb_proc)(char *);

struct emos_cb_ctx {
    emos_cb_proc proc;
};

static emos_cb_ctx emos_ctx;

static void callback(void *ctxt, const char *msg) {
    emos_cb_ctx *c = reinterpret_cast<emos_cb_ctx *>(ctxt);
    c->proc(const_cast<char *>(msg));
}

extern "C" void intlogs(emos_cb_proc proc) {

    eckit::Log::info() << "++++++ intlogs" << std::endl;

    emos_ctx.proc = proc;


    eckit::ContextBehavior &behavior = eckit::Context::instance().behavior();
    try {
        eckit::LibBehavior &libbehavior = dynamic_cast<eckit::LibBehavior &>(behavior);
        libbehavior.default_callback(&callback, &emos_ctx);
    } catch (std::bad_cast &) {
        eckit::Log::warning() << "INTLOGS: ContextBehavior is not a LibBehavior" << std::endl;
    }
}

extern "C" fortint areachk_(fortfloat *we, fortfloat *ns, fortfloat *north, fortfloat *west, fortfloat *south,
                            fortfloat *east) {

    /* FROM EMOSLIB:
    C     Input
    C     -----
    C
    C     For latitude/longitude grids:
    C     EW    =  East-west grid interval (degrees)
    C     NS    =  North-south grid interval (degrees)
    C
    C     For gaussian grids:
    C     EW    =  gaussian grid number
    C     NS    =  0
    C
    C     NORTH =  North latitude (degrees)
    C     WEST  =  West longitude (degrees)
    C     SOUTH =  South latitude (degrees)
    C     EAST  =  East longitude (degrees)
    C
    C     For spherical harmonics:
    C     EW    =  0
    C     NS    =  0
    C     NORTH =  0
    C     WEST  =  0
    C     SOUTH =  0
    C     EAST  =  0
    C
    C
    C     Output
    C     ------
    C
    C     NORTH =  North latitude, adjusted if necessary (degrees)
    C     WEST  =  West longitude, adjusted if necessary (degrees)
    C     SOUTH =  South latitude, adjusted if necessary (degrees)
    C     EAST  =  East longitude, adjusted if necessary (degrees)
    */

    eckit::Log::info() << "++++++ areachk" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif

        if (*we == 0 || *ns == 0) { // Looks like mars call areachk for gaussian grids as well
            return 0;
        }

        ASSERT(*we > 0 && *ns > 0); // Only regular LL for now
        // This is not the code in EMOSLIB, just a guess
        double n = long(*north / *ns) * *ns;
        double s = long(*south / *ns) * *ns;
        double w = long(*west / *we) * *we;
        double e = long(*east / *we) * *we;

        if (*north != n) {
            n += *ns;
            if (n > 90) {
                n = 90;
            }
        }

        if (*south != s) {
            s += *ns;
            if (s < -90) {
                s = -90;
            }
        }

        if (*west != w) {
            w -= *we;
        }

        if (*east != e) {
            e += *we;
        }

        while (e > 360) {
            e -= 360;
            w -= 360;
        }

        while (e < -180) {
            e += 360;
            w += 360;
        }

        while(w > e) {
            w -= 360;
        }

        *north = n;
        *south = s;
        *west = w;
        *east = e;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}


extern "C" fortint emosnum_(fortint *value) {

    eckit::Log::info() << "++++++ emosnum" << std::endl;
    *value = 0;
    return 42424242;
}


}  // (anonymous namespace)
}  // namespace api
}  // namespace mir

