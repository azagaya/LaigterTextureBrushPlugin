#include "downloadmanager.h"
DownloadManager::DownloadManager() {
  connect(&manager, &QNetworkAccessManager::finished, this,
          &DownloadManager::downloadFinished);
  connect(this, SIGNAL(ready_for_next()), &readyLoop, SLOT(quit()));
}

void DownloadManager::doDownload(const QUrl &url) {
  QNetworkRequest request(url);
  QNetworkReply *reply = manager.get(request);
#if QT_CONFIG(ssl)
  connect(reply, &QNetworkReply::sslErrors, this, &DownloadManager::sslErrors);
#endif
  connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this,
          SLOT(updateProgress(qint64, qint64)));
  currentDownloads.append(reply);
}

void DownloadManager::updateProgress(qint64 rcv, qint64 total) {
  if (total > 0)
    progress(100 * rcv / total);
}

QString DownloadManager::saveFileName(const QUrl &url) {
  QString path = url.path();
  QString basename = QFileInfo(path).fileName();
  QString temppath =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation);

  if (basename.isEmpty())
    basename = "download";

  if (QFile::exists(basename)) {
    // already exists, don't overwrite
    int i = 0;
    basename += '.';
    while (QFile::exists(basename + QString::number(i)))
      ++i;

    basename += QString::number(i);
  }

  return temppath + "/" + basename;
}

bool DownloadManager::saveToDisk(const QString &filename, QIODevice *data) {
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    fprintf(stderr, "Could not open %s for writing: %s\n", qPrintable(filename),
            qPrintable(file.errorString()));
    return false;
  }

  file.write(data->readAll());
  file.close();

  return true;
}

bool DownloadManager::isHttpRedirect(QNetworkReply *reply) {
  int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  return statusCode == 301 || statusCode == 302 || statusCode == 303 ||
         statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::sslErrors(const QList<QSslError> &sslErrors) {
#if QT_CONFIG(ssl)
  for (const QSslError &error : sslErrors)
    fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
  Q_UNUSED(sslErrors);
#endif
}

void DownloadManager::downloadFinished(QNetworkReply *reply) {
  QUrl url = reply->url();
  if (reply->error()) {
    fprintf(stderr, "Download of %s failed: %s\n", url.toEncoded().constData(),
            qPrintable(reply->errorString()));
    if (download_type == DownloadType::PREVIEW) {
      next_asset++;
      if (next_asset != assets.end())
        doDownload(get_preview_url(next_asset));
    }
  } else {
    if (isHttpRedirect(reply)) {
      fputs("Request was redirected.\n", stderr);
    } else {
      QUrl next;
      QImage preview;
      switch (download_type) {
      case DownloadType::JSON:
        json = QJsonDocument::fromJson(reply->readAll());
        assets = json.object().value("Assets").toObject();
        assets_names = assets.keys();

        next_asset = assets.begin();
        download_type = DownloadType::PREVIEW;

        doDownload(get_preview_url(next_asset));
        break;
      case DownloadType::PREVIEW:
        preview = QImage::fromData(reply->readAll());
        preview_ready(preview, next_asset.key());
        next_asset++;
        if (next_asset != assets.end())
          doDownload(get_preview_url(next_asset));
        break;
      case DownloadType::PBR:
        QString filename = saveFileName(url);
        if (saveToDisk(filename, reply)) {
          printf("Download of %s succeeded (saved to %s)\n",
                 url.toEncoded().constData(), qPrintable(filename));
        }
        texture_ready(filename);
        break;
      }
    }
  }

  currentDownloads.removeAll(reply);
  reply->deleteLater();
  ready_for_next();
}

QUrl DownloadManager::get_preview_url(QJsonObject::const_iterator iterator) {
  QUrl url(iterator.value()
               .toObject()
               .value("PreviewSphere")
               .toObject()
               .value("128-PNG")
               .toString());
  return url;
}

QString DownloadManager::get_texture(QString name) {
  QUrl url = get_texture_url(name);
  readyLoop.exec();
  download_type = DownloadType::PBR;
  doDownload(url);
  readyLoop.exec();
  return "asdf";
}

QUrl DownloadManager::get_texture_url(QString name) {
  QUrl url = assets.value(name)
                 .toObject()
                 .value("Downloads")
                 .toObject()
                 .value("2K-JPG")
                 .toObject()
                 .value("RawDownloadLink")
                 .toString();
  return url;
}
