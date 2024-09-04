set -ex

ECCODES_VERSION=2.35.1

mkdir -p /src /target /build/eccodes /build/eckit /build/mir \
	&& cd /src \
	&& git clone https://github.com/ecmwf/ecbuild \
	&& git clone https://github.com/ecmwf/eckit \
	&& git clone https://github.com/ecmwf/mir \
	&& curl "https://confluence.ecmwf.int/download/attachments/45757960/eccodes-${ECCODES_VERSION}-Source.tar.gz?api=v2" --output eccodes.tar.gz && tar xzf eccodes.tar.gz

cd /build/eccodes \
	&& /src/ecbuild/bin/ecbuild "/src/eccodes-${ECCODES_VERSION}-Source/" -DENABLE_AEC=0 -DENABLE_PYTHON=0 -DENABLE_BUILD_TOOLS=0 -DENABLE_JPG_LIBJASPER=0 -DENABLE_MEMFS=0 -DENABLE_INSTALL_ECCODES_DEFINITIONS=1 -DENABLE_INSTALL_ECCODES_SAMPLES=1 -DCMAKE_INSTALL_PREFIX=/target \
	&& make -j10 \
	&& make install

cd /build/eckit \
	&& /src/ecbuild/bin/ecbuild --prefix=/target /src/eckit \
	&& make -j10 \
	&& make install

cd /build/mir \
	&& /src/ecbuild/bin/ecbuild --prefix=/target /src/mir \
	&& make -j10 \
	&& make install

cd /tmp && curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.cargo/env \
	&& uv python install python3.11 \
	&& uv venv \
	&& source .venv/bin/activate \
	&& uv pip install build \
	&& yum install -y clang

cd /src/mir-python \
	&& python -m build --installer uv .

# now eg
# uv pip install ./dist/mir_python-0.0.1-cp311-cp311-linux_x86_64.whl
# to validate, or twine and publish, etc
