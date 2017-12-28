#include "panelfilter.h"

#include <wx/xml/xml.h>
#include <wx/srchctrl.h>

PanelFilter::PanelFilter(wxWindow *parent) : wxPanel(parent, wxID_ANY)
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"PanelFilter\" object\n"));
#endif // DEBUG

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
			m_txtSearch=new wxSearchCtrl(this, -1);
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
	//
}

bool PanelFilter::DoesItemMatchSearch(wxXmlNode* item)
{
	return true;
}
