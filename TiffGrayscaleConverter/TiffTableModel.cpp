#include "TiffTableModel.h"

#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QMessageBox>

QMap<int, QString> typeNames = {
    {TIFF_SETGET_UNDEFINED ,"UNDEFINED"},
    {TIFF_SETGET_UINT8 ,"UINT8"},
    {TIFF_SETGET_UINT16,"UINT16"},
    {TIFF_SETGET_UINT32,"UINT32"},
    {TIFF_SETGET_UINT64,"UINT64"},
    {TIFF_SETGET_SINT8 ,"SINT8"},
    {TIFF_SETGET_SINT16,"SINT16"},
    {TIFF_SETGET_SINT32,"SINT32"},
    {TIFF_SETGET_SINT64,"SINT64"},
    {TIFF_SETGET_FLOAT ,"FLOAT"},
    {TIFF_SETGET_DOUBLE,"DOUBLE"},
    {TIFF_SETGET_ASCII ,"ASCII"},
    {TIFF_SETGET_INT ,"INT"},
    {TIFF_SETGET_IFD8 ,"IFD8"},

    {TIFF_SETGET_C0_UINT8 ,"C0_UINT8"},
    {TIFF_SETGET_C0_UINT16,"C0_UINT16"},
    {TIFF_SETGET_C0_UINT32,"C0_UINT32"},
    {TIFF_SETGET_C0_UINT64,"C0_UINT64"},
    {TIFF_SETGET_C0_SINT8 ,"C0_SINT8"},
    {TIFF_SETGET_C0_SINT16,"C0_SINT16"},
    {TIFF_SETGET_C0_SINT32,"C0_SINT32"},
    {TIFF_SETGET_C0_SINT64,"C0_SINT64"},
    {TIFF_SETGET_C0_FLOAT ,"C0_FLOAT"},
    {TIFF_SETGET_C0_DOUBLE,"C0_DOUBLE"},
    {TIFF_SETGET_C0_ASCII ,"C0_ASCII"},
    {TIFF_SETGET_C0_IFD8 ,"C0_IFD8"},

    {TIFF_SETGET_C16_UINT8 ,"C16_UINT8"},
    {TIFF_SETGET_C16_UINT16,"C16_UINT16"},
    {TIFF_SETGET_C16_UINT32,"C16_UINT32"},
    {TIFF_SETGET_C16_UINT64,"C16_UINT64"},
    {TIFF_SETGET_C16_SINT8 ,"C16_SINT8"},
    {TIFF_SETGET_C16_SINT16,"C16_SINT16"},
    {TIFF_SETGET_C16_SINT32,"C16_SINT32"},
    {TIFF_SETGET_C16_SINT64,"C16_SINT64"},
    {TIFF_SETGET_C16_FLOAT ,"C16_FLOAT"},
    {TIFF_SETGET_C16_DOUBLE,"C16_DOUBLE"},
    {TIFF_SETGET_C16_ASCII ,"C16_ASCII"},
    {TIFF_SETGET_C16_IFD8  ,"C16_IFD8"},

    {TIFF_SETGET_C32_UINT8 ,"C32_UINT8"},
    {TIFF_SETGET_C32_UINT16,"C32_UINT16"},
    {TIFF_SETGET_C32_UINT32,"C32_UINT32"},
    {TIFF_SETGET_C32_UINT64,"C32_UINT64"},
    {TIFF_SETGET_C32_SINT8 ,"C32_SINT8"},
    {TIFF_SETGET_C32_SINT16,"C32_SINT16"},
    {TIFF_SETGET_C32_SINT32,"C32_SINT32"},
    {TIFF_SETGET_C32_SINT64,"C32_SINT64"},
    {TIFF_SETGET_C32_FLOAT ,"C32_FLOAT"},
    {TIFF_SETGET_C32_DOUBLE,"C32_DOUBLE"},
    {TIFF_SETGET_C32_ASCII ,"C32_ASCII"},
    {TIFF_SETGET_C32_IFD8  ,"C32_IFD8"},
    };

QMap<int, QString> compressionNames = {
    {1, "NONE"},
    {2, "CCITTRLE"},
    {3, "CCITTFAX3(T4)"},
    {4, "CCITTFAX4(T6)"},
    {5, "LZW"},
    {6, "OJPEG"},
    {7, "JPEG"},
    {32766, "NEXT"},
    {32771, "CCITTRLEW"},
    {32773, "PACKBITS"},
    {32809, "THUNDERSCAN"},
    {32895, "IT8CTPAD"},
    {32896, "IT8LW"},
    {32897, "IT8MP"},
    {32898, "IT8BL"},
    {32908, "PIXARFILM"},
    {32909, "PIXARLOG"},
    {32946, "DEFLATE"},
    {8, "ADOBE_DEFLATE"},
    {32947, "DCS"},
    {34661, "JBIG"},
    {34676, "SGILOG"},
    {34677, "SGILOG24"},
    {34712, "JP2000"}
};

QMap<int, QString> PhotometricNames = {
    {0, "WhiteIsZero"},
    {1, "BlackIsZero"},
    {2, "RGB"},
    {3, "PALETTE"},
    {4, "TransparencyMask"},
    {5, "Seperated"},
    {6, "YCBCR"},
    {8, "CIELAB"},
    {9, "ICCLAB"},
    {10, "ITULAB"},
    {32844, "LOGL"},
    {32845, "LOGLUV"},
    {32803, "CFA"},
    {34892, "LinearRaw"},
    {51177, "Depth"},
    {51711, "Depth"}
};

template <typename T>
void readTagVal(TIFF *tiffFile, TiffTag &item) {
    T v;
    if(!TIFFGetField(tiffFile, item.m_tagID, &v))
        return;
    item.m_value = v;
}

template <typename T>
void writeTagVal(TIFF *tiffFile, TiffTag &item) {
    T v = item.m_value.value<T>();
    TIFFSetField(tiffFile, item.m_tagID, v);
}

TiffTableModel::TiffTableModel(QObject *parent)
    : QAbstractItemModel{parent},
    m_headerData({"Name", "Path", "Width", "Length", "XRes", "YRes", "Photometric", "BitsPerSample", "SamplesPerPixel", "Compression"})
{
    unsigned firstIndex = 2;
    m_tagsMap = {{firstIndex++, 256},
                 {firstIndex++, 257},
                 {firstIndex++, 282},
                 {firstIndex++, 283},
                 {firstIndex++, 262},
                 {firstIndex++, 258},
                 {firstIndex++, 277},
                 {firstIndex++, 259}};
}

QModelIndex TiffTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()
        || row < 0 || row >= rowCount(parent)
        || column < 0 || column >= columnCount(parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TiffTableModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int TiffTableModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int TiffTableModel::columnCount(const QModelIndex &parent) const
{
    return m_headerData.size();
}

QVariant TiffTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const TiffModelItem& item = m_data[index.row()];
    auto keys = m_tagsMap.keys();
    switch (role) {
    case Qt::DisplayRole: {
        if (m_headerData[index.column()] == "Name")
            return item.m_name;
        if (m_headerData[index.column()] == "Path")
            return item.m_inputPath;
        if(m_headerData[index.column()] == "Photometric")
            return PhotometricNames[item.m_tags[m_tagsMap[index.column()]].m_value.toInt()];
        if(m_headerData[index.column()] == "Compression")
            return compressionNames[item.m_tags[m_tagsMap[index.column()]].m_value.toInt()];
        else if (index.column() >= keys.first() && index.column() <= keys.last())
            return item.m_tags[m_tagsMap[index.column()]].m_value;
    }break;
    case Qt::EditRole: {
        if (index.column() >= keys.first() && index.column() <= keys.last())
            return item.m_tags[m_tagsMap[index.column()]].m_value;
    }break;
    case Qt::FontRole: {
        if (index.column() >= keys.first() && index.column() <= keys.last()){
            QFont font;
            font.setBold(item.m_tags[m_tagsMap[index.column()]].m_changed);
            return font;
        }
    }break;
    }

    return QVariant();
}

bool TiffTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    auto keys = m_tagsMap.keys();
    if (index.column() >= keys.first() && index.column() <= keys.last()) {
        m_data[index.row()].m_tags[m_tagsMap[index.column()]].m_value = value;
        m_data[index.row()].m_tags[m_tagsMap[index.column()]].m_changed = true;
        emit dataChanged(index, index);
    }
    return true;
}

QVariant TiffTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section >= 0 && section < m_headerData.size()) {
            return m_headerData[section];
        }
    }
    return QVariant();
}

Qt::ItemFlags TiffTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    //if(index.column() == 2)
    //    flags |= Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}

bool TiffTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(row < 0 || row >= m_data.size() || (row + count) > m_data.size())
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    for(int i = 0; i < count; ++i) {
        m_data.removeAt(row);
    }
    endRemoveRows();
    return true;
}

QStringList TiffTableModel::files()
{
    QStringList files;
    for(auto &item: m_data) {
        QDir dir(item.m_inputPath);
        files.append(dir.absoluteFilePath(item.m_name));
    }
    return files;
}

void TiffTableModel::openTiff(QStringList pathes)
{
    beginResetModel();
    QStringList vfiles = files();
    for(auto &path: pathes) {
        if(!vfiles.contains(path)) {
            TiffModelItem item = loadFromTiff(path, m_tagsMap.values());
            if(!item.m_tags.size()) {
                QMessageBox msgBox;
                msgBox.setText(QString("Can't open file: %1").arg(path));
                msgBox.exec();
                continue;
            }
            m_data.append(item);
        }
    }
    endResetModel();
}

void TiffTableModel::removeAll()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

TiffModelItem TiffTableModel::loadFromTiff(QString tiffFile, QVector<unsigned> tags)
{
    TiffModelItem item;
    QFileInfo info(tiffFile);
    item.m_name = info.fileName();
    item.m_inputPath = info.absolutePath();
    TIFF *tiff = TIFFOpen(tiffFile.toLocal8Bit().data(), "r+");
    if(!tiff) {
        return item;
    }
    for (int fi = 0, nfi = tiff->tif_nfields; fi < nfi; ++fi) {
        const TIFFField* pField = tiff->tif_fields[fi];
        if(tags.size() && !tags.contains(pField->field_tag)) {
            continue;
        }
        TiffTag tag;
        tag.m_name = QString(pField->field_name);
        tag.m_tagID = pField->field_tag;
        tag.m_valueType = pField->set_field_type;
        if(!tag.m_name.size()) continue;
        if (TIFFFieldWithTag(tiff, tag.m_tagID)) {
            switch (tag.m_valueType) {
            case TIFF_SETGET_UINT8:  readTagVal<uint8_t> (tiff, tag); break;
            case TIFF_SETGET_UINT16: readTagVal<uint16_t>(tiff, tag); break;
            case TIFF_SETGET_UINT32: readTagVal<uint32_t>(tiff, tag); break;
            case TIFF_SETGET_UINT64: readTagVal<float>   (tiff, tag); break;
            case TIFF_SETGET_SINT8:  readTagVal<int8_t>  (tiff, tag); break;
            case TIFF_SETGET_SINT16: readTagVal<int16_t> (tiff, tag); break;
            case TIFF_SETGET_SINT32: readTagVal<int32_t> (tiff, tag); break;
            case TIFF_SETGET_SINT64: readTagVal<int64_t> (tiff, tag); break;
            case TIFF_SETGET_FLOAT:  readTagVal<float>   (tiff, tag); break;
            case TIFF_SETGET_DOUBLE: readTagVal<double>  (tiff, tag); break;
            case TIFF_SETGET_ASCII: {
                char* tagValue;
                if (TIFFGetField(tiff, tag.m_tagID, &tagValue)) {
                    tag.m_value = QString(tagValue);
                }
            }break;
            default:
                break;
            }
        }
        if(!tag.m_value.isNull())
            item.m_tags[tag.m_tagID] = tag;
    }
    TIFFClose(tiff);
    return item;
}
