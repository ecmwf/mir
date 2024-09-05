set -ex
set -o pipefail

# run in the base image
yum install -y clang
alias python=/opt/python/cp311-cp311/bin/python

mkdir -p /src /target /build/eccodes /build/eckit /build/atlas /build/mir \
	&& cd /src \
	&& git clone --depth=1 --branch=master https://github.com/ecmwf/ecbuild \
	&& git clone --depth=1 --branch=2.36.0 https://github.com/ecmwf/eccodes \
	&& git clone --depth=1 --branch=1.27.0 https://github.com/ecmwf/eckit \
	&& git clone --depth=1 --branch=0.37.0 https://github.com/ecmwf/atlas \
	&& git clone --depth=1 --branch=1.22.0 https://github.com/ecmwf/mir \

cd /build/eccodes \
	&& /src/ecbuild/bin/ecbuild --prefix=/target /src/eccodes -DENABLE_AEC=0 -DENABLE_PYTHON=0 -DENABLE_BUILD_TOOLS=0 -DENABLE_JPG_LIBJASPER=0 -DENABLE_MEMFS=1 -DENABLE_INSTALL_ECCODES_DEFINITIONS=0 -DENABLE_INSTALL_ECCODES_SAMPLES=0 \
	&& make -j10 \
	&& make test \
	&& make install

cd /build/eckit \
	&& /src/ecbuild/bin/ecbuild --prefix=/target /src/eckit \
	&& make -j10 \
	&& make test \
	&& make install

cd /build/atlas \
	&& /src/ecbuild/bin/ecbuild --prefix=/target /src/atlas \
	&& make -j10 \
	&& make test \
	&& make install


cd /build/mir \
	&& /src/ecbuild/bin/ecbuild --prefix=/target /src/mir -DENABLE_MIR_DOWNLOAD_MASKS=OFF \
	&& make -j10 \
	&& make test \
	&& make install

cd /src/mir-python \
	&& SOURCE_LIB_ROOT=/target/ /opt/python/cp311-cp311/bin/python -m build --installer uv -w .

# now eg
# uv pip install ./dist/mir_python-0.0.1-cp311-cp311-linux_x86_64.whl
# to validate, or twine and publish, etc
