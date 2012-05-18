dnl Lines with "dnl" are comments...

PHP_ARG_ENABLE(
	groundctrl,
	whether to enable Ground(CTRL) support, 
	[ --enable-groundctrl	Enable Ground(CTRL) Support ]
)

dnl - NOTE: the var below MUST start with PHP_extname or the ./configure command will not function 
if test "$PHP_GROUNDCTRL" = "yes"; then
	dnl AC_DEFINE(HAVE_GROUNDCTRL, 1, [Whether you have GroundCTRL])
	
	dnl - This defines the extension
	PHP_NEW_EXTENSION(groundctrl, groundctrl.c, $ext_shared)
	
	case $build_os in
	darwin1*.*.*)
		AC_MSG_CHECKING([whether to compile for recent osx architectures])
		CFLAGS="$CFLAGS -arch i386 -arch x86_64 -mmacosx-version-min=10.5"
		AC_MSG_RESULT([yes])
		;;
	darwin*)
		AC_MSG_CHECKING([whether to compile for every osx architecture ever])
		CFLAGS="$CFLAGS -arch i386 -arch x86_64 -arch ppc -arch ppc64"
		AC_MSG_RESULT([yes])
		;;
	esac
fi
