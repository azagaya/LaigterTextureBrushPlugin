#include "texturebrush.h"
#include <QDebug>
#include <QIcon>
#include <QList>
#include <QRadialGradient>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>

QImage TextureBrush::updateOverlay(int xmin, int xmax, int ymin, int ymax,
                                   QImage ov, QImage old, QImage aux,
                                   float ov_alpha) {
  QPoint topLeft;
  QPoint botRight;

  topLeft = QPoint(xmin, ymin);
  botRight = QPoint(xmax, ymax);

  QRect imageRect = diffuse.rect();
  bool tile_x = m_processor->get_tile_x();
  bool tile_y = m_processor->get_tile_y();

  int w = diffuse.width();
  int h = diffuse.height();

  for (int x = xmin; x < xmax; x++) {
    for (int y = ymin; y < ymax; y++) {

      int ix = x;
      int iy = y;

      if (!imageRect.contains(QPoint(x, y))) {
        if (!tile_x && !tile_y) {
          continue;
        }
        if (tile_x) {
          ix = WrapCoordinate(ix, w);
        }
        if (tile_y) {
          iy = WrapCoordinate(iy, h);
        }
      }

      QColor oldColor = old.pixelColor(ix, iy);
      QColor auxColor = aux.pixelColor(ix, iy);
      float aux_alpha = auxOv.pixelColor(ix, iy).alphaF();
      float a = ov_alpha * aux_alpha * alpha;
      QColor newColor(0, 0, 0, 0);

      if (aux_alpha <= 1e-6 || diffuse.pixelColor(ix, iy).alphaF() == 0) {
        newColor = oldColor;
      } else {
        float outA = a + oldColor.alphaF() * (1 - a);
        int r = auxColor.red() * a / outA +
                oldColor.red() * oldColor.alphaF() * (1 - a) / outA;

        int g = auxColor.green() * a / outA +
                oldColor.green() * oldColor.alphaF() * (1 - a) / outA;

        int b = auxColor.blue() * a / outA +
                oldColor.blue() * oldColor.alphaF() * (1 - a) / outA;

        newColor = QColor(r, g, b);
        newColor.setAlphaF(outA);
      }

      ov.setPixelColor(ix, iy, newColor);
    }
  }
  return ov;
}

void TextureBrush::drawAt(QPoint point, QPainter *p, bool alpha_mode,
                          bool tile_x, bool tile_y) {
  int w = p->device()->width();
  int h = p->device()->height();

  QList<QPoint> pointList;

  if (tile_x) {
    point.setX(WrapCoordinate(point.x(), w));
  }

  if (tile_y) {
    point.setY(WrapCoordinate(point.y(), h));
  }

  if (tile_x) {

    pointList.append(point - QPoint(w, 0));
    pointList.append(point + QPoint(w, 0));
  }

  if (tile_y) {
    pointList.append(point - QPoint(0, h));
    pointList.append(point + QPoint(0, h));
  }

  pointList.append(point);

  foreach (QPoint point, pointList) {

    if (!alpha_mode) {
      p->setPen(
          QPen(m_brush, radius, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
      p->drawEllipse(point.x() - radius / 2, point.y() - radius / 2, radius,
                     radius);
    } else {
      QRadialGradient gradient(point, radius);

      gradient.setColorAt(0, QColor(255.0, 255.0, 255.0, 255.0));
      gradient.setColorAt(pow(hardness * radius, 2) / pow(radius, 2),
                          QColor(255.0, 255.0, 255.0, hardness * 255));
      gradient.setColorAt(1,
                          QColor(255.0, 255.0, 255.0, floor(hardness) * 255));

      QBrush b(gradient);
      p->setPen(QPen(b, radius, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
      p->drawEllipse(point.x() - radius / 2, point.y() - radius / 2, radius,
                     radius);
    }
  }
}

void TextureBrush::mouseMove(const QPoint &oldPos, const QPoint &newPos) {

  m_processor = *processorPtr;

  if (!selected)
    return;

  QPoint oldP = WorldToLocal(oldPos);
  QPoint newP = WorldToLocal(newPos);

  bool tile_x = m_processor->get_tile_x();
  bool tile_y = m_processor->get_tile_y();

  QPoint in(oldP);
  QPoint fi(newP);

  int xmin = std::min(in.x(), fi.x());
  int xmax = std::max(in.x(), fi.x());

  int ymin = std::min(in.y(), fi.y());
  int ymax = std::max(in.y(), fi.y());

  if (!gui->get_button_eraser()) {

    QList<Overlay> imageList;
    if (gui->get_normal_enabled()) {
      imageList.append(Overlay(&auxHeight, height, "normal"));
    }
    if (gui->get_parallax_enabled()) {
      imageList.append(Overlay(&auxParallax, parallax, "parallax"));
    }
    if (gui->get_specular_enabled()) {
      imageList.append(Overlay(&auxSpecular, spec, "specular"));
    }
    if (gui->get_occlussion_enabled()) {
      imageList.append(Overlay(&auxOcclussion, occ, "occlussion"));
    }
    if (gui->get_diffuse_enabled()) {
      imageList.append(Overlay(&auxDiffuse, diff, "diffuse"));
    }
    imageList.append(Overlay(&auxOv, 255.0, "alpha"));

    foreach (Overlay overlay, imageList) {

      QImage *aux = overlay.ov;
      maxV = overlay.color;
      QPainter p(aux);
      bool alpha = false;
      QPainterPath path;
      path.moveTo(in);
      path.lineTo(fi);
      qreal length = path.length();
      qreal pos = 0;

      if (overlay.type == "alpha") {
        alpha = true;
      } else if (overlay.type == "specular") {
        m_brush = m_specular_brush;
      } else if (overlay.type == "normal") {
        m_brush = m_normal_brush;
      } else if (overlay.type == "parallax") {
        m_brush = m_parallax_brush;
      } else if (overlay.type == "diffuse") {
        m_brush = m_diffuse_brush;
      }
      while (pos < length) {
        qreal percent = path.percentAtLength(pos);
        QPoint point = path.pointAtPercent(percent).toPoint();

        drawAt(point, &p, alpha, tile_x, tile_y);
        pos += radius / 4.0;
      }
    }
    xmin -= radius;
    xmax += radius;
    ymin -= radius;
    ymax += radius;
    if (gui->get_normal_enabled())
      m_processor->set_normal_overlay(updateOverlay(
          xmin, xmax, ymin, ymax, m_processor->get_normal_overlay(), oldHeight,
          auxHeight, height / 255.0));
    if (gui->get_parallax_enabled())
      m_processor->set_parallax_overlay(updateOverlay(
          xmin, xmax, ymin, ymax, m_processor->get_parallax_overlay(),
          oldParallax, auxParallax, parallax / 255.0));
    if (gui->get_specular_enabled())
      m_processor->set_specular_overlay(updateOverlay(
          xmin, xmax, ymin, ymax, m_processor->get_specular_overlay(),
          oldSpecular, auxSpecular, spec / 255.0));
    if (gui->get_occlussion_enabled())
      m_processor->set_occlussion_overlay(updateOverlay(
          xmin, xmax, ymin, ymax, m_processor->get_occlusion_overlay(),
          oldOcclussion, auxOcclussion, occ / 255.0));
    if (gui->get_diffuse_enabled())
      m_processor->set_texture_overlay(updateOverlay(
          xmin, xmax, ymin, ymax, m_processor->get_texture_overlay(),
          oldDiffuse, auxDiffuse, diff / 255.0));

  } else {
    // TODO make it work in tiles
    QImage erased(auxHeight.size(), QImage::Format_RGBA8888_Premultiplied);
    erased.fill(QColor(0, 0, 0, 0));

    QList<Overlay> imageList;
    if (gui->get_normal_enabled()) {
      imageList.append(Overlay(&oldHeight, height, "normal"));
    }
    if (gui->get_parallax_enabled()) {
      imageList.append(Overlay(&oldParallax, parallax, "parallax"));
    }
    if (gui->get_specular_enabled()) {
      imageList.append(Overlay(&oldSpecular, spec, "specular"));
    }
    if (gui->get_occlussion_enabled()) {
      imageList.append(Overlay(&oldOcclussion, occ, "occlussion"));
    }
    if (gui->get_diffuse_enabled()) {
      imageList.append(Overlay(&oldDiffuse, 255.0, "diffuse"));
    }
    foreach (Overlay ov, imageList) {
      QImage *overlay = ov.ov;
      QPainter p(overlay);
      p.setCompositionMode(QPainter::CompositionMode_Source);
      QPen pen(QColor(0, 0, 0, 0));
      pen.setWidth(2 * radius);
      pen.setStyle(Qt::SolidLine);
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::MiterJoin);
      p.setPen(pen);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.drawLine(in, fi);
      if (ov.type == "normal") {
        m_processor->set_normal_overlay(*overlay);
      } else if (ov.type == "parallax") {
        m_processor->set_parallax_overlay(*overlay);
      } else if (ov.type == "specular") {
        m_processor->set_specular_overlay(*overlay);
      } else if (ov.type == "occlussion") {
        m_processor->set_occlussion_overlay(*overlay);
      } else if (ov.type == "diffuse") {
        m_processor->set_texture_overlay(*overlay);
      }
    }
  }

  xmin = std::min(in.x(), fi.x());
  xmax = std::max(in.x(), fi.x());

  ymin = std::min(in.y(), fi.y());
  ymax = std::max(in.y(), fi.y());

  QRect r(QPoint(xmin - radius, ymin - radius),
          QPoint(xmax + radius, ymax + radius));

  if (gui->get_specular_enabled())
    QtConcurrent::run(m_processor, &ImageProcessor::calculate_specular);
  if (gui->get_parallax_enabled())
    QtConcurrent::run(m_processor, &ImageProcessor::calculate_parallax);
  if (gui->get_occlussion_enabled())
    QtConcurrent::run(m_processor, &ImageProcessor::calculate_occlusion);
  if (gui->get_normal_enabled())
    QtConcurrent::run(m_processor, &ImageProcessor::generate_normal_map, false,
                      false, false, r);
}

void TextureBrush::mousePress(const QPoint &pos) {
  m_processor = *processorPtr;

  oldHeight = m_processor->get_normal_overlay();
  auxHeight = QImage(oldHeight.width(), oldHeight.height(),
                     QImage::Format_RGBA8888_Premultiplied);
  auxHeight.fill(QColor(0, 0, 0, 0));

  oldParallax = m_processor->get_parallax_overlay();
  auxParallax = QImage(oldParallax.width(), oldParallax.height(),
                       QImage::Format_RGBA8888_Premultiplied);
  auxParallax.fill(QColor(0, 0, 0, 0));

  oldSpecular = m_processor->get_specular_overlay();
  auxSpecular = QImage(oldSpecular.width(), oldSpecular.height(),
                       QImage::Format_RGBA8888_Premultiplied);
  auxSpecular.fill(QColor(0, 0, 0, 0));

  oldOcclussion = m_processor->get_occlusion_overlay();
  auxOcclussion = QImage(oldOcclussion.width(), oldOcclussion.height(),
                         QImage::Format_RGBA8888_Premultiplied);
  auxOcclussion.fill(QColor(0, 0, 0, 0));

  oldDiffuse = m_processor->get_texture_overlay();
  auxDiffuse = QImage(oldDiffuse.size(), QImage::Format_RGBA8888_Premultiplied);
  auxDiffuse.fill(Qt::transparent);

  auxOv = QImage(oldHeight.size(), QImage::Format_RGBA8888_Premultiplied);
  auxOv.fill(Qt::transparent);

  bool tile_x = m_processor->get_tile_x();
  bool tile_y = m_processor->get_tile_y();

  QPoint newP = WorldToLocal(pos);
  QPoint fi(newP);
  if (!gui->get_button_eraser()) {
    QList<Overlay> imageList;
    if (gui->get_normal_enabled()) {
      imageList.append(Overlay(&auxHeight, height, "normal"));
    }
    if (gui->get_parallax_enabled()) {
      imageList.append(Overlay(&auxParallax, parallax, "parallax"));
    }
    if (gui->get_specular_enabled()) {
      imageList.append(Overlay(&auxSpecular, spec, "specular"));
    }
    if (gui->get_occlussion_enabled()) {
      imageList.append(Overlay(&auxOcclussion, occ, "occlussion"));
    }
    if (gui->get_diffuse_enabled()) {
      imageList.append(Overlay(&auxDiffuse, diff, "diffuse"));
    }

    imageList.append(Overlay(&auxOv, 255.0, "alpha"));
    foreach (Overlay overlay, imageList)

    {
      /* Draw the point */
      QImage *aux = overlay.ov;
      QPainter p(aux);
      bool alpha = false;
      if (overlay.type == "alpha") {
        alpha = true;
      } else if (overlay.type == "specular") {
        m_brush = m_specular_brush;
      } else if (overlay.type == "normal") {
        m_brush = m_normal_brush;
      } else if (overlay.type == "parallax") {
        m_brush = m_parallax_brush;
      } else if (overlay.type == "diffuse") {
        m_brush = m_diffuse_brush;
      }

      drawAt(fi, &p, alpha, tile_x, tile_y);
    }

  } else {
    QImage erased(auxHeight.size(), QImage::Format_RGBA8888_Premultiplied);
    erased.fill(QColor(0, 0, 0, 0));

    QList<Overlay> imageList;
    if (gui->get_normal_enabled()) {
      imageList.append(Overlay(&oldHeight, height, "normal"));
    }
    if (gui->get_parallax_enabled()) {
      imageList.append(Overlay(&oldParallax, parallax, "parallax"));
    }
    if (gui->get_specular_enabled()) {
      imageList.append(Overlay(&oldSpecular, spec, "specular"));
    }
    if (gui->get_occlussion_enabled()) {
      imageList.append(Overlay(&oldOcclussion, occ, "occlussion"));
    }
    if (gui->get_diffuse_enabled()) {
      imageList.append(Overlay(&oldDiffuse, diff, "diffuse"));
    }
    foreach (Overlay ov, imageList) {
      QImage *overlay = ov.ov;
      QPainter p(overlay);
      p.setCompositionMode(QPainter::CompositionMode_Source);
      QPen pen(QColor(0, 0, 0, 0));
      pen.setWidth(2 * radius);
      pen.setStyle(Qt::SolidLine);
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::MiterJoin);
      p.setPen(pen);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.drawPoint(fi);
      if (ov.type == "normal") {
        m_processor->set_normal_overlay(*overlay);
      } else if (ov.type == "parallax") {
        m_processor->set_parallax_overlay(*overlay);
      } else if (ov.type == "specular") {
        m_processor->set_specular_overlay(*overlay);
      } else if (ov.type == "occlussion") {
        m_processor->set_occlussion_overlay(*overlay);
      } else if (ov.type == "diffuse") {
        m_processor->set_texture_overlay(*overlay);
      }
    }
  }
  int xmin = fi.x() - radius, xmax = fi.x() + radius, ymin = fi.y() - radius,
      ymax = fi.y() + radius;

  QRect r(QPoint(xmin, ymin), QPoint(xmax, ymax));
  if (gui->get_normal_enabled()) {
    m_processor->set_normal_overlay(
        updateOverlay(xmin, xmax, ymin, ymax, m_processor->get_normal_overlay(),
                      oldHeight, auxHeight, height / 255.0));
    QtConcurrent::run(m_processor, &ImageProcessor::generate_normal_map, false,
                      false, false, r);
  }
  if (gui->get_parallax_enabled()) {
    m_processor->set_parallax_overlay(updateOverlay(
        xmin, xmax, ymin, ymax, m_processor->get_parallax_overlay(),
        oldParallax, auxParallax, parallax / 255.0));
    QtConcurrent::run(m_processor, &ImageProcessor::calculate_parallax);
  }
  if (gui->get_specular_enabled()) {
    m_processor->set_specular_overlay(updateOverlay(
        xmin, xmax, ymin, ymax, m_processor->get_specular_overlay(),
        oldSpecular, auxSpecular, spec / 255.0));
    QtConcurrent::run(m_processor, &ImageProcessor::calculate_specular);
  }
  if (gui->get_occlussion_enabled()) {
    m_processor->set_occlussion_overlay(updateOverlay(
        xmin, xmax, ymin, ymax, m_processor->get_occlusion_overlay(),
        oldOcclussion, auxOcclussion, occ / 255.0));
    QtConcurrent::run(m_processor, &ImageProcessor::calculate_occlusion);
  }
  if (gui->get_diffuse_enabled()) {
    m_processor->set_texture_overlay(updateOverlay(
        xmin, xmax, ymin, ymax, m_processor->get_texture_overlay(), oldDiffuse,
        auxDiffuse, diff / 255.0));
  }
}

void TextureBrush::mouseRelease(const QPoint &pos) {}

void TextureBrush::setProcessor(ImageProcessor **processor) {
  processorPtr = processor;
  (*processor)->get_current_frame()->get_image(TextureTypes::Diffuse, &diffuse);
}

QWidget *TextureBrush::loadGUI(QWidget *parent) {
  radius = 15;
  gui = new TextureBrushGui(parent);
  connect(gui, SIGNAL(selected_changed(bool)), this,
          SLOT(set_lineSelected(bool)));
  connect(gui, SIGNAL(size_changed(int)), this, SLOT(set_radius(int)));
  connect(gui, SIGNAL(hardness_changed(int)), this, SLOT(set_hardness(int)));

  connect(gui, SIGNAL(height_changed(int)), this, SLOT(set_height(int)));
  connect(gui, SIGNAL(specular_changed(int)), this, SLOT(set_specular(int)));
  connect(gui, SIGNAL(occlussion_changed(int)), this,
          SLOT(set_occlussion(int)));
  connect(gui, SIGNAL(parallax_changed(int)), this, SLOT(set_parallax(int)));
  connect(gui, SIGNAL(diffuse_changed(int)), this, SLOT(set_diffuse(int)));

  connect(gui, SIGNAL(mix_changed(int)), this, SLOT(set_mix(int)));
  connect(gui, SIGNAL(setTextures(QString, QStringList)), this,
          SLOT(setTextures(QString, QStringList)));
  connect(gui, SIGNAL(scaleChanged(double)), this, SLOT(set_scale(double)));
  updateBrushSprite();
  return gui;
}

void TextureBrush::setTextures(QString path, QStringList list) {

  current_texture_path = path;
  current_texture_list = list;
  gui->set_normal_enabled(false);
  gui->set_diffuse_enabled(false);
  gui->set_parallax_enabled(false);
  gui->set_specular_enabled(false);
  gui->set_occlusion_enabled(false);

  foreach (QString texture, list) {
    QImage image(path + "/" + texture);
    image = image.scaled(image.size() * scale);

    if (texture.contains("Normal")) {
      m_normal_brush.setTextureImage(image);
      gui->set_normal_enabled(true);
    } else if (texture.contains("Occlusion")) {
      m_occlussion_brush.setTextureImage(image);
      gui->set_occlusion_enabled(true);
    } else if (texture.contains("Metallness")) {
      m_specular_brush.setTextureImage(image);
      gui->set_specular_enabled(true);
    } else if (texture.contains("Displacement")) {
      m_parallax_brush.setTextureImage(image);
      m_heightmap_brush.setTextureImage(image);
      gui->set_parallax_enabled(true);
    } else if (texture.contains("Color")) {
      m_diffuse_brush.setTextureImage(image);
      gui->set_diffuse_enabled(true);
    }
  }
}

void TextureBrush::set_height(int height) {
  this->height = height;
  updateBrushSprite();
}

void TextureBrush::set_parallax(int p) { parallax = p; }

void TextureBrush::set_specular(int s) { spec = s; }

void TextureBrush::set_occlussion(int o) { occ = o; }

void TextureBrush::set_diffuse(int d) { diff = d; }

void TextureBrush::set_radius(int r) {
  radius = r;
  updateBrushSprite();
}

void TextureBrush::set_hardness(int h) {
  hardness = h / 100.0;
  updateBrushSprite();
}

void TextureBrush::set_max(int m) {
  maxV = m;
  updateBrushSprite();
}

void TextureBrush::set_min(int m) {
  minV = m;
  updateBrushSprite();
}

void TextureBrush::set_mix(int m) {
  alpha = m / 100.0;
  updateBrushSprite();
}

void TextureBrush::set_lineSelected(bool l) {
  linesSelected = l;
  set_selected(l);
  updateBrushSprite();
}

void TextureBrush::set_eraserSelected(bool e) {
  eraserSelected = e;
  if (e && !selected)
    set_selected(e);
  else if (!e && !brushSelected)
    set_selected(e);
}

void TextureBrush::set_brushSelected(bool b) {
  brushSelected = b;
  if (b && !selected)
    set_selected(b);
  else if (!b && !eraserSelected)
    set_selected(b);
}

QIcon TextureBrush::getIcon() { return QIcon(QPixmap(":/icon.png")); }

QString TextureBrush::getName() { return m_name; }

bool TextureBrush::get_selected() { return selected; }

void TextureBrush::set_selected(bool s) {
  selected = s;
  if (s) {
    selected_changed(this);
  } else {
    // TODO uncheck button
  }
  //  if (!s){
  //    gui->unselect_all();
  //  }
}

QImage TextureBrush::getBrushSprite() { return brushSprite; }

void TextureBrush::updateBrushSprite() {
  brushSprite = QImage(2 * radius, 2 * radius, QImage::Format_RGBA8888);
  brushSprite.fill(0.0);
  QPainter p(&brushSprite);
  maxV = 255;
  drawAt(QPoint(radius, radius), &p, true);

  brush_sprite_updated(brushSprite);
}

QObject *TextureBrush::getObject() { return this; }

int TextureBrush::WrapCoordinate(int coord, int interval) {
  coord %= interval;
  if (coord < 0) {
    coord += interval;
  }
  return coord;
}

QPoint TextureBrush::WorldToLocal(QPoint world) {
  int w = diffuse.width();
  int h = diffuse.height();

  QPoint origin = m_processor->get_position()->toPoint();

  QPoint local = world + QPoint(0.5 * w, -0.5 * h) - origin;
  local.setY(-local.y());

  return local;
}

void TextureBrush::set_scale(double s){
    scale = s;
    setTextures(current_texture_path, current_texture_list);
}
