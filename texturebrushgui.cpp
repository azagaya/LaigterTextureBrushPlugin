#include "texturebrushgui.h"
#include "ui_texturebrushgui.h"
#include <QPainter>

TextureBrushGui::TextureBrushGui(QWidget *parent)
    : QWidget(parent), ui(new Ui::TextureBrushGui) {
  ui->setupUi(this);
  connect(&texture_selector, SIGNAL(selectTexture(QString, QIcon)), this,
          SLOT(textureSelected(QString, QIcon)));
}

TextureBrushGui::~TextureBrushGui() { delete ui; }

void TextureBrushGui::textureSelected(QString path, QIcon preview) {
  ui->imageLabel->setPixmap(preview.pixmap(preview.availableSizes().at(0)));
  QDir dir(path);
  if (dir.exists()) {
    QStringList texture_list = dir.entryList();
    setTextures(path, texture_list);
  }
}

bool TextureBrushGui::get_normal_enabled() {
  return ui->checkBoxNormal->isChecked();
}

bool TextureBrushGui::get_parallax_enabled() {
  return ui->checkBoxParallax->isChecked();
}

bool TextureBrushGui::get_specular_enabled() {
  return ui->checkBoxSpecular->isChecked();
}

bool TextureBrushGui::get_occlussion_enabled() {
  return ui->checkBoxOcclussion->isChecked();
}

bool TextureBrushGui::get_diffuse_enabled() {
  return ui->checkBoxDiffuse->isChecked();
}

void TextureBrushGui::set_normal_enabled(bool e) {
  ui->checkBoxNormal->setEnabled(e);
  ui->checkBoxNormal->setChecked(e);
}
void TextureBrushGui::set_diffuse_enabled(bool e) {
  ui->checkBoxDiffuse->setChecked(e);
  ui->checkBoxDiffuse->setEnabled(e);
}
void TextureBrushGui::set_specular_enabled(bool e) {
  ui->checkBoxSpecular->setChecked(e);
  ui->checkBoxSpecular->setEnabled(e);
}
void TextureBrushGui::set_parallax_enabled(bool e) {
  ui->checkBoxParallax->setChecked(e);
  ui->checkBoxParallax->setEnabled(e);
}
void TextureBrushGui::set_occlusion_enabled(bool e) {
  ui->checkBoxOcclussion->setChecked(e);
  ui->checkBoxOcclussion->setEnabled(e);
}

void TextureBrushGui::brush_sprite_updated(QImage sprite) {
  QImage fill(QSize(200, 200), QImage::Format_RGBA8888);
  fill.fill(QColor(51, 51, 77, 255));

  QPainter p(&fill);
  if (sprite.width() > 200 || sprite.height() > 200)
    sprite = sprite.scaled(200, 200);
  p.drawImage(QPointF(100 - sprite.width() / 2.0, 100 - sprite.height() / 2.0),
              sprite);

  //  p.setPen(QColor(153,153,255));
  //  p.drawEllipse(100-sprite.width()/2.0,100-sprite.height()/2.0,sprite.width(),sprite.height());
  ui->imageLabel->resize(sprite.size());
  ui->imageLabel->setPixmap(QPixmap::fromImage(fill));
}

void TextureBrushGui::on_sizeSlider_valueChanged(int value) {
  size_changed(value);
}

void TextureBrushGui::on_hardnessSlider_valueChanged(int value) {
  hardness_changed(value);
}

void TextureBrushGui::on_sliderNormal_valueChanged(int value) {
  height_changed(value);
}

void TextureBrushGui::on_mixSlider_valueChanged(int value) {
  mix_changed(value);
}

void TextureBrushGui::on_sliderSpecular_valueChanged(int value) {
  specular_changed(value);
}

void TextureBrushGui::on_sliderOcclussion_valueChanged(int value) {
  occlussion_changed(value);
}

void TextureBrushGui::on_sliderParallax_valueChanged(int value) {
  parallax_changed(value);
}

void TextureBrushGui::unselect_all() {
  ui->buttonRounded->setChecked(false);
  ui->buttonEraser->setChecked(false);
}

void TextureBrushGui::on_buttonRounded_toggled(bool checked) {
  if (checked) {
    ui->buttonEraser->setChecked(false);
  }
  selected_changed(checked);
}

void TextureBrushGui::on_buttonEraser_toggled(bool checked) {
  if (checked) {
    ui->buttonRounded->setChecked(false);
  }
  selected_changed(checked);
}

bool TextureBrushGui::get_button_eraser() {
  return ui->buttonEraser->isChecked();
}
bool TextureBrushGui::get_button_rounded() {
  return ui->buttonRounded->isChecked();
}

void TextureBrushGui::on_pushButton_pressed() { texture_selector.show(); }

void TextureBrushGui::on_sliderDiffuse_valueChanged(int value) {
  diffuse_changed(value);
}


void TextureBrushGui::on_scaleSpinBox_editingFinished()
{
    scaleChanged(ui->scaleSpinBox->value());
}
