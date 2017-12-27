#include "panelfilter.h"

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
	wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL);

		wxStaticBoxSizer *szrStb=new wxStaticBoxSizer(wxVERTICAL, this, _("View filter:"));

			// Fake control to see the panel "in place"
			szrStb->Add(new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY), 1, wxALL|wxEXPAND, 5);

		szrMain->Add(szrStb, 1, wxALL|wxEXPAND, 5);

	SetSizer(szrMain);
}

void PanelFilter::ConnectControls()
{
	//
}
