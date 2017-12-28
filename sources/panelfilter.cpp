#include "panelfilter.h"

#include <wx/xml/xml.h>
#include <wx/srchctrl.h>

#include "datasmanager.h"

wxDEFINE_EVENT(wxEVT_FILTER_CHANGED, wxCommandEvent);

PanelFilter::PanelFilter(wxWindow *parent, wxWindowID id) : wxPanel(parent, id),
	m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"PanelFilter\" object\n"));
#endif // DEBUG

	m_bLastSearchEmpty=true;

	CreateControls();

	ConnectControls();
}

PanelFilter::~PanelFilter()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"PanelFilter\" object\n"));
#endif // DEBUG
}

void PanelFilter::CreateControls()
{
	wxStaticText *label;

	wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL);

		wxStaticBoxSizer *szrStb=new wxStaticBoxSizer(wxHORIZONTAL, this, _("View filter:"));

			label=new wxStaticText(this, wxID_STATIC, _("String to search:"));
			szrStb->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
			m_txtSearch=new wxSearchCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
				m_txtSearch->ShowCancelButton(true);
			szrStb->Add(m_txtSearch, 1,  wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);

			label=new wxStaticText(this, wxID_STATIC, _("Search in:"));
			szrStb->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

			m_optSearch[0]=new wxRadioButton(this, -1, _("First Name"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			szrStb->Add(m_optSearch[0], 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);

			m_optSearch[1]=new wxRadioButton(this, -1, _("Last Name"));
			szrStb->Add(m_optSearch[1], 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);

			m_optSearch[2]=new wxRadioButton(this, -1, _("Full name"));
			szrStb->Add(m_optSearch[2], 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5);

		szrMain->Add(szrStb, 0, wxALL|wxEXPAND, 5);

	SetSizer(szrMain);
}

void PanelFilter::ConnectControls()
{
	Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &PanelFilter::OnSearchButtonClicked, this);
	Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &PanelFilter::OnSearchButtonClicked, this);
	Bind(wxEVT_TEXT_ENTER, &PanelFilter::OnSearchButtonClicked, this);
	for (int i=0; i<WXSIZEOF(m_optSearch); ++i)
	{
		m_optSearch[i]->Bind(wxEVT_RADIOBUTTON, &PanelFilter::OnOptSearchClicked, this);
	}
}

bool PanelFilter::DoesItemMatchSearch(wxXmlNode* item)
{
	if (m_txtSearch->IsEmpty())
		return true;

	if (item==NULL)
		return false;

	wxString sTxt=(m_optSearch[0]->GetValue()?m_datas.GetItemFirstName(item):(m_optSearch[1]->GetValue()?m_datas.GetItemLastName(item):m_datas.GetItemFullName(item)));

	// Try to match undepending on case
	sTxt.MakeLower();

	return (sTxt.Find(m_txtSearch->GetValue().Lower())!=wxNOT_FOUND);
}

void PanelFilter::OnSearchButtonClicked(wxCommandEvent& event)
{
	// To avoid posting unnecessary events, check if the search field has really changed
	if (m_txtSearch->IsEmpty())
	{
		if (m_bLastSearchEmpty==true)
			return;
		m_bLastSearchEmpty=true;
	}
	else
	{
		m_bLastSearchEmpty=false;
	}
	wxCommandEvent evt(wxEVT_FILTER_CHANGED, this->GetId());
	AddPendingEvent(evt);
}

void PanelFilter::OnOptSearchClicked(wxCommandEvent& event)
{
	// No need to tell the mainframe that the filter has changed if the search field is empty
	if (m_txtSearch->IsEmpty())
	{
		if (m_bLastSearchEmpty==false)
			m_bLastSearchEmpty=true;
		return;
	}
	else
	{
		m_bLastSearchEmpty=false;
	}

	wxCommandEvent evt(wxEVT_FILTER_CHANGED, this->GetId());
	AddPendingEvent(evt);
}
