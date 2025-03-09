#!/bin/bash

# NOTE: This script is designed to run in a docker container. Do not run directly
# The image for creating containers is appimagecrafters/appimage-builder:latest
set -e

# set correct permissions for /tmp (why were they broken in the first place?)
chmod 1777 /tmp

# update apt database
apt-get update

# install build tools for meson
apt install -y ninja-build python3.8 software-properties-common

# install GCC 9
add-apt-repository -y ppa:ubuntu-toolchain-r/test
apt install -y gcc-9 g++-9

# the default GCC 7 doesn't support some C++17 features, update to GCC 9
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 900 \
                    --slave /usr/bin/g++ g++ /usr/bin/g++-9

# we need more fresh python version to get latest meson
update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.6 1
update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 2
update-alternatives --set python3 /usr/bin/python3.8

# install meson (python has more recent version than apt)
pip3 install -I "meson>=1.1.0"

# here we go again: SFML-3.0 requires fresh cmake
wget https://github.com/Kitware/CMake/releases/download/v3.31.5/cmake-3.31.5-linux-x86_64.tar.gz
# unpack cmake binaries
tar -xzf cmake-3.31.5-linux-x86_64.tar.gz
# set and environment variable for new cmake executable
export CMAKE=$(pwd)/cmake-3.31.5-linux-x86_64/bin/cmake

# install the build dependencies
apt install -y libxrandr-dev libxdo-dev libjsoncpp-dev libgl-dev libudev-dev libxcursor-dev libxi-dev

# for some reason libxdo-dev misses a pkg-config file, create it manually
cp appimage/libxdo.pc /usr/share/pkgconfig/

# cxxopts package is not present in current docker image's repos
wget https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.2.0.tar.gz
# unpack the cxxopts sources
tar -xzf v3.2.0.tar.gz && cd cxxopts-3.2.0
# configure cxxopts
${CMAKE} . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
# install cxxopts
make && make install && cd ..

# cleanup: delete cxxopts sources
rm -rf v3.2.0.tar.gz cxxopts-3.2.0

# the SFML version in the apt repo is way outdated, we need to backport
# the latest version. i.e. download the source and build locally
wget https://github.com/SFML/SFML/archive/refs/tags/3.0.0.tar.gz
# unpack the SFML sources
tar -xzf 3.0.0.tar.gz && cd SFML-3.0.0
# configure the library's sources
${CMAKE} . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
           -DSFML_PKGCONFIG_INSTALL_PREFIX=/usr/share/pkgconfig \
           -DSFML_BUILD_NETWORK=false -DSFML_BUILD_AUDIO=false -DBUILD_SHARED_LIBS=ON
# build and install SFML
make && make install && cd ..

# cleanup: delete SFML and cmake sources
rm -rf 3.0.0.tar.gz SFML-3.0.0
rm -rf cmake-3.31.5-linux-x86_64.tar.gz cmake-3.31.5-linux-x86_64

APPDIR="$(pwd)/AppDir"

# configure meson build
meson setup build --buildtype=release \
        --prefix ${APPDIR}/usr/local/ \
        -Dicondir=${APPDIR}/usr/share/icons/hicolor/

# build the application
meson compile -C build

# install the application
meson install -C build

# revert back python to make appimage-builder work
update-alternatives --set python3 /usr/bin/python3.6

# build the appimage
appimage-builder --recipe=appimage/AppImageBuilder.yml
