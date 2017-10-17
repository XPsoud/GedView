#include "dlgcheckdatas.h"

#include "datasmanager.h"

DlgCheckDatas::DlgCheckDatas(wxWindow *parent) : wxDialog(parent, -1, _("Check loaded datas")),
	m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Creating a \"DlgCheckDatas\" object\n"));
#endif // __WXDEBUG__

	CreateControls();

	ConnectControls();

	CenterOnParent();
}

DlgCheckDatas::~DlgCheckDatas()
{
#ifdef __WXDEBUG__
	wxPrintf(_T("Destroying a \"DlgCheckDatas\" object\n"));
#endif // __WXDEBUG__
}

void DlgCheckDatas::CreateControls()
{
}

void DlgCheckDatas::ConnectControls()
{
}
