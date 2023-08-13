#!/bin/bash

# NOTE: This script is designed to run in a docker container. Do not run directly
# The image for creating containers is appimagecrafters/appimage-builder:latest

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
apt install -y libxrandr-dev libxdo-dev libsdl2-dev libjsoncpp-dev

# for some reason libxdo-dev misses a pkg-config file, create it manually
cp appimage/libxdo.pc /usr/share/pkgconfig/

# the SFML version in the apt repo is way outdated, we need to backport
# the latest version. e.g. download the source and build locally
wget https://github.com/SFML/SFML/archive/refs/tags/2.6.0.tar.gz
# unpack the sources
tar -xzf 2.6.0.tar.gz && cd SFML-2.6.0
# configure the library's sources
cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
        -DSFML_PKGCONFIG_INSTALL_PREFIX=/usr/share/pkgconfig \
        -DSFML_BUILD_NETWORK=false -DSFML_BUILD_AUDIO=false
# build and install SFML
make && make install && cd ..

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
install -Dm755 ./appimage/bongo.sh ./AppDir/usr/local/bin
install -Dm644 ./flatpak/bongo-16x16.png ${ICONDIR}/16x16/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./flatpak/bongo-24x24.png ${ICONDIR}/24x24/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./flatpak/bongo-32x32.png ${ICONDIR}/32x32/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./flatpak/bongo-48x48.png ${ICONDIR}/48x48/apps/com.linuxdasein.BongoCat.png
install -Dm644 ./flatpak/bongo-170x170.png ${ICONDIR}/170x170/apps/com.linuxdasein.BongoCat.png

# create the AppImage
appimage-builder --recipe appimage/AppImageBuilder.yml
