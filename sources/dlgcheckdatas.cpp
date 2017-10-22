#include "dlgcheckdatas.h"

#include <wx/xml/xml.h>
#include <wx/statline.h>

#include "geddate.h"
#include "datasmanager.h"

wxDEFINE_EVENT(wxEVT_DBCHECK_STARTED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DBCHECK_WORKING, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DBCHECK_STOPPED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DBCHECK_ENDED, wxCommandEvent);

#define PGB_RANGE 1000

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

			label=new wxStaticText(m_pnlParams, wxID_STATIC, _("Basic check operations:"));
			pnlszr->Add(label, 0, wxALL, 5);
			label=new wxStaticText(m_pnlParams, wxID_STATIC, _("- Isolated people"));
			pnlszr->Add(label, 0, wxLEFT|wxRIGHT, 5);
			label=new wxStaticText(m_pnlParams, wxID_STATIC, _("- People death date regarding its marriage date"));
			pnlszr->Add(label, 0, wxLEFT|wxRIGHT, 5);
			label=new wxStaticText(m_pnlParams, wxID_STATIC, _("- People death regarding its children birth"));
			pnlszr->Add(label, 0, wxLEFT|wxRIGHT, 5);

				lnszr=new wxBoxSizer(wxHORIZONTAL);
					label=new wxStaticText(m_pnlParams, wxID_STATIC, _("Customizable parameters:"));
				lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
				lnszr->Add(new wxStaticLine(m_pnlParams, wxID_STATIC), 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
			pnlszr->Add(lnszr, 0, wxALL|wxEXPAND, 5);

			label=new wxStaticText(m_pnlParams, wxID_STATIC, _("None yet..."));
			pnlszr->Add(label, 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

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

			m_pgbWork=new wxGauge(m_pnlResults, -1, PGB_RANGE);
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
	AddPendingEvent(evt);
	m_arsErrors.Clear();

	int iCount=m_datas.GetItemsCount(GIT_INDI);
	int iNbSteps=iCount; // 1 pass for isolated people
	iNbSteps += iCount;  // 1 pass for death date / marriage date and child birth
	int iCurrStep=0;
	double dPrct=0., dOldPrct=0.;
	int iPrct;
	evt.SetEventType(wxEVT_DBCHECK_WORKING);

	wxXmlNode *root, *item;
	int iErrs;

	/// First pass : Check for isolated people (no family)
	root=m_datas.GetDatas();
	item=root->GetChildren();
	iErrs=0;
	evt.SetString(wxString(_T("\n"))<<_("- Checking isolated people"));
	evt.SetInt(dPrct);
	AddPendingEvent(evt);
	evt.SetString(wxEmptyString);
	while((item!=NULL) && (m_bStopWanted==false))
	{
		if (item->GetAttribute(_T("Type"))==_T("INDI"))
		{
			iCurrStep++;
			dPrct=PGB_RANGE*iCurrStep/iNbSteps;
			iPrct=dPrct*10;
			dPrct=iPrct/10;
			if (dPrct!=dOldPrct)
			{
				evt.SetInt(dPrct);
				AddPendingEvent(evt);
				dOldPrct=dPrct;
			}
			wxXmlNode *subItem=item->GetChildren();
			bool bOk=false;
			while(subItem!=NULL)
			{
				if ((subItem->GetAttribute(_T("Type"))==_T("FAMC"))||(subItem->GetAttribute(_T("Type"))==_T("FAMS")))
					bOk=true;
				subItem=subItem->GetNext();
			}
			if (!bOk)
			{
				iErrs++;
				m_arsErrors.Add(wxString::Format(_("Person with ID %s seems to be isolated"), item->GetAttribute(_T("GedId"))));
			}
		}
		item=item->GetNext();
	}
	if (m_bStopWanted)
	{
		evt.SetEventType(wxEVT_DBCHECK_STOPPED);
		evt.SetString(_("\nCancelled..."));
		AddPendingEvent(evt);
		return;
	}
	else
	{
		if (iErrs==0)
		{
			evt.SetString(_T(" => Ok"));
		}
		else
		{
			if (iErrs==1)
				evt.SetString(_(" => 1 Error"));
			else
				evt.SetString(wxString::Format(_(" => %d Errors"), iErrs));
		}
	}
	// Make a 1/2 second wait until continuation
	{
		wxStopWatch sw;
		while(sw.Time()<500)
			wxTheApp->Yield();
	}
	AddPendingEvent(evt);

	/// Second pass : Death date before child birth (can arrive, but rarely) and marriage date
	item=root->GetChildren();
	iErrs=0;
	evt.SetString(wxString(_T("\n"))<<_("- Checking death date / marriage date and children birth"));
	evt.SetInt(dPrct);
	AddPendingEvent(evt);
	evt.SetString(wxEmptyString);
	while((item!=NULL) && (m_bStopWanted==false))
	{
		if (item->GetAttribute(_T("Type"))==_T("INDI"))
		{
			iCurrStep++;
			dPrct=PGB_RANGE*iCurrStep/iNbSteps;
			iPrct=dPrct*10;
			dPrct=iPrct/10;
			if (dPrct!=dOldPrct)
			{
				evt.SetInt(dPrct);
				AddPendingEvent(evt);
				dOldPrct=dPrct;
			}
			GEDITEMSEX itmSex=m_datas.GetItemSex(item);
			GedDate gdDeath;
			if (m_datas.GetItemDeath(item, gdDeath))
			{
				wxXmlNode *subItem=item->GetChildren();
				while(subItem!=NULL)
				{
					if (subItem->GetAttribute(_T("Type"))==_T("FAMS"))
					{
						wxXmlNode* evtNode=m_datas.FindItemByGedId(subItem->GetAttribute(_T("Value")));
						if (evtNode!=NULL)
						{
							wxXmlNode *subEvtNode=evtNode->GetChildren();
							while(subEvtNode)
							{
								if (subEvtNode->GetAttribute(_T("Type"))==_T("MARR"))
								{
									// Check the marriage date
									GedDate gdMarr;
									if (m_datas.GetEventDate(subEvtNode, gdMarr))
									{
										// We can do the check
										if (gdDeath.IsBefore(gdMarr))
										{
											iErrs++;
											m_arsErrors.Add(wxString::Format(_("Person with ID %s is dead before its marriage"), item->GetAttribute(_T("GedId"))));
										}
									}
								}
								if (subEvtNode->GetAttribute(_T("Type"))==_T("CHIL"))
								{
									// Check child birth date
									wxXmlNode *chldNode=m_datas.FindItemByGedId(subEvtNode->GetAttribute(_T("GedId")));
									if (chldNode!=NULL)
									{
										GedDate gdBirth;
										if (m_datas.GetItemBirth(chldNode, gdBirth))
										{
											// We can do the check
											if (gdDeath.IsBefore(gdBirth))
											{
												iErrs++;
												wxString sErr=wxString::Format(_("Person with ID %s is dead death before its child %s birth"), item->GetAttribute(_T("GedId")), subEvtNode->GetAttribute(_T("GedId")));
												if (itmSex==GIS_MALE)
													sErr << _T(" (") << _("this can be normal because it is a man") << _T(")");
												m_arsErrors.Add(sErr);
											}
										}
									}
								}
								subEvtNode=subEvtNode->GetNext();
							}
						}
					}
					subItem=subItem->GetNext();
				}
			}
		}
		item=item->GetNext();
	}
	if (m_bStopWanted)
	{
		evt.SetEventType(wxEVT_DBCHECK_STOPPED);
		evt.SetString(wxString(_T("\n")) << _("Cancelled..."));
		AddPendingEvent(evt);
		return;
	}
	else
	{
		if (iErrs==0)
		{
			evt.SetString(_T(" => Ok"));
		}
		else
		{
			if (iErrs==1)
				evt.SetString(_(" => 1 Error"));
			else
				evt.SetString(wxString::Format(_(" => %d Errors"), iErrs));
		}
	}
	AddPendingEvent(evt);

	evt.SetEventType(wxEVT_DBCHECK_ENDED);
	evt.SetString(wxString(_T("\n")) << _("Terminated successfully"));
	m_bIsRunning=false;
	AddPendingEvent(evt);
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
		if (!event.GetString().IsEmpty())
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
		if (m_arsErrors.IsEmpty())
		{
			*m_txtLog << _T("\n") << _("No errors found");
		}
		else
		{
			*m_txtLog << _T("\n") << _("Some errors were found:");
			for (size_t i=0; i<m_arsErrors.GetCount(); ++i)
			{
				*m_txtLog << _T("\n") << m_arsErrors[i];
			}
		}
		m_btnStartStop->Disable();
	}
}
