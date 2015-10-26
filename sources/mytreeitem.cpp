#include "mytreeitem.h"

#include "datasmanager.h"

#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ListOfTreeItems);

MyTreeItem::MyTreeItem(int level, wxXmlNode* item)
{
#ifdef __WXDEBUG__
    //wxPrintf(_T("Creating a \"MyTreeItem\" object\n"));
#endif // DEBUG
    m_iLevel=level;
    m_NodeItem=item;
    m_Father=NULL;
    m_NodeFather=NULL;
    m_Mother=NULL;
    m_NodeMother=NULL;
    m_Child=NULL;
    m_dXPos=0.;
    m_dYPos=0.;
    m_dDec=0.;
    m_LeftItem=NULL;
    m_dWidth=MIN_WIDTH;
    m_dHeight=MIN_HEIGHT;
    m_sItemId.Clear();
    m_sItemFName.Clear();
    m_sItemLName.Clear();
}

MyTreeItem::~MyTreeItem()
{
    if (m_Father!=NULL) delete m_Father;
    if (m_Mother!=NULL) delete m_Mother;
#ifdef __WXDEBUG__
    //wxPrintf(_T("Destroying a \"MyTreeItem\" object\n"));
#endif // DEBUG
}

void MyTreeItem::SetLevel(int level)
{
    m_iLevel=level;
}

void MyTreeItem::SetNodeItem(wxXmlNode* item)
{
    m_NodeItem=item;
}

wxString MyTreeItem::GetItemId()
{
    if (m_sItemId.IsEmpty())
    {
        if (m_NodeItem==NULL)
            m_sItemId=_T("@###@");
        else
            m_sItemId=m_NodeItem->GetAttribute(_T("GedId"), _T("@???@"));
    }
    return m_sItemId;
}

void MyTreeItem::SetFather(MyTreeItem* father)
{
    m_Father=father;
    if (m_Father!=NULL)
        m_Father->SetChild(this);
}

void MyTreeItem::SetMother(MyTreeItem* mother)
{
    m_Mother=mother;
    if (m_Mother!=NULL)
        m_Mother->SetChild(this);
}

void MyTreeItem::SetChild(MyTreeItem* child)
{
    m_Child=child;
}

void MyTreeItem::UpdateParents()
{
    if (m_NodeItem==NULL) return;
    wxXmlNode* famItem=NULL, *node=m_NodeItem->GetChildren();
    DatasManager& datas=DatasManager::Get();
    // Search for the family item from witch this individual is issued
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("Type"))==_T("FAMC"))
        {
            famItem=datas.FindItemByGedId(node->GetAttribute(_T("Value")));
            if (famItem==NULL) return;
        }
        node=node->GetNext();
    }
    if (famItem==NULL) return;
    // Extract the "HUSB" and "WIFE" items from the found FAM item
    node=famItem->GetChildren();
    wxString sType, sId;
    while(node!=NULL)
    {
        if (node->GetName()==_T("FamIndi"))
        {
            sType=node->GetAttribute(_T("Type"));
            sId=node->GetAttribute(_T("GedId"));
            if (sType==_T("HUSB"))
            {
                m_NodeFather=datas.FindItemByGedId(sId);
            }
            else
            {
                if (sType==_T("WIFE"))
                {
                    m_NodeMother=datas.FindItemByGedId(sId);
                }
            }
        }
        node=node->GetNext();
    }
}

void MyTreeItem::SetXPos(double pos)
{
    m_dXPos=pos;
}

void MyTreeItem::SetYPos(double pos)
{
    m_dYPos=pos;
}

void MyTreeItem::SetItemDec(double value)
{
    m_dDec=value;
}

void MyTreeItem::SetLeftItem(MyTreeItem* item)
{
    m_LeftItem=item;
}

double MyTreeItem::GetXMin()
{
    double x=m_dXPos-(m_dWidth/2.);
    if (m_Father!=NULL)
    {
        double xx=m_Father->GetXMin();
        if (xx<x) x=xx;
    }
    if (m_Mother!=NULL)
    {
        double xx=m_Mother->GetXMin();
        if (xx<x) x=xx;
    }
    return x;
}

double MyTreeItem::GetXMax()
{
    double x=m_dXPos+(m_dWidth/2.);
    if (m_Father!=NULL)
    {
        double xx=m_Father->GetXMax();
        if (xx>x) x=xx;
    }
    if (m_Mother!=NULL)
    {
        double xx=m_Mother->GetXMax();
        if (xx>x) x=xx;
    }
    return x;
}

double MyTreeItem::GetYMin()
{
    double y=m_dYPos-m_dHeight/2.;
    if (m_Father!=NULL)
    {
        double yy=m_Father->GetYMin();
        if (yy<y) y=yy;
    }
    if (m_Mother!=NULL)
    {
        double yy=m_Mother->GetYMin();
        if (yy<y) y=yy;
    }
    return y;
}

double MyTreeItem::GetYMax()
{
    double y=m_dYPos+m_dHeight/2.;
    if (m_Father!=NULL)
    {
        double yy=m_Father->GetYMax();
        if (yy>y) y=yy;
    }
    if (m_Mother!=NULL)
    {
        double yy=m_Mother->GetYMax();
        if (yy>y) y=yy;
    }
    return y;
}

void MyTreeItem::SetRelativePos(double yPos)
{
    double dXF=1., dXM=1.;
    m_dYPos=yPos+(m_dHeight/2);
    bool bFinal=true;
    if (m_Father!=NULL)
    {
        bFinal=false;
        m_Father->SetRelativePos(m_dYPos+(m_dHeight/2)+V_GAP);
        dXF+=m_Father->GetItemDec();
    }
    if (m_Mother!=NULL)
    {
        bFinal=false;
        m_Mother->SetRelativePos(m_dYPos+(m_dHeight/2)+V_GAP);
        dXM+=m_Mother->GetItemDec();
    }
    if (!bFinal)
    {
        m_dDec=(dXF+dXM)/2.;
    }
}

void MyTreeItem::SetFinalPos(double xPos)
{
    m_dXPos=xPos;
    if (m_LeftItem!=NULL)
    {
        if((m_dXPos-m_dWidth/2)<=(m_LeftItem->GetXPos()+m_LeftItem->GetItemWidth()/2+H_GAP))
            m_dXPos=m_LeftItem->GetXPos()+m_LeftItem->GetItemWidth()/2+H_GAP+m_dWidth/2;
    }
    if (m_Father!=NULL)
        m_Father->SetFinalPos(m_dXPos-m_dDec);
    if (m_Mother!=NULL)
        m_Mother->SetFinalPos(m_dXPos+m_dDec);

    if ((m_Father!=NULL) && (m_Mother!=NULL))
        m_dXPos=(m_Father->GetXPos()+m_Mother->GetXPos())/2.;
}

void MyTreeItem::SetItemWidth(double width)
{
    if (width>MIN_WIDTH)
        m_dWidth=width;
}

void MyTreeItem::SetItemHeight(double height)
{
    if (height>MIN_HEIGHT)
        m_dHeight=height;
}

void MyTreeItem::UpdateItemText()
{
    if (m_NodeItem==NULL)
    {
        m_sItemFName=_T("Unknown");
        m_sItemLName.Clear();
    }
    else
    {
        m_sItemLName=DatasManager::Get().GetItemLastName(m_NodeItem);
        m_sItemFName=DatasManager::Get().GetItemFirstName(m_NodeItem);
    }
}

const wxString& MyTreeItem::GetItemFirstName()
{
    if (m_sItemFName.IsEmpty()) UpdateItemText();
    return m_sItemFName;
}

const wxString& MyTreeItem::GetItemLastName()
{
    if (m_sItemLName.IsEmpty()) UpdateItemText();
    return m_sItemLName;
}

const wxString MyTreeItem::GetItemText()
{
    return m_sItemLName + _T("\n") + m_sItemFName + _T("\n") + m_sItemId;
}

int MyTreeItem::GetMaxLevel()
{
    int iRes=m_iLevel, iFLvl=0, iMLvl=0;
    if (m_Father!=NULL)
        iFLvl=m_Father->GetMaxLevel();
    if (m_Mother!=NULL)
        iMLvl=m_Mother->GetMaxLevel();
    if (iFLvl>iRes) iRes=iFLvl;
    if (iMLvl>iRes) iRes=iMLvl;

    return iRes;
}
