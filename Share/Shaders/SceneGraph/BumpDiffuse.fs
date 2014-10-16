/***********************************************************************
BumpDiffuse.fs - Fragment shader for bump mapping and diffuse-only
lighting.
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

uniform sampler2D normalMap;
uniform sampler2D diffuseMap;

varying vec3 eyeVec;
varying vec3 lightVec;

void main()
	{
	/* Get the local normal vector from the normal map: */
	vec3 normal=normalize(texture2D(normalMap,gl_TexCoord[0].st).rgb-vec3(0.5));
	// vec3 normal=vec3(0.0,0.0,1.0);
	
	/* Get the surface base color from the diffuse map: */
	vec4 sDiffuse=texture2D(diffuseMap,gl_TexCoord[0].st);
	
	/* Calculate the ambient color component: */
	vec4 color=gl_LightSource[0].ambient*sDiffuse;
	
	/* Calculate the diffuse lighting term: */
	float lightDist=length(lightVec);
	vec3 nLightVec=normalize(lightVec);
	float nl=dot(normal,nLightVec);
	if(nl>0.0)
		{
		/* Calculate the diffuse color component: */
		color+=gl_LightSource[0].diffuse*sDiffuse*nl;
		}
	
	/* Attenuate the light source's color contribution: */
	float lightAtt=1.0/((gl_LightSource[0].quadraticAttenuation*lightDist+gl_LightSource[0].linearAttenuation)*lightDist+gl_LightSource[0].constantAttenuation);
	color*=lightAtt;
	
	/* Calculate the global ambient light term: */
	color+=gl_LightModel.ambient*sDiffuse;
	
	/* Set the fragment color: */
	color[3]=sDiffuse[3];
	gl_FragColor=color;
	}
