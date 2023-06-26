#include "draw.hpp"
#include "thread"

using namespace std;

void ImageDrawer::show_resulting_image(const ImageBin& image, const VectorImage vec_image)
{
    // --- Drawer setup
    if(!vec_image.empty()){
        std::cout << "DRAW: " << "Showing VectorImage" << std::endl;
        testImageVect = vec_image;
    }else{
        std::cout << "DRAW: " << "Showing ImageBin" << std::endl;
        testImageVect = im.imggen.ImageBinToVector(image, BitMapRows, BitMapCols, true);
    }
    window->setWindowTitle("Image Read from file");
    window->setCentralWidget(widget);
    widget->setImage(testImageVect);
    // --- END of setup
    drawEdge();
    drawDistFromEdgeToCross();
    drawLineSets();
    drawSearchBoundaries(im.imgopt.miscOpt.preProcType);
    drawMarkOnCross();
    setOpacities();
    saveCombinedImage();
    
    std::cout << "DRAW: " << "Preparing to open window" << std::endl;
    window->show();
    app->exec();
}
void ImageDrawer::setOpacities(){
    float opacity = 0.99;
    for(int i = 0; i < overLayers.size(); i++){
        if(i == 0) opacity = 0.60; else opacity = 0.99;
        widget->setOverlayImage(overLayers.at(i), 0.99);
    }
}

void ImageDrawer::saveCombinedImage(){
    // -- SAVING resulted combined image into jpeg
    std::string filename_out_relative = std::string();
    std::string filename_out = std::string();
    std::string filename_out_combined = std::string();
    
    filename_out_relative = get_next_image_filename(im.imgopt.fileNames.image_file_marked_dir_name_abs, cam_baseName, cam_extension);
    filename_out = im.imgopt.fileNames.image_file_marked_dir_name_abs + "/" + filename_out_relative;
    cout << "DRAW: " << "Filename: to write " << filename_out << endl;
    filename_out_combined = im.imgopt.fileNames.image_file_marked_dir_name_abs + "/" + "comb_" + filename_out_relative;
    
    widget->writeJPGImage(filename_out.c_str(),widget->m_o_image,75);
    widget->writeJPGCombinedImage(filename_out_combined.c_str(), widget->p_o_image); 

}

void ImageDrawer::drawEdge(){    

    // Draw Edge distance.
    if (im.blackEdge.first.first > 0){
        widget->drawLine(overLayers[0],
                        im.blackEdge.first.first - (im.blackEdge.first.first/2),
                        im.imgopt.preProcOpt.vec_img_h/2,
                        im.blackEdge.first.first,
                        1,
                        true,
                        HSV_ORANGE
                        );
                     
#ifdef DRAWDEBUG        
        std::cout << "DRAW: " << "! coords of edge: x( " << im.blackEdge.first.first << " ; " << im.imgopt.preProcOpt.vec_img_h/2 << ") , Length: " << 
                    im.imgopt.preProcOpt.vec_img_h << " Angle: " << - (90 - im.blackEdge.second) << std::endl;
#endif
    // The edge itslef
        widget->drawLineAngle(overLayers[0], 
                            im.blackEdge.first.first,
                            im.imgopt.preProcOpt.vec_img_h/2,
                            im.imgopt.preProcOpt.vec_img_h,
                            3,
                            (90 - im.blackEdge.second),
                            HSV_YELLOW
                            );

    // 
    }

}

void ImageDrawer::drawLineSets(){
    // Draw Set of Horizontal lines
    for (auto i = 0; i < im.imgproc.hor_line_set.size(); i++) {
        color_h_l +=i; if (color_h_l >= HSV_RED) color_h_l = 0;
        widget->drawLine(
            overLayers[0],
            im.imgproc.hor_line_set[i].first,
            im.imgproc.hor_line_set[i].second,
            im.imgopt.finderOpt.target_line_length_px, 
            im.imgopt.finderOpt.target_line_width_px,
            true,
            color_h_l
        );
    }
    // Draw Set of Vertical lines
    for (auto i = 0; i < im.imgproc.ver_line_set.size(); i++) {
        color_v_l += 10*i; if (color_v_l >= HSV_RED) color_v_l = 0;
        widget->drawLine(overLayers[1], 
                        im.imgproc.ver_line_set[i].first,
                        im.imgproc.ver_line_set[i].second, 
                        im.imgopt.finderOpt.target_line_length_px, 
                        im.imgopt.finderOpt.target_line_width_px, 
                        false, 
                        color_v_l);
    }
     // Drawing middle points of horizontal lines set
    for (auto i = 0; i < im.imgproc.hor_line_set.size(); i++) {
        color_h_p += i; if (i % 2 == 0) { color_h_p = color_h_p / 2; color_h_p += i; }
        widget->drawPoint(overLayers[2], im.imgproc.hor_line_set[i].first, im.imgproc.hor_line_set[i].second, color_h_p);
    }
    // Drawing middle points of verical lines set
    for (auto i = 0; i < im.imgproc.ver_line_set.size(); i++) {
        color_v_p += i; if (i % 2 == 0) { color_v_p = color_v_p / 2; color_v_p += i; }
        widget->drawPoint(overLayers[3], im.imgproc.ver_line_set[i].first, im.imgproc.ver_line_set[i].second, color_v_p);
    }

}

void ImageDrawer::drawDistFromEdgeToCross(){
     // Line form edge to cros
    int dist_lay = 0;
    for(const auto& cred : im.crosses_from_edge_hor){
        if(!im.crosses.empty())
            std::cout << "DRAW drawing lines from cross " << std::endl;
            widget->drawLineTwoPoints(overLayers[dist_lay],
                                    im.crosses[dist_lay].first - cred.first,
                                    im.crosses[dist_lay].second,
                                    im.crosses[dist_lay].first,
                                    im.crosses[dist_lay].second,
                                    HSV_CRIMSON
                                );
    }
        if(!im.crosses.empty())
            widget->drawLineTwoPoints(overLayers[dist_lay],
                                    im.crosses[dist_lay].first,
                                    im.crosses[dist_lay].second - im.crosses_from_edge_ver[0].first,
                                    im.crosses[dist_lay].first,
                                    im.crosses[dist_lay].second,
                                    HSV_CRIMSON    
                                    );
    
}

void ImageDrawer::drawSearchBoundaries(ImageProcessingOptions::PreProcType preProcType){
     // For regular search margins on a big image:
    if (preProcType == ImageProcessingOptions::PreProcType::NONE) {
        widget->drawRect(overLayers[6],
                        im.imgopt.preProcOpt.region.first.first, 
                        im.imgopt.preProcOpt.region.first.second,
                        im.imgopt.preProcOpt.region.second.first,
                        im.imgopt.preProcOpt.region.second.second,
                        HSV_GREEN);
    }
    // Drawing search chunks of image                        
    if (preProcType == ImageProcessingOptions::PreProcType::SEQCHUNK || 
        preProcType == ImageProcessingOptions::PreProcType::MASSCLUST ) {
        if(!im.imgproc.sub_images_debug.empty()){
            // For sub images:
            for (int i = 0; i < im.imgproc.sub_images_debug.size(); i++) {
                int color; 
                if (im.imgproc.sub_image_weight_debug[i].first > 0) { color = HSV_ORANGE; }
                else {color = HSV_GREEN;}
                widget->drawRectFourCoords(overLayers[6], im.imgproc.sub_images_debug[i], color);
                widget->drawDiagonals(overLayers[6], im.imgproc.sub_images_debug[i], color);    
            }
        }
        if (preProcType == ImageProcessingOptions::PreProcType::MASSCLUST) {
            // Mass Cluster Rectangle
            for (const auto& cl : im.imgproc.clusters_debug) {
                widget->drawRectFourCoords(overLayers[6], cl, HSV_PURPLE);
            }
            // Mass Cluster Center of Mass point
            for(const auto& com : im.imgproc.sub_image_centreOfmass_debug) {
                if (com.first != TwoCoordinates()){
                        widget->drawThickPoint(overLayers[6], com.first.first,
                                                            com.first.second, HSV_PURPLE);
                        widget->drawPoint(overLayers[6], com.first.first,
                                                            com.first.second, HSV_GREEN);
                }
            }
        }
    }

}

void ImageDrawer::drawMarkOnCross(){
        // Marking the cross(es)
    if (!im.crosses.empty()) {
        for(int i = 0; i < im.crosses.size(); i++){
            widget->drawPoint(overLayers[4], im.crosses[i].first, im.crosses[i].second);
            int x1 = im.crosses[i].first - im.imgopt.finderOpt.target_line_length_px / 2;
            int y1 = im.crosses[i].second - im.imgopt.finderOpt.target_line_length_px / 2;
            widget->drawRect(overLayers[5], x1, y1, im.imgopt.finderOpt.target_line_length_px,
                                                    im.imgopt.finderOpt.target_line_length_px,
                                                    HSV_RED
                                                    );
#ifdef DRAWDEBUG            
            std::cout << "DRAW: " << "Cross found at: " << im.crosses[i].first << ", " 
                                                        << im.crosses[i].second << " absolute pixel values" << std::endl;
            if (im.blackEdge != TwoCoordinatesAndAngle()) {
                if(i <= im.crosses_from_edge_hor.size()-1)
                std::cout << "DRAW: " << "Cross found at: " << im.crosses_from_edge_hor[i].first 
                                                    << ", " << im.crosses_from_edge_hor[i].second 
                                                    << " from the edge" << std::endl;
                if(i <= im.crosses_from_edge_ver.size()-1)
                std::cout << "DRAW: " << "!2 Cross found at: " << im.crosses_from_edge_ver[i].first 
                                                    << ", " << im.crosses_from_edge_ver[i].second 
                                                    << " from the edge" << std::endl;
            }
#endif
        }
    } 
}