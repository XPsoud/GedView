#ifndef __DLGTREEPDF_H_INCLUDED__
#define __DLGTREEPDF_H_INCLUDED__

#include <wx/wx.h>
#include <wx/dialog.h>

class wxXmlNode;
class MyTreeItem;
class TreePdfDoc;
class DatasManager;

class DlgTreePdf : public wxDialog
{
    public:
        DlgTreePdf(wxWindow *parent);
        virtual ~DlgTreePdf();
        void SetSelectedItem(wxXmlNode *itmNode);
    private:
        // Misc functions
        void CreateControls();
        void ConnectControls();
        void UpdateCmbLevels(int maxLvl=-1);
        void UpdateControlsStates();
        void GetPaperSize(int* width, int* height);
        // Events handlers
        void OnSaveAsClicked(wxCommandEvent &event);
        void OnOptSizeClicked(wxCommandEvent &event);
        void OnUpdateUI_BtnSaveAs(wxUpdateUIEvent &event);
        // Controls vars
        wxChoice *m_cmbLevels, *m_cmbFormat, *m_cmbHeight, *m_cmbWidth;
        wxCheckBox *m_chkMarrDate, *m_chkSosaNmbr, *m_chkSummary;
        wxRadioButton *m_optSize[3];
        wxButton *m_btnSave, *m_btnClose;
        // Misc vars
        wxXmlNode *m_SelectedItem;
        DatasManager& m_datas;
        MyTreeItem *m_RootItem;
        TreePdfDoc *m_TreePdf;
};

#endif // __DLGTREEPDF_H_INCLUDED__
