#!/bin/sh
# SGDK's own mkfiles/makefile.gen hardcodes
# `nm --plugin=liblto_plugin-0.dll` for its symbol.txt rule -- a
# Windows-toolchain DLL that doesn't exist on Linux. Plain nm reads a
# linked ELF's symbols fine without it, so strip the flag and pass the
# rest straight through to the real nm.
args=""
for a in "$@"; do
    case "$a" in
        --plugin=*) ;;
        *) args="$args $a" ;;
    esac
done
exec "${GENDEV:-/opt/gendev}/sgdk/bin/nm" $args
