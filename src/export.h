#if !defined(__cctz_export_h__)
#define __cctz_export_h__

#if defined (_WIN32)
    #if defined(BUILDING_CCTZ_DLL)
        #define CCTZ_EXPORT __declspec(dllexport)
    #else
        #define CCTZ_EXPORT __declspec(dllimport)
    #endif
#else
    #define CCTZ_EXPORT
    #define CCTZ_LOCAL
#endif


#endif /* !defined (__cctz_export_h__) */
