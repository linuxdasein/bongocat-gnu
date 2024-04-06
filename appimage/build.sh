#!/bin/bash

# NOTE: This script is designed to run in a docker container. Do not run directly
# The image for creating containers is appimagecrafters/appimage-builder:latest

# set correct permissions for /tmp (why were they broken in the first place?)
chmod 1777 /tmp

# install meson (python has more recent version than apt)
pip3 install meson

# update apt database
apt-get update

# install build tools for meson
apt install -y ninja-build

# install GCC 8 and cmake
apt install -y gcc-8 g++-8 cmake

# the default GCC 7 doesn't support some C++17 features, update to GCC 8
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 \
                    --slave /usr/bin/g++ g++ /usr/bin/g++-8

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

# configure meson build
meson setup build --buildtype=release

# build the application
cd build && meson compile

# install the application
DESTDIR=../AppDir meson install && cd ..

# install the app resources
cp -r ./img ./AppDir/usr/local/bin/
cp -r ./share ./AppDir/usr/local/bin/
ICONDIR=./AppDir/usr/share/icons/hicolor/
install -Dm755 ./appimage/bongo.sh ./AppDir/usr/local/bin/bongo.sh
install -Dm644 ./ico/bongo-16x16.png ${ICONDIR}/16x16/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./ico/bongo-24x24.png ${ICONDIR}/24x24/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./ico/bongo-32x32.png ${ICONDIR}/32x32/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./ico/bongo-48x48.png ${ICONDIR}/48x48/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./ico/bongo-170x170.png ${ICONDIR}/170x170/apps/com.linuxdasein.BongoCat.png

# build the appimage
appimage-builder --recipe=appimage/AppImageBuilder.yml
