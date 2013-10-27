#include "main.h"
#include "document.h"

retainer<trackDocument> trackDocument::hidden;

point2<double> trackDocument::analysis( const point2<int> & p )
{
	for ( int i = 0; i < verticalLinks.size; ++i ) {
		for ( list<link>::iterator it( verticalLinks[i] ); it; ++it ) {
			return analysisNear( p );
		}
	}
	for ( int i = 0; i < horizontalLinks.size; ++i ) {
		for ( list<link>::iterator it( horizontalLinks[i] ); it; ++it ) {
			return analysisNear( p );
		}
	}
	for ( int i = 0; i < ascentLinks.size; ++i ) {
		for ( list<link>::iterator it( ascentLinks[i] ); it; ++it ) {
			return analysisNear( p );
		}
	}
	for ( int i = 0; i < descentLinks.size; ++i ) {
		for ( list<link>::iterator it( descentLinks[i] ); it; ++it ) {
			return analysisNear( p );
		}
	}
	return analysisBearings( p );
}
void trackDocument::againAnalysis()
{
	for ( list< boundaryArea >::iterator it( areas ); it; ++it ) {
		it->again();
	}
	imageWindow->invalidate();
}
