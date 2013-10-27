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
	DebugBreak();//�f�o�b�O���[�h�Ȃ�u���C�N�|�C���g��u���B���̓o�O�Ȃ̂ł����Ŏ~�߂�悤��
	switch ( MessageBox( NULL, string, "AGL ASSERT ERROR", MB_ABORTRETRYIGNORE ) ) {
	case IDIGNORE://������͌�����
		retry = false;
		return;
	case IDRETRY://�����`�F�b�N����
		retry = true;
		return;
	case IDABORT://�����I������
	default:
		abort();
	}
}

#endif
