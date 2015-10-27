#ifndef __DATASMANAGER_H_INCLUDED__
#define __DATASMANAGER_H_INCLUDED__

#include <wx/wx.h>

class wxXmlNode;

enum GEDITEMTYPE
{
    GIT_UNKNOWN = 0,
    GIT_ALL=GIT_UNKNOWN,
    GIT_INDI,
    GIT_FAM
};

enum GEDITEMSEX
{
    GIS_UNKNOWN = wxID_ANY,
    GIS_MALE = 0,
    GIS_FEMALE =1
};

class DatasManager
{
    public:
        static DatasManager& Get();
        void Clear();
        // Accessors
        bool IsModified();
        bool HasDatas();
        const wxString& GetCurrentFileName() { return m_sFileName; }
        int GetItemsCount(GEDITEMTYPE type=GIT_ALL);
        wxXmlNode *GetDatas() { return m_datas; }
        wxXmlNode *FindItemByGedId(const wxString& gedId);
        const wxString& GetLastError() { return m_sLastError; }
        // Parsing items datas
        wxString ParseEvent(wxXmlNode* event);
        wxString ParseDate(const wxString& value);
        // Accessing items entries
        wxString GetItemFirstName(const wxString& itmId);
        wxString GetItemFirstName(wxXmlNode* itmNode);
        wxString GetItemLastName(const wxString& itmId);
        wxString GetItemLastName(wxXmlNode* itmNode);
        wxString GetItemFullName(const wxString& itmId, bool lastFirst=true);
        wxString GetItemFullName(wxXmlNode* itmNode, bool lastFirst=true);
        GEDITEMSEX GetItemSex(const wxString& itmId);
        GEDITEMSEX GetItemSex(const wxXmlNode *itmNode);
        wxString GetItemBirth(const wxString& itmId, bool yearOnly=false);
        wxString GetItemBirth(const wxXmlNode *itmNode, bool yearOnly=false);
        wxString GetItemDeath(const wxString& itmId, bool yearOnly=false);
        wxString GetItemDeath(const wxXmlNode *itmNode, bool yearOnly=false);
        wxString GetItemInfos(wxXmlNode *itmNode);
        // Reading and saving datas
        bool ParseGedToXml(wxInputStream *source, wxXmlNode* dest);
        bool ReadGedFile(const wxString& filename);
        bool SaveXmlFile(const wxString& filename, int compLevel=0);
        // Creating a tree form the datas
        bool CreateTree();
        // Comparison with another gedcom file
        bool CompareWithGedFile(const wxString& filename);
        bool HasCompResults(bool deleted=false);
        wxString GetCompResultsSummary();
        const wxArrayString& GetModifiedCompResults() { return m_arsCompModified; }
        const wxArrayString& GetAddedCompResults() { return m_arsCompAdded; }
        const wxArrayString& GetRemovedCompResults() { return m_arsCompRemoved; }
    protected:
    private:
        DatasManager();
        virtual ~DatasManager();
        void Initialize();
        // Unique instance of the singleton
        static DatasManager m_instance;
        // Misc vars
        bool m_bInitialized, m_bModified;
        wxString m_sFileName;
        wxXmlNode* m_datas;
        int m_iMaxLevels;
        wxArrayString m_arsCompAdded, m_arsCompRemoved, m_arsCompModified;
        wxString m_sCmpFile, m_sLastError;
};

#endif // __DATASMANAGER_H_INCLUDED__
