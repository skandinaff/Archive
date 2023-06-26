#include "ImageDisplay.h"
#include <iostream>
#include <iomanip>
#include <cmath>

ImageDisplay::ImageDisplay(QWidget* parent)
    : QWidget(parent)
    , m_scaleFactor(1)
    , layerCounter(0)
{
    //layerCounter = 1;
    // Set up the scroll area
    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidgetResizable(true);

    //widgetLayout = new QVBoxLayout(scrollArea);

    // Set up the image label
    imageLabel = new QLabel(scrollArea);
    imageLabel->setBackgroundRole(QPalette::Midlight);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageLabel->setScaledContents(false);
    imageLabel->setAlignment(Qt::AlignCenter);
    // Add the image label to the scroll area
    scrollArea->setWidget(imageLabel);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Set up the layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea);
    
    

    // Set the minimum size of the widget
    setMinimumSize(640, 640);
}

QImage ImageDisplay::vecToQImage(const VectorImage& image, bool base) {
    int width = image[0].size();
    int height = image.size();
    if (base) {
        base_image_h = height; base_image_w = width;
    }
    QImage r_image = QImage(width, height, QImage::Format_ARGB32);
    QColor color;
    int black_value_0 = 1, black_value_1 = 255, white_value_0 = 0, white_value_1 = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int value = image[y][x];
            //if (value == 1 || value == 255) r_image.setPixel(x, y, qRgba(0, 0, 0, 255)); // Black
            if (value == black_value_0 || value == black_value_1) { r_image.setPixel(x, y, qRgb(0, 0, 0)); }
            else if (value == white_value_0 || value == white_value_1) {
                if (base) r_image.setPixel(x, y, qRgba(255, 255, 255, 255)); // White
                else r_image.setPixel(x, y, qRgba(255, 255, 255,0)); // Transparent
            }

            else {
                color.setHsv(value, 255, 255);
                int r, g, b;
                color.getRgb(&r, &g, &b);
                r_image.setPixel(x, y, qRgba(r, g, b,180));
            }
        }
    }
    return r_image;
}

void ImageDisplay::setImage(const VectorImage& image)
{
    m_image = vecToQImage(image, true);
    m_o_image = m_image;
    p_image = QPixmap::fromImage(m_image);
    p_image_base = p_image;
    imageLabel->setPixmap(p_image);
    imageLabel->adjustSize();
    

    // Set the scroll bar range to the size of the image
    scrollArea->setWidgetResizable(true);
    scrollArea->sizeAdjustPolicy();    
    
    
    update();

}

void ImageDisplay::paintEvent(QPaintEvent* event){
    int h = p_image.height();
    int w = p_image.width();    
    imageLabel->setPixmap(p_image.scaled(w*m_scaleFactor,h*m_scaleFactor,Qt::KeepAspectRatio));
    imageLabel->adjustSize();
 
}

void ImageDisplay::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        m_scaleFactor += event->angleDelta().y() / 1000.0;
        m_scaleFactor = qMax(0.1, qMin(m_scaleFactor, 10.0));
        update();
    }
}

void ImageDisplay::keyPressEvent(QKeyEvent* event) {
    if(event->modifiers() == Qt::CTRL){
        switch (event->key())
        {
        case Qt::Key_0:
            m_scaleFactor = 1.0;
            break;
        case Qt::Key_1:
            m_scaleFactor = 0.2;
            break;
        case Qt::Key_2:
            m_scaleFactor = 0.3;
            break;
        case Qt::Key_3:
            m_scaleFactor = 0.4;
            break;
        case Qt::Key_4:
            m_scaleFactor = 0.5;
            break;
        case Qt::Key_Equal:
            m_scaleFactor += 0.1; 
            break;
        case Qt::Key_Minus:
            m_scaleFactor -= 0.1;
            break;
        default:
            break;
        }
        update();
    }
}

void ImageDisplay::setOverlayImage(const VectorImage& overlayImageVect, qreal opacity)
{
    // Set the overlay image in the label and resize the label to fit
    m_opacity = opacity;
    QPixmap p_result;
    m_overlayImage = vecToQImage(overlayImageVect, false);
    layers.push_back(QPixmap::fromImage(m_overlayImage));
    p_image = overlayPixmaps( p_image, layers.back(), m_opacity);
    layerCounter++;
    p_o_image = p_image;
    imageLabel->setPixmap(p_image);
    imageLabel->adjustSize();
    update();

    // Update the display to show the overlay image

}

QPixmap ImageDisplay::overlayPixmaps(const QPixmap& basePixmap, const QPixmap& overlayPixmap, qreal opacity)
{
    // Create a new pixmap with the same size as the base pixmap
    QPixmap resultPixmap(basePixmap.size());

    // Create a painter for the new pixmap
    QPainter painter(&resultPixmap);
    if(opacity < 1.0) painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    // Draw the base pixmap onto the new pixmap
    painter.drawPixmap(0, 0, basePixmap);

    // Set the opacity of the painter
    painter.setOpacity(opacity);

    // Draw the overlay pixmap onto the new pixmap
    painter.drawPixmap(0, 0, overlayPixmap);

    // Return the result pixmap
    return resultPixmap;
}

void ImageDisplay::mousePressEvent(QMouseEvent* event){
    int x; int y;
    if (imageLabel->underMouse() && event->button() == Qt::LeftButton) {
    //if(){
        // Get the position of the mouse in the image label
        QPoint pos = event->pos();
        // Get the position of the mouse in the pixmap
        QPoint labelPos = imageLabel->mapFrom(this, pos);
        // Get the position of the mouse in the pixmap's coordinate system
        QPoint pixmapPos = mapToPixmap(labelPos);
        // Get the size of the pixmap
        QSize pixmapSize = imageLabel->pixmap()->size();
        QSize labelSize = imageLabel->size();
        // Calculate the coordinates of the pixel at the mouse position
        int deltaX = (labelSize.width() - pixmapSize.width()) / 2;
        int deltaY = (labelSize.height() - pixmapSize.height()) / 2;
        x = labelPos.x() - deltaX;
        y = labelPos.y() - deltaY;

        // Output the pixel coordinates to the console
        //std::cout << "Label size: (" << labelSize.width() << "," << labelSize.height() << ") "; 
        //std::cout << "Pixmap size: (" << pixmapSize.width() << "," << pixmapSize.height() << ") ";
        //std::cout << "Scale Factor: " << m_scaleFactor << std::endl;
        //std::cout << "Pixel in image coords: (" << x / m_scaleFactor << "," << y / m_scaleFactor << ") ";// << std::endl;
        //std::cout << "Pixmap coords: (" << pixmapPos.x() * m_scaleFactor << "," << pixmapPos.y() * m_scaleFactor << ") ";// << std::endl;
        //std::cout << "Label coords: (" << labelPos.x() * m_scaleFactor << "," << labelPos.y() * m_scaleFactor << ")" << std::endl; // Same as pixmap
        std::cout << "Scale Factor: " << m_scaleFactor << std::endl;
        std::cout << "Pixel in image coords: (" << x / m_scaleFactor << "; " << y / m_scaleFactor << ") "<< std::endl;
    }
    if (event->button() == Qt::RightButton ) {
        m_isPanning = true;
        m_lastPos = event->pos();
    }
    if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier){
        std::cout << "CTRL + mouse Pressed!" << std::endl;
        if(recroistate == RecordROIState::NONE){
            std::cout << "New ROI first point obtained!" << std::endl;
            newROI.first.first = x/m_scaleFactor;
            newROI.first.second = y/m_scaleFactor;
            recroistate = RecordROIState::FIRST_POINT;
            return;
        }
        if(recroistate == RecordROIState::FIRST_POINT){
            std::cout << "New ROI second point obtained!" << std::endl;
            newROI.second.first = x/m_scaleFactor;
            newROI.second.second = y/m_scaleFactor;
            recroistate = RecordROIState::SECOND_POINT;

            int result = QMessageBox::question(this, "Confirmation", "Save new region of interest?", QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes) {
                std::cout << "Your new ROI recorded as: {" << newROI.first.first << "," << newROI.first.second << "}" <<
                                                      " {" << newROI.second.first << "," << newROI.second.second << "}" << std::endl;   
                recroistate = RecordROIState::NONE;

            } else {
                // User canceled, reset the state to FIRST_POINT to record a new ROI
                std::cout << "Current ROI discarede" << std::endl;
                newROI = FourCoordinates();
                recroistate = RecordROIState::NONE;
            }
            return;            
        }
    }
}

void ImageDisplay::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        m_isPanning = false;
    }
}

void ImageDisplay::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastPos;
        m_lastPos = event->pos();
        adjustImagePosition(delta);
        update();
    }
}

void ImageDisplay::adjustImagePosition(const QPoint& delta) {
    // Get the current scroll position
    int horizontalScrollValue = scrollArea->horizontalScrollBar()->value();
    int verticalScrollValue = scrollArea->verticalScrollBar()->value();
    // Adjust the scroll position based on the delta
    scrollArea->horizontalScrollBar()->setValue(horizontalScrollValue - delta.x());
    scrollArea->verticalScrollBar()->setValue(verticalScrollValue - delta.y());
}

QPoint ImageDisplay::mapToPixmap(const QPoint& pos)
{
    // Get the position of the pixmap within the label
    QPoint pixmapPos = imageLabel->pos();

    // Calculate the position of the point in the pixmap's coordinate system
    int x = pos.x() - pixmapPos.x();
    int y = pos.y() - pixmapPos.y();

    // Convert the position to a floating point value
    float xf = (float)x;
    float yf = (float)y;

    // Get the size of the pixmap
    QSize pixmapSize = imageLabel->pixmap()->size();

    // Calculate the scale factors to convert from label coordinates to pixmap coordinates
    float xscale = pixmapSize.width() / (float)imageLabel->width();
    float yscale = pixmapSize.height() / (float)imageLabel->height();

    // Apply the scale factors to get the position in the pixmap's coordinate system
    xf *= xscale;
    yf *= yscale;

    // Return the position as a QPointF
    return QPoint(xf, yf);
}



void ImageDisplay::drawRect(VectorImage& vecImage, int x, int y, int height, int width, int color) {
    // Calculate the boundaries of the rectangle that are within the image bounds
    int x1 = std::max(x, 0);
    int y1 = std::max(y, 0);
    int x2 = std::min(x + width - 1, (int)vecImage[0].size() - 1);
    int y2 = std::min(y + height - 1, (int)vecImage.size() - 1);

    // Draw the rectangle within the image bounds
    for (int i = y1; i <= y2; i++) {
        vecImage[i][x1] = color;
        vecImage[i][x2] = color;
    }
    for (int j = x1; j <= x2; j++) {
        vecImage[y1][j] = color;
        vecImage[y2][j] = color;
    }
}

void ImageDisplay::drawRectFourCoords(VectorImage& vecImage, FourCoordinates coords, int color) {
    int x1 = coords.first.first;
    int y1 = coords.first.second;
    int x2 = coords.second.first;
    int y2 = coords.second.second;
    
    int height = coords.second.second - coords.first.second;
    int width = coords.second.first - coords.first.first;
    //int height = y2 - y1;
    //int width = x2 - x1;

    for (int i = y1; i < y2; i++) {
        if(i < vecImage.size() && x1 < vecImage[i].size() && x2 < vecImage[i].size()){
            vecImage[i][x1] = color;
            vecImage[i][x2] = color;
        }
    }
    for (int j = x1; j < x2; j++) {
        if (j < vecImage.size() && y1 < vecImage.size() && y2 < vecImage.size()) {
            vecImage[y1][j] = color;
            vecImage[y2][j] = color;
        }
    }
}

void ImageDisplay::drawLine(VectorImage& vecImage, int x, int y, int length, int thickness, bool horver, int color) {
    if (horver) { // draw horizontal line
        int x_start = x - length / 2;
        int x_end = x_start + length - 1;
        if (x_end > vecImage[0].size()) x_end = vecImage[0].size() - 1;

        for (int i = y - thickness / 2; i <= y + thickness / 2; ++i) {
            if (i < 0 || i >= vecImage.size()) continue;
            for (int j = x_start; j <= x_end; ++j) {
                if (j < 0 || j >= vecImage[0].size()) continue;
                vecImage[i][j] = color;
            }
        }
    }
    else { // draw vertical line
        int y_start = y - length / 2;
        int y_end = y_start + length - 1;
        if (y_end > vecImage.size()) y_end = vecImage.size() - 1;

        for (int i = x - thickness / 2; i <= x + thickness / 2; ++i) {
            if (i < 0 || i >= vecImage.size()) continue;
            for (int j = y_start; j <= y_end; ++j) {
                if (j < 0 || j >= vecImage[0].size()) continue;
                vecImage[j][i] = color;
            }
        }
    }
}



void ImageDisplay::drawLineAngle(VectorImage& vecImage, int x, int y, int length, int thickness, double angleDeg, int color) {
    // Convert angle from degrees to radians
    double angleRad = angleDeg * M_PI / 180.0;

    // Calculate the half length of the line
    double halfLength = length / 2.0;

    // Calculate the displacement from the midpoint to the endpoints
    double dx = halfLength * cos(angleRad);
    double dy = halfLength * sin(angleRad);

    // Calculate the start and end coordinates of the line
    double x_start = x - dx;
    double y_start = y - dy;
    double x_end = x + dx;
    double y_end = y + dy;

    // Determine the step size for iterating along the line
    double stepSize = 1.0 / std::max(std::abs(dx), std::abs(dy));

    // Iterate along the line and set the pixel values
    for (double t = 0.0; t <= 1.0; t += stepSize) {
        int px = static_cast<int>(x_start + t * (x_end - x_start));
        int py = static_cast<int>(y_start + t * (y_end - y_start));

        for (int i = py - thickness / 2; i <= py + thickness / 2; ++i) {
            for (int j = px - thickness / 2; j <= px + thickness / 2; ++j) {
                if (i >= 0 && i < vecImage.size() && j >= 0 && j < vecImage[0].size()) {
                    vecImage[i][j] = color;
                }
            }
        }
    }

}


void ImageDisplay::drawLineTwoPoints(VectorImage& vecImage, int x1, int y1, int x2, int y2, int color) {
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = abs(y2 - y1);
    int error = dx / 2;
    int ystep = (y1 < y2) ? 1 : -1;
    int y = y1;
    for (int x = x1; x <= x2; ++x) {
        if (steep) {
            if (x >= 0 && x < vecImage.size() && y >= 0 && y < vecImage[0].size()) {
                vecImage[x][y] = color;
            }
        }
        else {
            if (x >= 0 && x < vecImage[0].size() && y >= 0 && y < vecImage.size()) {
                vecImage[y][x] = color;
            }
        }
        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}


void ImageDisplay::drawPoint(VectorImage& vecImage, int x, int y, int color){
    vecImage[y][x] = color;
}
void ImageDisplay::drawThickPoint(VectorImage& vecImage, int x, int y, int color) {
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            if (x > 0 && y > 0) {
                vecImage[y+j][x+i] = color;
            }
        }
    }
}
void ImageDisplay::drawDiagonals(VectorImage& vecImage, const FourCoordinates& coords, int color)
{
    int x1 = coords.first.first;
    int y1 = coords.first.second;
    int x2 = coords.second.first;
    int y2 = coords.second.second;

    drawLineTwoPoints(vecImage, x1, y1, x2, y2, color);
    drawLineTwoPoints(vecImage, x1, y2, x2, y1, color);
}

void ImageDisplay::setScaleToMin() {
        m_scaleFactor = 0.4;
        update();
}

void ImageDisplay::writeJPGCombinedImage(const char* fileName, const QPixmap& pixmap, int quality) {
    QImage image = pixmap.toImage(); // Convert QPixmap to QImage
    writeJPGImage(fileName, image, quality);
}


void ImageDisplay::writeJPGImage(const char* fileName, const QImage& image, int quality) {
    FILE* outfile = fopen(fileName, "wb");
    if (outfile == nullptr) {
        qCritical() << "Error: could not open JPG file." << fileName;
        return;
    }

    // Set up the JPEG compression object
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    // Set options for compression
    cinfo.image_width = image.width();
    cinfo.image_height = image.height();
    cinfo.input_components = 3; // RGB image
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Allocate buffer for row of pixels
    JSAMPROW rowBuffer = new JSAMPLE[cinfo.image_width * cinfo.input_components];

    // Write scanlines
    while (cinfo.next_scanline < cinfo.image_height) {
        const uchar* scanline = image.constScanLine(cinfo.next_scanline);
        for (int col = 0; col < cinfo.image_width; col++) {
            const uchar* pixel = scanline + col * 4; // 4 bytes per pixel (RGBA)
            rowBuffer[col * 3] = pixel[2];     // Red
            rowBuffer[col * 3 + 1] = pixel[1]; // Green
            rowBuffer[col * 3 + 2] = pixel[0]; // Blue
        }
        jpeg_write_scanlines(&cinfo, &rowBuffer, 1);
    }

    // Clean up
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    delete[] rowBuffer;
}
