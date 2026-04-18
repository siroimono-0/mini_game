#ifndef MAP_MD_H
#define MAP_MD_H

#include <QAbstractListModel>
#include <QDate>
#include <QObject>
#include <bits/stdc++.h>

using namespace std;
class Map_md : public QAbstractListModel

{
    Q_OBJECT
public:
    explicit Map_md(QObject *parent = nullptr);

    // QML에서 접근할 "역할(role)" 이름 정의
    enum Role {
        groundRole = Qt::UserRole + 1,
        itemRole = Qt::UserRole + 2,
    };
    Q_ENUM(Role) // QML에서도 enum을 인식할 수 있게 함

    // 모델의 총 행 개수 반환 (QML의 model.count 등에서 사용)
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // 특정 인덱스의 데이터 반환 (QML에서 model.text 요청 시 호출됨)
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // QML에서 model.text로 접근할 수 있도록 role과 이름 매핑
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reset_md();

    void set_vec(const QVector<QPair<QPair<int, int>, char>> set);

public slots:
    // void slot_qvec_update(QVector<hmi_current_stat_admin> new_qvec);

signals:

private:
    QVector<QVector<QPair<char, char>>> qvec;
};

#endif // MAP_MD_H
