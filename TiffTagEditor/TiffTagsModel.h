#ifndef TIFFTAGSMODEL_H
#define TIFFTAGSMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <qsharedpointer.h>
#include <QWeakPointer>

#include <tif_dir.h>
#include <tiff.h>
#include <tiffiop.h>
#include <tiffio.h>

struct TiffTagsModelItem
{
    QString m_name;
    __int32 m_tagID = 0;
    QVariant m_value;
    TIFFSetGetFieldType m_valueType = TIFF_SETGET_UNDEFINED;
    bool m_changed = false;
};

class TiffTagsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TiffTagsModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    void loadFromTiff(TIFF *tiffFile);
    void closeTiff();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
    void SaveToTiff();
    void setShowAllTags(int newShowAllTags);

private:
    void loadFromTiff();

private:
    TIFF* m_tiffFile;
    QStringList m_headerData;
    QMap<int, QString> m_typeNames;
    QVector<TiffTagsModelItem> m_data;
    bool m_showAllTags = false;
};

#endif // TIFFTAGSMODEL_H
