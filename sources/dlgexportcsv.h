#ifndef __DLGEXPORTCSV_H_INCLUDED__
#define __DLGEXPORTCSV_H_INCLUDED__

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/filename.h>

class wxXmlNode;
class DatasManager;

class DlgExportCsv : public wxDialog
{
    public:
        DlgExportCsv(wxWindow *parent);
        virtual ~DlgExportCsv();
    private:
        // Misc functions
        void CreateControls();
        void ConnectControls();
        wxString GedItem2CsvLine(wxXmlNode* itmNode);
        // Events handlers
        void OnBtnExportClicked(wxCommandEvent &event);
        // Controls vars
        wxRadioButton *m_optExportType[3];
        wxButton *m_btnExport, *m_btnCancel;
        // Misc vars
        wxFileName m_FName;
        DatasManager& m_datas;
};

#endif // __DLGEXPORTCSV_H_INCLUDED__
