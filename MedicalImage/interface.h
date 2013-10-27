//専用拡張コントロールクラス

class editboxSpinSlider
{
public:
 	fc::editbox editbox;
	fc::spin spin;
	fc::slider slider;
	int value;
	editboxSpinSlider() : value( 0 )
	{
	}
	int operator()() const { return value; }
	void operator()( int src ) {
		slider.set( value = src );
	}
	void initialize( HWND hDlg, int editbox_id, int spin_id, int slider_id, int minimum, int maximum )
	{
		editbox.initialize( hDlg, editbox_id );
		spin.initialize( hDlg, spin_id );
		slider.initialize( hDlg, slider_id );
		spin.edit = slider.edit = & editbox;
		spin.create();
		initialize( minimum, maximum );
	}
	void enable( bool value )
	{
		spin.enable( value );
		slider.enable( value );
		editbox.enable( value );
	}
	void initialize( int minimum, int maximum )
	{
		if ( minimum <= maximum ) {
			spin.set( minimum, maximum );
			slider.set( minimum, maximum );
			slider.set( value );
		} else {
			value = 0;
			enable( false );
			spin.set( 0, 0 );
			slider.set( 0, 0 );
			slider.set( 0 );
		}
	}
	void finalize()
	{
		value = slider.index;
	}
	bool process()
	{
		if ( ! editbox || ! spin || ! slider ) return false;
		if ( value == slider.index ) return false;
		value = slider.index;
		return true;
	}
};

class editboxSpinSliderLimit
{
public:
	editboxSpinSlider up, down;
	bool process()
	{
		if ( up.process() ) {
			down( clamp( down.slider.minimum, down(), up.slider.minimum ) );
			down.initialize( down.slider.minimum, up() );
			return true;
		} else if ( down.process() ) {
			up( clamp( down.slider.maximum, up(), up.slider.maximum ) );
			up.initialize( down(), up.slider.maximum );
			return true;
		}
		return false;
	}
	void initialize( HWND hDlg, int up_editbox_id, int up_spin_id, int up_slider_id, int down_editbox_id, int down_spin_id, int down_slider_id, int minimum, int maximum )
	{
		up.initialize( hDlg, up_editbox_id, up_spin_id, up_slider_id, down(), maximum );
		down.initialize( hDlg, down_editbox_id, down_spin_id, down_slider_id, minimum, up() );
	}
	void enable( bool value )
	{
		up.enable( value );
		down.enable( value );
	}
};

class editboxSpin
{
public:
 	fc::editbox editbox;
	fc::spin spin;
	int value;
	int minimum, maximum;
	editboxSpin() : value( 0 )
	{
	}
	int operator()() const { return value; }
	void operator()( int src ) {
		editbox.set( value = src );
	}
	void initialize( HWND hDlg, int editbox_id, int spin_id, int _minimum, int _maximum )
	{
		editbox.initialize( hDlg, editbox_id );
		spin.initialize( hDlg, spin_id );
		spin.edit = & editbox;
		spin.create();
		initialize( _minimum, _maximum );
	}
	void enable( bool value )
	{
		spin.enable( value );
		editbox.enable( value );
	}
	void initialize( int _minimum, int _maximum )
	{
		minimum = _minimum;
		maximum = _maximum;
		if ( minimum <= maximum ) {
			enable( true );
			spin.set( minimum, maximum );
			editbox.set( value );
		} else {
			value = 0;
			enable( false );
			spin.set( 0, 0 );
			editbox.set( 0 );
		}
	}
	void finalize()
	{
		value = atoi( editbox.get() );
	}
	bool process()
	{
		if ( ! editbox || ! spin ) return false;
		if ( value == atoi( editbox.get() ) ) return false;
		value = atoi( editbox.get() );
		if ( value != clamp( minimum, value, maximum ) ) {
			value = clamp( minimum, value, maximum );
			editbox.set( value );
		}
		return true;
	}
};

