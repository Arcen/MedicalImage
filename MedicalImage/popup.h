//ポップアップメニュー処理クラス

typedef fc::popupmenu::item popupmenuitem;
class regionMenuInterface
{
public:
	fc::popupmenu rclick;
	popupmenuitem * deleteRegion, * appendRegion, * samplingRegion, * averageRegion, * deleteMinorRegion, * mergeMinorRegion;
	bool initialized;
	regionMenuInterface() : initialized( false ){}
	void initialize()
	{
		rclick.initialize( imageWindow->hwnd );
		rclick.push_back( appendRegion = new popupmenuitem( rclickAppendRegionID, "対象領域追加" ) );
		rclick.push_back( deleteRegion = new popupmenuitem( rclickDeleteRegionID, "対象領域削除" ) );
		rclick.push_back( samplingRegion = new popupmenuitem( rclickSamplingRegionID, "標本領域定義" ) );
		rclick.push_back( averageRegion = new popupmenuitem( rclickAverageRegionID, "領域の平均値" ) );
		rclick.push_back( deleteMinorRegion = new popupmenuitem( rclickDeleteMinorRegionID, "小領域削除" ) );
		rclick.push_back( mergeMinorRegion = new popupmenuitem( rclickMergeMinorRegionID, "小領域併合" ) );
		initialized = true;
	}
	popupmenuitem * call( int x, int y )
	{
		if ( ! initialized ) return NULL;
		RECT r;
		GetWindowRect( imageWindow->hwnd, & r );
		if ( ! between<int>( r.left, x, r.right - 1 ) ||
			! between<int>( r.top, y, r.bottom - 1 ) ) return NULL;
		return rclick.call( x, y );
	}
};
extern regionMenuInterface * regionMenu;
class clickMenuInterface
{
public:
	fc::popupmenu rclick;
	popupmenuitem * deleteRegion;
	popupmenuitem * deleteContinuousRegion;
	popupmenuitem * defineCenter;
	bool initialized;
	clickMenuInterface() : initialized( false ){}
	void initialize()
	{
		rclick.initialize( imageWindow->hwnd );
		rclick.push_back( deleteRegion = new popupmenuitem( rclickDeleteID, "対象の領域削除" ) );
		rclick.push_back( deleteContinuousRegion = new popupmenuitem( rclickContinouseDeleteID, "対象の連続領域削除" ) );
		rclick.push_back( defineCenter = new popupmenuitem( rclickDefineCenterID, "気管支分岐部の設定" ) );
		initialized = true;
	}
	popupmenuitem * call( int x, int y )
	{
		if ( ! initialized ) return NULL;
		RECT r;
		GetWindowRect( imageWindow->hwnd, & r );
		if ( ! between<int>( r.left, x, r.right - 1 ) ||
			! between<int>( r.top, y, r.bottom - 1 ) ) return NULL;
		return rclick.call( x, y );
	}
};
extern clickMenuInterface * clickMenu;
