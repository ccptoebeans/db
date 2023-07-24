#ifdef TEST
    class MockAccessor;
    #define ACCESSOR MockAccessor
#else
    #define ACCESSOR OurAccessor
#endif