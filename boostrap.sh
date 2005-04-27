#! /bin/bash

set -x
aclocal-1.9
autoheader-2.59
automake-1.9 --add-missing --copy
autoconf-2.59
