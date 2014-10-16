########################################################################
# Makefile for the Vrui example programs.
# Copyright (c) 2003-2014 Oliver Kreylos
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
########################################################################

# Directory containing the Vrui build system. The directory below
# matches the default Vrui installation; if Vrui's installation
# directory was changed during Vrui's installation, the directory below
# must be adapted.
VRUI_MAKEDIR := /home/okreylos/Vrui-3.2/share/make
ifdef DEBUG
  VRUI_MAKEDIR := $(VRUI_MAKEDIR)/debug
endif

# Base installation directory for the example programs. If this is set
# to the default of $(PWD), the example programs do not have to be
# installed to be run. Created executables and resources will be
# installed in the bin and share directories under the given base
# directory, respectively.
# Important note: Do not use ~ as an abbreviation for the user's home
# directory here; use $(HOME) instead.
INSTALLDIR := $(shell pwd)

# Set resource directory:
RESOURCEDIR = share/VruiExamplePrograms

########################################################################
# Everything below here should not have to be changed
########################################################################

# Include definitions for the system environment and system-provided
# packages
include $(VRUI_MAKEDIR)/SystemDefinitions
include $(VRUI_MAKEDIR)/Packages.System
include $(VRUI_MAKEDIR)/Configuration.Vrui
include $(VRUI_MAKEDIR)/Packages.Vrui

# Set installation directory structure:
BININSTALLDIR = $(INSTALLDIR)/$(EXEDIR)
RESOURCEINSTALLDIR = $(INSTALLDIR)/$(RESOURCEDIR)

########################################################################
# List common packages used by all components of this project
# (Supported packages can be found in $(VRUI_MAKEDIR)/Packages.*)
########################################################################

PACKAGES = MYVRUI

########################################################################
# Specify all final targets
#
# Note: To build individual targets, run "make bin/<target name>"
# instead of just "make <target name>."
########################################################################

ALL = $(EXEDIR)/VruiDemo \
      $(EXEDIR)/VruiDemoSmall \
      $(EXEDIR)/VruiGLTest \
      $(EXEDIR)/VruiAppTemplate \
      $(EXEDIR)/VruiLocatorDemo \
      $(EXEDIR)/VruiEventToolDemo \
      $(EXEDIR)/VruiCustomToolDemo \
      $(EXEDIR)/FileSelection \
      $(EXEDIR)/VruiCalibrator \
      $(EXEDIR)/DrawEnvironment \
      $(EXEDIR)/PrecisionTest \
      $(EXEDIR)/VruiSceneGraphDemo \
      $(EXEDIR)/VruiSoundTest \
      $(EXEDIR)/ImageViewer \
      $(EXEDIR)/ImageSequenceViewer \
      $(EXEDIR)/VideoViewer \
      $(EXEDIR)/Animation \
      $(EXEDIR)/ShowEarthModel \
      $(EXEDIR)/Jello \
      $(EXEDIR)/ClusterJello \
      $(EXEDIR)/SharedJelloServer \
      $(EXEDIR)/SharedJello \
      $(EXEDIR)/VirtualClay

.PHONY: all
all: $(ALL)

########################################################################
# Specify other actions to be performed on a `make clean'
########################################################################

.PHONY: extraclean
extraclean:

.PHONY: extrasqueakyclean
extrasqueakyclean:

# Include basic makefile
include $(VRUI_MAKEDIR)/BasicMakefile

########################################################################
# Specify build rules for executables
########################################################################

#
# Simple single-source demo programs:
#

$(EXEDIR)/VruiDemo: $(OBJDIR)/VruiDemo.o

$(EXEDIR)/VruiDemoSmall: $(OBJDIR)/VruiDemoSmall.o

$(EXEDIR)/VruiGLTest: $(OBJDIR)/VruiGLTest.o

$(EXEDIR)/VruiAppTemplate: $(OBJDIR)/VruiAppTemplate.o

$(EXEDIR)/VruiLocatorDemo: $(OBJDIR)/VruiLocatorDemo.o

$(EXEDIR)/VruiEventToolDemo: $(OBJDIR)/VruiEventToolDemo.o

$(EXEDIR)/VruiCustomToolDemo: $(OBJDIR)/VruiCustomToolDemo.o

$(EXEDIR)/FileSelection: $(OBJDIR)/FileSelection.o

$(EXEDIR)/VruiCalibrator: $(OBJDIR)/VruiCalibrator.o

$(EXEDIR)/DrawEnvironment: $(OBJDIR)/DrawEnvironment.o

$(EXEDIR)/PrecisionTest: $(OBJDIR)/PrecisionTest.o

$(EXEDIR)/VruiSceneGraphDemo: $(OBJDIR)/VruiSceneGraphDemo.o

$(EXEDIR)/VruiSoundTest: $(OBJDIR)/VruiSoundTest.o

$(EXEDIR)/ImageViewer: $(OBJDIR)/ImageViewer.o

$(EXEDIR)/ImageSequenceViewer: $(OBJDIR)/ImageSequenceViewer.o

$(EXEDIR)/VideoViewer: $(OBJDIR)/VideoViewer.o

$(EXEDIR)/Animation: $(OBJDIR)/Animation.o

#
# ShowEarthModel, a viewer for earthquake catalogs and other assorted
# Earth science data.
#

SHOWEARTHMODEL_SOURCES = EarthFunctions.cpp \
                         PointSet.cpp \
                         SeismicPath.cpp \
                         EarthquakeSet.cpp \
                         EarthquakeTool.cpp \
                         EarthquakeQueryTool.cpp \
                         ShowEarthModel.cpp

$(OBJDIR)/ShowEarthModel.o: CFLAGS += -DSHOWEARTHMODEL_IMAGEDIR='"$(RESOURCEINSTALLDIR)"'

$(EXEDIR)/ShowEarthModel: $(SHOWEARTHMODEL_SOURCES:%.cpp=$(OBJDIR)/%.o)

#
# There's always room for Jell-O!
#

# Simple version (single threaded, non-cluster aware, not shared):
$(EXEDIR)/Jello: $(OBJDIR)/JelloAtom.o \
                 $(OBJDIR)/JelloCrystal.o \
                 $(OBJDIR)/JelloRenderer.o \
                 $(OBJDIR)/Jello.o

# Multithreaded and cluster-aware version:
$(EXEDIR)/ClusterJello: $(OBJDIR)/JelloAtom.o \
                        $(OBJDIR)/JelloCrystal.o \
                        $(OBJDIR)/JelloRenderer.o \
                        $(OBJDIR)/ClusterJello.o

# Client/server distributed and collaborative version:
# Override default package list -- the server does not need to link against Vrui
$(EXEDIR)/SharedJelloServer: PACKAGES = MYGLGEOMETRYWRAPPERS MYGEOMETRY MYMATH MYCOMM MYIO MYTHREADS MYMISC GL
$(EXEDIR)/SharedJelloServer: $(OBJDIR)/JelloAtom.o \
                             $(OBJDIR)/JelloCrystal.o \
                             $(OBJDIR)/SharedJelloServer.o

$(EXEDIR)/SharedJello: $(OBJDIR)/JelloAtom.o \
                       $(OBJDIR)/JelloCrystal.o \
                       $(OBJDIR)/JelloRenderer.o \
                       $(OBJDIR)/SharedJello.o

#
# Very simple virtual clay modeling application using a density volume
# and interactive isosurface extraction:
#

$(EXEDIR)/VirtualClay: $(OBJDIR)/EditableGrid.o \
                       $(OBJDIR)/GridEditor.o

# Rule to install the example programs in a destination directory
install: $(ALL)
	@echo Installing Vrui example programs in $(INSTALLDIR)...
	@install -d $(BININSTALLDIR)
	@install $(ALL) $(BININSTALLDIR)
	@install -d $(RESOURCEINSTALLDIR)
	@install $(RESOURCEDIR)/EarthTopography.png $(RESOURCEDIR)/EarthTopography.ppm $(RESOURCEINSTALLDIR)
