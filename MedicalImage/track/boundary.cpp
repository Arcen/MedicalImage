#include "main.h"
#include "document.h"

void pointsToRegion( list< point2<int> > & points, region<int> & rgn, int width, int height )
{
	static array2<bool> flag;
	flag.allocate( width, height );
	memset( flag.data, 0, sizeof( bool ) * flag.size );
	for ( list< point2<int> >::iterator it( points ); it; ++it ) {
		flag( clamp( 0, it->x, width - 1 ), clamp( 0, it->y, height - 1 ) ) = true;
	}
	rgn.set( flag, 0, 0 );
}

boundaryPart boundaryPart::factory()
{
	boundaryPart r;
	r.curves.allocate( 1 );
	boundaryCurveIndex & c = r.curves[0];
	c.curve = 0;
	c.start = 0;
	c.end = 0;
	c.size = 0;
	trackDocument * doc = trackDocument::get();
	if ( doc ) {
		r.shapes.allocate( doc->sizeTime() );
	}
	return r;
}
boundaryPoint boundaryPoint::factory( const point2<int> & p )
{
	boundaryPoint r;
	trackDocument * doc = trackDocument::get();
	if ( doc ) r.points.allocate( doc->sizeTime() );
	for ( array<boundaryPointFrame>::iterator it( r.points ); it; ++it ) {
		it->type = boundaryPointFrame::typeInterpolated;
		it->position = p;
	}
	if ( doc ) r.points[doc->baseIndex].type = boundaryPointFrame::typeInitial;
	return r;
}
boundaryArea boundaryArea::factory()
{
	boundaryArea r;
	r.divisions.push_back( boundaryCurve() );
	r.divisions.first().cyclic = true;
	r.parts.push_back( boundaryPart::factory() );
	trackDocument * doc = trackDocument::get();
	if ( doc ) {
		r.boundaryCurves.allocate( doc->sizeTime() );
		r.shapes.allocate( doc->sizeTime() );
		r.valid.allocate( doc->sizeTime() );
		r.touch( -1 );
	}
	return r;
}
void boundaryArea::touch( int timeIndex )
{
	if ( timeIndex == -1 ) {
		for ( int i = 0; i < valid.size; ++i ) {
			valid[i] = false;
		}
	} else {
		valid[timeIndex] = false;
	}
}

void boundaryPoint::update( bool local, bool edit, bool force )
{
	if ( reference ) return;
	trackDocument * doc = trackDocument::get();
	if ( ! doc ) return;
	const array<int> & wave = doc->wave;
	int initialCount = 0;
	int editedCount = 0;
	point2<int> initialPoint;
	bool adjusted = true;
	int i = 0;
	for ( array<boundaryPointFrame>::iterator itp( points ); itp; ++itp ) {
		if ( ! itp->adjusted ) {
			adjusted = false;
		}
		switch ( itp->type ) {
		case boundaryPointFrame::typeInterpolated:
			break;
		case boundaryPointFrame::typeInitial:
			initialPoint = itp->position;
			++initialCount;
			break;
		case boundaryPointFrame::typeEdited:
			++editedCount;
			break;
		}
		++i;
	}
	if ( adjusted && ( ! force ) ) return;
	if ( initialCount != 1 ) return;//�f�[�^�G���[
	if ( local && ( ! force ) ) {
		for ( int i = 0; i < points.size; ++i ) {
			points[i].adjusted = true;
		}
	} else {
		if ( ( edit && points[doc->currentViewImageIndex].type == boundaryPointFrame::typeInitial ) ||
			( ( ! edit ) ) ) {//��͉���p����
			point2<int> p = initialPoint;
			point2<double> c = doc->analysis( p );
			//point2<double> c = doc->analysisBearings( p );//�O�����������ꍇ�͂�����
			for ( int i = 0; i < points.size; ++i ) {
				points[i].adjusted = true;
				if ( points[i].type == boundaryPointFrame::typeInterpolated ) {
					points[i].position.x = p.x + c.x * wave[i];
					points[i].position.y = p.y + c.y * wave[i];
				}
			}
		} else if ( points[doc->currentViewImageIndex].type == boundaryPointFrame::typeEdited ) {//�ݒ����p����
			if ( wave[doc->currentViewImageIndex] ) {//�������܂�
				// wave[editedIndex] * c + now = edited
				// c = ( edited - now ) / wave[editedIndex]
				point2<double> c;
				point2<int> ep = points[doc->currentViewImageIndex].position;
				point2<int> p = initialPoint;
				c.x = ( ep.x - p.x ) / double( wave[doc->currentViewImageIndex] );
				c.y = ( ep.y - p.y ) / double( wave[doc->currentViewImageIndex] );
				for ( int i = 0; i < points.size; ++i ) {
					points[i].adjusted = true;
					if ( points[i].type == boundaryPointFrame::typeInterpolated ) {
						points[i].position.x = p.x + c.x * wave[i];
						points[i].position.y = p.y + c.y * wave[i];
					}
				}
			}
		}
	}
}

bool getControlPoint( const point2<int> & prev, const point2<int> & start, const point2<int> & end, const point2<int> & next, 
				  point2<int> & p0, point2<int> & p1, point2<int> & p2, point2<int> & p3 )
{
	// prev - start - end - next�̂S�_�̐܂���̂����Cprev, next�𐧌�_�v�Z�p�Ɏg�p���Cstart - end�����ԋȐ����쐬����
	vector2 ps( start.x - prev.x, start.y - prev.y );
	vector2 se( end.x - start.x, end.y - start.y );
	vector2 en( next.x - end.x, next.y - end.y );
	double len_se = vector2::length2( se );
	if ( vector2::length2( ps ) ) ps = vector2::normalize2( ps );
	if ( vector2::length2( se ) ) se = vector2::normalize2( se );
	if ( vector2::length2( en ) ) en = vector2::normalize2( en );
	vector2 pe = ps + se;
	vector2 ns = - ( se + en );
	if ( len_se == 0 ) return false;
	if ( vector2::length2( pe ) ) pe = vector2::normalize2( pe );
	if ( vector2::length2( ns ) ) ns = vector2::normalize2( ns );
	double cos_pe_se = vector2::dot2( pe, se );
	double cos_ns_es = vector2::dot2( ns, - se );
	p0 = start;
	p3 = end;
	p1 = p0;
	p2 = p3;
	if ( cos_pe_se > 1 / root2 && vector2::length2( ps ) ) {
		p1 = point2<int>( p0.x + len_se / 3 * pe.x, p0.y + len_se / 3 * pe.y );
	}
	if ( cos_ns_es > 1 / root2 && vector2::length2( en ) ) {
		p2 = point2<int>( p3.x + len_se / 3 * ns.x, p3.y + len_se / 3 * ns.y );
	}
	return true;
}
//�x�W�F�Ȑ���̓_����쐬
void getBezier3( const point2<int> & p0, const point2<int> & p1, const point2<int> & p2, const point2<int> & p3, list< point2<int> > & points )
{
	// p0 - p3�ɂ��O���x�W�F�Ȑ��̓_����쐬����D
	points.release();
	point2<int> current = p0;
	points.push_back( current );
	double d = 0.125;
	double t = d;
	while ( current != p3 ) {
		const double t2 = t * t;
		const double t3 = t2 * t;
		const double u = 1 - t;
		const double u2 = u * u;
		const double u3 = u2 * u;
		double x = p0.x * u3 + 3 * p1.x * u2 * t + 3 * p2.x * u * t2 + p3.x * t3;
		double y = p0.y * u3 + 3 * p1.y * u2 * t + 3 * p2.y * u * t2 + p3.y * t3;
		point2<int> next( round<int,double>( x ), round<int,double>( y ) );
		int step = maximum<int>( absolute( next.x - current.x ), absolute( next.y - current.y ) );
		if ( step == 0 ) {
			t = clamp<double>( 0, t + d, 1 );
		} else if ( step == 1 ) {
			points.push_back( next );
			current = next;
			t = clamp<double>( 0, t + d, 1 );
		} else {
			d /= 2;
			t = clamp<double>( 0, t - d, 1 );
		}
	}
	//���A���ɒ��������������
	bool cont = true;
	while ( cont ) {
		bool removed = false;
		for ( list< point2<int> >::iterator it( points ); it; ++it ) {
			list< point2<int> >::iterator previt = it.prev();
			list< point2<int> >::iterator nextit = it.next();
			if ( previt && nextit ) {
				const point2<int> & prev = previt();
				const point2<int> & next = nextit();
				const point2<int> & now = it();
				int step = maximum<int>( absolute( next.x - prev.x ), absolute( next.y - prev.y ) );
				if ( step == 1 ) {
					removed = true;
					points.pop( it );
					break;
				}
			}
		}
		if ( ! removed ) return;//�I��
	}
}

void boundaryCurve::segment( list< point2<int> > & pixels, int index, int time )
{
	if ( points.size <= 1 ) return;
	point2<int> prev, start, end, next;
	if ( cyclic ) {
		if ( ! between( 0, index, points.size - 1 ) ) return;
		prev = points.value( ( index + points.size - 1 ) % points.size )( time );
		start = points.value( ( index ) % points.size )( time );
		end = points.value( ( index + 1 ) % points.size )( time );
		next = points.value( ( index + 2 ) % points.size )( time );
	} else {
		if ( ! between( 0, index, points.size - 2 ) ) return;
		prev = points.value( clamp( 0, index - 1, points.size - 1 ) )( time );
		start = points.value( clamp( 0, index, points.size - 1 ) )( time );
		end = points.value( clamp( 0, index + 1, points.size - 1 ) )( time );
		next = points.value( clamp( 0, index + 2, points.size - 1 ) )( time );
	}
	point2<int> p0, p1, p2, p3;
	if ( getControlPoint( prev, start, end, next, p0, p1, p2, p3 ) ) {
		list< point2<int> > wk;
		getBezier3( p0, p1, p2, p3, wk );
		for ( list< point2<int> >::iterator it( wk ); it; ++it ) {
			pixels.push_back( it() );
		}
	}
}

void boundaryArea::updatePoint( bool local, bool edit, bool force )
{
	for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
		it->update( local, edit, force );
	}
	touch( -1 );
}
void boundaryArea::updateConnection()
{
	//�R���g���[���|�C���g�̃A�b�v�f�[�g
	controlPoints.release();
	for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
		for ( list<boundaryPoint>::iterator itp( it->points ); itp; ++itp ) {
			if ( ! itp->referencing() ) controlPoints.push_back( & itp() );
		}
	}
	touch( -1 );
}
void boundaryArea::update( int timeIndex )
{	
	//���E�̓_��̍쐬
	for ( int time = 0; time < shapes.size; ++time ) {
		if ( timeIndex != -1 ) {
			time = timeIndex;
		}
		if ( ! valid[time] ) {
			boundaryCurves[time].release();
			for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
				int index = 0;
				for ( list<boundaryPoint>::iterator itp( it->points ); itp; ++itp ) {
					it->segment( boundaryCurves[time], index, time );
					++index;
				}
			}
		}
		if ( timeIndex != -1 ) {
			break;
		}
	}

	if ( ! enable ) return;

	//�e�̈�̃A�b�v�f�[�g
	for ( int time = 0; time < shapes.size; ++time ) {
		if ( timeIndex != -1 ) {
			time = timeIndex;
		}
		if ( ! valid[time] ) {
			//�S�̗̂̈�̃A�b�v�f�[�g
			if ( ! divisions.empty() ) {
				boundaryCurve & circumference = divisions.first();
				region<int> shape;
				for ( int index = 0; index < circumference.points.size; ++index ) {
					list< point2<int> > seg;
					circumference.segment( seg, index, time );
					region<int> segshape;
					for ( list< point2<int> >::iterator itp( seg ); itp; ++itp ) {
						segshape |= region<int>( itp() );
					}
					shape |= segshape;
				}
				shape.fill( shapes[time] );
			}
			for ( list<boundaryPart>::iterator it( parts ); it; ++it ) {
				it->update( this, time );
			}
			//�e�̈�̒���
			region<int> & wholeshape = shapes[time];
			region<int> rest = wholeshape;
			for ( list<boundaryPart>::iterator it( parts ); it; ++it ) {
				it->shapes[time] = rest & it->shapes[time];
				rest -= it->shapes[time];
			}
			wholeshape -= rest;
			valid[time] = true;
		}
		if ( timeIndex != -1 ) {
			break;
		}
	}
}

void boundaryPart::update( boundaryArea * area, int timeIndex )
{
	trackDocument * doc = trackDocument::get();
	if ( ! doc ) return;
	for ( int time = 0; time < shapes.size; ++time ) {
		if ( timeIndex != -1 ) {
			time = timeIndex;
		}
		list< point2<int> > seg;
		for ( array<boundaryCurveIndex>::iterator it( curves ); it; ++it ) {
			boundaryCurve & curve = area->divisions[it->curve];//�ΏۃJ�[�u
			for ( boundaryCurveIndex::iterator iti( it() ); iti; ++iti ) {
				curve.segment( seg, iti(), time );
			}
		}
		region<int> shape;
		pointsToRegion( seg, shape, doc->width, doc->height );
		shape.fill( shapes[time] );
		if ( timeIndex != -1 ) {
			break;
		}
	}
}

void boundaryCurve::setAsDivider( boundaryPoint & first, boundaryPoint & end )
{
	points.push_back( boundaryPoint::factory() );//�����_
	points.push_back( boundaryPoint::factory() );//�ŏI�_
	points.first().setReference( & first );//�Q�Ƃ�����
	points.last().setReference( & end );//�Q�Ƃ�����
}

void boundaryArea::divide( int targetPartIndex, int firstCurveIndex, int firstPointIndex, int lastCurveIndex, int lastPointIndex )
{
	//��������p�[�g��ۑ����č폜
	boundaryPart bporiginal = parts[targetPartIndex];
	parts.pop( parts[targetPartIndex] );
	//������̃p�[�g���쐬�DFirst Point->Last Point�̏��ɐ؂��āC���X�g�����v���Ŏ�����bpFL�C�t�ɂ��镨��bpLF�Ƃ���D
	boundaryPart & bpFL = parts.push_back( boundaryPart::factory() );
	boundaryPart & bpLF = parts.push_back( boundaryPart::factory() );
	//�]�v�ȏ����N���A����
	bpFL.resetConnection();
	bpLF.resetConnection();
	//���͂̃J�[�u�𕪊�
	bool interest1 = true;
	//�����p�̃J�[�u
	boundaryPoint & bpfirst = divisions[firstCurveIndex].points[firstPointIndex];//�ŏ��̓_
	boundaryPoint & bplast = divisions[lastCurveIndex].points[lastPointIndex];//�Ō�̓_
	boundaryCurve & divider = divisions.push_back( boundaryCurve() );//������
	divider.setAsDivider( bpfirst, bplast );
	int dividerCurveIndex = divisions.size - 1;//�������̃J�[�u���X�g�̃C���f�b�N�X
	//�����_������J�[�u�̋��E�����X�g�ł̃C���f�b�N�X
	int startingIndex = bporiginal.find( firstCurveIndex, firstPointIndex );
	int endingIndex = bporiginal.find( lastCurveIndex, lastPointIndex );
	boundaryCurveIndex * bci = NULL;
	//first->last�̕�������ʂ�����C�c���T��
	for ( int i = endingIndex; i <= endingIndex + bporiginal.curves.size; ++i ) {
		int clampedIndex = i % bporiginal.curves.size;//�͈͊O���N�����v
		if ( i == endingIndex ) {//�ŏ������̏���
			//��������ǉ�
			bci = & bpFL.curves.push_back( boundaryCurveIndex() );
			bci->curve = dividerCurveIndex;
			bci->start = 0;
			bci->end = 2;
			bci->size = 2;
		}
		//�ʏ�̕�����ǉ�
		bci = & bpFL.curves.push_back( bporiginal.curves[clampedIndex] );
		if ( i == endingIndex ) {//�ŏ�
			//�ŏ��̃J�[�u�͏����l��؂����ʒu�ɍ��킹��
			bci->start = lastPointIndex;
			//��̋��E����ōŏ��ƍŌオ����Ƃ�
			if ( i == startingIndex ) {
				//���̐��������̕�������Ă��Ȃ�����̐��̏ꍇ��
				if ( ( bporiginal.curves[clampedIndex].whole() && bci->curve == 0 ) ||
					//�c��̐���ɍŏ��̕����_������ꍇ�ɂ�
					bci->include( firstPointIndex ) ) {//����ŏI���
					bci->end = firstPointIndex;
					break;
				}
			}
		} else if ( clampedIndex == startingIndex ) {//������Ė߂��Ă���
			bci->end = firstPointIndex;
			break;
		}
	}
	bpFL.clean();
	//last->first�̕�������ʂ�����C�c���T��
	for ( int i = startingIndex; i <= startingIndex + bporiginal.curves.size; ++i ) {
		int clampedIndex = i % bporiginal.curves.size;
		if ( i == startingIndex ) {
			//������
			bci = & bpLF.curves.push_back( boundaryCurveIndex() );
			bci->curve = dividerCurveIndex;
			bci->start = 2;
			bci->end = 0;
			bci->size = 2;
		}
		//�ʏ�̕���
		bci = & bpLF.curves.push_back( bporiginal.curves[clampedIndex] );
		if ( i == startingIndex ) {//�ŏ�
			bci->start = firstPointIndex;
			if ( i == endingIndex ) {
				if ( ( bporiginal.curves[clampedIndex].whole() && bci->curve == 0 ) ||
					bci->include( firstPointIndex ) ) {//����ŏI���
					bci->end = lastPointIndex;
					break;
				}
			}
		} else if ( clampedIndex == endingIndex ) {//������Ė߂��Ă���
			bci->end = lastPointIndex;
			break;
		}
	}
	bpLF.clean();
	updateConnection();
}
void boundaryArea::again()
{
	for ( list<boundaryCurve>::iterator it( divisions ); it; ++it ) {
		it->again();
	}
	updatePoint();
	touch( -1 );
}
