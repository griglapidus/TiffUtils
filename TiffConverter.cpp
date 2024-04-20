#include "TiffConverter.h"
#include <tiffio.h>
#include <QFileInfo>
#include <QDir>
#include <QFile>

TiffConverter::TiffConverter(QObject *parent)
    : QObject{parent}
{}

void TiffConverter::ConvertTiff(QStringList inFiles, QString outputFolder, int targetValue)
{
    m_stopFlag = false;
    if(!inFiles.size()){
        return;
    }
    int count = inFiles.size();
    int curIndex = 0;
    for(auto& file: inFiles) {
        if(m_stopFlag)
            break;
        QFileInfo info(file);
        QString fileName = info.fileName();
        QString outputFilePath;
        if(outputFolder.size()) {
            QDir dir(outputFolder);
            outputFilePath = dir.absoluteFilePath(fileName);
        } else {
            QDir dir(info.absolutePath());
            dir.mkdir("Output_2Bit");
            dir.cd("Output_2Bit");
            outputFilePath = dir.absoluteFilePath(fileName);
        }
        QFile fFile(outputFilePath);
        if (fFile.exists())
            fFile.remove();

        ConvertTiff(file, outputFilePath, targetValue);
        curIndex++;
        emit progressSignal(100 * curIndex / count);
    }
}

void TiffConverter::stopProcess()
{
    m_stopFlag = true;
}

void TiffConverter::ConvertTiff(QString inFile, QString outFile, int targetValue)
{
    TIFF* inTiffImage = TIFFOpen(inFile.toLocal8Bit(), "r");
    if (inTiffImage == NULL)
        return;

    int nWidth = 0, nHeight = 0, nRowsPerStrip = 0;
    qint16 nInBpp, nInSpp, nInPlanConf, nInPhotomrtric, nInComp;
    nInBpp = nInSpp = nInPlanConf = nInPhotomrtric = nInComp = 0;
    float fXRes = 0, fYRes = 0;
    // Configure the tiff parameters (use 8 bits palette configuration)
    TIFFGetField(inTiffImage, TIFFTAG_IMAGEWIDTH, &nWidth);
    TIFFGetField(inTiffImage, TIFFTAG_IMAGELENGTH, &nHeight);
    TIFFGetField(inTiffImage, TIFFTAG_XRESOLUTION, &fXRes);
    TIFFGetField(inTiffImage, TIFFTAG_YRESOLUTION, &fYRes);
    TIFFGetField(inTiffImage, TIFFTAG_SAMPLESPERPIXEL, &nInSpp);
    TIFFGetField(inTiffImage, TIFFTAG_BITSPERSAMPLE, &nInBpp);
    TIFFGetField(inTiffImage, TIFFTAG_PLANARCONFIG, &nInPlanConf);
    TIFFGetField(inTiffImage, TIFFTAG_PHOTOMETRIC, &nInPhotomrtric);
    TIFFGetField(inTiffImage, TIFFTAG_COMPRESSION, &nInComp);
    TIFFGetField(inTiffImage, TIFFTAG_ROWSPERSTRIP, &nRowsPerStrip);

    if(nInBpp != 1)
        return;

    TIFF* outTiffImage = TIFFOpen(outFile.toLocal8Bit(), "w+");
    if (outTiffImage == NULL)
        return;

    TIFFSetField(outTiffImage, TIFFTAG_IMAGEWIDTH, nWidth);
    TIFFSetField(outTiffImage, TIFFTAG_IMAGELENGTH, nHeight);
    TIFFSetField(outTiffImage, TIFFTAG_XRESOLUTION, fXRes);
    TIFFSetField(outTiffImage, TIFFTAG_YRESOLUTION, fYRes);
    TIFFSetField(outTiffImage, TIFFTAG_SAMPLESPERPIXEL, nInSpp);
    TIFFSetField(outTiffImage, TIFFTAG_BITSPERSAMPLE, 2);
    TIFFSetField(outTiffImage, TIFFTAG_PLANARCONFIG, nInPlanConf);
    TIFFSetField(outTiffImage, TIFFTAG_PHOTOMETRIC, nInPhotomrtric);
    TIFFSetField(outTiffImage, TIFFTAG_COMPRESSION, nInComp);
    TIFFSetField(outTiffImage, TIFFTAG_ROWSPERSTRIP, nRowsPerStrip);


    unsigned allocatedSize256In = (nWidth + 31) / 32;
    unsigned allocatedSize256Out = allocatedSize256In * 2;

    const __m256i swapOddEvenBytesMask = _mm256_setr_epi8(
        1, 0, 3, 2, 5, 4, 7, 6,
        9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6,
        9, 8, 11, 10, 13, 12, 15, 14
        );

    uint64_t exendedMask = 0x5555555555555555; // 0b0101....

    std::vector<__m256i> inLineData(allocatedSize256In);
    std::vector<__m256i> outLineData(allocatedSize256Out);
    std::vector<__m256i> emptyData(allocatedSize256Out);

    std::vector<__m256i> &leftData = targetValue == 1 ? emptyData : outLineData;
    std::vector<__m256i> &rightData = targetValue == 2 ? emptyData : outLineData;

    for (int i = 0; i < nHeight; i++)
    {
        TIFFReadScanline(inTiffImage,(qint8*)inLineData.data(), i);
        __int32 *in = (__int32*)inLineData.data();
        __int64 *out = (__int64*)outLineData.data();
        for (int j = 0; j < inLineData.size() * 8; ++j) {
            out[j] = _pdep_u64(in[j], exendedMask);
        }
        for (int j = 0; j < outLineData.size(); ++j) {
            outLineData[j] = _mm256_or_si256(_mm256_slli_epi64(leftData[j], 1), rightData[j]);
            outLineData[j] = _mm256_shuffle_epi8(outLineData[j], swapOddEvenBytesMask); // It's needed to swap odd and even bytes due to extension
        }
        TIFFWriteScanline(outTiffImage, outLineData.data(), i, 0);
    }

    TIFFClose(inTiffImage);
    inTiffImage = NULL;
    TIFFClose(outTiffImage);
    outTiffImage = NULL;
}
