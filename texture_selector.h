#ifndef TEXTURE_SELECTOR_H
#define TEXTURE_SELECTOR_H

#include <QListWidgetItem>
#include <QWidget>

#include "downloadmanager.h"

namespace Ui {
class TextureSelector;
}

class TextureSelector : public QWidget {
  Q_OBJECT

public:
  explicit TextureSelector(QWidget *parent = nullptr);
  ~TextureSelector();
signals:
  void selectTexture(QString path, QIcon preview);

private:
  Ui::TextureSelector *ui;
  DownloadManager download_manager, texture_downloader;
  QImage m_preview;

private slots:
  void new_preview(QImage preview, QString name);
  void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
  void textureReady(QString path);
};

#endif // TEXTURE_SELECTOR_H
