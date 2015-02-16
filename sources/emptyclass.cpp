#include "emptyclass.h"

EmptyClass::EmptyClass()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"EmptyClass\" object\n"));
#endif // DEBUG
}

EmptyClass::~EmptyClass()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"EmptyClass\" object\n"));
#endif // DEBUG
}
