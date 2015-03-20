#ifndef __DLGEXPORTPDF_H_INCLUDED__
#define __DLGEXPORTPDF_H_INCLUDED__

#include "wx/wx.h"

#include <wx/dialog.h>

class DlgExportPdf : public wxDialog
{
    public:
        DlgExportPdf(wxWindow *parent);
        virtual ~DlgExportPdf();
    private:
        // Misc functions
        void CreateControls();
        void ConnectControls();
        // Events handlers
        // Controls vars
        // Misc vars
};

#endif // __DLGEXPORTPDF_H_INCLUDED__
