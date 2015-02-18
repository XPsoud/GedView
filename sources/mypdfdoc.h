#ifndef MYPDFDOC_H_INLUCDED
#define MYPDFDOC_H_INLUCDED

#include <wx/wx.h>
#include <wx/gdicmn.h>
#include "wx/pdfdoc.h"
#include "gedtreeitem.h"

class MyPdfDoc : public wxPdfDocument
{
    public:
        MyPdfDoc(double pageWidth=297, double pageHeight=210);
        virtual ~MyPdfDoc();
        void ExtractItemNames(wxXmlNode *container, wxString& first, wxString& last);
        bool Generate(ListOfGedTreeItems *items, int nbLevels);
    protected:
    private:
        double m_pgWidth, m_pgHeight, m_dY;
        bool m_bStarted;
};

#endif // MYPDFDOC_H_INLUCDED
