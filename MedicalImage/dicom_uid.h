struct dicomUID {
	char uid[1024];
	char name[1024];
	char comment[1024];
	char type[1024];
};
static dicomUID dicomUIDTable[] = {
{ "1.2.840.10008.1.1", "Verification SOP Class", "mFrnoNX", "rnoNX" }, 
{ "1.2.840.10008.1.2", "Implicit VR Little Endian: Default Transfer Syntax for DICOM", "ÃÙIuqgGfBAFchbnlpftHg]\¶", "]\¶" }, 
{ "1.2.840.10008.1.2.1", "Explicit VR Little Endian", "¾¦IuqgGfBA", "]\¶" }, 
{ "1.2.840.10008.1.2.2", "Explicit VR Big Endian", "¾¦IuqrbNGfBA", "]\¶" }, 
{ "1.2.840.10008.1.2.4.50", "JPEG Baseline (Process 1): Default Transfer Syntax for Lossy JPEG 8 Bit Image Compression", "JPEG îi 1jFñÂtJPEG 8 rbgæ³kpftHg]\¶", "]\¶" }, 
{ "1.2.840.10008.1.2.4.51", "JPEG Extended (Process 2 & 4): Default Transfer Syntax for Lossy JPEG 12 Bit Image Compression (Process 4 only)", "JPEG g£i2 & 4jFñÂtJPEG 12 rbgæ³kpftHg]\¶i 4 ÌÝj", "]\¶" }, 
{ "1.2.840.10008.1.2.4.52", "JPEG Extended (Process 3 & 5)", "JPEG g£i 3 & 5j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.53", "JPEG Spectral SelectionC Non-Hierarchical (Process 6 & 8)", "JPEG XyNgIðCñKwi 6 & 8j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.54", "JPEG Spectral SelectionC Non-Hierarchical (Process 7 & 9)", "JPEG XyNgIðCñKwi 7 & 9j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.55", "JPEG Full ProgressionC Non-Hierarchical (Process 10 & 12)", "JPEG SñCñKwi 10 & 12j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.56", "JPEG Full ProgressionC Non-Hierarchical (Process 11 & 13)", "JPEG SñCñKwi 11 & 13j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.57", "JPEG LosslessC Non-Hierarchical (Process 14)", "JPEG ÂtCñKwi 14j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.58", "JPEG LosslessC Non-Hierarchical (Process 15)", "JPEG ÂtCñKwi 15j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.59", "JPEG ExtendedC Hierarchical (Process 16 & 18)", "JPEG g£CKwi 16 & 18j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.60", "JPEG ExtendedC Hierarchical (Process 17 & 19)", "JPEG g£CKwi17 & 19j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.61", "JPEG Spectral SelectionC Hierarchical (Process 20 & 22)", "JPEG XyNgIðCKwi 20 & 22j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.62", "JPEG Spectral SelectionC Hierarchical (Process 21 & 23)", "JPEG XyNgIðCKwi 21 & 23j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.63", "JPEG Full ProgressionC Hierarchical (Process 24 & 26)", "JPEG SñCKwi 24 & 26j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.64", "JPEG Full ProgressionC Hierarchical (Process 25 & 27)", "JPEG SñCKwi 25 & 27j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.65", "JPEG LosslessC Hierarchical (Process 28)", "JPEG ÂtCKwi 28j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.66", "JPEG LosslessC Hierarchical (Process 29)", "JPEG ÂtCKwi 29j", "]\¶" }, 
{ "1.2.840.10008.1.2.4.70", "JPEG LosslessC Non-HierarchicalC First-Order Prediction (Process 14 [Selection Value 1]): Default Transfer Syntax for Lossless JPEG Image Compression", "JPEG ÂtCñKwCê\ªi 14 [Iðl 1]jFÂt JPEG æ³kpftHg]\¶", "]\¶" }, 
{ "1.2.840.10008.1.2.4.80", "JPEG-LS Lossless Image Compression", "JPEG-LS Âtæ³k", "]\¶" }, 
{ "1.2.840.10008.1.2.4.81", "JPEG-LS Lossy (Near-Lossless) Image Compression", "JPEG-LSñÂtiÂtjæ³k", "]\¶" }, 
{ "1.2.840.10008.1.2.5", "RLE Lossless", "RLE Ât", "]\¶" }, 
{ "1.2.840.10008.1.20.1", "Storage Commitment Push Model SOP Class", "Û¶ÏõvbVfrnoNX", "rnoNX" }, 
{ "1.2.840.10008.1.20.1.1", "Storage Commitment Push Model SOP Instance", "Û¶ÏõvbVfrnoCX^X", "ümÌrnoCX^X" }, 
{ "1.2.840.10008.1.20.2", "Storage Commitment Pull Model SOP Class", "Û¶ÏõvfrnoNX", "rnoNX" }, 
{ "1.2.840.10008.1.20.2.1", "Storage Commitment Pull Model SOP Instance", "Û¶ÏõvfrnoCX^X", "ümÌrnoCX^X" }, 
{ "1.2.840.10008.1.3.10", "Media Storage Directory Storage", "}ÌÛ¶fBNgÛ¶", "rnoNX" }, 
{ "1.2.840.10008.1.9", "Basic Study Content Notification SOP Class", "î{¸àeÊmrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.1.1", "DICOM Application Context Name", "chbnlpReLXg¼", "pReLXg¼" }, 
{ "1.2.840.10008.3.1.2.1.1", "Detached Patient Management SOP Class", "ª£³ÒÇrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.1.4", "Detached Patient Management Meta SOP Class", "ª£³ÒÇ^rnoNX", "^rnoNX" }, 
{ "1.2.840.10008.3.1.2.2.1", "Detached Visit Management SOP Class", "ª£@ÇrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.3.1", "Detached Study Management SOP Class", "ª£¸ÇrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.3.2", "Study Component Management SOP Class", "¸\¬vfÇrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.3.3", "Modality Performed Procedure Step SOP Class", "_eBÀ{Ïè±«XebvrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.3.4", "Modality Performed Procedure Step Retrieve SOP Class", "_eBÀ{Ïè±«Xebvæ¾rnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.3.5", "Modality Performed Procedure Step Notification SOP Class", "_eBÀ{Ïè±«XebvÊmrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.5.1", "Detached Results Management SOP Class", "ª£ÊÇrnoNX", "rnoNX" }, 
{ "1.2.840.10008.3.1.2.5.4", "Detached Results Management Meta SOP Class", "ª£ÊÇ^rnoNX", "^rnoNX" }, 
{ "1.2.840.10008.3.1.2.5.5", "Detached Study Management Meta SOP Class", "¸Ç^rnoNX", "^rnoNX" }, 
{ "1.2.840.10008.3.1.2.6.1", "Detached Interpretation Management SOP Class", "ª£ðßÇrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.1", "Basic Film Session SOP Class", "î{tBZbVrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.14", "Print Job SOP Class", "vgWurnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.15", "Basic Annotation Box SOP Class", "î{ß{bNXrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.16", "Printer SOP Class", "v^rnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.16.376", "Printer Configuration Retrieval SOP Class", "v^\¬æ¾rnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.17", "Printer SOP Instance", "v^rnoCX^X", "ümÌv^rnoCX^X" }, 
{ "1.2.840.10008.5.1.1.17.376", "Printer Configuration Retrieval SOP Instance", "v^\¬æ¾rnoCX^X", "ümÌv^rnoCX^X" }, 
{ "1.2.840.10008.5.1.1.18", "Basic Color Print Management Meta SOP Class", "î{J[vgÇ^rnoNX", "^rnoNX" }, 
{ "1.2.840.10008.5.1.1.18.1", "Referenced Color Print Management Meta SOP Class (Retired)", "QÆJ[vgÇ^rnoNXiÞðj", "^rnoNX" }, 
{ "1.2.840.10008.5.1.1.2", "Basic Film Box SOP Class", "î{tB{bNXrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.22", "VOI LUT Box SOP Class", "unh kts{bNXrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.23", "Presentation LUT SOP Class", "ñ¦kts rnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.24", "Image Overlay Box SOP Class (Retired)", "æI[oC{bNXrnoNXiÞðj", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.24.1", "Basic Print Image Overlay Box SOP Class", "î{vgæI[oC{bNXrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.25", "Print Queue SOP Instance", "vgL[rnoCX^X", "ümÌvgL[rnoCX^X" }, 
{ "1.2.840.10008.5.1.1.26", "Print Queue Management SOP Class", "vgL[ÇrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.27", "Stored Print Storage SOP Class", "Û¶vgÛ¶rnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.29", "Hardcopy  Grayscale Image Storage SOP Class", "n[hRs[O[XP[æÛ¶rnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.30", "Hardcopy Color Image Storage SOP Class", "n[hRs[J[æÛ¶rnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.31", "Pull Print Request SOP Class", "vvgËrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.32", "Pull Stored Print Management Meta SOP Class", "vÛ¶vgÇ^rnoNX", "^rnoNX" }, 
{ "1.2.840.10008.5.1.1.4", "Basic Grayscale Image Box SOP Class", "î{O[XP[æ{bNXrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.4.1", "Basic Color Image Box SOP Class", "î{J[æ{bNXrnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.4.2", "Referenced Image Box SOP Class (Retired)", "QÆæ{bNXrnoNXiÞðj", "rnoNX" }, 
{ "1.2.840.10008.5.1.1.9", "Basic Grayscale Print Management Meta SOP Class", "î{O[XP[vgÇ^rnoNX", "^rnoNX" }, 
{ "1.2.840.10008.5.1.1.9.1", "Referenced Grayscale Print Management Meta SOP Class (Retired)", "QÆO[XP[vgÇ^rnoNXiÞðj", "^rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.1", "Computed Radiography Image Storage", "Rs[ebhWIOtBæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.1.1", "Digital X-Ray Image Storage - For Presentation", "fW^XüæÛ¶|ñ¦p", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.1.1.1", "Digital X-Ray Image Storage - For Processing", "fW^XüæÛ¶|p", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.1.2", "Digital Mammography X-Ray Image Storage - For Presentation", "fW^}OtBæÛ¶|ñ¦p", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.1.2.1", "Digital Mammography X-Ray Image Storage - For Processing", "fW^}OtBæÛ¶|p", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.1.3", "Digital Intra-oral X-Ray Image Storage - For Presentation", "fW^ûoàXüæÛ¶|ñ¦p", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.1.3.1", "Digital Intra-oral X-Ray Image Storage - For Processing", "fW^ûoàXüæÛ¶|p", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.10", "Standalone Modality LUT Storage", "Æ§_eBktsÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.11", "Standalone VOI LUT Storage", "Æ§unh ktsÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.11.1", "Grayscale Softcopy Presentation State Storage SOP Class", "O[XP[\tgRs[ñ¦óÔÛ¶rnoNX", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.12.1", "X-Ray Angiographic Image Storage", "wüAMIOtBæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.12.2", "X-Ray Radiofluoroscopic Image Storage", "wü§BeæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.12.3", "X-Ray Angiographic Bi-Plane Image Storage (Retired)", "wüAMIOtBoCv[æÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.128", "Positron Emission Tomography Image Storage", "|WgG~bVgOtBæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.129", "Standalone PET Curve Storage", "Æ§odsJ[uÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.2", "CT Image Storage", "bsæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.20", "Nuclear Medicine Image  Storage", "jãwæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.3", "Ultrasound Multi-frame Image Storage (Retired)", "´¹g¡t[æÛ¶iÞðj", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.3.1", "Ultrasound Multi-frame Image Storage", "´¹g¡t[æÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.4", "MR Image Storage", "lqæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.481.1", "RT Image Storage", "qsæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.481.2", "RT Dose Storage", "qsüÊÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.481.3", "RT Structure Set Storage", "qs\¢WÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.481.4", "RT Beams Treatment Record Storage", "qsr[¡ÃL^Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.481.5", "RT Plan Storage", "qsvæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.481.6", "RT Brachy Treatment Record Storage", "qsßÚÆË¡ÃL^Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.481.7", "RT Treatment Summary Record Storage", "qsr[¡ÃL^Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.5", "Nuclear Medicine Image  Storage (Retired)", "jãwæÛ¶iÞðj", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.6", "Ultrasound Image Storage (Retired)", "´¹gæÛ¶iÞðj", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.6.1", "Ultrasound Image Storage", "´¹gæÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.7", "Secondary Capture Image Storage", "ñæ¾æÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.77.1", "VL Image Storage (Retired)", "", "" }, 
{ "1.2.840.10008.5.1.4.1.1.77.1.1", "VL Endoscopic Image Storage", "ukà¾æÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.77.1.2", "VL Microscopic Image Storage", "uk°÷¾æÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.77.1.3", "VL Slide-Coordinates Microscopic Image Storage", "ukXChÀW÷¾æÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.77.1.4", "VL Photographic Image Storage", "ukÊ^æÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.77.2", "VL Multi-frame Image Storage (Retired)", "", "" }, 
{ "1.2.840.10008.5.1.4.1.1.8", "Standalone Overlay  Storage", "Æ§I[oCÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.88.11", "Basic Text SR", "î{eLXgrq", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.88.22", "Enhanced SR", "g£rq", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.88.33", "Comprehensive SR", "ïIrq", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.9", "Standalone Curve Storage", "Æ§J[uÛ¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.9.1.1", "12-lead ECG Waveform Storage", "PQ[hdbfg`Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.9.1.2", "General ECG Waveform Storage", "êÊdbfg`Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.9.1.3", "Ambulatory ECG Waveform Storage", "àsdbfg`Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.9.2.1", "Hemodynamic Waveform Storage", "s®Ôg`Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.9.3.1", "Cardiac Electrophysiology Waveform Storage", "SdC¶wg`Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.1.9.4.1", "Basic Voice Audio Waveform Storage", "î{¹ºI[fBIg`Û¶", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.1.1", "Patient Root Query/Retrieve Information Model - FIND", "³Ò[gâ¹^æ¾îñf ? FIND", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.1.2", "Patient Root Query/Retrieve Information Model - MOVE", "³Ò[gâ¹^æ¾îñf ? MOVE", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.1.3", "Patient Root Query/Retrieve Information Model - GET", "³Ò[gâ¹^æ¾îñf ? GET", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.2.1", "Study Root Query/Retrieve Information Model - FIND", "¸[gâ¹^æ¾îñf ? FIND", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.2.2", "Study Root Query/Retrieve Information Model - MOVE", "¸[gâ¹^æ¾îñf ? MOVE", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.2.3", "Study Root Query/Retrieve Information Model - GET", "¸[gâ¹^æ¾îñf GET", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.3.1", "Patient/Study Only Query/Retrieve Information Model  - FIND", "³Ò^¸ÌÝâ¹^æ¾îñf - FIND", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.3.2", "Patient/Study Only Query/Retrieve Information Model  - MOVE", "³Ò^¸ÌÝâ¹^æ¾îñf - MOVE", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.1.2.3.3", "Patient/Study Only Query/Retrieve Information Model  - GET", "³Ò^¸ÌÝâ¹^æ¾îñf - GET", "rnoNX" }, 
{ "1.2.840.10008.5.1.4.31", "Modality Worklist Information Model - FIND", "_eB[NXgîñf ? FIND", "rnoNX" }
};
inline void initializeUID( array<dicomUID> & values )
{
	values.allocate( sizeof( dicomUIDTable ) / sizeof( dicomUIDTable[0] ) );
	memcpy( values.data, dicomUIDTable, sizeof( dicomUIDTable ) );
}
