#ifndef __DLGEXPORTPDF_H_INCLUDED__
#define __DLGEXPORTPDF_H_INCLUDED__

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/filename.h>

class wxXmlNode;
class DatasManager;
class wxPdfDocument;

class DlgExportPdf : public wxDialog
{
    public:
        DlgExportPdf(wxWindow *parent);
        virtual ~DlgExportPdf();
        void SetSelectedItem(wxXmlNode *itmNode);
    private:
        // Misc functions
        void CreateControls();
        void ConnectControls();
        void UpdateControlsStates();
        void Summary2Pdf(wxPdfDocument *doc, wxArrayString* pArsItems=NULL);
        void GedItem2Pdf(wxXmlNode* itmNode, wxPdfDocument *doc);
        void AddHrTitle(double Y, const wxString& title, wxPdfDocument *doc);
        void DoExportSelectedItem();
        void DoExportAllItems();
        void DoExportCompResults();
        // Events handlers
        void OnOptExportTypeClicked(wxCommandEvent &event);
        void OnBtnExportClicked(wxCommandEvent &event);
        // Controls vars
        wxRadioButton *m_optExportType[3];
        wxCheckBox *m_chkSummary, *m_chkSplitPdf;
        wxButton *m_btnExport, *m_btnCancel;
        wxGauge *m_pgbProgress;
        // Misc vars
        wxXmlNode *m_SelectedItem;
        wxFileName m_FName;
        DatasManager& m_datas;
};

#endif // __DLGEXPORTPDF_H_INCLUDED__
