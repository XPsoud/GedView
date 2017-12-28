#ifndef __PANELFILTER_H_INCLUDED__
#define __PANELFILTER_H_INCLUDED__

#include <wx/wx.h>

class wxXmlNode;
class wxSearchCtrl;

class PanelFilter : public wxPanel
{
	public:
		PanelFilter(wxWindow *parent);
		virtual ~PanelFilter();
		bool DoesItemMatchSearch(wxXmlNode* item);
	protected:
	private:
		// Misc functions
        void CreateControls();
        void ConnectControls();
        // Events handlers
        // Controls vars
		wxStaticBox *m_stbMain;
		wxSearchCtrl *m_txtSearch;
		wxRadioButton *m_optSearch[3];
		// Misc vars
};

#endif // __PANELFILTER_H_INCLUDED__
