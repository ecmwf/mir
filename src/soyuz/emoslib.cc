#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

typedef long fortint;
typedef double fortfloat;

extern "C" fortint intout_(char *, fortint *, fortfloat *, const char *, fortint, fortint) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint intin_(char *, fortint *, fortfloat *, const char *, fortint, fortint) {
    try {
        NOTIMP;
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
        NOTIMP;
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

extern "C" fortint ibasini_(fortint *) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" void intlogm_(fortint (*)(char *, fortint)) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
    }
}

extern "C" fortint areachk_(fortfloat *, fortfloat *, fortfloat *, fortfloat *, fortfloat *,
                            fortfloat *) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}

extern "C" fortint emosnum_(fortint *) {
    try {
        NOTIMP;
    } catch (std::exception &e) {
        eckit::Log::error() << "EMOSLIB/MIR wrapper: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}
