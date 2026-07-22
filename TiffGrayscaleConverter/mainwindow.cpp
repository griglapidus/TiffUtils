#include "mainwindow.h"
#include <QLabel>
#include <QApplication>
#include <QLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QHeaderView>
#include <QDoubleValidator>
#include <limits>

// 1 << bits overflows/UB once bits reaches the width of int (32), and a 32-bit unsigned range
// (up to 4294967295) doesn't fit in QSpinBox's int-based API anyway, so clamp to INT_MAX.
static int maxSpinBoxValueForBits(int bits)
{
    qint64 maxValue = (qint64(1) << bits) - 1;
    return static_cast<int>(std::min<qint64>(maxValue, std::numeric_limits<int>::max()));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_converterThread(new QThread()),
    m_tiffConverter(new TiffConverter()),
    m_inputGroupBox(new QGroupBox("Input", this)),
    m_filesTableView(new QTableView(this)),
    m_addFilesBtn(new QPushButton("Add Files...", this)),
    m_addFolderBtn(new QPushButton("Add Folder...", this)),
    m_removeBtn(new QPushButton("Remove", this)),
    m_removeAllBtn(new QPushButton("Remove all", this)),
    m_outputGroupBox(new QGroupBox("Output", this)),
    m_useOutSubDirCheck(new QCheckBox("Save to Output subdirectory", this)),
    m_openOutputOnFinish(new QCheckBox("Open Output folder(s)", this)),
    m_outputPath(new QLineEdit(this)),
    m_getOutputFolderBtn(new QPushButton("...", this)),
    m_outputBitDepth(new QComboBox(this)),
    m_targetPixValue(new QComboBox(this)),
    m_targetPixValueSpin(new QSpinBox(this)),
    m_negativeCheck(new QCheckBox("Negative (invert image)", this)),
    m_changeResolutionCheck(new QCheckBox("Change resolution", this)),
    m_xResolution(new QLineEdit("500", this)),
    m_yResolution(new QLineEdit("600", this)),
    m_processBtn(new QPushButton("Process")),
    m_stopBtn(new QPushButton("Stop")),
    m_totalProgressBar(new QProgressBar(this))
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QVBoxLayout *inputLayout = new QVBoxLayout(m_inputGroupBox);
    inputLayout->addWidget(m_filesTableView);
    QGridLayout *inputBtnsGrid = new QGridLayout();
    inputBtnsGrid->addWidget(m_addFilesBtn, 0,0);
    inputBtnsGrid->addWidget(m_addFolderBtn, 1,0);
    inputBtnsGrid->addWidget(m_removeBtn, 0,2);
    inputBtnsGrid->addWidget(m_removeAllBtn, 1,2);
    inputBtnsGrid->setColumnStretch(1, 1);
    inputLayout->addLayout(inputBtnsGrid);
    m_inputGroupBox->setLayout(inputLayout);
    mainLayout->addWidget(m_inputGroupBox);

    QVBoxLayout *outputLayout = new QVBoxLayout(m_outputGroupBox);
    QHBoxLayout *outputPathLayout = new QHBoxLayout();
    outputPathLayout->addWidget(new QLabel("Output path", this));
    outputPathLayout->addWidget(m_outputPath);
    outputPathLayout->addWidget(m_getOutputFolderBtn);
    outputLayout->addLayout(outputPathLayout);
    outputLayout->addWidget(m_useOutSubDirCheck);
    outputLayout->addWidget(m_openOutputOnFinish);
    QHBoxLayout *bitDepthLayout = new QHBoxLayout();
    bitDepthLayout->addWidget(new QLabel("Output bit depth", this));
    bitDepthLayout->addWidget(m_outputBitDepth);
    outputLayout->addLayout(bitDepthLayout);
    m_targetPixValueLabel = new QLabel("Set-pixel value (1-bit sources only)", this);
    QHBoxLayout *pixValLayout = new QHBoxLayout();
    pixValLayout->addWidget(m_targetPixValueLabel);
    pixValLayout->addWidget(m_targetPixValue);
    pixValLayout->addWidget(m_targetPixValueSpin);
    outputLayout->addLayout(pixValLayout);
    outputLayout->addWidget(m_negativeCheck);
    QHBoxLayout *resolutionLayout = new QHBoxLayout();
    resolutionLayout->addWidget(m_changeResolutionCheck);
    resolutionLayout->addWidget(new QLabel("XRes", this));
    resolutionLayout->addWidget(m_xResolution);
    resolutionLayout->addWidget(new QLabel("YRes", this));
    resolutionLayout->addWidget(m_yResolution);
    outputLayout->addLayout(resolutionLayout);
    m_outputGroupBox->setLayout(outputLayout);
    mainLayout->addWidget(m_outputGroupBox);
    QHBoxLayout *startStopLayout = new QHBoxLayout();
    startStopLayout->addWidget(m_processBtn);
    startStopLayout->addWidget(m_stopBtn);
    mainLayout->addLayout(startStopLayout);
    mainLayout->addWidget(m_totalProgressBar);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    m_xResolution->setValidator(new QDoubleValidator(0.0, 10000.0, 4, m_xResolution));
    m_yResolution->setValidator(new QDoubleValidator(0.0, 10000.0, 4, m_yResolution));
    m_xResolution->setEnabled(false);
    m_yResolution->setEnabled(false);

    m_filesTableView->setModel(&m_filesModel);
    m_filesTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_filesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_filesTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    m_outputBitDepth->addItem("1 bit", 1);
    m_outputBitDepth->addItem("2 bit", 2);
    m_outputBitDepth->addItem("4 bit", 4);
    m_outputBitDepth->addItem("8 bit", 8);
    m_outputBitDepth->addItem("16 bit", 16);
    m_outputBitDepth->addItem("32 bit", 32);
    m_outputBitDepth->setCurrentIndex(1); // 2 bit, matches the previous fixed output depth

    m_targetPixValue->addItems({"Small", "Medium", "Large"});
    m_targetPixValueSpin->setValue(1);
    QString targetPixValueTip = "Only used when a source image is 1-bit: input 0 always maps to "
                                 "output 0, this picks the output level for input 1. 2-bit and 8-bit "
                                 "sources are converted automatically instead (rescaled when going to "
                                 "a wider output, reduced to the most significant bits when going to a "
                                 "narrower one).";
    m_targetPixValueLabel->setToolTip(targetPixValueTip);
    m_targetPixValue->setToolTip(targetPixValueTip);
    m_targetPixValueSpin->setToolTip(targetPixValueTip);

    m_totalProgressBar->setMaximum(100);

    m_tiffConverter->moveToThread(m_converterThread);

    setMinimumWidth(600);

    QObject::connect(m_addFilesBtn, SIGNAL(clicked(bool)), this, SLOT(addFiles()));
    QObject::connect(m_addFolderBtn, SIGNAL(clicked(bool)), this, SLOT(addFolder()));
    QObject::connect(m_removeBtn, SIGNAL(clicked(bool)), this, SLOT(removeFile()));
    QObject::connect(m_removeAllBtn, SIGNAL(clicked(bool)), this, SLOT(removeAll()));
    QObject::connect(m_getOutputFolderBtn, SIGNAL(clicked(bool)), this, SLOT(getOutputFolder()));

    QObject::connect(m_processBtn, SIGNAL(clicked(bool)), this, SLOT(ConvertTiff()));
    QObject::connect(m_stopBtn, SIGNAL(clicked(bool)), m_tiffConverter, SLOT(stopProcess()), Qt::DirectConnection);
    QObject::connect(m_useOutSubDirCheck, SIGNAL(stateChanged(int)), this, SLOT(onUseOutSubDirCheck(int)));
    QObject::connect(m_changeResolutionCheck, SIGNAL(stateChanged(int)), this, SLOT(onChangeResolutionCheck(int)));
    QObject::connect(m_outputBitDepth, SIGNAL(currentIndexChanged(int)), this, SLOT(onOutputBitDepthChanged(int)));
    QObject::connect(this, SIGNAL(ConvertTiffSignal(QStringList,QString,int,int,bool,bool)), m_tiffConverter, SLOT(ConvertTiff(QStringList,QString,int,int,bool,bool)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(setRes(double,double)), m_tiffConverter, SLOT(setRes(double,double)), Qt::QueuedConnection);
    QObject::connect(m_tiffConverter, SIGNAL(progressSignal(quint32)), this, SLOT(setProgress(quint32)), Qt::QueuedConnection);
    QObject::connect(m_tiffConverter, SIGNAL(finished()), this, SLOT(finished()), Qt::QueuedConnection);
    QObject::connect(m_tiffConverter, SIGNAL(showMsg(QString)), this, SLOT(showMsg(QString)), Qt::QueuedConnection);
    m_useOutSubDirCheck->setChecked(true);
    m_changeResolutionCheck->setChecked(false);
    updateTargetPixValueState();
    m_converterThread->start();
    resize(650, 500);
}

MainWindow::~MainWindow() {
}

void MainWindow::ConvertTiff()
{
    QStringList files = m_filesModel.files();
    if(!files.size()) return;
    bool useOutputSubFolder = m_useOutSubDirCheck->isChecked();
    QString outputPath = "";
    if(!useOutputSubFolder) {
        outputPath = m_outputPath->text();
        if(!outputPath.size()) {
            QMessageBox msgBox;
            msgBox.setText("Output folder should be setted, or use Save to Output subdirectory.");
            msgBox.exec();
            return;
        }
    }
    if(m_changeResolutionCheck) {
        emit setRes(m_xResolution->text().toDouble(), m_yResolution->text().toDouble());
    } else {
        emit setRes(0, 0);
    }

    setProgress(0);
    m_processBtn->setEnabled(false);
    int outputBitDepth = m_outputBitDepth->currentData().toInt();
    int targetValue = outputBitDepth > 2 ? m_targetPixValueSpin->value() : (m_targetPixValue->currentIndex() + 1);
    emit ConvertTiffSignal(files, outputPath, outputBitDepth, targetValue, m_negativeCheck->isChecked(), m_openOutputOnFinish->isChecked());
}

void MainWindow::addFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                                        "",
                                                        tr("Images (*.tif *.tiff)"));
    m_filesModel.openTiff(files);
    updateTargetPixValueState();
}

void MainWindow::addFolder()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Open File"));
    QDir dir(path);
    QFileInfoList filesInfo = dir.entryInfoList({"*.tif", "*.tiff"});
    QStringList files;
    for(auto &info: filesInfo) {
        files.append(info.absoluteFilePath());
    }
    m_filesModel.openTiff(files);
    updateTargetPixValueState();
}

void MainWindow::removeFile()
{
    auto indexList = m_filesTableView->selectionModel()->selectedIndexes();
    QVector<unsigned> rows;
    for(auto& index: indexList) {
        rows.append(index.row());
    }
    std::sort(rows.begin(), rows.end());
    for(int i = rows.size() - 1; i >= 0; --i) {
        m_filesModel.removeRows(rows[i], 1);
    }
    updateTargetPixValueState();
}

void MainWindow::removeAll()
{
    m_filesModel.removeAll();
    updateTargetPixValueState();
}

void MainWindow::getOutputFolder()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Open File"));
    if(path.size()) {
        m_outputPath->setText(path);
    }
}

void MainWindow::onUseOutSubDirCheck(int checked)
{
    m_outputPath->setEnabled(checked == 0);
    m_getOutputFolderBtn->setEnabled(checked == 0);
}

void MainWindow::onChangeResolutionCheck(int checked)
{
    m_xResolution->setEnabled(checked != 0);
    m_yResolution->setEnabled(checked != 0);
}

void MainWindow::onOutputBitDepthChanged(int index)
{
    Q_UNUSED(index);
    updateTargetPixValueState();
}

void MainWindow::updateTargetPixValueState()
{
    int outputBitDepth = m_outputBitDepth->currentData().toInt();
    bool enabled = outputBitDepth > 1 && m_filesModel.bitDepths().contains(1);
    bool useSpin = outputBitDepth > 2;

    m_targetPixValueSpin->setRange(0, maxSpinBoxValueForBits(outputBitDepth));
    m_targetPixValue->setVisible(!useSpin);
    m_targetPixValueSpin->setVisible(useSpin);

    m_targetPixValueLabel->setEnabled(enabled);
    m_targetPixValue->setEnabled(enabled);
    m_targetPixValueSpin->setEnabled(enabled);
}

void MainWindow::setProgress(quint32 progressVal)
{
    m_totalProgressBar->setValue(progressVal);
}

void MainWindow::finished()
{
    m_processBtn->setEnabled(true);
}

void MainWindow::showMsg(QString msg)
{
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.exec();
}
