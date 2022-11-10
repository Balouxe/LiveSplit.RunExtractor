#include "wxApplication.h"
#include "tools/FileIO.h"
#include "tinyxml2/tinyxml2.h"

#include <wx/splitter.h>

IMPLEMENT_APP(Application)

bool Application::OnInit()
{ 
	m_frame = new MainFrame(_("LiveSplit Run Extractor"));
	m_frame->SetSize(wxSize(1200, 800));
	m_frame->Show(true);
	SetTopWindow(m_frame);
	return true;
}

MainFrame::MainFrame(const wxString& title)
	: wxFrame((wxFrame*)NULL, ID_FRAME, title)
{
	// Create Menus
	wxMenu* fileMenu = new wxMenu;
	fileMenu->Append(ID_MENU_OPENFILE, "&Open File\tCtrl+O", "Open File");
	fileMenu->Append(ID_MENU_SAVEFILE, "&Save File\tCtrl+S", "Save File");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "&Quit\tAlt+F4", "Quit");

	wxMenu* optionsMenu = new wxMenu;
	wxMenuItem* showUnfinishedItem = new wxMenuItem(optionsMenu, ID_MENU_SHOW_UNFINISHED, "&Show unfinished runs", "Show unfinished runs", wxITEM_CHECK);
	optionsMenu->Append(showUnfinishedItem);
	showUnfinishedItem->Check(true);

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(optionsMenu, "&Options");
	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &MainFrame::OnQuit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnOpenFile, this, ID_MENU_OPENFILE);
	Bind(wxEVT_MENU, &MainFrame::OnSaveFile, this, ID_MENU_SAVEFILE);
	Bind(wxEVT_MENU, &MainFrame::OnShowUnfinished, this, ID_MENU_SHOW_UNFINISHED);

	// Open the file dialog to select a splits file
	wxFileDialog FileDialog(this, _("Open Splits"), "", "", "LSS Files (*.lss)|*.lss", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (FileDialog.ShowModal() == wxID_CANCEL) {
		exit;
	}
	wxString path = FileDialog.GetPath();
	Shared::fileio.OpenFile(path);

	wxSplitterWindow* m_splitter = new wxSplitterWindow(this, ID_SPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);
	m_splitter->SetMinimumPaneSize(100);

	m_runPanel = new wxPanel(m_splitter, ID_RUNPANEL);
	m_runSizer = new wxBoxSizer(wxVERTICAL);

	m_listCtrl = new wxListCtrl(m_splitter, ID_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxLC_REPORT);
	InitList();
	m_listCtrl->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::ItemSelected, this, ID_LISTCTRL);

	Shared::fileio.GetInfo();

	m_category = new wxStaticText(m_runPanel, ID_CATEGORY, Shared::fileio.info.GetTitle(), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	m_notebook = new wxNotebook(m_runPanel, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize);

	m_realGrid = new wxGrid(m_notebook, ID_GRID, wxDefaultPosition, wxDefaultSize);
	m_realGrid->CreateGrid(Shared::fileio.info.nbSplits, 3, wxGrid::wxGridSelectCells);
	m_notebook->AddPage(m_realGrid, "Real Time");
	m_gameGrid = new wxGrid(m_notebook, ID_GRID, wxDefaultPosition, wxDefaultSize);
	m_gameGrid->CreateGrid(Shared::fileio.info.nbSplits, 3, wxGrid::wxGridSelectCells);
	m_notebook->AddPage(m_gameGrid, "Game Time");
	InitRealGrid();
	InitGameGrid();

	m_runSizer->Add(m_category, wxSizerFlags().Expand());
	m_runSizer->Add(m_notebook, wxSizerFlags().Expand());

	m_splitter->SplitVertically(m_listCtrl, m_runPanel);
	m_runPanel->SetSizer(m_runSizer);

	wxColour bgcolor;
	bgcolor.Set(245, 245, 245);
	this->SetBackgroundColour(bgcolor);

}

void MainFrame::ItemSelected(wxListEvent& event) {
	wxListItem info;
	info.m_itemId = event.m_itemIndex;
	info.m_col = 0;
	wxString id = m_listCtrl->GetItemText(info);
	RefreshRealGrid(id);
	RefreshGameGrid(id);
}

void MainFrame::OnQuit(wxCommandEvent& event) {
	this->Close();
}

void MainFrame::OnOpenFile(wxCommandEvent& event) {
	wxFileDialog FileDialog(this, _("Open Splits"), "", "", "LSS Files (*.lss)|*.lss", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (FileDialog.ShowModal() == wxID_CANCEL) {
	}
	wxString path = FileDialog.GetPath();
	Shared::fileio.OpenFile(path);
	m_realGrid->DeleteRows(0, Shared::fileio.info.nbSplits);
	m_gameGrid->DeleteRows(0, Shared::fileio.info.nbSplits);
	Shared::fileio.GetInfo();
	m_realGrid->InsertRows(0, Shared::fileio.info.nbSplits);
	m_gameGrid->InsertRows(0, Shared::fileio.info.nbSplits);
	InitRealGrid();
	InitGameGrid();
	m_realGrid->SetSize(wxSize(m_runPanel->GetSize()));
	m_gameGrid->SetSize(wxSize(m_runPanel->GetSize()));
	m_category->SetLabelText(Shared::fileio.info.GetTitle());

	InitList();
}

void MainFrame::OnSaveFile(wxCommandEvent& event) {
	// cool stuff happening here (not used yet, maybe ever)
	// only added this in case i expand this to be a more general application to analyse splits
}

void MainFrame::OnShowUnfinished(wxCommandEvent& event) {
	m_showUnfinished = !m_showUnfinished;
	InitList();
}

void MainFrame::InitList() {
	m_listCtrl->DeleteAllColumns();
	m_listCtrl->DeleteAllItems();

	Shared::fileio.GetAllRuns();

	wxListItem itemCol;
	itemCol.SetText("Attempt n°");
	m_listCtrl->InsertColumn(0, itemCol);
	m_listCtrl->SetColumnWidth(0, 120);

	itemCol.SetText("Finished");
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	m_listCtrl->InsertColumn(1, itemCol);
	m_listCtrl->SetColumnWidth(1, 120);

	itemCol.SetText("Final Real Time");
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	m_listCtrl->InsertColumn(2, itemCol);
	m_listCtrl->SetColumnWidth(2, 120);

	itemCol.SetText("Final Game Time");
	itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
	m_listCtrl->InsertColumn(3, itemCol);
	m_listCtrl->SetColumnWidth(3, 120);

	m_listCtrl->Hide(); // Hiding the list control speeds up adding the runs.

	int iter = 0;
	for (RunInfo& run : Shared::fileio.runsVec) {
		if (run.finished) {
			wxListItem item;
			item.SetId(iter);
			m_listCtrl->InsertItem(item);
			m_listCtrl->SetItem(iter, 0, run.charID);
			m_listCtrl->SetItem(iter, 1, "Yes");
			m_listCtrl->SetItem(iter, 2, run.realTime);
			m_listCtrl->SetItem(iter, 3, run.gameTime);
			iter++;
		}
		else if (m_showUnfinished) {
			wxListItem item;
			item.SetId(iter);
			m_listCtrl->InsertItem(item);
			m_listCtrl->SetItem(iter, 0, run.charID);
			m_listCtrl->SetItem(iter, 1, "No");
			m_listCtrl->SetItem(iter, 2, run.realTime);
			m_listCtrl->SetItem(iter, 3, run.gameTime);
			iter++;
		}
	}
	m_listCtrl->Show();
}

void MainFrame::InitRealGrid() {
	m_realGrid->ClearGrid();
	m_realGrid->EnableEditing(false);
	m_realGrid->SetRowLabelSize(160);
	m_realGrid->SetColSize(0, 120);
	m_realGrid->SetColSize(1, 120);
	m_realGrid->SetColSize(2, 120);

	m_realGrid->SetColLabelValue(0, wxString("Split Time"));
	m_realGrid->SetColLabelValue(1, wxString("Segment Time"));
	m_realGrid->SetColLabelValue(2, wxString("Best Time"));

	Shared::fileio.GetSplitsNames();
	int iter = 0;
	for (std::string& split : Shared::fileio.namesVec) {
		m_realGrid->SetRowLabelValue(iter, split);
		iter++;
	}
	iter = 0;
	Shared::fileio.GetBestSplitsTimes();
	for (std::string& best : Shared::fileio.bestSplitsRealVec) {
		m_realGrid->SetCellValue(wxGridCellCoords(iter, 2), best);
		iter++;
	}
	iter = 0;
	Shared::fileio.GetSplitsTimes("1");
	std::chrono::duration<double, std::milli> dur = std::chrono::duration<double, std::milli>::zero();
	std::chrono::duration<double, std::milli> addDur = std::chrono::duration<double, std::milli>::zero();
	for (std::string& time : Shared::fileio.splitsVec) {
		m_realGrid->SetCellValue(wxGridCellCoords(iter, 0), time);
		dur = FileIO::sToDuration(time);
		addDur += dur;
		m_realGrid->SetCellValue(wxGridCellCoords(iter, 1), FileIO::durationToS(addDur));
		iter++;
	}
}

void MainFrame::RefreshRealGrid(const char* id) {
	int iter = 0;
	for (int i = 0; i < Shared::fileio.info.nbSplits; i++) {
		m_realGrid->SetCellValue(wxGridCellCoords(i, 0), "");
		m_realGrid->SetCellValue(wxGridCellCoords(i, 1), "");
	}
	Shared::fileio.GetSplitsTimes(id);
	std::chrono::duration<double, std::milli> dur = std::chrono::duration<double, std::milli>::zero();
	std::chrono::duration<double, std::milli> addDur = std::chrono::duration<double, std::milli>::zero();
	for (std::string& time : Shared::fileio.splitsVec) {
		m_realGrid->SetCellValue(wxGridCellCoords(iter, 0), time);
		dur = FileIO::sToDuration(time);
		if (dur == dur.zero()) {
			m_realGrid->SetCellValue(wxGridCellCoords(iter, 1), "");
		}
		else {
			addDur += dur;
			m_realGrid->SetCellValue(wxGridCellCoords(iter, 1), FileIO::durationToS(addDur));
		}
		iter++;
	}
}

void MainFrame::InitGameGrid() {
	m_gameGrid->ClearGrid();
	m_gameGrid->EnableEditing(false);
	m_gameGrid->SetRowLabelSize(160);
	m_gameGrid->SetColSize(0, 120);
	m_gameGrid->SetColSize(1, 120);
	m_gameGrid->SetColSize(2, 120);

	m_gameGrid->SetColLabelValue(0, wxString("Split Time"));
	m_gameGrid->SetColLabelValue(1, wxString("Segment Time"));
	m_gameGrid->SetColLabelValue(2, wxString("Best Time"));

	Shared::fileio.GetSplitsNames();
	int iter = 0;
	for (std::string& split : Shared::fileio.namesVec) {
		m_gameGrid->SetRowLabelValue(iter, split);
		iter++;
	}
	iter = 0;
	Shared::fileio.GetBestSplitsTimes();
	for (std::string& best : Shared::fileio.bestSplitsGameVec) {
		m_gameGrid->SetCellValue(wxGridCellCoords(iter, 2), best);
		iter++;
	}
	iter = 0;
	Shared::fileio.GetSplitsTimes("1", true);
	std::chrono::duration<double, std::milli> dur = std::chrono::duration<double, std::milli>::zero();
	std::chrono::duration<double, std::milli> addDur = std::chrono::duration<double, std::milli>::zero();
	for (std::string& time : Shared::fileio.splitsVec) {
		m_gameGrid->SetCellValue(wxGridCellCoords(iter, 0), time);
		dur = FileIO::sToDuration(time);
		addDur += dur;
		m_gameGrid->SetCellValue(wxGridCellCoords(iter, 1), FileIO::durationToS(addDur));
		iter++;
	}
}

void MainFrame::RefreshGameGrid(const char* id) {
	int iter = 0;
	for (int i = 0; i < Shared::fileio.info.nbSplits; i++) {
		m_gameGrid->SetCellValue(wxGridCellCoords(i, 0), "");
		m_gameGrid->SetCellValue(wxGridCellCoords(i, 1), "");
	}
	Shared::fileio.GetSplitsTimes(id, true);
	std::chrono::duration<double, std::milli> dur = std::chrono::duration<double, std::milli>::zero();
	std::chrono::duration<double, std::milli> addDur = std::chrono::duration<double, std::milli>::zero();
	for (std::string& time : Shared::fileio.splitsVec) {
		m_gameGrid->SetCellValue(wxGridCellCoords(iter, 0), time);
		dur = FileIO::sToDuration(time);
		if (dur == dur.zero()) {
			m_gameGrid->SetCellValue(wxGridCellCoords(iter, 1), "");
		}
		else {
			addDur += dur;
			m_gameGrid->SetCellValue(wxGridCellCoords(iter, 1), FileIO::durationToS(addDur));
		}
		iter++;
	}
}