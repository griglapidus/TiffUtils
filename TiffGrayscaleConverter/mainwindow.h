#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QCheckBox>
#include <QGroupBox>
#include <QStringListModel>
#include <QListView>
#include "TiffConverter.h"
#include <QThread>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addFiles();
    void addFolder();
    void removeFile();
    void removeAll();

    void getOutputFolder();

    void ConvertTiff();
    void onUseOutSubDirCheck(int checked);
    void setProgress(quint32 progressVal);
    void finished();
signals:
    void ConvertTiffSignal(QStringList inFiles, QString outputFolder, int targetValue, bool negative, bool openOutput);

private:
    QThread *m_converterThread = nullptr;
    TiffConverter *m_tiffConverter = nullptr;
    QStringListModel m_filesModel;

    QGroupBox *m_inputGroupBox = nullptr;
    QListView *m_filesListView = nullptr;
    QPushButton *m_addFilesBtn = nullptr;
    QPushButton *m_addFolderBtn = nullptr;
    QPushButton *m_removeBtn = nullptr;
    QPushButton *m_removeAllBtn = nullptr;

    QGroupBox *m_outputGroupBox = nullptr;
    QCheckBox *m_useOutSubDirCheck = nullptr;
    QCheckBox *m_openOutputOnFinish = nullptr;
    QLineEdit *m_outputPath = nullptr;
    QPushButton *m_getOutputFolderBtn = nullptr;
    QComboBox *m_targetPixValue = nullptr;
    QCheckBox *m_negativeCheck = nullptr;

    QPushButton *m_processBtn = nullptr;
    QPushButton *m_stopBtn = nullptr;
    QProgressBar *m_totalProgressBar = nullptr;
};
#endif // MAINWINDOW_H
