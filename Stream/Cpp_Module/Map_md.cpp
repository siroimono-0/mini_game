#include "Map_md.h"

Map_md::Map_md(QObject *parent)
    : QAbstractListModel{parent}
{
    this->qvec.assign(13, QVector<QPair<char, char>>(15));
    for (int i = 0; i < 13; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            if (j < 5)
            {
                qvec[i][j].first = 'A';
            }
            else if (j < 10)
            {
                qvec[i][j].first = 'B';
            }
            else if (j < 15)
            {
                qvec[i][j].first = 'C';
            }
        }
    }
    /*
    int cnt = 0;
    for (auto &v : this->qvec)
    {
        for (auto &vv : v)
        {
            qDebug() << vv.first << "   " << cnt++;
        }
    }*/
}

int Map_md::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return 13 * 15;
}

/*
int Map_md::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return this->qvec[0].size();
}*/

QVariant Map_md::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    int cy = index.row() / 15;
    int cx = index.row() % 15;

    if (cy < 0 || cy >= 13 || cx >= 15 || cx < 0)
    {
        return QVariant();
    }

    if (role == groundRole)
    {
        return QVariant(this->qvec[cy][cx].first);
    }

    if (role == itemRole)
    {
        return QVariant(this->qvec[cy][cx].second);
    }

    return QVariant();
}

QHash<int, QByteArray> Map_md::roleNames() const
{
    QHash<int, QByteArray> qh;
    qh[groundRole] = "ground";
    qh[itemRole] = "item";

    return qh;
}

Q_INVOKABLE void Map_md::reset_md()
{
    beginResetModel();
    endResetModel();
}

void Map_md::set_vec(const QVector<QPair<QPair<int, int>, char>> set)
{
    for (auto &v : set)
    {
        if (this->qvec[v.first.first][v.first.second].second != v.second)
        {
            // qDebug() << v.second;
            this->qvec[v.first.first][v.first.second].second = v.second;

            int row = v.first.second + (15 * v.first.first);

            QModelIndex idx = index(row, 0);
            emit dataChanged(idx, idx, {itemRole});
        }
    }

    /*
    cout << endl;
    for (auto &v : this->qvec)
    {
        for (auto &vv : v)
        {
            cout << vv.second;
        }
        cout << endl;
    }
    cout << endl;*/
    return;
}
