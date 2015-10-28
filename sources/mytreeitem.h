#ifndef __MYTREEITEM_H_INCLUDED__
#define __MYTREEITEM_H_INCLUDED__

#include <wx/wx.h>

#define V_GAP 6.
#define H_GAP 2.
#define MIN_WIDTH 15.
#define MIN_HEIGHT 10.

class wxXmlNode;

class MyTreeItem
{
	public:
		MyTreeItem(int level=-1, wxXmlNode *item=NULL);
		virtual ~MyTreeItem();
		void SetLevel(int level);
		int GetLevel() { return m_iLevel; }
		void SetSosa(int value);
		int GetSosa() { return m_iSosa; }
		void SetNodeItem(wxXmlNode *item);
		wxXmlNode* GetNodeItem() { return m_NodeItem; }
		wxString GetItemId();
		void SetFather(MyTreeItem *father);
		MyTreeItem* GetFather() { return m_Father; }
		wxXmlNode* GetFatherNode() { return m_NodeFather; }
		void SetMother(MyTreeItem *mother);
		MyTreeItem* GetMother() { return m_Mother; }
		wxXmlNode* GetMotherNode() { return m_NodeMother; }
		void SetChild(MyTreeItem *child);
		MyTreeItem* GetChild() { return m_Child; }
		void UpdateParents();
		void SetXPos(double pos);
		double GetXPos() { return m_dXPos; }
		void SetYPos(double pos);
		double GetYPos() { return m_dYPos; }
		void SetItemDec(double value);
		double GetItemDec() { return m_dDec; }
		void SetLeftItem(MyTreeItem *item);
		MyTreeItem* GetLeftItem() { return m_LeftItem; }
		double GetXMin();
		double GetXMax();
		double GetYMin();
		double GetYMax();
		void SetRelativePos(double yPos);
		void SetFinalPos(double xPos);
		double GetItemWidth() { return m_dWidth; }
		void SetItemWidth(double width);
		double GetItemHeight() { return m_dHeight; }
		void SetItemHeight(double height);
		void UpdateItemText();
		const wxString& GetItemFirstName();
		const wxString& GetItemLastName();
		const wxString GetItemText();
		const wxString& GetItemDates();
		const wxString& GetItemMarriage();
		int GetMaxLevel();
		int GetSubItemsCount();
	protected:
	private:
	    MyTreeItem *m_Father, *m_Mother, *m_Child, *m_LeftItem;
	    wxXmlNode *m_NodeItem, *m_NodeFather, *m_NodeMother;
	    int m_iLevel, m_iSosa;
	    wxString m_sItemId, m_sItemFName, m_sItemLName, m_sItemDates, m_sItemMarr;
	    double m_dXPos, m_dYPos, m_dDec;
	    double m_dWidth, m_dHeight;
};

WX_DECLARE_LIST(MyTreeItem, ListOfTreeItems);

#endif // __MYTREEITEM_H_INCLUDED__
