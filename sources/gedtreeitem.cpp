#include "gedtreeitem.h"

int GedTreeItem::m_iItemsCount = 0;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ListOfGedTreeItems);

#include <wx/xml/xml.h>

GedTreeItem::GedTreeItem(wxXmlNode* node)
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"GedTreeItem\" object\n"));
#endif // __WXDEBUG__
    m_iItemsCount++;
    m_Item=node;
    m_parent1=NULL;
    m_parent2=NULL;
    m_HusbWife1=NULL;
    m_HusbWife2=NULL;
    m_FirstChild=NULL;
    m_FirstBS=NULL;
    m_iLevel=wxID_ANY;
}

GedTreeItem::~GedTreeItem()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"GedTreeItem\" object\n"));
#endif // __WXDEBUG__
    m_iItemsCount--;
}

bool GedTreeItem::IsAlone()
{
    if (m_parent1!=NULL) return false;
    if (m_parent2!=NULL) return false;
    if (m_HusbWife1!=NULL) return false;
    if (m_HusbWife2!=NULL) return false;
    if (m_FirstChild!=NULL) return false;
    if (m_FirstBS!=NULL) return false;

    return true;
}

GedTreeItem *GedTreeItem::GetLastChild()
{
    if (m_FirstChild==NULL) return NULL;
    GedTreeItem* last=m_FirstChild;
    while(last->m_FirstBS!=NULL)
        last=last->m_FirstBS;

    return last;
}

bool GedTreeItem::AddHusbOrWife(GedTreeItem* item)
{
    if ((m_HusbWife1!=NULL) && (m_HusbWife2!=NULL))
    {
        if ((m_HusbWife1==item) || (m_HusbWife2==item))
            return true;
        else
            return false;
    }
    if (m_HusbWife1==NULL)
    {
        m_HusbWife1=item;
    }
    else
    {
        if (m_HusbWife1!=item)
            m_HusbWife2=item;
    }
#ifdef __WXDEBUG__
    wxPrintf(_T("*"));
#endif // __WXDEBUG__
    return true;
}

void GedTreeItem::AddChild(GedTreeItem* item)
{
    if ((m_FirstChild==NULL)||(m_FirstChild==item))
    {
        m_FirstChild=item;
        return;
    }
    GedTreeItem* last=m_FirstChild;
    while(last->m_FirstBS!=NULL)
    {
        last=last->m_FirstBS;
        if (last==item) return;
    }
    last->m_FirstBS=item;
#ifdef __WXDEBUG__
    wxPrintf(_T("*"));
#endif // __WXDEBUG__
}

bool GedTreeItem::AddParent(GedTreeItem* item)
{
    if ((m_parent1!=NULL) && (m_parent2!=NULL))
    {
        if ((m_parent1==item)||(m_parent2==item))
            return true;
        else
            return false;
    }
    if (m_parent1==NULL)
    {
        m_parent1=item;
    }
    else
    {
        if (m_parent1!=item)
            m_parent2=item;
    }
#ifdef __WXDEBUG__
    wxPrintf(_T("*"));
#endif // __WXDEBUG__
    return true;
}

int GedTreeItem::GetAscLevelsCount()
{
    int ip1=0, ip2=0;
    if (m_parent1!=NULL)
    {
        ip1=m_parent1->GetAscLevelsCount()+1;
    }
    if (m_parent2!=NULL)
    {
        ip2=m_parent2->GetAscLevelsCount()+1;
    }
    int iRes=(ip1>ip2?ip1:ip2);
    return iRes;
}

int GedTreeItem::GetDescLevelsCount()
{
    return 0;
}

bool GedTreeItem::AutoUpdateLevels()
{
    bool bRes=false;
    GedTreeItem *item;
    // We must know the item level to proceed
    // If it is not set, try to find it
    if (m_iLevel==wxID_ANY)
    {
        // First, try with the parents items
        if (m_parent1!=NULL)
        {
            if (m_parent1->GetLevel()!=wxID_ANY)
            {
                m_iLevel=m_parent1->GetLevel()-1;
                bRes=true;
            }
        }
        else
        {
            if (m_parent2!=NULL)
            {
                if (m_parent2->GetLevel()!=wxID_ANY)
                {
                    m_iLevel=m_parent2->GetLevel()-1;
                    bRes=true;
                }
            }
        }
        if (!bRes) // Not yet found ?
        {
            // Try with the brothers and sisters items
            item=m_FirstBS;
            while(item!=NULL)
            {
                if (item->GetLevel()!=wxID_ANY)
                {
                    m_iLevel=item->GetLevel();
                    bRes=true;
                    item=NULL;
                }
                else
                    item=item->m_FirstBS;
            }
        }
        if (!bRes) // Not yet found ?
        {
            // Try with the children items
            item=m_FirstChild;
            while(item!=NULL)
            {
                if (item->GetLevel()!=wxID_ANY)
                {
                    m_iLevel=item->GetLevel()+1;
                    bRes=true;
                    item=NULL;
                }
                else
                    item=item->m_FirstBS;
            }
        }
        if (!bRes) // Not yet found ?
        {
            // Try with the husbands or wives
            if (m_HusbWife1!=NULL)
            {
                if (m_HusbWife1->GetLevel()!=wxID_ANY)
                {
                    m_iLevel=m_HusbWife1->GetLevel();
                    bRes=true;
                }
            }
            else
            {
                if (m_HusbWife2!=NULL)
                {
                    if (m_HusbWife2->GetLevel()!=wxID_ANY)
                    {
                        m_iLevel=m_HusbWife2->GetLevel();
                        bRes=true;
                    }
                }
            }
        }
        // If the level can't be found, exit from here
        if (!bRes) return false;
    }
    // Update the brothers and sisters's levels if needed
    item=m_FirstBS;
    while(item!=NULL)
    {
        if (item->GetLevel()==wxID_ANY)
        {
            bRes=true;
            item->SetLevel(m_iLevel);
        }
        item=item->m_FirstBS;
    }

    return bRes;
}

wxString GedTreeItem::GetFirstName()
{
    wxString sResult=wxEmptyString;
    if (m_Item!=NULL)
    {
        //
    }
    return sResult;
}

wxString GedTreeItem::GetLastName()
{
    wxString sResult=wxEmptyString;
    if (m_Item!=NULL)
    {
        //
    }
    return sResult;
}

wxString GedTreeItem::GetDates()
{
    wxString sBirth=_T("....");
    wxString sDeath=_T("....");
    wxString sTmp, sValue;
    if (m_Item!=NULL)
    {
        wxXmlNode *node=m_Item->GetChildren();
        while(node!=NULL)
        {
            wxString sType=node->GetAttribute(_T("Type"));
            if ((sType==_T("BIRT"))||(sType==_T("DEAT")))
            {
                wxXmlNode *subNode=node->GetChildren();
                sTmp=_T("");
                while(subNode!=NULL)
                {
                    if (subNode->GetAttribute(_T("Type"))==_T("DATE"))
                    {
                        sValue=subNode->GetAttribute(_T("Value"));
                        if (sValue.Length()>3)
                        {
                            sTmp=sValue.Right(4);
                            if (sValue.StartsWith(_T("ABT")))
                            {
                                sTmp.Prepend(_T("~"));
                            }
                            if (sValue.StartsWith(_T("AFT")))
                            {
                                sTmp.Append(_T("+"));
                            }
                        }
                    }
                    subNode=subNode->GetNext();
                }
                if (!sTmp.IsEmpty())
                {
                    if (sType==_T("BIRT"))
                        sBirth=sTmp;
                    else
                        sDeath=sTmp;
                }
            }
            node=node->GetNext();
        }
    }
    wxString sResult=sBirth + _T(" => ") + sDeath;
    return sResult;
}
