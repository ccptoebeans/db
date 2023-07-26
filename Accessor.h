
/* 
	*************************************************************************

	Accessor.h

	Author:    James Hawk
	Created:   July. 2023

	Description:   

        Creates a way that the accessor can be simply swapped out.
        Opens ability to create an accessor disconnected from db for testing.
        Macro used to limit code changes in db extension

        Without macro one would have to pass in the type to the template
        This change then has reprocussions throughout many files.
        Obfuscates the code.
        NSession would then also need to be a templated class due to
        some of its members which reference accessors. This is
        a blue class which would break the exposure macros, so would require
        refactor to work which is undesirable when adding this testing hook.

	(c) CCP 2023

	*************************************************************************
*/
#ifdef TEST
    class MockAccessor;
    #define ACCESSOR MockAccessor
#else
    #include "OurAccessor.h"
    #define ACCESSOR OurAccessor
#endif