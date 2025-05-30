SUMMARY = "Controls the configuration of serial ports"
DESCRIPTION = "setserial is a program designed to set and/or report the configuration information associated with a serial port"
HOMEPAGE = "http://setserial.sourceforge.net"
SECTION = "console/utils"

LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://version.h;beginline=1;endline=6;md5=2e7c59cb9e57e356ae81f50f4e4dfd99"

DEPENDS += "groff-native"

inherit autotools-brokensep sourceforge-releases

SRC_URI = "${SOURCEFORGE_MIRROR}/setserial/${BPN}-${PV}.tar.gz \
           file://add_stdlib.patch \
           file://ldflags.patch \
           file://0001-setserial.c-Add-needed-system-headers-for-ioctl-and-.patch \
           "

SRC_URI[sha256sum] = "7e4487d320ac31558563424189435d396ddf77953bb23111a17a3d1487b5794a"

do_install() {
    install -d ${D}${bindir}
    install -d ${D}${mandir}/man8

    install -m 0755 ${S}/setserial   ${D}${bindir}
    install -m 0644 ${S}/setserial.8 ${D}${mandir}/man8
}
