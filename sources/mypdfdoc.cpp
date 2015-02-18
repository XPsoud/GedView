#include "mypdfdoc.h"

MyPdfDoc::MyPdfDoc(double pageWidth, double pageHeight) : wxPdfDocument(wxPORTRAIT, pageWidth, pageHeight, _T("mm"))
{
    m_pgWidth=pageWidth;
    m_pgHeight=pageHeight;
    m_dY=5.;
    m_bStarted=false;
}

MyPdfDoc::~MyPdfDoc()
{
    //dtor
}


void MyPdfDoc::ExtractItemNames(wxXmlNode *container, wxString& first, wxString& last)
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

bool MyPdfDoc::Generate(ListOfGedTreeItems *items, int nbLevels)
{
    int iMaxLevels=nbLevels;
    double dRatio=0.5, x;

    int iNbCases=1<<iMaxLevels;
    double dBaseSpc=5., dMargin=1.;
    double dWdth=(m_pgWidth-(iNbCases*dBaseSpc))/iNbCases;
    double dHght=dWdth*dRatio, dSpc=dBaseSpc;
    double cw=dWdth-(2*dMargin), ch=(dHght-(2*dMargin))/3, cx, cy;

    if (!m_bStarted)
    {
        // Création du document Pdf
        AliasNbPages();
        AddPage();
        SetFont(_T("Helvetica"), _T(""), 6);
        SetLineHeight(ch);
        m_bStarted=true;
        //Line(dBaseSpc, PgHght/2, PgWdth-dBaseSpc, PgHght/2);
    }
    else
    {
        Line(dBaseSpc, m_dY, m_pgWidth-dBaseSpc, m_dY);
        m_dY+=dBaseSpc*2;
    }

    wxString sFirstName, sLastName, sDates;
    GedTreeItem fakeItem;
    wxString sEmptyDates=fakeItem.GetDates();

    for (int lvl=iMaxLevels; lvl>=0; lvl--)
    {
        ListOfGedTreeItems::iterator iter=items[lvl].begin();
        iNbCases=1<<lvl;
        dSpc=(m_pgWidth-(iNbCases*dWdth))/iNbCases;
        x=dSpc/2.;
        for (int i=0; i<iNbCases; i++)
        {
            GedTreeItem *item=*iter;
            RoundedRect(x, m_dY, dWdth, dHght, dMargin);

            if (item->GetRelatedXmlNode()!=NULL)
            {
                ExtractItemNames(item->GetRelatedXmlNode(), sFirstName, sLastName);
                sDates=item->GetDates();
                cx=x+dMargin;
                cy=m_dY+dMargin;
                SetXY(cx, cy);
                ClippedCell(cw, ch, sLastName, wxPDF_BORDER_NONE, 0, wxPDF_ALIGN_CENTER);
                SetXY(cx, cy+ch);
                MultiCell(cw, ch, sFirstName, wxPDF_BORDER_NONE, wxPDF_ALIGN_CENTER, 0, 1);
                SetXY(cx, cy+ch+ch);
                ClippedCell(cw, ch, sDates, wxPDF_BORDER_NONE, 0, wxPDF_ALIGN_CENTER);
            }

            iter++;

            x+=dSpc+dWdth;
        }
        m_dY+=dHght+dBaseSpc;
        iNbCases/=2;
        if (lvl>0)
        {
            double x1=dSpc/2. + dWdth/2;
            double x2=x1+dSpc+dWdth;
            double x3=(x1+x2)/2;
            double y1=m_dY-dBaseSpc;
            double y2=y1+(dBaseSpc/2);
            for (int i=0; i<iNbCases; i++)
            {
                Line(x1, y1, x1, y2);
                Line(x1, y2, x2, y2);
                Line(x2, y2, x2, y1);
                Line(x3, y2, x3, m_dY);

                x1+=2*(dSpc+dWdth);
                x2+=2*(dSpc+dWdth);
                x3+=2*(dSpc+dWdth);
            }
        }

    }

    return true;
}
