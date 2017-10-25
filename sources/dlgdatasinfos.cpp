#include "dlgdatasinfos.h"

#include <wx/xml/xml.h>
#include <wx/statline.h>

#include "geddate.h"
#include "datasmanager.h"

DlgDatasInfos::DlgDatasInfos(wxWindow *parent) : wxDialog(parent, -1, _("Datas informations")),
	m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Creating a \"DlgDatasInfos\" object\n"));
#endif // __WXDEBUG__

	CreateControls();

	FillControls();

	GetSizer()->SetSizeHints(this);

	CenterOnParent();
}

DlgDatasInfos::~DlgDatasInfos()
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Destroying a \"DlgDatasInfos\" object\n"));
#endif // __WXDEBUG__
}

void DlgDatasInfos::CreateControls()
{
	wxBoxSizer *szrMain;
	wxFlexGridSizer *flxszr;

	wxStaticText *label;

	szrMain=new wxBoxSizer(wxVERTICAL);

		flxszr=new wxFlexGridSizer(2, 5, 5);

			label=new wxStaticText(this, wxID_STATIC, _("Gedcom file:"));
				wxFont fntBold=label->GetFont();
				fntBold.MakeBold();
			flxszr->Add(label, 0, wxALIGN_RIGHT);
			m_lblFileName=new wxStaticText(this, wxID_STATIC, _T("..."));
				m_lblFileName->SetFont(fntBold);
			flxszr->Add(m_lblFileName);

			label=new wxStaticText(this, wxID_STATIC, _("Items count:"));
			flxszr->Add(label, 0, wxALIGN_RIGHT);
			m_lblItems=new wxStaticText(this, wxID_STATIC, _T("..."));
				m_lblItems->SetFont(fntBold);
			flxszr->Add(m_lblItems);

			label=new wxStaticText(this, wxID_STATIC, _("File source:"));
			flxszr->Add(label, 0, wxALIGN_RIGHT);
			m_lblFileSource=new wxStaticText(this, wxID_STATIC, _T("..."));
				m_lblFileSource->SetFont(fntBold);
			flxszr->Add(m_lblFileSource);

			label=new wxStaticText(this, wxID_STATIC, _("Date:"));
			flxszr->Add(label, 0, wxALIGN_RIGHT);
			m_lblFileDate=new wxStaticText(this, wxID_STATIC, _T("..."));
				m_lblFileDate->SetFont(fntBold);
			flxszr->Add(m_lblFileDate);

		szrMain->Add(flxszr, 1, wxALL|wxEXPAND, 5);

		wxSizer *btnSzr=CreateSeparatedButtonSizer(wxCLOSE);
		szrMain->Add(btnSzr, 0, wxALL|wxEXPAND, 5);

	SetSizer(szrMain);
}

void DlgDatasInfos::FillControls()
{
	m_lblFileName->SetLabel(m_datas.GetCurrentFileName());
	int iIndi=m_datas.GetItemsCount(GIT_INDI);
	int iFams=m_datas.GetItemsCount(GIT_FAM);
	int iCount=iIndi+iFams;
	m_lblItems->SetLabel(wxString::Format(_("%d items in total (%d Individuals, %d Family events)"), iCount, iIndi, iFams));

	wxXmlNode *root=m_datas.GetDatas();
	wxXmlNode *node=root->GetChildren();
	wxString sSource=_("Unknown");
	wxString sDate=sSource;
	while(node)
	{
		if (node->GetAttribute(_T("RawStr"))==_T("HEAD"))
		{
			wxXmlNode *subNode=node->GetChildren();
			while(subNode)
			{
				if (subNode->GetAttribute(_T("Type"))==_T("SOUR"))
				{
					sSource=subNode->GetAttribute(_T("Value"));
					// There can be a sub-sub-node with the version value
					wxXmlNode *versNode=subNode->GetChildren();
					while(versNode)
					{
						wxString sVers=versNode->GetAttribute(_T("RawStr"));
						if (sVers.StartsWith(_T("VERS")))
						{
							sVers.Remove(0, 4);
							sSource << _T(" ") << _("Version") << sVers;
							break;
						}
						versNode=versNode->GetNext();
					}
				}
				if (subNode->GetAttribute(_T("Type"))==_T("DATE"))
				{
					sDate=m_datas.ParseDate(subNode->GetAttribute(_T("Value")));
				}
				subNode=subNode->GetNext();
			}
			break;
		}
		node=node->GetNext();
	}
	m_lblFileSource->SetLabel(sSource);
	m_lblFileDate->SetLabel(sDate);
}
