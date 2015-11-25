#include "dlgoptions.h"

#include "settingsmanager.h"

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/statline.h>
#include <wx/notebook.h>

DlgOptions::DlgOptions(wxWindow *parent)
    : wxDialog(parent, -1, _("Preferences"), wxDefaultPosition, wxDefaultSize),
    m_options(SettingsManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DlgOptions\" object\n"));
#endif // __WXDEBUG__

	CreateControls();

	ConnectControls();

	CenterOnParent();

	FillControls();
}

DlgOptions::~DlgOptions()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"DlgOptions\" object\n"));
#endif // __WXDEBUG__
}

void DlgOptions::CreateControls()
{
    wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL), *lnszr, *pageszr;

        m_nBook=new wxNotebook(this, -1);
        wxPanel *page;
        wxStaticBoxSizer *box, *box2;
        wxFlexGridSizer *flxszr;
        wxStaticText *label;

        // "General" tab
        page=new wxPanel(m_nBook, -1);
            pageszr=new wxBoxSizer(wxVERTICAL);

                box=new wxStaticBoxSizer(wxVERTICAL, page, _("Position of the main window: "));
                    label=new wxStaticText(page, -1, _("Position of the main window at application startup"));
                    box->Add(label, 0, wxALL, 5);
                    lnszr=new wxBoxSizer(wxHORIZONTAL);
                        m_optStartType[0]=new wxRadioButton(page, -1, _("Predefined position"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
                        lnszr->Add(m_optStartType[0], 0, wxALL, 5);
                        m_optStartType[1]=new wxRadioButton(page, -1, _("Last registered position"));
                        lnszr->Add(m_optStartType[1], 0, wxTOP|wxBOTTOM|wxRIGHT, 5);
                    box->Add(lnszr, 0, wxALL, 0);

                    box2=new wxStaticBoxSizer(wxVERTICAL, page, _("Predefined positions:"));
                    const wxChar* szLabels[9] = {   _("Top-Left"), _("Top-Center"), _("Top-Right"),
                                                    _("Middle-Left"), _("Center-Screen"), _("Middle-Right"),
                                                    _("Bottom-Left"), _("Bottom-Center"), _("Bottom-Right")};
                    m_stbPos=box2->GetStaticBox();
                    flxszr=new wxFlexGridSizer(3, 5, 5);
                        for (int i=0; i<9; i++)
                        {
                            m_optDefPos[i]=new wxRadioButton(page, -1, szLabels[i], wxDefaultPosition, wxDefaultSize, (i==0?wxRB_GROUP:0));
                            flxszr->Add(m_optDefPos[i]);
                        }
                        flxszr->AddGrowableCol(0, 1);
                        flxszr->AddGrowableCol(1, 1);
                        box2->Add(flxszr, 0, wxALL|wxEXPAND, 5);
                    box->Add(box2, 0, wxALL|wxEXPAND, 5);
                pageszr->Add(box, 0, wxLEFT|wxRIGHT|wxEXPAND, 5);

                box=new wxStaticBoxSizer(wxVERTICAL, page, _("Misc : "));
                    m_chkSingleInstance=new wxCheckBox(page, -1, _("Allow only one instance of the application"));
                    box->Add(m_chkSingleInstance, 0, wxALL, 5);
                pageszr->Add(box, 0, wxALL|wxEXPAND, 5);

            page->SetSizer(pageszr);
        m_nBook->AddPage(page, _("General"));

        szrMain->Add(m_nBook, 1, wxALL|wxEXPAND, 0);

        szrMain->Add(new wxStaticLine(this, -1), 0, wxTOP|wxBOTTOM|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            m_btnOk=new wxButton(this, wxID_OK, wxGetStockLabel(wxID_OK));
            lnszr->Add(m_btnOk, 0, wxALL, 0);
            m_btnCancel=new wxButton(this, wxID_CANCEL, wxGetStockLabel(wxID_CANCEL));
            lnszr->Add(m_btnCancel, 0, wxLEFT|wxRIGHT, 5);
            m_btnApply=new wxButton(this, wxID_APPLY, wxGetStockLabel(wxID_APPLY));
            lnszr->Add(m_btnApply, 0, wxALL, 0);
        szrMain->Add(lnszr, 0, wxALL|wxALIGN_RIGHT, 5);
    SetSizer(szrMain);

    szrMain->SetSizeHints(this);
}

void DlgOptions::ConnectControls()
{
    Connect(wxID_APPLY, wxEVT_BUTTON, wxCommandEventHandler(DlgOptions::OnBtnApplyClicked));
    Connect(wxID_OK, wxEVT_BUTTON, wxCommandEventHandler(DlgOptions::OnBtnOkClicked));
    for (int i=0; i<2; i++)
    {
        m_optStartType[i]->Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(DlgOptions::OnStartupPosTypeChanged), NULL, this);
    }
    for (int i=0; i<9; i++)
    {
        m_optDefPos[i]->Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(DlgOptions::OnSomethingHasChanged), NULL, this);
    }
    Connect(wxEVT_CHECKBOX, wxCommandEventHandler(DlgOptions::OnSomethingHasChanged));
    Connect(wxEVT_TEXT, wxCommandEventHandler(DlgOptions::OnSomethingHasChanged));
    Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(DlgOptions::OnSomethingHasChanged));
    Connect(wxEVT_CHOICE, wxCommandEventHandler(DlgOptions::OnSomethingHasChanged));
}

void DlgOptions::FillControls()
{
    int iStartPos=m_options.GetMainWndStartupPos();
	int iOptButton;
	wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, 0);
	if (iStartPos==wxALIGN_NOT)
    {
        evt.SetId(m_optStartType[1]->GetId());
        m_optStartType[1]->SetValue(true);
        m_optDefPos[4]->SetValue(true);
    }
    else
    {
        evt.SetId(m_optStartType[0]->GetId());
        m_optStartType[0]->SetValue(true);
        if (iStartPos==wxCENTER_ON_SCREEN)
        {
            iOptButton=4;
        }
        else
        {
            int iX=(((iStartPos&wxRIGHT)==wxRIGHT)?2:((iStartPos&wxLEFT)==wxLEFT)?0:1);
            int iY=(((iStartPos&wxBOTTOM)==wxBOTTOM)?6:((iStartPos&wxTOP)==wxTOP)?0:3);
            iOptButton=iX+iY;
        }
        m_optDefPos[iOptButton]->SetValue(true);
    }
    OnStartupPosTypeChanged(evt);

    m_chkSingleInstance->SetValue(m_options.GetMultipleInstancesAllowed()==false);

    m_btnApply->Disable();
}

bool DlgOptions::ApplySettings()
{
    int iIndex=wxNOT_FOUND;
    if (m_optStartType[0]->GetValue())
    {
        for (int i=0; i<9; i++)
        {
            if (m_optDefPos[i]->GetValue())
            {
                iIndex=i;
                break;
            }
        }
    }
    int iStartPos=wxALIGN_NOT;
    if (iIndex>wxNOT_FOUND)
    {
        int iH=(((iIndex==0)||(iIndex==3)||(iIndex==6))?wxLEFT:((iIndex==1)||(iIndex==4)||(iIndex==7))?(wxDirection)wxCENTER:wxRIGHT);
        int iV=(((iIndex>-1)&&(iIndex<3))?wxTOP:((iIndex>2)&&(iIndex<6))?(wxDirection)wxCENTER:wxBOTTOM);
        if ((iH==wxCENTER)&&(iV==wxCENTER))
        {
            iStartPos=wxCENTER_ON_SCREEN;
        }
        else
        {
            iStartPos=iH|iV;
        }
    }
    m_options.SetMainWndStartupPos(iStartPos);

    m_options.SetMultipleInstancesAllowed(m_chkSingleInstance->IsChecked()==false);

    m_btnApply->Disable();
    return true;
}

void DlgOptions::OnStartupPosTypeChanged(wxCommandEvent& event)
{
    bool bEnable=m_optStartType[0]->GetValue();
    m_stbPos->Enable(bEnable);
    for (int i=0; i<9; i++)
    {
        m_optDefPos[i]->Enable(bEnable);
    }
    OnSomethingHasChanged(event);
}

void DlgOptions::OnSomethingHasChanged(wxCommandEvent& event)
{
    m_btnApply->Enable();
}

void DlgOptions::OnBtnApplyClicked(wxCommandEvent& event)
{
    ApplySettings();
}

void DlgOptions::OnBtnOkClicked(wxCommandEvent& event)
{
    if (ApplySettings())
        EndModal(wxID_OK);
}