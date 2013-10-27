//ある時刻の１点
class boundaryPointFrame
{
public:
	enum {
		typeInitial,
		typeEdited,
		typeInterpolated,
	};
	point2<int> position;
	bool adjusted;
	int type;
	boundaryPointFrame() : type( typeInterpolated ), position( 0, 0 ), adjusted( false )
	{
	}
	boundaryPointFrame( const boundaryPointFrame & v )
	{
		*this = v;
	}
	boundaryPointFrame & operator=( const boundaryPointFrame & v )
	{
		position = v.position;
		adjusted = v.adjusted;
		type = v.type;
		return *this;
	}
	bool operator==( const boundaryPointFrame & v ) const
	{
		return this == & v;
	}
	bool operator!=( const boundaryPointFrame & v ) const { return ! ( *this == v ); }
	void again()
	{
		if ( type == typeEdited ) {
			type = typeInterpolated;
		}
		adjusted = false;
	}
};
//１点
class boundaryPoint
{
public:
	array<boundaryPointFrame> points;
	boundaryPoint * reference;
	list<boundaryPoint *> referencedList;
	boundaryPoint() : reference( NULL )
	{
	}
	boundaryPoint( const boundaryPoint & v )
	{
		*this = v;
	}
	~boundaryPoint()
	{
		removeReferenced();
		removeReference();
	}
	boundaryPoint & operator=( const boundaryPoint & v )
	{
		reference = v.reference;
		points = v.points;
		return *this;
	}
	bool operator==( const boundaryPoint & v ) const
	{
		return this == & v;
	}
	bool operator!=( const boundaryPoint & v ) const { return ! ( *this == v ); }
	void setReference( boundaryPoint * src )
	{
		reference = src;
		src->referencedList.push_back( this );
	}
	void removeReference()
	{
		if ( reference ) {
			reference->referencedList.pop( this );
		}
		reference = NULL;
	}
	void removeReferenced()
	{
		while ( ! referencedList.empty() ) {
			referencedList.first()->removeReference();
		}
	}
	bool referencing() const { return reference ? true : false; }
	bool referencing( boundaryPoint * r ) const { return r == reference ? true : false; }
	point2<int> & edit( int index )
	{
		if ( reference ) return reference->edit( index );
		if ( ! between( 0, index, points.size ) ) {
			static point2<int> wk( 0, 0 );
			return wk;
		}
		if ( points[index].type == boundaryPointFrame::typeInitial ) {
			points[index].adjusted = false;
			return points[index].position;
		}
		points[index].adjusted = false;
		points[index].type = boundaryPointFrame::typeEdited;
		return points[index].position;
	}
	void undo( int index, const point2<int> & p )
	{
		if ( reference ) { reference->undo( index, p ); return; }
		if ( ! between( 0, index, points.size ) ) return;
		points[index].position = p;
		points[index].adjusted = false;
		if ( points[index].type != boundaryPointFrame::typeInitial ) {
			points[index].type = boundaryPointFrame::typeInterpolated;
		}
	}
	point2<int> & operator()( int index )
	{
		if ( reference ) return ( *reference )( index );
		if ( ! between( 0, index, points.size ) ) {
			static point2<int> wk( 0, 0 );
			return wk;
		}
		return points[index].position;
	}
	int & type( int index )
	{
		if ( reference ) return reference->type( index );
		if ( ! between( 0, index, points.size ) ) {
			static int wk = boundaryPointFrame::typeInterpolated;
			return wk;
		}
		return points[index].type;
	}
	static boundaryPoint factory( const point2<int> & p );
	static boundaryPoint factory(){ return boundaryPoint(); }
	void update( bool local, bool edit, bool force );
	void again()
	{
		if ( reference ) return;
		for ( array<boundaryPointFrame>::iterator it( points ); it; ++it ) {
			it->again();
		}
	}
};
//曲線
class boundaryCurve
{
public:
	bool cyclic;
	list<boundaryPoint> points;
	boundaryCurve() : cyclic( false )
	{
	}
	boundaryCurve( const boundaryCurve & v ) 
	{
		*this = v;
	}
	boundaryCurve & operator=( const boundaryCurve & v )
	{
		cyclic = v.cyclic;
		points = v.points;
		return *this;
	}
	bool operator==( const boundaryCurve & v ) const
	{
		return this == & v;
	}
	bool operator!=( const boundaryCurve & v ) const { return ! ( *this == v ); }
	void setAsDivider( boundaryPoint & first, boundaryPoint & end );
	void appendPoint( const point2<int> & p )
	{
		points.push_back( boundaryPoint::factory( p ) );
	}
	void appendIntermidiatePoint( const point2<int> & p )
	{
		volatile int wk = points.size - 1;
		points.push( boundaryPoint::factory( p ), wk );
	}
	void appendIntermidiatePoint( int index, const point2<int> & p )
	{
		boundaryPoint & bp = points.push( boundaryPoint::factory( p ), index );
	}
	void removeIntermidiatePoint( int index )
	{
		points.pop( points[index] );
	}
	void update( bool local, bool edit, bool force )
	{
		for ( list<boundaryPoint>::iterator it( points ); it; ++it ) {
			it->update( local, edit, force );
		}
	}
	void again()
	{
		for ( list<boundaryPoint>::iterator it( points ); it; ++it ) {
			it->again();
		}
	}
	void segment( list< point2<int> > & points, int index, int time );
};
//複数の曲線の一部に囲まれた部分
class boundaryArea;
class boundaryCurveIndex
{
public:
	int curve, start, end, size;
	class iterator
	{
		const boundaryCurveIndex * source;
		int index;
	public:
		////////////////////////////////////////////////////////////////////////////////
		//生成・消滅
		iterator()
			: source( NULL ), index( -1 ) {}
		iterator( const boundaryCurveIndex & _src ) 
			: source( & _src ), index( -1 )
		{
			if ( source->size == 0 ) return;
			if ( source->curve == 0 ) {//cyclic
				index = source->start % source->size;
			} else {
				if ( source->start < source->end ) {
					index = source->start;
				} else {
					index = source->end;
				}
			}
		}
		iterator( const iterator & it ) 
			: source( it.source ), index( it.index ){}
		iterator & operator=( const iterator & src )
		{
			source = src.source;
			index = src.index;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator bool () const { if ( ! source ) return false; return index != -1; }
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		int operator()() const { return index; }
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		iterator & operator++()
		{
			if ( source && index != -1 ) {
				if ( source->curve == 0 ) {
					if ( source->whole() ) {
						++index;
					} else {
						index = ( index + 1 ) % source->size;
					}
					if ( index == source->end ) {
						index = -1;
					}
				} else {
					++index;
					if ( source->start < source->end ) {
						if ( index == source->end ) {
							index = -1;
						}
					} else {
						if ( index == source->start ) {
							index = -1;
						}
					}
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
	};
	boundaryCurveIndex()
	{
	}
	boundaryCurveIndex( const boundaryCurveIndex & v ) 
	{
		*this = v;
	}
	boundaryCurveIndex & operator=( const boundaryCurveIndex & v )
	{
		curve = v.curve;
		start = v.start;
		end = v.end;
		size = v.size;
		return *this;
	}
	bool operator==( const boundaryCurveIndex & v ) const
	{
		return this == & v;
	}
	bool operator!=( const boundaryCurveIndex & v ) const { return ! ( *this == v ); }
	bool include( int curveIndex, int pointIndex )
	{
		if ( curveIndex != curve ) return false;
		return include( pointIndex );
	}
	bool include( int index )
	{
		if ( curve == 0 ) {//周期的
			if ( start < end ) {
				return between( start, index, end );
			} else {//順序が逆なら，0点を通過する
				return ( index <= end || start <= index );
			}
		} else {
			if ( start < end ) {
				return between( start, index, end );
			} else {//順序が逆なら，曲線を逆に見る
				return between( end, index, start );
			}
		}
	}
	void appended( int index )
	{
		// indexは[0,size]の範囲で可能性があるとする
		if ( curve == 0 ) {
			if ( whole() ) {//全体を指しているとき
				++end;
			} else {
				if ( index == 0 ) {//端点//ここは来ないかもしれない
					//端点と同じ場所を指しているときにはそちらに移動する
					if ( index < start ) {
						start += 1;
					}
					if ( index < end ) {
						end += 1;
					}
				} else if ( index == size ) {//端点
				} else {
					//点と同じ場所を指しているときにはそちらに移動しない
					if ( index <= start ) {
						start += 1;
					}
					if ( index <= end ) {
						end += 1;
					}
				}
			}
		} else {
			if ( index == 0 || index == size ) {//端点
				//データがおかしい
				size = -1;
			} else {
				//点と同じ場所を指しているときにはそちらに移動しない
				if ( index <= start ) {
					start += 1;
				}
				if ( index <= end ) {
					end += 1;
				}
			}
		}
		++size;
	}
	void removed( int index )
	{
		// indexは[0,size-1]の範囲で可能性があるとする
		if ( curve == 0 ) {
			if ( whole() ) {
				--end;
			} else {
				if ( index < start ) {
					start -= 1;
				}
				if ( index < end ) {
					end -= 1;
				}
			}
		} else {
			if ( index == 0 || index == size ) {//端点
				//データがおかしい
				size = -1;
			} else {
				if ( index < start ) {
					start -= 1;
				}
				if ( index < end ) {
					end -= 1;
				}
			}
		}
		--size;
	}
	
	bool whole() const
	{
		return ( start == 0 && end == size );
	}
	void debug()
	{
		string wk;
		wk.print( "curve %d ( %d -> %d )\n", curve, start, end );
		OutputDebugString( wk );
	}
};
class boundaryPart
{
public:
	array<boundaryCurveIndex> curves;
	array< region<int> > shapes;
	int type;
	boundaryPart() : type( 0 )
	{
	}
	boundaryPart( const boundaryPart & v ) 
	{
		*this = v;
	}
	boundaryPart & operator=( const boundaryPart & v )
	{
		type = v.type;
		curves = v.curves;
		shapes = v.shapes;
		return *this;
	}
	bool operator==( const boundaryPart & v ) const
	{
		return this == & v;
	}
	bool operator!=( const boundaryPart & v ) const { return ! ( *this == v ); }
	static boundaryPart factory();
	void update( boundaryArea * area, int timeIndex );
	int find( int curveIndex, int pointIndex )
	{
		int i = 0;
		for ( array<boundaryCurveIndex>::iterator it( curves ); it; ++it ) {
			if ( it->curve == curveIndex ) {
				if ( it->include( pointIndex ) ) break;
			}
			++i;
		}
		return i;
	}
	bool including( int curveIndex, int pointIndex )
	{
		int i = find( curveIndex, pointIndex );
		return between( 0, i, curves.size - 1 );
	}
	void resetConnection()
	{
		curves.release();
	}
	void appended( int curveIndex, int pointIndex )
	{
		for ( array<boundaryCurveIndex>::iterator it( curves ); it; ++it ) {
			if ( it->curve != curveIndex ) continue;
			it->appended( pointIndex );
		}
	}
	void removed( int curveIndex, int pointIndex )
	{
		for ( array<boundaryCurveIndex>::iterator it( curves ); it; ++it ) {
			if ( it->curve != curveIndex ) continue;
			it->removed( pointIndex );
		}
	}
	bool clean()
	{
		bool yet = true;
		bool cleaned = false;
		while ( yet ) {
			yet = false;
			for ( array<boundaryCurveIndex>::iterator it( curves ); it; ++it ) {
				if ( it->start == it->end ) {
					yet = true;
					cleaned = true;
					curves.pop( it() );
					break;
				}
			}
		}
		return cleaned;
	}
	void debug()
	{
		OutputDebugString( "Part\n" );
		for ( array<boundaryCurveIndex>::iterator it( curves ); it; ++it ) {
			it->debug();
		}
	}
};
//曲線での部分と内部分割線により分かれた部分
class boundaryArea
{
public:
	list<boundaryCurve> divisions;
	list<boundaryPart> parts;
	list<boundaryPoint*> controlPoints;
	array< list< point2<int> > > boundaryCurves;
	array< region<int> > shapes;
	array<bool> valid;
	bool enable;
	boundaryArea() : enable( true )
	{
	}
	boundaryArea( const boundaryArea & v )
	{
		*this = v;
	}
	boundaryArea & operator=( const boundaryArea & v )
	{
		divisions = v.divisions;
		parts = v.parts;
		shapes = v.shapes;
		enable = v.enable;
		boundaryCurves = v.boundaryCurves;
		valid = v.valid;

		controlPoints.release();
		return *this;
	}
	bool operator==( const boundaryArea & v ) const
	{
		return this == & v;
	}
	bool operator!=( const boundaryArea & v ) const { return ! ( *this == v ); }
	static boundaryArea factory();
	void appendCircumferencePoint( const point2<int> & p )
	{
		volatile int curveIndex = 0;
		boundaryCurve & curve = divisions[curveIndex];
		volatile int pointIndex = curve.points.size;
		appendPoint( curveIndex, pointIndex, p );
	}
	void appendDividePoint( const point2<int> & p )
	{
		volatile int curveIndex = divisions.size - 1;
		boundaryCurve & curve = divisions[curveIndex];
		volatile int pointIndex = curve.points.size - 1;
		appendPoint( curveIndex, pointIndex, p );
	}
	void appendPoint( int curveIndex, int pointIndex, const point2<int> & p )
	{
		boundaryCurve & curve = divisions[curveIndex];
		if ( curve.cyclic && pointIndex == 0 ) {//後ろに追加する
			pointIndex = curve.points.size;
		}
		curve.appendIntermidiatePoint( pointIndex, p );
		for ( list<boundaryPart>::iterator it( parts ); it; ++it ) {
			it->appended( curveIndex, pointIndex );
			it->clean();
		}
		updateConnection();
	}
	void removePoint( int curveIndex, int pointIndex )
	{
		int num = numberRelation( curveIndex, pointIndex );
		boundaryCurve & curve = divisions[curveIndex];
		if ( 1 < num ) {
			//リンクが張られているので全て削除
			while ( divisions.size > 1 ) {
				divisions.pop_back();
			}
			parts.release();
			parts.push_back( boundaryPart::factory() );
			boundaryCurveIndex & c = parts[0].curves[0];
			c.end = c.size = divisions[0].points.size;
		}
		if ( curveIndex == 0 || num == 1 ) {
			boundaryCurve & curve = divisions[curveIndex];
			curve.removeIntermidiatePoint( pointIndex );
			for ( list<boundaryPart>::iterator it( parts ); it; ++it ) {
				it->removed( curveIndex, pointIndex );
				it->clean();
			}
		}
		updateConnection();
	}
	const point2<int> & point( int curveIndex, int pointIndex, int time )
	{
		boundaryCurve & curve = divisions[curveIndex];
		return curve.points[ ( pointIndex + curve.points.size ) % curve.points.size ]( time );
	}
	bool find( int & curveIndex, int & pointIndex, boundaryPoint * point )
	{
		curveIndex = 0;
		for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
			pointIndex = 0;
			for ( list<boundaryPoint>::iterator itp( it->points ); itp; ++itp ) {
				if ( & itp() == point ) {
					return true;
				}
				++pointIndex;
			}
			++curveIndex;
		}
		return false;
	}
	bool findOriginal( int & curveIndex, int & pointIndex, boundaryPoint * point )
	{
		while ( point->referencing() ) {
			point = point->reference;
		}
		return find( curveIndex, pointIndex, point );
	}
	bool find( int & curveIndex, int & pointIndex, int time, const point2<int> & p )
	{
		curveIndex = 0;
		for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
			pointIndex = 0;
			for ( list<boundaryPoint>::iterator itp( it->points ); itp; ++itp ) {
				if ( ! itp->referencing() ) {
					point2<int> & c = itp()( time );
					if ( between( p.x - 1, c.x, p.x + 1 ) && 
						between( p.y - 1, c.y, p.y + 1 ) ) {
						return true;
					}
				}
				++pointIndex;
			}
			++curveIndex;
		}
		return false;
	}
	bool findOnLine( int & curveIndex, int & pointIndex, int time, const point2<int> & p )
	{
		curveIndex = 0;
		for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
			pointIndex = 0;
			for ( list<boundaryPoint>::iterator itp( it->points ); itp; ++itp ) {
				list< point2<int> > points;
				it->segment( points, pointIndex, time );
				for ( list< point2<int> >::iterator itl( points ); itl; ++itl ) {
					point2<int> & c = itl();
					if ( between( p.x - 1, c.x, p.x + 1 ) && 
						between( p.y - 1, c.y, p.y + 1 ) ) {
						return true;
					}
				}
				++pointIndex;
			}
			++curveIndex;
		}
		return false;
	}
	void movePoint( int curveIndex, int pointIndex, int time, const point2<int> & p )
	{
		divisions[curveIndex].points[pointIndex].edit( time ) = p;
		touch( -1 );
	}
	void undoPoint( int curveIndex, int pointIndex, int time, const point2<int> & p )
	{
		divisions[curveIndex].points[pointIndex].undo( time, p );
		touch( -1 );
	}
	int numberRelation( int curveIndex, int pointIndex )
	{
		boundaryPoint & bp = divisions[curveIndex].points[pointIndex];
		int result = 1;
		for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
			for ( list<boundaryPoint>::iterator itp( it->points ); itp; ++itp ) {
				if ( itp->referencing( & bp ) ) {
					++result;
				}
			}
		}
		return result;
	}
	int numberPart( int curveIndex, int pointIndex )
	{
		int result = 0;
		for ( list<boundaryPart>::iterator it( parts ); it; ++it ) {
			if ( it->including( curveIndex, pointIndex ) ) {
				++result;
			}
		}
		return result;
	}
	int partIndex( int curveIndex, int pointIndex )
	{
		int result = 0;
		for ( list<boundaryPart>::iterator it( parts ); it; ++it ) {
			if ( it->including( curveIndex, pointIndex ) ) {
				break;
			}
			++result;
		}
		return result;
	}
	void updatePoint( bool local = false, bool edit = false, bool force = false );//コントロールポイントの解析
	void updateConnection();//グラフ構造の変化時のアップデート
	void update( int timeIndex );//表示用データのアップデート
	void again();
	void divide( int targetPartIndex, int firstCurveIndex, int firstPointIndex, int lastCurveIndex, int lastPointIndex );
	void touch( int timeIndex );
	void reset()
	{
		divisions.release();
		parts.release();
	}
	void debug()
	{
		OutputDebugString( "Parts\n" );
		for ( list<boundaryPart>::iterator it( parts ); it; ++it ) {
			it->debug();
		}
	}
};
