#pragma once
#ifndef SERIAL_SENDER_H
#define SERIAL_SENDER_H

#include <vector>
#include <boost/asio.hpp>

class SerialSender {
    public:
        SerialSender();        
        ~SerialSender();        

        enum Baud {
            BR_9600 = 9600,
            BR_19200 = 19200,
            BR_115200 = 115200
        };
         
        void setSerialPath(std::string path);
        void setBaud(Baud b);
        std::string getSerialPath();        
        bool connect(Baud b);  
        bool connect();
        bool disconnect(); 
        bool isOpen();
        std::vector<std::string> findAvailableSerialPorts();
        std::string setSerialPath();

        bool sendByteVector(std::vector<uint8_t> bytes);
        size_t sendByteVectorAndRead(std::vector<uint8_t> bytes, std::vector<uint8_t>& bytesRead, 
                                     size_t maxBytesToRead);
        bool sendByteVectors(std::vector<std::vector<uint8_t>> bytes, std::vector<int> delays);        
        size_t sendByteVectorsAndRead(std::vector<std::vector<uint8_t>> bytes,
            std::vector<std::vector<uint8_t>>& bytesRead, std::vector<int> delays);        
        static void handler(const boost::system::error_code& ec, std::size_t bytes_transferred);

    private:       
        std::string serial_path;
        boost::asio::io_service io;
        boost::asio::serial_port sp;
};
#endif
