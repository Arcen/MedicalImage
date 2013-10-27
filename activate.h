#pragma once

#include "threading.h"

//�������̋󂫗e�ʂ𒲂ׂāA�J���ł�����̂͊J������}�l�[�W��.
template<typename T>
class managementActive
{
    critical_section cs;
public:
	list<T*> actives;
private:
    //�����������邩���ׂ�.
	bool enoughMemorySpace()
	{
		MEMORYSTATUS ms;
		GlobalMemoryStatus( & ms );
		const DWORD limitsize = 64 * 1024 * 1024;//64M�ȉ����������������������ĂȂ��ꍇ�ɂ͊J�����Ă���
		return ( ms.dwAvailPhys > limitsize );
	}
public:
    //������.
	void initialize( T * target )
	{
        check_critical_section ccs( cs );
		target->active = false;
	}
    //�I����.
	void finalize( T * target )
	{
        check_critical_section ccs( cs );
		target->active = false;
		actives.pop( target );//���X�g���猟�����č폜
	}
    //�S�ĊJ��������.
	void inactivate()
	{
        check_critical_section ccs( cs );
		while ( ! actives.empty() ) {
            T * target = actives.first();//��A�N�e�B�u�ɂ���D
			target->finalize();
			finalize( target );
		}
	}
    //���L��������.
	void activate( T * target )
	{
        check_critical_section ccs( cs );
		while ( ! enoughMemorySpace() ) {
			if ( actives.empty() ) break;
			T * target = actives.first();//��A�N�e�B�u�ɂ���D
			target->finalize();
			finalize( target );
		}
		if ( ! target->active ) {//�A�N�e�B�u�łȂ��ꍇ�A�ǉ�����.
			actives.push_back( target );
			target->active = true;
		}
	}
};
