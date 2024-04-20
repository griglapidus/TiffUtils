#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QCheckBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void ConvertTiff(QString inFile, QString outFile, int targetValue = 1);

private slots:
    void ConvertTiff();
    void getFileNames();
    void getFileNames(QString path);
    void onUseOutSubDirCheck(int checked);

private:
    QStringList m_files;

    QCheckBox *m_convertAllFilesCheck = nullptr;
    QCheckBox *m_useOutSubDirCheck = nullptr;
    QLineEdit *m_filePath = nullptr;
    QPushButton *m_openBtn = nullptr;
    QLineEdit *m_outputPath = nullptr;
    QComboBox *m_targetPixValue = nullptr;
    QPushButton *m_processBtn = nullptr;
    QProgressBar *m_totalProgressBar = nullptr;
    QProgressBar *m_fileProgressBar = nullptr;
};
#endif // MAINWINDOW_H
