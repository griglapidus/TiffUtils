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

signals:
    void progressSignal(quint32 progress);

private:
    void ConvertTiff(QString inFile, QString outFile, int targetValue, bool negative);

private:
    std::atomic_bool m_stopFlag = false;
};

#endif // TIFFCONVERTER_H
