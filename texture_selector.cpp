#include "texture_selector.h"
#include "ui_texture_selector.h"

extern "C" {
#include "thirdparty/zip.h"

int on_extract_entry(const char *filename, void *arg) {
  static int i = 0;
  int n = *(int *)arg;
  printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

  return 0;
}
}

TextureSelector::TextureSelector(QWidget *parent)
    : QWidget(parent), ui(new Ui::TextureSelector) {
  ui->setupUi(this);
  download_manager.doDownload(
      QUrl("https://cc0textures.com/api/v1/"
           "full_json?q=&method=&type=PhotoTexturePBR&sort=Latest"));
  connect(&download_manager, SIGNAL(preview_ready(QImage, QString)), this,
          SLOT(new_preview(QImage, QString)));
  connect(&texture_downloader, SIGNAL(progress(int)), ui->progressBar,
          SLOT(setValue(int)));
  connect(&texture_downloader, SIGNAL(texture_ready(QString)), this,
          SLOT(textureReady(QString)));

  ui->progressBar->setVisible(false);
}

TextureSelector::~TextureSelector() { delete ui; }

void TextureSelector::new_preview(QImage preview, QString name) {
  m_preview = preview;
  QListWidgetItem *item = new QListWidgetItem;
  item->setIcon(QPixmap::fromImage(preview));
  item->setText(name);
  ui->listWidget->addItem(item);
}

void TextureSelector::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {

  QString extracted_path =
      "/tmp/" + ui->listWidget->currentItem()->text().toUtf8();

  QDir dir(extracted_path);
  if (!dir.exists()) {
    ui->progressBar->setVisible(true);
    ui->listWidget->setEnabled(false);
    QUrl url = download_manager.get_texture_url(item->text());
    texture_downloader.download_type = DownloadType::PBR;
    texture_downloader.doDownload(url);
  } else {
    selectTexture(extracted_path, ui->listWidget->currentItem()->icon());
    this->hide();
  }
}

void TextureSelector::textureReady(QString path) {
  ui->progressBar->setVisible(false);
  ui->listWidget->setEnabled(true);
  QString extracted_path =
      "/tmp/" + ui->listWidget->currentItem()->text().toUtf8();
  zip_extract(path.toUtf8(), extracted_path.toUtf8(), 0, nullptr);
  selectTexture(extracted_path, ui->listWidget->currentItem()->icon());
  this->hide();
}
