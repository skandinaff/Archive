#ifndef DRAW_H
#define DRAW_H

#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QImage>
#include <QPainter>
#include <chrono>
//#include <QTimer>

#include "../ImgProcBin/ImageProcessingBin.h"
#include "../ImgProcBin/ImageGeneratorBin.h"
#include "../ImgProcBin/ImgProcTypes.h"
#include "../ImgProcBin/ImageProcessingOptions.hpp"
#include "../ImgProcBin/ImageProcessingManager.hpp"

#include "../Core/img_app.hpp"
#include "ImageDisplay.h"
#include "file_handler.h"

enum HSVColors {
    HSV_RED = 360,
    HSV_GREEN = 88,
    HSV_ORANGE = 40,
    HSV_PURPLE = 268,
    HSV_YELLOW = 48,
    HSV_TURQUOISE = 180,
    HSV_DARK_OLIVE = 55,
    HSV_DARK_PURPLE = 305,
    HSV_DARK_TURQUOISE = 175,
    HSV_CRIMSON = 320
};

class ImageDrawer : public QObject
{
    Q_OBJECT
private:
    int layers_num = 7;
    int color_h_l = HSV_TURQUOISE; 
    int color_v_l = HSV_DARK_OLIVE;
    int color_h_p = HSV_DARK_PURPLE;
    int color_v_p = HSV_DARK_TURQUOISE;
    QApplication* app;
    FourCoordinates newROI = FourCoordinates();
    VectorImage testImageVect;
    ImageProcessingManager& im;
    std::vector<VectorImage> overLayers;
public:
    ImageDrawer(ImageProcessingManager& imRef) :
        im(imRef),
        overLayers(layers_num, VectorImage(BitMapRows, VectorImageRow(BitMapColsPx)))
        {
        int argc = 0;
        app = new QApplication(argc, nullptr);
        window = new QMainWindow;
        widget = new ImageDisplay;
        std::cout << "Image widget created\n";
        if (im.imgopt.preProcOpt.vec_img_w > 1920) widget->setScaleToMin();
    }

    ~ImageDrawer() {
        delete widget;
        delete window;
        delete app;
    }
    void show_resulting_image(const ImageBin& image = ImageBin{}, const VectorImage vec_image = VectorImage{} );
    void close_window(){
        app->closeAllWindows();
    }
    void drawEdge();
    void drawLineSets();
    void drawDistFromEdgeToCross();
    void drawSearchBoundaries(ImageProcessingOptions::PreProcType preProcType);
    void drawMarkOnCross();
    void saveCombinedImage();
    void setOpacities();

    FourCoordinates getROI() {
        return widget->newROI;
    }
    QMainWindow* window;
    ImageDisplay* widget;
};


#endif