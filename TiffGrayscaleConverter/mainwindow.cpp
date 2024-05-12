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
    m_targetPixValue(new QComboBox(this)),
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
    QHBoxLayout *pixValLayout = new QHBoxLayout();
    pixValLayout->addWidget(new QLabel("Target pixels value", this));
    pixValLayout->addWidget(m_targetPixValue);
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

    m_targetPixValue->addItems({"Small","Medium","Large"});

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
    QObject::connect(this, SIGNAL(ConvertTiffSignal(QStringList,QString,int,bool,bool)), m_tiffConverter, SLOT(ConvertTiff(QStringList,QString,int,bool,bool)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(setRes(double,double)), m_tiffConverter, SLOT(setRes(double,double)), Qt::QueuedConnection);
    QObject::connect(m_tiffConverter, SIGNAL(progressSignal(quint32)), this, SLOT(setProgress(quint32)), Qt::QueuedConnection);
    QObject::connect(m_tiffConverter, SIGNAL(finished()), this, SLOT(finished()), Qt::QueuedConnection);
    QObject::connect(m_tiffConverter, SIGNAL(showMsg(QString)), this, SLOT(showMsg(QString)), Qt::QueuedConnection);
    m_useOutSubDirCheck->setChecked(true);
    m_changeResolutionCheck->setChecked(false);
    m_converterThread->start();
    resize(650, 500);
}

MainWindow::~MainWindow() {
}

void MainWindow::ConvertTiff()
{
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
    emit ConvertTiffSignal(m_filesModel.files(), outputPath, m_targetPixValue->currentIndex() + 1, m_negativeCheck->isChecked(), m_openOutputOnFinish->isChecked());
}

void MainWindow::addFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                                        "",
                                                        tr("Images (*.tif *.tiff)"));
    m_filesModel.openTiff(files);
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
}

void MainWindow::removeAll()
{
    m_filesModel.removeAll();
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
