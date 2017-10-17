#include "dlgcheckdatas.h"

#include <wx/statline.h>

#include "datasmanager.h"

DlgCheckDatas::DlgCheckDatas(wxWindow *parent) : wxDialog(parent, -1, _("Check loaded datas")),
	m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Creating a \"DlgCheckDatas\" object\n"));
#endif // __WXDEBUG__

	m_bIsRunning=false;

	CreateControls();

	ConnectControls();

	CenterOnParent();
}

DlgCheckDatas::~DlgCheckDatas()
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Destroying a \"DlgCheckDatas\" object\n"));
#endif // __WXDEBUG__
}

void DlgCheckDatas::CreateControls()
{
	wxBoxSizer *szrMain, *lnszr;
	wxStaticText *label;

	szrMain=new wxBoxSizer(wxVERTICAL);

		lnszr=new wxBoxSizer(wxHORIZONTAL);
			label=new wxStaticText(this, wxID_STATIC, _("Parameters:"));
			lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
			lnszr->Add(new wxStaticLine(this, wxID_STATIC), 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
		szrMain->Add(lnszr, 0, wxALL|wxEXPAND, 5);

		wxSizer *btnSzr=CreateSeparatedButtonSizer(wxAPPLY|wxCLOSE);
			m_btnStartStop=(wxButton*)FindWindowById(wxID_APPLY, this);
			m_btnStartStop->SetLabel(_("Start"));
		szrMain->Add(btnSzr, 0, wxALL|wxEXPAND, 5);

	SetSizer(szrMain);
}

void DlgCheckDatas::ConnectControls()
{
	Bind(wxEVT_BUTTON, &DlgCheckDatas::OnBtnStartStopClicked, this, wxID_APPLY);
	Bind(wxEVT_UPDATE_UI, &DlgCheckDatas::OnUpdateUI_BtnClose, this, wxID_CLOSE);
}

void DlgCheckDatas::OnBtnStartStopClicked(wxCommandEvent& event)
{
	if (m_bIsRunning)
	{
		m_bIsRunning=false;
		m_btnStartStop->SetLabel(_("Start"));
	}
	else
	{
		m_bIsRunning=true;
		m_btnStartStop->SetLabel(_("Stop"));
	}
}

void DlgCheckDatas::OnUpdateUI_BtnClose(wxUpdateUIEvent& event)
{
	event.Enable(m_bIsRunning==false);
}
