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

	m_grid = new wxGrid(m_runPanel, ID_GRID, wxDefaultPosition, wxDefaultSize);
	m_grid->CreateGrid(Shared::fileio.info.nbSplits, 3, wxGrid::wxGridSelectCells);
	InitGrid();

	m_runSizer->Add(m_category, wxSizerFlags().Expand());
	m_runSizer->Add(m_grid, wxSizerFlags().Expand());

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
	RefreshGrid(id);
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
	m_grid->DeleteRows(0, Shared::fileio.info.nbSplits);
	Shared::fileio.GetInfo();
	m_grid->InsertRows(0, Shared::fileio.info.nbSplits);
	InitGrid();
	m_grid->SetSize(wxSize(m_runPanel->GetSize()));

	InitList();
}

void MainFrame::OnSaveFile(wxCommandEvent& event) {
	// cool stuff happening here
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
	m_listCtrl->SetColumnWidth(0, 150);

	itemCol.SetText("Finished");
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	m_listCtrl->InsertColumn(1, itemCol);
	m_listCtrl->SetColumnWidth(1, 150);

	itemCol.SetText("Final Time");
	itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
	m_listCtrl->InsertColumn(2, itemCol);
	m_listCtrl->SetColumnWidth(2, 150);

	m_listCtrl->Hide(); // Hiding the list control speeds up adding the runs.

	int iter = 0;
	for (RunInfo& run : Shared::fileio.runsVec) {
		if (run.finished) {
			wxListItem item;
			item.SetId(iter);
			m_listCtrl->InsertItem(item);
			m_listCtrl->SetItem(iter, 0, run.charID);
			m_listCtrl->SetItem(iter, 1, "Yes");
			m_listCtrl->SetItem(iter, 2, run.finishedTime);
			iter++;
		}
		else if (m_showUnfinished) {
			wxListItem item;
			item.SetId(iter);
			m_listCtrl->InsertItem(item);
			m_listCtrl->SetItem(iter, 0, run.charID);
			m_listCtrl->SetItem(iter, 1, "No");
			m_listCtrl->SetItem(iter, 2, run.finishedTime);
			iter++;
		}
	}
	m_listCtrl->Show();
}

void MainFrame::InitGrid() {
	m_grid->ClearGrid();
	m_grid->EnableEditing(false);
	m_grid->EnableDragColSize(false);
	m_grid->SetRowLabelSize(120);
	m_grid->SetColSize(0, 120);
	m_grid->SetColSize(1, 120);
	m_grid->SetColSize(2, 120);

	m_grid->SetColLabelValue(0, wxString("Split Time"));
	m_grid->SetColLabelValue(1, wxString("Segment Time"));
	m_grid->SetColLabelValue(2, wxString("Best Time"));

	Shared::fileio.GetSplitsNames();
	int iter = 0;
	for (std::string& split : Shared::fileio.namesVec) {
		m_grid->SetRowLabelValue(iter, split);
		iter++;
	}
	iter = 0;
	Shared::fileio.GetBestSplitsTimes();
	for (std::string& best : Shared::fileio.bestSplitsVec) {
		m_grid->SetCellValue(wxGridCellCoords(iter, 2), best);
		iter++;
	}
	iter = 0;
	Shared::fileio.GetSplitsTimes("1");
	std::chrono::duration<double, std::milli> dur = std::chrono::duration<double, std::milli>::zero();
	std::chrono::duration<double, std::milli> addDur = std::chrono::duration<double, std::milli>::zero();
	for (std::string& time : Shared::fileio.splitsVec) {
		m_grid->SetCellValue(wxGridCellCoords(iter, 0), time);
		dur = FileIO::sToDuration(time);
		addDur += dur;
		m_grid->SetCellValue(wxGridCellCoords(iter, 1), FileIO::durationToS(addDur));
		iter++;
	}
}

void MainFrame::RefreshGrid(const char* id) {
	int iter = 0;
	for (int i = 0; i < Shared::fileio.info.nbSplits; i++) {
		m_grid->SetCellValue(wxGridCellCoords(i, 0), "");
		m_grid->SetCellValue(wxGridCellCoords(i, 1), "");
	}
	Shared::fileio.GetSplitsTimes(id);
	std::chrono::duration<double, std::milli> dur = std::chrono::duration<double, std::milli>::zero();
	std::chrono::duration<double, std::milli> addDur = std::chrono::duration<double, std::milli>::zero();
	for (std::string& time : Shared::fileio.splitsVec) {
		m_grid->SetCellValue(wxGridCellCoords(iter, 0), time);
		dur = FileIO::sToDuration(time);
		if (dur.count() > 6480000000 || dur.count() < -6480000000) { // 30 hours here because people who have 30 hours splits shalln't liveth
			// i'm actually doing this because if a split has no time it will display as a huge number for some reason
			dur = dur.zero();
			m_grid->SetCellValue(wxGridCellCoords(iter, 1), "");
		}
		else {
			addDur += dur;
			m_grid->SetCellValue(wxGridCellCoords(iter, 1), FileIO::durationToS(addDur));
		}
		iter++;
	}
}