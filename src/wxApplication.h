#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif 

#include "wx/listctrl.h"
#include "wx/grid.h"
#include "wx/notebook.h"

enum userActions {
	ID_MENU_OPENFILE,
	ID_MENU_SAVEFILE,
	ID_MENU_SHOW_UNFINISHED,
	ID_LIST_ITEM_SELECTED
};

enum wxIDs {
	ID_FRAME,
	ID_SPLITTER,
	ID_LISTCTRL,
	ID_RUNPANEL,
	ID_CATEGORY,
	ID_GRID,
	ID_NOTEBOOK
};

class Application : public wxApp
{
public:
	virtual bool OnInit();
protected:
	class MainFrame* m_frame;
};

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);
	
	void OnQuit(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnSaveFile(wxCommandEvent& event);
	void OnShowUnfinished(wxCommandEvent& event);

	void ItemSelected(wxListEvent& event);

	void InitList();

	void InitRealGrid();
	void InitGameGrid();

	void RefreshRealGrid(const char* id);
	void RefreshGameGrid(const char* id);
private:
	bool m_showUnfinished = true;
	wxPanel* m_runPanel;
	wxBoxSizer* m_runSizer;
	wxNotebook* m_notebook;
	wxListCtrl* m_listCtrl;

	wxStaticText* m_category;
	wxGrid* m_realGrid;
	wxGrid* m_gameGrid;
};

DECLARE_APP(Application)