#ifndef __DLGEXPORTPDF_H_INCLUDED__
#define __DLGEXPORTPDF_H_INCLUDED__

#include "wx/wx.h"

#include <wx/dialog.h>

class wxXmlNode;

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
        // Events handlers
        void OnOptExportTypeClicked(wxCommandEvent &event);
        void OnBtnExportClicked(wxCommandEvent &event);
        // Controls vars
        wxRadioButton *m_optExportType[2];
        wxCheckBox *m_chkSummary;
        wxButton *m_btnExport, *m_btnCancel;
        // Misc vars
        wxXmlNode *m_SelectedItem;
};

#endif // __DLGEXPORTPDF_H_INCLUDED__
