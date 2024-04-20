#include "mainwindow.h"
#include <QLabel>
#include <QApplication>
#include <QLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_converterThread(new QThread()),
    m_tiffConverter(new TiffConverter()),
    m_inputGroupBox(new QGroupBox("Input", this)),
    m_filesListView(new QListView(this)),
    m_addFilesBtn(new QPushButton("Add Files...", this)),
    m_addFolderBtn(new QPushButton("Add Folder...", this)),
    m_removeBtn(new QPushButton("Remove", this)),
    m_removeAllBtn(new QPushButton("Remove all", this)),
    m_outputGroupBox(new QGroupBox("Output", this)),
    m_useOutSubDirCheck(new QCheckBox("Save to Output subdirectory", this)),
    m_outputPath(new QLineEdit(this)),
    m_getOutputFolderBtn(new QPushButton("...", this)),
    m_targetPixValue(new QComboBox(this)),
    m_processBtn(new QPushButton("Process")),
    m_stopBtn(new QPushButton("Stop")),
    m_totalProgressBar(new QProgressBar(this))
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QVBoxLayout *inputLayout = new QVBoxLayout(m_inputGroupBox);
    inputLayout->addWidget(m_filesListView);
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
    QHBoxLayout *pixValLayout = new QHBoxLayout();
    pixValLayout->addWidget(new QLabel("Target pixels value", this));
    pixValLayout->addWidget(m_targetPixValue);
    outputLayout->addLayout(pixValLayout);
    m_outputGroupBox->setLayout(outputLayout);
    mainLayout->addWidget(m_outputGroupBox);
    QHBoxLayout *startStopLayout = new QHBoxLayout();
    startStopLayout->addWidget(m_processBtn);
    startStopLayout->addWidget(m_stopBtn);
    mainLayout->addLayout(startStopLayout);
    mainLayout->addWidget(m_totalProgressBar);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    m_filesListView->setModel(&m_filesModel);
    m_filesListView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_targetPixValue->addItems({"Small","Medium","Large"});

    m_totalProgressBar->setMaximum(100);

    m_tiffConverter->moveToThread(m_converterThread);

    setMinimumWidth(400);

    QObject::connect(m_addFilesBtn, SIGNAL(clicked(bool)), this, SLOT(addFiles()));
    QObject::connect(m_addFolderBtn, SIGNAL(clicked(bool)), this, SLOT(addFolder()));
    QObject::connect(m_removeBtn, SIGNAL(clicked(bool)), this, SLOT(removeFile()));
    QObject::connect(m_removeAllBtn, SIGNAL(clicked(bool)), this, SLOT(removeAll()));
    QObject::connect(m_getOutputFolderBtn, SIGNAL(clicked(bool)), this, SLOT(getOutputFolder()));

    QObject::connect(m_processBtn, SIGNAL(clicked(bool)), this, SLOT(ConvertTiff()));
    QObject::connect(m_stopBtn, SIGNAL(clicked(bool)), m_tiffConverter, SLOT(stopProcess()), Qt::DirectConnection);
    QObject::connect(m_useOutSubDirCheck, SIGNAL(stateChanged(int)), this, SLOT(onUseOutSubDirCheck(int)));
    QObject::connect(this, SIGNAL(ConvertTiffSignal(QStringList,QString,int)), m_tiffConverter, SLOT(ConvertTiff(QStringList,QString,int)), Qt::QueuedConnection);
    QObject::connect(m_tiffConverter, SIGNAL(progressSignal(quint32)), this, SLOT(setProgress(quint32)), Qt::QueuedConnection);

    m_converterThread->start();
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
    emit ConvertTiffSignal(m_filesModel.stringList(), outputPath, m_targetPixValue->currentIndex() + 1);
}

void MainWindow::addFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                                        "",
                                                        tr("Images (*.tif *.tiff)"));

    files += m_filesModel.stringList();
    files.removeDuplicates();
    m_filesModel.setStringList(files);
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

    files += m_filesModel.stringList();
    files.removeDuplicates();
    m_filesModel.setStringList(files);
}

void MainWindow::removeFile()
{
    QStringList files = m_filesModel.stringList();

    QStringList list;
    foreach(const QModelIndex &index,
             m_filesListView->selectionModel()->selectedIndexes())
        list.append(files[index.row()]);
    for(auto& file: list) {
        files.removeAll(file);
    }
    m_filesModel.setStringList(files);
}

void MainWindow::removeAll()
{
    m_filesModel.setStringList(QStringList());
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

void MainWindow::setProgress(quint32 pogressVal)
{
    m_totalProgressBar->setValue(pogressVal);
}
