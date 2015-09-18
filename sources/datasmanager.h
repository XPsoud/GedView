#ifndef __DATASMANAGER_H_INCLUDED__
#define __DATASMANAGER_H_INCLUDED__

#include <wx/wx.h>

class wxXmlNode;
#include "gedtreeitem.h"
#include "mypdfdoc.h"

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
        wxString GetItemBirth(const wxString& itmId);
        wxString GetItemBirth(const wxXmlNode *itmNode);
        wxString GetItemDeath(const wxString& itmId);
        wxString GetItemDeath(const wxXmlNode *itmNode);
        wxString GetItemInfos(wxXmlNode *itmNode);
        // Reading and saving datas
        bool ParseGedToXml(wxInputStream *source, wxXmlNode* dest);
        bool ReadGedFile(const wxString& filename);
        bool SaveXmlFile(const wxString& filename, int compLevel=0);
        bool SaveTree2TxtFile(const wxString& filename);
        bool SaveTree2TxtFile(const wxString& filename, GedTreeItem *base, int wdth=-1, MyPdfDoc* doc=NULL);
        bool SaveTree2PdfFile(const wxString& filename);
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
        GedTreeItem *FindTreeItem(const wxString& itemID);
        // Unique instance of the singleton
        static DatasManager m_instance;
        // Misc vars
        bool m_bInitialized, m_bModified;
        wxString m_sFileName;
        wxXmlNode* m_datas;
        ListOfGedTreeItems m_treeItems;
        GedTreeItem *m_baseItem;
        int m_iMaxLevels;
        wxArrayString m_arsCompAdded, m_arsCompRemoved, m_arsCompModified;
        wxString m_sCmpFile;
};

#endif // __DATASMANAGER_H_INCLUDED__
