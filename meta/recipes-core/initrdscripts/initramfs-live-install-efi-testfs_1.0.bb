SUMMARY = "Live image install script for with a second rootfs/kernel option"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
SRC_URI = "file://init-install-efi-testfs.sh"

RDEPENDS:${PN} = "parted e2fsprogs-mke2fs dosfstools"

S = "${UNPACKDIR}"

do_install() {
        install -m 0755 ${S}/init-install-efi-testfs.sh ${D}/install-efi.sh
}

INHIBIT_DEFAULT_DEPS = "1"
FILES:${PN} = " /install-efi.sh "
COMPATIBLE_HOST = "(i.86|x86_64).*-linux"
