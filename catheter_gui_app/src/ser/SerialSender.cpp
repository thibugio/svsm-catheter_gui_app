#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "common_utils.h"
#include "SerialSender.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>


SerialSender::SerialSender() 
: io(), sp(io) {
    setBaud(Baud::BR_9600);
    serial_path = setSerialPath();
}

SerialSender::~SerialSender() {
    if (sp.is_open()) 
        sp.close();
}

bool SerialSender::isOpen() {
    return sp.is_open();
}

void SerialSender::setBaud(Baud b) {
    boost::system::error_code ec;
    unsigned int baud = static_cast<int>(b);
    sp.set_option(boost::asio::serial_port_base::baud_rate(baud), ec);
    if (ec) {
        printf("Error setting Baud Rate.\n");
    }
}

std::vector<std::string> SerialSender::findAvailableSerialPorts() {
    std::vector<std::string> ports;
    #ifdef __WXMSW__ 
        boost::system::error_code ec;
        for (int i = 0; i < 64; i++) {
            char p[7] = "COM";
            char n[3];
            sprintf(n, "%d", i);
            sp.open(std::strcat(p, n), ec);
            if (!ec) {
                ports.push_back(std::string(p));
                sp.close();
            }
            ec.clear();
        }
        if (sp.is_open())
            sp.close();
    #else
        path = "/dev/tty";
        FILE *pipe = std::popen("ls /dev/tty* | egrep -o \"(ACM|USB)[0-9]\" | tr -d '\n'", "r");
        if (!pipe) return false;
        char buf[8];
        while (!feof(pipe)) {
            if (fgets(buf, 8, pipe) != NULL)
                path += buf;
        }
        std::pclose(pipe);
    #endif
    return ports;
}

std::string SerialSender::setSerialPath() {
    std::vector<std::string> ports = findAvailableSerialPorts();
    if (!ports.empty()) {
        return ports[0];
    }  else {
        return std::string("");
    }
}

void SerialSender::setSerialPath(std::string path) {
    serial_path = path;
}

std::string SerialSender::getSerialPath() {
    return serial_path;
}

bool SerialSender::connect(Baud b) {
    SerialSender::setBaud(b);
    return connect();
}

bool SerialSender::connect() {    
    if (sp.is_open()) {
        sp.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    boost::system::error_code ec;
    sp.open(serial_path, ec);
    if (!ec) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return sp.is_open() && !ec;
}

bool SerialSender::disconnect() {
    boost::system::error_code ec;
    sp.close(ec);
    return !ec && !sp.is_open();
}

bool SerialSender::sendByteVector(std::vector<uint8_t> bytes) {
    size_t size_exp = bytes.size() * sizeof(uint8_t);
    //size_t size_ret = boost::asio::write(sp, boost::asio::buffer(bytes, bytes.size()));
    size_t size_ret = sp.write_some(boost::asio::buffer(bytes, bytes.size()));
    return size_ret == size_exp;
}

size_t SerialSender::sendByteVectorAndRead(std::vector<uint8_t> bytes, 
                                              std::vector<uint8_t>& bytesRead, size_t maxBytesToRead) {
    if (!(SerialSender::sendByteVector(bytes)))
        return 0;
    bytesRead.clear();
    size_t nbytes=0;
    uint8_t b;
    while(1) {
        //boost::asio::read(sp, boost::asio::buffer(&b, 1));
        sp.read_some(boost::asio::buffer(&b, 1));
        switch ((char)b) {
        case '\r': 
            break;
        case '\n': 
            return nbytes;
        default: 
            bytesRead.push_back((uint8_t)b);
            nbytes++;
        }
    }
    return nbytes;
}

bool SerialSender::sendByteVectors(std::vector<std::vector<uint8_t>> byteVecs, std::vector<int> delays) {
    if (byteVecs.size() != delays.size()) return false;

    unsigned int complete = 0;
    for (unsigned int i=0; i<byteVecs.size(); i++) {
        complete += SerialSender::sendByteVector(byteVecs[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(delays[i]));
    }
    return complete == byteVecs.size();
}

void handler(
    const boost::system::error_code& error, // Result of operation.
    std::size_t bytes_transferred           // Number of bytes read.
    ) { }

size_t SerialSender::sendByteVectorsAndRead(std::vector<std::vector<uint8_t>> byteVecs,
                                            std::vector<std::vector<uint8_t>>& bytesRead,
                                            std::vector<int> delays) {
    size_t nbytes=0;
    size_t npackets = byteVecs.size();
    bytesRead.clear();
    std::vector<uint8_t> retBytes;
    for (unsigned int i=0; i<npackets; i++) {
        sendByteVector(byteVecs[i]);
        int max_pause = 50 + delays[i]; //ms greater than the specified delay
        int pause_ms = 5; //ms increment to wait in
        int pause_cnt_ms = 0; //ms running count of total time waiting
        int nBytesRead = 0;
        boost::system::error_code ec;
        char b;
        // under the current (15-01-2016) protocol, the arduino is expected to
        // send back 3 bytes per command regardless of success status
        while (nBytesRead < 3 && pause_cnt_ms < max_pause) {
            //clock() start call goes here
            bool finished = false;
            char b = 0;
            // non-blocking
            sp.async_read_some(boost::asio::buffer(&b, 1), handler);
            if (b) {
                switch (b) {
                case '\r':
                    break;
                case '\n':
                    bytesRead.push_back(retBytes);
                    finished = true;
                    break;
                default:
                    nBytesRead++;
                    retBytes.push_back(b);
                    //clock() stop call goes here
                    // calculate time elapsed and add to pause_cnt_ms
                }                
            }
            if (finished && pause_cnt_ms < delays[i]) {
                //sleep for the difference in time
                //break out of this loop
            }
        }

        //nbytes += SerialSender::sendByteVectorAndRead(byteVecs[i], retBytes, byteVecs[i].size());
        //bytesRead.push_back(retBytes);
        //std::this_thread::sleep_for(std::chrono::milliseconds(delays[i]));
    }
    return nbytes;
}

void SerialSender::handler(const boost::system::error_code& ec, std::size_t bytes_transferred) {}
