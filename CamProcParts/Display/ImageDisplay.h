#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QLabel>
#include <QPainter>
#include <QImage>
#include <QDebug>
#include <QMessageBox>
#include <jpeglib.h>

#include "../ImgProcBin/ImgProcTypes.h"

class ImageDisplay : public QWidget
{
    Q_OBJECT
public:
    QImage m_o_image;
    QPixmap p_o_image;
    ImageDisplay(QWidget* parent = nullptr);

    void setImage(const VectorImage& image);
    void setOverlayImage(const VectorImage& overlayImageVect, qreal opacity);
    QImage vecToQImage(const VectorImage& image, bool base = false);
    void updatePixmap();
    QPixmap overlayPixmaps(const QPixmap& basePixmap, const QPixmap& overlayPixmap, qreal opacity);
    QPoint mapToPixmap(const QPoint& pos);
    
    void drawRect(VectorImage& vecImage, int x, int y, int height, int width, int color = 359);
    void drawRectFourCoords(VectorImage& vecImage, FourCoordinates coords, int color);
    void drawPoint(VectorImage& vecImage, int x, int y, int color = 359);
    void drawThickPoint(VectorImage& vecImage, int x, int y, int color = 359);
    void drawLine(VectorImage& vecImage, int x, int y, int length, int thickness, bool horver, int color = 359);
    void drawLineAngle(VectorImage& vecImage, int x, int y, int length, int thickness, double angleDeg, int color);
    void drawLineTwoPoints(VectorImage& vecImage, int x1, int y1, int x2, int y2, int color);
    void drawDiagonals(VectorImage& vecImage, const FourCoordinates& coords, int color);

    void setScaleToMin();
    void writeJPGImage(const char* fileName, const QImage& image, int quality = 90);
    void writeJPGCombinedImage(const char* fileName, const QPixmap& pixmap, int quality = 90);

    enum RecordROIState {NONE,FIRST_POINT,SECOND_POINT,CONFIRM};
    RecordROIState recroistate = RecordROIState::NONE;

    FourCoordinates newROI = FourCoordinates();
protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private:
    void adjustImagePosition(const QPoint& delta);

    QScrollArea* scrollArea;
    QVBoxLayout* widgetLayout;
    QLabel* imageLabel;
    QLabel* overlayLabel;
    QImage m_image;
    QImage m_overlayImage;
    std::vector<QPixmap> layers;
    QPixmap p_image;
    QPixmap p_image_base;
    QPixmap p_overlayImage;
    QPixmap p_combined;
    qreal m_scaleFactor;
    double m_opacity;
    int layerCounter;
    int base_image_h;
    int base_image_w;

    QPoint m_imagePosition;
    QPoint m_lastPos;
    bool m_isPanning;

};

#endif // IMAGEDISPLAY_H
