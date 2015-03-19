#ifndef __MAINFRAME_H_INCLUDED__
#define __MAINFRAME_H_INCLUDED__

#include <wx/wx.h>

class SettingsManager;
class DatasManager;

class MainFrame: public wxFrame
{
    public:
        MainFrame(const wxString& title);
        ~MainFrame();
    private:
        // Misc functions
        void CreateControls();
        void ConnectControls();
        // Events handlers
        void OnSize(wxSizeEvent &event);
        void OnMove(wxMoveEvent &event);
        void OnClose(wxCloseEvent &event);
        void OnOpenGedFileClicked(wxCommandEvent &event);
        void OnSaveXmlFileClicked(wxCommandEvent &event);
        void OnPreferencesClicked(wxCommandEvent &event);
        void OnAboutClicked(wxCommandEvent &event);
        void OnUpdateUI_Save(wxUpdateUIEvent &event);
        // Controls vars
        // Misc vars
        SettingsManager& m_settings;
        DatasManager& m_datas;
};

#endif // __MAINFRAME_H_INCLUDED__
