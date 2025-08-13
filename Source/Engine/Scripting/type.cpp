#include "Engine/Scripting/type.h"
#include "Engine/Scripting/lib.h"

#if GLEX_REPORT_MEMORY_LEAKS
void glex::py::WatchLists(MethodList& methodList, PropertyList& propertyList)
{
	LibraryInfo::s_methodLists.push_back(&methodList);
	LibraryInfo::s_propertyLists.push_back(&propertyList);
}
#endif