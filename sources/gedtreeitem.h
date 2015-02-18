#ifndef __GEDTREEITEM_H_INCLUDED__
#define __GEDTREEITEM_H_INCLUDED__

#include <wx/wx.h>

class wxXmlNode;

class GedTreeItem
{
	public:
		GedTreeItem(wxXmlNode* node=NULL);
		virtual ~GedTreeItem();
		bool IsAlone();
		void SetRelatedXmlNode(wxXmlNode* node) { m_Item=node; }
		wxXmlNode* GetRelatedXmlNode() { return m_Item; }
		static int GetItemsCount() { return m_iItemsCount; }
		GedTreeItem *GetLastChild();
		bool AddHusbOrWife(GedTreeItem *item);
		void AddChild(GedTreeItem *item);
		bool AddParent(GedTreeItem *item);
		int GetAscLevelsCount();
		int GetDescLevelsCount();
		int GetLevel() { return m_iLevel; }
		void SetLevel(int value) { m_iLevel=value; }
		bool AutoUpdateLevels();
		GedTreeItem *m_parent1, *m_parent2, *m_HusbWife1, *m_HusbWife2, *m_FirstChild, *m_FirstBS;
		wxString GetFirstName();
		wxString GetLastName();
		wxString GetDates();
	protected:
	private:
	    static int m_iItemsCount;
	    wxXmlNode *m_Item;
	    int m_iLevel;
};

#include <wx/list.h>
WX_DECLARE_LIST(GedTreeItem, ListOfGedTreeItems);

#endif // __GEDTREEITEM_H_INCLUDED__
