#ifndef __PANELFILTER_H_INCLUDED__
#define __PANELFILTER_H_INCLUDED__

#include <wx/wx.h>

class PanelFilter : public wxPanel
{
	public:
		PanelFilter(wxWindow *parent);
		virtual ~PanelFilter();
	protected:
	private:
		// Misc functions
        void CreateControls();
        void ConnectControls();
        // Events handlers
        // Controls vars
		wxStaticBox *m_stbMain;
		// Misc vars
};

#endif // __PANELFILTER_H_INCLUDED__
