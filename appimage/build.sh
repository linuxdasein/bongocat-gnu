#!/bin/bash

# NOTE: This script is designed to run in a docker container. Do not run directly
# The image for creating containers is appimagecrafters/appimage-builder:latest
set -e

# set correct permissions for /tmp (why were they broken in the first place?)
chmod 1777 /tmp

# update apt database
apt-get update

# install build tools for meson
apt install -y ninja-build python3.8

# install GCC 8 and cmake
apt install -y gcc-8 g++-8 cmake

# the default GCC 7 doesn't support some C++17 features, update to GCC 8
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 \
                    --slave /usr/bin/g++ g++ /usr/bin/g++-8

# we need more fresh python version to get latest meson
update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.6 1
update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 2
update-alternatives --set python3 /usr/bin/python3.8

# install meson (python has more recent version than apt)
pip3 install -I "meson>=1.1.0"

# install the build dependencies
apt install -y libxrandr-dev libxdo-dev libjsoncpp-dev libgl-dev libudev-dev libxcursor-dev

# for some reason libxdo-dev misses a pkg-config file, create it manually
cp appimage/libxdo.pc /usr/share/pkgconfig/

# cxxopts package is not present in current docker image's repos
wget https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.2.0.tar.gz
# unpack the cxxopts sources
tar -xzf v3.2.0.tar.gz && cd cxxopts-3.2.0
# configure cxxopts
cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
# install cxxopts
make && make install && cd ..

# cleanup: delete cxxopts sources
rm -rf v3.2.0.tar.gz cxxopts-3.2.0

# the SFML version in the apt repo is way outdated, we need to backport
# the latest version. i.e. download the source and build locally
wget https://github.com/SFML/SFML/archive/refs/tags/2.6.1.tar.gz
# unpack the SFML sources
tar -xzf 2.6.1.tar.gz && cd SFML-2.6.1
# configure the library's sources
cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
        -DSFML_PKGCONFIG_INSTALL_PREFIX=/usr/share/pkgconfig \
        -DSFML_BUILD_NETWORK=false -DSFML_BUILD_AUDIO=false
# build and install SFML
make && make install && cd ..

# cleanup: delete SFML sources
rm -rf 2.6.1.tar.gz SFML-2.6.1

APPDIR="$(pwd)/AppDir"

# configure meson build
meson setup build --buildtype=release \
        --prefix ${APPDIR}/usr/local/ \
        -Dicondir=${APPDIR}/usr/share/icons/hicolor/

# build the application
meson compile -C build

# install the application
meson install -C build

# install the app wrapper script
install -Dm755 ./appimage/bongo.sh ${APPDIR}/usr/local/bin/bongo.sh

# revert back python to make appimage-builder work
update-alternatives --set python3 /usr/bin/python3.6

# build the appimage
appimage-builder --recipe=appimage/AppImageBuilder.yml
