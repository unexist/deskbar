## Process this file with automake to produce Makefile.ins

plugins = cpu.la time.la

if HAVE_BAT
plugins += battery.la
endif

if HAVE_XMMS
plugins += xmms.la
endif

if HAVE_BMP
plugins += bmp.la
endif

if HAVE_DEBUG
plugins += oom.la
endif

lib_LTLIBRARIES = $(plugins)
libdir = $(prefix)/lib/@PACKAGE_NAME@

if HAVE_BAT
battery_la_SOURCES = battery.c
battery_la_LDFLAGS = -module -avoid-version
endif

if HAVE_XMMS
xmms_la_SOURCES = xmms.c
xmms_la_CFLAGS = $(shell xmms-config --cflags)
xmms_la_LDFLAGS = -module -avoid-version $(shell xmms-config --libs)
endif

if HAVE_BMP
bmp_la_SOURCES = bmp.c
bmp_la_CFLAGS = $(shell beep-config --cflags)
bmp_la_LDFLAGS = -module -avoid-version $(shell beep-config --libs)
endif

if HAVE_DEBUG
oom_la_SOURCES = oom.c
oom_la_LDFLAGS = -module -avoid-version
endif

cpu_la_SOURCES = cpu.c
cpu_la_LDFLAGS = -module -avoid-version

time_la_SOURCES = time.c
time_la_LDFLAGS = -module -avoid-version

AM_CFLAGS = -Wall -Wpointer-arith -Wstrict-prototypes -O2
