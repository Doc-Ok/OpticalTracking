/***********************************************************************
FileSelectionDialog - A popup window to select a file name.
Copyright (c) 2008-2012 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <iostream>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <Misc/StringMarshaller.h>
#include <Misc/FileNameExtensions.h>
#include <Misc/GetCurrentDirectory.h>
#include <Misc/FileTests.h>
#include <IO/Directory.h>
#include <IO/SeekableFilter.h>
#include <IO/ZipArchive.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Blind.h>
#include <GLMotif/ListBox.h>
#include <GLMotif/ScrolledListBox.h>
#include <GLMotif/RowColumn.h>

#include <GLMotif/FileSelectionDialog.h>

namespace GLMotif {

namespace {

/**************
Helper classes:
**************/

class StringCompare // Class to compare strings according to case-insensitive lexicographical order
	{
	/* Methods: */
	public:
	bool operator()(const std::string& s1,const std::string& s2) const
		{
		/* Find the first different character in the common prefix of both strings: */
		int cmp=0;
		int caseCmp=0;
		std::string::const_iterator s1It,s2It;
		for(s1It=s1.begin(),s2It=s2.begin();s1It!=s1.end()&&s2It!=s2.end()&&cmp==0;++s1It,++s2It)
			{
			cmp=toupper(*s1It)-toupper(*s2It);
			if(caseCmp==0)
				caseCmp=*s1It-*s2It;
			}
		
		/* Check if the common prefix is identical: */
		if(cmp==0)
			{
			/* Check if the first string is shorter: */
			if(s2It!=s2.end())
				cmp=-1;
			else if(s1It!=s1.end())
				cmp=1;
			}
		
		/* If the strings are identical, use the case-dependent result: */
		if(cmp==0)
			cmp=caseCmp;
		
		return cmp<0;
		}
	};

}

/************************************
Methods of class FileSelectionDialog:
************************************/

void FileSelectionDialog::readDirectory(void)
	{
	/* Read all directory entries: */
	std::vector<std::string> directories;
	std::vector<std::string> files;
	currentDirectory->rewind();
	while(currentDirectory->readNextEntry())
		{
		/* Check for hidden entries: */
		const char* entryName=currentDirectory->getEntryName();
		if(entryName[0]!='.')
			{
			/* Determine the type of the directory entry: */
			Misc::PathType pt=currentDirectory->getEntryType();
			if(pt==Misc::PATHTYPE_DIRECTORY)
				{
				/* Store a directory name: */
				std::string dirName(entryName);
				dirName.push_back('/');
				directories.push_back(dirName);
				}
			else if(pt==Misc::PATHTYPE_FILE)
				{
				/* Check for zip archives: */
				if(Misc::hasCaseExtension(entryName,".zip"))
					{
					/* Add the zip archive as a directory: */
					std::string dirName(entryName);
					dirName.push_back('/');
					directories.push_back(dirName);
					}
				else
					{
					/* Check if the file name matches any of the supplied extensions: */
					bool passesFilters=true;
					if(fileNameFilters!=0)
						{
						/* Find the file name's extension: */
						const char* extPtr="";
						for(const char* enPtr=entryName;*enPtr!='\0';++enPtr)
							if(*enPtr=='.')
								extPtr=enPtr;
						
						/* Match against the list of allowed extensions: */
						passesFilters=false;
						const char* filterPtr=fileNameFilters;
						while(*filterPtr!='\0'&&!passesFilters)
							{
							/* Extract the next extension: */
							const char* extStart=filterPtr;
							for(;*filterPtr!='\0'&&*filterPtr!=';';++filterPtr)
								;
							
							/* See if it matches: */
							passesFilters=int(strlen(extPtr))==filterPtr-extStart&&memcmp(extPtr,extStart,filterPtr-extStart)==0;
							
							/* Skip the separator: */
							if(*filterPtr==';')
								++filterPtr;
							}
						}
					
					if(passesFilters)
						{
						/* Store a file name: */
						files.push_back(entryName);
						}
					}
				}
			}
		}
	
	/* Sort the directory and file names separately: */
	StringCompare sc;
	std::sort(directories.begin(),directories.end(),sc);
	std::sort(files.begin(),files.end(),sc);
	
	/* Copy all names into the list box: */
	fileList->getListBox()->clear();
	for(std::vector<std::string>::const_iterator dIt=directories.begin();dIt!=directories.end();++dIt)
		fileList->getListBox()->addItem(dIt->c_str());
	for(std::vector<std::string>::const_iterator fIt=files.begin();fIt!=files.end();++fIt)
		fileList->getListBox()->addItem(fIt->c_str());
	}

void FileSelectionDialog::setSelectedPathButton(int newSelectedPathButton)
	{
	/* Get the style sheet: */
	const StyleSheet& ss=*getManager()->getStyleSheet();
	
	if(selectedPathButton>=0)
		{
		/* Un-"arm" the previously selected path button: */
		Button* oldButton=static_cast<Button*>(pathButtonBox->getChild(selectedPathButton));
		oldButton->setBorderType(Widget::RAISED);
		oldButton->setBackgroundColor(ss.bgColor);
		oldButton->setArmedBackgroundColor(ss.buttonArmedBackgroundColor);
		}
	
	/* Set the index of the selected path button: */
	selectedPathButton=newSelectedPathButton;
	
	/* "Arm" the new selected path button: */
	Button* newButton=static_cast<Button*>(pathButtonBox->getChild(selectedPathButton));
	newButton->setBorderType(Widget::LOWERED);
	newButton->setBackgroundColor(ss.buttonArmedBackgroundColor);
	newButton->setArmedBackgroundColor(ss.bgColor);
	
	/* Read the directory corresponding to the path button: */
	currentDirectory=pathButtonDirectories[selectedPathButton];
	readDirectory();
	}

void FileSelectionDialog::pathButtonSelectedCallback(Button::SelectCallbackData* cbData)
	{
	/* Change the selected path button: */
	setSelectedPathButton(pathButtonBox->getChildIndex(cbData->button));
	}

void FileSelectionDialog::fileNameFieldValueChangedCallback(TextField::ValueChangedCallbackData* cbData)
	{
	/* Check if this was a confirmation event: */
	if(cbData->confirmed)
		{
		if(fileNameField->getString()[0]=='\0')
			{
			/* Call the OK callbacks with the current directory: */
			OKCallbackData cbData(this,currentDirectory,0);
			okCallbacks.call(&cbData);
			}
		else
			{
			/* Check if the file name is a directory: */
			// ...
			
			/* Call the OK callbacks with the value of the file name field: */
			OKCallbackData cbData(this,currentDirectory,fileNameField->getString());
			okCallbacks.call(&cbData);
			}
		}
	}

void FileSelectionDialog::listValueChangedCallback(ListBox::ValueChangedCallbackData* cbData)
	{
	/* Get the index of the newly selected entry: */
	int selectedEntry=fileList->getListBox()->getSelectedItem();
	if(selectedEntry>=0&&canCreateFile)
		{
		/* Check that the item is a file and not a directory: */
		const char* item=fileList->getListBox()->getItem(selectedEntry);
		if(item[0]=='\0'||item[strlen(item)-1]!='/')
			{
			/* Copy the selected file / directory name into the file name text field: */
			fileNameField->setString(item);
			
			/* Select the entire file / directory name: */
			fileNameField->setSelection(0,fileNameField->getLabelLength());
			getManager()->requestFocus(fileNameField);
			}
		}
	}

bool FileSelectionDialog::selectListItem(int selectedItem)
	{
	/* Get the selected list item's name: */
	std::string item=fileList->getListBox()->getItem(selectedItem);
	
	/* Check if it's a file or directory: */
	if(item[item.size()-1]=='/')
		{
		try
			{
			/* Check if the directory is a zip archive: */
			IO::DirectoryPtr newDirectory;
			if(strcasecmp(Misc::getExtension(item.c_str(),item.c_str()+item.size()-1),".zip/")==0)
				{
				/* Open the zip archive: */
				IO::FilePtr zipFile=currentDirectory->openFile(std::string(item.begin(),item.end()-1).c_str());
				IO::SeekableFilePtr seekableZipFile=zipFile;
				if(seekableZipFile==0)
					seekableZipFile=new IO::SeekableFilter(zipFile);
				
				/* Open the zip archive's root directory: */
				IO::ZipArchive* zipArchive=new IO::ZipArchive(seekableZipFile);
				newDirectory=zipArchive->openDirectory("/");
				}
			else
				{
				/* Open the selected directory: */
				newDirectory=currentDirectory->openDirectory(item.c_str());
				}
			
			/* Remove all path buttons after the selected one: */
			for(GLint i=pathButtonBox->getNumColumns()-1;i>selectedPathButton;--i)
				{
				pathButtonBox->removeWidgets(i);
				pathButtonDirectories.pop_back();
				}
			
			/* Add a new path button for the selected directory: */
			item.erase(item.end()-1);
			char pathButtonName[20];
			snprintf(pathButtonName,sizeof(pathButtonName),"PathButton%04d",selectedPathButton);
			Button* pathButton=new Button(pathButtonName,pathButtonBox,item.c_str());
			pathButton->setBorderWidth(pathButton->getBorderWidth()*0.5f);
			pathButton->getSelectCallbacks().add(this,&FileSelectionDialog::pathButtonSelectedCallback);
			
			/* Associate the new directory with the new button: */
			pathButtonDirectories.push_back(newDirectory);
			
			/* Select the new path button: */
			setSelectedPathButton(selectedPathButton+1);
			
			return true;
			}
		catch(std::runtime_error)
			{
			return false;
			}
		}
	else
		{
		/* Call the OK callbacks: */
		OKCallbackData cbData(this,currentDirectory,item.c_str());
		okCallbacks.call(&cbData);
		
		return true;
		}
	}

void FileSelectionDialog::listItemSelectedCallback(ListBox::ItemSelectedCallbackData* cbData)
	{
	selectListItem(cbData->selectedItem);
	}

void FileSelectionDialog::filterListValueChangedCallback(DropdownBox::ValueChangedCallbackData* cbData)
	{
	/* Set the current file name filters to the new selected item: */
	fileNameFilters=cbData->newSelectedItem>0?cbData->dropdownBox->getItem(cbData->newSelectedItem):0;
	
	/* Re-read the current directory: */
	readDirectory();
	}

void FileSelectionDialog::okButtonSelectedCallback(Misc::CallbackData* cbData)
	{
	if(canCreateFile)
		{
		if(fileNameField->getString()[0]=='\0')
			{
			/* Call the OK callbacks with the current directory: */
			OKCallbackData cbData(this,currentDirectory,0);
			okCallbacks.call(&cbData);
			}
		else
			{
			/* Check if the file name is a directory: */
			// ...
			
			/* Call the OK callbacks with the value of the file name field: */
			OKCallbackData cbData(this,currentDirectory,fileNameField->getString());
			okCallbacks.call(&cbData);
			}
		}
	else
		{
		/* Check if there is a selected directory entry: */
		int selectedItem=fileList->getListBox()->getSelectedItem();
		if(selectedItem>=0)
			{
			/* Select the item: */
			selectListItem(selectedItem);
			}
		else if(canSelectDirectory)
			{
			/* Call the OK callbacks with the current directory: */
			OKCallbackData cbData(this,currentDirectory,0);
			okCallbacks.call(&cbData);
			}
		}
	}

void FileSelectionDialog::cancelButtonSelectedCallback(Misc::CallbackData* cbData)
	{
	/* Call the cancel callbacks: */
	CancelCallbackData myCbData(this);
	cancelCallbacks.call(&myCbData);
	}

void FileSelectionDialog::createDialog(const char* sFileNameFilters)
	{
	/* Add a close button to the dialog: */
	setCloseButton(true);
	getCloseCallbacks().add(this,&FileSelectionDialog::cancelButtonSelectedCallback);
	
	/* Create the file selection dialog contents: */
	RowColumn* fileSelectionDialog=new RowColumn("FileSelectionDialog",this,false);
	fileSelectionDialog->setOrientation(RowColumn::VERTICAL);
	fileSelectionDialog->setPacking(RowColumn::PACK_TIGHT);
	fileSelectionDialog->setNumMinorWidgets(1);
	
	if(canCreateFile)
		{
		/* Create the file name text field: */
		fileNameField=new GLMotif::TextField("FileNameField",fileSelectionDialog,40);
		fileNameField->setHAlignment(GLFont::Left);
		fileNameField->setEditable(true);
		}
	
	/* Create the path button box: */
	pathButtonBox=new RowColumn("PathButtonBox",fileSelectionDialog,false);
	pathButtonBox->setOrientation(RowColumn::HORIZONTAL);
	pathButtonBox->setPacking(RowColumn::PACK_TIGHT);
	pathButtonBox->setAlignment(Alignment::LEFT);
	pathButtonBox->setNumMinorWidgets(1);
	pathButtonBox->setMarginWidth(0.0f);
	pathButtonBox->setSpacing(0.0f);
	
	/* Create a list of all parent directories of the current directory: */
	std::vector<IO::DirectoryPtr> parents;
	IO::DirectoryPtr dir=currentDirectory;
	do
		{
		parents.push_back(dir);
		}
	while((dir=dir->getParent())!=0);
	
	/* Create the set of path buttons: */
	unsigned int numParents=parents.size();
	for(unsigned int buttonIndex=0;buttonIndex<numParents;++buttonIndex)
		{
		/* Create a button for the current parent directory: */
		char buttonName[40];
		snprintf(buttonName,sizeof(buttonName),"PathButton%04u",buttonIndex);
		Button* pathButton=new Button(buttonName,pathButtonBox,parents[numParents-1-buttonIndex]->getName().c_str());
		pathButton->setBorderWidth(pathButton->getBorderWidth()*0.5f);
		pathButton->getSelectCallbacks().add(this,&FileSelectionDialog::pathButtonSelectedCallback);
		
		/* Associate the current parent directory with the path button: */
		pathButtonDirectories.push_back(parents[numParents-1-buttonIndex]);
		}
	
	pathButtonBox->manageChild();
	
	/* Create the file list box: */
	fileList=new ScrolledListBox("FileList",fileSelectionDialog,ListBox::ATMOST_ONE,50,15);
	fileList->showHorizontalScrollBar(true);
	fileList->getListBox()->getItemSelectedCallbacks().add(this,&FileSelectionDialog::listItemSelectedCallback);
	fileList->getListBox()->getValueChangedCallbacks().add(this,&FileSelectionDialog::listValueChangedCallback);
	
	/* Create the button box: */
	RowColumn* buttonBox=new RowColumn("ButtonBox",fileSelectionDialog,false);
	buttonBox->setOrientation(RowColumn::HORIZONTAL);
	buttonBox->setPacking(RowColumn::PACK_TIGHT);
	buttonBox->setNumMinorWidgets(1);
	
	{
	/* Create the filter list: */
	std::vector<std::string> filterListItems;
	filterListItems.push_back("All Files");
	if(sFileNameFilters!=0)
		{
		const char* startPtr=sFileNameFilters;
		while(*startPtr!='\0')
			{
			const char* endPtr;
			for(endPtr=startPtr;*endPtr!='\0'&&*endPtr!=',';++endPtr)
				;
			filterListItems.push_back(std::string(startPtr,endPtr));
			if(*endPtr==',')
				++endPtr;
			startPtr=endPtr;
			}
		}
	filterList=new DropdownBox("FilterList",buttonBox,filterListItems);
	filterList->setSelectedItem(filterList->getNumItems()-1);
	filterList->getValueChangedCallbacks().add(this,&FileSelectionDialog::filterListValueChangedCallback);
	if(sFileNameFilters!=0)
		fileNameFilters=filterList->getItem(filterList->getNumItems()-1);
	}
	
	/* Create a separator: */
	Blind* separator=new Blind("Separator",buttonBox);
	separator->setPreferredSize(Vector(buttonBox->getSpacing(),0.0f,0.0f));
	
	/* Create the command button box: */
	RowColumn* commandButtonBox=new RowColumn("CommandButtonBox",buttonBox,false);
	commandButtonBox->setOrientation(RowColumn::HORIZONTAL);
	commandButtonBox->setPacking(RowColumn::PACK_GRID);
	commandButtonBox->setNumMinorWidgets(1);
	
	/* Create the command buttons: */
	Button* okButton=new Button("OK",commandButtonBox,"OK");
	okButton->getSelectCallbacks().add(this,&FileSelectionDialog::okButtonSelectedCallback);
	
	Button* cancelButton=new Button("Cancel",commandButtonBox,"Cancel");
	cancelButton->getSelectCallbacks().add(this,&FileSelectionDialog::cancelButtonSelectedCallback);
	
	commandButtonBox->manageChild();
	
	/* Let the separator eat any size increases: */
	buttonBox->setColumnWeight(1,1.0f);
	
	buttonBox->manageChild();
	
	/* Let the file list widget eat any size increases: */
	fileSelectionDialog->setRowWeight(fileSelectionDialog->getChildRow(fileList),1.0f);
	
	/* Select the last path button (to read the initial directory): */
	setSelectedPathButton(numParents-1);
	
	fileSelectionDialog->manageChild();
	}

FileSelectionDialog::FileSelectionDialog(WidgetManager* widgetManager,const char* titleString,IO::DirectoryPtr sCurrentDirectory,const char* sFileNameFilters)
	:PopupWindow("FileSelectionDialogPopup",widgetManager,titleString),
	 currentDirectory(sCurrentDirectory),
	 fileNameFilters(0),
	 canSelectDirectory(false),
	 canCreateFile(false),fileNameField(0),
	 pathButtonBox(0),selectedPathButton(-1),
	 fileList(0),filterList(0)
	{
	/* Create the dialog: */
	createDialog(sFileNameFilters);
	}

FileSelectionDialog::FileSelectionDialog(WidgetManager* widgetManager,const char* titleString,IO::DirectoryPtr sCurrentDirectory,const char* initialFileName,const char* sFileNameFilters)
	:PopupWindow("FileSelectionDialogPopup",widgetManager,titleString),
	 currentDirectory(sCurrentDirectory),
	 fileNameFilters(0),
	 canSelectDirectory(false),
	 canCreateFile(true),fileNameField(0),
	 pathButtonBox(0),selectedPathButton(-1),
	 fileList(0),filterList(0)
	{
	/* Create the dialog: */
	createDialog(sFileNameFilters);
	
	if(initialFileName!=0)
		{
		/* Copy the initial file name to the file name field: */
		fileNameField->setString(initialFileName);
		
		/* Find the initial file name's first extension: */
		const char* extPtr=0;
		for(extPtr=initialFileName;*extPtr!='\0'&&*extPtr!='.';++extPtr)
			;
		
		/* Request text entry focus and select the initial file name's base part: */
		getManager()->requestFocus(fileNameField);
		fileNameField->setSelection(0,extPtr-initialFileName);
		}
	}

FileSelectionDialog::~FileSelectionDialog(void)
	{
	}

void FileSelectionDialog::addFileNameFilters(const char* newFileNameFilters)
	{
	/* Add the filters to the filter list: */
	const char* startPtr=newFileNameFilters;
	while(*startPtr!='\0')
		{
		const char* endPtr;
		for(endPtr=startPtr;*endPtr!='\0'&&*endPtr!=',';++endPtr)
			;
		filterList->addItem(std::string(startPtr,endPtr).c_str());
		if(*endPtr==',')
			++endPtr;
		startPtr=endPtr;
		}
	}

void FileSelectionDialog::setCanSelectDirectory(bool newCanSelectDirectory)
	{
	canSelectDirectory=newCanSelectDirectory;
	}

void FileSelectionDialog::deleteOnCancel(void)
	{
	cancelCallbacks.add(deleteFunction);
	}

}
