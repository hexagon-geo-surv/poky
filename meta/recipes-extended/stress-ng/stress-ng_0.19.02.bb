SUMMARY = "System load testing utility"
DESCRIPTION = "Deliberately simple workload generator for POSIX systems. It \
imposes a configurable amount of CPU, memory, I/O, and disk stress on the system."
HOMEPAGE = "https://github.com/ColinIanKing/stress-ng#readme"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/ColinIanKing/stress-ng.git;protocol=https;branch=master;tag=V${PV}"
SRCREV = "8d5399b282225f758606cd2b522382f65d947a8d"

DEPENDS = "coreutils-native libbsd"

PROVIDES = "stress"
RPROVIDES:${PN} = "stress"
RREPLACES:${PN} = "stress"
RCONFLICTS:${PN} = "stress"

inherit bash-completion

EXTRA_OEMAKE = "VERBOSE=1"

do_configure() {
    mkdir -p configs
    touch configs/HAVE_APPARMOR
    oe_runmake makeconfig
}

do_install() {
    oe_runmake DESTDIR=${D} BINDIR=${bindir} install
    ln -s stress-ng ${D}${bindir}/stress
}

# upstream issue: https://github.com/ColinIanKing/stress-ng/issues/315
DEBUG_BUILD = "0"
