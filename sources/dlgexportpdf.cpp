#include "dlgexportpdf.h"

#include "main.h"
#include "appversion.h"

#include <wx/bitmap.h>
#include <wx/statbmp.h>
#include <wx/statline.h>

#include "xpsi.png.h"

DlgExportPdf::DlgExportPdf(wxWindow *parent) : wxDialog(parent, -1, _("Export as pdf file"), wxDefaultPosition, wxDefaultSize)
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DlgExportPdf\" object\n"));
#endif // __WXDEBUG__

	CreateControls();

	ConnectControls();

	CenterOnParent();
}

DlgExportPdf::~DlgExportPdf()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"DlgExportPdf\" object\n"));
#endif // __WXDEBUG__
}

void DlgExportPdf::CreateControls()
{
    //
}

void DlgExportPdf::ConnectControls()
{
    //
}
