#include "treepdfdoc.h"

TreePdfDoc::TreePdfDoc(MyTreeItem* root, double pageWidth, double pageHeight) : wxPdfDocument(wxPORTRAIT, pageWidth, pageHeight, _T("mm"))
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"TreePdfDoc\" object\n"));
#endif // __WXDEBUG__
    m_pgWidth=pageWidth;
    m_pgHeight=pageHeight;
    m_rootItem=root;
    m_dScale=1.;
    m_dDecX=10.; m_dDecY=10.;
}

TreePdfDoc::~TreePdfDoc()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"TreePdfDoc\" object\n"));
#endif // __WXDEBUG__
}

bool TreePdfDoc::CreateTree()
{
    if (m_rootItem==NULL) return false;

    m_rootItem->UpdateParents();

    ListOfTreeItems items[MAX_ITER];

    if (m_rootItem->GetFatherNode()!=NULL)
    {
        MyTreeItem *item=new MyTreeItem(1, m_rootItem->GetFatherNode());
        m_rootItem->SetFather(item);
        items[0].Append(item);
    }
    if (m_rootItem->GetMotherNode()!=NULL)
    {
        MyTreeItem *item=new MyTreeItem(1, m_rootItem->GetMotherNode());
        m_rootItem->SetMother(item);
        items[0].Append(item);
    }
    if (m_rootItem->GetFather()==NULL) return false;
    if (m_rootItem->GetMother()==NULL) return false;

    for (int i=0; i<=(MAX_ITER-1); ++i)
    {
        int iFound=0;
        ListOfTreeItems::iterator iter;
        MyTreeItem *PrevItem=NULL;
        for (iter = items[i].begin(); iter != items[i].end(); ++iter)
        {
            MyTreeItem *item=*iter;
            // Update Left item
            item->SetLeftItem(PrevItem);
            PrevItem=item;
            if (i>=(MAX_ITER-1)) continue;
            item->UpdateParents();
            if (item->GetFatherNode()!=NULL)
            {
                iFound++;
                MyTreeItem *fitem=new MyTreeItem(i+2, item->GetFatherNode());
                item->SetFather(fitem);
                items[i+1].Append(fitem);
            }
            if (item->GetMotherNode()!=NULL)
            {
                iFound++;
                MyTreeItem *mitem=new MyTreeItem(i+2, item->GetMotherNode());
                item->SetMother(mitem);
                items[i+1].Append(mitem);
            }
        }
        if (iFound==0) break;
    }

    SetFont(_T("Helvetica"), _T("B"), 6);
    SetMargins(10, 10, 10);
    SetAutoPageBreak(false, 10);

    UpdateItemDatas(m_rootItem);
    m_rootItem->SetRelativePos(0);
    m_rootItem->SetFinalPos(0);

    return true;
}

bool TreePdfDoc::Generate(double pageWidth, double pageHeight)
{
    if (m_rootItem==NULL) return false;
    if (m_rootItem->GetFather()==NULL) return false;
    if (m_rootItem->GetMother()==NULL) return false;

    double dXMin=m_rootItem->GetXMin(), dXMax=m_rootItem->GetXMax(), dYMin=m_rootItem->GetYMin(), dYMax=m_rootItem->GetYMax();
    dXMin-=0.5*H_GAP;
    dXMax+=0.5*H_GAP;
    dYMax+=0.5*V_GAP;
    dYMin-=0.5*V_GAP;
    UpdateItemXPos(m_rootItem, -1.*dXMin);
    dXMax-=dXMin; dXMin=0;

    double dPageWidth=(pageWidth==-1?m_pgWidth:pageWidth);
    double dPageHeight=(pageHeight==-1?m_pgHeight:pageHeight);

    double dScaleX=(dPageWidth-GetLeftMargin()-GetRightMargin())/(dXMax-dXMin);
    double dScaleY=(dPageHeight-GetTopMargin()-GetBreakMargin())/(dYMax-dYMin);

    if (dScaleX<dScaleY)
        m_dScale=dScaleX;
    else
        m_dScale=dScaleY;

    m_dDecX=GetLeftMargin();
    m_dDecY=dPageHeight-GetBreakMargin()+dYMin-m_dScale*m_rootItem->GetItemHeight();

    // Creating the Pdf document
    AliasNbPages();
    AddPage(wxPORTRAIT, dPageWidth, dPageHeight);
/*
    Rect(GetLeftMargin(), GetBreakMargin(), GetPageWidth()-GetLeftMargin()-GetRightMargin(), GetPageHeight()-GetTopMargin()-GetBreakMargin());
    SetFont(_T("Helvetica"), _T(""), 10);
    wxString sBB=wxString::Format(_T("XMin=%0.2f XMax=%0.2f YMin=%0.2f YMax=%0.2f DecX=%0.2f DecY=%0.2f"), dXMin, dXMax, dYMin, dYMax, m_dDecX, m_dDecY);
    sBB << _T(" Scale=") << m_dScale << _T(" ScaleX=") << dScaleX << _T(" ScaleY=") << dScaleY << _T(")");
    Cell(GetPageWidth()-GetLeftMargin()-GetRightMargin(), GetLineHeight()+GetTopMargin(), sBB, wxPDF_BORDER_NONE, 1);
*/
    DrawItem(m_rootItem);

    return true;
}

void TreePdfDoc::DrawItem(MyTreeItem* item)
{
    if (item==NULL) return;

    if (item->GetFather()!=NULL)
    {
        DrawItem(item->GetFather());
    }
    if (item->GetMother()!=NULL)
    {
        DrawItem(item->GetMother());
    }

    double dw=m_dScale*item->GetItemWidth();
    double dh=m_dScale*item->GetItemHeight();
    double dX0=m_dDecX+(item->GetXPos()*m_dScale)-(dw/2.);
    double dY0=m_dDecY-(item->GetYPos()*m_dScale)+(dh/2.);
    double dR=(dw<dh?dw/6:dh/6);
    double delta=dh/4;

    SetLineWidth(0.2*m_dScale);

    SetFont(_T("Helvetica"), _T("B"), 6*m_dScale);
    SetXY(dX0, dY0);
    Cell(dw, delta, item->GetItemLastName(), 0, 0, wxPDF_ALIGN_CENTER);
    SetXY(dX0, dY0+delta);
    Cell(dw, delta, item->GetItemFirstName(), 0, 0, wxPDF_ALIGN_CENTER);
    SetXY(dX0, dY0+2*delta);
    SetFont(_T("Courier"), _T("I"), 5*m_dScale);
    Cell(dw, delta, item->GetItemId(), 0, 0, wxPDF_ALIGN_CENTER);
    SetXY(dX0, dY0+3*delta);
    SetFont(_T("Courier"), _T(""), 4*m_dScale);
    Cell(dw, delta, _T("---- => ----"), 0, 0, wxPDF_ALIGN_CENTER);
    //Cell(dw, delta, wxString::Format(_T("%0.2f x %0.2f"), item->GetXPos(), item->GetItemWidth()), 0, 0, wxPDF_ALIGN_CENTER);
    RoundedRect(dX0, dY0, dw, dh, dR);

    if (item->GetChild()==NULL) return;
    dX0+=(dw/2.);
    dY0+=dh;
    double dX1=m_dDecX+(item->GetChild()->GetXPos()*m_dScale);
    double dY1=m_dDecY-(item->GetChild()->GetYPos()*m_dScale)+(item->GetChild()->GetItemHeight()*m_dScale/2);
    double dY2=(dY0+dY1)/2.;
    Line(dX0, dY0, dX0, dY2);
    Line(dX0, dY2, dX1, dY2);
    Line(dX1, dY2, dX1, dY1);
    dw=GetStringWidth(_T(" ---- "));
    SetXY(dX1-0.5*dw, dY2-2*m_dScale);
    Cell(dw, 2*m_dScale, _T("----"), 0, 0, wxPDF_ALIGN_CENTER);

}

void TreePdfDoc::UpdateItemDatas(MyTreeItem* item)
{
    if (item==NULL) return;
    item->UpdateItemText();
    double dW1=GetStringWidth(item->GetItemLastName()+_T("OO"));
    double dW2=GetStringWidth(item->GetItemFirstName()+_T("OO"));
    double dW3=GetStringWidth(item->GetItemId()+_T("OO"));
    double dW=(dW1>dW2?dW1:dW2);
    if (dW3>dW) dW=dW3;
    item->SetItemWidth(dW);
    if (item->GetFather()!=NULL)
        UpdateItemDatas(item->GetFather());
    if (item->GetMother()!=NULL)
        UpdateItemDatas(item->GetMother());
}

void TreePdfDoc::UpdateItemXPos(MyTreeItem *item, double dec)
{
    if (item==NULL) return;
    item->SetXPos(item->GetXPos()+dec);
    if (item->GetFather()!=NULL)
        UpdateItemXPos(item->GetFather(), dec);
    if (item->GetMother()!=NULL)
        UpdateItemXPos(item->GetMother(), dec);
}

void TreePdfDoc::GetTreeSize(double *width, double *height)
{
    if (m_rootItem==NULL)
    {
        if (width!=NULL)
            *width=0.;
        if (height!=NULL)
            *height=0.;
        return;
    }

    double dXMin=m_rootItem->GetXMin(), dXMax=m_rootItem->GetXMax(), dYMin=m_rootItem->GetYMin(), dYMax=m_rootItem->GetYMax();
    dXMin-=0.5*H_GAP;
    dXMax+=0.5*H_GAP;
    dYMax+=0.5*V_GAP;
    dYMin-=0.5*V_GAP;

    dXMax-=dXMin;
    dYMax-=dYMin;
    if (width!=NULL)
        *width=dXMax;
    if (height!=NULL)
        *height=dYMax;
}
