#ifndef __SETTINGSMANAGER_H_INCLUDED__
#define __SETTINGSMANAGER_H_INCLUDED__

#include <wx/wx.h>

extern const wxChar* szStdXmlFileHeader;
extern const int iStdXmlHeaderSize;

#define MINIMUM_PANE_WIDTH 250

class SettingsManager
{
    public:
        static SettingsManager& Get();
        bool ReadSettings();
		bool SaveSettings();
		// Accessors
		bool IsModified();
		wxString GetAppPath() { return m_sAppPath; }
		wxString GetDatasPath() { return m_sDatasPath; }
		// General password
		wxString GetPassword() { return m_sPassword; }
		void SetPassword(const wxString& pwd);
		// Datas and settings compression
		bool GetCompressSettings() { return m_bCompSettings; }
		void SetCompressSettings(bool value=true);
		bool GetCompressDatas() { return m_bCompDatas; }
		void SetCompressDatas(bool value=true);
		// Position of the main window at application startup
		int GetMainWndStartupPos() { return m_iStartPos; }
		void SetMainWndStartupPos(int value);
		void GetMainWndStartupPos(wxPoint& pos) { pos=m_ptStartPos; }
		const wxSize& GetMainWndStartupSize() { return m_szStartSize; }
        void SetLastWindowRect(const wxPoint& pos, const wxSize& size);
        int GetLastSashPos() { return m_iShashPos; }
        void SetLastSashPos(int pos);
        // Multiple instances
        bool GetMultipleInstancesAllowed() { return (m_bSingleInstance==false); }
        void SetMultipleInstancesAllowed(bool value);
        // Show splash screen at startup
        bool GetShowSplashScreen() { return m_bShowSplashScreen; }
        void SetShowSplashScreen(bool show);
    protected:
    private:
        SettingsManager();
        virtual ~SettingsManager();
        void Initialize();
        const wxString StartupPos2String(int iValue);
        int StartupString2Pos(const wxString& sValue);
        // Unique instance of the singleton
        static SettingsManager m_instance;
        // Misc vars
        bool m_bInitialized, m_bModified;
        wxString m_sAppPath, m_sDatasPath;
        // Settings vars
        wxString m_sPassword;
        bool m_bCompSettings, m_bCompDatas;
        int m_iStartPos, m_iShashPos;
        wxPoint m_ptStartPos;
        wxSize m_szStartSize;
        bool m_bSingleInstance, m_bShowSplashScreen;
};

#endif // __SETTINGSMANAGER_H_INCLUDED__
