from distutils.core import setup
from distutils.extension import Extension
import os
from Cython.Build import cythonize
from Cython.Distutils import build_ext

mir_lib_dir_default = "/var/tmp/mafr/builds/debug/mir/lib"
mir_include_dirs_default = ["/home/ma/mafr/dev/mir/src", "/var/tmp/mafr/builds/debug/mir/src",
                            "/home/ma/mafr/dev/eckit/src", "/var/tmp/mafr/builds/debug/eckit/src"]

mir_lib_dir = os.environ.get("MIR_LIB_DIR", mir_lib_dir_default)
mir_include_dirs = os.environ.get("MIR_INCLUDE_DIRS", ":".join(mir_include_dirs_default)).split(":")

print("MIR lib dir:", mir_lib_dir)
print("MIR include dirs:", " ".join(mir_include_dirs))

# FIXME: sort out how to discover MIR libraries
setup(
    name = 'mir',
    ext_modules = cythonize(
        Extension(
            "mir",
            ["mir.pyx", "pyio.cc"],
            language = "c++",
            libraries = ["mir"],
            library_dirs = [mir_lib_dir],
            runtime_library_dirs = [mir_lib_dir],
            include_dirs = mir_include_dirs,
            extra_compile_args = ["-std=c++11"],
            extra_link_args = ["-std=c++11"],
        ),
        compiler_directives={'language_level': 3, 'c_string_encoding': 'default'}
    )
)
