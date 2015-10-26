#ifndef __DLGTREEPDF_H_INCLUDED__
#define __DLGTREEPDF_H_INCLUDED__

#include <wx/wx.h>
#include <wx/dialog.h>

class wxXmlNode;
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
        // Events handlers
        // Controls vars
        wxChoice *m_cmbLevels, *m_cmbFormat;
        wxRadioButton *m_optSize[3];
        // Misc vars
        wxXmlNode *m_SelectedItem;
        DatasManager& m_datas;
};

#endif // __DLGTREEPDF_H_INCLUDED__
