#include "pc_utils.h"

using namespace std;

/* parse a playfile into a command vector */
int loadPlayFile(const char* fileIn, std::vector<CatheterChannelCmd>& outputCmdsVect) {

    ifstream inFile(fileIn, ifstream::in);
    if(inFile.bad()) return -1;
	
    outputCmdsVect.clear();
    int npackets = 0;

    string line;
    int linenum = 0;
    while (getline (inFile, line)) {
		
        string item;

        size_t posOcto1  = line.find ("#"); // line comment
        size_t posComma1 = line.find (","); // after channel, before current
        size_t posComma2 = line.find (",", posComma1 + 1); // after current (MA), before delay (MS)

        //verify the line's validity.
        if((posComma1 < posComma2) && (posComma2 < posOcto1)) {  //line is ok.

            istringstream linestream(line);
        
            CatheterChannelCmd cmd;
       
            /* parse channel */
            getline (linestream, item, ','); 
            int channelIn = atoi(item.c_str());
            if(!(channelIn >= 0 && channelIn <= NCHANNELS)) continue;   // bad channel; skip line
            cmd.channel = channelIn;

            /* parse current data, given in MA */
            getline (linestream, item, ',');
            cmd.currentMA = atof(item.c_str());

            cmd.poll = false;

            /* parse delay */
            getline (linestream, item, '#'); //works even if there is no #.
            int waitTime = atoi(item.c_str());
            if(!(waitTime >= 0)) continue;  // bad delay; skip line
            cmd.delayMS = waitTime;
            
            outputCmdsVect.push_back(cmd);
            
            if (waitTime) {
                npackets++;
            }
            linenum++;
        }
    }
    inFile.close();
    return npackets;
}

bool writePlayFile(const char * fname, std::vector<CatheterChannelCmd>& cmdVect) {
    ofstream outFile(fname, ofstream::out);
    if (outFile.bad())
        return false;
    CatheterChannelCmd cmd;
    for (int i = 0; i < cmdVect.size(); i++) {
        cmd = cmdVect[i];
        outFile << cmd.channel << ", " << cmd.currentMA << ", " << cmd.delayMS << std::endl;
    }
    outFile.close();
    return true;
}

void summarizeCmd(CatheterChannelCmd& cmd) {
    bool enable;
    bool update;
    dir_t dir;
    expandCatheterCmd(cmd, &enable, &update, &dir);
    printf("channel: %d\n",cmd.channel);
    printf("poll: %d\n", cmd.poll);
    printf("enable: %d\n", enable);
    printf("update: %d\n", update);
    printf("dir: %d\n", dir);
    printf("current (MA): %3.3f\n", cmd.currentMA);
}

void print_string_as_bits(int len, std::string bytes) {
    int i,j;
    for (i=0; i<len; i++) {
        for (j=7; j>=0; j--) {
            if ((bytes[i] & (1<<j)))  printf("1");
            else  printf("0");
        }
        printf(" ");
    }
    printf("\n");
}

void print_bytes_as_bits(int len, std::vector<uint8_t> bytes) {
    int i,j;
    for (i=0; i<len; i++) {
        for (j=7; j>=0; j--) {
            if ((bytes[i] & (1<<j)))  printf("1");
            else  printf("0");
        }
        printf(" ");
    }
    printf("\n");
}


