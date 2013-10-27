//同期チェック用セマフォクラス

class semaphore
{
	HANDLE smphr;
public:
	semaphore() : smphr( NULL )
	{
		smphr = CreateSemaphore( NULL, 0, 1, NULL );
	}
	~semaphore()
	{
		if ( smphr ) {
			CloseHandle( smphr );
			smphr = NULL;
		}
	}
	bool wait( DWORD wait_ms = INFINITE )
	{
		if ( ! smphr ) return false;
		if ( check() ) return true;
		if ( wait_ms != INFINITE ) return ( WaitForSingleObject( smphr, wait_ms ) == WAIT_OBJECT_0 );

		if ( WaitForSingleObject( smphr, 30000 ) == WAIT_OBJECT_0 ) return true;
		int r = MessageBox( NULL, "30 sec past. wait for forever", "WaitForSingleObject", MB_YESNO );
		if ( r == IDYES ) WaitForSingleObject( smphr, INFINITE );
		return true;
	}
	void signal()
	{
		if ( ! smphr ) return;
		ReleaseSemaphore( smphr, 1, NULL );
	}
	bool check()
	{
		if ( ! smphr ) return false;
		return ( WaitForSingleObject( smphr, 0 ) == WAIT_OBJECT_0 );
	}
};

//宣言により有効スコープ内でのセマフォによる同期チェック用クラス
class semaphoreCheck
{
public:
	bool semachk;
	semaphore & s;
	semaphoreCheck( semaphore & _s, bool _semachk = true ) : s( _s ), semachk( _semachk )
	{
		if ( semachk ) {
			if ( s.check() ) return;
			s.wait();
		}
	}
	~semaphoreCheck()
	{
		if ( semachk ) s.signal();
	}
};