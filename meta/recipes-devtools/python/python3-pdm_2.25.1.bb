SUMMARY = "A modern Python package and dependency manager supporting the latest PEP standards"
HOMEPAGE = "https://pdm-project.org/latest/"
LICENSE = "MIT"
SECTION = "devel/python"
LIC_FILES_CHKSUM = "file://LICENSE;md5=2eb31a2cc1a758c34b499f287dd04ef2"

SRC_URI[sha256sum] = "3145251f37a3f94fa211ade992e7db7792da4c3e8e3eeeb1993d952adebe0b96"

inherit pypi python_setuptools_build_meta

DEPENDS += " \
    python3-pdm-backend-native \
    python3-pdm-build-locked-native \
"

BBCLASSEXTEND = "native nativesdk"
