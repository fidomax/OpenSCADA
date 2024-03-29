AC_PREREQ([2.50])

AC_INIT([Tmpl],[0.0.1],[my@email.org])
AC_PREFIX_DEFAULT([/usr])
oscd_modulpref="oscd_"

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.10 subdir-objects])
AM_CONFIG_HEADER(confMod.h)

AC_LANG(C++)

AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(yes)

# Checking for the programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

if test "$ac_cv_prog_cc_g" = "no" -o "$ac_cv_prog_cxx_g" = "no"; then
    AC_MSG_ERROR([No C or C++ compiler present. Install GCC and G++.]);
fi
AC_CHECK_PROG(have_pkg_config, pkg-config, yes, no)
if test x"$have_pkg_config" = "xno"; then
    AC_MSG_ERROR(pkg-config is required to install this program)
fi
PKG_PROG_PKG_CONFIG

AM_SANITY_CHECK

# Checking for the header files
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([time.h signal.h semaphore.h fcntl.h math.h errno.h syslog.h unistd.h dirent.h stdio.h stdarg.h stdlib.h getopt.h dlfcn.h locale.h langinfo.h sys/types.h sys/stat.h sys/time.h sys/utsname.h pthread.h],[],
    [AC_MSG_ERROR(Some GLibC headers not found. Install or check GLibC developing package!)])
AC_CHECK_HEADERS([string vector map exception typeinfo deque],[],
    [AC_MSG_ERROR([Some standard template libs' STL headers not found. Install or check STL-StdC++ developing package!])])
AC_CHECK_HEADERS([libintl.h],[ ],[AC_MSG_NOTICE(libintl.h header not found. Using is disabled!)])

# Checking for the Gettext utilites
if test $ac_cv_header_libintl_h = yes; then
    AC_CHECK_PROG(XGETTEXT, xgettext, xgettext, false)
    AC_CHECK_PROG(MSGMERGE, msgmerge, msgmerge, false)
    AC_CHECK_PROG(MSGFMT, msgfmt, msgfmt, false)
    if test $XGETTEXT = false -o $MSGMERGE = false -o $MSGFMT = false; then
	AC_MSG_ERROR([Some gettext I18N utilite no present! Install gettext utilites package.]);
    fi
    AC_CHECK_LIB([intl],[bindtextdomain],[LIB_CORE="${LIB_CORE}-lintl "])
fi

# Checking for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

AC_SUBST(LIB_CORE)

# Checking for library functions
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_ISC_POSIX

oscd_moduldir="\${libdir}/openscada"
AC_SUBST(oscd_moduldir)
AC_SUBST(oscd_modulpref)

AC_CONFIG_FILES(Makefile)

#====== Generic functions enable/disable ======
AC_ARG_ENABLE(CrossCompile,AC_HELP_STRING([--enable-CrossCompile], [enable crosscompile build]),[ ],
[ if test $host = $build; then enable_CrossCompile=no; else enable_CrossCompile=yes; fi])
if test $enable_CrossCompile = no; then
    AC_FUNC_MALLOC
    AC_FUNC_REALLOC
fi

PKG_CHECK_MODULES([openscada],[openscada >= 0.8.0],
    [ CXXFLAGS="$CXXFLAGS $openscada_CFLAGS"; LDFLAGS="$LDFLAGS $openscada_LIBS"; ],
    [AC_MSG_ERROR(OpenSCADA core resources not found. Install or check OpenSCADA development package!)])

AM_CONDITIONAL([TmplIncl],[test])

AC_OUTPUT
