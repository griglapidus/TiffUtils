#ifndef TIFFCONVERTER_H
#define TIFFCONVERTER_H

#include <QObject>
#include <QStringList>

class TiffConverter : public QObject
{
    Q_OBJECT
public:
    explicit TiffConverter(QObject *parent = nullptr);


public slots:
    void ConvertTiff(QStringList inFiles, QString outputFolder, int targetValue, bool negative, bool openOutput);
    void stopProcess();
    void setRes(double newXRes, double newYRes);

signals:
    void progressSignal(quint32 progress);
    void finished();
    void showMsg(QString);

private:
    void ConvertTiff(QString inFile, QString outFile, int targetValue, bool negative);

private:
    std::atomic_bool m_stopFlag = false;
    double m_xRes = 0;
    double m_yRes = 0;
};

#endif // TIFFCONVERTER_H
