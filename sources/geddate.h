#ifndef __GEDDATE_H_INCLUDED__
#define __GEDDATE_H_INCLUDED__

#include <wx/wx.h>

enum DatePrefix
{
	DP_NONE=-1,
	// Order of the following values must follow the content
	// of the GedDate::m_sGedPrefix static vars
	DP_ABOUT,
	DP_BEFORE,
	DP_AFTER,
	DP_ESTIMATED,
	//
	DP_COUNT
};

class GedDate
{
	public:
		GedDate(const wxString& source=wxEmptyString);
		GedDate(GedDate& src);
		virtual ~GedDate();
		void Clear();
		bool FromGedString(const wxString& source);
		bool IsSet();
		bool IsBefore(const GedDate& other) const;
		bool IsAfter(const GedDate& other) const;
		// Only for debug purpose (needs to be removed)
		wxString ToString();
	protected:
	private:
		// Misc functions
		void Initialize();
		// Vars
		static const wxChar* m_sGedPrefix[];
		static const wxChar* m_sGedMonth[];
		static const wxChar* m_sMonthName[];
		int m_iYear, m_iMonth, m_iDay, m_iPrefix;
};

#endif // __GEDDATE_H_INCLUDED__
