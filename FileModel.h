#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QDir>
#include <QFileInfoList>
#include <QThread>
#include <QMutex>

class FileModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FileModel(QObject *parent = nullptr);

    enum FileRoles {
        Name = Qt::UserRole + 1,
        Size,
        Status
    };

    void setFileStatus(int index, const QString &status);

    void setDirectory(const QString &path);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refreshData();
    Q_INVOKABLE void process(int rowIndex);

private:
    QDir                    m_directory;
    QFileInfoList           m_fileInfoList;
    QMap<QString, QString>  m_statusMap;
    std::mutex              m_statusMutex;
};

#endif // FILEMODEL_H
