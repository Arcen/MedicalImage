//同期チェック用セマフォクラス

class semaphore
{
	HANDLE smphr;
public:
	semaphore() : smphr( CreateSemaphore( NULL, 0, 1, NULL ) ) {}
	~semaphore()
	{
		if ( smphr ) CloseHandle( smphr );
	}
	bool wait( DWORD wait_ms = INFINITE )
	{
		if ( ! smphr ) return false;
		return ( WaitForSingleObject( smphr, wait_ms ) == WAIT_OBJECT_0 );
	}
	void signal()
	{
		if ( ! smphr ) return;
		ReleaseSemaphore( smphr, 1, NULL );
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
			s.wait();
		}
	}
	~semaphoreCheck()
	{
		if ( semachk ) s.signal();
	}
};