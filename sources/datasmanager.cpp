#include "datasmanager.h"

#include <wx/xml/xml.h>
#include <wx/txtstrm.h>
#include <wx/zstream.h>
#include <wx/wfstream.h>
#include <wx/filename.h>

const wxChar* szKnownSubItems[] = { _T("DATE"), _T("TIME"), _T("NAME"), _T("PLAC"), _T("SEX"), _T("FAMC"), _T("FAMS")};
const wxChar* szKnownEvents[] = {_T("UNKN"), _T("BIRT"), _T("DEAT"), _T("MARR")};
const wxChar* szKnownFamItems[] = { _T("HUSB"), _T("WIFE"), _T("CHIL")};
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
    m_baseItem=NULL;

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

    if (!m_treeItems.IsEmpty())
    {
        m_treeItems.DeleteContents(true);
        m_treeItems.Clear();
    }

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

GedTreeItem *DatasManager::FindTreeItem(const wxString& itemID)
{
    if ((itemID.IsEmpty()) || (!itemID.Matches(_T("@*@")))) return NULL;
    if (m_datas==NULL) return NULL;

    // First, find the corresponding xml node
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("Type"))==_T("INDI"))
        {
            if (node->GetAttribute(_T("GedId"))==itemID)
            {
                // Now, find the GedTreeItem related to this node
                ListOfGedTreeItems::iterator iter;
                for (iter=m_treeItems.begin(); iter!=m_treeItems.end(); ++iter)
                {
                    GedTreeItem* item=*iter;
                    if (item->GetRelatedXmlNode()==node)
                        return item;
                }
            }
        }
        node=node->GetNext();
    }

    return NULL;
}

bool DatasManager::CreateTree()
{
    if (m_datas==NULL) return false;

    if (!m_treeItems.IsEmpty())
    {
        m_treeItems.DeleteContents(true);
        m_treeItems.Clear();
    }
    wxXmlNode *node=m_datas->GetChildren();
    wxString sValue;
    GedTreeItem *gtItem=NULL;
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating tree : First iteration (registering \"GedTreeItem\" objects)\n"));
#endif // __WXDEBUG__
    if (!m_treeItems.IsEmpty())
    {
        m_treeItems.DeleteContents(true);
        m_treeItems.Clear();
    }
    while(node!=NULL)
    {
        sValue=node->GetAttribute(_T("Type"));
        if (sValue==_T("INDI"))
        {
            gtItem=new GedTreeItem(node);
            m_treeItems.Append(gtItem);
        }
        node=node->GetNext();
    }
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating tree : First iteration ended (%d items created)\n"), m_treeItems.GetCount());
    {
        ListOfGedTreeItems::iterator it;
        int iAlones=0;
        for (it=m_treeItems.begin(); it!=m_treeItems.end(); ++it)
        {
            GedTreeItem* item=*it;
            if (item->IsAlone()) iAlones++;
        }
        wxPrintf(_T("                Memory list contains %d alone items\n"), iAlones);
    }
    wxPrintf(_T("                Second iteration (registering familly items)\n"));
#endif // __WXDEBUG__

    node=m_datas->GetChildren();
    while(node!=NULL)
    {
        sValue=node->GetAttribute(_T("Type"));
        if (sValue==_T("FAM"))
        {
#ifdef __WXDEBUG__
    wxPrintf(_T("!"));
#endif // __WXDEBUG__
            wxXmlNode *subNode=node->GetChildren();
            GedTreeItem *Husb=NULL, *Wife=NULL;
            ListOfGedTreeItems children;
            children.DeleteContents(false);
            children.Clear();
            while(subNode!=NULL)
            {
                wxString sType=subNode->GetAttribute(_T("Type"));
                wxString sId=subNode->GetAttribute(_T("GedId"));
                if (subNode->GetName()==_T("FamIndi") && (!sId.IsEmpty()) && (!sType.IsEmpty()))
                {
                    GedTreeItem* dest=FindTreeItem(sId);
                    if (dest!=NULL)
                    {
#ifdef __WXDEBUG__
    wxPrintf(_T("+"));
#endif // __WXDEBUG__
                        if (sType==_T("HUSB"))
                        {
                            Husb=dest;
                        }
                        else
                        {
                            if (sType==_T("WIFE"))
                            {
                                Wife=dest;
                            }
                            else
                            {
                                if (sType==_T("CHIL"))
                                {
                                    children.Append(dest);
                                }
                            }
                        }
                    }
                    else
                    {
#ifdef __WXDEBUG__
    wxPrintf(_T("-"));
#endif // __WXDEBUG__
                    }
                }
                subNode=subNode->GetNext();
            }
            if ((Husb!=NULL) && (Wife!=NULL))
            {
                Husb->AddHusbOrWife(Wife);
                Wife->AddHusbOrWife(Husb);
            }
            ListOfGedTreeItems::iterator iter;
            for (iter=children.begin(); iter!=children.end(); ++iter)
            {
                GedTreeItem *child=*iter;
                if (Husb!=NULL)
                {
                    Husb->AddChild(child);
                    child->AddParent(Husb);
                }
                if (Wife!=NULL)
                {
                    Wife->AddChild(child);
                    child->AddParent(Wife);
                }
            }
        }
        node=node->GetNext();
    }
#ifdef __WXDEBUG__
    {
        ListOfGedTreeItems::iterator it;
        int iAlones=0;
        for (it=m_treeItems.begin(); it!=m_treeItems.end(); ++it)
        {
            GedTreeItem* item=*it;
            if (item->IsAlone()) iAlones++;
        }
        wxPrintf(_T("\nCreating tree : Second iteration ended (%d items alone remaining)\n"), iAlones);
    }
    wxPrintf(_T("Creating tree : Third iteration (searching for a base item)\n"));
#endif // __WXDEBUG__
    ListOfGedTreeItems::iterator iter;
    int iTmp;
    m_baseItem=NULL;
    m_iMaxLevels=0;
    for (iter=m_treeItems.begin(); iter!=m_treeItems.end(); ++iter)
    {
        GedTreeItem* item=*iter;
        if (m_baseItem==NULL)
        {
            m_baseItem=item;
            m_iMaxLevels=item->GetAscLevelsCount();
        }
        else
        {
            iTmp=item->GetAscLevelsCount();
            if (iTmp>m_iMaxLevels)
            {
                m_baseItem=item;
                m_iMaxLevels=iTmp;
            }
        }
    }
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating tree : Third iteration ended\n"));
    node=m_baseItem->GetRelatedXmlNode();
    sValue=node->GetAttribute(_T("GedId"));
    wxPrintf(_T("                Base item (%s) has %d levels up to him\n"), sValue, m_iMaxLevels);
    wxPrintf(_T("Creating tree : Fourth iteration (updating levels)\n"));
#endif // __WXDEBUG__
    int iIterCount=0;
    bool bModif=true;
    m_baseItem->SetLevel(0);
    while((bModif==true) && (iIterCount<50))
    {
        bModif=false;
        iIterCount++;
        for (iter=m_treeItems.begin(); iter!=m_treeItems.end(); ++iter)
        {
            GedTreeItem *item=*iter;
            bModif |= item->AutoUpdateLevels();
        }
    }
    for (iter=m_treeItems.begin(); iter!=m_treeItems.end(); ++iter)
    {
        GedTreeItem *item=*iter;
        node=item->GetRelatedXmlNode();
        node->AddAttribute(_T("Level"), wxString::Format(_T("%d"), item->GetLevel()));
    }
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating tree : Fourth iteration ended (%d sub-iterations)\n"), iIterCount);
    {
        ListOfGedTreeItems::iterator it;
        int iAlones=0;
        for (it=m_treeItems.begin(); it!=m_treeItems.end(); ++it)
        {
            GedTreeItem* item=*it;
            if (item->GetLevel()==wxID_ANY) iAlones++;
        }
        wxPrintf(_T("                %d items with unknown level remaining\n"), iAlones);
    }
#endif // __WXDEBUG__
    return true;
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
bool DatasManager::SaveTree2TxtFile(const wxString& filename, GedTreeItem *base, int wdth, MyPdfDoc* doc)
{
    if (base==NULL) return false;
    if (m_iMaxLevels<2) return false;
    GedTreeItem FakeItem;
    GedTreeItem *NullItem=&FakeItem;

    GedTreeItem *TreeBase=base;
    int iTreeLevels=m_iMaxLevels-base->GetLevel();
    //TreeBase=TreeBase->m_parent1; iTreeLevels--;
    //TreeBase=TreeBase->m_parent1; iTreeLevels--;

    ListOfGedTreeItems *lstLvl=new ListOfGedTreeItems[iTreeLevels+1];

    lstLvl[0].Append(TreeBase);

    for (int lvl=1; lvl<=iTreeLevels; lvl++)
    {
#ifdef __WXDEBUG__
        wxPrintf(_T("\tProcessing level %d : "), lvl);
#endif // __WXDEBUG__
        int iCount=0, iReal=0;
        ListOfGedTreeItems::iterator iter;
        for (iter=lstLvl[lvl-1].begin(); iter!=lstLvl[lvl-1].end(); ++iter)
        {
            iCount+=2;
            GedTreeItem *item=*iter;
            if (item==NullItem)
            {
                lstLvl[lvl].Append(NullItem);
                lstLvl[lvl].Append(NullItem);
            }
            else
            {
                if (item->m_parent1!=NULL)
                {
                    lstLvl[lvl].Append(item->m_parent1);
                    iReal++;
                }
                else
                    lstLvl[lvl].Append(NullItem);
                if (item->m_parent2!=NULL)
                {
                    lstLvl[lvl].Append(item->m_parent2);
                    iReal++;
                }
                else
                    lstLvl[lvl].Append(NullItem);
            }
        }
#ifdef __WXDEBUG__
        wxPrintf(_T("%d items (%d reals)\n"), iCount, iReal);
#endif // __WXDEBUG__
    }

    int iWdth=0;
    wxFileOutputStream f_out(filename);
    wxTextOutputStream t_out(f_out);

    wxString sFirstName, sLastName;

    if (wdth==-1)
    {
        for (int lvl=iTreeLevels; lvl>=0; lvl--)
        {
            ListOfGedTreeItems::iterator iter;
            for (iter=lstLvl[lvl].begin(); iter!=lstLvl[lvl].end(); ++iter)
            {
                GedTreeItem *item=*iter;
                if (item!=NullItem)
                {
                    getItemNames(item->GetRelatedXmlNode(), sFirstName, sLastName);
                    if (sLastName.Length()>(size_t)iWdth) iWdth=sLastName.Length();
                }
            }
        }
    }
    else
    {
        iWdth=wdth;
    }
    if ((iWdth%2)!=1) iWdth++;
    int iTotalWdth=(1<<iTreeLevels)*(iWdth+3)-1, iNbSpaces=0;

    for (int lvl=iTreeLevels; lvl>=0; lvl--)
    {
        int iNbCases=lstLvl[lvl].GetCount();
        iNbSpaces=(iTotalWdth - (iNbCases*(iWdth+2)))/iNbCases;
        wxString sLine1=wxEmptyString, sLine2=wxEmptyString, sLine3=wxEmptyString, sLine4=wxEmptyString, sLine5=wxEmptyString;
        ListOfGedTreeItems::iterator iter;
        for (iter=lstLvl[lvl].begin(); iter!=lstLvl[lvl].end(); ++iter)
        {
            if (iter==lstLvl[lvl].begin())
            {
                sLine1.Append(_T(' '), (iNbSpaces/2));
                sLine2.Append(_T(' '), (iNbSpaces/2));
                sLine3.Append(_T(' '), (iNbSpaces/2));
                sLine4.Append(_T(' '), (iNbSpaces/2));
                sLine5.Append(_T(' '), (iNbSpaces/2));
            }
            sLine1.Append(_T("╔"));
            sLine1.Append(_T('═'), iWdth/2);
            sLine1.Append((lvl==iTreeLevels?_T("═"):_T("╧")));
            sLine1.Append(_T('═'), iWdth/2);
            sLine1.Append(_T("╗"));

            sLine2.Append(_T("║"));
            sLine3.Append(_T("║"));
            sLine5.Append(_T("║"));
            sLine4.Append(_T("╚"));

            GedTreeItem *item=*iter;
            sLine5.Append(_T(' '), iWdth);
            if (item==NullItem)
            {
                sLine2.Append(_T(' '), iWdth);
                sLine3.Append(_T(' '), iWdth);
            }
            else
            {
                getItemNames(item->GetRelatedXmlNode(), sFirstName, sLastName);
                sFirstName.Append(_T(' '), iWdth); sFirstName.Truncate(iWdth);
                sLastName.Append(_T(' '), iWdth); sLastName.Truncate(iWdth);
                sLine2.Append(sLastName);
                sLine3.Append(sFirstName);
            }
            sLine4.Append(_T('═'), iWdth/2);
            sLine4.Append((lvl==0?_T("═"):_T("╤")));
            sLine4.Append(_T('═'), iWdth/2);

            sLine2.Append(_T("║"));
            sLine3.Append(_T("║"));
            sLine5.Append(_T("║"));
            sLine4.Append(_T("╝"));

            sLine1.Append(_T(' '), iNbSpaces+1);
            sLine2.Append(_T(' '), iNbSpaces+1);
            sLine3.Append(_T(' '), iNbSpaces+1);
            sLine4.Append(_T(' '), iNbSpaces+1);
            sLine5.Append(_T(' '), iNbSpaces+1);
        }
        sLine1.Trim();
        sLine2.Trim();
        sLine3.Trim();
        sLine4.Trim();
        t_out.WriteString(sLine1 + _T("\n"));
        t_out.WriteString(sLine5 + _T("\n"));
        t_out.WriteString(sLine2 + _T("\n"));
        t_out.WriteString(sLine3 + _T("\n"));
        t_out.WriteString(sLine5 + _T("\n"));
        t_out.WriteString(sLine4 + _T("\n"));
/*
        mdc.DrawText(sLine1, wxPoint(10, iY)); iY+=sz.GetHeight();
        mdc.DrawText(sLine2, wxPoint(10, iY)); iY+=sz.GetHeight();
        mdc.DrawText(sLine3, wxPoint(10, iY)); iY+=sz.GetHeight();
        mdc.DrawText(sLine4, wxPoint(10, iY)); iY+=sz.GetHeight();
*/
        iNbCases/=2;
        if (lvl>0)
        {
            sLine1=wxEmptyString;
            sLine1.Append(_T(' '), iWdth/2+1);
            sLine1.Append(_T(' '), (iNbSpaces/2));
            for (int i=0; i<iNbCases; i++)
            {
                sLine1.Append(_T("└"));
                sLine1.Append(_T('─'), (iNbSpaces/2 + iWdth/2)+1);
                sLine1.Append(_T("┬"));
                sLine1.Append(_T('─'), (iNbSpaces/2 + iWdth/2)+1);
                sLine1.Append(_T("┘"));
                if (i<(iNbCases-1))
                {
                    sLine1.Append(_T(' '), iNbSpaces+iWdth+2);
                }
            }
            t_out.WriteString(sLine1 + _T("\n"));
/*
            mdc.DrawText(sLine1, wxPoint(10, iY)); iY+=sz.GetHeight();
*/
        }
    }

    if (doc!=NULL)
    {
        doc->Generate(&lstLvl[0], iTreeLevels);
    }

    for (int i=0; i<=iTreeLevels; i++)
    {
        lstLvl[i].DeleteContents(false);
    }
    delete [] lstLvl;
/*
    mdc.SelectObject(wxNullBitmap);
#ifdef __WXDEBUG__
    wxPrintf(_T("Saving resulting image"));
#endif // __WXDEBUG__
    wxFileName fname(filename);
    fname.SetExt(_T("jpg"));
    bmp.SaveFile(fname.GetFullPath(), wxBITMAP_TYPE_JPEG);
#ifdef __WXDEBUG__
    wxPrintf(_T(" : Done\n"));
#endif // __WXDEBUG__
*/
    return true;
}

bool DatasManager::SaveTree2TxtFile(const wxString& filename)
{
    wxFileName fname(filename);
    wxString sName=fname.GetName();
    fname.SetExt(_T("txt"));
    GedTreeItem *base=m_baseItem;
    if (base==NULL) return false;
    base=base->m_parent1; if (base==NULL) return false;
    base=base->m_parent1; if (base==NULL) return false;
    base=base->m_parent1; if (base==NULL) return false;
    GedTreeItem *item[8];
    item[0]=base->m_parent1->m_parent1;
    item[1]=base->m_parent1->m_parent2;
    item[2]=base->m_parent2->m_parent1;
    item[3]=base->m_parent2->m_parent2;
    base=m_baseItem;
    base=base->m_parent1; if (base==NULL) return false;
    base=base->m_parent1; if (base==NULL) return false;
    base=base->m_parent2; if (base==NULL) return false;
    item[4]=base->m_parent1->m_parent1;
    item[5]=base->m_parent1->m_parent2;
    item[6]=base->m_parent2->m_parent1;
    item[7]=base->m_parent2->m_parent2;

    MyPdfDoc doc(1500, 910);

    for (int i=0; i<8; i++)
    {
        fname.SetName(sName + wxString::Format(_T("%d"), i+1));
        if (!SaveTree2TxtFile(fname.GetFullPath(), item[i], 13, &doc)) return false;
    }
    fname.SetName(sName);
    fname.SetExt(_T("pdf"));
    doc.SaveAsFile(fname.GetFullPath());

    MyPdfDoc doc2(297, 210);
    int iTmp=m_iMaxLevels; m_iMaxLevels=5;
    fname.SetName(sName + _T("0"));
    fname.SetExt(_T("txt"));
    SaveTree2TxtFile(fname.GetFullPath(), m_baseItem->m_parent1->m_parent1, 13, &doc2);
    m_iMaxLevels=iTmp;
    fname.SetExt(_T("pdf"));
    doc2.SaveAsFile(fname.GetFullPath());
    return true;

#ifdef __WXDEBUG__
    wxPrintf(_T("Creating destination bitmap"));
#endif // __WXDEBUG__
    //wxBitmap bmp(3508, 2479, 1); // A4 297x210
    //wxBitmap bmp(4963, 3508, 1); // A3 420x297
    //wxBitmap bmp(7017, 4963, 1); // A2 594x420
    //wxBitmap bmp(9933, 7017, 1); // A1 840x594
    //wxBitmap bmp(14042, 9933, 1); // A0 1188x840
    wxBitmap bmp(17716, 10629, 1); // A0 1500x900
    //wxBitmap bmp(23621, 10629, 1); // A0+ 2000x900 300 p/inch
    wxMemoryDC mdc(bmp);
    mdc.SetBrush(*wxWHITE_BRUSH);
    mdc.SetPen(*wxWHITE_PEN);
    mdc.DrawRectangle(wxPoint(0,0), mdc.GetSize());
    wxFont fnt(22, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    mdc.SetFont(fnt);
    mdc.SetPen(*wxBLACK_PEN);
    wxSize sz=mdc.GetTextExtent(_T("║"));
    int iY=10;
#ifdef __WXDEBUG__
    wxPrintf(_T(" : Done\n"));
#endif // __WXDEBUG__
    wxMBConvUTF8 conv;
    for (int i=0; i<4; i++)
    {
        fname.SetName(sName + wxString::Format(_T("%d"), i+1));
        wxFileInputStream f_in(fname.GetFullPath());
        wxTextInputStream t_in(f_in, _T(" \t"), conv);
        wxString sLine=t_in.ReadLine();
        while(!sLine.IsEmpty())
        {
            mdc.DrawText(sLine, wxPoint(10, iY)); iY+=sz.GetHeight();
            sLine=t_in.ReadLine();
        }
        iY += sz.GetHeight();
        mdc.DrawLine(wxPoint(10, iY), wxPoint(mdc.GetSize().GetWidth()-20, iY));
        iY+=sz.GetHeight()*2;
    }

    mdc.SelectObject(wxNullBitmap);
#ifdef __WXDEBUG__
    wxPrintf(_T("Saving resulting image"));
#endif // __WXDEBUG__
    fname.SetName(sName);
    fname.SetExt(_T("jpg"));
    bmp.SaveFile(fname.GetFullPath(), wxBITMAP_TYPE_JPEG);
#ifdef __WXDEBUG__
    wxPrintf(_T(" : Done\n"));
#endif // __WXDEBUG__
    return true;
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

wxString DatasManager::ParseDate(const wxString& value)
{
    static const wxChar* szSpec[]={_T("AFT"), _T("BEF"), _T("ABT"), _T("CAL"), _T("EST")};

    wxString sRes=wxEmptyString;

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

wxString DatasManager::GetItemBirth(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemBirth(node);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemBirth(const wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("BIRT")))
        {
            return ParseEvent(subNode);
        }
        subNode=subNode->GetNext();
    }

    return wxEmptyString;
}

wxString DatasManager::GetItemDeath(const wxString& itmId)
{
    wxXmlNode *node=m_datas->GetChildren();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("GedId"))==itmId)
        {
            return GetItemDeath(node);
        }
        node=node->GetNext();
    }
    return wxEmptyString;
}

wxString DatasManager::GetItemDeath(const wxXmlNode* itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxXmlNode *subNode=itmNode->GetChildren();
    while(subNode!=NULL)
    {
        if ((subNode->GetName()==_T("Event")) && (subNode->GetAttribute(_T("Type"))==_T("DEAT")))
        {
            return ParseEvent(subNode);
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
    if (!HasDatas()) return false;
    if (filename==m_sFileName) return false;

    wxFileInputStream f_in(filename);
    if (!f_in.IsOk()) return false;

    wxXmlNode cmpNode, *subNode;
    if (!ParseGedToXml(&f_in, &cmpNode)) return false;

    wxArrayString arsSrc, arsCmp;

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

    return true;
}

wxString DatasManager::GetCompResultsSummary()
{
    wxString sResult=_("Changes from file:");
    sResult << _T("\n") << m_sCmpFile;

    if (!m_arsCompAdded.IsEmpty())
    {
        int iCount=m_arsCompAdded.GetCount();
        sResult << _T("\n") << iCount << (iCount==1? _("item added:"):_("items added:"));
        for (int i=0; i<iCount; ++i)
        {
            sResult << _T("\n - ") << m_arsCompAdded[i];
        }
    }
    if (!m_arsCompRemoved.IsEmpty())
    {
        int iCount=m_arsCompRemoved.GetCount();
        sResult << _T("\n") << iCount << (iCount==1? _("item removed:"):_("items removed:"));
        for (int i=0; i<iCount; ++i)
        {
            sResult << _T("\n - ") << m_arsCompRemoved[i];
        }
    }
    if (!m_arsCompModified.IsEmpty())
    {
        int iCount=m_arsCompModified.GetCount();
        sResult << _T("\n") << iCount << (iCount==1? _("item modified:"):_("items modified:"));
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
