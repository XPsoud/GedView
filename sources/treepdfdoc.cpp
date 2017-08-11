#include "treepdfdoc.h"

#include "appversion.h"
#include <wx/pdffontmanager.h>

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

bool TreePdfDoc::CreateTree(int MaxLevels)
{
    if (m_rootItem==NULL) return false;

    m_rootItem->UpdateParents();

    int iMaxLvls=(MaxLevels==-1?MAX_ITER:MaxLevels);

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

    for (int i=0; i<=(iMaxLvls-1); ++i)
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
            if (i>=(iMaxLvls-1)) continue;
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

    wxFont fontArial(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    wxPdfFontManager* fntMngr = wxPdfFontManager::GetFontManager();
    fntMngr->RegisterFont(fontArial);
    SetFont(fontArial);
    SetMargins(10, 10, 10);
    SetAutoPageBreak(false, 10);

    UpdateItemDatas(m_rootItem);
    m_rootItem->SetRelativePos(0);
    m_rootItem->SetFinalPos(0);

    m_rootItem->SetSosa(1);

    return true;
}

int TreePdfDoc::GetLevelMax()
{
    if (m_rootItem==NULL) return 0;
    return m_rootItem->GetMaxLevel();
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

    double dRTree=(dXMax-dXMin)/(dYMax-dYMin);
    double dPageWidth, dPageHeight;

    if ((pageWidth==-1) && (pageHeight==-1))
    {
        dPageWidth=m_pgWidth;
        dPageHeight=m_pgHeight;
    }
    else
    {
        if (pageWidth==-1)
        {
            dPageWidth=20+(pageHeight-20)*dRTree;
        }
        else
        {
            dPageWidth=pageWidth;
        }
        if (pageHeight==-1)
        {
            dPageHeight=20+(pageWidth-20)/dRTree;
        }
        else
        {
            dPageHeight=pageHeight;
        }
    }

    double dScaleX=(dPageWidth-GetLeftMargin()-GetRightMargin())/(dXMax-dXMin);
    double dScaleY=(dPageHeight-GetTopMargin()-GetBreakMargin())/(dYMax-dYMin);

    if (dScaleX<dScaleY)
        m_dScale=dScaleX;
    else
        m_dScale=dScaleY;

    m_dDecX=GetLeftMargin();
    m_dDecY=dPageHeight-GetBreakMargin()+m_dScale*dYMin;
    //m_dDecY=dPageHeight-GetBreakMargin()-GetTopMargin()+dYMin-m_dScale*(m_rootItem->GetItemHeight()-2);

    // Creating the Pdf document
    AliasNbPages();
    AddPage(wxPORTRAIT, dPageWidth, dPageHeight);
/*
    SetDrawColour(*wxBLUE);
    Rect(GetLeftMargin(), GetBreakMargin(), GetPageWidth()-GetLeftMargin()-GetRightMargin(), GetPageHeight()-GetTopMargin()-GetBreakMargin());
    SetDrawColour(*wxRED);
    Rect(m_dDecX+m_dScale*dXMin, m_dDecY-m_dScale*dYMin, m_dScale*(dXMax-dXMin), -m_dScale*(dYMax-dYMin));
    SetDrawColour(*wxBLACK);
*/
/*
    SetFont(_T("Helvetica"), _T(""), 10);
    wxString sTxt=wxString::Format(_T("XMin=%0.2f XMax=%0.2f YMin=%0.2f YMax=%0.2f DecX=%0.2f DecY=%0.2f"), dXMin, dXMax, dYMin, dYMax, m_dDecX, m_dDecY);
    sTxt << _T(" Scale=") << m_dScale << _T(" ScaleX=") << dScaleX << _T(" ScaleY=") << dScaleY << _T(")");
    Cell(GetPageWidth()-GetLeftMargin()-GetRightMargin(), GetLineHeight()+GetTopMargin(), sTxt, wxPDF_BORDER_NONE, 1);
    sTxt.Printf(_T("RootItem : X=%0.2f, Y=%0.2f W=%0.2f H=%0.2f %d sub-items"), m_rootItem->GetXPos(), m_rootItem->GetYPos(), m_rootItem->GetItemWidth(), m_rootItem->GetItemHeight(), m_rootItem->GetSubItemsCount());
    Cell(GetPageWidth()-GetLeftMargin()-GetRightMargin(), GetLineHeight()+GetTopMargin(), sTxt, wxPDF_BORDER_NONE, 1);
*/
    DrawItem(m_rootItem);
    return true;
}

void TreePdfDoc::WriteSummary()
{
    SetLineHeight(6);
    wxString sLine=wxEmptyString;
    wxDateTime dtNow=wxDateTime::Now();
    wxString sNow=dtNow.FormatDate();

    sLine.Printf(_("Ascending tree of %s %s (%d levels)"), m_rootItem->GetItemFirstName(), m_rootItem->GetItemLastName(), m_rootItem->GetMaxLevel()+1);
    SetFont(_T("Helvetica"), _T(""), 12);
    SetXY(m_dDecX, m_dDecY-2*GetLineHeight());
    Cell(1, GetLineHeight(), sLine, 0, 1);

    sLine.Printf(_("Generated on %s with"), sNow);

    SetFont(_T("Helvetica"), _T(""), 12);
    double dx=GetStringWidth(sLine + _T(" "));
    SetXY(m_dDecX, m_dDecY-GetLineHeight());
    Cell(dx, GetLineHeight(), sLine, 0, 0);
    SetFont(_T("Helvetica"), _T("B"), 12);
    sLine=wxTheApp->GetAppName();
    sLine << _T("-v") << VERSION_MAJOR << _T(".") << VERSION_MINOR << _T(".") << VERSION_REV;
    dx=GetStringWidth(sLine + _T(" "));
    Cell(dx, GetLineHeight(), sLine, 0, 0);
    SetFont(_T("Helvetica"), _T("I"), 12);
    Cell(dx, GetLineHeight(), _T(SMALLCOPYRIGHT), 0, 1);

    if (m_rootItem==NULL) return;

    SetFont(_T("Helvetica"), _T(""), 12);
    sLine.Printf(_("Number of elements of the tree: %d"), int(m_rootItem->GetSubItemsCount()+1));
    Cell(1, GetLineHeight(), sLine, 0, 1);
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
    double dY0=m_dDecY-(item->GetYPos()*m_dScale)-(dh/2.);
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
    Cell(dw, delta, item->GetItemDates(), 0, 0, wxPDF_ALIGN_CENTER);
    RoundedRect(dX0, dY0, dw, dh, dR);
}

void TreePdfDoc::DrawConnexions(MyTreeItem* item, bool withMarrDate)
{
    if (item==NULL) return;

    if (item->GetFather()!=NULL)
    {
        DrawConnexions(item->GetFather(), withMarrDate);
    }
    if (item->GetMother()!=NULL)
    {
        DrawConnexions(item->GetMother(), withMarrDate);
    }
    if (item->GetChild()==NULL) return;

    double dh=m_dScale*item->GetItemHeight();
    double dX0=m_dDecX+(item->GetXPos()*m_dScale);
    double dY0=m_dDecY-(item->GetYPos()*m_dScale)+(dh/2.);
    double dX1=m_dDecX+(item->GetChild()->GetXPos()*m_dScale);
    double dY1=dY0+(V_GAP*m_dScale);
    double dY2=(dY0+dY1)/2.;
    double dR=1.5*m_dScale;

    Line(dX0, dY0, dX0, dY2-dR);
    if (dX0<dX1)
    {
        // Father connexion ligne
        Circle(dX0+dR, dY2-dR, dR, 180, 270);
        Line(dX0+dR, dY2, dX1-dR, dY2);
        Circle(dX1-dR, dY2+dR, dR, 0, 90);
        Line(dX1, dY2+dR, dX1, dY1);
        if (withMarrDate)
        {
            SetFont(_T("Courier"), _T(""), 4*m_dScale);
            double dw=GetStringWidth(_T(" ---- "));
            SetXY(dX1-0.5*dw, dY2-2*m_dScale);
            Cell(dw, 2*m_dScale, item->GetItemMarriage(), 0, 0, wxPDF_ALIGN_CENTER);
        }
    }
    else
    {
        // Mother connexion line
        Circle(dX0-dR, dY2-dR, dR, 270, 360);
        Line(dX0-dR, dY2, dX1+dR, dY2);
        Circle(dX1+dR, dY2+dR, dR, 90, 180);
        // If there were no father, draw the final line and the marriage date if wanted
        if (item->GetChild()->GetFather()==NULL)
        {
            Line(dX1, dY2+dR, dX1, dY1);
            if (withMarrDate)
            {
                SetFont(_T("Courier"), _T("B"), 4*m_dScale);
                double dw=GetStringWidth(_T(" ---- "));
                SetXY(dX1-0.5*dw, dY2-2*m_dScale);
                Cell(dw, 2*m_dScale, item->GetItemMarriage(), 0, 0, wxPDF_ALIGN_CENTER);
            }
        }
    }
}

void TreePdfDoc::DrawSosaNumber(MyTreeItem* item)
{
    if (item==NULL) return;

    if (item->GetFather()!=NULL)
    {
        DrawSosaNumber(item->GetFather());
    }
    if (item->GetMother()!=NULL)
    {
        DrawSosaNumber(item->GetMother());
    }
    if (item->GetSosa()<1) return;

    double dw=m_dScale*item->GetItemWidth();
    double dh=m_dScale*item->GetItemHeight();
    double dX0=m_dDecX+(item->GetXPos()*m_dScale);
    double dY0=m_dDecY-(item->GetYPos()*m_dScale)+(dh/2.);

    SetFont(_T("Courier"), _T("B"), 3*m_dScale);
    if (item->GetChild()==NULL)
    {
        SetXY(dX0-dw/2, dY0);
        Cell(dw, 2*m_dScale, wxString::Format(_T("(%d)"), item->GetSosa()), wxPDF_BORDER_NONE, 0, wxPDF_ALIGN_CENTER);
        return;
    }
    if (item==item->GetChild()->GetFather())
    {
        // Father's sosa on the left
        SetXY(dX0-dw/2, dY0);
        Cell(dw/2, 2*m_dScale, wxString::Format(_T("(%d)"), item->GetSosa()), wxPDF_BORDER_NONE, 0, wxPDF_ALIGN_LEFT);
    }
    else
    {
        // Mother's sosa on the right
        SetXY(dX0, dY0);
        Cell(dw/2, 2*m_dScale, wxString::Format(_T("(%d)"), item->GetSosa()), wxPDF_BORDER_NONE, 0, wxPDF_ALIGN_RIGHT);
    }
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
