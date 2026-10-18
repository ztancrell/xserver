#!/bin/bash
# Build, test, and install this XLibre fork plus xf86-input-libinput to
# /usr/local, leaving the packaged xlibre-xserver in /usr as a fallback.
#
# Usage: ./install-xlibre.sh [--use-ly | --restore-ly]
#   --use-ly      point Ly's x_cmd at /usr/local/bin/X after installing
#   --restore-ly  point Ly back at the packaged /usr/bin/X and exit
set -euo pipefail

PREFIX=/usr/local
SRC=$(cd "$(dirname "$0")" && pwd)
LIBINPUT=${LIBINPUT:-$SRC/../xf86-input-libinput}
LY_CONF=/etc/ly/config.ini

set_ly() {
    sudo sed -i.bak "s|^x_cmd = .*|x_cmd = $1|" "$LY_CONF"
    grep '^x_cmd' "$LY_CONF"
}

case ${1:-} in
    --restore-ly) set_ly /usr/bin/X; exit ;;
    --use-ly|'') ;;
    *) echo "unknown option: $1" >&2; exit 1 ;;
esac

[ "$(id -u)" -ne 0 ] || { echo "run as your user; sudo is used for install" >&2; exit 1; }

cd "$SRC"
[ -d build ] || meson setup build --prefix=$PREFIX --buildtype=debugoptimized \
    -Dxorg=true -Dxephyr=false -Dxnest=false -Dxvfb=false -Dxfbdev=false -Dxfake=false \
    -Dsystemd_logind=true -Dseatd_libseat=false -Dsuid_wrapper=false -Dudev=true -Dglamor=true \
    -Dxkb_dir=/usr/share/X11/xkb -Dxkb_output_dir=/var/lib/xkb -Dxkb_bin_dir=/usr/bin \
    -Dlog_dir=/var/log -Ddocs=false
meson compile -C build
meson test -C build
sudo meson install -C build

# libinput must build against the server just installed, not the packaged one
cd "$LIBINPUT"
[ -d build ] || PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig meson setup build --prefix=$PREFIX
meson compile -C build
meson test -C build
sudo meson install -C build

"$PREFIX/bin/Xorg" -version 2>&1 | grep 'X Server'
ls "$PREFIX"/lib/xorg/modules/xlibre-25/{drivers/modesetting_drv.so,input/libinput_drv.so}

[ "${1:-}" = --use-ly ] && set_ly $PREFIX/bin/X
echo "done. fallback: $0 --restore-ly (or set x_cmd = /usr/bin/X in $LY_CONF from a TTY)"
