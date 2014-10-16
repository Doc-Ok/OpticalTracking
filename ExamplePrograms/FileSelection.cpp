/***********************************************************************
FileSelection - Example application for Vrui's file selection dialog and
cluster-transparent file handling via the IO abstraction library.
Copyright (c) 2014 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <string>
#include <stdexcept>
#include <iostream>
#include <Misc/SizedTypes.h>
#include <Misc/FunctionCalls.h>
#include <IO/File.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Separator.h>
#include <GLMotif/Button.h>
#include <GLMotif/FileSelectionDialog.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include <Vrui/FileSelectionHelper.h>
#include <Vrui/OpenFile.h>

class FileSelection:public Vrui::Application
	{
	/* Elements: */
	private:
	Vrui::FileSelectionHelper fooHelper; // Helper object to load/save "foo" files (extension .foo)
	Vrui::FileSelectionHelper barHelper; // Helper object to load/save "bar" files (extension .bar/.baz)
	GLMotif::PopupMenu* mainMenuPopup; // The program's main menu
	
	/* Private methods: */
	void loadFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData,int fileType); // Callback when user selected a file to load
	void saveFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData,int fileType); // Callback when user selected a file to save
	
	/* Constructors and destructors */
	public:
	FileSelection(int& argc,char**& argv);
	virtual ~FileSelection(void);
	};

/******************************
Methods of class FileSelection:
******************************/

void FileSelection::loadFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData,int fileType)
	{
	/* Print the full name of the selected file: */
	std::cout<<"Loading "<<(fileType==0?"foo":"bar")<<" file "<<cbData->getSelectedPath()<<std::endl;
	
	try
		{
		/* Open the file through a (cluster- and zip file-transparent) IO::Directory abstraction: */
		IO::FilePtr file=cbData->selectedDirectory->openFile(cbData->selectedFileName);
		
		/* Read some data: */
		file->setEndianness(Misc::LittleEndian);
		Misc::UInt32 magic=file->read<Misc::UInt32>();
		switch(fileType)
			{
			case 0:
				if(magic!=0x12345678U)
					{
					std::string message="File ";
					message+=cbData->getSelectedPath();
					message+=" is not a \"foo\" file";
					Vrui::showErrorMessage("Load File...",message.c_str());
					}
				break;
					
			case 1:
				if(magic!=0x87654321U)
					{
					std::string message="File ";
					message+=cbData->getSelectedPath();
					message+=" is not a \"bar\" file";
					Vrui::showErrorMessage("Load File...",message.c_str());
					}
				break;
			}
		}
	catch(std::runtime_error err)
		{
		/* Show an error message: */
		Vrui::showErrorMessage("Load File...",err.what());
		}
	}

void FileSelection::saveFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData,int fileType)
	{
	/* Print the full name of the selected file: */
	std::cout<<"Saving "<<(fileType==0?"foo":"bar")<<" file "<<cbData->getSelectedPath()<<std::endl;
	
	try
		{
		/* Open the file through a (cluster- and zip file-transparent) IO::Directory abstraction: */
		IO::FilePtr file=cbData->selectedDirectory->openFile(cbData->selectedFileName,IO::File::WriteOnly);
		
		/* Write something into the file: */
		file->setEndianness(Misc::LittleEndian);
		switch(fileType)
			{
			case 0: // It's a "foo" file
				file->write<Misc::UInt32>(0x12345678U);
				break;
			
			case 1: // It's a "bar" file
				file->write<Misc::UInt32>(0x87654321U);
				break;
			}
		}
	catch(std::runtime_error err)
		{
		/* Show an error message: */
		Vrui::showErrorMessage("Save File...",err.what());
		}
	}

FileSelection::FileSelection(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 fooHelper("FooFile.foo",".foo",Vrui::openDirectory(".")),
	 barHelper("BarFile.bar",".bar;.baz",Vrui::openDirectory(".")),
	 mainMenuPopup(0)
	{
	/* Create a top-level shell for the main menu: */
	mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("File Selection");
	
	/* Create the actual menu inside the top-level shell: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create buttons to load/save "foo" files: */
	GLMotif::Button* loadFooButton=new GLMotif::Button("LoadFooButton",mainMenu,"Load Foo...");
	GLMotif::Button* saveFooButton=new GLMotif::Button("SaveFooButton",mainMenu,"Save Foo...");
	
	/* Hook the "foo" file selection helper into the pair of buttons: */
	fooHelper.addLoadCallback(loadFooButton,Misc::createFunctionCall(this,&FileSelection::loadFileCallback,0));
	fooHelper.addSaveCallback(saveFooButton,Misc::createFunctionCall(this,&FileSelection::saveFileCallback,0));
	
	new GLMotif::Separator("Sep1",mainMenu,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	/* Create buttons to load/save "bar" files: */
	GLMotif::Button* loadBarButton=new GLMotif::Button("LoadBarButton",mainMenu,"Load Bar...");
	GLMotif::Button* saveBarButton=new GLMotif::Button("SaveBarButton",mainMenu,"Save Bar...");
	
	/* Hook the "bar" file selection helper into the pair of buttons: */
	barHelper.addLoadCallback(loadBarButton,Misc::createFunctionCall(this,&FileSelection::loadFileCallback,1));
	barHelper.addSaveCallback(saveBarButton,Misc::createFunctionCall(this,&FileSelection::saveFileCallback,1));
	
	mainMenu->manageChild();
	
	/* Register the main menu: */
	Vrui::setMainMenu(mainMenuPopup);
	}

FileSelection::~FileSelection(void)
	{
	/* Delete the main menu: */
	delete mainMenuPopup;
	}

VRUI_APPLICATION_RUN(FileSelection)
