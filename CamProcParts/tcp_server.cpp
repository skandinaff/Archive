#include <boost/bind/bind.hpp>
#include <iostream>
#include "tcp_server.h"
#include <sys/syscall.h>
#include "../ImgProcBin/ImgProcTypes.h"
#include "vector_file_writer.h"

TcpServer::TcpServer(boost::asio::io_context& io_context, 
                        uint16_t port,
                        std::queue<std::string>& dataQueue, 
                        std::mutex& stateMutex, 
                        std::condition_variable& stateCV,
                        std::atomic<bool>& exitFlag,
                        std::atomic<bool>& dataReady,
                        std::queue<VectorImage>& imageQueue
                        )
    : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    dataQueue_(dataQueue),
    stateMutex_(stateMutex),
    stateCV_(stateCV),
    exitFlag_(exitFlag),
    dataReady_(dataReady),
    imageQueue_(imageQueue)

{
}
void TcpServer::start()
{
    pid_t tid = syscall(SYS_gettid); std::cout << "TCP server Thread LWP ID: " << tid << std::endl;
    acceptor_.async_accept([this](const boost::system::error_code& error, boost::asio::ip::tcp::socket socket) {
        std::cout << "TCP SERVER: async accept" << std::endl;
        if (!error) {
            handleConnection(std::move(socket));
        }
        else {
            // Handle the error
        }

        // Start accepting new connections
        start();
    });
}

void TcpServer::handleConnection(boost::asio::ip::tcp::socket&& socket) {

    try
    {
        while(!exitFlag_.load()){
            std::unique_lock<std::mutex> lock(stateMutex_);
            stateCV_.wait(lock, [this]() { return dataReady_.load(); });
            if (!imageQueue_.empty())
            {
                std::cout << "TCP SERVER: Got data on queue" << std::endl;
                VectorImage imageToSend = imageQueue_.front();
                imageQueue_.pop();
                //std::vector<uint8_t> serializedData = serializeVectorImage(imageToSend);
                std::vector<uint8_t> serializedData = serializeImageToBytes(imageToSend);
                writeVectorToFile(serializedData, "file_to_be_sent.txt");
                boost::asio::write(socket, boost::asio::buffer(serializedData));
                std::cout << "TCP SERVER: Data theoretically sent to the socket" << std::endl;
            }
            std::atomic_store(&dataReady_, false);
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error in handleConnection: " << ex.what() << std::endl;
    }
}

std::string TcpServer::receiveData(boost::asio::ip::tcp::socket& socket)
{
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, '\n');
    return boost::asio::buffer_cast<const char*>(buffer.data());
}

void TcpServer::sendData(boost::asio::ip::tcp::socket& socket, const std::string& data)
{
    boost::asio::write(socket, boost::asio::buffer(data));
}

std::vector<uint8_t> TcpServer::serializeVectorImage(const VectorImage& vectorImage) {
  // Calculate the total size of the vectorImage
  size_t totalSize = vectorImage.size() * vectorImage[0].size() * sizeof(int);
  // Serialize the vectorImage into a byte stream
  std::vector<uint8_t> serializedData;
  serializedData.push_back('V');
  serializedData.resize(totalSize);

  uint8_t* dataPtr = serializedData.data() + 1;  // Start after the type identifier
  for (const auto& row : vectorImage) {
    memcpy(dataPtr, row.data(), row.size() * sizeof(int));
    dataPtr += row.size() * sizeof(int);
  }

  return serializedData;
}

VectorImage TcpServer::deserializeVectorImage(const std::vector<uint8_t> serializedData) {
    //const std::vector<uint8_t> serializedData(serializedData_.begin(), serializedData_.end());
    // Verify that the data has the correct type identifier
    if (serializedData.empty()) {
        // Handle invalid data or incorrect type identifier
        // For example, you could throw an exception or return an empty VectorImage
        return VectorImage();
    }
    // Calculate the number of rows and columns in the VectorImage
    size_t numRows = BitMapRows;
    size_t numCols = BitMapColsPx;

    // Create a VectorImage object
    VectorImage vectorImage(numRows, std::vector<uint8_t>(numCols));

    // Deserialize the data
    const uint8_t* dataPtr = serializedData.data() + 1; // Start after the type identifier
    for (auto& row : vectorImage) {
        memcpy(row.data(), dataPtr, numCols * sizeof(int));
        dataPtr += numCols * sizeof(int);
    }
    return vectorImage;
}

std::vector<uint8_t> TcpServer::serializeImageProcessingManager(const ImageProcessingManager &manager) {
    std::vector<uint8_t> serializedData;
  // Serialize the ImageProcessingManager object into a byte stream
  // Implement your serialization logic here
    return serializedData;
}

VectorImage TcpServer::deserializeImage(const std::vector<uint8_t>& flattenedImage, int width, int height) {
    VectorImage image;
    int index = 0;
    for (int i = 0; i < height; ++i) {
        image.emplace_back(flattenedImage.begin() + index, flattenedImage.begin() + index + width);
        index += width;
    }
    return image;
}
std::vector<uint8_t> TcpServer::serializeImage(const VectorImage& image) {
    std::vector<uint8_t> flattenedImage;
    flattenedImage.push_back('V');
    for (const auto& row : image) {
        flattenedImage.insert(flattenedImage.end(), row.begin(), row.end());
    }
    return flattenedImage;
}
std::vector<uint8_t> TcpServer::serializeImageToBytes(const VectorImage& image) {
    std::vector<uint8_t> flattenedImage = serializeImage(image);
    std::vector<uint8_t> serializedData;
    for (const auto& value : flattenedImage) {
        serializedData.push_back(static_cast<uint8_t>(value));
    }
    return serializedData;
}