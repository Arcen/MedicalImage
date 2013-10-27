//�R�}���h�̎����Ɣh�����ꂽ�R�}���h�I�u�W�F�N�g���B�����邽�߂ɁA�O���[�o���֐�����s���B
namespace operationCommand
{
	//�L�����u���[�V����
	void calibration();
	//��͏���
	void divideRegionThreshhold( bool allSlices = false );
	void divideRegionBySmoothedDifference( bool allSlices = false, int nearRegionRadius = 3 );
	void divideRegionByKolmogorovSmirnov( bool allSlices = false, int nearRegionRadius = 3 );
	void divideRegionByGaussianHistogramCorrelation( bool allSlices = false, bool dlg = false, bool normalized = false, bool edge = false );
	void divideRegionByCorrelationMultiParam( bool allSlices = false );
	void divideRegionBySmoothedDifferenceMultiParam( bool allSlices = false );
	void removeSmallLungRegion();
	void analyzeLungRegion();
};
