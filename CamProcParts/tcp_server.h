#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <boost/asio.hpp>
#include <cstdint>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "common.h"
#include "../ImgProcBin/ImageProcessingManager.hpp"

class TcpServer {
public:
    TcpServer(boost::asio::io_context& io_context, 
                        uint16_t port,
                        std::queue<std::string>& dataQueue, 
                        std::mutex& stateMutex, 
                        std::condition_variable& stateCV,
                        std::atomic<bool>& exitFlag,
                        std::atomic<bool>& dataReady,
                        std::queue<VectorImage>& imageQueue
                        );
    // Start the server and listen for incoming connections
    void start();

    static std::vector<uint8_t> serializeVectorImage(const VectorImage& vectorImage);
    static VectorImage deserializeVectorImage(const std::vector<uint8_t> serializedData);

    static VectorImage deserializeImage(const std::vector<uint8_t>& flattenedImage, int width, int height);
    static std::vector<uint8_t> serializeImage(const VectorImage& image);
    static std::vector<uint8_t> serializeImageToBytes(const VectorImage& image);
private:

    void handleConnection(boost::asio::ip::tcp::socket&& socket);
    std::vector<uint8_t> serializeImageProcessingManager(const ImageProcessingManager &manager);
    std::string receiveData(boost::asio::ip::tcp::socket& socket);
    void sendData(boost::asio::ip::tcp::socket& socket, const std::string& data);

    boost::asio::ip::tcp::acceptor acceptor_;
    std::queue<std::string>& dataQueue_;
    std::mutex& stateMutex_;
    std::condition_variable& stateCV_;
    std::atomic<bool>& exitFlag_;
    std::atomic<bool>& dataReady_;
    std::queue<VectorImage>& imageQueue_;
};

#endif  // TCP_SERVER_H
