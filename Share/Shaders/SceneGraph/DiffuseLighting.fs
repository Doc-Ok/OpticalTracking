/***********************************************************************
DiffuseLighting.fs - Fragment shader for per-fragment lighting using the
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

uniform sampler2D diffuseMap;

varying vec4 ambientGlobal,ambient,diffuse,specular;
varying vec3 normal,lightDir,halfVector;
varying float lightDist;

void main()
	{
	/* Calculate the per-source ambient light term: */
	vec4 color=ambient;
	
	/* Compute the diffuse lighting angle: */
	vec3 nNormal=normalize(normal);
	float nl=dot(nNormal,normalize(lightDir));
	vec4 specColor=vec4(0.0);
	if(nl>0.0)
		{
		/* Calculate the per-source diffuse light term: */
		color+=diffuse*nl;
		
		/* Calculate the specular lighting angle: */
		float nhv=max(dot(nNormal,normalize(halfVector)),0.0);
		specColor=specular*pow(nhv,gl_FrontMaterial.shininess);
		}
	
	/* Attenuate the per-source light terms: */
	float att=1.0/((gl_LightSource[0].quadraticAttenuation*lightDist+gl_LightSource[0].linearAttenuation)*lightDist+gl_LightSource[0].constantAttenuation);
	color*=att;
	specColor*=att;
	
	/* Calculate the global ambient light term: */
	color+=ambientGlobal;
	
	/* Get the texture color: */
	color*=texture2D(diffuseMap,gl_TexCoord[0].st);
	
	/* Add the separate specular color: */
	color.xyz+=specColor.xyz;
	
	/* Set the fragment color: */
	gl_FragColor=color;
	}
