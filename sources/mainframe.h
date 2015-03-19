#ifndef __MAINFRAME_H_INCLUDED__
#define __MAINFRAME_H_INCLUDED__

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/html/htmlwin.h>

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
        void UpdateSummary();
        void UpdateList();
        void UpdateListItem(long item);
        void UpdateItemDetails();
        // Events handlers
        void OnSize(wxSizeEvent &event);
        void OnMove(wxMoveEvent &event);
        void OnClose(wxCloseEvent &event);
        void OnOpenGedFileClicked(wxCommandEvent &event);
        void OnSaveXmlFileClicked(wxCommandEvent &event);
        void OnSavePdfFileClicked(wxCommandEvent &event);
        void OnPreferencesClicked(wxCommandEvent &event);
        void OnAboutClicked(wxCommandEvent &event);
        void OnListItemSelected(wxListEvent &event);
        void OnListItemDeselected(wxListEvent &event);
        void OnTimerSelectionCheck(wxTimerEvent &event);
        void OnUpdateUI_Save(wxUpdateUIEvent &event);
        // Controls vars
        wxStaticText *m_lblSummary;
        wxListView *m_lstItems;
        wxHtmlWindow *m_htwDetails;
        wxTimer m_tmrLstSel;
        // Misc vars
        SettingsManager& m_settings;
        DatasManager& m_datas;
};

#endif // __MAINFRAME_H_INCLUDED__
