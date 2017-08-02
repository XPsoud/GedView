#include "recentslist.h"

#include <wx/xml/xml.h>

RecentsList::RecentsList()
{
	m_iMaxEntries=10;
	sEntry.Clear();
	m_mnuRecents=NULL;
	m_bModified=false;
}

RecentsList::~RecentsList()
{
    //
}

void RecentsList::Append(const wxString &sValue)
{
	wxString sTxt;
	if (((int)sEntry.Count()<m_iMaxEntries)&&(sValue!=wxEmptyString))
	{
		sEntry.Add(sValue);
		UpdateMenu();
	}
}

void RecentsList::SetLastUsed(const wxString &sValue)
{
	if (sValue==wxEmptyString) return;
	wxString pSVal=sValue;

	int idx=sEntry.Index(pSVal, false);
	wxString sTxt;
	if (idx!=wxNOT_FOUND)
	{
		if (idx==0) return;
		sEntry.RemoveAt(idx);
	}
	sEntry.Insert(pSVal, 0);

	while ((int)sEntry.Count()>m_iMaxEntries)
	{
		sEntry.RemoveAt(sEntry.Count()-1);
	}
	UpdateMenu();
	m_bModified=true;
}

wxString RecentsList::GetEntry(int Index)
{
	if (Index>(m_iMaxEntries-1)) return wxEmptyString;
	if (Index>(int)(sEntry.Count()-1)) return wxEmptyString;
	return sEntry[Index];
}

void RecentsList::SetMaxEntries(int iNbEntries)
{
	if ((iNbEntries==0)||(iNbEntries==m_iMaxEntries)) return;
	if (iNbEntries>10) return;
	if (iNbEntries<m_iMaxEntries)
	{
		while(iNbEntries<(int)sEntry.Count())
		{
			sEntry.RemoveAt(sEntry.Count()-1);
		}
	}
	m_iMaxEntries=iNbEntries;
	UpdateMenu();
	m_bModified=true;
}

void RecentsList::SetAssociatedMenu(wxMenu *Menu)
{
	m_mnuRecents=Menu;
	if (m_mnuRecents==NULL) return;
	if ((int)Menu->GetMenuItemCount()>0) return;
	int imax;
	wxString sTxt;
	imax=m_iMaxEntries;
	if ((int)sEntry.Count()<imax) imax=sEntry.Count();

	m_mnuRecents->Append(wxID_FILE, _("Empty list"));
	m_mnuRecents->Enable(wxID_FILE, false);
	UpdateMenu();
}

void RecentsList::UpdateMenu()
{
	int i,imax,immax;
	wxString sTxt;
	if (m_mnuRecents==NULL) return;
	immax=m_mnuRecents->GetMenuItemCount();
	imax=sEntry.Count();
	if (immax!=imax)
	{
		if ((immax==1)&&(imax==0)) return;
		while(immax>imax)
		{
			m_mnuRecents->Destroy(wxID_FILE+immax-1);
			immax--;
		}
		while(immax<imax)
		{
			m_mnuRecents->Append(wxID_FILE+immax, _T("Recent Entry"));
			immax++;
		}
	}
	for (i=0;i<immax;i++)
	{
		sTxt.Printf(_("Open file \"%s\""), sEntry[i].c_str());
		m_mnuRecents->SetLabel(wxID_FILE+i, sEntry[i]);
		m_mnuRecents->SetHelpString(wxID_FILE+i, sTxt);
	}
	m_mnuRecents->Enable(wxID_FILE, true);
}

void RecentsList::FromXmlNode(wxXmlNode *container)
{
    long l;
    container->GetAttribute(_T("MaxEntries"), _T("9")).ToLong(&l); SetMaxEntries(l);
    wxXmlNode* node=container->GetChildren();
    sEntry.Clear();
    while(node)
    {
        Append(node->GetAttribute(_T("Path"), _T("")));
        node = node->GetNext();
    }
    m_bModified=false;
}

void RecentsList::ToXmlNode(wxXmlNode *container)
{
    int iCount=(int)sEntry.Count();
    container->AddAttribute(_T("MaxEntries"), wxString::Format(_T("%0d"), m_iMaxEntries));
    if (iCount>0)
    {
        wxXmlNode* node=NULL;
        for (int i=0;i<iCount; i++)
        {
            if (node==NULL)
            {
                container->AddChild(node = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("RecentFile")));
            } else {
                node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("RecentFile")));
                node = node->GetNext();
            }
            node->AddAttribute(_T("Path"), sEntry[i]);
        }
    }
}
