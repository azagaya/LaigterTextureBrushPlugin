#ifndef TextureBrushGui_H
#define TextureBrushGui_H

#include "texture_selector.h"
#include <QWidget>

namespace Ui {
class TextureBrushGui;
}

class TextureBrushGui : public QWidget {
  Q_OBJECT

public:
  explicit TextureBrushGui(QWidget *parent = nullptr);
  ~TextureBrushGui();

signals:
  void selected_changed(bool s);
  void size_changed(int size);
  void hardness_changed(int hardness);
  void height_changed(int height);
  void specular_changed(int spec);
  void occlussion_changed(int occ);
  void parallax_changed(int par);
  void diffuse_changed(int diff);
  void mix_changed(int mix);
  void setTextures(QString path, QStringList textures);

public slots:
  void brush_sprite_updated(QImage sprite);
  bool get_normal_enabled();
  bool get_parallax_enabled();
  bool get_specular_enabled();
  bool get_occlussion_enabled();
  bool get_diffuse_enabled();

  void set_normal_enabled(bool e);
  void set_parallax_enabled(bool e);
  void set_specular_enabled(bool e);
  void set_occlusion_enabled(bool e);
  void set_diffuse_enabled(bool e);

  bool get_button_rounded();
  bool get_button_eraser();
  void unselect_all();

  void textureSelected(QString path, QIcon preview);

private slots:

  void on_sizeSlider_valueChanged(int value);

  void on_hardnessSlider_valueChanged(int value);

  void on_sliderNormal_valueChanged(int value);

  void on_mixSlider_valueChanged(int value);

  void on_sliderSpecular_valueChanged(int value);

  void on_sliderOcclussion_valueChanged(int value);

  void on_sliderParallax_valueChanged(int value);

  void on_buttonRounded_toggled(bool checked);

  void on_buttonEraser_toggled(bool checked);

  void on_pushButton_pressed();

  void on_sliderDiffuse_valueChanged(int value);

private:
  Ui::TextureBrushGui *ui;
  TextureSelector texture_selector;
};

#endif // TextureBrushGui_H
