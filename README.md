# Description

This project is a GNU/Linux fork of the [osu! Bongo Cat overlay](https://github.com/kuroni/bongocat-osu).

## Original description
An osu! Bongo Cat overlay with smooth paw movement and simple skinning ability, written in C++. Originally created by [HamishDuncanson](https://github.com/HamishDuncanson).

Download the program [here](https://github.com/kuroni/bongocat-osu/releases).

Hugs and kisses to [CSaratakij](https://github.com/CSaratakij) for creating the Linux port for this project!

Any suggestion and/or collaboration, especially that relating to sprites, is welcomed! Thank you!

[Original post](https://www.reddit.com/r/osugame/comments/9hrkte/i_know_bongo_cat_is_getting_old_but_heres_a_nicer/) by [Kuvster](https://github.com/Kuvster).

## License

The project is licensed under the GNU GPL v3 license (see [LICENSE](LICENSE)), however it contains parts from the original project licensed under
a MIT license. A copy of the full text of the original copyright and license is included in [LICENSE.ORIG.MIT](LICENSE.ORIG.MIT).

## Configuration
The application uses a config.json file for storing user settings. You can refer to the original project's [wiki](https://github.com/kuroni/bongocat-osu/wiki/Settings) 
to find out how to write a config.json of your own. There are the following directories, where a config file can be found:
- The directory the application was launched from. This file has higher priority than the other ones.
- The user's home config directory: `~/.config/bongocat-gnu/config.json`.
If no config file is found the application launches with the default settings.

## Further information
Press Ctrl + R to reload configuration and images (will only reload configurations when the window is focused).

## For developers
This project uses [SFML](https://www.sfml-dev.org/index.php) and [JsonCpp](https://github.com/open-source-parsers/jsoncpp).

### Dependencies

You need to have these dependencies installed. Check with your package manager for the exact name of these dependencies on your distro:
- g++
- libxdo
- sdl2
- jsoncpp
- sfml
- x11
- xrandr

### Building and testing

The project uses meson as the build system, which requires building in a separate directory.
For example, to build the application in directory `build`, run these commands from the base directory:

```sh
meson setup build
cd build
meson compile
```
By default meson setups a debug build, in order to make a release build use the following setup command:
```
meson setup build --buildtype=release
```

Next, you can copy the newly-compiled `build/bongo` into the base directory and execute it.

#### Archlinux
On Arch based distros you can also use this [PKGBUILD](Archlinux/PKGBUILD) to build a package from your local repo by running,
for instance, the following commands:
```
cd Archlinux
makepkg -fi
```

If you have troubles compiling, it can be due to version mismatch between your compiler and SFML. See [#43](https://github.com/kuroni/bongocat-osu/issues/43) for more information.

