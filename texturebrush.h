#ifndef TEXTUREBRUSH_H
#define TEXTUREBRUSH_H

#include "texturebrushgui.h"

#include <QWidget>
#include <brush_interface.h>
#include <image_processor.h>
class Overlay {
public:
  Overlay(QImage *o, int c, QString t) {
    ov = o;
    color = c;
    type = t;
  }
  QImage *ov;
  int color;
  QString type;
};

class TextureBrush : public QObject, public BrushInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.azagaya.laigter.plugins.BrushInterface" FILE
                        "metadata.json")
  Q_INTERFACES(BrushInterface)
public:
  void mousePress(const QPoint &pos) override;
  void mouseMove(const QPoint &oldPos, const QPoint &newPos) override;
  void mouseRelease(const QPoint &pos) override;
  bool get_selected() override;
  void set_selected(bool s) override;
  QWidget *loadGUI(QWidget *parent = nullptr) override;
  void setProcessor(ImageProcessor **processor) override;
  QIcon getIcon() override;
  QString getName() override;
  QImage getBrushSprite() override;
  void drawAt(QPoint point, QPainter *p, bool alpha_mode = false,
              bool tile_x = false, bool tile_y = false);
  QImage updateOverlay(int xmin, int xmax, int ymin, int ymax, QImage ov,
                       QImage old, QImage aux, float ov_alpha = 1.0f);
  void updateBrushSprite();
  QObject *getObject() override;
  int WrapCoordinate(int coord, int interval);
  QPoint WorldToLocal(QPoint world);
signals:
  void selected_changed(BrushInterface *brush);
  void brush_sprite_updated(QImage sprite);

public slots:
  void set_radius(int r);
  void set_max(int m);
  void set_min(int m);
  void set_mix(int m);
  void set_hardness(int h);
  void set_lineSelected(bool l);
  void set_eraserSelected(bool e);
  void set_brushSelected(bool b);
  void set_height(int heigt);
  void set_specular(int s);
  void set_parallax(int p);
  void set_occlussion(int o);
  void set_diffuse(int d);
  void set_scale(double s);
  void setTextures(QString path, QStringList list);

private:
  QImage *m_heightmap, auxHeight, oldHeight;
  QImage *m_specular, auxSpecular, oldSpecular;
  QImage *m_parallax, auxParallax, oldParallax;
  QImage *m_occlussion, auxOcclussion, oldOcclussion;
  QImage *m_diffuse, auxDiffuse, oldDiffuse, diffuse;
  QImage auxOv;
  TextureBrushGui *gui;
  QImage brushSprite;
  int radius = 15;
  int maxV = 255, minV = 0, height = 255, spec = 255, occ = 255, parallax = 255,
      diff = 255;
  ImageProcessor **processorPtr;
  ImageProcessor *m_processor;
  float alpha = 0.5;
  float hardness = 0.5;
  QBrush m_normal_brush;
  QBrush m_parallax_brush;
  QBrush m_specular_brush;
  QBrush m_occlussion_brush;
  QBrush m_heightmap_brush;
  QBrush m_diffuse_brush;
  QBrush m_brush;

  float scale = 0.125;

  bool linesSelected = true, brushSelected = true, eraserSelected = false,
       selected = true;
  QString m_name = tr("Texture Plugin"), current_texture_path;
  QStringList current_texture_list;
};

#endif // TEXTUREBRUSH_H
