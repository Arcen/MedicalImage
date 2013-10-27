template<class T, class U = int>
class checkMaximum
{
public:
	T value;
	U sub;
	bool initialized;
	checkMaximum() : initialized( false ){}
	void initialize(){ initialized = false; }
	//値をチェックする
	bool operator()( const T & _value, const U & _sub = 0 )
	{
		if ( ! initialized ) {
			initialized = true;
			value = _value;
			sub = _sub;
			return true;
		}
		if ( value < _value ) {
			value = _value;
			sub = _sub;
			return true;
		}
		return false;
	}
	//値が有効かどうか調べる
	operator bool(){
		return initialized;
	}
	//値を得る
	T operator()(){
		return value;
	}
};
template<class T, class U = int>
class checkMinimum
{
public:
	T value;
	U sub;
	bool initialized;
	checkMinimum() : initialized( false ){}
	void initialize( bool _max ) { initialized = false; }
	//値をチェックする
	bool operator()( const T & _value, const U & _sub = 0 )
	{
		if ( ! initialized ) {
			initialized = true;
			value = _value;
			sub = _sub;
			return true;
		}
		if ( _value < value ) {
			value = _value;
			sub = _sub;
			return true;
		}
		return false;
	}
	//値が有効かどうか調べる
	operator bool(){
		return initialized;
	}
	//値を得る
	T operator()(){
		return value;
	}
};
template<class T, class U = int>
class checkMinimumMaximum
{
public:
	T minimumValue, maximumValue;
	U minimumSub, maximumSub;
	bool initialized;
	bool maximum;
	checkMinimumMaximum() : initialized( false ) {}
	void initialize() { initialized = false; }
	//値をチェックする
	bool operator()( const T & _value, const U & _sub = 0 )
	{
		if ( ! initialized ) {
			initialized = true;
			minimumValue = _value;
			maximumValue = _value;
			minimumSub = _sub;
			maximumSub = _sub;
			return true;
		}
		if ( maximumValue < _value ) {
			maximumValue = _value;
			maximumSub = _sub;
			return true;
		} else if ( _value < minimumValue ) {
			minimumValue = _value;
			minimumSub = _sub;
			return true;
		}
		return false;
	}
	//値が有効かどうか調べる
	operator bool(){
		return initialized;
	}
};
