#ifndef __PANELFILTER_H_INCLUDED__
#define __PANELFILTER_H_INCLUDED__

#include <wx/wx.h>

wxDECLARE_EVENT(wxEVT_FILTER_CHANGED, wxCommandEvent);

class wxXmlNode;
class wxSearchCtrl;

class PanelFilter : public wxPanel
{
	public:
		PanelFilter(wxWindow *parent, wxWindowID id);
		virtual ~PanelFilter();
		bool DoesItemMatchSearch(wxXmlNode* item);
	protected:
	private:
		// Misc functions
        void CreateControls();
        void ConnectControls();
        // Events handlers
        void OnSearchButtonClicked(wxCommandEvent &event);
        void OnOptSearchClicked(wxCommandEvent &event);
        // Controls vars
		wxStaticBox *m_stbMain;
		wxSearchCtrl *m_txtSearch;
		wxRadioButton *m_optSearch[3];
		// Misc vars
		bool m_bLastSearchEmpty;
};

#endif // __PANELFILTER_H_INCLUDED__
