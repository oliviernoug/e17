
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@EVIL_CFLAGS@

if BUILD_LOADER_BMP
if !EVAS_STATIC_BUILD_BMP

pkgdir = $(libdir)/evas/modules/loaders/bmp/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = evas_image_load_bmp.c

module_la_LIBADD = @EVAS_GENERAL_LIBS@ @EVIL_LIBS@ -lm $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_loader_bmp.la
libevas_loader_bmp_la_SOURCES = evas_image_load_bmp.c
libevas_loader_bmp_la_LIBADD =

endif
endif
