import platform
import sys

from setuptools import setup
from wheel.bdist_wheel import bdist_wheel

with open("VERSION", "r") as fVersion:
    version = fVersion.readlines()[0].strip()

install_requires = [
    f"mirlib=={version}",
    "eckit",  # NOTE we don't pin here, because mirlib is exactly pinned, which in turn exactly pins eckitlib, which in turn constraints eckit
    "findlibs",
    "numpy>=2.0,<3.0",  # NOTE may need tighter range in case of ABI issues. Dont forget to keep in sync with pre-compile.sh (or cmake files if numpy install refactored)
    "scipy",
    "pyyaml",
]


# NOTE see ci-utils/wheelmaker/buildscripts/setup_utils, we need to get the right abi compat tag
class bdist_wheel_ext(bdist_wheel):
    def get_tag(self):
        python, abi, plat = bdist_wheel.get_tag(self)  # noqa: F841
        return python, abi, f"manylinux_2_28_{platform.machine()}"


ext_kwargs = {
    "darwin": {},
    "linux": {"cmdclass": {"bdist_wheel": bdist_wheel_ext}},
}

setup(
    version=version,
    package_data={
        "": ["*.so"],
    },
    has_ext_modules=lambda: True,
    install_requires=install_requires,
    **ext_kwargs[sys.platform],
)
