#pragma once
#include <map>
#include <atlbase.h>

typedef MSXML2::ISAXLocator ISAXLocator;
typedef MSXML2::ISAXAttributes ISAXAttributes;
typedef MSXML2::ISAXContentHandler ISAXContentHandler;
typedef MSXML2::ISAXErrorHandler ISAXErrorHandler;
typedef MSXML2::ISAXLexicalHandler ISAXLexicalHandler;
typedef MSXML2::ISAXDTDHandler ISAXDTDHandler;
typedef MSXML2::ISAXDeclHandler ISAXDeclHandler;
typedef MSXML2::ISAXEntityResolver ISAXEntityResolver;
typedef MSXML2::ISAXXMLReaderPtr ISAXXMLReaderPtr;
typedef MSXML2::IMXWriterPtr IMXWriterPtr;
typedef MSXML2::ISAXContentHandlerPtr ISAXContentHandlerPtr;
typedef MSXML2::IMXAttributesPtr IMXAttributesPtr;
typedef MSXML2::ISAXAttributesPtr ISAXAttributesPtr;


inline string wstring2string(const std::wstring & source, UINT CodePage = CP_THREAD_ACP)
{
    const int size = ::WideCharToMultiByte( CodePage, 0, source.c_str(), source.size(), NULL, 0, NULL, NULL);
    if ( 0 < size )
    {
        string result;
        result.allocate( size );
        if ( ::WideCharToMultiByte( CodePage, 0, source.c_str(), source.size(), result.chars(), size, NULL, NULL ) == size )
        {
		    result[size] = '\0';
            return result;
        }
    }
    return string();
}

inline std::wstring string2wstring(const string & source, UINT CodePage = CP_THREAD_ACP )
{
    const int size = ::MultiByteToWideChar( CodePage, MB_ERR_INVALID_CHARS, source.chars(), source.length(), NULL, 0);
    if ( 0 < size )
    {
        std::wstring result;
        result.resize( size, 0 );
        if ( ::MultiByteToWideChar( CodePage, MB_ERR_INVALID_CHARS, source.chars(), source.length(), &*result.begin(), size ) == size )
        {
            return result;
        }
    }
    return std::wstring();
}

inline string wstring2utf8(const std::wstring & source)
{
    return wstring2string( source, CP_UTF8 );
}

inline std::wstring utf82wstring(const string & source, UINT CodePage = CP_THREAD_ACP )
{
    return string2wstring( source, CP_UTF8 );
}

class SAXLocator
{
    ISAXLocator * locator;
    SAXLocator();
public:
    explicit SAXLocator( ISAXLocator * locator_ )
        : locator( locator_ ) {}
    SAXLocator( const SAXLocator & locator_ ) : locator( locator_.locator ) {}
    int getColumnNumber() const { return const_cast<ISAXLocator*>( locator )->getColumnNumber(); }
    int getLineNumber() const { return const_cast<ISAXLocator*>( locator )->getLineNumber(); }
    wchar_t * getPublicId() const { return reinterpret_cast<wchar_t*>( const_cast<ISAXLocator*>( locator )->getPublicId() ); }
    wchar_t * getSystemId() const { return reinterpret_cast<wchar_t*>( const_cast<ISAXLocator*>( locator )->getSystemId() ); }
};
class SAXAttributes
{
    ISAXAttributes * attributes;
public:
    explicit SAXAttributes( ISAXAttributes * attributes_ ) : attributes( ( ISAXAttributes * ) attributes_ ) {}
    SAXAttributes( const SAXAttributes & attributes_ ) : attributes( attributes_.attributes ) {}

    int getLength() { return attributes->getLength(); }
    bool getURI( int nIndex, wchar_t * & uri, int & uriSize )
    {
        return attributes->getURI( nIndex, reinterpret_cast<unsigned short **>( & uri ), & uriSize ) == S_OK;
    }
    bool getLocalName( int nIndex, wchar_t * & localname, int & localnameSize )
    {
        return attributes->getLocalName( nIndex, reinterpret_cast<unsigned short **>( & localname ), & localnameSize ) == S_OK;
    }
    bool getQName( int nIndex, wchar_t * & qname, int & qnameSize )
    {
        return attributes->getQName( nIndex, reinterpret_cast<unsigned short **>( & qname ), & qnameSize ) == S_OK;
    }
    bool getName( int nIndex, wchar_t * & uri, int & uriSize, wchar_t * & localname, int & localnameSize, wchar_t * & qname, int & qnameSize )
    {
        return attributes->getName( nIndex,
            reinterpret_cast<unsigned short **>( & uri ), & uriSize,
            reinterpret_cast<unsigned short **>( & localname ), & localnameSize,
            reinterpret_cast<unsigned short **>( & qname ), & qnameSize
            ) == S_OK;
    }
    int getIndexFromName( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize )
    {
        return attributes->getIndexFromName( 
            reinterpret_cast<unsigned short *>( uri ), uriSize,
            reinterpret_cast<unsigned short *>( localname ), localnameSize
            );
    }
    int getIndexFromQName( wchar_t * qname, int qnameSize )
    {
        return attributes->getIndexFromQName(
            reinterpret_cast<unsigned short *>( qname ), qnameSize
            );
    }
    bool getType( int nIndex, wchar_t * & type, int & typeSize )
    {
        return attributes->getType( nIndex, reinterpret_cast<unsigned short **>( & type ), & typeSize ) == S_OK;
    }
    bool getTypeFromName( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * & type, int & typeSize )
    {
        return attributes->getTypeFromName( 
            reinterpret_cast<unsigned short *>( uri ), uriSize,
            reinterpret_cast<unsigned short *>( localname ), localnameSize,
            reinterpret_cast<unsigned short **>( & type ), & typeSize ) == S_OK;
    }
    bool getTypeFromQName( wchar_t * qname, int qnameSize, wchar_t * & type, int & typeSize )
    {
        return attributes->getTypeFromQName(
            reinterpret_cast<unsigned short *>( qname ), qnameSize,
            reinterpret_cast<unsigned short **>( & type ), & typeSize ) == S_OK;
    }
    bool getValue( int nIndex, wchar_t * & value, int & valueSize )
    {
        return attributes->getValue( nIndex, reinterpret_cast<unsigned short **>( & value ), & valueSize ) == S_OK;
    }
    bool getValueFromName( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * & value, int & valueSize )
    {
        return attributes->getValueFromName( 
            reinterpret_cast<unsigned short *>( uri ), uriSize,
            reinterpret_cast<unsigned short *>( localname ), localnameSize,
            reinterpret_cast<unsigned short **>( & value ), & valueSize ) == S_OK;
    }
    bool getValueFromQName( wchar_t * qname, int qnameSize, wchar_t * & value, int & valueSize )
    {
        return attributes->getValueFromQName( 
            reinterpret_cast<unsigned short *>( qname ), qnameSize,
            reinterpret_cast<unsigned short **>( & value ), & valueSize ) == S_OK;
    }
};

class SAXContentHandler : public ISAXContentHandler
{
public:
    virtual void putDocumentLocator( const SAXLocator & locator )
    {
    }
    virtual void startDocument()
    {
    }
    virtual void endDocument()
    {
    }
    virtual void startElement( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * qname, int qnameSize, SAXAttributes & attributes )
    {
    }
    virtual void endElement( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * qname, int qnameSize )
    {
    }
    virtual void characters( wchar_t * chars, int charsSize )
    {
    }
    virtual void ignorableWhitespace( wchar_t * chars, int charsSize )
    {
    }
    virtual void processingInstruction( wchar_t * target, int targetSize, wchar_t * data, int dataSize )
    {
    }
    virtual void skippedEntity( wchar_t * name, int nameSize )
    {
    }
private:
    virtual HRESULT __stdcall raw_putDocumentLocator ( ISAXLocator * pLocator )
    {
        SAXLocator locator( pLocator );
        putDocumentLocator( locator );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_startDocument( )
    {
        startDocument();
        return S_OK;
    }
    virtual HRESULT __stdcall raw_endDocument ( )
    {
        endDocument();
        return S_OK;
    }
    virtual HRESULT __stdcall raw_startPrefixMapping ( unsigned short * pwchPrefix, int cchPrefix, unsigned short * pwchUri, int cchUri )
    {
        return S_OK;
    }
    virtual HRESULT __stdcall raw_endPrefixMapping ( unsigned short * pwchPrefix, int cchPrefix )
    {
        return S_OK;
    }
    virtual HRESULT __stdcall raw_startElement ( unsigned short * pwchNamespaceUri, int cchNamespaceUri, unsigned short * pwchLocalName, int cchLocalName, unsigned short * pwchQName, int cchQName, ISAXAttributes * pAttributes )
    {
        SAXAttributes attributes( pAttributes );
        startElement( 
            reinterpret_cast<wchar_t *>( pwchNamespaceUri ), cchNamespaceUri,
            reinterpret_cast<wchar_t *>( pwchLocalName ), cchLocalName,
            reinterpret_cast<wchar_t *>( pwchQName ), cchQName,
            attributes );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_endElement ( unsigned short * pwchNamespaceUri, int cchNamespaceUri, unsigned short * pwchLocalName, int cchLocalName, unsigned short * pwchQName, int cchQName )
    {
        endElement( 
            reinterpret_cast<wchar_t *>( pwchNamespaceUri ), cchNamespaceUri,
            reinterpret_cast<wchar_t *>( pwchLocalName ), cchLocalName,
            reinterpret_cast<wchar_t *>( pwchQName ), cchQName );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_characters ( unsigned short * pwchChars, int cchChars )
    {
        characters( reinterpret_cast<wchar_t *>( pwchChars ), cchChars );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_ignorableWhitespace ( unsigned short * pwchChars, int cchChars )
    {
        ignorableWhitespace( reinterpret_cast<wchar_t *>( pwchChars ), cchChars );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_processingInstruction ( unsigned short * pwchTarget, int cchTarget, unsigned short * pwchData, int cchData )
    {
        processingInstruction( 
            reinterpret_cast<wchar_t *>( pwchTarget ), cchTarget,
            reinterpret_cast<wchar_t *>( pwchData ), cchData );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_skippedEntity ( unsigned short * pwchName, int cchName )
    {
        skippedEntity( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName );
        return S_OK;
    }
};
class SAXErrorHandler : public ISAXErrorHandler
{
public:
    virtual bool error( const SAXLocator & locator, wchar_t * message, int code )
    {
        return false;
    }
    virtual bool fatalError( const SAXLocator & locator, wchar_t * message, int code )
    {
        return false;
    }
    virtual bool ignorableWarning( const SAXLocator & locator, wchar_t * message, int code )
    {
        return true;
    }
private:
    virtual HRESULT __stdcall raw_error ( ISAXLocator * pLocator, unsigned short * pwchErrorMessage, HRESULT hrErrorCode )
    {
        SAXLocator locator( pLocator );
        return error( locator, reinterpret_cast<wchar_t*>( pwchErrorMessage ), static_cast<int>( hrErrorCode ) ) ? S_OK : -1;
    }
    virtual HRESULT __stdcall raw_fatalError ( ISAXLocator * pLocator, unsigned short * pwchErrorMessage, HRESULT hrErrorCode )
    {
        SAXLocator locator( pLocator );
        return fatalError( locator, reinterpret_cast<wchar_t*>( pwchErrorMessage ), static_cast<int>( hrErrorCode ) ) ? S_OK : -1;
    }
    virtual HRESULT __stdcall raw_ignorableWarning ( ISAXLocator * pLocator, unsigned short * pwchErrorMessage, HRESULT hrErrorCode )
    {
        SAXLocator locator( pLocator );
        return ignorableWarning( locator, reinterpret_cast<wchar_t*>( pwchErrorMessage ), static_cast<int>( hrErrorCode ) ) ? S_OK : -1;
    }
};
class SAXLexicalHandler : public ISAXLexicalHandler
{
public:
    virtual void startDTD( wchar_t * name, int nameSize, wchar_t * publicId, int publicIdSize, wchar_t * systemId, int systemIdSize )
    {
    }
    virtual void endDTD()
    {
    }
    virtual void startEntity( wchar_t * name, int nameSize )
    {
    }
    virtual void endEntity( wchar_t * name, int nameSize )
    {
    }
    virtual void startCDATA()
    {
    }
    virtual void endCDATA()
    {
    }
    virtual void comment( wchar_t * chars, int charsSize )
    {
    }
private:
    virtual HRESULT __stdcall raw_startDTD ( unsigned short * pwchName, int cchName, unsigned short * pwchPublicId, int cchPublicId, unsigned short * pwchSystemId, int cchSystemId )
    {
        startDTD( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName,
            reinterpret_cast<wchar_t *>( pwchPublicId ), cchPublicId,
            reinterpret_cast<wchar_t *>( pwchSystemId ), cchSystemId );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_endDTD ( )
    {
        endDTD();
        return S_OK;
    }
    virtual HRESULT __stdcall raw_startEntity ( unsigned short * pwchName, int cchName )
    {
        startEntity( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_endEntity ( unsigned short * pwchName, int cchName )
    {
        endEntity( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_startCDATA ( )
    {
        startCDATA();
        return S_OK;
    }
    virtual HRESULT __stdcall raw_endCDATA ( )
    {
        endCDATA();
        return S_OK;
    }
    virtual HRESULT __stdcall raw_comment ( unsigned short * pwchChars, int cchChars )
    {
        comment( 
            reinterpret_cast<wchar_t *>( pwchChars ), cchChars );
        return S_OK;
    }
};
class SAXDTDHandler : public ISAXDTDHandler
{
public:
    virtual void notationDecl( wchar_t * name, int nameSize, wchar_t * publicId, int publicIdSize, wchar_t * systemId, int systemIdSize )
    {
    }
    virtual void unparsedEntityDecl( wchar_t * name, int nameSize, wchar_t * publicId, int publicIdSize, wchar_t * systemId, int systemIdSize, wchar_t * notationName, int notationNameSize )
    {
    }
public:
    virtual HRESULT __stdcall raw_notationDecl ( unsigned short * pwchName, int cchName, unsigned short * pwchPublicId, int cchPublicId, unsigned short * pwchSystemId, int cchSystemId )
    {
        notationDecl( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName,
            reinterpret_cast<wchar_t *>( pwchPublicId ), cchPublicId,
            reinterpret_cast<wchar_t *>( pwchSystemId ), cchSystemId );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_unparsedEntityDecl ( unsigned short * pwchName, int cchName, unsigned short * pwchPublicId, int cchPublicId, unsigned short * pwchSystemId, int cchSystemId, unsigned short * pwchNotationName, int cchNotationName )
    {
        unparsedEntityDecl( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName,
            reinterpret_cast<wchar_t *>( pwchPublicId ), cchPublicId,
            reinterpret_cast<wchar_t *>( pwchSystemId ), cchSystemId,
            reinterpret_cast<wchar_t *>( pwchNotationName ), cchNotationName );
        return S_OK;
    }
};
class SAXDeclHandler : public ISAXDeclHandler
{
public:
    virtual void elementDecl( wchar_t * name, int nameSize, wchar_t * model, int modelSize )
    {
    }
    virtual void attributeDecl( wchar_t * ename, int enameSize, wchar_t * aname, int anameSize, wchar_t * type, int typeSize, wchar_t * valueDefault, int valueDefaultSize, wchar_t * value, int valueSize )
    {
    }
    virtual void internalEntityDecl( wchar_t * name, int nameSize, wchar_t * value, int valueSize )
    {
    }
    virtual void externalEntityDecl( wchar_t * name, int nameSize, wchar_t * publicId, int publicIdSize, wchar_t * systemId, int systemIdSize )
    {
    }
public:
    virtual HRESULT __stdcall raw_elementDecl ( unsigned short * pwchName, int cchName, unsigned short * pwchModel, int cchModel )
    {
        elementDecl( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName,
            reinterpret_cast<wchar_t *>( pwchModel ), cchModel );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_attributeDecl ( unsigned short * pwchElementName, int cchElementName, unsigned short * pwchAttributeName, int cchAttributeName, unsigned short * pwchType, int cchType, unsigned short * pwchValueDefault, int cchValueDefault, unsigned short * pwchValue, int cchValue )
    {
        attributeDecl( 
            reinterpret_cast<wchar_t *>( pwchElementName ), cchElementName,
            reinterpret_cast<wchar_t *>( pwchAttributeName ), cchAttributeName,
            reinterpret_cast<wchar_t *>( pwchType ), cchType,
            reinterpret_cast<wchar_t *>( pwchValueDefault ), cchValueDefault,
            reinterpret_cast<wchar_t *>( pwchValue ), cchValue );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_internalEntityDecl ( unsigned short * pwchName, int cchName, unsigned short * pwchValue, int cchValue )
    {
        internalEntityDecl( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName,
            reinterpret_cast<wchar_t *>( pwchValue ), cchValue );
        return S_OK;
    }
    virtual HRESULT __stdcall raw_externalEntityDecl ( unsigned short * pwchName, int cchName, unsigned short * pwchPublicId, int cchPublicId, unsigned short * pwchSystemId, int cchSystemId )
    {
        externalEntityDecl( 
            reinterpret_cast<wchar_t *>( pwchName ), cchName,
            reinterpret_cast<wchar_t *>( pwchPublicId ), cchPublicId,
            reinterpret_cast<wchar_t *>( pwchSystemId ), cchSystemId );
        return S_OK;
    }
};
class SAXDefaultHandler : public SAXContentHandler, public SAXErrorHandler, public SAXLexicalHandler, public SAXDTDHandler, public SAXDeclHandler
{
    LONG _refcount;
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID iid,void __RPC_FAR *__RPC_FAR *ppvObject)
	{
        if ( iid == __uuidof(ISAXContentHandler) )
        {
            *ppvObject = static_cast<ISAXContentHandler*>(this);
            AddRef();
            return S_OK;
        }
        if ( iid == __uuidof(ISAXErrorHandler) )
        {
            *ppvObject = static_cast<ISAXErrorHandler*>(this);
            AddRef();
            return S_OK;
        }
        if ( iid == __uuidof(ISAXLexicalHandler) )
        {
            *ppvObject = static_cast<ISAXLexicalHandler*>(this);
            AddRef();
            return S_OK;
        }
        if ( iid == __uuidof(ISAXDTDHandler) )
        {
            *ppvObject = static_cast<ISAXDTDHandler*>(this);
            AddRef();
            return S_OK;
        }
        if ( iid == __uuidof(ISAXDeclHandler) )
        {
            *ppvObject = static_cast<ISAXDeclHandler*>(this);
            AddRef();
            return S_OK;
        }
        if ( iid == __uuidof(IUnknown) )
        {
            *ppvObject = static_cast<ISAXContentHandler*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE; 
    }
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
        return (ULONG)InterlockedIncrement(&_refcount); 
    }
	virtual ULONG STDMETHODCALLTYPE Release()
	{
        ULONG res = (ULONG) InterlockedDecrement(&_refcount);
        if (res == 0) 
            delete this;
        return res;
    }
    SAXDefaultHandler()
    {
        _refcount = 1;
    }
};
class SAXXMLReader
{
    ISAXXMLReaderPtr reader;
    SAXDefaultHandler * handler;
public:
    SAXXMLReader( SAXDefaultHandler * handler_ ) : handler( handler_ )
    {
        try {
            reader.CreateInstance( MSXML2::CLSID_SAXXMLReader60 );
	        reader->putContentHandler( handler );
	        //reader->putEntityResolver( handler );
	        reader->putDTDHandler( handler );
	        reader->putErrorHandler( handler );
            //reader->putFeature( reinterpret_cast<unsigned short*>( const_cast<wchar_t*>( L"http://xml.org/sax/properties/lexical-handler " ) ), handler );
            //reader->putFeature( reinterpret_cast<unsigned short*>( const_cast<wchar_t*>( L"http://xml.org/sax/properties/declaration-handler " ) ), handler );
        } catch (_com_error& ) {
        } catch (...) {
        }
    }
    bool parse( const char * data )
    {
        try {
            reader->parse( data );
        } catch (_com_error&) {
            return false;
        }
        return true;
    }
    bool parsePath( const char * path )
    {
        std::string url( path );
        for ( std::string::iterator it = url.begin(); it != url.end(); ++it ) {
            if ( *it == '\\' ) {
                *it = '/';
            }
        }
        if ( url.find( ':' ) != std::string::npos ) {//â‘ÎƒpƒX.
            std::string tmp( "file:///" );
            tmp += url;
            url = tmp;
        }
        return parseURL( url.c_str() );
    }
    bool parseURL( const char * url )
    {
        try {
            std::wstring wurl = string2wstring( string( url ) );
            reader->parseURL( reinterpret_cast<unsigned short*>( const_cast<wchar_t*>( wurl.c_str() ) ) );
        } catch (_com_error&) {
            return false;
        }
        return true;
    }
};
class FileStream : public IStream
{
public:
    static bool OpenFile(const char * name, IStream ** ppStream, bool fWrite)
    {
        HANDLE hFile = ::CreateFile(name, fWrite ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ,
            NULL, fWrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return false;
        *ppStream = new FileStream(hFile);
        if(*ppStream == NULL) {
            CloseHandle(hFile);
            return false;   
        }
        return true;
    }
private:
    FileStream(HANDLE hFile) 
    {
        _refcount = 1;
        _hFile = hFile;
    }
    ~FileStream()
    {
        if (_hFile != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(_hFile);
        }
    }
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
    { 
        if (iid == __uuidof(IUnknown)
            || iid == __uuidof(IStream)
            || iid == __uuidof(ISequentialStream))
        {
            *ppvObject = static_cast<IStream*>(this);
            AddRef();
            return S_OK;
        } else
            return E_NOINTERFACE; 
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void) 
    { 
        return (ULONG)InterlockedIncrement(&_refcount); 
    }
    virtual ULONG STDMETHODCALLTYPE Release(void) 
    {
        ULONG res = (ULONG) InterlockedDecrement(&_refcount);
        if (res == 0) 
            delete this;
        return res;
    }
    // ISequentialStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
    {
        BOOL rc = ReadFile(_hFile, pv, cb, pcbRead, NULL);
        return (rc) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten)
    {
        BOOL rc = WriteFile(_hFile, pv, cb, pcbWritten, NULL);
        return rc ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }

    // IStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER)
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*,
        ULARGE_INTEGER*) 
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD)                                      
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void)                                       
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **)                                  
    { 
        return E_NOTIMPL;   
    }

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin,
        ULARGE_INTEGER* lpNewFilePointer)
    { 
        DWORD dwMoveMethod;

        switch(dwOrigin)
        {
        case STREAM_SEEK_SET:
            dwMoveMethod = FILE_BEGIN;
            break;
        case STREAM_SEEK_CUR:
            dwMoveMethod = FILE_CURRENT;
            break;
        case STREAM_SEEK_END:
            dwMoveMethod = FILE_END;
            break;
        default:   
            return STG_E_INVALIDFUNCTION;
            break;
        }

        if (SetFilePointerEx(_hFile, liDistanceToMove, (PLARGE_INTEGER) lpNewFilePointer,
                             dwMoveMethod) == 0)
            return HRESULT_FROM_WIN32(GetLastError());
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg, DWORD grfStatFlag) 
    {
        if (GetFileSizeEx(_hFile, (PLARGE_INTEGER) &pStatstg->cbSize) == 0)
            return HRESULT_FROM_WIN32(GetLastError());
        return S_OK;
    }

private:
    HANDLE _hFile;
    LONG _refcount;
};
class SAXXMLWriter
{
    IMXWriterPtr writer;
    ISAXContentHandlerPtr handler;
    CComPtr<FileStream> stream;
    IMXAttributesPtr mxAttrs;
    ISAXAttributesPtr attrs;
public:
    SAXXMLWriter( const char * filename )
    {
        try {
            writer.CreateInstance(MSXML2::CLSID_MXXMLWriter60);
            writer->encoding = _bstr_t( "shift_jis" );
            writer->indent = VARIANT_TRUE;
            writer->standalone = VARIANT_TRUE;
            writer->byteOrderMark = VARIANT_FALSE;
            if ( ! FileStream::OpenFile( filename, reinterpret_cast<IStream**>( & stream ), true ) ) return;
            writer->output = stream.p;
            handler = writer;
            mxAttrs.CreateInstance(MSXML2::CLSID_SAXAttributes);
            attrs = mxAttrs;
        } catch (...) {
        }
    }
    void startDocument()
    {
        handler->startDocument();
    }
    void endDocument()
    {
        handler->endDocument();
    }
    void startElement( const char * qname )
    {
        std::wstring wqname = string2wstring( string( qname ) );
        handler->startElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size(), 
            0 );
    }
    void endElement( const char * qname )
    {
        std::wstring wqname = string2wstring( string( qname ) );
        handler->endElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size() );
    }
    void startElement( const char * qname, const std::map<string,string> & attributes )
    {
        std::wstring wqname = string2wstring( string( qname ) );
        mxAttrs->clear();
        for ( std::map<string,string>::const_iterator it = attributes.begin(), sentinel = attributes.end(); it != sentinel; ++it ) {
            mxAttrs->addAttribute( "", "", it->first.chars(), "", it->second.chars() );
        }
        handler->startElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size(), 
            attrs );
    }
    void characters( const char * text )
    {
        std::wstring wtext = string2wstring( string( text ) );
        handler->characters( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wtext.c_str() ) ), wtext.size() ); 
    }

    void element( const char * qname )
    {
        std::wstring wqname = string2wstring( string( qname ) );
        handler->startElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size(), 
            0 );
        handler->endElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size() );
    }

    void element( const char * qname, const char * text )
    {
        std::wstring wqname = string2wstring( string( qname ) );
        handler->startElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size(), 
            0 );
        characters( text );
        handler->endElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size() );
    }
    void element( const char * qname, const std::map<string,string> & attributes )
    {
        std::wstring wqname = string2wstring( string( qname ) );
        mxAttrs->clear();
        for ( std::map<string,string>::const_iterator it = attributes.begin(), sentinel = attributes.end(); it != sentinel; ++it ) {
            mxAttrs->addAttribute( "", "", it->first.chars(), "", it->second.chars() );
        }
        handler->startElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size(), 
            attrs );
        handler->endElement( 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( L"" ) ), 0, 
            const_cast<unsigned short*>( reinterpret_cast<const unsigned short*>( wqname.c_str() ) ), wqname.size() );
    }
};
