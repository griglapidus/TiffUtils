#include "mainwindow.h"
#include <QLayout>
#include <QImageReader>
#include <QHeaderView>
#include <QApplication>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_tableViev(new QTableView(this)),
    m_filePath(new QLineEdit(this)),
    m_openFileBtn(new QPushButton("Open", this)),
    m_getFileName(new QPushButton(qApp->style()->standardIcon(QStyle::SP_ArrowLeft), "", this)),
    m_loadAllTagsCheck(new QCheckBox("Show all tags",this)),
    m_saveFileBtn(new QPushButton("Save", this))
//m_imageView(new QGraphicsView(this)),
//m_description(new QTextEdit(this)),
//m_loadPreviewCheck(new QCheckBox("Load preview?",this))
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *topLineLayout = new QHBoxLayout();
    topLineLayout->addWidget(m_filePath);
    topLineLayout->addWidget(m_getFileName);
    topLineLayout->addWidget(m_openFileBtn);
    //QHBoxLayout *imageLineLayout = new QHBoxLayout();
    // imageLineLayout->addWidget(m_description);
    // imageLineLayout->addWidget(m_imageView);
    // mainLayout->addWidget(m_loadPreviewCheck);
    mainLayout->addLayout(topLineLayout);
    QHBoxLayout *saveLineLayout = new QHBoxLayout();
    saveLineLayout->addWidget(m_loadAllTagsCheck);
    saveLineLayout->addStretch(1);
    saveLineLayout->addWidget(m_saveFileBtn);
    mainLayout->addLayout(saveLineLayout);
    //mainLayout->addLayout(imageLineLayout);
    mainLayout->addWidget(m_tableViev);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    m_tableViev->setModel(&m_model);
    m_tableViev->resizeColumnsToContents();
    //m_imageView->setScene(&m_imageScene);
    //m_imageView->setFixedSize(150, 150);
    //m_description->setFixedHeight(150);
    //m_loadPreviewCheck->setCheckState(Qt::Unchecked);

    // m_loadPreviewCheck->setVisible(false);
    // m_description->setVisible(false);
    // m_imageView->setVisible(false);
    m_tableViev->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableViev->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableViev->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    QObject::connect(m_openFileBtn, SIGNAL(clicked(bool)), this, SLOT(openTiff()));
    QObject::connect(m_saveFileBtn, SIGNAL(clicked(bool)), this, SLOT(saveTiff()));
    QObject::connect(m_getFileName, SIGNAL(clicked(bool)), this, SLOT(getFileName()));

    QObject::connect(m_loadAllTagsCheck, SIGNAL(stateChanged(int)), &m_model, SLOT(setShowAllTags(int)));
    m_loadAllTagsCheck->setChecked(false);
}

MainWindow::~MainWindow() {
    closeTiff();
}

void MainWindow::openTiff()
{
    QString path = m_filePath->text();
    if(!path.size()) {
        return;
    }
    m_tiffFile = TIFFOpen(path.toLocal8Bit().data(), "r+");
    m_model.loadFromTiff(m_tiffFile);

    if (m_tiffFile && false) { //m_loadPreviewCheck->isChecked()) {
        // uint32_t height, width;
        // uint32_t roesPerStrip;
        // uint32_t row;
        // uint16_t compression, config;
        // uint8_t* raster;
        // uint64_t npixels;

        // TIFFGetField(m_tiffFile, TIFFTAG_IMAGEWIDTH, &width);
        // TIFFGetField(m_tiffFile, TIFFTAG_IMAGELENGTH, &height);
        // TIFFGetField(m_tiffFile, TIFFTAG_ROWSPERSTRIP, &roesPerStrip);
        // TIFFGetField(m_tiffFile, TIFFTAG_COMPRESSION, &compression);
        // TIFFGetField(m_tiffFile, TIFFTAG_PLANARCONFIG, &config);

        // npixels = width * height;
        // if(roesPerStrip) {
        //     uint32_t strip_size = width * roesPerStrip * sizeof (uint32_t);
        //     uint32_t stripsCount = floor ((height + roesPerStrip - 1) / roesPerStrip);
        //     raster = (uint8_t*) _TIFFmalloc(strip_size * stripsCount * sizeof (uint32_t));
        //     memset(raster, 0, strip_size * stripsCount * sizeof (uint32_t));
        //     for (uint32_t strip = 0; strip < stripsCount; strip++) {
        //         if (TIFFReadRGBAStripExt(m_tiffFile, strip, (uint32_t*)(raster + strip * strip_size), 0) == -1) {
        //             fprintf(stderr, "Error reading strip %u\n", strip);
        //             return;
        //         }
        //     }
        // } else {
        //     raster = (uint8_t*) _TIFFmalloc(npixels * sizeof (uint32_t));
        //     memset(raster, 0, npixels * sizeof (uint32_t));
        //     if (!TIFFReadRGBAImage(m_tiffFile, width, height, (uint32_t*)raster, 1)) {
        //         return;
        //     }
        // }
        //QImage img(raster, width, height, QImage::Format_ARGB32);
        //m_imageView->resetCachedContent();
        //auto formats = QImageReader::supportedImageFormats();
        //QImage img(path);
        //QPixmap pixmap = QPixmap::fromImage(img);
        //m_imageScene.addPixmap(pixmap);
        //m_imageView->fitInView(m_imageScene.sceneRect(), Qt::KeepAspectRatio);
        //_TIFFfree(raster);
    }
}

void MainWindow::getFileName()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("Images (*.tif *.tiff)"));
    if(!fileName.isEmpty()) {
        m_filePath->setText(fileName);
    }
    if(m_filePath->text().size()) {
        openTiff();
    }
}

void MainWindow::saveTiff()
{
    m_model.SaveToTiff();
}

void MainWindow::closeTiff()
{
    if(m_tiffFile) {
        m_model.closeTiff();
        TIFFClose(m_tiffFile);
        m_tiffFile = nullptr;
    }
}
