//コマンドの実装と派生されたコマンドオブジェクトを隠蔽するために、グローバル関数から行う。
namespace operationCommand
{
	//キャリブレーション
	void calibration();
	//解析処理
	void divideRegionThreshhold( bool allSlices = false );
	void divideRegionBySmoothedDifference( bool allSlices = false, int nearRegionRadius = 3 );
	void divideRegionByKolmogorovSmirnov( bool allSlices = false, int nearRegionRadius = 3 );
	void divideRegionByGaussianHistogramCorrelation( bool allSlices = false, bool dlg = false, bool normalized = false, bool edge = false );
	void divideRegionByCorrelationMultiParam( bool allSlices = false );
	void divideRegionBySmoothedDifferenceMultiParam( bool allSlices = false );
	void removeSmallLungRegion();
	void analyzeLungRegion();
};
