
////////////////////////////////////////////////////////////////////////////////
// ANIMATION
void Maya::exportAnimation()
{
	int	startFrame = static_cast<int>( ceil( maximum<double>( 0, MAnimControl::minTime().value() ) ) );
	int	endFrame = static_cast<int>( maximum<double>( MAnimControl::maxTime().value(), startFrame ) );
	int frames = endFrame - startFrame + 1;
	animationNode::time->first( 0 );
	animationNode::time->final( endFrame - startFrame );

	for ( list< retainer<animationNode> >::iterator it( animationNodes ); it; ++it ) {
		it()->reserve( frames );
	}
	MTime currentTime = MAnimControl::currentTime();
	MTime current( static_cast<double>(startFrame), MTime::uiUnit() );
	MTime add( 1, MTime::uiUnit() );
	for ( int i = 0; i < frames; i++ ) {
		MAnimControl::setCurrentTime( current );
		for ( list< retainer<animationNode> >::iterator it( animationNodes ); it; ++it ) {
			it()->set();
		}
		current += add;
	}
	MAnimControl::setCurrentTime(currentTime);
	for ( list< retainer<animationNode> >::iterator it( animationNodes ); it; ++it ) {
		it()->finish();
	}
	animationNodes.release();
}
