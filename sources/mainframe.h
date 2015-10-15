#ifndef __MAINFRAME_H_INCLUDED__
#define __MAINFRAME_H_INCLUDED__

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/html/htmlwin.h>

class SettingsManager;
class DatasManager;

extern const wxEventType wxEVT_FILEOPEN;

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
        void OnAutoOpenGedFile(wxCommandEvent &event);
        void OnSaveXmlFileClicked(wxCommandEvent &event);
        void OnSavePdfFileClicked(wxCommandEvent &event);
        void OnPreferencesClicked(wxCommandEvent &event);
        void OnAboutClicked(wxCommandEvent &event);
        void OnListItemSelected(wxListEvent &event);
        void OnListItemDeselected(wxListEvent &event);
        void OnColumnHeaderClicked(wxListEvent &event);
        void OnTimerSelectionCheck(wxTimerEvent &event);
        void OnHtmlLinkClicked(wxHtmlLinkEvent& event);
        void OnHistoryBackClicked(wxCommandEvent &event);
        void OnHistoryNextClicked(wxCommandEvent &event);
        void OnCompareClicked(wxCommandEvent &event);
        void OnShashPosChanged(wxSplitterEvent &event);
        void OnUpdateUI_Save(wxUpdateUIEvent &event);
        void OnUpdateUI_Backward(wxUpdateUIEvent& event);
        void OnUpdateUI_Forward(wxUpdateUIEvent& event);
        void OnUpdateUI_Compare(wxUpdateUIEvent& event);
        static int wxCALLBACK SortCompFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
        // Controls vars
        wxStaticText *m_lblSummary;
        wxSplitterWindow *m_spwSplitter;
        wxListView *m_lstItems;
        wxHtmlWindow *m_htwDetails;
        wxTimer m_tmrLstSel;
        // Misc vars
        wxArrayString m_arsHistory;
        int m_iHistPos, m_iSortCol;
        bool m_bHistClicked;
        SettingsManager& m_settings;
        DatasManager& m_datas;
};

#endif // __MAINFRAME_H_INCLUDED__
