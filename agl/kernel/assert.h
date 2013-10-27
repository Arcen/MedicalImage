////////////////////////////////////////////////////////////////////////////////
// Assert

#ifdef assert
#undef assert
#endif

void assertInstance( const char * exp, const char * file, const int line );

//#ifdef NDEBUG
#ifdef NASSERT

#define assert(exp);

#else

#define assert(exp);	if ( ! ( exp ) ) assertInstance( #exp, __FILE__, __LINE__ );

#endif

#ifdef __GLOBAL__

void assertInstance( const char * exp, const char * file, const int line )
{
	static bool retry = true;
	if ( ! retry ) return;
	char string[1024];
	sprintf( string, "Assert Error %s at %s ( %d )", exp, file, line );
	DebugBreak();//デバッグモードならブレイクポイントを置く。大抵はバグなのでここで止めるように
	switch ( MessageBox( NULL, string, "AGL ASSERT ERROR", MB_ABORTRETRYIGNORE ) ) {
	case IDIGNORE://次からは見逃す
		retry = false;
		return;
	case IDRETRY://次もチェックする
		retry = true;
		return;
	case IDABORT://強制終了する
	default:
		abort();
	}
}

#endif
