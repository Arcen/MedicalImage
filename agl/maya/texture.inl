
////////////////////////////////////////////////////////////////////////////////
// TEXTURE
sgTexture * Maya::exportTexture( MObject & mObj )
{
	MStatus status;
	MFn::Type apiType = mObj.apiType();
	MFnDependencyNode dependNode( mObj, &status ); statusError( status, "Fail fnDepencencyNode" );
	if ( MFn::kFileTexture != apiType ) return NULL;

	string name = getName( dependNode );
	
	MString fileName;
	MPlug & fileTextureName = dependNode.findPlug( "fileTextureName", & status ); Maya::statusError( status, "Fail fileTextureName" );
	fileTextureName.getValue( fileName );

	for ( list<sgTexture*>::iterator it( textures ); it; ++it ) {
		if ( it()->url() == name ) return it();
	}

	int width, height;
	MPlug & outSize = dependNode.findPlug( "outSize", & status ); Maya::statusError( status, "Fail outSize" );
	outSize.child(0).getValue( width );
	outSize.child(1).getValue( height );

	char filename[MAX_PATH];
	strcpy( filename, fileName.asChar() );
	strrep( filename, '/', '\\' );

	MayaTextureAttribute mta;
	mta.initializeByTexture( mObj );
	int mapping = ( mta.wrapU || mta.wrapV ) ? ( mta.mirror ? sgTexture::mirror : sgTexture::wrap ) : sgTexture::clamp;

	sgTexture * texture = NULL;
	char drive[MAX_PATH], dir[MAX_PATH], file[MAX_PATH], ext[MAX_PATH];
	_splitpath( filename, drive, dir, file, ext );
	if ( stricmp( ext, ".iff" ) == 0 ) {
		IFFimageReader iffir;
		statusError( iffir.open(filename), "Fail IFFimageReader::open" );
		statusError( iffir.readImage(), "Fail IFFimageReader::readImage" );
		bool rgb = iffir.isRGB();
		bool gray = iffir.isGrayscale();
		bool alpha = iffir.hasAlpha();
		if ( rgb || gray ) {
			int bytesPerChannel = iffir.getBytesPerChannel();
			int channelPerPixel = ( rgb ? ( alpha ? 4 : 3 ) : ( alpha ? 2 : 1 ) );
			int bytesPerPixel = bytesPerChannel * channelPerPixel;
			const byte *bitmap = iffir.getPixelMap();
			const byte *pixel = bitmap;
			if ( pixel ) {
				texture = new sgTexture( *graph );
				texture->name = name + "$texture";
				texture->url() = name;
				texture->mapping( mapping );
				textures.push_back( texture );
				image *img = & texture->texture;
				img->create( width, height );
				pixelT8 p;
				if ( bytesPerChannel == 1 ) {
					if ( rgb ) {
						if ( alpha ) {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = pixel[0];
									p.g = pixel[1];
									p.b = pixel[2];
									p.a = pixel[3];
									img->set( x, y, p );
								}
							}
						} else {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = pixel[0];
									p.g = pixel[1];
									p.b = pixel[2];
									p.a = 255;
									img->set( x, y, p );
								}
							}
						}
					} else {
						if ( alpha ) {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = p.g = p.b = pixel[0];
									p.a = pixel[1];
									img->set( x, y, p );
								}
							}
						} else {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = p.g = p.b = pixel[0];
									p.a = 255;
									img->set( x, y, p );
								}
							}
						}
					}
				} else if ( bytesPerChannel == 2 ) {
					if ( rgb ) {
						if ( alpha ) {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = pixel[0];
									p.g = pixel[2];
									p.b = pixel[4];
									p.a = pixel[6];
									img->set( x, y, p );
								}
							}
						} else {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = pixel[0];
									p.g = pixel[2];
									p.b = pixel[4];
									p.a = 255;
									img->set( x, y, p );
								}
							}
						}
					} else {
						if ( alpha ) {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = p.g = p.b = pixel[0];
									p.a = pixel[2];
									img->set( x, y, p );
								}
							}
						} else {
							for ( int y = 0; y < height; y++ ) {
								for ( int x = 0; x < width; x++, pixel += bytesPerPixel ) {
									p.r = p.g = p.b = pixel[0];
									p.a = 255;
									img->set( x, y, p );
								}
							}
						}
					}
				}
			}
		}
		iffir.close();
	} else {
		texture = new sgTexture( *graph );
		texture->name = name + "$texture";
		texture->url( name );
		texture->mapping( mapping );
		textures.push_back( texture );
		image *img = & texture->texture;
		if ( ! img->load( filename ) ) {
			delete texture;
			texture = NULL;
		}
	}
	return texture;
}
