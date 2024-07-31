#include "FileModel.h"
#include "BitmapManager.h"

FileModel::FileModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void FileModel::setDirectory(const QString &path)
{
    m_directory = QDir(path);
    refreshData();
}

void FileModel::refreshData()
{
    beginResetModel();

    QFileInfoList allFiles = m_directory.entryInfoList(QDir::Files);
    m_fileInfoList.clear();

    QStringList validExtensions = { "bmp", "png", "barch" };

    for(const QFileInfo &fileInfo : allFiles)
    {
        QString suffix = fileInfo.suffix().toLower();

        if(validExtensions.contains(suffix))
            m_fileInfoList.append(fileInfo);
    }

    endResetModel();
}

void FileModel::process(int rowIndex)
{
    if(rowIndex < 0 || rowIndex >= m_fileInfoList.count())
        return;

    const QFileInfo fileInfo = m_fileInfoList.at(rowIndex);
    QString filePath = fileInfo.filePath();
    bool encode = fileInfo.suffix().toLower() == "bmp";

    if(m_statusMap.contains(filePath))
        return;

    std::thread([this, filePath, encode]() {
        BitmapManager bm;

        {
            std::lock_guard<std::mutex> lock(m_statusMutex);
            m_statusMap[filePath] = encode ? "encoding" : "decoding";
        }

        // Виклик refreshData() в основному потоці
        QMetaObject::invokeMethod(this, "refreshData", Qt::QueuedConnection);

        if(encode)
            bm.encode(filePath);
        else
            bm.decode(filePath);

        {
            std::lock_guard<std::mutex> lock(m_statusMutex);
            m_statusMap.remove(filePath);
        }

        // Виклик refreshData() в основному потоці
        QMetaObject::invokeMethod(this, "refreshData", Qt::QueuedConnection);

    }).detach();
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_fileInfoList.count();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= m_fileInfoList.count())
        return QVariant();

    const QFileInfo &fileInfo = m_fileInfoList.at(index.row());

    switch(role)
    {
    case Name:
        return fileInfo.fileName();
    case Size:
        return fileInfo.size();
    case Status:
    {
        if(m_statusMap.contains(fileInfo.filePath()))
            return m_statusMap.value(fileInfo.filePath());
        else
            return "";
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FileModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Size] = "size";
    roles[Status] = "status";
    return roles;
}
