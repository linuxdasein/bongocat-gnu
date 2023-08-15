#! /bin/bash
# This script is designed to run in a docker container. Do not run directly
# inside docker execute this script from the parent directory; i.e. ./archlinux/build.sh

# install the essential toolchain for building
pacman -Syu --noconfirm --needed base-devel

# makepkg does not allow running as root
useradd builder -m

# allow the builder to install packages
echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# change ownership and go to the build directory
chown -R builder . && cd archlinux

# build and install the package
sudo -H -u builder makepkg -si --noconfirm
