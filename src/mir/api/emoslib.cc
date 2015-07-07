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
#include "eckit/io/StdFile.h"

#include "mir/api/MIRJob.h"
#include "mir/api/ProdgenJob.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/output/GribMemoryOutput.h"

#include "atlas/grids/GaussianLatitudes.h"

#include "mir/input/VODInput.h"
#include "mir/output/UVOutput.h"

#include "mir/input/WindInput.h"
#include "mir/output/WindOutput.h"

#include "mir/input/RawInput.h"
#include "mir/output/RawOutput.h"


#include "atlas/Grid.h"
#include "atlas/grids/grids.h"
#include "atlas/grids/GaussianLatitudes.h"

namespace mir {
namespace api {
namespace {


typedef int fortint;
typedef double fortfloat;


static eckit::ScopedPtr<ProdgenJob> intin(0);

static eckit::ScopedPtr<MIRJob> job(0);
static bool unpacked = false;

static void tidy(const char *in, char *out, size_t max) {
    size_t n = 0;
    while (*in && n < max - 1) {
        *out = (*in == ' ') ? '-' : tolower(*in);
        in++;
        out++;
        n++;
    }
    *out = 0;
}


static bool boolean(const char *in) {
    if (in[0] == 'n') return false;
    if (in[0] == 'y') return true;
    throw eckit::SeriousBug(std::string("Invalid boolean: ") + in);
}

extern "C" fortint intout_(const char *name, fortint *ints, fortfloat *reals,
                           const char *value, fortint name_len, fortint value_len) {
    std::string n(name);
    n = n.substr(0, name_len);
    eckit::Log::info() << "++++++ intout [" << n << "]" <<  std::endl;
    char buffer[1024];

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        if (strncasecmp(name, "grid", name_len) == 0) {
            if (reals[0] != 0 ||  reals[1] != 0) {
                job->set("grid", reals[0], reals[1]);
            } else {
                job->clear("grid");
            }
            return 0;
        }

        if (strncasecmp(name, "area", name_len) == 0) {
            if (reals[0] != 0 ||  reals[1] != 0 || reals[2] != 0 || reals[3] != 0 ) {
                job->set("area", reals[0], reals[1], reals[2], reals[3]);
            } else {
                job->clear("area");
            }
            return 0;
        }

        // if(strncasecmp(name, "gaussian") == 0) {
        //      // TODO:
        //     return 0;
        // }

        if (strncasecmp(name, "reduced", name_len) == 0) {
            job->set("reduced", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "truncation", name_len) == 0) {
            job->set("truncation", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "regular", name_len) == 0) {
            job->set("regular", long(ints[0]));
            return 0;
        }

        // TODO: Check that gaussian == regular in all cases
        if (strncasecmp(name, "gaussian", name_len) == 0) {
            job->set("regular", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "rotation", name_len) == 0) {
            job->set("rotation", double(reals[0]) , double(reals[1]));
            return 0;
        }

        if (strncasecmp(name, "autoresol", name_len) == 0) {
            job->set("autoresol", ints[0] != 0);
            return 0;
        }

        if (strncasecmp(name, "style", name_len) == 0) {
            job->set("style", value);
            return 0;
        }

        if (strncasecmp(name, "bitmap", name_len) == 0) {
            job->set("bitmap", value);
            return 0;
        }

        if (strncasecmp(name, "accuracy", name_len) == 0) {
            job->set("accuracy", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "frame", name_len) == 0) {
            job->set("frame", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "intermediate_gaussian", name_len) == 0) {
            job->set("intermediate_gaussian", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "interpolation", name_len) == 0) {
            tidy(value, buffer, sizeof(buffer));
            job->set("interpolation", buffer);
            return 0;
        }

        if (strncasecmp(name, "packing", name_len) == 0) {
            tidy(value, buffer, sizeof(buffer));
            job->set("packing", buffer);
            return 0;
        }

        if (strncasecmp(name, "form", name_len) == 0) {
            tidy(value, buffer, sizeof(buffer));
            if (strncasecmp(buffer, "unpacked", value_len) == 0) {
                unpacked = true;
                return 0;
            }
        }
        std::string v(value);
        v = v.substr(0, value_len);
        eckit::Log::info() << "INTOUT " << n << ", s=" << v << " - i[0]=" << ints[0] << " -r[0]=" << reals[0] << std::endl;
        throw eckit::SeriousBug(std::string("Unexpected name in INTOUT: [") + n + "]");

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intin_(const char *name, fortint *ints, fortfloat *reals, const char *value, fortint name_len, fortint value_len) {

    std::string n(name);
    n = n.substr(0, name_len);
    eckit::Log::info() << "++++++ intin [" << n << "]" <<  std::endl;    char buffer[1024];
    std::string v(value);
    v = v.substr(0, value_len);
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif

        eckit::Log::info() << "INTIN " << n << ", s=[" << v << "] - i[0]=" << ints[0] << " -r[0]=" << reals[0] << std::endl;


        if (!intin.get()) {
            intin.reset(new ProdgenJob());
        }

        if (strncasecmp(name, "usewind", name_len) == 0) {
            intin->usewind(boolean(value));
            return 0;
        }
        if (strncasecmp(name, "uselsm", name_len) == 0) {
            intin->uselsm(boolean(value));
            return 0;
        }

        if (strncasecmp(name, "useprecip", name_len) == 0) {
            intin->useprecip(boolean(value));
            return 0;
        }

        if (strncasecmp(name, "lsm_param", name_len) == 0) {
            intin->lsm_param(boolean(value));
            return 0;
        }

        if (strncasecmp(name, "parameter", name_len) == 0) {
            intin->parameter(ints[0]);
            return 0;
        }

        if (strncasecmp(name, "table", name_len) == 0) {
            intin->table(ints[0]);
            return 0;
        }

        if (strncasecmp(name, "reduced", name_len) == 0) {
            intin->reduced(ints[0]);
            return 0;
        }

        if (strncasecmp(name, "truncation", name_len) == 0) {
            intin->truncation(ints[0]);
            return 0;
        }

        if (strncasecmp(name, "g_pnts", name_len) == 0) {
            intin->g_pnts(ints);
            return 0;
        }

        if (strncasecmp(name, "missingvalue", name_len) == 0) {
            intin->missingvalue(boolean(value));
            return 0;
        }

        if (strncasecmp(name, "form", name_len) == 0) {
            tidy(value, buffer, sizeof(buffer));
            if (strncasecmp(buffer, "unpacked", value_len) == 0) {
                return 0;
            }
        }

        eckit::Log::info() << "INTIN " << n << ", s=" << v << " - i[0]=" << ints[0] << " -r[0]=" << reals[0] << std::endl;
        throw eckit::SeriousBug(std::string("Unexpected name in INTIN: [") + n + "]");

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint intf_(char *grib_in, fortint *length_in, fortfloat *values_in,
                         char *grib_out, fortint *length_out, fortfloat *values_out) {

    eckit::Log::info() << "++++++ intf in="  << *length_in << ", out=" << *length_out << std::endl;

    if (*length_in == 0) {
        // Prodgen seems to be doing that!
        return 0;
    }

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        ASSERT(unpacked); // Only for PRODGEN

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        if (!intin.get()) {
            intin.reset(new ProdgenJob());
        }

        mir::input::RawInput input(*intin, values_in, *length_in);
        mir::output::RawOutput output(values_out, *length_out);

        job->execute(input, output);

        *length_out = output.size();

        // unpacked = false;
        intin->reset();

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

        static const char *capture = getenv("MIR_CAPTURE_CALLS");
        if (capture) {
            std::ofstream out(capture);
            out << "mars<<EOF" << std::endl;
            out << "retrieve,target=in.grib,";
            input.marsRequest(out);
            out << std::endl;
            out << "EOF" << std::endl;
            job->mirToolCall(out);
            out << std::endl;
        }

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

        // Second order packing may return different sizes
        ::memset(u_grib_out, 0, *length_out);
        ::memset(v_grib_out, 0, *length_out);

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


        static const char *capture = getenv("MIR_CAPTURE_CALLS");
        if (capture) {
            std::ofstream out(capture);
            out << "mars<<EOF" << std::endl;
            out << "retrieve,target=in.grib,";
            vort_input.marsRequest(out);
            out << std::endl;
            out << "retrieve,target=in.grib,";
            div_input.marsRequest(out);
            out << std::endl;
            out << "EOF" << std::endl;
            job->mirToolCall(out);
            out << std::endl;
        }

        job->execute(input, output);

        job->clear("vod2uv");

        ASSERT(u_output.interpolated() + u_output.saved() == 1);
        ASSERT(v_output.interpolated() + v_output.saved() == 1);

        // If packing=so, u and v will have different sizes
        // ASSERT(u_output.length() == v_output.length());
        *length_out = std::max(u_output.length(), v_output.length());

        {
            eckit::StdFile f("debug.u", "w");
            fwrite(u_grib_out, 1, *length_out, f);
        }
        {
            eckit::StdFile f("debug.v", "w");
            fwrite(v_grib_out, 1, *length_out, f);
        }

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

        mir::input::WindInput input(vort_input, div_input);
        mir::output::WindOutput output(u_output, v_output);

        job->set("wind", true);

        static const char *capture = getenv("MIR_CAPTURE_CALLS");
        if (capture) {
            std::ofstream out(capture);
            out << "mars<<EOF" << std::endl;
            out << "retrieve,target=in.grib,";
            vort_input.marsRequest(out);
            out << std::endl;
            out << "retrieve,target=in.grib,";
            div_input.marsRequest(out);
            out << std::endl;
            out << "EOF" << std::endl;
            job->mirToolCall(out);
            out << std::endl;
        }

        job->execute(input, output);

        job->clear("wind");

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

        while (w > e) {
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

extern "C" void freecf_(fortint *flag) {
    // C     KFLAG - Flag indicating whether flushing of memory is done or not
    // C              = 1 to turn on flushing
    // C              = any other value to turn off flushing (default)
    eckit::Log::info() << "++++++ freecf flag=" << *flag << std::endl;
}

extern "C" void jvod2uv_(fortfloat *vor, fortfloat *div, fortint *ktin, fortfloat *u, fortfloat *v, fortint *ktout) {
    eckit::Log::info() << "++++++ jvod2uv" << std::endl;
    NOTIMP;
}

extern "C" void wv2dint_(fortint *knum, fortint *numpts, fortint *ke_w, fortint *kn_s, fortfloat *reson,
                         fortfloat *oldwave, fortfloat *newwave, fortfloat *nort, fortfloat *west,
                         fortint *knspec, fortfloat *pmiss, fortfloat *rns) {
    eckit::Log::info() << "++++++ wv2dint" << std::endl;
    NOTIMP;
}

extern "C" fortint jgglat_(fortint *KLAT, fortfloat *PGAUSS) {

    eckit::Log::info() << "++++++ jgglat " << *KLAT << std::endl;
    size_t N = *KLAT / 2;
    atlas::grids::gaussian_latitudes_npole_equator(N, PGAUSS);

    return 0;
}

extern "C" void jnumgg_(fortint *knum, char *htype, fortint *kpts, fortint *kret, fortint htype_len) {
    eckit::Log::info() << "++++++ jnumgg " << htype[0] << " " << *knum << std::endl;

    *kret = 0;
#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        eckit::ScopedPtr<atlas::grids::ReducedGrid> grid(0);

        if (htype[0] == 'R') {
            eckit::StrStream os;
            os << "rgg.N" << *knum << eckit::StrStream::ends;
            grid.reset(dynamic_cast<atlas::grids::ReducedGrid *>(atlas::Grid::create(std::string(os))));
        }

        if (htype[0] == 'F') {
            grid.reset(dynamic_cast<atlas::grids::ReducedGrid *>(new atlas::grids::GaussianGrid(*knum)));
        }

        ASSERT(grid.get());

        const std::vector<int> &v = grid->npts_per_lat();
        for (size_t i = 0; i < v.size(); i++) {
            kpts[i] = v[i];
        }

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        *kret = -2;
    }
#endif
}

extern "C" fortint wvqlint_(fortint *, fortint *, fortint *, fortint *, fortfloat *, fortfloat *, fortfloat *, fortfloat *, fortfloat *, fortint *, fortfloat *, fortfloat *) {

    eckit::Log::info() << "++++++ wvqlint" << std::endl;
    NOTIMP;
}

extern "C" fortint hirlam_( fortint *l12pnt, fortfloat *oldfld, fortint *kount, fortint *kgauss,
                            fortfloat *area, fortfloat *pole, fortfloat *grid, fortfloat *newfld,
                            fortint *ksize, fortint *nlon, fortint *nlat) {
    eckit::Log::info() << "++++++ hirlam" << std::endl;

    // C     L12PNT - Chooses between 12-point and 4-point interpolation
    // C              = .TRUE. for 12-point horizontal
    // C              = .FALSE. for 4-point
    // C     OLDFLD  - The array of values from the gaussian field
    // C     KOUNT   - Number of values in OLDFLD
    // C     KGAUSS  - Gaussian number for the gaussian field
    // C     AREA    - Limits of output area (N/W/S/E)
    // C     POLE    - Pole of rotation (lat/long)
    // C     GRID    - Output lat/long grid increments (we/ns)
    // C     KSIZE   - The size of the output array to fill with the regular
    // C               lat/long field


#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        // ASSERT(unpacked); // Only for PRODGEN

        // HIRLAM routines do not respect INTIN/INTOUT

        eckit::ScopedPtr<ProdgenJob> intin(new ProdgenJob());
        eckit::ScopedPtr<MIRJob> job(new MIRJob());

        mir::input::RawInput input(*intin, oldfld, *kount);
        mir::output::RawOutput output(newfld, *ksize);

        intin->reduced(*kgauss);
        intin->auto_pl();

        job->set("area", area[0], area[1], area[2], area[3]);
        job->set("grid", grid[0], grid[1]);
        job->set("rotation", pole[0], pole[1]);

        job->execute(input, output);

        size_t ni = 0;
        size_t nj = 0;
        output.shape(ni, nj);

        *nlon = nj;
        *nlat = ni;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint hirlsm_( fortint *l12pnt, fortfloat *oldfld, fortint *kount, fortint *kgauss,
                            fortfloat *area, fortfloat *pole, fortfloat *grid, fortfloat *newfld,
                            fortint *ksize, fortint *nlon, fortint *nlat) {
    eckit::Log::info() << "++++++ hirlsm" << std::endl;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    try {
#endif
        // ASSERT(unpacked); // Only for PRODGEN

        // HIRLAM routines do not respect INTIN/INTOUT

        eckit::ScopedPtr<ProdgenJob> intin(new ProdgenJob());
        eckit::ScopedPtr<MIRJob> job(new MIRJob());

        mir::input::RawInput input(*intin, oldfld, *kount);
        mir::output::RawOutput output(newfld, *ksize);

        intin->reduced(*kgauss);
        intin->auto_pl();

        job->set("area", area[0], area[1], area[2], area[3]);
        job->set("grid", grid[0], grid[1]);
        job->set("rotation", pole[0], pole[1]);
        job->set("interpolation", "nn");

        job->execute(input, output);

        size_t ni = 0;
        size_t nj = 0;
        output.shape(ni, nj);

        *nlon = nj;
        *nlat = ni;

#ifdef EMOSLIB_CATCH_EXCECPTIONS
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
#endif
    return 0;
}

extern "C" fortint hirlamw_(fortint *l12pnt, fortfloat *oldfldu, fortfloat *oldfldv, fortint *kount, fortint *kgauss,
                            fortfloat *area, fortfloat *pole, fortfloat *grid, fortfloat *newfldu, fortfloat *newfldv,
                            fortint *ksize, fortint *nlon, fortint *nlot) {
    eckit::Log::info() << "++++++ hirlamw" << std::endl;



    NOTIMP;
}

}  // (anonymous namespace)
}  // namespace api
}  // namespace mir

