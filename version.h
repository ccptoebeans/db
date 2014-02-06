#include "windows.h"

#define EVEFILEDESC "CCP DB Library\0"
#ifndef _DEBUG
#define EVEINTFILENAME "db\0"
#define EVEFILENAME "db.dll\0"
#else
#define EVEINTFILENAME "db_d\0"
#define EVEFILENAME "db_d.dll\0"
#endif
#define EVEFILETYPE VFT_DLL

#include "autoversion.h"
//standard file version thing
#include <../version/evebuildver.h>
