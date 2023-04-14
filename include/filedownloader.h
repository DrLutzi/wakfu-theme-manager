#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

class FileDownloader : public QObject
{
	Q_OBJECT

public:
	explicit FileDownloader(QUrl url, QObject *parent = 0);
	virtual ~FileDownloader();
	QByteArray downloadedData() const;
	void launchDownload();

signals:
	void downloaded(QUrl url);
	void errorMsg(QString errorMsg, int timeout = 0);
	void updateProgress(qint64 bytesReceived, qint64 bytesTotal);

private slots:
	void fileDownloaded(QNetworkReply* pReply);
	void on_updateProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
	QNetworkAccessManager	m_WebCtrl;
	QByteArray	m_DownloadedData;
	QUrl		m_url;
};

#endif // FILEDOWNLOADER_H
