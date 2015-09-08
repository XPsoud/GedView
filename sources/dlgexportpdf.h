#ifndef __DLGEXPORTPDF_H_INCLUDED__
#define __DLGEXPORTPDF_H_INCLUDED__

#include <wx/wx.h>

#include <wx/dialog.h>

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
        void Summary2Pdf(wxPdfDocument *doc);
        void GedItem2Pdf(wxXmlNode* itmNode, wxPdfDocument *doc);
        void AddHrTitle(double Y, const wxString& title, wxPdfDocument *doc);
        // Events handlers
        void OnOptExportTypeClicked(wxCommandEvent &event);
        void OnBtnExportClicked(wxCommandEvent &event);
        // Controls vars
        wxRadioButton *m_optExportType[2];
        wxCheckBox *m_chkSummary, *m_chkSplitPdf;
        wxButton *m_btnExport, *m_btnCancel;
        wxGauge *m_pgbProgress;
        // Misc vars
        wxXmlNode *m_SelectedItem;
        DatasManager& m_datas;
};

#endif // __DLGEXPORTPDF_H_INCLUDED__
