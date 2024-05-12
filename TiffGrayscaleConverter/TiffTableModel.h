#ifndef TIFFTABLEMODEL_H
#define TIFFTABLEMODEL_H

#include <QAbstractItemModel>

#include <tif_dir.h>
#include <tiff.h>
#include <tiffiop.h>
#include <tiffio.h>

struct TiffTag
{
    QString m_name;
    __int32 m_tagID = 0;
    QVariant m_value;
    TIFFSetGetFieldType m_valueType = TIFF_SETGET_UNDEFINED;
    bool m_changed = false;
};

struct TiffModelItem
{
    QString m_name;
    QString m_inputPath;
    QString m_outputPath;
    QMap<unsigned, TiffTag> m_tags;
};

class TiffTableModel : public QAbstractItemModel
{
public:
    explicit TiffTableModel(QObject *parent = nullptr);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    QStringList files();

public slots:
    void openTiff(QStringList pathes);

    void removeAll();

private:
    TiffModelItem loadFromTiff(QString tiffFile, QVector<unsigned> tags = QVector<unsigned>());

private:
    QStringList m_headerData;
    QVector<TiffModelItem> m_data;
    QMap<unsigned, unsigned> m_tagsMap;
};

#endif // TIFFTABLEMODEL_H
