#!/bin/bash

# exit on errors
set -e

srcdir=$(dirname $0)
test -z "$srcdir" && $srcdir=.

autoreconf -ifv $srcdir
