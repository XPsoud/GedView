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
        // Reading and saving datas
        bool ReadGedFile(const wxString& filename);
        bool SaveXmlFile(const wxString& filename, int compLevel=0);
        bool SaveTree2TxtFile(const wxString& filename);
        bool SaveTree2TxtFile(const wxString& filename, GedTreeItem *base, int wdth=-1, MyPdfDoc* doc=NULL);
        bool SaveTree2PdfFile(const wxString& filename);
        // Creating a tree form the datas
        bool CreateTree();
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
};

#endif // __DATASMANAGER_H_INCLUDED__
