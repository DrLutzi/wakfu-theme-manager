#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl url, QObject *parent) :
	QObject(parent),
	m_url(url)
{
	connect(
		&m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
		this, SLOT (fileDownloaded(QNetworkReply*))
	);
}

FileDownloader::~FileDownloader() { }

void FileDownloader::launchDownload()
{
	qDebug() << "Launching download request of link: " << m_url;
	QNetworkRequest request(m_url);
	QNetworkReply *reply = m_WebCtrl.get(request);
	connect(reply, &QNetworkReply::downloadProgress, this, &FileDownloader::updateProgress);
	//Connect the return value of get and connect it to a slot that emits a new signal
}

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
	if(pReply)
	{
		int errorCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		QString errorString = pReply->errorString();
		qDebug() << "Downloaded file with error code " << QString::number(errorCode) << ": " << errorString;
		if(pReply->error() != QNetworkReply::NoError)
		{
			emit errorMsg(pReply->errorString());
		}
		else if(errorCode == 302 || errorCode==301)
		{
			QUrl newUrl = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
			qDebug() << "redirected to " + newUrl.toString();
			QNetworkRequest newRequest(newUrl);
			m_WebCtrl.get(newRequest);
		}
		else
		{
			m_DownloadedData = pReply->readAll();
			//emit a signal
			emit downloaded(m_url);
		}
		pReply->deleteLater();
	}
    return;
}

QByteArray FileDownloader::downloadedData() const
{
	return m_DownloadedData;
}

void FileDownloader::on_updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	emit updateProgress(bytesReceived, bytesTotal);
}
