#include "setsmodel.h"

SetsModel::SetsModel(CardDatabase *_db, QObject *parent)
    : QAbstractTableModel(parent), sets(_db->getSetList())
{
    sets.sortByKey();
}

SetsModel::~SetsModel()
{
}

int SetsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return sets.size();
}

QVariant SetsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.column() >= NUM_COLS) || (index.row() >= rowCount()) || (role != Qt::DisplayRole))
        return QVariant();

    CardSet *set = sets[index.row()];
    switch (index.column()) {
        case SortKeyCol: return set->getSortKey();
        case SetTypeCol: return set->getSetType();
        case ShortNameCol: return set->getShortName();
        case LongNameCol: return set->getLongName();
        case ReleaseDateCol: return set->getReleaseDate();
        default: return QVariant();
    }
}

QVariant SetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
        return QVariant();
    switch (section) {
        case SortKeyCol: return tr("Key");
        case SetTypeCol: return tr("Set type");
        case ShortNameCol: return tr("Short name");
        case LongNameCol: return tr("Long name");
        case ReleaseDateCol: return tr("Release date");
        default: return QVariant();
    }
}

Qt::ItemFlags SetsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = QAbstractTableModel::flags(index);
    return result | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions SetsModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData *SetsModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return 0;

    SetsMimeData *result = new SetsMimeData(indexes[0].row());
    return qobject_cast<QMimeData *>(result);
}

bool SetsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int /*column*/, const QModelIndex &parent)
{
    if (action != Qt::MoveAction)
        return false;
    if (row == -1) {
        if (!parent.isValid())
            return false;
        row = parent.row();
    }
    int oldRow = qobject_cast<const SetsMimeData *>(data)->getOldRow();
    beginRemoveRows(QModelIndex(), oldRow, oldRow);
    CardSet *temp = sets.takeAt(oldRow);
    endRemoveRows();
    if (oldRow < row)
        row--;
    beginInsertRows(QModelIndex(), row, row);
    sets.insert(row, temp);
    endInsertRows();

    for (int i = 0; i < sets.size(); i++)
        sets[i]->setSortKey(i);

    sets.sortByKey();

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

    return true;
}

QStringList SetsModel::mimeTypes() const
{
    return QStringList() << "application/x-cockatricecardset";
}

SetsProxyModel::SetsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void SetsProxyModel::saveOrder()
{
    int numRows = rowCount();
    SetsModel * model = (SetsModel*) sourceModel();
    for(int row = 0; row < numRows; ++row) {
        QModelIndex idx = index(row, 0);
        int oldRow = data(idx, Qt::DisplayRole).toInt(); 
        CardSet *temp = model->sets.at(oldRow);
        temp->setSortKey(row);
    }
    model->sets.sortByKey();

    emit model->dataChanged(model->index(0, 0), model->index(model->rowCount() - 1, model->columnCount() - 1));
}
