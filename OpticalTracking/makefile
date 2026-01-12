########################################################################
# Makefile for the optical/inertial sensor fusion tracking package.
# Copyright (c) 2011-2026 Oliver Kreylos
# 
# This file is part of the optical/inertial sensor fusion tracking
# package.
# 
# The optical/inertial sensor fusion tracking package is free software;
# you can redistribute it and/or modify it under the terms of the GNU
# General Public License as published by the Free Software Foundation;
# either version 2 of the License, or (at your option) any later
# version.
# 
# The optical/inertial sensor fusion tracking package is distributed in
# the hope that it will be useful, but WITHOUT ANY WARRANTY; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# 
# You should have received a copy of the GNU General Public License
# along with the optical/inertial sensor fusion tracking package; if
# not, write to the Free Software Foundation, Inc., 59 Temple Place,
# Suite 330, Boston, MA 02111-1307 USA
########################################################################

# Directory containing the Vrui build system. The directory below
# matches the default Vrui installation; if Vrui's installation
# directory was changed during Vrui's installation, the directory below
# must be adapted.
VRUI_MAKEDIR = /opt/Vrui-14.0/share/make

# Base installation directory for this project. If set to the default of
# $(PROJECT_ROOT), the project does not need to be installed.
# Important note: Do not use ~ as an abbreviation for the user's home
# directory here; use $(HOME) instead.
INSTALLDIR = $(PROJECT_ROOT)

########################################################################
# Specify information about this project
########################################################################

# Name of the project
PROJECT_NAME = OpticalTracking

# The display name defaults to the project's name
PROJECT_DISPLAYNAME = Rift DK2 Optical Tracking

# Version number for dynamic libraries and installation subdirectories.
# This is used to keep subsequent release versions of the project from
# clobbering each other.
PROJECT_MAJOR = 1
PROJECT_MINOR = 0
PROJECT_BUILD = 0

# Include definitions for the system environment and system-provided
# packages
include $(VRUI_MAKEDIR)/SystemDefinitions
include $(VRUI_MAKEDIR)/Packages.System
include $(VRUI_MAKEDIR)/Packages.Vrui
include $(VRUI_MAKEDIR)/Configuration.Vrui

########################################################################
# Specify additional compiler and linker flags
########################################################################

########################################################################
# List common packages used by all components of this project
# (Supported packages can be found in $(VRUI_MAKEDIR)/Packages.*)
########################################################################

PACKAGES = 

########################################################################
# Specify all final targets
# Use $(EXEDIR)/ before executable names
########################################################################

ALL = $(EXEDIR)/PSMoveUtil \
      $(EXEDIR)/IMUCalibrator \
      $(EXEDIR)/IMUTest \
      $(EXEDIR)/ShowLEDs \
      $(EXEDIR)/LEDFinder

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

$(EXEDIR)/PSMoveUtil: PACKAGES += MYUSB LIBDBUS BLUETOOTH
$(EXEDIR)/PSMoveUtil: $(OBJDIR)/PSMoveUtil.o

IMUCALIBRATOR_SOURCES = IMU.cpp \
                        PSMove.cpp \
                        OculusRiftHIDReports.cpp \
                        OculusRift.cpp \
                        IMUTracker.cpp \
                        EllipsoidFitter.cpp \
                        IMUCalibrator.cpp

$(EXEDIR)/IMUCalibrator: PACKAGES += MYVRUI MYGLSUPPORT MYGEOMETRY MYMATH MYRAWHID MYIO MYTHREADS MYMISC
$(EXEDIR)/IMUCalibrator: $(IMUCALIBRATOR_SOURCES:%.cpp=$(OBJDIR)/%.o)

IMUTEST_SOURCES = IMU.cpp \
                  PSMove.cpp \
                  OculusRiftHIDReports.cpp \
                  OculusRift.cpp \
                  IMUTracker.cpp \
                  IMUTest.cpp

$(EXEDIR)/IMUTest: PACKAGES += MYVRUI MYGLSUPPORT MYGEOMETRY MYMATH MYRAWHID MYIO MYTHREADS MYMISC
$(EXEDIR)/IMUTest:  $(IMUTEST_SOURCES:%.cpp=$(OBJDIR)/%.o)

$(EXEDIR)/ShowLEDs: PACKAGES += MYVRUI MYGLGEOMETRY MYGLSUPPORT MYRAWHID
$(EXEDIR)/ShowLEDs: $(OBJDIR)/HMDModel.o \
                    $(OBJDIR)/ShowLEDs.o
.PHONY: ShowLEDs
ShowLEDs: $(EXEDIR)/ShowLEDs

$(EXEDIR)/LEDFinder: PACKAGES += MYVRUI MYVIDEO MYGLMOTIF MYIMAGES MYGLSUPPORT MYRAWHID MYIO
$(EXEDIR)/LEDFinder: $(OBJDIR)/OculusRiftHIDReports.o \
                     $(OBJDIR)/RiftLEDControl.o \
                     $(OBJDIR)/HMDModel.o \
                     $(OBJDIR)/LensDistortionParameters.o \
                     $(OBJDIR)/ModelTracker.o \
                     $(OBJDIR)/LEDFinder.o
.PHONY: LEDFinder
LEDFinder: $(EXEDIR)/LEDFinder

########################################################################
# Specify installation rules
########################################################################

install: $(ALL)
	@echo Installing $(PROJECT_DISPLAYNAME) in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(EXECUTABLEINSTALLDIR)
	@install $(EXECUTABLES) $(EXECUTABLEINSTALLDIR)
