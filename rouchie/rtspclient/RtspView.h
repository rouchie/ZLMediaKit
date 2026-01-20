#pragma once

#include <QCache>
#include <QDebug>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <QStringList>
#include <QTableView>

class RtspView : public QTableView {
public:
    RtspView(QWidget *parent = nullptr);
    ~RtspView();

private:
};

class RQTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit RQTableModel(QList<QString> cols, QObject *parent = nullptr);

    void AppendRow(const QList<QString> &rowData);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    virtual QVariant displayRole(const QModelIndex &index) const;
    virtual QVariant textAlignmentRole(const QModelIndex &index) const;

private:
    int _rowCount = 0;
    int _columnCount = 0;

    QList<QString> _headerColumns;
    QMap<int, QList<QString>> _data;
};