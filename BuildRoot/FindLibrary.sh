#!/bin/bash
########################################################################
# Shell script to find include and library file directories for
# libraries that might be installed in non-standard places, or not be
# installed at all.
# Copyright (c) 2008-2011 Oliver Kreylos
# 
# This file is part of the WhyTools Build Environment.
# 
# The WhyTools Build Environment is free software; you can redistribute
# it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# The WhyTools Build Environment is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the WhyTools Build Environment; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
########################################################################

########################################################################
# Script argument list:
########################################################################

# $1 - name of header file to find
# $2 - name of library file to find (can be "None" to search for headers only)
# $3 - include directory ("include")
# $4 - library directory ("lib" or "lib64")
# $5-$n - base directories to search in order

# Store search parameters:
SEARCH_HEADER=$1
SEARCH_DSO=$2
INCLUDEEXT=$3
LIBEXT=$4

# Remove search parameters from command line:
shift ; shift ; shift ; shift

if [ $SEARCH_DSO != None ]
then
	# Search all given base directories for header and library files:
	for BASEDIR in $*
	do
		if [ -e $BASEDIR/$INCLUDEEXT/$SEARCH_HEADER ] # -a -e $BASEDIR/$LIBEXT/$SEARCH_DSO ] Yeah, still not working right
		then
			echo $BASEDIR
			exit 0
		fi
	done
else
	# Search all given base directories for header files:
	for BASEDIR in $*
	do
		if [ -e $BASEDIR/$INCLUDEEXT/$SEARCH_HEADER ]
		then
			echo $BASEDIR
			exit 0
		fi
	done
fi
