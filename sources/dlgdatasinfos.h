#ifndef __DLGDATASINFOS_H_INCLUDED__
#define __DLGDATASINFOS_H_INCLUDED__

#include <wx/wx.h>

#include <wx/dialog.h>

class DatasManager;

class DlgDatasInfos : public wxDialog
{
	public:
		DlgDatasInfos(wxWindow *parent);
		virtual ~DlgDatasInfos();
	private:
		// Misc functions
		void CreateControls();
		void ConnectControls();
		// Events handlers
		// Controls vars
		// Misc vars
		DatasManager& m_datas;
};

#endif // __DLGDATASINFOS_H_INCLUDED__
