########################################################################
# Makefile for MeshEditor Vrui example program, using Vrui's legacy
# build system.
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

# Set the Vrui installation directory:
VRUISHAREDIR := /home/okreylos/Vrui-3.2/share

# Set up additional flags for the C++ compiler:
CFLAGS = 

# Include the Vrui application makefile fragment:
ifdef DEBUG
  # Build debug version of the applications, using the debug version of Vrui:
  include $(VRUISHAREDIR)/Vrui.debug.makeinclude
  CFLAGS += -g2 -O0
else
  # Build release version of the applications, using the release version of Vrui:
  include $(VRUISHAREDIR)/Vrui.makeinclude
  CFLAGS += -g0 -O2
endif

# List all project targets:
ALL = VRMeshEditor
.PHONY: all
all: $(ALL)

# Rule to remove all targets:
.PHONY: clean
clean:
	-rm -f $(ALL)

# Build the mesh editor program:
VRMeshEditor: PolygonMesh.cpp \
              PlyFileStructures.cpp \
              MeshGenerators.cpp \
              CatmullClark.cpp \
              AutoTriangleMesh.cpp \
              BallPivoting.cpp \
              SphereRenderer.cpp \
              Influence.cpp \
              MorphBox.cpp \
              BallPivotLocator.cpp \
              VertexRemovalLocator.cpp \
              FaceRemovalLocator.cpp \
              MeshDragger.cpp \
              MorphBoxDragger.cpp \
              VRMeshEditor.cpp
	g++ -o $@ -I. $(VRUI_CFLAGS) $(CFLAGS) $^ $(VRUI_LINKFLAGS)
