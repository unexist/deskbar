## Process this file with automake to produce Makefile.in

bin_PROGRAMS = deskbar

deskbar_SOURCES = \
	desk.c lpng.c main.c plug.c sig.c xml.c

deskbar_LDFLAGS = -Wl,--export-dynamic
deskbar_LDADD = $(top_builddir)/libdeskbar/libdeskbar.la

INCLUDES = -I$(top_srcdir)

noinst_HEADERS = \
	desk.h lpng.h plug.h sig.h xml.h

AM_CFLAGS = -Wall -Wpointer-arith -Wstrict-prototypes -O2
