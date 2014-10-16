/***********************************************************************
BumpMapping.vs - Vertex shader for per-fragment bump mapping using
tangent-space computations.
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

attribute vec3 tangentS;
attribute vec3 tangentT;

varying vec3 eyeVec;
varying vec3 lightVec;

void main()
	{
	/* Copy the texture coordinate for all images: */
	gl_TexCoord[0]=gl_MultiTexCoord0;
	
	/* Compute the tangent space frame in eye coordinates: */
	vec3 eNormal=normalize(gl_NormalMatrix*gl_Normal);
	vec3 eTangentS=normalize((gl_ModelViewMatrix*vec4(tangentS,0.0)).xyz); // Important: tangents transform like vectors!
	vec3 eTangentT=normalize((gl_ModelViewMatrix*vec4(tangentT,0.0)).xyz); // Important: tangents transform like vectors!
	
	/* Compute the vertex position in eye coordinates: */
	vec4 eVertex=gl_ModelViewMatrix*gl_Vertex;
	
	/* Compute the eye vector in tangent space: */
	vec3 eEyeVec=-eVertex.xyz/eVertex.w;
	eyeVec.x=dot(eEyeVec,eTangentS);
	eyeVec.y=dot(eEyeVec,eTangentT);
	eyeVec.z=dot(eEyeVec,eNormal);
	
	/* Compute the light vector in tangent space: */
	vec3 eLightVec=gl_LightSource[0].position.xyz*eVertex.w-eVertex.xyz*gl_LightSource[0].position.w;
	eLightVec/=gl_LightSource[0].position.w*eVertex.w;
	lightVec.x=dot(eLightVec,eTangentS);
	lightVec.y=dot(eLightVec,eTangentT);
	lightVec.z=dot(eLightVec,eNormal);
	
	/* Transform the vertex: */
	gl_Position=ftransform();
	}
