#pragma once

#include "threading.h"

//メモリの空き容量を調べて、開放できるものは開放するマネージャ.
template<typename T>
class managementActive
{
    critical_section cs;
public:
	list<T*> actives;
private:
    //メモリがあるか調べる.
	bool enoughMemorySpace()
	{
		MEMORYSTATUS ms;
		GlobalMemoryStatus( & ms );
		const DWORD limitsize = 64 * 1024 * 1024;//64M以下しか物理メモリがあいてない場合には開放していく
		return ( ms.dwAvailPhys > limitsize );
	}
public:
    //初期化.
	void initialize( T * target )
	{
        check_critical_section ccs( cs );
		target->active = false;
	}
    //終了化.
	void finalize( T * target )
	{
        check_critical_section ccs( cs );
		target->active = false;
		actives.pop( target );//リストから検索して削除
	}
    //全て開放させる.
	void inactivate()
	{
        check_critical_section ccs( cs );
		while ( ! actives.empty() ) {
            T * target = actives.first();//非アクティブにする．
			target->finalize();
			finalize( target );
		}
	}
    //一つを有効化する.
	void activate( T * target )
	{
        check_critical_section ccs( cs );
		while ( ! enoughMemorySpace() ) {
			if ( actives.empty() ) break;
			T * target = actives.first();//非アクティブにする．
			target->finalize();
			finalize( target );
		}
		if ( ! target->active ) {//アクティブでない場合、追加する.
			actives.push_back( target );
			target->active = true;
		}
	}
};
