#ifndef __DLGCHECKDATAS_H_INCLUDED__
#define __DLGCHECKDATAS_H_INCLUDED__

#include <wx/wx.h>

#include <wx/dialog.h>

class DatasManager;

wxDECLARE_EVENT(wxEVT_DBCHECK_STARTED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DBCHECK_WORKING, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DBCHECK_STOPPED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DBCHECK_ENDED, wxCommandEvent);

class DlgCheckDatas : public wxDialog
{
	public:
		DlgCheckDatas(wxWindow *parent);
		virtual ~DlgCheckDatas();
	private:
		// Misc functions
		void CreateControls();
		void ConnectControls();
		void DoCheckDB();
		// Events handlers
		void OnBtnStartStopClicked(wxCommandEvent &event);
		void OnUpdateUI_BtnClose(wxUpdateUIEvent &event);
		void OnDbCheckEvent(wxCommandEvent &event);
		// Controls vars
		wxBoxSizer *m_szrMain;
		wxPanel *m_pnlParams, *m_pnlResults;
		wxTextCtrl *m_txtLog;
		wxGauge *m_pgbWork;
		wxButton *m_btnStartStop;
		// Misc vars
		bool m_bIsRunning, m_bStopWanted;
		DatasManager& m_datas;
};

#endif // __DLGCHECKDATAS_H_INCLUDED__
