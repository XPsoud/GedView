#ifndef __RECENTSLIST_H_INCLUDED__
#define __RECENTSLIST_H_INCLUDED__

#include <wx/wx.h>
class wxXmlNode;

class RecentsList
{
	public:
		RecentsList();
		virtual ~RecentsList();
		bool IsModified() { return m_bModified; }
		void Append(const wxString &sValue);
		void SetLastUsed(const wxString &sValue);
		int GetEntryCount() { return (int)sEntry.Count(); }
		bool IsEmpty() { return sEntry.IsEmpty(); }
		int GetMaxEntries() { return m_iMaxEntries; }
		wxString GetEntry(int Index);
		void SetMaxEntries(int iNbEntries);
		void SetAssociatedMenu(wxMenu *Menu);
		void FromXmlNode(wxXmlNode *container);
		void ToXmlNode(wxXmlNode *container);
	private:
		void UpdateMenu();
		int m_iMaxEntries;
		wxArrayString sEntry;
		wxMenu *m_mnuRecents;
		bool m_bModified;
};

#endif // __RECENTSLIST_H_INCLUDED__
