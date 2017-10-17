#ifndef __DLGCHECKDATAS_H_INCLUDED__
#define __DLGCHECKDATAS_H_INCLUDED__

#include <wx/wx.h>

#include <wx/dialog.h>

class DatasManager;

class DlgCheckDatas : public wxDialog
{
	public:
		DlgCheckDatas(wxWindow *parent);
		virtual ~DlgCheckDatas();
	private:
		// Misc functions
		void CreateControls();
		void ConnectControls();
		// Events handlers
		void OnBtnStartStopClicked(wxCommandEvent &event);
		void OnUpdateUI_BtnClose(wxUpdateUIEvent &event);
		// Controls vars
		wxButton *m_btnStartStop;
		// Misc vars
		bool m_bIsRunning;
		DatasManager& m_datas;
};

#endif // __DLGCHECKDATAS_H_INCLUDED__
