callsign2name
=============

A program to look up the name of a ham radio operator from their callsign using data from SDPPI and QRZ.

Usage
-----

    Usage: callsign2name [options]

    Options:
      -h [ --help ]                  Show help
      -c [ --callsign ] CALLSIGN     Callsign
      -l [ --callsign-list ] FILE    Callsign list file. One callsign per line. Output CSV file must be specified
      -o [ --output-csv ] FILE       Output CSV file
      -q [ --qrz-username ] USERNAME Your QRZ username (if you also want to use QRZ in addition to SDPPI)

    Example:
    callsign2name -c k7mds
    callsign2name -l callsigns.txt -o callsigns.csv -q yd0bjp

Building
--------

    # Install vcpkg. See https://vcpkg.io/en/getting-started.html

    vcpkg install boost-program-options boost-algorithm cpr nlohmann-json pugixml
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake ..
    cmake --build .

License
-------

    Copyright (C) 2022 Andika Wasisto

    callsign2name is free software: you can redistribute it and/or modify it under
    the terms of the GNU Affero General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    callsign2name is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
    more details.

    You should have received a copy of the GNU Affero General Public License
    along with callsign2name.  If not, see <https://www.gnu.org/licenses/>.