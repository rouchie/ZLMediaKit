#include "RtspView.h"

RtspView::RtspView(QWidget *parent)
    : QTableView(parent) {}

RtspView::~RtspView() {}

RQTableModel::RQTableModel(QList<QString> cols, QObject *parent)
    : QAbstractTableModel(parent)
    , _headerColumns(cols) {
    _columnCount = cols.size();
}

void RQTableModel::AppendRow(const QList<QString> &rowData) {
    beginInsertRows(QModelIndex(), _rowCount, _rowCount);

    _data[_rowCount++] = rowData;

    endInsertRows();
}

int RQTableModel::rowCount(const QModelIndex &parent) const {
    return _rowCount;
}

int RQTableModel::columnCount(const QModelIndex &parent) const {
    return _columnCount;
}

QVariant RQTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole: return displayRole(index); // 0
        case Qt::EditRole: return displayRole(index); // 2
        case Qt::TextAlignmentRole: return textAlignmentRole(index); // 7
    }

    return QVariant();
}

QVariant RQTableModel::displayRole(const QModelIndex &index) const {
    int row = index.row();
    int column = index.column();

    auto list = _data[row];
    if (column >= 0 && column < list.size()) {
        return list[column];
    }
    return QString();
}

QVariant RQTableModel::textAlignmentRole(const QModelIndex &index) const {
    return Qt::AlignCenter;
}

QVariant RQTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        // 返回列标题
        if (section >= 0 && section < _headerColumns.size())
            return _headerColumns[section];
    } else if (orientation == Qt::Vertical) {
        // 返回行号（从1开始）
        return QString::number(section + 1);
    }

    return QVariant();
}