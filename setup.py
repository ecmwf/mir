from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

# FIXME: sort out how to discover MIR libraries
setup(
    name = 'mir',
    ext_modules = cythonize(
        Extension(
            "mir",
            ["mir.pyx"],
            language = "c++",
            libraries = ["mir"],
            library_dirs = ["/var/tmp/mafr/builds/debug/mir/lib"],
            runtime_library_dirs = ["/var/tmp/mafr/builds/debug/mir/lib"],
            include_dirs = ["/home/ma/mafr/dev/mir/src", "/var/tmp/mafr/builds/debug/mir/src",
                            "/home/ma/mafr/dev/eckit/src", "/var/tmp/mafr/builds/debug/eckit/src"],
            extra_compile_args = ["-std=c++11"],
            extra_link_args = ["-std=c++11"],
            cmdclass = {'build_ext': build_ext}
        )
    )
)
