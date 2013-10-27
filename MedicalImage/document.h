//医用画像ドキュメントクラス
class miDocument
{
	static miDocument * document;
public:
	static const int progressStepForPixel = 10000;//画素単位の処理でプログレスバーを進めるときの画素数.
    critical_section cs_statusbar;
	string drive, path;
	ctSlices slices;
	const int & slicesize;
	const int & sliceindex;
	const int & regionindex;
	int patternindex;

	//領域形状の変更のみ対応する
	struct memento
	{
		array<bool> flags;//コマンドにより変更された個所を保存しておく
		ctSlices slices;
		memento(){}
		//状態を示すフラグを初期化
		void initialize()
		{
			miDocument * doc = miDocument::get();
			if ( ! doc ) return;
			flags.allocate( doc->slicesize );
			for ( array<bool>::iterator it( flags ); it; ++it ) {
				it() = false;
			}
		}
		//状態を示すフラグを変更
		void change( bool allSlices )
		{
			miDocument * doc = miDocument::get();
			if ( ! doc || flags.size != doc->slicesize ) return;
			for ( int s = 0; s < doc->slicesize; ++s ) {
				if ( ! allSlices ) { s = doc->sliceindex; }
				flags[s] = true;
				if ( ! allSlices ) break;
			}
		}
	};
	class regionType
	{
	public:
		string name;
		pixel color;
		array< retainer<miPattern> > patterns;
		regionType( const string & _name = "" ) : name( _name ) {}
	};
	array<regionType> regionTypes;
	class regionIterator
	{
		miDocument & doc;
		array<regionType>::iterator it;
	public:
		////////////////////////////////////////////////////////////////////////////////
		//生成・消滅
		regionIterator( miDocument & _doc, bool cluster = false ) 
			: doc( _doc ), it( doc.regionTypes, cluster ? regionNormal : 0 ){}
		regionIterator( const regionIterator & rit ) 
			: doc( rit.doc ), it( rit.it ){}
		regionIterator & operator=( const regionIterator & src )
		{
			it = src.it;
			return *this;
		}
		int index(){ return doc.regionTypes.index( it ); }
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator regionType * () { return it; }
		operator const regionType * () const { return it; }
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		regionType & operator()()				{ return it(); }
		const regionType & operator()() const	{ return it(); }
		////////////////////////////////////////////////////////////////////////////////
		//前へ
		regionIterator & operator--() { --it; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		regionIterator & operator++() { ++it; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		regionType & operator*() { return ( * this )(); } const regionType & operator*() const { return ( * this )(); }
		regionType * operator->() { return & ( * this )(); } const regionType * operator->() const { return & ( * this )(); }
		regionIterator operator++(int) { regionIterator wk(*this); ++(*this); return wk; }
		regionIterator operator--(int) { regionIterator wk(*this); --(*this); return wk; }
	};

	miDocument() : slicesize( slices.slices.size ), 
		sliceindex( slices.index ), 
		regionindex( controlWindow->targeted ),
		patternindex( 0 )
	{
		//領域の種類の初期設定
		regionTypes.reserve( 32 );
		regionTypes.push_back( regionType( "human" ) );
		regionTypes.push_back( regionType( "lung" ) );
		regionTypes.push_back( regionType( "normal" ) );
	}

	static miDocument * get();
	static void set( miDocument * doc );
	static miDocument * create();
	static void release();
	static void open( const char * indexed_path = NULL );
	static void close();
	bool load();
	static void save();
	static void saveSampleHistogram();
	static void loadAsRgn();
	static void saveAsRgn( const char * filename = NULL );
	static void loadPattern( string drive, string path, bool clear, bool update );
	static void savePattern( string drive, string path, bool clear );
	static void savePatternSetting();
	static void deletePattern( const string & drive, const string & path );
	static void clearPattern();
	void loadLabel();
	ctSlice & slice();
	int numberRegion() const { return regionTypes.size; }
	miPattern * pattern( int r_index = -1, int p_index = -1 )
	{
		array< retainer<miPattern> > & ps = patterns( r_index );
		if ( ! ps.size ) return NULL;
		if ( ! between( 0, patternindex, ps.size - 1 ) ) {
			patternindex = 0;
		}
		return & ps[ between( 0, p_index, ps.size - 1 ) ? p_index : patternindex ]();
	}
	void registerRegion( const string & name );
	array< retainer<miPattern> > & patterns( int r_index = -1 )
	{
		regionType & rt = regionTypes[between( 0, r_index, regionTypes.size - 1 ) ? r_index : regionindex];
		return rt.patterns;
	}
	string & regionName( int r_index = -1 )
	{
		regionType & rt = regionTypes[between( 0, r_index, regionTypes.size - 1 ) ? r_index : regionindex];
		return rt.name;
	}
	pixel & regionColor( int r_index = -1 )
	{
		regionType & rt = regionTypes[between( 0, r_index, regionTypes.size - 1 ) ? r_index : regionindex];
		return rt.color;
	}
	void exportPatterns();
	void change( bool force = false );
	void update( int16 windowSize, int16 windowLevel );
	bool divideRegionThreshhold( bool allSlices = false );
	bool divideRegionBySmoothedDifference( bool allSlices = false, int nearRegionRadiusStart = 4 );
	bool divideRegionByKolmogorovSmirnov( bool allSlices = false, int nearRegionRadiusStart = 4 );
	bool divideRegionByGaussianHistogramCorrelation( bool allSlices, const array<stepParameter> & sp, bool normalized, bool edge );
	void removeSmallLungRegion();
	void recognitionNetlike();

	void analysis( const char * in_filename = NULL, const char * out_filename = NULL, bool enableAbbrivation = false );
	void analysis( ctSlices & srcslices, const char * out_filename = NULL, bool enableAbbrivation = false );
	void displayAverageVariance();
	void displayAverageRegion( ctRegion & rgn );
	void deleteMinorRegion( ctRegion & rgn, bool merge );
	void deleteContinuousRegion( int x, int y, int zslice, int rtype );
	void saveHistogram();
	void saveAsGraph();
	void saveDistoribution();
	void saveInformation();

	int patternsInitialized()
	{
		int count = 0;
		for ( regionIterator rit( *this, true ); rit; ++rit ) {
			count += rit->patterns.size;
		}
		return count;
	}
	void patternsInitialize()
	{
		for ( regionIterator rit( *this, true ); rit; ++rit ) {
			for ( array< retainer<miPattern> >::iterator it( rit->patterns ); it; ++it ) {
				it()->initializeParameter();
			}
		}
	}

	void load( memento & m );
	void save( memento & m );
	void exchange( memento & m );
	static bool regGet( const string & name, pixel & p )
	{
		string v;
        bool result = false;
        if ( ::regGet( name + "R", v ) ) { p.r = v.toi(); result = true; }
		if ( ::regGet( name + "G", v ) ) { p.g = v.toi(); result = true; }
		if ( ::regGet( name + "B", v ) ) { p.b = v.toi(); result = true; }
        if ( ::regGet( name + "A", v ) ) { p.a = v.toi(); result = true; }
        return result;
	}
	static void regSet( const string & name, pixel & p )
	{
		::regSet( name + "R", string( p.r ) );
		::regSet( name + "G", string( p.g ) );
		::regSet( name + "B", string( p.b ) );
		::regSet( name + "A", string( p.a ) );
	}
    void progress();
private:
    struct GHNC_status
    {
        miDocument * doc;
        bool normalized;
        bool considerGradient;
        const array< pair<int,int> > * radiusIndexList;
        const array<int> * radiusList;
        const array<ctRegion> * nearRegion;
        const array<ctRegion> * subtractNearRegion;
    };
    static void GHNC_process( void * data, int index )
    {
        GHNC_status * self = ( GHNC_status * ) data;
        self->doc->divideRegionByGaussianHistogramCorrelation( self->normalized, self->considerGradient, 
            self->doc->slices.slices[index], 
            *(self->radiusIndexList), *(self->radiusList), *(self->nearRegion), *(self->subtractNearRegion) );
    }
    bool divideRegionByGaussianHistogramCorrelation( bool normalized, bool considerGradient, ctSlice & slice, const array< pair<int,int> > & radiusIndexList, const array<int> & radiusList, const array<ctRegion> & nearRegion, const array<ctRegion> & subtractNearRegion );
    struct KS_status
    {
        miDocument * doc;
        const ctRegion * nearRegion;
    };
    static void KS_process( void * data, int index )
    {
        KS_status * self = ( KS_status * ) data;
        self->doc->divideRegionByKolmogorovSmirnov( 
            self->doc->slices.slices[index], 
            *(self->nearRegion) );
    }
    struct SD_status
    {
        miDocument * doc;
        const ctRegion * nearRegion;
    };
    static void SD_process( void * data, int index )
    {
        SD_status * self = ( SD_status * ) data;
        self->doc->divideRegionBySmoothedDifference( 
            self->doc->slices.slices[index], 
            *(self->nearRegion) );
    }
    bool divideRegionByKolmogorovSmirnov( ctSlice & slice, const ctRegion & nearRegion );
    bool divideRegionBySmoothedDifference( ctSlice & slice, const ctRegion & nearRegion );
};
