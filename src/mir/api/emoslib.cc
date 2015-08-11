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

static void clear(MIRJob &job) {
    job.clear("grid");
    job.clear("truncation");
    job.clear("gaussian");
    job.clear("regular");
    job.clear("reduced");
}

extern "C" fortint intout_(const char *name,
                           const fortint ints[],
                           const fortfloat reals[],
                           const char *value,
                           const fortint name_len,
                           const fortint value_len) {
    std::string n(name);
    n = n.substr(0, name_len);
    eckit::Log::info() << "++++++ intout [" << n << "]" <<  std::endl;
    char buffer[1024];

    try {

        if (!job.get()) {
            job.reset(new MIRJob());
        }


        if (strncasecmp(name, "area", name_len) == 0) {
            if (reals[0] != 0 ||  reals[1] != 0 || reals[2] != 0 || reals[3] != 0 ) {
                job->set("area", reals[0], reals[1], reals[2], reals[3]);
            } else {
                job->clear("area");
            }
            return 0;
        }

        if (strncasecmp(name, "grid", name_len) == 0) {
            clear(*job);
            if (reals[0] != 0 ||  reals[1] != 0) {
                job->set("grid", reals[0], reals[1]);
            } else {
                job->clear("grid");
            }
            return 0;
        }

        if (strncasecmp(name, "reduced", name_len) == 0) {
            clear(*job);
            job->set("reduced", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "truncation", name_len) == 0) {
            clear(*job);
            job->set("truncation", long(ints[0]));
            return 0;
        }

        if (strncasecmp(name, "regular", name_len) == 0) {
            clear(*job);
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

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}

extern "C" fortint intin_(const char *name,
                          const fortint ints[],
                          const fortfloat reals[],
                          const char *value,
                          const fortint name_len,
                          const fortint value_len) {

    std::string n(name);
    n = n.substr(0, name_len);

    std::string v(value);
    v = v.substr(0, value_len);

    eckit::Log::info() << "++++++ intin [" << n << "] v=[" <<  v << "] r=" << reals[0] << " i=" << ints[0] << std::endl;

    char buffer[1024];

    try {

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
            if (strncasecmp(value, "no", 2) == 0) {
                intin->hasMissing(false);
            } else {
                intin->missingValue(reals[0]);
            }
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

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;

}

extern "C" fortint intf_(const void *grib_in,
                         const fortint &length_in,
                         const fortfloat values_in[],
                         void *grib_out,
                         fortint &length_out,
                         fortfloat values_out[]) {

    eckit::Log::info() << "++++++ intf in="  << length_in << ", out=" << length_out << std::endl;

    try {

        ASSERT(unpacked); // Only for PRODGEN

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        if (!intin.get()) {
            intin.reset(new ProdgenJob());
        }

        mir::input::RawInput input(*intin, values_in, length_in);
        mir::output::RawOutput output(values_out, length_out);

        job->execute(input, output);

        length_out = output.size();

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intf2(const void *grib_in,
                         const fortint &length_in,
                         void *grib_out,
                         fortint &length_out) {

    eckit::Log::info() << "++++++ intf2" << std::endl;

    try {

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        mir::input::GribMemoryInput input(grib_in, length_in);
        mir::output::GribMemoryOutput output(grib_out, length_out);

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
            length_out = 0; // Not interpolation performed
        } else {
            length_out = output.length();
        }

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}

extern "C" fortint intuvs2_(char *vort_grib_in,
                            char *div_grib_in,
                            const fortint &length_in,
                            char *u_grib_out,
                            char *v_grib_out,
                            const fortint &length_out) {

    eckit::Log::info() << "++++++ intuvs2" << std::endl;

    try {
        NOTIMP;

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intuvp2_(const void *vort_grib_in,
                            const void *div_grib_in,
                            const fortint &length_in,
                            void *u_grib_out,
                            void *v_grib_out,
                            fortint &length_out) {

    eckit::Log::info() << "++++++ intuvp2" << std::endl;

    try {

        // Second order packing may return different sizes
        ::memset(u_grib_out, 0, length_out);
        ::memset(v_grib_out, 0, length_out);

        if (!job.get()) {
            job.reset(new MIRJob());
        }

        mir::input::GribMemoryInput vort_input(vort_grib_in, length_in);
        mir::input::GribMemoryInput div_input(div_grib_in, length_in);


        mir::output::GribMemoryOutput u_output(u_grib_out, length_out);
        mir::output::GribMemoryOutput v_output(v_grib_out, length_out);

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
        length_out = std::max(u_output.length(), v_output.length());

        // {
        //     eckit::StdFile f("debug.u", "w");
        //     fwrite(u_grib_out, 1, *length_out, f);
        // }
        // {
        //     eckit::StdFile f("debug.v", "w");
        //     fwrite(v_grib_out, 1, *length_out, f);
        // }


    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}

extern "C" fortint intvect2_(const void *u_grib_in,
                             const void *v_grib_in,
                             const fortint &length_in,
                             void *u_grib_out,
                             void *v_grib_out,
                             fortint &length_out) {

    eckit::Log::info() << "++++++ intvect2" << std::endl;

    try {
        if (!job.get()) {
            job.reset(new MIRJob());
        }

        mir::input::GribMemoryInput vort_input(u_grib_in, length_in);
        mir::input::GribMemoryInput div_input(v_grib_in, length_in);

        mir::output::GribMemoryOutput u_output(u_grib_out, length_out);
        mir::output::GribMemoryOutput v_output(v_grib_out, length_out);

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

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}

extern "C" fortint intuvs_(const void *vort_grib_in,
                           const void *div_grib_in,
                           const fortint &length_in,
                           void *u_grib_out,
                           void *v_grib_out,
                           fortint &length_out) {

    eckit::Log::info() << "++++++ intuvs" << std::endl;

    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}

extern "C" fortint intuvp_(const void *vort_grib_in,
                           const void *div_grib_in,
                           const fortint &length_in,
                           void *u_grib_out,
                           void *v_grib_out,
                           fortint &length_out) {

    eckit::Log::info() << "++++++ intuvp" << std::endl;

    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intvect_(const void *u_grib_in,
                            const void *v_grib_in,
                            const fortint &length_in,
                            void *u_grib_out,
                            void *v_grib_out,
                            fortint &length_out) {

    eckit::Log::info() << "++++++ intvect" << std::endl;

    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint iscrsz_() {

    eckit::Log::info() << "++++++ iscrsz" << std::endl;

    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint ibasini_(const fortint &force) {

    eckit::Log::info() << "++++++ ibasini" << std::endl;

    // Init interpolation package
    job.reset(0);
    intin.reset(0);

    return 0;
}

extern "C" void intlogm_(fortint (*)(char *, fortint)) {

    eckit::Log::info() << "++++++ intlogm" << std::endl;

    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        throw;
    }
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

extern "C" fortint areachk_(const fortfloat &we,
                            const fortfloat &ns,
                            fortfloat &north,
                            fortfloat &west,
                            fortfloat &south,
                            fortfloat &east) {


    eckit::Log::info() << "++++++ areachk" << std::endl;

    try {

        if (we == 0 || ns == 0) { // Looks like mars call areachk for gaussian grids as well
            return 0;
        }

        ASSERT(we > 0 && ns > 0); // Only regular LL for now
        // This is not the code in EMOSLIB, just a guess
        double n = long(north / ns) * ns;
        double s = long(south / ns) * ns;
        double w = long(west / we) * we;
        double e = long(east / we) * we;

        if (north != n) {
            n += ns;
            if (n > 90) {
                n = 90;
            }
        }

        if (south != s) {
            s += ns;
            if (s < -90) {
                s = -90;
            }
        }

        if (west != w) {
            w -= we;
        }

        if (east != e) {
            e += we;
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

        north = n;
        south = s;
        west = w;
        east = e;

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}

extern "C" fortint emosnum_(fortint &value) {

    eckit::Log::info() << "++++++ emosnum" << std::endl;
    value = 0;
    return 42424242;
}

extern "C" void freecf_(const fortint &flag) {
    // C     KFLAG - Flag indicating whether flushing of memory is done or not
    // C              = 1 to turn on flushing
    // C              = any other value to turn off flushing (default)
    eckit::Log::info() << "++++++ freecf flag=" << flag << std::endl;
}

extern "C" void jvod2uv_(const fortfloat vor[],
                         const fortfloat div[],
                         const fortint &ktin,
                         fortfloat u[],
                         fortfloat v[],
                         const fortint &ktout) {
    eckit::Log::info() << "++++++ jvod2uv in=" << ktin << ", out=" << ktout << std::endl;
    try {

        if (!intin.get()) {
            intin.reset(new ProdgenJob());
        }

        MIRJob job;

        size_t size_in = ((ktin) + 1) * ((ktin) + 2) / 2;
        size_t size_out = ((ktout) + 1) * ((ktout) + 2) / 2;
        //ASSERT(ktin == ktout);

        intin->truncation(ktin);
        // job->set("truncation", (long)ktout);

        mir::input::RawInput vort_input(*intin, vor, size_in * 2);
        mir::input::RawInput div_input(*intin, div, size_in * 2);

        mir::output::RawOutput u_output(u, size_out * 2);
        mir::output::RawOutput v_output(v, size_out * 2);

        mir::input::VODInput input(vort_input, div_input);
        mir::output::UVOutput output(u_output, v_output);

        job.set("vod2uv", true);
        job.set("truncation", long(ktout));

        job.execute(input, output);
        intin->truncation(ktout);


    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        throw;
    }
}



extern "C" fortint jgglat_(const fortint &KLAT, fortfloat PGAUSS[]) {

    eckit::Log::info() << "++++++ jgglat " << KLAT << std::endl;
    size_t N = KLAT / 2;
    atlas::grids::gaussian_latitudes_npole_equator(N, PGAUSS);

    return 0;
}

extern "C" void jnumgg_(const fortint &knum,
                        const char *htype,
                        fortint kpts[],
                        fortint &kret,
                        fortint htype_len) {

    eckit::Log::info() << "++++++ jnumgg " << htype[0] << " " << knum << std::endl;

    kret = 0;
    try {
        eckit::ScopedPtr<atlas::grids::ReducedGrid> grid(0);

        if (htype[0] == 'R') {
            std::ostringstream os;
            os << "rgg.N" << knum;
            grid.reset(dynamic_cast<atlas::grids::ReducedGrid *>(atlas::Grid::create(os.str())));
        }

        if (htype[0] == 'F') {
            grid.reset(dynamic_cast<atlas::grids::ReducedGrid *>(new atlas::grids::GaussianGrid(knum)));
        }

        ASSERT(grid.get());

        const std::vector<int> &v = grid->npts_per_lat();
        for (size_t i = 0; i < v.size(); i++) {
            kpts[i] = v[i];
        }

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        kret = -2;
    }

}

extern "C" fortint wvqlint_(const fortint &knum,
                            const fortint numpts[],
                            const fortint &ke_w,
                            const fortint &kn_s,
                            const fortfloat &reson,
                            const fortfloat oldwave[],
                            fortfloat newwave[],
                            const fortfloat &north,
                            const fortfloat &west,
                            const fortint &kparam,
                            const fortfloat &pmiss,
                            const fortfloat &rns) {
    //     C     KNUM    - No. of meridians from North to South pole (input field)
    // C     NUMPTS  - Array giving number of points along each latitude
    // C               (empty latitudes have entry 0)
    // C     KE_W    - First dimension of new array
    // C               = Number of points E-W in new grid
    // C     KN_S    - Second dimension of new array
    // C               = Number of points N-S in new grid
    // C     RESON   - Output grid resolution (degrees)
    // C     OLDWAVE - Original wave field
    // C     NORTH   - Input and output grid northernmost latitude (degree)
    // C     WEST    - Input and output grid westernmost  longitude (degree)
    // C     KPARAM  - Field parameter code
    // C     PMISS   - Missing value indicator
    // C     RNS     - Difference in degrees in NS disrection
    eckit::Log::info() << "++++++ wvqlint knum=" << knum
                       << ", ke_w=" << ke_w
                       << ", kn_s=" << kn_s
                       << ", reson=" << reson
                       << ", north=" << north
                       << ", west=" << west
                       << ", kparam=" << kparam
                       << ", pmiss=" << pmiss
                       << ", rns=" << rns
                       << std::endl;
    try {

        // Only global for now
        ASSERT(north == 90);
        ASSERT(west == 0);


        ProdgenJob intin;
        MIRJob job;

        size_t size = 0;
        for (size_t i = 0; i < knum; i++) {
            size += numpts[i];
        }

        mir::input::RawInput input(intin, oldwave, size);
        mir::output::RawOutput output(newwave, ke_w * kn_s);

        job.set("grid", reson, reson);
        // job.set("area", north, west, north - (kn_s - 1) * reson, west + (ke_w - 1) * reson);

        intin.missingValue(pmiss);
        intin.reduced_ll(knum, numpts);

        job.execute(input, output);


    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" void wv2dint_(const fortint &knum,
                         const fortint numpts[],
                         const fortint &ke_w,
                         const fortint &kn_s,
                         const fortfloat &reson,
                         const fortfloat oldwave[],
                         fortfloat newwave[],
                         const fortfloat &north,
                         const fortfloat &west,
                         const fortint &knspec, // <== What is that?
                         const fortfloat &pmiss,
                         const fortfloat &rns) {

    eckit::Log::info() << "++++++ wv2dint knum=" << knum
                       << ", ke_w=" << ke_w
                       << ", kn_s=" << kn_s
                       << ", reson=" << reson
                       << ", north=" << north
                       << ", west=" << west
                       << ", knspec=" << knspec
                       << ", pmiss=" << pmiss
                       << ", rns=" << rns
                       << std::endl;
    try {

        // Only global for now
        ASSERT(north == 90);
        ASSERT(west == 0);


        ProdgenJob intin;
        MIRJob job;

        size_t size = 0;
        for (size_t i = 0; i < knum; i++) {
            size += numpts[i];
        }

        mir::input::RawInput input(intin, oldwave, size);
        mir::output::RawOutput output(newwave, ke_w * kn_s);

        job.set("grid", reson, reson);
        job.set("interpolation", "nn");
        // job.set("area", north, west, north - (kn_s - 1) * reson, west + (ke_w - 1) * reson);

        intin.missingValue(pmiss);
        intin.reduced_ll(knum, numpts);

        job.execute(input, output);
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        throw;
    }

}

extern "C" fortint hirlam_( const fortint &l12pnt,
                            const fortfloat oldfld[],
                            const fortint &kount,
                            const fortint &kgauss,
                            const fortfloat area[],
                            const fortfloat pole[],
                            const fortfloat grid[],
                            fortfloat newfld[],
                            const fortint &ksize,
                            fortint &nlon,
                            fortint &nlat) {

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


    try {

        // HIRLAM routines do not respect INTIN/INTOUT

        ProdgenJob intin;
        MIRJob job;

        mir::input::RawInput input(intin, oldfld, kount);
        mir::output::RawOutput output(newfld, ksize);

        intin.reduced(kgauss);
        intin.auto_pl();

        job.set("area", area[0], area[1], area[2], area[3]);
        job.set("grid", grid[0], grid[1]);
        job.set("rotation", pole[0], pole[1]);

        job.execute(input, output);

        size_t ni = 0;
        size_t nj = 0;
        output.shape(ni, nj);

        nlon = nj;
        nlat = ni;

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}

extern "C" fortint hirlsm_( const fortint &l12pnt,
                            const fortfloat oldfld[],
                            const fortint &kount,
                            const fortint &kgauss,
                            const fortfloat area[],
                            const fortfloat pole[],
                            const fortfloat grid[],
                            fortfloat newfld[],
                            const fortint &ksize,
                            fortint &nlon,
                            fortint &nlat) {

    eckit::Log::info() << "++++++ hirlsm" << std::endl;

    try {
        // ASSERT(unpacked); // Only for PRODGEN

        // HIRLAM routines do not respect INTIN/INTOUT

        ProdgenJob intin;
        MIRJob job;

        mir::input::RawInput input(intin, oldfld, kount);
        mir::output::RawOutput output(newfld, ksize);

        intin.reduced(kgauss);
        intin.auto_pl();

        job.set("area", area[0], area[1], area[2], area[3]);
        job.set("grid", grid[0], grid[1]);
        job.set("rotation", pole[0], pole[1]);
        job.set("lsm", true);

        job.execute(input, output);

        size_t ni = 0;
        size_t nj = 0;
        output.shape(ni, nj);

        nlon = nj;
        nlat = ni;

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint hirlamw_(const fortint &l12pnt,
                            const fortfloat oldfldu[],
                            const fortfloat oldfldv[],
                            const fortint &kount,
                            const fortint &kgauss,
                            const fortfloat area[],
                            const fortfloat pole[],
                            const fortfloat grid[],
                            fortfloat newfldu[],
                            fortfloat newfldv[],
                            const fortint &ksize,
                            fortint &nlon,
                            fortint &nlat) {
    eckit::Log::info() << "++++++ hirlamw" << std::endl;

    try {
        ProdgenJob u_intin;
        ProdgenJob v_intin;

        MIRJob job;

        mir::input::RawInput u_input(u_intin, oldfldu, kount);
        mir::input::RawInput v_input(v_intin, oldfldv, kount);

        mir::output::RawOutput u_output(newfldu, ksize);
        mir::output::RawOutput v_output(newfldv, ksize);

        mir::input::WindInput input(u_input, v_input);
        mir::output::WindOutput output(u_output, v_output);

        u_intin.reduced(kgauss);
        u_intin.auto_pl();
        u_intin.parameter(131);

        v_intin.reduced(kgauss);
        v_intin.auto_pl();
        v_intin.parameter(132);

        job.set("area", area[0], area[1], area[2], area[3]);
        job.set("grid", grid[0], grid[1]);
        job.set("rotation", pole[0], pole[1]);
        job.set("wind", true);
        // job.set("interpolation", "nn");

        job.execute(input, output);

        size_t ni = 0;
        size_t nj = 0;
        u_output.shape(ni, nj);

        nlon = nj;
        nlat = ni;

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

}  // (anonymous namespace)
}  // namespace api
}  // namespace mir

