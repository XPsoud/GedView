#include "dlgdatasinfos.h"

#include <wx/xml/xml.h>
#include <wx/statline.h>

#include "geddate.h"
#include "datasmanager.h"

DlgDatasInfos::DlgDatasInfos(wxWindow *parent) : wxDialog(parent, -1, _("Datas informations")),
	m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Creating a \"DlgDatasInfos\" object\n"));
#endif // __WXDEBUG__

	CreateControls();

	ConnectControls();

	CenterOnParent();
}

DlgDatasInfos::~DlgDatasInfos()
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Destroying a \"DlgDatasInfos\" object\n"));
#endif // __WXDEBUG__
}

void DlgDatasInfos::CreateControls()
{
	//
}

void DlgDatasInfos::ConnectControls()
{
	//
}
