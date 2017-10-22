#include "dlgcheckdatas.h"

#include <wx/statline.h>

#include "datasmanager.h"

wxDEFINE_EVENT(wxEVT_DBCHECK_STARTED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DBCHECK_WORKING, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DBCHECK_STOPPED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DBCHECK_ENDED, wxCommandEvent);

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
	wxBoxSizer *pnlszr, *lnszr;
	wxStaticText *label;

	m_szrMain=new wxBoxSizer(wxVERTICAL);

		// First part of the dialog : parameters
		m_pnlParams=new wxPanel(this, wxID_STATIC);
			pnlszr=new wxBoxSizer(wxVERTICAL);
				lnszr=new wxBoxSizer(wxHORIZONTAL);
					label=new wxStaticText(m_pnlParams, wxID_STATIC, _("Parameters:"));
				lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
				lnszr->Add(new wxStaticLine(m_pnlParams, wxID_STATIC), 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
			pnlszr->Add(lnszr, 0, wxALL|wxEXPAND, 5);

			m_pnlParams->SetSizer(pnlszr);
		m_szrMain->Add(m_pnlParams, 1, wxALL|wxEXPAND, 0);

		// Second part of the dialog : Check resutls
		m_pnlResults=new wxPanel(this, wxID_STATIC);
			pnlszr=new wxBoxSizer(wxVERTICAL);
				lnszr=new wxBoxSizer(wxHORIZONTAL);
					label=new wxStaticText(m_pnlResults, wxID_STATIC, _("Results:"));
				lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
				lnszr->Add(new wxStaticLine(m_pnlResults, wxID_STATIC), 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
			pnlszr->Add(lnszr, 0, wxALL|wxEXPAND, 5);

			m_txtLog=new wxTextCtrl(m_pnlResults, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_MULTILINE);
			pnlszr->Add(m_txtLog, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

			m_pgbWork=new wxGauge(m_pnlResults, -1, 100);
			pnlszr->Add(m_pgbWork, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

			m_pnlResults->SetSizer(pnlszr);
		m_szrMain->Add(m_pnlResults, 1, wxALL|wxEXPAND, 0);

		wxSizer *btnSzr=CreateSeparatedButtonSizer(wxAPPLY|wxCLOSE);
			m_btnStartStop=(wxButton*)FindWindowById(wxID_APPLY, this);
			m_btnStartStop->SetLabel(_("Start"));
		m_szrMain->Add(btnSzr, 0, wxALL|wxEXPAND, 5);

	SetSizer(m_szrMain);

	m_pnlResults->Hide();
	m_szrMain->Layout();
}

void DlgCheckDatas::ConnectControls()
{
	Bind(wxEVT_DBCHECK_STARTED, &DlgCheckDatas::OnDbCheckEvent, this);
	Bind(wxEVT_DBCHECK_WORKING, &DlgCheckDatas::OnDbCheckEvent, this);
	Bind(wxEVT_DBCHECK_STOPPED, &DlgCheckDatas::OnDbCheckEvent, this);
	Bind(wxEVT_DBCHECK_ENDED, &DlgCheckDatas::OnDbCheckEvent, this);
	Bind(wxEVT_BUTTON, &DlgCheckDatas::OnBtnStartStopClicked, this, wxID_APPLY);
	Bind(wxEVT_UPDATE_UI, &DlgCheckDatas::OnUpdateUI_BtnClose, this, wxID_CLOSE);
}

void DlgCheckDatas::DoCheckDB()
{
	if (m_bIsRunning)
		return;
	m_bIsRunning=true;
	m_bStopWanted=false;
	wxCommandEvent evt(wxEVT_DBCHECK_STARTED);
	GetEventHandler()->AddPendingEvent(evt);
	wxStopWatch sw;
	int i=0;
	evt.SetEventType(wxEVT_DBCHECK_WORKING);
	while((i<100) && (m_bStopWanted==false))
	{
		evt.SetString(wxString::Format(_T("\n- Step #%02d"), i+1));
		evt.SetInt(i+1);
		GetEventHandler()->AddPendingEvent(evt);

		while(sw.Time()<50)
			wxTheApp->Yield();
		sw.Start();

		i++;
	}
	if (m_bStopWanted)
	{
		evt.SetEventType(wxEVT_DBCHECK_STOPPED);
		evt.SetString(_("\nCancelled..."));
	}
	else
	{
		evt.SetEventType(wxEVT_DBCHECK_ENDED);
		evt.SetString(_T("\nTerminated successfully"));
	}
	m_bIsRunning=false;
	GetEventHandler()->AddPendingEvent(evt);
}

void DlgCheckDatas::OnBtnStartStopClicked(wxCommandEvent& event)
{
	if (m_bIsRunning)
	{
		m_bStopWanted=true;
	}
	else
	{
		CallAfter(&DlgCheckDatas::DoCheckDB);
	}
}

void DlgCheckDatas::OnUpdateUI_BtnClose(wxUpdateUIEvent& event)
{
	event.Enable(m_bIsRunning==false);
}

void DlgCheckDatas::OnDbCheckEvent(wxCommandEvent& event)
{
	int iType=event.GetEventType();
	if (iType==wxEVT_DBCHECK_STARTED)
	{
		m_btnStartStop->SetLabel(_("Stop"));
		m_pnlParams->Hide();
		m_pnlResults->Show();
		m_szrMain->Layout();
		m_txtLog->ChangeValue(_T("Working:"));
		m_pgbWork->SetValue(0);
	}
	if (iType==wxEVT_DBCHECK_WORKING)
	{
		m_pgbWork->SetValue(event.GetInt());
		*m_txtLog << event.GetString();
	}
	if (iType==wxEVT_DBCHECK_STOPPED)
	{
		*m_txtLog << event.GetString();
		wxStopWatch sw;
		// Wait a 1/2 second before comming back to the params panel
		while(sw.Time()<500)
			wxTheApp->Yield();
		m_btnStartStop->SetLabel(_("Start"));
		m_pnlParams->Show();
		m_pnlResults->Hide();
		m_szrMain->Layout();
	}
	if (iType==wxEVT_DBCHECK_ENDED)
	{
		*m_txtLog << event.GetString();
		m_btnStartStop->Disable();
	}
}
