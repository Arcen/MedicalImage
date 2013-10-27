#pragma once
#include <process.h>
#include <vector>

//クリティカルセクション
class critical_section
{
    CRITICAL_SECTION cs;
public:
    critical_section()
    {
        InitializeCriticalSection( & cs );
    }
    ~critical_section()
    {
        DeleteCriticalSection( & cs );
    }
    void enter()
    {
        EnterCriticalSection( & cs );
    }
    void leave()
    {
        LeaveCriticalSection( & cs );
    }
};
class check_critical_section
{
    critical_section & cs;
public:
    check_critical_section( critical_section & cs_ ) : cs( cs_ )
    {
        cs.enter();
    }
    ~check_critical_section()
    {
        cs.leave();
    }
};
//イベント
class event_object
{
    HANDLE handle;
public:
    event_object( bool manual_reset, bool initial_status ) : handle( INVALID_HANDLE_VALUE )
    {
        handle = ::CreateEvent( NULL, manual_reset ? TRUE : FALSE, initial_status ? TRUE : FALSE, NULL );
    }
    ~event_object()
    {
        if ( handle != INVALID_HANDLE_VALUE )
        {
            ::CloseHandle( handle );
        }
    }
    bool set()
    {
        if ( handle == INVALID_HANDLE_VALUE ) return false;
        return !! SetEvent( handle );
    }
    bool reset()
    {
        if ( handle == INVALID_HANDLE_VALUE ) return false;
        return !! ResetEvent( handle );
    }
    bool pulse()
    {
        if ( handle == INVALID_HANDLE_VALUE ) return false;
        return !! PulseEvent( handle );
    }
    bool wait()
    {
        if ( handle == INVALID_HANDLE_VALUE ) return false;
        return ::WaitForSingleObject( handle, INFINITE ) == WAIT_OBJECT_0;
    }
    bool wait( DWORD millisecond )
    {
        if ( handle == INVALID_HANDLE_VALUE ) return false;
        return ::WaitForSingleObject( handle, millisecond ) == WAIT_OBJECT_0;
    }
    bool is_error()
    {
        return ( handle != INVALID_HANDLE_VALUE );
    }
};
#include <deque>
class job
{
public:
    virtual ~job(){}
};
template<typename T>
class event_queue
{
    critical_section cs;
    std::deque<T> items;
    event_object not_empty;
    bool destroy;
public:
    event_queue() : not_empty( true, false ), destroy( false )
    {
    }
    ~event_queue()
    {
        stop();
    }
    bool add_item( const T & item )
    {
        check_critical_section ccs( cs );
        if ( destroy ) return false;
        if ( items.empty() )
        {
            items.push_back( item );
            not_empty.set();
        } else {
            items.push_back( item );
        }
        return true;
    }
    bool get_item( T & item )
    {
        check_critical_section ccs( cs );
        if ( destroy ) return false;
        if ( items.empty() )
        {
            not_empty.reset();
            return false;
        }
        item = items.front();
        items.pop_front();
        if ( items.empty() )
        {
            not_empty.reset();
        }
        return true;
    }
    bool wait_any_item( DWORD millisecond )
    {
        if ( not_empty.is_error() ) return false;
        not_empty.wait( millisecond );
        check_critical_section ccs( cs );
        return ! items.empty();
    }
    bool stop()
    {
        {
            check_critical_section ccs( cs );
            destroy = true;
        }
        return not_empty.set();
    }
    bool check()
    {
        check_critical_section ccs( cs );
        return ! destroy;
    }
};
class thread
{
    HANDLE handle;
    unsigned id;
    bool error;
    event_object start;
    critical_section cs;
public:
    typedef void (*function_type)( void * data );
private:
    function_type function;
    void * extra;
public:
    thread( function_type function_, void * extra_ )
        : handle( INVALID_HANDLE_VALUE ), id( 0 ), error( false ), start( true, false ), 
        function( function_ ), extra( extra_ )
    {
        handle = ( HANDLE ) _beginthreadex( NULL, 0, thread_entry, this, 0, & id );
        if ( handle == 0 )//error
        {
            handle = INVALID_HANDLE_VALUE;
            error = true;
        } else {
            if ( ! start.wait() )
            {
                error = true;
                CloseHandle( handle );
            }
        }
    }
    ~thread()
    {
        if ( ! error )
        {
            ::WaitForSingleObject( handle, INFINITE );
            CloseHandle( handle );
        }
    }
    static unsigned int __stdcall thread_entry( void * extra )
    {
        thread * self = ( thread * ) extra;
        self->start.set();//スレッドの開始を知らせる.
        (*(self->function))( self->extra );
        return 0;
    }
    HANDLE get_handle() { return handle; }
};
