#include "datasmanager.h"

#include <wx/artprov.h>
#include <wx/xml/xml.h>
#include <wx/txtstrm.h>
#include <wx/zstream.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/busyinfo.h>

const wxChar* szKnownSubItems[] = { _T("DATE"), _T("TIME"), _T("NAME"), _T("PLAC"), _T("SEX"), _T("FAMC"), _T("FAMS")};
const wxChar* szKnownEvents[] = {_T("UNKN"), _T("BIRT"), _T("DEAT"), _T("MARR"), _T("BAPM"), _T("BURI")};
const wxChar* szKnownFamItems[] = { _T("HUSB"), _T("WIFE"), _T("CHIL")};
const wxChar* g_sUnknownYear=_T("----");

DatasManager DatasManager::m_instance;

DatasManager::DatasManager()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DatasManager\" object\n"));
#endif // __WXDEBUG__
    m_datas=NULL;
    m_bInitialized=false;
    m_bModified=false;
}

DatasManager::~DatasManager()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"DatasManager\" object\n"));
#endif // __WXDEBUG__
    Clear();
    // The "Clear()" function recreate the node : we have to delete it
    delete m_datas;
}

DatasManager& DatasManager::Get()
{
    if (!m_instance.m_bInitialized)
        m_instance.Initialize();

    return m_instance;
}

void DatasManager::Initialize()
{
    // Do all initialization stuff here
    m_datas=new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T(""));
    m_sFileName=wxEmptyString;
    m_iMaxLevels=-1;
    m_sLastError=wxEmptyString;

    // Mark the manager as initialized
    m_bInitialized=true;
}

void DatasManager::Clear()
{
    if (m_datas!=NULL)
    {
        delete m_datas;
        m_datas=new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T(""));
    }
    m_sFileName.Clear();

    m_arsCompAdded.Clear();
    m_arsCompModified.Clear();
    m_arsCompRemoved.Clear();
    m_sCmpFile.Clear();

    m_bModified=false;
}

bool DatasManager::IsModified()
{
    return m_bModified;
}

bool DatasManager::HasDatas()
{
    return (m_datas->GetChildren()!=NULL);
}

int DatasManager::GetItemsCount(GEDITEMTYPE type)
{
    int iRes=0;

    wxXmlNode *node=m_datas->GetChildren();

    wxString sType;

    while(node!=NULL)
    {
        sType=node->GetAttribute(_T("Type"));
        if (type==GIT_ALL)
        {
            ++iRes;
        }
        else
        {
            switch(type)
            {
                case GIT_INDI:
                    if (sType==_T("INDI")) ++iRes;
                    break;
                case GIT_FAM:
                    if (sType==_T("FAM")) ++iRes;
                        break;
            }
        }

        node=node->GetNext();
    }

    return iRes;
}

wxXmlNode* DatasManager::FindItemByGedId(const wxString& gedId)
{
    if (gedId.IsEmpty()) return NULL;

    wxXmlNode *node=m_datas->GetChildren();

    while(node)
    {
        if (node->GetAttribute(_T("GedId"))==gedId)
            return node;

        node=node->GetNext();
    }

    return NULL;
}

bool DatasManager::ParseGedToXml(wxInputStream* source, wxXmlNode* dest)
{
    if (source==NULL) return false;
    if (!source->IsOk()) return false;
    if (dest==NULL) return false;

    // Start reading the source stream
    wxTextInputStream t_in(*source);

    wxString sLine=wxEmptyString, sID, sValue;
    wxChar c=_T('0'), lastC=_T('0');
    wxXmlNode *node=NULL;

    while(sLine!=_T("TRLR"))
    {
        sLine=t_in.ReadLine();
        sID=wxEmptyString;
        sLine.Trim(false);
        sLine.Trim(true);
        c=sLine[0];
        sLine[0]=_T(' ');
        sLine.Trim(false);
        if (sLine.Matches(_T("@*@ *")))
        {
            int p=sLine.Find(_T('@'), true);
            if (p!=wxNOT_FOUND)
            {
                sID=sLine.Left(p+1);
                sLine.Remove(0, p+2);
            }
        }
        if (c==lastC)
        {
            // Same level than the last item
            if (node==NULL)
            {
                node=new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T(""));
                dest->AddChild(node);
            }
            else
            {
                node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("")));
                node->GetNext()->SetParent(node->GetParent());
                node=node->GetNext();
            }
        }
        else
        {
            if (c<lastC)
            {
                // Up one or more level
                while(lastC>c)
                {
                    node=node->GetParent();
                    lastC--;
                }
                node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("")));
                node->GetNext()->SetParent(node->GetParent());
                node=node->GetNext();
            }
            else
            {
                // Children of the last level
                // Should be only one level different, but who knows ?
                while(lastC<c)
                {
                    wxXmlNode *subnode=new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T(""));
                    node->AddChild(subnode);
                    node=subnode;
                    lastC++;
                }
            }
        }
        wxString sName=_T("Item");
        for(wxChar ic=c; ic>_T('0'); ic--) sName.Prepend(_T("Sub"));
        node->SetName(sName);

        if (!sID.IsEmpty())
        {
            node->AddAttribute(_T("GedId"), sID);
            sLine.Trim();
            int p=sLine.Find(_T(' '));
            if (p==wxNOT_FOUND)
            {
                node->AddAttribute(_T("Type"), sLine);
            }
            else
            {
                node->AddAttribute(_T("Type"), sLine.Left(p));
                node->AddAttribute(_T("Value"), sLine.Mid(p+1));
            }
        }
        else
        {
            bool bKnown=false;
            for (unsigned int i=0; i<WXSIZEOF(szKnownEvents); i++)
            {
                if (sLine.StartsWith(szKnownEvents[i], &sValue))
                {
                    bKnown=true;
                    node->SetName(_T("Event"));
                    node->AddAttribute(_T("Type"), szKnownEvents[i]);
                    sValue.Trim();
                    if (!sValue.IsEmpty())
                        node->AddAttribute(_T("Value"), sValue);
                    break;
                }
            }
            if (!bKnown)
            {
                for (unsigned int i=0; (bKnown==false) && (i<WXSIZEOF(szKnownSubItems)); i++)
                {
                    if (sLine.StartsWith(szKnownSubItems[i], &sValue))
                    {
                        sValue.Trim(false);
                        bKnown=true;
                        node->AddAttribute(_T("Type"), szKnownSubItems[i]);
                        if (!sValue.IsEmpty()) node->AddAttribute(_T("Value"), sValue);
                        break;
                    }
                }
            }
            if (!bKnown)
            {
                for (unsigned int i=0; (bKnown==false) && (i<WXSIZEOF(szKnownFamItems)); i++)
                {
                    if (sLine.StartsWith(szKnownFamItems[i], &sValue))
                    {
                        sValue.Trim(false);
                        node->SetName(_T("FamIndi"));
                        bKnown=true;
                        node->AddAttribute(_T("Type"), szKnownFamItems[i]);
                        if (!sValue.IsEmpty()) node->AddAttribute(_T("GedId"), sValue);
                        break;
                    }
                }
            }
            if (!bKnown)
                node->AddAttribute(_T("RawStr"), sLine);
        }
    }

    return true;
}

bool DatasManager::ReadGedFile(const wxString& filename)
{
    // Clean-up all remaining datas
    Clear();
    // Try to open the source file
    if (!wxFileExists(filename)) return false;
    wxFileInputStream f_in(filename);
    if (!f_in.IsOk()) return false;
    // We can now store the souce file name
    m_sFileName=filename;

    // The file will just been read : it won't been modified yet
    m_bModified=false;

    return ParseGedToXml(&f_in, m_datas);
}

bool DatasManager::SaveXmlFile(const wxString& filename, int compLevel)
{
    // Is there something to save ?
    if (m_datas==NULL) return false;
    // Is compLevel valid ?
    if ((compLevel<0) || (compLevel>9)) return false;

    m_datas->SetName(_T("GedView-Exported-File"));
    m_datas->AddAttribute(_T("Version"),_T("1.0"));
    m_datas->AddAttribute(_T("Date"), wxDateTime::Now().FormatISOCombined(_T(' ')));

    wxXmlDocument doc;
    doc.SetRoot(m_datas);

    wxFileOutputStream f_out(filename);

    bool bRes=false;
    if (compLevel>0)
    {
        wxZlibOutputStream z_out(f_out, compLevel);
        bRes=doc.Save(z_out);
    }
    else
    {
        bRes=doc.Save(f_out);
    }
    doc.DetachRoot();
    return bRes;
}

void getItemNames(wxXmlNode *container, wxString& first, wxString& last)
{
    if (container==NULL)
    {
        first=wxEmptyString;
        last=wxEmptyString;
        return;
    }
    wxXmlNode *node=container->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("Type"))==_T("NAME"))
        {
            wxString sName=node->GetAttribute(_T("Value"));
            int p=sName.Find(_T('/'));
            if (p==wxNOT_FOUND)
            {
                last=wxEmptyString;
                first=sName;
                return;
            }
            else
            {
                first=sName.Left(p);
                last=sName.Mid(p+1);
                last.RemoveLast();
                return;
            }
        }
        node=node->GetNext();
    }
    first=wxEmptyString;
    last=wxEmptyString;
}

wxString DatasManager::ParseEvent(wxXmlNode* event)
{
    if (event==NULL) return wxEmptyString;
    wxString sRes=wxEmptyString;
    wxString sType=event->GetAttribute(_T("Type"));
    if (sType.IsEmpty()) return wxEmptyString;

    if (sType==_T("BIRT"))
    {
        sRes << _("Birth:");
    }
    else if (sType==_T("DEAT"))
    {
        if (event->HasAttribute(_T("Value")))
            sRes << _("Dead");
        else
            sRes << _("Death:");
    }
    else if (sType==_T("MARR"))
    {
        if (!event->HasAttribute(_T("Value")))
            sRes << _("Marriage:");
    }
    else if (sType==_T("BAPM"))
    {
        if (!event->HasAttribute(_T("Value")))
            sRes << _("Baptism:");
    }
    else if (sType==_T("BURI"))
    {
        if (!event->HasAttribute(_T("Value")))
            sRes << _("Inhumation:");
    }
    else
        sRes << _("Unknown event:");

    wxXmlNode *subNode=event->GetChildren();
    while(subNode)
    {
        wxString sSubType=subNode->GetAttribute(_T("Type"));
        if (sSubType==_T("DATE"))
            sRes << (sRes.IsEmpty()?_T(""):_T(" ")) << ParseDate(subNode->GetAttribute(_T("Value")));
        else if (sSubType==_T("PLAC"))
            sRes << _T(" ") << _("in") << _T(" ") << subNode->GetAttribute(_T("Value"));

        subNode=subNode->GetNext();
    }

    return sRes;
}

wxString DatasManager::ParseDate(const wxString& value, bool yearOnly)
{
    // static const wxChar* szSpec[]={_T("AFT"), _T("BEF"), _T("ABT"), _T("EST")};
    wxString sRes=wxEmptyString;

    if (yearOnly)
    {
        if (value.Length()>3)
        {
            sRes=value.Right(4);
            if (value.StartsWith(_T("AFT"))) sRes.Append(_T("+"));
            if (value.StartsWith(_T("BEF"))) sRes.Append(_T("-"));
            if (value.StartsWith(_T("ABT"))) sRes.Prepend(_T("~"));
            if (value.StartsWith(_T("EST"))) sRes.Append(_T("?"));
            return sRes;
        }
        else
        {
            return g_sUnknownYear;
        }
    }

    return value;
}

wxString DatasManager::GetItemFirstName(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemFirstName(node);
        }
        node=node->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemFirstName(wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if (subNode->GetAttribute(_T("Type"))==_T("NAME"))
        {
            wxString sName=subNode->GetAttribute(_T("Value"));
            int iPos=sName.Find(_T('/'));
            if (iPos!=wxNOT_FOUND)
            {
                return sName.Left(iPos);
            }
            else
            {
                return sName;
            }
        }
        subNode=subNode->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemLastName(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemLastName(node);
        }
        node=node->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemLastName(wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxString sResult=wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if (subNode->GetAttribute(_T("Type"))==_T("NAME"))
        {
            wxString sName=subNode->GetAttribute(_T("Value"));
            int iPos=sName.Find(_T('/'));
            if (iPos!=wxNOT_FOUND)
            {
                sResult=sName.Mid(iPos+1);
                if (sResult.EndsWith(_T("/")))
                    sResult.RemoveLast(1);

                return sResult;
            }
            else
            {
                return sName;
            }
        }
        subNode=subNode->GetNext();
    }

    return sResult;
}

wxString DatasManager::GetItemFullName(const wxString& itmId, bool lastFirst)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemFullName(node, lastFirst);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemFullName(wxXmlNode* itmNode, bool lastFirst)
{
    if (itmNode==NULL) return wxEmptyString;

    wxString sFirst=wxEmptyString, sLast=wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if (subNode->GetAttribute(_T("Type"))==_T("NAME"))
        {
            wxString sName=subNode->GetAttribute(_T("Value"));
            int iPos=sName.Find(_T('/'));
            if (iPos!=wxNOT_FOUND)
            {
                sLast=sName.Mid(iPos+1);
                if (sLast.EndsWith(_T("/")))
                    sLast.RemoveLast(1);
                sFirst=sName.Left(iPos);
            }
            else
            {
                sLast=sName;
            }
        }
        subNode=subNode->GetNext();
    }

    if (sFirst.IsEmpty()) return sLast;

    wxString sResult=wxEmptyString;
    if (lastFirst)
        sResult << sLast << _T(" ") << sFirst;
    else
        sResult << sFirst << _T(" ") << sLast;

    return sResult;
}

GEDITEMSEX DatasManager::GetItemSex(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemSex(node);
        }
        node=node->GetNext();
    }
    return GIS_UNKNOWN;
}

GEDITEMSEX DatasManager::GetItemSex(const wxXmlNode* itmNode)
{
    if (itmNode==NULL) return GIS_UNKNOWN;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if (subNode->GetAttribute(_T("Type"))==_T("SEX"))
        {
            wxString sVal=subNode->GetAttribute(_T("Value")).Upper();
            return (sVal==_T("M")?GIS_MALE:(sVal==_T("F")?GIS_FEMALE:GIS_UNKNOWN));
        }
        subNode=subNode->GetNext();
    }

    return GIS_UNKNOWN;
}

wxString DatasManager::GetItemBirth(const wxString& itmId, bool yearOnly)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemBirth(node, yearOnly);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemBirth(const wxXmlNode* itmNode, bool yearOnly)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("BIRT")))
        {
            if (!yearOnly)
            {
                return ParseEvent(subNode);
            }
            else
            {
                wxXmlNode *subsubNode=subNode->GetChildren();
                while(subsubNode!=NULL)
                {
                    if (subsubNode->GetAttribute(_T("Type"))==_T("DATE"))
                    {
                        return ParseDate(subsubNode->GetAttribute(_T("Value")), true);
                    }
                    subsubNode=subsubNode->GetNext();
                }
                return g_sUnknownYear;
            }
        }
        subNode=subNode->GetNext();
    }

    return (yearOnly?g_sUnknownYear:wxEmptyString);
}

wxString DatasManager::GetItemBirthPlace(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemBirthPlace(node);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemBirthPlace(const wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("BIRT")))
        {
            wxXmlNode *subsubNode=subNode->GetChildren();
            while(subsubNode!=NULL)
            {
                if (subsubNode->GetAttribute(_T("Type"))==_T("PLAC"))
                {
                    return subsubNode->GetAttribute(_T("Value"));
                }
                subsubNode=subsubNode->GetNext();
            }
            return wxEmptyString;
        }
        subNode=subNode->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemBaptism(const wxString& itmId, bool yearOnly)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemBaptism(node, yearOnly);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemBaptism(const wxXmlNode* itmNode, bool yearOnly)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("BAPM")))
        {
            if (!yearOnly)
            {
                return ParseEvent(subNode);
            }
            else
            {
                wxXmlNode *subsubNode=subNode->GetChildren();
                while(subsubNode!=NULL)
                {
                    if (subsubNode->GetAttribute(_T("Type"))==_T("DATE"))
                    {
                        return ParseDate(subsubNode->GetAttribute(_T("Value")), true);
                    }
                    subsubNode=subsubNode->GetNext();
                }
                return g_sUnknownYear;
            }
        }
        subNode=subNode->GetNext();
    }

    return (yearOnly?g_sUnknownYear:wxEmptyString);
}

wxString DatasManager::GetItemBaptismPlace(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemBaptismPlace(node);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemBaptismPlace(const wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("BAPM")))
        {
            wxXmlNode *subsubNode=subNode->GetChildren();
            while(subsubNode!=NULL)
            {
                if (subsubNode->GetAttribute(_T("Type"))==_T("PLAC"))
                {
                    return subsubNode->GetAttribute(_T("Value"));
                }
                subsubNode=subsubNode->GetNext();
            }
            return wxEmptyString;
        }
        subNode=subNode->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemDeath(const wxString& itmId, bool yearOnly)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemDeath(node, yearOnly);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemDeath(const wxXmlNode* itmNode, bool yearOnly)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("DEAT")))
        {
            if (!yearOnly)
            {
                return ParseEvent(subNode);
            }
            else
            {
                wxXmlNode *subsubNode=subNode->GetChildren();
                while(subsubNode!=NULL)
                {
                    if (subsubNode->GetAttribute(_T("Type"))==_T("DATE"))
                    {
                        return ParseDate(subsubNode->GetAttribute(_T("Value")), true);
                    }
                    subsubNode=subsubNode->GetNext();
                }
                return g_sUnknownYear;
            }
        }
        subNode=subNode->GetNext();
    }

    return (yearOnly?g_sUnknownYear:wxEmptyString);
}

wxString DatasManager::GetItemDeathPlace(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemDeathPlace(node);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemDeathPlace(const wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("DEAT")))
        {
            wxXmlNode *subsubNode=subNode->GetChildren();
            while(subsubNode!=NULL)
            {
                if (subsubNode->GetAttribute(_T("Type"))==_T("PLAC"))
                {
                    return subsubNode->GetAttribute(_T("Value"));
                }
                subsubNode=subsubNode->GetNext();
            }
            return wxEmptyString;
        }
        subNode=subNode->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemBurial(const wxString& itmId, bool yearOnly)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemBurial(node, yearOnly);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemBurial(const wxXmlNode* itmNode, bool yearOnly)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("BURI")))
        {
            if (!yearOnly)
            {
                return ParseEvent(subNode);
            }
            else
            {
                wxXmlNode *subsubNode=subNode->GetChildren();
                while(subsubNode!=NULL)
                {
                    if (subsubNode->GetAttribute(_T("Type"))==_T("DATE"))
                    {
                        return ParseDate(subsubNode->GetAttribute(_T("Value")), true);
                    }
                    subsubNode=subsubNode->GetNext();
                }
                return g_sUnknownYear;
            }
        }
        subNode=subNode->GetNext();
    }

    return (yearOnly?g_sUnknownYear:wxEmptyString);
}

wxString DatasManager::GetItemBurialPlace(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemBurialPlace(node);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemBurialPlace(const wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("BURI")))
        {
            wxXmlNode *subsubNode=subNode->GetChildren();
            while(subsubNode!=NULL)
            {
                if (subsubNode->GetAttribute(_T("Type"))==_T("PLAC"))
                {
                    return subsubNode->GetAttribute(_T("Value"));
                }
                subsubNode=subsubNode->GetNext();
            }
            return wxEmptyString;
        }
        subNode=subNode->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemInfos(wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxString sItmID=itmNode->GetAttribute(_T("GedId"));
    if (!sItmID.Matches(_T("@I*@"))) return wxEmptyString;

    wxString sResult=wxEmptyString;
    int iSex=GetItemSex(itmNode);

    sResult << sItmID << wxString::Format(_T(" (%c) : "), (iSex==GIS_MALE?_T('M'):(iSex==GIS_FEMALE?_T('F'):_T('?')))) << GetItemFullName(itmNode) << _T("\n");

    wxXmlNode *subNode=itmNode->GetChildren();
    bool bUnions=false;
    while(subNode!=NULL)
    {
        wxString sType=subNode->GetAttribute(_T("Type"));
        if (subNode->GetName()==_T("Event"))
        {
            wxString sEvt=ParseEvent(subNode);
            if ((sEvt==_("Dead"))&&(iSex==GIS_FEMALE))
            {
                sEvt=_("Dead_F");
            }
            sResult << _T(" ") << sEvt << _T("\n");
        }
        if (sType==_T("FAMC"))
        {
            sResult << _("Parents") << _T("\n");
            wxArrayString arsSiblings;
            arsSiblings.Clear();
            wxXmlNode* evtNode=FindItemByGedId(subNode->GetAttribute(_T("Value")));
            if (evtNode!=NULL)
            {
                wxXmlNode *subEvt=evtNode->GetChildren();
                wxString sSubTyp, sSubId, sEvent;
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    sSubId=subEvt->GetAttribute(_T("GedId"));
                    if ((sSubTyp==_T("HUSB"))||(sSubTyp==_T("WIFE")))
                    {
                        sResult << (sSubTyp==_T("HUSB")?_("Father:"):_("Mother:")) << sSubId << _T(" - ") << GetItemFullName(sSubId) << _T("\n");
                        sEvent=GetItemBirth(sSubId);
                        if (!sEvent.IsEmpty())
                            sResult << _T(" ") << sEvent << _T("\n");
                        sEvent=GetItemDeath(sSubId);
                        if (!sEvent.IsEmpty())
                        {
                            if ((sEvent==_("Dead"))&&(GetItemSex(sSubId)==GIS_FEMALE))
                                sEvent=_("Dead_F");
                            sResult << _T(" ") << sEvent << _T("\n");
                        }
                    }
                    if (sSubTyp==_T("CHIL"))
                    {
                        arsSiblings.Add(sSubId);
                    }
                    subEvt=subEvt->GetNext();
                }
                if (arsSiblings.GetCount()>1)
                {
                    sResult << _("Siblings") << _T("\n");
                    for (size_t s=0; s<arsSiblings.GetCount(); s++)
                    {
                        if (arsSiblings[s]!=sItmID)
                        {
                            sResult << arsSiblings[s] << _T(" - ")<< GetItemFirstName(arsSiblings[s]) << _T("\n");
                            sEvent=GetItemBirth(arsSiblings[s]);
                            if (!sEvent.IsEmpty())
                                sResult << _T(" ") << sEvent << _T("\n");
                            sEvent=GetItemDeath(arsSiblings[s]);
                            if (!sEvent.IsEmpty())
                            {
                                if ((sEvent==_("Dead"))&&(GetItemSex(arsSiblings[s])==GIS_FEMALE))
                                    sEvent=_("Dead_F");
                                sResult << _T(" ") << sEvent << _T("\n");
                            }
                        }
                    }
                }
            }
        }
        if (sType==_T("FAMS"))
        {
            if (!bUnions)
            {
                sResult << _("Union(s)") << _T("\n");
                bUnions=true;
            }
            wxXmlNode* evtNode=FindItemByGedId(subNode->GetAttribute(_T("Value")));
            wxString sEvent;
            if (evtNode!=NULL)
            {
                wxXmlNode *subEvt=evtNode->GetChildren();
                wxString sSubTyp;
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    wxString sEvtId=subEvt->GetAttribute(_T("GedId"));
                    if (subEvt->GetName()==_T("Event"))
                    {
                        wxString sTmp=ParseEvent(subEvt);
                        if (!sTmp.IsEmpty())
                            sResult << _T(" ") << sTmp << _T("\n");
                    }
                    if (((sSubTyp==_T("HUSB"))||(sSubTyp==_T("WIFE")))&&(!sEvtId.IsEmpty())&&(sEvtId!=sItmID))
                    {
                        sResult << sEvtId << _T(" - ") << GetItemFullName(sEvtId) << _T("\n");
                        sEvent=GetItemBirth(sEvtId);
                        if (!sEvent.IsEmpty())
                            sResult << _T(" ") << sEvent << _T("\n");
                        sEvent=GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                        {
                            if (sEvent==_("Dead") && (sSubTyp==_T("WIFE")))
                            {
                                sEvent=_("Dead_F");
                            }
                            sResult << _T(" ") << sEvent << _T("\n");
                        }
                    }
                    if (sSubTyp==_T("CHIL"))
                    {
                        sResult << _T(" -> ") << sEvtId << _T(" - ") << GetItemFirstName(sEvtId) << _T("\n");
                        sEvent=GetItemBirth(sEvtId);
                        if (!sEvent.IsEmpty())
                            sResult << _T("     ") << sEvent << _T("\n");
                        sEvent=GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                        {
                            if ((sEvent==_("Dead")) && (GetItemSex(sEvtId)==GIS_FEMALE))
                                sEvent=_("Dead_F");
                            sResult << _T("     ") << sEvent << _T("\n");
                        }
                    }
                    subEvt=subEvt->GetNext();
                }
            }
        }
        subNode=subNode->GetNext();
    }

    return sResult;
}

bool DatasManager::CompareWithGedFile(const wxString& filename)
{
    if (!HasDatas())
    {
        m_sLastError=_("No datas in memory can be compared");
        return false;
    }
    if (filename==m_sFileName)
    {
        m_sLastError=_("The selected file is already loaded in memory");
        return false;
    }

    wxFileInputStream f_in(filename);
    if (!f_in.IsOk())
    {
        m_sLastError=_("Unable to open the selected file");
        return false;
    }

    wxXmlNode cmpNode, *subNode;
    if (!ParseGedToXml(&f_in, &cmpNode))
    {
        m_sLastError=_("Unable to parse the selected file");
        return false;
    }

    wxStopWatch sw;
    sw.Start();
    wxWindowDisabler disableAll;
    wxBusyInfo info(
                    wxBusyInfoFlags()
                    .Parent(wxTheApp->GetTopWindow())
                    .Title(_T("<b>") + _("Comparing datas") + _T("</b>"))
                    .Text(_("Please wait..."))
                    .Foreground(*wxWHITE)
                    .Background(*wxBLACK)
                    .Transparency(3*wxALPHA_OPAQUE/5)
                    );

    wxArrayString arsSrc, arsCmp;
    m_arsCompAdded.Clear();
    m_arsCompModified.Clear();
    m_arsCompRemoved.Clear();
    m_sCmpFile=filename;

    // Get the lists of the items
    subNode=m_datas->GetChildren();
    while(subNode!=NULL)
    {
        if (subNode->GetAttribute(_T("Type"))==_T("INDI"))
            arsSrc.Add(subNode->GetAttribute(_T("GedId")));
        subNode=subNode->GetNext();
    }
    subNode=cmpNode.GetChildren();
    while(subNode!=NULL)
    {
        if (subNode->GetAttribute(_T("Type"))==_T("INDI"))
            arsCmp.Add(subNode->GetAttribute(_T("GedId")));
        subNode=subNode->GetNext();
    }
    // Check for added / removed items
    int iSrcCount=arsSrc.GetCount(), iCmpCount=arsCmp.GetCount();
    for (int i=0; i<iSrcCount; ++i)
    {
        if (arsCmp.Index(arsSrc[i])==wxNOT_FOUND)
            m_arsCompAdded.Add(arsSrc[i]);
        else
            m_arsCompModified.Add(arsSrc[i]);
    }
    for (int i=0; i<iCmpCount; ++i)
    {
        if (arsSrc.Index(arsCmp[i])==wxNOT_FOUND)
            m_arsCompRemoved.Add(arsCmp[i]);
    }
    // Check for modified items
    wxString sSrc, sCmp;
    int iCount=m_arsCompModified.GetCount();
    for (int i=iCount-1; i>=0; i--)
    {
        sSrc.Empty();
        sCmp.Empty();
        subNode=m_datas->GetChildren();
        while(subNode!=NULL)
        {
            if (subNode->GetAttribute(_T("GedId"))==m_arsCompModified[i])
            {
                sSrc=GetItemInfos(subNode);
                subNode=NULL;
            }
            else
            {
                subNode=subNode->GetNext();
            }
        }
        subNode=cmpNode.GetChildren();
        while(subNode!=NULL)
        {
            if (subNode->GetAttribute(_T("GedId"))==m_arsCompModified[i])
            {
                sCmp=GetItemInfos(subNode);
                subNode=NULL;
            }
            else
            {
                subNode=subNode->GetNext();
            }
        }
        if (!sSrc.IsEmpty() && !sCmp.IsEmpty())
        {
            if (sSrc==sCmp)
                m_arsCompModified.RemoveAt(i);
        }
    }
    if (!m_arsCompAdded.IsEmpty())
    {
        iCount=m_arsCompAdded.GetCount();
        for (int i=0; i<iCount; ++i)
        {
            subNode=m_datas->GetChildren();
            while(subNode!=NULL)
            {
                if (subNode->GetAttribute(_T("GedId"))==m_arsCompAdded[i])
                {
                    m_arsCompAdded[i] << _T(" - ") << GetItemFullName(subNode);
                    subNode=NULL;
                }
                else
                {
                    subNode=subNode->GetNext();
                }
            }
        }
    }
    if (!m_arsCompRemoved.IsEmpty())
    {
        iCount=m_arsCompRemoved.GetCount();
        for (int i=0; i<iCount; ++i)
        {
            subNode=cmpNode.GetChildren();
            while(subNode!=NULL)
            {
                if (subNode->GetAttribute(_T("GedId"))==m_arsCompRemoved[i])
                {
                    m_arsCompRemoved[i] << _T(" - ") << GetItemFullName(subNode);
                    subNode=NULL;
                }
                else
                {
                    subNode=subNode->GetNext();
                }
            }
        }
    }
    if (!m_arsCompModified.IsEmpty())
    {
        iCount=m_arsCompModified.GetCount();
        for (int i=0; i<iCount; ++i)
        {
            subNode=m_datas->GetChildren();
            while(subNode!=NULL)
            {
                if (subNode->GetAttribute(_T("GedId"))==m_arsCompModified[i])
                {
                    m_arsCompModified[i] << _T(" - ") << GetItemFullName(subNode);
                    subNode=NULL;
                }
                else
                {
                    subNode=subNode->GetNext();
                }
            }
        }
    }

    while(sw.Time()<2000)
        wxMilliSleep(150);
    return true;
}

wxString DatasManager::GetCompResultsSummary()
{
    wxString sResult=_("Changes from file:");
    sResult << _T("\n") << m_sCmpFile;

    if (!m_arsCompAdded.IsEmpty())
    {
        int iCount=m_arsCompAdded.GetCount();
        if (iCount==1)
            sResult << _T("\n") << _("1 item added:");
        else
            sResult << _T("\n") << wxString::Format(_("%d items added:"), iCount);
        for (int i=0; i<iCount; ++i)
        {
            sResult << _T("\n - ") << m_arsCompAdded[i];
        }
    }
    if (!m_arsCompRemoved.IsEmpty())
    {
        int iCount=m_arsCompRemoved.GetCount();
        if (iCount==1)
            sResult << _T("\n") << _("1 item removed:");
        else
            sResult << _T("\n") << wxString::Format(_("%d items removed:"), iCount);
        for (int i=0; i<iCount; ++i)
        {
            sResult << _T("\n - ") << m_arsCompRemoved[i];
        }
    }
    if (!m_arsCompModified.IsEmpty())
    {
        int iCount=m_arsCompModified.GetCount();
        if (iCount==1)
            sResult << _T("\n") << _("1 item modified:");
        else
            sResult << _T("\n") << wxString::Format(_("%d items modified:"), iCount);
        for (int i=0; i<iCount; ++i)
        {
            sResult << _T("\n - ") << m_arsCompModified[i];
        }
    }

    return sResult;
}

bool DatasManager::HasCompResults(bool deleted)
{
    if (!m_arsCompAdded.IsEmpty()) return true;
    if (!m_arsCompModified.IsEmpty()) return true;
    if (deleted && !m_arsCompRemoved.IsEmpty()) return true;

    return false;
}
