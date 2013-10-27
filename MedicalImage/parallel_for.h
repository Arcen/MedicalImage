#pragma once
#include "threading.h"

class parallel_for
{
    critical_section cs;
    int start;
    int sentinel;
    std::vector<thread*> threads;
    typedef void (*function_type)( void * data, int index );
    function_type function;
    void * extra;
    event_object process_status;
    fc::window * parent;
public:
    parallel_for( fc::window * parent_, int size, int start_, int sentinel_, function_type function_, void * extra_ )
        : start( start_ ), sentinel( sentinel_ ), function( function_ ), extra( extra_ ), 
        process_status( true, false ), parent( parent_ )
    {
        threads.resize( size );
        for ( int i = 0; i < threads.size(); ++i )
        {
            threads[i] = new thread( process, this );
        }
    }
    ~parallel_for()
    {
        {
            check_critical_section ccs( cs );
            start = sentinel;//新しい処理を開始しない.
        }
        join();
    }
    void begin()
    {
        process_status.set();
    }
    void join()
    {
        //全てのスレッド完了を待つ.
        DWORD n = threads.size();
        if ( n )
        {
            std::vector<HANDLE> handles;
            handles.reserve( n );
            for ( int i = 0; i < static_cast<int>( n ); ++i )
            {
                HANDLE handle = threads[i]->get_handle();
                if ( handle != INVALID_HANDLE_VALUE ) handles.push_back( handle );
            }
            n = handles.size();
            if ( n )
            {
                parent->enable( false );
                while ( true )
                {
                    DWORD result = ::MsgWaitForMultipleObjectsEx( n, & handles.front(), 100, QS_ALLINPUT, MWMO_WAITALL );
                    if ( WAIT_OBJECT_0 <= result && result < WAIT_OBJECT_0 + n ) break;//全てのオブジェクトがシグナル状態.
                    if ( WAIT_ABANDONED_0 <= result && result < WAIT_ABANDONED_0 + n ) break;//ミューテックスオブジェクトの場合にキャンセル状態も含む.
                    //メッセージイベント、APCイベント、タイムアウト時.
                    parent->process_message();//メッセージ.
                    //::OutputDebugString(":");
                }
                parent->enable( true );
            }
        }
        for ( int i = 0; i < threads.size(); ++i )
        {
            if ( threads[i] )
            {
                delete threads[i];
                threads[i] = 0;
            }
        }
        threads.clear();
    }
    static void process( void * data )
    {
        parallel_for * self = ( parallel_for * ) data;
        self->process_status.wait();//開始を待つ.
        while ( true )
        {
            int index = 0;
            {
                check_critical_section ccs( self->cs );
                if ( self->sentinel <= self->start ) return;//終了.
                index = self->start;
                ++self->start;
            }
            self->function( self->extra, index );
        }
    }
};
