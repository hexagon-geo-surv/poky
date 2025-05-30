SUMMARY = "Free peer-reviewed portable C++ source libraries"
DESCRIPTION = "Provides free peer-reviewed portable C++ source libraries.  The emphasis is on libraries which work well with the C++ \
Standard Library.  One goal is to establish 'existing practice' and \
provide reference implementations so that the Boost libraries are suitable for eventual standardization.  Some of the libraries have already been proposed for inclusion in the C++ Standards Committee's \
upcoming C++ Standard Library Technical Report."
SECTION = "libs"
DEPENDS = "boost-build-native zlib bzip2"

CVE_PRODUCT = "boost:boost"

ARM_INSTRUCTION_SET:armv4 = "arm"
ARM_INSTRUCTION_SET:armv5 = "arm"

B = "${WORKDIR}/build"
do_configure[cleandirs] = "${B}"

BOOST_LIBS = "\
	atomic \
	charconv \
	chrono \
	container \
	context \
	contract \
	coroutine \
	date_time \
	exception \
	fiber \
	filesystem \
	graph \
	headers \
	iostreams \
	json \
	log \
	math \
	process \
	program_options \
	random \
	regex \
	serialization \
	system \
	test \
	thread \
	timer \
	type_erasure \
	url \
	wave \
	${@bb.utils.filter('PACKAGECONFIG', 'locale python', d)} \
	${@bb.utils.contains('PACKAGECONFIG', 'graph_parallel', 'graph_parallel mpi', \
			     bb.utils.filter('PACKAGECONFIG', 'mpi', d), d)} \
	"

# optional libraries
PACKAGECONFIG ??= "locale python"
PACKAGECONFIG[locale] = ",,icu"
PACKAGECONFIG[graph_parallel] = ",,,boost-mpi mpich"
PACKAGECONFIG[mpi] = ",,mpich"
PACKAGECONFIG[python] = ",,python3"

inherit python3-dir
PYTHON_ROOT = "${STAGING_DIR_HOST}/${prefix}"

# Make a package for each library, plus -dev
PACKAGES = "${PN}-dbg ${BOOST_PACKAGES}"
python __anonymous () {
    packages = []
    extras = []
    pn = d.getVar("PN")
    mlprefix = d.getVar("MLPREFIX")
    for lib in d.getVar('BOOST_LIBS').split():
        extras.append("--with-%s" % lib)
        pkg = "%s-%s" % (d.getVar("BPN"), lib.replace("_", "-"))
        if "-native" in pn:
            pkg = pkg + "-native"
        packages.append(mlprefix + pkg)
        if not d.getVar("FILES:%s%s" % (mlprefix, pkg)):
                d.setVar("FILES:%s%s" % (mlprefix, pkg), "${libdir}/libboost_%s*.so.*" % lib)

    d.setVar("BOOST_PACKAGES", " ".join(packages))
    d.setVar("BJAM_EXTRA", " ".join(extras))
}

# Override the contents of specific packages
FILES:${PN}-graph_parallel = "${libdir}/libboost_graph_parallel.so.*"
FILES:${PN}-locale = "${libdir}/libboost_locale.so.*"
FILES:${PN}-mpi = "${libdir}/mpi.so ${libdir}/libboost_mpi*.so.*"
FILES:${PN}-serialization = "${libdir}/libboost_serialization*.so.* \
	${libdir}/libboost_wserialization*.so.*"
FILES:${PN}-test = "${libdir}/libboost_prg_exec_monitor*.so.* \
	${libdir}/libboost_unit_test_framework*.so.*"

# -dev last to pick up the remaining stuff
PACKAGES += "${PN}-dev ${PN}-staticdev"
FILES:${PN}-dev = "${includedir} ${libdir}/libboost_*.so ${libdir}/cmake"
FILES:${PN}-staticdev = "${libdir}/libboost_*.a"

# "boost" is a metapackage which pulls in all boost librabries
PACKAGES += "${PN}"
FILES:${PN} = ""
ALLOW_EMPTY:${PN} = "1"
RRECOMMENDS:${PN} += "${BOOST_PACKAGES}"
RRECOMMENDS:${PN}:class-native = ""

# to avoid GNU_HASH QA errors added LDFLAGS to ARCH; a little bit dirty but at least it works
TARGET_CC_ARCH += "${LDFLAGS}"

# Oh yippee, a new build system, it's sooo cooool I could eat my own
# foot.  inlining=on lets the compiler choose, I think.  At least this
# stuff is documented...
# NOTE: if you leave <debug-symbols>on then in a debug build the build sys
# objcopy will be invoked, and that won't work.  Building debug apparently
# requires hacking gcc-tools.jam
#
# Sometimes I wake up screaming.  Famous figures are gathered in the nightmare,
# Steve Bourne, Larry Wall, the whole of the ANSI C committee.  They're just
# standing there, waiting, but the truely terrifying thing is what they carry
# in their hands.  At first sight each seems to bear the same thing, but it is
# not so for the forms in their grasp are ever so slightly different one from
# the other.  Each is twisted in some grotesque way from the other to make each
# an unspeakable perversion impossible to perceive without the onset of madness.
# True insanity awaits anyone who perceives all of these horrors together.
#
# Quotation marks, there might be an easier way to do this, but I can't find
# it.  The problem is that the user.hpp configuration file must receive a
# pre-processor macro defined as the appropriate string - complete with "'s
# around it.  (<> is a possibility here but the danger to that is that the
# failure case interprets the < and > as shell redirections, creating
# random files in the source tree.)
#
#bjam: '-DBOOST_PLATFORM_CONFIG=\"config\"'
#do_compile: '-sGCC=... '"'-DBOOST_PLATFORM_CONFIG=\"config\"'"
SQD = '"'
EQD = '\"'
#boost.bb:   "...  '-sGCC=... '${SQD}'-DBOOST_PLATFORM_CONFIG=${EQD}config${EQD}'${SQD} ..."
BJAM_CONF = "${SQD}'-DBOOST_PLATFORM_CONFIG=${EQD}boost/config/platform/${TARGET_OS}.hpp${EQD}'${SQD}"

BJAM_TOOLS   = "--ignore-site-config \
		'-sTOOLS=gcc' \
		'-sGCC=${CC} '${BJAM_CONF} \
		'-sGXX=${CXX} '${BJAM_CONF} \
		'-sGCC_INCLUDE_DIRECTORY=${STAGING_INCDIR}' \
		'-sGCC_STDLIB_DIRECTORY=${STAGING_LIBDIR}' \
		'-sBUILD=release <optimization>space <threading>multi <inlining>on <debug-symbols>off' \
		'-sPYTHON_ROOT=${PYTHON_ROOT}' \
		'--layout=system' \
		"

# use PARALLEL_MAKE to speed up the build
BOOST_PARALLEL_MAKE = "${@oe.utils.parallel_make_argument(d, '-j%d')}"
BJAM_OPTS    = '${BOOST_PARALLEL_MAKE} -d+2 -q \
		${BJAM_TOOLS} \
		-sBOOST_BUILD_USER_CONFIG=${WORKDIR}/user-config.jam \
		-sICU_PATH=${STAGING_EXECPREFIXDIR} \
		--build-dir=${B} \
		--disable-icu \
		${BJAM_EXTRA}'

# Native compilation of bzip2 isn't working
BJAM_OPTS:append:class-native = ' -sNO_BZIP2=1'

# Adjust the build for x32
BJAM_OPTS:append:x86-x32 = " abi=x32 address-model=64"

# cross compiling for arm fails to detect abi, so provide some help
BJAM_OPTS:append:arm = " abi=aapcs architecture=arm"
BJAM_OPTS:append:aarch64 = " abi=aapcs address-model=64 architecture=arm"

do_configure() {
	cd ${S}
	cp -f ${S}/boost/config/platform/linux.hpp ${S}/boost/config/platform/linux-gnueabi.hpp

	# D2194:Fixing the failure of "error: duplicate initialization of gcc with the following parameters" during compilation.
	rm -f ${WORKDIR}/user-config.jam
	echo 'using gcc : : ${CXX} : <cflags>"${CFLAGS}" <cxxflags>"${CXXFLAGS}" <linkflags>"${LDFLAGS}" ;' >> ${WORKDIR}/user-config.jam

	# If we want Python then we need to tell Boost *exactly* where to find it
	if ${@bb.utils.contains('BOOST_LIBS', 'python', 'true', 'false', d)}; then
		echo "using python : ${PYTHON_BASEVERSION} : ${STAGING_DIR_HOST}${bindir}/python3 : ${STAGING_DIR_HOST}${includedir}/${PYTHON_DIR}${PYTHON_ABI} : ${STAGING_DIR_HOST}${libdir}/${PYTHON_DIR} ;" >> ${WORKDIR}/user-config.jam
	fi

	if ${@bb.utils.contains('BOOST_LIBS', 'mpi', 'true', 'false', d)}; then
		echo "using mpi : : <find-shared-library>mpi ;" >> ${WORKDIR}/user-config.jam
	fi

	CC="${BUILD_CC}" CFLAGS="${BUILD_CFLAGS}" ./bootstrap.sh --with-bjam=b2 --with-toolset=gcc

	# Boost can't be trusted to find Python on it's own, so remove any mention
	# of it from the boost configuration
	sed -i '/using python/d' ${S}/project-config.jam
}

do_compile() {
	cd ${S}
	b2 ${BJAM_OPTS} \
		--prefix=${prefix} \
		--exec-prefix=${exec_prefix} \
		--libdir=${libdir} \
		--includedir=${includedir} \
		--debug-configuration
}

do_install() {
	cd ${S}
	b2 ${BJAM_OPTS} \
		--libdir=${D}${libdir} \
		--includedir=${D}${includedir} \
		install
	for lib in ${BOOST_LIBS}; do
		if [ -e ${D}${libdir}/libboost_${lib}.a ]; then
			ln -s libboost_${lib}.a ${D}${libdir}/libboost_${lib}-mt.a
		fi
		if [ -e ${D}${libdir}/libboost_${lib}.so ]; then
			ln -s libboost_${lib}.so ${D}${libdir}/libboost_${lib}-mt.so
		fi
	done

        # Cmake files reference full paths to image
        find ${D}${libdir}/cmake -type f | \
             grep 'cmake$' | \
             xargs -n 1 sed -e 's,${D}${libdir}/cmake,${libdir}/cmake,' -i

}

BBCLASSEXTEND = "native nativesdk"
