#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl url, QObject *parent) :
	QObject(parent),
	m_url(url)
{
	connect(
		&m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
		this, SLOT (fileDownloaded(QNetworkReply*))
	);
	QNetworkRequest request(url);
	m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
	m_DownloadedData = pReply->readAll();
	//emit a signal
	pReply->deleteLater();
	emit downloaded(m_url);
}

QByteArray FileDownloader::downloadedData() const
{
	return m_DownloadedData;
}
