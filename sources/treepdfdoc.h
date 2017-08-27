#ifndef TREEPDFDOC_H_INLUCDED
#define TREEPDFDOC_H_INLUCDED

#include <wx/wx.h>
#include <wx/pdfdoc.h>
#include "mytreeitem.h"

WX_DECLARE_STRING_HASH_MAP(int, PdfLinksMap);
#define MAX_ITER 20

class TreePdfDoc : public wxPdfDocument
{
    public:
        TreePdfDoc(MyTreeItem* root, double pageWidth=297, double pageHeight=210);
        virtual ~TreePdfDoc();
        bool CreateTree(int MaxLevels=-1);
        bool Generate(double pageWidth=-1, double pageHeight=-1);
        void DrawConnexions(bool withMarrDate=true) { DrawConnexions(m_rootItem, withMarrDate); }
        void DrawSosaNumbers() { DrawSosaNumber(m_rootItem); }
        void WriteSummary();
        void GetTreeSize(double *width, double *height);
        int GetLevelMax();
        void EnableLinks(bool enable=true) { m_bAddLinks=enable; }
        PdfLinksMap& GetLinks() { return m_hmLinks; }
    protected:
    private:
        void DrawItem(MyTreeItem* item);
        void DrawConnexions(MyTreeItem* item, bool withMarrDate=false);
        void DrawSosaNumber(MyTreeItem* item);
        void UpdateItemDatas(MyTreeItem *item);
        void UpdateItemXPos(MyTreeItem *item, double dec);
        double m_pgWidth, m_pgHeight;
        double m_dScale, m_dDecX, m_dDecY;
        MyTreeItem *m_rootItem;
        bool m_bAddLinks;
        PdfLinksMap m_hmLinks;
};

#endif // TREEPDFDOC_H_INLUCDED
