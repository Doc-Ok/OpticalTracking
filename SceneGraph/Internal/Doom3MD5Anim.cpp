/***********************************************************************
Doom3MD5Anim - Class to represent animation sequences for animated mesh
models in Doom3's MD5 format.
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

#include <SceneGraph/Internal/Doom3MD5Anim.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Misc/Utility.h>
#include <Misc/ThrowStdErr.h>
#include <SceneGraph/Internal/Doom3FileManager.h>
#include <SceneGraph/Internal/Doom3ValueSource.h>
#include <SceneGraph/Internal/Doom3MD5Mesh.h>

namespace SceneGraph {

/*****************************
Methods of class Doom3MD5Anim:
*****************************/

Doom3MD5Anim::Doom3MD5Anim(Doom3FileManager& fileManager,const char* animFileName)
	:numFrames(0),
	 numJoints(0),
	 joints(0),
	 numAnimatedComponents(0),
	 frameComponents(0)
	{
	/* Check if the animation file name has an extension: */
	const char* extPtr=0;
	for(const char* mfnPtr=animFileName;*mfnPtr!='\0';++mfnPtr)
		if(*mfnPtr=='.')
			extPtr=mfnPtr;
	
	/* Add the .md5anim extension to the mesh file name: */
	char fileNameBuffer[1024];
	if(extPtr==0)
		{
		snprintf(fileNameBuffer,sizeof(fileNameBuffer),"%s.md5anim",animFileName);
		animFileName=fileNameBuffer;
		}
	
	/* Open the animation file and create a tokenizer for it: */
	Doom3ValueSource source(fileManager.getFile(animFileName),animFileName);
	
	/* Parse the animation file header: */
	if(!source.isString("MD5Version"))
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s is not a valid MD5 animation file",animFileName);
	int md5Version=source.readInteger();
	if(md5Version!=10)
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Cannot parse MD5 animation files of version %d",md5Version);
	
	/* Read and otherwise ignore the commandline string: */
	if(!source.isString("commandline"))
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s is not a valid MD5 animation file",animFileName);
	source.skipString();
	
	/* Read the number of frames and joints: */
	if(!source.isString("numFrames"))
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s is not a valid MD5 animation file",animFileName);
	numFrames=source.readInteger();
	if(!source.isString("numJoints"))
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s is not a valid MD5 animation file",animFileName);
	numJoints=source.readInteger();
	
	/* Read the frame rate: */
	if(!source.isString("frameRate"))
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s is not a valid MD5 animation file",animFileName);
	frameRate=Scalar(source.readNumber());
	frameTime=Scalar(1)/frameRate;
	
	/* Read the number of animated components: */
	if(!source.isString("numAnimatedComponents"))
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s is not a valid MD5 animation file",animFileName);
	numAnimatedComponents=source.readInteger();
	
	/* Allocate the joint array and parse the joint tree: */
	joints=new Joint[numJoints];
	if(!source.isString("hierarchy")||source.readChar()!='{')
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s does not contain a joint hierarchy",animFileName);
	for(int jointIndex=0;jointIndex<numJoints;++jointIndex)
		{
		if(source.peekc()=='}')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Short joint hierarchy at %s",source.where().c_str());
		Joint& j=joints[jointIndex];
		
		/* Read the joint's name: */
		std::string name=source.readString();
		j.name=new char[name.size()+1];
		memcpy(j.name,name.c_str(),name.size()+1);
		
		/* Read the joint's parent index: */
		j.parentIndex=source.readInteger();
		if(j.parentIndex<-1||j.parentIndex>=numJoints)
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Parent joint index out of range at %s",source.where().c_str());
		
		/* Read the joint's flags: */
		j.flags=source.readInteger();
		
		/* Read the joint's start index: */
		j.startIndex=source.readInteger();
		}
	if(source.readChar()!='}') // Oops, additional joints in list
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Long joint hierarchy at %s",source.where().c_str());
	
	/* Skip the bounding box information for now: */
	if(!source.isString("bounds")||source.readChar()!='{')
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s does not contain a bounding box list",animFileName);
	for(int frameIndex=0;frameIndex<numFrames;++frameIndex)
		{
		/* Read the bounding box's min and max vertices: */
		Point bb[2];
		for(int i=0;i<2;++i)
			{
			if(source.readChar()!='(')
				Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed bounding box corner at %s",source.where().c_str());
			for(int j=0;j<3;++j)
				bb[i][j]=Scalar(source.readNumber());
			if(source.readChar()!=')')
				Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed bounding box corner at %s",source.where().c_str());
			}
		}
	if(source.readChar()!='}') // Oops, additional bounding boxes in list
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Long bounding box list at %s",source.where().c_str());
	
	/* Read the animation's base transformations: */
	if(!source.isString("baseframe")||source.readChar()!='{')
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Input file %s does not contain a base frame",animFileName);
	for(int jointIndex=0;jointIndex<numJoints;++jointIndex)
		{
		if(source.peekc()=='}')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Short base frame at %s",source.where().c_str());
		Joint& j=joints[jointIndex];
		
		/* Read the joint's translation: */
		if(source.readChar()!='(')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed joint position at %s",source.where().c_str());
		for(int i=0;i<3;++i)
			j.baseTranslation[i]=Scalar(source.readNumber());
		if(source.readChar()!=')')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed joint position at %s",source.where().c_str());
		
		/* Read the joint's rotation quaternion: */
		if(source.readChar()!='(')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed joint orientation for joint index %d at %s",jointIndex,source.where().c_str());
		for(int i=0;i<3;++i)
			j.baseRotation[i]=Scalar(source.readNumber());
		if(source.readChar()!=')')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed joint orientation for joint index %d at %s",jointIndex,source.where().c_str());
		}
	if(source.readChar()!='}') // Oops, additional joint in list
		Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Long base frame at %s",source.where().c_str());
	
	/* Read the animation component values for each frame: */
	frameComponents=new Scalar[numFrames*numAnimatedComponents];
	for(int frameIndex=0;frameIndex<numFrames;++frameIndex)
		{
		/* Parse the frame's header: */
		if(!source.isString("frame"))
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed animation frame at %s",source.where().c_str());
		if(source.readInteger()!=frameIndex)
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Mismatching frame index at %s",source.where().c_str());
		if(source.readChar()!='{')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed animation frame at %s",source.where().c_str());
		
		/* Read the frame's animation component values: */
		for(int componentIndex=0;componentIndex<numAnimatedComponents;++componentIndex)
			frameComponents[frameIndex*numAnimatedComponents+componentIndex]=Scalar(source.readNumber());
		
		if(source.readChar()!='}')
			Misc::throwStdErr("Doom3MD5Anim::Doom3MD5Anim: Malformed animation frame at %s",source.where().c_str());
		}
	}

Doom3MD5Anim::~Doom3MD5Anim(void)
	{
	delete[] joints;
	delete[] frameComponents;
	}

void Doom3MD5Anim::animateMesh(Doom3MD5Mesh* mesh,int frameIndex) const
	{
	/* Go through the joint hierarchy and update the mesh's joint transformations on-the-fly: */
	const Scalar* frameBase=&frameComponents[frameIndex*numAnimatedComponents];
	for(int jointIndex=0;jointIndex<numJoints;++jointIndex)
		{
		const Joint& j=joints[jointIndex];
		Doom3MD5Mesh::Joint& mj=mesh->joints[jointIndex];
		
		/* Compose the joint's animated transformation in local coordinates: */
		Scalar translation[3];
		Scalar rotation[4];
		const Scalar* avPtr=frameBase+j.startIndex;
		int flagMask=0x1;
		for(int i=0;i<3;++i,flagMask<<=1)
			{
			if(j.flags&flagMask)
				{
				translation[i]=*avPtr;
				++avPtr;
				}
			else
				translation[i]=j.baseTranslation[i];
			}
		for(int i=0;i<3;++i,flagMask<<=1)
			{
			if(j.flags&flagMask)
				{
				rotation[i]=*avPtr;
				++avPtr;
				}
			else
				rotation[i]=j.baseRotation[i];
			}
		Scalar weightDet=Scalar(1)-Math::sqr(rotation[0])-Math::sqr(rotation[1])-Math::sqr(rotation[2]);
		rotation[3]=weightDet>Scalar(0)?-Math::sqrt(weightDet):Scalar(0);
		Transform jointT(translation,Transform::Rotation(rotation));
		
		/* Compose the transformation with the joint's parent's transformation, and update the mesh: */
		if(j.parentIndex>=0)
			jointT.leftMultiply(mesh->joints[j.parentIndex].transform);
		mj.transform=jointT;
		}
	
	/* Update the mesh's joint tree version: */
	++mesh->jointTreeVersion;
	}

}
