#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTableView>
#include "TiffTagsModel.h"
#include <QTextEdit>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QCheckBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void openTiff();
    void getFileName();
    void saveTiff();
    void closeTiff();

private:
    TIFF* m_tiffFile = nullptr;

    TiffTagsModel m_model;
    QTableView *m_tableViev = nullptr;
    QLineEdit *m_filePath = nullptr;
    QPushButton *m_openFileBtn = nullptr;
    QPushButton *m_getFileName = nullptr;
    QCheckBox *m_loadAllTagsCheck = nullptr;
    QPushButton *m_saveFileBtn = nullptr;
    //QGraphicsScene m_imageScene;
    //QGraphicsView *m_imageView = nullptr;
    //QTextEdit *m_description = nullptr;
    //QCheckBox *m_loadPreviewCheck = nullptr;
};
#endif // MAINWINDOW_H
