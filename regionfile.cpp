#include "main.h"
#include "sax.h"
#include "parallel_for.h"
extern int thread_count;

static void outputRegion( SAXXMLWriter & writer, const ctRegion & rgn )
{
    std::map<string,string> rectAttr;
    string & left = rectAttr[string("left")];
    string & right = rectAttr[string("right")];
    string & top = rectAttr[string("top")];
    string & bottom = rectAttr[string("bottom")];
    for ( ctRegion::rectangle_const_enumrator it = rgn.range(); it; ++it ) {
        const ctRegion::rectangle & r = *it;
        left = string( r.left );
        right = string( r.right );
        top = string( r.top );
        bottom = string( r.bottom );
        writer.element( "rectangle", rectAttr );
    }
}

//形状データのXMLからの保存と読み込み
void regionIO::save( const ctSlices & slices, const char * filename )
{
#if defined( USE_DOM )
	domDocument->appendChild( domDocument->createProcessingInstruction( "xml", "version='1.0' encoding='Shift_JIS'" ) );
	IXMLDOMElementPtr domRoot = createChildElement( domDocument, "regions" );

	IXMLDOMElementPtr calibrationNode = createChildElement( domRoot, "calibration" );
	IXMLDOMElementPtr cairNode = createChildElement( calibrationNode, "air" );
	setAttribute( cairNode, "value", string( slices.air ) );
	setAttribute( cairNode, "valid", string( slices.airValid ? 1 : 0 ) );
	IXMLDOMElementPtr cwaterNode = createChildElement( calibrationNode, "water" );
	setAttribute( cwaterNode, "value", string( slices.water ) );
	setAttribute( cwaterNode, "valid", string( slices.waterValid ? 1 : 0 ) );

	IXMLDOMElementPtr positionNode = createChildElement( domRoot, "position" );
	IXMLDOMElementPtr ptopNode = createChildElement( positionNode, "top" );
	setAttribute( ptopNode, "value", string( slices.numberTop ) );
	IXMLDOMElementPtr pbottomNode = createChildElement( positionNode, "bottom" );
	setAttribute( pbottomNode, "value", string( slices.numberBottom ) );
	IXMLDOMElementPtr pcenterNode = createChildElement( positionNode, "center" );
	setAttribute( pcenterNode, "value", string( slices.numberCenter ) );
	setAttribute( pcenterNode, "x", string( slices.centerposition.x ) );
	setAttribute( pcenterNode, "y", string( slices.centerposition.y ) );
	setAttribute( createChildElement( positionNode, "prone" ), "value", slices.prone ? "1" : "0" );
	setAttribute( createChildElement( positionNode, "bottomFirst" ), "value", slices.bottomFirst ? "1" : "0" );

	statusBar->reset();
	statusBar->expand( slices.slices.size );
	miDocument * doc = miDocument::get();
	for ( int i = 0; i < slices.slices.size; ++i ) {
		const ctSlice & slice = slices.slices[i];
		IXMLDOMElementPtr sliceNode = createChildElement( domRoot, "slice" );
		for ( int j = 0; j < doc->numberRegion(); ++j ) {
			IXMLDOMElementPtr rgnNode = createChildElement( sliceNode, doc->regionName( j ) );
			regionIO::setRegion( rgnNode, slice.regions[j] );
		}
		statusBar->progress();
	}
	statusBar->reset();
	try {
		domDocument->save( _bstr_t( filename ) );
	} catch (...) {
		//セーブエラーはメッセージを表示するのみとする
	}
#else
    SAXXMLWriter writer( filename );
    writer.startDocument();
    writer.startElement( "regions" );
    {
        writer.startElement( "calibration" );
        {
            {
                std::map<string,string> attr;
                attr[string("value")] = string( slices.air );
                attr[string("valid")] = string( slices.airValid ? 1 : 0 );
                writer.element( "air", attr );
            }
            {
                std::map<string,string> attr;
                attr[string("value")] = string( slices.water );
                attr[string("valid")] = string( slices.waterValid ? 1 : 0 );
                writer.element( "water", attr );
            }
        }
        writer.endElement( "calibration" );
        writer.startElement( "position" );
        {
            {
                std::map<string,string> attr;
                attr[string("value")] = string( slices.numberTop );
                writer.element( "top", attr );
            }
            {
                std::map<string,string> attr;
                attr[string("value")] = string( slices.numberBottom );
                writer.element( "bottom", attr );
            }
            {
                std::map<string,string> attr;
                attr[string("value")] = string( slices.numberCenter );
                attr[string("x")] = string( slices.centerposition.x );
                attr[string("y")] = string( slices.centerposition.y );
                writer.element( "center", attr );
            }
            {
                std::map<string,string> attr;
                attr[string("value")] = string( slices.prone ? 1 : 0 );
                writer.element( "prone", attr );
            }
            {
                std::map<string,string> attr;
                attr[string("value")] = string( slices.bottomFirst ? 1 : 0 );
                writer.element( "bottomFirst", attr );
            }
        }
        writer.endElement( "position" );
	    statusBar->reset();
	    statusBar->expand( slices.slices.size );
	    miDocument * doc = miDocument::get();
	    for ( int i = 0; i < slices.slices.size; ++i ) {
		    const ctSlice & slice = slices.slices[i];
            writer.startElement( "slice" );
		    for ( int j = 0; j < doc->numberRegion(); ++j ) {
                std::string regName = doc->regionName( j );
                writer.startElement( regName.c_str() );
                outputRegion( writer, slice.regions[j] );
                writer.endElement( regName.c_str() );
		    }
            writer.endElement( "slice" );
		    statusBar->progress();
	    }
	    statusBar->reset();
    }
    writer.endElement( "regions" );
    writer.endDocument();
#endif
}
class regionHandler : public SAXDefaultHandler
{
public:
protected:
    bool parseText;
private:
    std::wstring text;
    //std::string textMb;
protected:
    std::wstring left, right, top, bottom;
    regionHandler() : parseText( false )
    {
        text.reserve( 1024 );
        left = L"left";
        right = L"right";
        top = L"top";
        bottom = L"bottom";
    }
    static int getIntAttribute( SAXAttributes & attributes, std::wstring & aname )
    {
        wchar_t * value = 0;
        int valueSize = 0;
        if ( attributes.getValueFromQName( const_cast<wchar_t*>( aname.c_str() ), aname.size(), value, valueSize ) )
        {
            return wstring2string( std::wstring( value, valueSize ) ).toi();
        }
        return 0;
    }
    virtual void characters( wchar_t * chars, int charsSize )
    {
        if ( parseText ) {
            text += std::wstring( chars, charsSize );
            //textMb += std::string( reinterpret_cast<char*>( chars ), charsSize );
        }
    }
    virtual void ignorableWhitespace( wchar_t * chars, int charsSize )
    {
        if ( parseText ) {
            text += std::wstring( chars, charsSize );
            //textMb += std::string( reinterpret_cast<char*>( chars ), charsSize );
        }
    }
protected:
    string getText()
    {
        string result = wstring2string( text );
        //string resultAsUtf8 = wstring2string( utf82wstring( string( textMb.c_str() ) ) );
        text.clear();
        //textMb.clear();
        //if ( result.length() < resultAsUtf8.length() ) result = resultAsUtf8;
        return result;
    }
    //virtual bool error( const SAXLocator & locator, wchar_t * message, int code )
    //{
    //    int column = locator.getColumnNumber();
    //    int line = locator.getLineNumber();
    //    return true;
    //}
    //virtual bool fatalError( const SAXLocator & locator, wchar_t * message, int code )
    //{
    //    int column = locator.getColumnNumber();
    //    int line = locator.getLineNumber();
    //    return true;
    //}
    //virtual bool ignorableWarning( const SAXLocator & locator, wchar_t * message, int code )
    //{
    //    int column = locator.getColumnNumber();
    //    int line = locator.getLineNumber();
    //    return true;
    //}
};

class slicesHandler : public regionHandler
{
    ctSlices * slices;
    ctSlice * slice;
	ctRegion * target;
	ctRegion * store;
	ctRegion region;
    std::wstring regionName;
    std::wstring calibration, air, water, center, prone, bottomFirst, value, valid, x, y, sliceTag;
    int index;
    bool foundNormalRgnNode;
public:
    std::vector<std::vector<ctRegion> > regions;
    slicesHandler( ctSlices & slices_ ) : slices( & slices_ ), target( 0 ), slice( 0 )
    {
        calibration = L"calibration";
        air = L"air";
        water = L"water";
        center = L"center";
        prone = L"prone";
        bottomFirst = L"bottomFirst";
        value = L"value";
        valid = L"valid";
        x = L"x";
        y = L"y";
        sliceTag = L"slice";
        foundNormalRgnNode = false;
        index = 0;
        //読み込み形状を保存.
        std::vector<ctRegion> rgns( miDocument::get()->numberRegion() + 1 );
        regions.resize( slices->slices.size, rgns );
    }
    virtual void startDocument()
    {
    }
    static void processFinalize( void * data, int index )
    {
        slicesHandler * self = ( slicesHandler * ) data;
        int numRegion = miDocument::get()->numberRegion();
        ctSlice & slice = self->slices->slices[index];
		miDocument * doc = miDocument::get();
		if ( ( ! slice.imageSize.x || ! slice.imageSize.y ) && doc && ! doc->slices.slices.empty() )
		{
			slice.imageSize = doc->slices.slices.first().imageSize;
		}
		if ( ! slice.imageSize.x || ! slice.imageSize.y )
		{
			slice.imageSize.x = 512;
			slice.imageSize.y = 512;
		}
        std::vector<char> flag( slice.imageSize.x * slice.imageSize.y, 0 );
	    for ( int j = 0; j < numRegion; ++j ) {
            ctRegion & rgn = self->regions[index][j];
			//形状がおかしい可能性があるので、設定しなおす.
            if ( rgn.empty() ) {
                rgn.release();
            } else {
                ctRegion::rectangle bbox = rgn.get();
                rgn.get( flag, bbox.width(), bbox.height(), bbox.left, bbox.top );
                rgn.set( flag, bbox.width(), bbox.height(), bbox.left, bbox.top );
            }
            slice.setRegion( j, rgn );
            rgn.release();
        }
	    slice.adjustRegion();
	    miDocument::get()->progress();
    }
    virtual void endDocument()
    {
        statusBar->reset();
        statusBar->expand( slices->slices.size );
        parallel_for pf( mainWindow, thread_count, 0, slices->slices.size, slicesHandler::processFinalize, this );
        pf.begin();
        pf.join();
        statusBar->reset();
    }
    int getRegionIndex( const string & name )
    {
        int numRegion = miDocument::get()->numberRegion();
	    for ( int j = 0; j < numRegion; ++j ) {
		    if ( miDocument::get()->regionName( j ) == name ) return j;
        }
        return numRegion;
    }
    virtual void startElement( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * qname, int qnameSize, SAXAttributes & attributes )
    {
        if ( slice && target )
        {
            target->push_back( ctRegion::rectangle( 
                getIntAttribute( attributes, left ), 
                getIntAttribute( attributes, top ), 
                getIntAttribute( attributes, right ), 
                getIntAttribute( attributes, bottom ) ) );
        } else if ( slice ) {
            std::wstring wqname( qname, qnameSize );
			string regionTag = wstring2string( wqname );
			////昔の名前のalveolusがあり，normalより先に出てきた場合にはnormalとして取り扱う．この事はバグの温床になるかもしれない
		 //   if ( regionTag == "alveolus" && ! foundNormalRgnNode ) {
			//    regionTag = "normal";
		 //   } else if ( regionTag == "normal" ) {
			//    foundNormalRgnNode = true;
		 //   }
            regionName = wqname;
            target = & region;
            store = & regions[index][getRegionIndex( regionTag )];
            target->reserve( slice->imageSize.x * slice->imageSize.y );
        } else {
            std::wstring wqname( qname, qnameSize );
            if ( wqname.compare( air ) == 0 ) {
				slices->air = getIntAttribute( attributes, value );
				slices->airValid = getIntAttribute( attributes, valid ) ? true : false;
            } else if ( wqname.compare( water ) == 0 ) {
				slices->water = getIntAttribute( attributes, value );
				slices->waterValid = getIntAttribute( attributes, valid ) ? true : false;
            } else if ( wqname.compare( top ) == 0 ) {
				slices->numberTop = getIntAttribute( attributes, value );
            } else if ( wqname.compare( bottom ) == 0 ) {
				slices->numberBottom = getIntAttribute( attributes, value );
            } else if ( wqname.compare( center ) == 0 ) {
				slices->numberCenter = getIntAttribute( attributes, value );
				slices->centerposition.x = getIntAttribute( attributes, x );
				slices->centerposition.y = getIntAttribute( attributes, y );
            } else if ( wqname.compare( prone ) == 0 ) {
				slices->prone = getIntAttribute( attributes, value ) ? true : false;
            } else if ( wqname.compare( bottomFirst ) == 0 ) {
				slices->bottomFirst = getIntAttribute( attributes, value ) ? true : false;
            } else if ( wqname.compare( sliceTag ) == 0 ) {
			    if ( index < slices->slices.size ) {
                    foundNormalRgnNode = false;
				    slice = & slices->slices[index];
                    //正解読み込みの場合など、画像サイズが初期化されていないので、画像取得する.
                    if ( ! slice->imageSize.x || ! slice->imageSize.y ) {
	                    miDocument * doc = miDocument::get();
                        if ( doc && & doc->slices != slices ) {
                            if ( ! doc->slices.slices.empty() ) {
                                slice->imageSize = doc->slices.slices.first().imageSize;
                            }
                        }
                    }
                    if ( ! slice->imageSize.x || ! slice->imageSize.y ) {
                        slice->imageSize.x = slice->imageSize.y = 512;
                    }
				    slice->initialize();
                } else {
                    slice = 0;
                }
            }
        }
    }
    virtual void endElement( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * qname, int qnameSize )
    {
        std::wstring wqname( qname, qnameSize );
        if ( wqname.compare( calibration ) == 0 ) {
			slices->calibration( slices->air, slices->water, slices->airValid, slices->waterValid );
        }
        if ( slice ) {
            if ( slice && wqname.compare( sliceTag ) == 0 ) {
                slice = 0;
	            miDocument::get()->progress();
                ++index;
		    }
            if ( slice && target && regionName.compare( wqname ) == 0 ) {
#if defined(_DEBUG) || defined(DEBUG)
                int left = target->get().left, top = target->get().top;
                int width = target->get().width(), height = target->get().height();
                if ( width && height ) {
		            array2<bool> flag;
                    flag.allocate( width, height );
			        target->get( flag, left, top );
			        store->set( flag, left, top );
                    array<ctRegion::rectangle>::iterator its( store->rectangleIterator() );
                    array<ctRegion::rectangle>::iterator itt( target->rectangleIterator() );
                    bool mismatch = false;
                    while ( its && itt ) {
                        if ( its() != itt() ) {
                            mismatch = true;
                        }
                        ++its;
                        ++itt;
                    }
                    if ( its || itt ) {
                        mismatch = true;
                    }
                }
#endif
                *store = *target;
                target = 0;
            }
        }
    }
};

bool regionIO::load( ctSlices & slices, const char * filename )
{
#if defined( USE_DOM )
	domDocument->validateOnParse = VARIANT_FALSE;
	if ( ! domDocument->load( _bstr_t( filename ) ) ) return false;
	IXMLDOMElementPtr domRoot = domDocument->documentElement;
	if ( string( "regions" ) != ( const char * ) domRoot->nodeName ) return false;
	statusBar->reset();
	statusBar->expand( slices.slices.size );
	slices.uncalibrate();
	int index = 0;
	for ( IXMLDOMElementPtr node = domRoot->firstChild; node; node = node->nextSibling ) {
		if ( string( "calibration" ) == ( const char * ) node->nodeName ) {
			for ( IXMLDOMElementPtr child = node->firstChild; child; child = child->nextSibling ) {
				if ( string( "air" ) == ( const char * ) child->nodeName ) {
					slices.air = getAttribute( child, "value" ).toi();
					slices.airValid = getAttribute( child, "valid" ).toi() ? true : false;
				} else if ( string( "water" ) == ( const char * ) child->nodeName ) {
					slices.water = getAttribute( child, "value" ).toi();
					slices.waterValid = getAttribute( child, "valid" ).toi() ? true : false;
				}
			}
			slices.calibration( slices.air, slices.water, slices.airValid, slices.waterValid );
		} else if ( string( "position" ) == ( const char * ) node->nodeName ) {
			for ( IXMLDOMElementPtr child = node->firstChild; child; child = child->nextSibling ) {
				if ( string( "top" ) == ( const char * ) child->nodeName ) {
					slices.numberTop = getAttribute( child, "value" ).toi();
				} else if ( string( "bottom" ) == ( const char * ) child->nodeName ) {
					slices.numberBottom = getAttribute( child, "value" ).toi();
				} else if ( string( "center" ) == ( const char * ) child->nodeName ) {
					slices.numberCenter = getAttribute( child, "value" ).toi();
					slices.centerposition.x = getAttribute( child, "x" ).toi();
					slices.centerposition.y = getAttribute( child, "y" ).toi();
				} else if ( string( "prone" ) == ( const char * ) child->nodeName ) {
					slices.prone = getAttribute( child, "value" ).toi() ? true : false;
				} else if ( string( "bottomFirst" ) == ( const char * ) child->nodeName ) {
					slices.bottomFirst = getAttribute( child, "value" ).toi() ? true : false;
				}
			}
		} else if ( string( "slice" ) == ( const char * ) node->nodeName ) {
			if ( index < slices.slices.size ) {
				ctSlice & slice = slices.slices[index++];
				slice.initialize();
				bool foundNormalRgnNode = false;
				for ( IXMLDOMElementPtr rgnNode = node->firstChild; rgnNode; rgnNode = rgnNode->nextSibling ) {
					for ( int j = 0; j < miDocument::get()->numberRegion(); ++j ) {
						string nodeName = ( const char * ) rgnNode->nodeName;
						////昔の名前のalveolusがあり，normalより先に出てきた場合にはnormalとして取り扱う．この事はバグの温床になるかもしれない
						//if ( nodeName == "alveolus" && ! foundNormalRgnNode ) {
						//	nodeName = "normal";
						//} else if ( nodeName == "normal" ) {
						//	foundNormalRgnNode = true;
						//}
						if ( miDocument::get()->regionName( j ) != nodeName ) continue;
						ctRegion wk;
						regionIO::getRegion( rgnNode, wk );
						//形状がおかしい可能性があるので、設定しなおす.
						//array2<bool> flag;
						//flag.allocate( 512, 512 );
						//wk.get( flag, 0, 0 );
						//wk.set( flag, 0, 0 );
						slice.setRegion( j, wk );
						break;
					}
				}
				slice.adjustRegion();
				statusBar->progress();
			}
		}
	}
	statusBar->reset();
	return true;
#else
    slices.uncalibrate();
    statusBar->reset();
    statusBar->expand( slices.slices.size );
    bool result;
    {
        slicesHandler handler( slices );
        {

            SAXXMLReader reader( & handler );
            result = reader.parsePath( filename );
        }
    }
	statusBar->reset();
    return result;
#endif
}

bool regionIO::loadLabel( ctSlices & slices, const char * filename )
{
	miDocument * doc = miDocument::get();
    if ( ! doc ) return false;
    slices.uncalibrate();
    statusBar->reset();
    statusBar->expand( slices.slices.size );
    int numRegion = doc->numberRegion();
	retainer<file> f = file::initialize( filename, false, true );
	if ( ! f ) return false;
    const int width = slices.slices.first().imageSize.x;
    const int height = slices.slices.first().imageSize.y;
    const int depth = slices.slices.size;
    if ( width * height * depth + 16 != f->getSize() ) return false;
    f->seek( -16, SEEK_END );
    int w, h, d, b;
    if ( ! f->read( w ) || ! f->read( h ) || ! f->read( d ) || ! f->read( b ) ) return false;
    if ( w != width || h != height || d != depth || b != 1 ) return false;
    struct loader
    {
        ctSlices * slices;
        std::vector<std::vector<ctRegion> > regions;
        loader( ctSlices * slices_, int size, int numRegion ) : slices( slices_ ), regions( size, std::vector<ctRegion>( numRegion ) )
        {
        }
        static void process( void * data, int index )
        {
            loader * self = ( loader * ) data;
            ctSlices * slices = self->slices;
            std::vector<ctRegion> & regions = self->regions[index];
	        miDocument * doc = miDocument::get();
            int numRegion = doc->numberRegion();
            ctSlice & slice = slices->slices[index];
	        if ( ( ! slice.imageSize.x || ! slice.imageSize.y ) && doc && ! doc->slices.slices.empty() )
	        {
		        slice.imageSize = doc->slices.slices.first().imageSize;
	        }
	        if ( ! slice.imageSize.x || ! slice.imageSize.y )
	        {
		        slice.imageSize.x = 512;
		        slice.imageSize.y = 512;
	        }
            //領域の連結
            for ( int j = 0; j < numRegion; ++j ) {
                ctRegion & rgn = regions[j];
                rgn.connect();
            }
            //肺の補間
            for ( int j = 2; j < numRegion; ++j ) {
                regions[1] |= regions[j];
            }
            //人体の補間
            regions[0] |= regions[1];
            //領域の設定.
            for ( int j = 0; j < numRegion; ++j ) {
                slice.setRegion( j, regions[j] );
                regions[j].release();
            }
            regions.clear();
            slice.adjustRegion();
            miDocument::get()->progress();
        }
    } data( & slices, depth, numRegion );
    f->seek( 0, SEEK_SET );
    char wk;
    for ( int z = 0; z < depth; ++z ) {
        std::vector<ctRegion> & rgns = data.regions[z];
	    for ( int y = 0; y < height; ++y ) {
		    for ( int x = 0; x < width; ++x ) {
			    f->read( wk );
                if ( wk < 0 ) {//範囲外.
                } else if ( wk < numRegion ) {
                    rgns[wk].push_back( x, y );
			    }
		    }
	    }
	}
    {
        parallel_for pf( mainWindow, thread_count, 0, depth, loader::process, & data );
        pf.begin();
        pf.join();
    }
	statusBar->reset();
    return true;
}
#if 0
void regionIO::save( const ctRegion & rgn, const char * filename )
{
#if defined( USE_DOM )
	domDocument->appendChild( domDocument->createProcessingInstruction( "xml", "version='1.0' encoding='Shift_JIS'" ) );
	IXMLDOMElementPtr domRoot = createChildElement( domDocument, "region" );
	setRegion( domRoot, rgn );
	try {
		domDocument->save( _bstr_t( filename ) );
	} catch (...) {
		//セーブエラーはメッセージを表示するのみとする
	}
#else
    SAXXMLWriter writer( filename );
    writer.startDocument();
    writer.startElement( "region" );
    {
        std::map<string,string> rectAttr;
        string & left = rectAttr[string("left")];
        string & right = rectAttr[string("right")];
        string & top = rectAttr[string("top")];
        string & bottom = rectAttr[string("bottom")];
		for ( array<ctRegion::rectangle>::iterator it( rgn.rectangleIterator() ); it; ++it ) {
            const ctRegion::rectangle & r = it();
            left = string( r.left );
            right = string( r.right );
            top = string( r.top );
            bottom = string( r.bottom );
            writer.element( "rectangle", rectAttr );
		}
    }
    writer.endElement( "region" );
    writer.endDocument();
#endif
}
#endif

void regionIO::save( const miPattern & teacher, const char * filename )
{
#if defined( USE_DOM )
	domDocument->appendChild( domDocument->createProcessingInstruction( "xml", "version='1.0' encoding='Shift_JIS'" ) );
	IXMLDOMElementPtr domRoot = createChildElement( domDocument, "teacher" );

	IXMLDOMElementPtr comment = createChildElement( domRoot, "comment" );
	setText( comment, teacher.comment );
	IXMLDOMElementPtr topbottom = createChildElement( domRoot, "topbottom" );
	setText( topbottom, string( teacher.topbottom ) );
	IXMLDOMElementPtr frontrear = createChildElement( domRoot, "frontrear" );
	setText( frontrear, string( teacher.frontrear ) );
	IXMLDOMElementPtr enable = createChildElement( domRoot, "enable" );
	setText( enable, string( teacher.enable ? 1 : 0 ) );

	IXMLDOMElementPtr shape = createChildElement( domRoot, "shape" );
	setRegion( shape, teacher.shape );

	try {
		domDocument->save( _bstr_t( filename ) );
	} catch (...) {
		//セーブエラーはメッセージを表示するのみとする
	}
#else
    SAXXMLWriter writer( filename );
    writer.startDocument();
    writer.startElement( "teacher" );
    {
        if ( teacher.comment.length() ) {
            writer.element( "comment", teacher.comment.chars() );
        }
        writer.element( "topbottom", string( teacher.topbottom ).chars() );
        writer.element( "frontrear", string( teacher.frontrear ).chars() );
        writer.element( "enable", teacher.enable ? "1" : "0" );
        writer.startElement( "shape" );
        outputRegion( writer, teacher.shape );
        writer.endElement( "shape" );
    }
    writer.endElement( "teacher" );
    writer.endDocument();
#endif
}

class patternHandler : public regionHandler
{
    miPattern * teacher;
	ctRegion * target;
    bool isTeature;
    std::wstring shape, comment, topbottom, frontrear, enable, teacherTag, region;
public:
    patternHandler( miPattern & teacher_ ) : teacher( & teacher_ ), isTeature( false ), target( 0 )
    {
        initialize();
    }
    void initialize()
    {
        shape = L"shape";
        comment = L"comment";
        topbottom = L"topbottom";
        frontrear = L"frontrear";
        enable = L"enable";
        teacherTag = L"teacher";
        region = L"region";
    }
    virtual void startDocument()
    {
    }
    virtual void endDocument()
    {
    }
    virtual void startElement( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * qname, int qnameSize, SAXAttributes & attributes )
    {
        if ( target )
        {
            target->push_back( ctRegion::rectangle( 
                getIntAttribute( attributes, left ), 
                getIntAttribute( attributes, top ), 
                getIntAttribute( attributes, right ), 
                getIntAttribute( attributes, bottom ) ) );
        } else {
            std::wstring wqname( qname, qnameSize );
            if ( isTeature )
            {
                if ( wqname.compare( shape ) == 0 ) {
                    target = & teacher->shape;
                    target->reserve( teacher->ctimg.width * teacher->ctimg.height );
                } else if ( wqname.compare( comment ) == 0 ) {
                    parseText = true;
                } else if ( wqname.compare( topbottom ) == 0 ) {
                    parseText = true;
                } else if ( wqname.compare( frontrear ) == 0 ) {
                    parseText = true;
                } else if ( wqname.compare( enable ) == 0 ) {
                    parseText = true;
			    }
                getText();
            } else {
                if ( wqname.compare( teacherTag ) == 0 ) {
                    isTeature = true;
                } else if ( wqname.compare( region ) == 0 ) {
                    target = & teacher->shape;
                    target->reserve( teacher->ctimg.width * teacher->ctimg.height );
                }
            }
        }
    }
    virtual void endElement( wchar_t * uri, int uriSize, wchar_t * localname, int localnameSize, wchar_t * qname, int qnameSize )
    {
        std::wstring wqname( qname, qnameSize );
        if ( wqname.compare( shape ) == 0 || wqname.compare( region ) == 0 ) {
            if ( target )
            {
                target->connect();
            }
            target = 0;
        } else {
            if ( parseText ) {
                if ( wqname.compare( comment ) == 0 ) {
				    teacher->comment = getText();
                } else if ( wqname.compare( topbottom ) == 0 ) {
				    teacher->topbottom = clamp<int>( lungTop, getText().toi(), lungBottom );
                } else if ( wqname.compare( frontrear ) == 0 ) {
				    teacher->frontrear = clamp<int>( lungFront, getText().toi(), lungRear );
                } else if ( wqname.compare( enable ) == 0 ) {
				    teacher->enable = getText().toi() ? true : false;
			    }
                parseText = false;
            }
		}
    }
};
#if 0

void regionIO::load( ctRegion & rgn, const char * filename )
{
	rgn.initialize();
#if defined( USE_DOM )
	domDocument->validateOnParse = VARIANT_FALSE;
	if ( ! domDocument->load( _bstr_t( filename ) ) ) return;
	IXMLDOMElementPtr domRoot = domDocument->documentElement;
	if ( string( "region" ) != ( const char * ) domRoot->nodeName ) return;
	getRegion( domRoot, rgn );
#else
    patternHandler handler( rgn );
    {
        SAXXMLReader reader( & handler );
        if ( ! reader.parsePath( filename ) ) return false;
    }
    return true;
#endif
}
#endif

bool regionIO::load( miPattern & teacher, const char * filename )
{
#if defined( USE_DOM )
	domDocument->validateOnParse = VARIANT_FALSE;
	if ( ! domDocument->load( _bstr_t( filename ) ) ) return false;
	IXMLDOMElementPtr domRoot = domDocument->documentElement;
	if ( string( "teacher" ) == ( const char * ) domRoot->nodeName ) {
		for ( IXMLDOMElementPtr node = domRoot->firstChild; node; node = node->nextSibling ) {
			if ( string( "shape" ) == ( const char * ) node->nodeName ) {
				teacher.shape.initialize();
				regionIO::getRegion( node, teacher.shape );
			} else if ( string( "comment" ) == ( const char * ) node->nodeName ) {
				teacher.comment = getText( node );
			} else if ( string( "topbottom" ) == ( const char * ) node->nodeName ) {
				teacher.topbottom = clamp<int>( lungTop, getText( node ).toi(), lungBottom );
			} else if ( string( "frontrear" ) == ( const char * ) node->nodeName ) {
				teacher.frontrear = clamp<int>( lungFront, getText( node ).toi(), lungRear );
			} else if ( string( "enable" ) == ( const char * ) node->nodeName ) {
				teacher.enable = getText( node ).toi() ? true : false;
			}
		}
	} else if ( string( "region" ) == ( const char * ) domRoot->nodeName ) {
		//古い形式
		getRegion( domRoot, teacher.shape );
	} else {
		return false;
	}
	return true;
#else
    patternHandler handler( teacher );
    {
        SAXXMLReader reader( & handler );
        if ( ! reader.parsePath( filename ) ) return false;
    }
    return true;
#endif
}
