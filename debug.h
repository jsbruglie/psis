// Verbose
#ifdef VERBOSE
#define debugPrint(str){printf(str);}
#define debugPrint1(str,arg){printf(str,arg);}
#define debugPrint4(str,arg1,arg2,arg3,arg4){printf(str,arg1,arg2,arg3,arg4);}
#else
#define debugPrint(str)
#define debugPrint1(str,arg)
#define debugPrint4(str,arg1,arg2,arg3,arg4)
#endif

