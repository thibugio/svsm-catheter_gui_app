/* file: srccpp/main.cpp */

#include <cstring>
#include <cstdio>
#include <stdio.h>

#include "common_utils.h"
#include "pc_utils.h"
#include "test_utils.h"
#include "SerialSender.h"

#ifdef WINDOWS 
    #include <direct.h>
    #define get_cwd _getcwd
#else
    #include <unistd.h>
    #define get_cwd getcwd
#endif

int main(int argc, char * argv[]) {
    char basepath[FILENAME_MAX];
    assertfail(get_cwd(basepath, FILENAME_MAX), "could not get current working directory.\n");

    SerialSender ss;
    
    std::string serialPath = ss.SerialSender::showSerialPath();
    printf("SerialSender detected device at %s.\n", serialPath.c_str());
    
    std::vector<std::vector<uint8_t>> packetbytes;
    std::vector<int> packetDelays;

    std::string path(basepath);
    if (argc > 1 && argv[1][0]=='/')
        path += std::string(argv[1]);
    else
        path += "/play/catheter_simple.play";
    printf("reading playfile from %s\n", path.c_str());
    
    getPacketBytesFromPlayfile(path.c_str(), packetbytes, packetDelays);

    ////print results////
    int npackets = packetbytes.size();
    for (int i=0; i<npackets; i++) {
        printf("packet %d (delay=%dms):\n", i, packetDelays[i]);
        print_bytes_as_bits(packetbytes[i].size(), packetbytes[i]);
    }

    assertfail(ss.connect(Baud::BR_9600), "failed to open connection to serial device.");
    printf("successfully opened connection to port %s\n", serialPath.c_str());

    // send the packets
    std::vector<std::vector<uint8_t>> bytesRead;
    int nbytes = ss.sendByteVectorsAndRead(packetbytes, bytesRead, packetDelays);
    printf("read a total of %d bytes.\n", nbytes);
    for (int i=0; i<npackets; i++) {
        printf("bytes returned for packet %d: \t", i);
        print_bytes_as_bits(bytesRead[i].size(), bytesRead[i]);
    }

    ss.disconnect();

    return 0;
}
