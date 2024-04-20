#include "mainwindow.h"
#include <tiffio.h>
#include <QLabel>
#include <QApplication>
#include <QLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_convertAllFilesCheck(new QCheckBox("Convert all files in folder", this)),
    m_useOutSubDirCheck(new QCheckBox("Save to Output subdirectory", this)),
    m_filePath(new QLineEdit(this)),
    m_openBtn(new QPushButton(qApp->style()->standardIcon(QStyle::SP_ArrowLeft), "", this)),
    m_outputPath(new QLineEdit(this)),
    m_targetPixValue(new QComboBox(this)),
    m_processBtn(new QPushButton("Process")),
    m_totalProgressBar(new QProgressBar(this)),
    m_fileProgressBar(new QProgressBar(this))
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *checkLayout = new QHBoxLayout();
    checkLayout->addWidget(m_convertAllFilesCheck);
    checkLayout->addWidget(m_useOutSubDirCheck);
    QHBoxLayout *filePathLayout = new QHBoxLayout();
    filePathLayout->addWidget(new QLabel("File's path", this));
    filePathLayout->addWidget(m_filePath);
    filePathLayout->addWidget(m_openBtn);
    QHBoxLayout *outputPathLayout = new QHBoxLayout();
    outputPathLayout->addWidget(new QLabel("Output path", this));
    outputPathLayout->addWidget(m_outputPath);
    QHBoxLayout *pixValLayout = new QHBoxLayout();
    pixValLayout->addWidget(new QLabel("Target pixels value", this));
    pixValLayout->addWidget(m_targetPixValue);
    mainLayout->addLayout(checkLayout);
    mainLayout->addLayout(filePathLayout);
    mainLayout->addLayout(outputPathLayout);
    mainLayout->addLayout(pixValLayout);
    mainLayout->addWidget(m_processBtn);
    mainLayout->addWidget(m_totalProgressBar);
    mainLayout->addWidget(m_fileProgressBar);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    QObject::connect(m_openBtn, SIGNAL(clicked(bool)), this, SLOT(getFileName()));
    QObject::connect(m_processBtn, SIGNAL(clicked(bool)), this, SLOT(ConvertTiff()));
    QObject::connect(m_useOutSubDirCheck, SIGNAL(stateChanged(int)), this, SLOT(onUseOutSubDirCheck(int)));
}

MainWindow::~MainWindow() {}

void MainWindow::ConvertTiff()
{
    ConvertTiff("C:/work/1/in.tif", "C:/work/1/out.tif", 3);
}

void MainWindow::getFileNames() {
    bool convFullDir = m_convertAllFilesCheck->isChecked();
    m_files.clear();
    if(convFullDir) {
        QString path = QFileDialog::getExistingDirectory(this, tr("Open File"));
        QDir dir(path);
        m_files = dir.entryList({"*.tif", "*.tiff"});
    } else {
        m_files = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                            "",
                                            tr("Images (*.tif *.tiff)"));
    }
}

void MainWindow::getFileNames(QString path)
{

}

void MainWindow::onUseOutSubDirCheck(int checked)
{
    m_outputPath->setReadOnly(checked);
    QString inFilePath = m_filePath->text();
    if(!inFilePath.size())
        return;
    QFileInfo fileInfo(inFilePath);
    QDir dir = fileInfo.absoluteDir();
    dir.mkdir("Output");
    dir.cd("Output");
    m_outputPath->setText(dir.absolutePath());
}

void MainWindow::ConvertTiff(QString inFile, QString outFile, int targetValue)
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

    TIFF* outTiffImage = TIFFOpen(outFile.toLocal8Bit(), "w");
    if (inTiffImage == NULL)
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
