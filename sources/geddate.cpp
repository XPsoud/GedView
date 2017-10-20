#include "geddate.h"

const wxChar* GedDate::m_sGedPrefix[] = { _T("ABT"), _T("BEF"), _T("AFT"), _T("EST")};
const wxChar* GedDate::m_sGedMonth[]  =  { _T("JAN"), _T("FEB"), _T("MAR"), _T("APR"), _T("MAY"), _T("JUN"), _T("JUL"), _T("AUG"), _T("SEP"), _T("OCT"), _T("NOV"), _T("DEC")};
const wxChar* GedDate::m_sMonthName[] = {_("January"), _("February"), _("March"), _("April"), _("May"), _("June"), _("July"), _("August"), _("September"), _("October"), _("November"), _("December")};
GedDate::GedDate(const wxString& source)
{
	Initialize();

	if (!source.IsEmpty())
		FromGedString(source);
}

GedDate::GedDate(GedDate& src)
{
	Initialize();

	m_iDay=src.m_iDay;
	m_iMonth=src.m_iMonth;
	m_iYear=src.m_iYear;
	m_iPrefix=src.m_iPrefix;
}

GedDate::~GedDate()
{
}

void GedDate::Clear()
{
	Initialize();
}

bool GedDate::FromGedString(const wxString& source)
{
	wxString sSrc=source;
	long lValue;

	sSrc.Trim(false);
	sSrc.Trim(true);

	if (sSrc.IsEmpty())
		return false;

	// First, check if a prefix is present
	for (int i=0; i<DP_COUNT; ++i)
	{
		if (sSrc.StartsWith(m_sGedPrefix[i]))
		{
			m_iPrefix=i;
			sSrc.Remove(0, 3);
			break;
		}
	}
	sSrc.Trim(false);
	// At this point, the source string must be at least 4 chars long (year number)
	if (sSrc.Length()<4)
		return false;
	// If the 3rd char is a space, the 2 previous are the day number
	if (sSrc[2]==_T(' '))
	{
		sSrc.Left(2).ToLong(&lValue);
		m_iDay=lValue;
		sSrc.Remove(0, 3);
	}
	// Now we can have the abreviated month name
	for (int i=0; i<12; ++i)
	{
		if (sSrc.StartsWith(m_sGedMonth[i]))
		{
			m_iMonth=i;
			sSrc.Remove(0, 4);
			break;
		}
	}
	// And now the year number
	sSrc.ToLong(&lValue);
	m_iYear=lValue;

	return true;
}

bool GedDate::IsSet()
{
	if ((m_iYear!=wxID_ANY)||(m_iMonth!=wxID_ANY)||(m_iDay!=wxID_ANY))
		return true;

	return false;
}

bool GedDate::IsBefore(const GedDate& other) const
{
	// First, compare years
	if (m_iYear>other.m_iYear)
		return false;
	if (m_iYear<other.m_iYear)
		return true;
	// Years are equals
	if ((m_iMonth==wxID_ANY)||(other.m_iMonth==wxID_ANY))
	{
		// Only years can be compared : Check depending prefix
		if ((m_iPrefix==other.m_iPrefix)||(m_iPrefix==DP_ABOUT)||(other.m_iPrefix==DP_ABOUT))
			return false;
		if ((m_iPrefix==DP_BEFORE)||(other.m_iPrefix==DP_AFTER))
			return true;
		// We can't really know...
		return false;
	}
	// Now compare months
	if (m_iMonth>other.m_iMonth)
		return false;
	if (m_iMonth<other.m_iMonth)
		return true;
	// Months are equals
	if ((m_iDay==wxID_ANY)||(other.m_iDay==wxID_ANY))
	{
		// Can't compare with days : check with prefix
		if ((m_iPrefix==other.m_iPrefix)||(m_iPrefix==DP_ABOUT)||(other.m_iPrefix==DP_ABOUT))
			return false;
		if ((m_iPrefix==DP_BEFORE)||(other.m_iPrefix==DP_AFTER))
			return true;
		// We can't really know...
		return false;
	}
	// Compare days
	if (m_iDay>other.m_iDay)
	{
		// TODO (Xaviou#1#): Make a check with prefix

		return false;
	}
	if (m_iDay<other.m_iDay)
	{
		// TODO (Xaviou#1#): Make a check with prefix

		return true;
	}
	// Days are equals : only prefix can make the difference
	if ((m_iPrefix==other.m_iPrefix)||(m_iPrefix==DP_ABOUT)||(other.m_iPrefix==DP_ABOUT))
		return false;
	if ((m_iPrefix==DP_BEFORE)||(other.m_iPrefix==DP_AFTER))
		return true;

	// We can't say
	return false;
}

bool GedDate::IsAfter(const GedDate& other) const
{
	return other.IsBefore(*this);
}

wxString GedDate::ToString()
{
	wxString sRes=wxEmptyString;
	switch(m_iPrefix)
	{
		case DP_NONE:
			sRes << _T("Exact");
			break;
		case DP_ABOUT:
			sRes << _T("About");
			break;
		case DP_AFTER:
			sRes << _T("After");
			break;
		case DP_BEFORE:
			sRes << _T("Before");
			break;
		case DP_ESTIMATED:
			sRes << _T("Estimated");
			break;
	}
	sRes << _T(" ");
	if (m_iDay!=wxID_ANY)
	{
		sRes << wxString::Format(_T("%02d "), m_iDay);
	}
	if (m_iMonth!=wxID_ANY)
	{
		sRes << wxString::Format(_T("%s "), m_sMonthName[m_iMonth]);
	}
	sRes << m_iYear;

	return sRes;
}

void GedDate::Initialize()
{
	m_iDay=wxID_ANY;
	m_iMonth=wxID_ANY;
	m_iYear=wxID_ANY;
	m_iPrefix=DP_NONE;
}
