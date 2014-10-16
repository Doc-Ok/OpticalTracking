/***********************************************************************
DiffuseLighting.vs - Vertex shader for per-fragment lighting using the
same formulae as fixed-functionality OpenGL. Now with simple texture
mapping goodness.
Copyright (c) 2007-2010 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

varying vec4 ambientGlobal,ambient,diffuse,specular;
varying vec3 normal,lightDir,halfVector;
varying float lightDist;

void main()
	{
	/* Calculate the texture coordinates for texture unit 0: */
	gl_TexCoord[0]=gl_MultiTexCoord0;
	
	/* Calculate global ambient light term: */
	ambientGlobal=gl_LightModel.ambient*gl_FrontMaterial.ambient;
	
	/* Calculate per-source ambient light term: */
	ambient=gl_LightSource[0].ambient*gl_FrontMaterial.ambient;
	
	/* Compute the normal vector: */
	normal=normalize(gl_NormalMatrix*gl_Normal);
	
	/* Compute the light direction (works both for directional and point lights): */
	vec4 vertexEc=gl_ModelViewMatrix*gl_Vertex;
	lightDir=gl_LightSource[0].position.xyz*vertexEc.w-vertexEc.xyz*gl_LightSource[0].position.w;
	lightDist=length(lightDir);
	lightDir=normalize(lightDir);
	
	/* Compute the per-source diffuse light term: */
	diffuse=gl_LightSource[0].diffuse*gl_FrontMaterial.diffuse;
	
	/* Compute the half-vector: */
	halfVector=normalize(normalize(-vertexEc.xyz)+lightDir);
	
	/* Compute the per-source specular light term: */
	specular=gl_LightSource[0].specular*gl_FrontMaterial.specular;
	
	/* Use standard vertex position: */
	gl_Position=ftransform();
	}
