## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
deskbarincludedir = $(prefix)/include/libdeskbar

lib_LTLIBRARIES = libdeskbar.la
libdeskbar_la_LDFLAGS = -export-dynamic

libdeskbar_la_SOURCES =\
	list.c log.c htable.c mem.c string.c times.c

deskbarinclude_HEADERS = \
	list.h log.h htable.h mem.h string.h times.h plug.h
