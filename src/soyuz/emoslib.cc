#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/runtime/Context.h"

typedef int fortint;
typedef double fortfloat;

#include "MIRJob.h"
#include "GribMemoryInput.h"
#include "GribMemoryOutput.h"


#include <memory>

std::auto_ptr<MIRJob> job(0);

extern "C" fortint intout_(char *name, fortint *ints, fortfloat *reals, const char *value, fortint, fortint) {
    try {

        if(!job.get()) {
            job.reset(new MIRJob());
        }

        if(strcasecmp(name, "grid") == 0) {
            eckit::StrStream os;
            os << reals[0] << "/" << reals[1] << eckit::StrStream::ends;
            job->set("grid", std::string(os));
            return 0;
        }

        if(strcasecmp(name, "area") == 0) {
            eckit::StrStream os;
            os << reals[0] << "/" << reals[1] << "/" << reals[2] << "/" << reals[3] << eckit::StrStream::ends;
            job->set("area", std::string(os));
            return 0;
        }

        if(strcasecmp(name, "gaussian") == 0) {
             // TODO:
            return 0;
        }

        if(strcasecmp(name, "reduced") == 0) {
            eckit::StrStream os;
            os << ints[0] << eckit::StrStream::ends;
            job->set("reduced", std::string(os));
            return 0;
        }

        if(strcasecmp(name, "truncation") == 0) {
            eckit::StrStream os;
            os << ints[0] << eckit::StrStream::ends;
            job->set("truncation", std::string(os));
            return 0;
        }

        if(strcasecmp(name, "regular") == 0) {
            eckit::StrStream os;
            os << ints[0] << eckit::StrStream::ends;
            job->set("regular", std::string(os));
            return 0;
        }

        if(strcasecmp(name, "rotation") == 0) {
            eckit::StrStream os;
            os << reals[0] << "/" << reals[1] << eckit::StrStream::ends;
            job->set("rotation", std::string(os));
            return 0;
        }

        if(strcasecmp(name, "autoresol") == 0) {
            job->set("autoresol", "1");
            return 0;
        }

        // if(strcasecmp(name, "resol") == 0) {
        //     job->set("resol", value);
        //     return 0;
        // }

        if(strcasecmp(name, "style") == 0) {
            job->set("style", value);
            return 0;
        }

        if(strcasecmp(name, "bitmap") == 0) {
            job->set("bitmap", value);
            return 0;
        }

        if(strcasecmp(name, "frame") == 0) {
            eckit::StrStream os;
            os << ints[0] << eckit::StrStream::ends;
            job->set("frame", std::string(os));
        }

        if(strcasecmp(name, "interpolation") == 0) {
            std::string low;
            const char* p = value;
            while(*p) {
                low += tolower(*p);
                p++;
            }
            job->set("interpolation", low);
            return 0;
        }

        throw eckit::SeriousBug(std::string("Unexpected name in INTOUT: ") + name);
        // job->set(

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intin_(char *name, fortint *ints, fortfloat *reals, const char *value, fortint, fortint) {
    try {
        eckit::Log::warning() << "INTIN not implemenent (ignored), name=" << name << std::endl;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intf_(char *, fortint *, fortfloat *, char *, fortint *, fortfloat *) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intf2(char *grib_in, fortint *length_in, char *grib_out, fortint *length_out) {
    try {

        if(!job.get()) {
            job.reset(new MIRJob());
        }

        GribMemoryInput input(grib_in, *length_in);
        GribMemoryOutput output(grib_out, *length_out);

        job->execute(input, output);

        ASSERT(output.interpolated() + output.saved() == 1);

        if(output.saved() == 1) {
            *length_out = 0; // Not interpolation performed
        }
        else {
            *length_out = output.length();
        }

    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intuvs2_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intuvp2_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intvect2_(char *u_grib_in, char *v_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intuvs_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intuvp_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intvect_(char *u_grib_in, char *v_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint iscrsz_() {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint ibasini_(fortint *force) {
    // Init interpolation package
    job.reset(0);
    return 0;
}

extern "C" void intlogm_(fortint (*)(char *, fortint)) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
    }
}

extern "C" void intlogs(void (*)(char *)) {
    // TODO: Register call back
    // Context::instance().behavior(
}

extern "C" fortint areachk_(fortfloat *ew, fortfloat *ns, fortfloat *north, fortfloat *west, fortfloat *south,
                            fortfloat *east) {
    try {
        eckit::Log::warning() << "AREACHK not implemenent (ignored)" << std::endl;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint emosnum_(fortint *value) {
    *value = 12345;
    return 0;
}
