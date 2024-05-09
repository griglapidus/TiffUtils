#include "TiffTagsModel.h"
#include <QTableView>

TiffTagsModel::TiffTagsModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    m_headerData = {"Name", "Type", "Value"};
    m_typeNames = {
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
}

template <typename T>
void readTagVal(TIFF *tiffFile, TiffTagsModelItem &item) {
    T v;
    if(!TIFFGetField(tiffFile, item.m_tagID, &v))
        return;
    item.m_value = v;
}

template <typename T>
void writeTagVal(TIFF *tiffFile, TiffTagsModelItem &item) {
    T v = item.m_value.value<T>();
    TIFFSetField(tiffFile, item.m_tagID, v);
}

void TiffTagsModel::setShowAllTags(int newShowAllTags)
{
    m_showAllTags = newShowAllTags;
    loadFromTiff();
}

void TiffTagsModel::loadFromTiff(TIFF* tiffFile)
{
    m_tiffFile = tiffFile;
    loadFromTiff();
}

void TiffTagsModel::loadFromTiff()
{
    if (!m_tiffFile) {
        return;
    }
    m_data.clear();
    beginResetModel();
    for (int fi = 0, nfi = m_tiffFile->tif_nfields; fi < nfi; ++fi) {
        m_tiffFile->tif_dir.td_customValues[fi].info;
        const TIFFField* pField = m_tiffFile->tif_fields[fi];
        TiffTagsModelItem item;
        item.m_name = QString(pField->field_name);
        item.m_tagID = pField->field_tag;
        item.m_valueType = pField->set_field_type;
        if(!item.m_name.size()) continue;
        if (TIFFFieldWithTag(m_tiffFile, item.m_tagID)) {
            switch (item.m_valueType) {
            case TIFF_SETGET_UINT8:  readTagVal<uint8_t> (m_tiffFile, item); break;
            case TIFF_SETGET_UINT16: readTagVal<uint16_t>(m_tiffFile, item); break;
            case TIFF_SETGET_UINT32: readTagVal<uint32_t>(m_tiffFile, item); break;
            case TIFF_SETGET_UINT64: readTagVal<float>   (m_tiffFile, item); break;
            case TIFF_SETGET_SINT8:  readTagVal<int8_t>  (m_tiffFile, item); break;
            case TIFF_SETGET_SINT16: readTagVal<int16_t> (m_tiffFile, item); break;
            case TIFF_SETGET_SINT32: readTagVal<int32_t> (m_tiffFile, item); break;
            case TIFF_SETGET_SINT64: readTagVal<int64_t> (m_tiffFile, item); break;
            case TIFF_SETGET_FLOAT:  readTagVal<float>   (m_tiffFile, item); break;
            case TIFF_SETGET_DOUBLE: readTagVal<double>  (m_tiffFile, item); break;
            case TIFF_SETGET_ASCII: {
                char* tagValue;
                if (TIFFGetField(m_tiffFile, item.m_tagID, &tagValue)) {
                    item.m_value = QString(tagValue);
                }
            }break;
            default:
                break;
            }
        }
        if(!item.m_value.isNull() || m_showAllTags)
            m_data.append(item);
    }
    endResetModel();
}

void TiffTagsModel::SaveToTiff()
{
    if(!m_tiffFile){
        return;
    }

    for (int i = 0; i < m_data.size(); ++i) {
        TiffTagsModelItem& item = m_data[i];
        if(!item.m_changed) continue;
        switch (item.m_valueType) {
        case TIFF_SETGET_UINT8: writeTagVal<uint8_t>(m_tiffFile, item); break;
        case TIFF_SETGET_UINT16: writeTagVal<uint16_t>(m_tiffFile, item); break;
        case TIFF_SETGET_UINT32: writeTagVal<uint32_t>(m_tiffFile, item); break;
        case TIFF_SETGET_UINT64: writeTagVal<float>(m_tiffFile, item); break;
        case TIFF_SETGET_SINT8: writeTagVal<int8_t>(m_tiffFile, item); break;
        case TIFF_SETGET_SINT16: writeTagVal<int16_t>(m_tiffFile, item); break;
        case TIFF_SETGET_SINT32: writeTagVal<int32_t>(m_tiffFile, item); break;
        case TIFF_SETGET_SINT64: writeTagVal<int64_t>(m_tiffFile, item); break;
        case TIFF_SETGET_FLOAT: writeTagVal<float>(m_tiffFile, item); break;
        case TIFF_SETGET_DOUBLE: writeTagVal<double>(m_tiffFile, item); break;
        case TIFF_SETGET_ASCII: {
            std::string s = item.m_value.toString().toStdString();
            TIFFSetField(m_tiffFile, item.m_tagID, s.c_str());
        }break;
        default:
            break;
        }
        item.m_changed = false;
    }
}

void TiffTagsModel::closeTiff()
{
    m_data.clear();
    m_tiffFile = nullptr;
}

QModelIndex TiffTagsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()
        || row < 0 || row >= rowCount(parent)
        || column < 0 || column >= columnCount(parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TiffTagsModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int TiffTagsModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int TiffTagsModel::columnCount(const QModelIndex &parent) const
{
    return m_headerData.size();
}

QVariant TiffTagsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const TiffTagsModelItem& item = m_data[index.row()];

    switch (role) {
    case Qt::DisplayRole: {
        if (index.column() == 0)
            return item.m_name;
        if (index.column() == 1)
            return m_typeNames[item.m_valueType];
        else if (index.column() == 2)
            return item.m_value;
    }break;
    case Qt::EditRole: {
        if (index.column() == 2)
            return item.m_value;
        }break;
    case Qt::FontRole: {
        if (index.column() == 2){
            QFont font;
            font.setBold(item.m_changed);
            return font;
        }
    }break;
    }

    return QVariant();
}

bool TiffTagsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    m_data[index.row()].m_value = value;
    m_data[index.row()].m_changed = true;
    emit dataChanged(index, index);
    return true;
}

QVariant TiffTagsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section >= 0 && section < m_headerData.size()) {
            return m_headerData[section];
        }
    }
    return QVariant();
}

Qt::ItemFlags TiffTagsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if(index.column() == 2)
        flags |= Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
