AM_CPPFLAGS = @VMOD_INCLUDES@

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_boltsort.la

libvmod_boltsort_la_LDFLAGS = -module -export-dynamic -avoid-version

libvmod_boltsort_la_SOURCES = \
	vcc_if.c \
	vcc_if.h \
	vmod_boltsort.c

vcc_if.c vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_boltsort.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_boltsort.vcc

VMOD_TESTS = tests/*.vtc
.PHONY: $(VMOD_TESTS)

tests/*.vtc:
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@

check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_boltsort.vcc \
	$(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c $(builddir)/vcc_if.h
