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

#ifndef GLMOTIF_FILESELECTIONDIALOG_INCLUDED
#define GLMOTIF_FILESELECTIONDIALOG_INCLUDED

#include <string>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <IO/Directory.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Button.h>
#include <GLMotif/ListBox.h>
#include <GLMotif/DropdownBox.h>
#include <GLMotif/PopupWindow.h>

/* Forward declarations: */
namespace GLMotif {
class RowColumn;
class ScrolledListBox;
}

namespace GLMotif {

class FileSelectionDialog:public PopupWindow
	{
	/* Embedded classes: */
	public:
	class CallbackData:public PopupWindow::CallbackData // Base class for file selection dialog callbacks
		{
		/* Elements: */
		public:
		FileSelectionDialog* fileSelectionDialog; // Pointer to the file selection dialog that caused the event; redundant, but so what
		
		/* Constructors and destructors: */
		CallbackData(FileSelectionDialog* sFileSelectionDialog)
			:PopupWindow::CallbackData(sFileSelectionDialog),
			 fileSelectionDialog(sFileSelectionDialog)
			{
			}
		};
	
	class OKCallbackData:public CallbackData // Callback data when the OK button was clicked, or a file name was double-clicked
		{
		/* Elements: */
		public:
		IO::DirectoryPtr selectedDirectory; // The directory containing the selected file; or the selected directory
		const char* selectedFileName; // Name of selected file relative to selected directory, or 0 if a directory was selected
		
		/* Constructors and destructors: */
		OKCallbackData(FileSelectionDialog* sFileSelectionDialog,IO::DirectoryPtr sSelectedDirectory,const char* sSelectedFileName)
			:CallbackData(sFileSelectionDialog),
			 selectedDirectory(sSelectedDirectory),
			 selectedFileName(sSelectedFileName)
			{
			}
		
		/* Methods: */
		std::string getSelectedPath(void) const // Returns the full path name of the selected file
			{
			return selectedDirectory->getPath(selectedFileName);
			}
		};
	
	class CancelCallbackData:public CallbackData // Callback data when the cancel button was clicked
		{
		/* Constructors and destructors: */
		public:
		CancelCallbackData(FileSelectionDialog* sFileSelectionDialog)
			:CallbackData(sFileSelectionDialog)
			{
			}
		};
	
	/* Elements: */
	private:
	IO::DirectoryPtr currentDirectory; // The currently-displayed directory
	const char* fileNameFilters; // Current filter expression for file names; semicolon-separated list of allowed extensions
	bool canSelectDirectory; // Flag whether the caller allows to select a directory by opening the directory and then pressing OK
	bool canCreateFile; // Flag whether the caller allows entering a non-existing file name into the file name text field
	TextField* fileNameField; // Text field to directly edit the selected file name
	RowColumn* pathButtonBox; // Box containing the path component buttons
	std::vector<IO::DirectoryPtr> pathButtonDirectories; // Directory objects associated with the path component buttons
	int selectedPathButton; // Index of the currently selected path button; determines the displayed directory
	ScrolledListBox* fileList; // Scrolled list box containing all directories and matching files in the current directory
	DropdownBox* filterList; // Drop down box containing the selectable file name filters
	Misc::CallbackList okCallbacks; // Callbacks to be called when the OK button is selected, or a file name is double-clicked
	Misc::CallbackList cancelCallbacks; // Callbacks to be called when the cancel button is selected
	
	/* Private methods: */
	void readDirectory(void); // Reads all directories and files from the selected directory into the list box
	void setSelectedPathButton(int newSelectedPathButton); // Changes the selected path button
	void pathButtonSelectedCallback(Button::SelectCallbackData* cbData); // Callback called when one of the path buttons is selected
	void fileNameFieldValueChangedCallback(TextField::ValueChangedCallbackData* cbData); // Callback called when the file name text field changes value
	void listValueChangedCallback(ListBox::ValueChangedCallbackData* cbData); // Callback when the selected item in the list changes
	bool selectListItem(int selectedItem); // Selects a list item on double click or OK button press; returns true if selection was successful
	void listItemSelectedCallback(ListBox::ItemSelectedCallbackData* cbData); // Callback when a list item gets double-clicked
	void filterListValueChangedCallback(DropdownBox::ValueChangedCallbackData* cbData); // Callback when the selected file name filter changes
	void okButtonSelectedCallback(Misc::CallbackData* cbData); // Callback called when the OK button is pressed
	void cancelButtonSelectedCallback(Misc::CallbackData* cbData); // Callback called when the Cancel button is pressed or the dialog window is closed
	void createDialog(const char* sFileNameFilters); // Creates the dialog widget
	
	/* Constructors and destructors: */
	public:
	FileSelectionDialog(WidgetManager* widgetManager,const char* titleString,IO::DirectoryPtr sCurrentDirectory,const char* sFileNameFilters); // Creates a "Load"-style file selection dialog with the given title, initial directory, and file name filter set
	FileSelectionDialog(WidgetManager* widgetManager,const char* titleString,IO::DirectoryPtr sCurrentDirectory,const char* initialFileName,const char* sFileNameFilters); // Creates a "Save"-style file selection dialog with the given title, initial directory, initial file name, and file name filter set
	virtual ~FileSelectionDialog(void);
	
	/* Methods: */
	void addFileNameFilters(const char* newFileNameFilters); // Adds another extension list to the list of selectable filters
	void setCanSelectDirectory(bool newCanSelectDirectory); // Selects whether the OK callback can contain a directory name instead of a file name
	Misc::CallbackList& getOKCallbacks(void) // Returns the list of OK callbacks
		{
		return okCallbacks;
		}
	Misc::CallbackList& getCancelCallbacks(void) // Returns the list of cancel callbacks
		{
		return cancelCallbacks;
		}
	void deleteOnCancel(void); // Convenience method to simply delete the file selection dialog when the cancel button is selected
	IO::DirectoryPtr getCurrentDirectory(void) // Returns the directory whose contents are currently displayed in the file selection dialog
		{
		return currentDirectory;
		}
	};

}

#endif
