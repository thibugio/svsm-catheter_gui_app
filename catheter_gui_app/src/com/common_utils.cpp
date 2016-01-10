/* common_utils.cpp */

#include "common_utils.h"

using namespace std;

//assume direction has already been dealt with
unsigned int convert_current(double ma) {
    unsigned int res;
    if (ma < 0) ma = ma * -1;
    if (ma == 0) {
        res = DAC_RES - 1;
    } else if (ma < 10) {
        //non-linear region: y(ma) = 119.5 - 0.02973*x(res)
        res = (int)((119.5-ma)/0.02973);
        res = (res >= DAC_RES ? DAC_RES - 1 : res); 
    } else {
        //linear region: y(ma) = 843.750-00.2292*x(res)
        res = (int)((843.75-ma)/0.2292);
        if (res < 0) res = 0;
        else if (res >= DAC_RES) res = DAC_RES - 1;
    }
    return res;
}

double convert_current(int res, dir_t dir) {
    double ma;
    if (res == DAC_RES - 1) return 0;
    else if (res < 3750) {
        //linear region
        ma = 843.75 - 0.2292 * res;
    } else {
        //non-linear region
        ma = 119.5 - 0.2973 * res;
    }
    if (dir == DIR_NEG) ma = -ma;
    return ma;
}

void get_current_constants_by_channel(double* m, double* b, int chan, bool set1) {
    switch(chan) {
        case 1: //may wish to define constants...
            if (set1) { //for currents below 10 mA
                *m = -29.07;
                *b = 4058;
            } else {
                *m = -16.30;
                *b = 3896;
            }
            break; 
        case 2: 
            if (set1) { //for currents below 10 mA
                *m = -28.99;
                *b = 4096;
            } else {
                *m = -16.48;
                *b = 3939;
            }
            break;
        case 3: 
            if (set1) { //for currents below 10 mA
                *m = -36.76;
                *b = 4040;
            } else {
                *m = -15.78;
                *b = 3837;
            }
            break;
    }
}

unsigned int convert_current_by_channel(double ma, int chan) {    
    if (ma == 0) return DAC_RES - 1;    
    if (ma < 0) ma = ma * -1;
    unsigned int res;
    double m, b;
    bool set1 = (ma <= 10);
    get_current_constants_by_channel(&m, &b, chan, set1);
    res = (int)(m*ma + b);
    if (res < 0) res = 0;
    else if (res >= DAC_RES) res = DAC_RES - 1;
    return res;
}

double convert_current_by_channel(int res, dir_t dir, int chan) {
    double ma;
    double m, b;
    bool set1 = (res >= 4000);
    get_current_constants_by_channel(&m, &b, chan, set1);
    ma = (res - b)/m;
    if (dir == DIR_NEG) ma = -ma;
    return ma;
}

uint8_t compactCmdVal(bool poll, bool en, bool update, dir_t dir) {
    uint8_t cmd = 0;
    if (poll)   cmd |= (1 << POL_BIT);
    if (en)     cmd |= (1 << ENA_BIT);
    if (update) cmd |= (1 << UPD_BIT);
    if (dir==DIR_POS)    cmd |= (1 << DIR_BIT);
    return cmd;
}

/* return the state associated with a particular (4 bit) command value */
void expandCmdVal(uint8_t cmdVal, bool* poll, bool* en, bool* update, dir_t* dir) {
    *poll = (cmdVal >> POL_BIT) & 1;
    *en = (cmdVal >> ENA_BIT) & 1;
    *update = (cmdVal >> UPD_BIT) & 1;
    if ((cmdVal >> DIR_BIT) & 1)
        *dir = DIR_POS;
    else
        *dir = DIR_NEG;
}

void expandCatheterCmd(CatheterChannelCmd& cmd, bool* enable, bool* update, dir_t* dir) {
    *enable = (cmd.currentMA != 0);
    *update = true;
    *dir = (cmd.currentMA < 0 ? DIR_NEG : DIR_POS);
}

void compactCatheterCmd(CatheterChannelCmd& cmd, unsigned int* cmd4, unsigned int* data12) {
    *data12 = convert_current_by_channel(cmd.currentMA, cmd.channel);
    bool enable;
    bool update;
    dir_t dir;
    expandCatheterCmd(cmd, &enable, &update, &dir);
    *cmd4 = compactCmdVal(cmd.poll, enable, update, dir);
}

/* calculate 8-bit fletcher checksum using blocksize=4 */
uint8_t fletcher8(int len, uint8_t data[]) {
    uint8_t sum1 = 0, sum2 = 0;
    int i;
    for (i=0; i<len; i++) {     
        sum1 += (data[i] >> 4);   //first 4 bits
        sum2 += sum1;
     
        sum1 += (data[i] & 15);   //last 4 bits
        sum2 += sum1;
     
        sum1 %= 15;   //modulo 15
        sum2 %= 15;
    }
    sum1 %= 15;
    sum2 %= 15;

    return ((sum2 & 15)<<4) + (sum1 & 15);
}

std::vector<uint8_t> compactPreambleBytes(int pseqnum, int ncmds) {
    std::vector<uint8_t> bytes;
    int i;
    for (i=0; i<PRE_LEN; i++) {
        if (i==0) {
            bytes.push_back(PCK_OK << 7);          /* ok1 */
            bytes[i] |= (pseqnum & 7) << 4;    /* index3 */
            bytes[i] |= (ncmds & 15);           /* cmdCnt4 */
        }
    }
    return bytes;
}

std::vector<uint8_t> compactCommandBytes(CatheterChannelCmd& cmd) {
    std::vector<uint8_t> bytes;
    
    unsigned int data12;
    unsigned int cmd4;
    compactCatheterCmd(cmd, &cmd4, &data12);    
    
    int i;
    for (i=0; i<CMD_LEN; i++) {
        if (i==0) {
            bytes.push_back(cmd.channel << 4);          // bits 1-4
            bytes[i] |= (cmd4 & 15);          // bits 5-8
        } else if (i==1) {            
            bytes.push_back((data12 >> 4) & 255);   // bits 9-16  (first 8 bits of data)
        } else if (i==2) {
            bytes.push_back((data12 & 15) << 4);    // bits 17-20 (last 4 bits of data)
        }  // last 4 bits reserved
    }
    return bytes;
}

std::vector<uint8_t> compactCommandVectBytes(int ncmds, std::vector<CatheterChannelCmd>& cmdVect) {
    std::vector<uint8_t> bytes;
    int i;
    for (i=0; i<ncmds; i++) {
        std::vector<uint8_t> tempV = compactCommandBytes(cmdVect[i]);
        bytes.insert(bytes.end(), tempV.begin(), tempV.end()); 
    }
    return bytes;
}

std::vector<uint8_t> compactPostambleBytes(int pseqnum) {
    std::vector<uint8_t> bytes;
    int i;
    for (i=0; i<POST_LEN; i++) {
        if (i==0) {
            bytes.push_back(pseqnum << 5);  // index3
            bytes[i] |= PCK_OK & 1;     // packet OK bit appended to beginning and end of packet
        }
    }
    return bytes;
} 

std::vector<uint8_t> compactPacketBytes(std::vector<CatheterChannelCmd>& cmdVect, int pseqnum) {
    std::vector<uint8_t> bytes;
    int ncmds = cmdVect.size();
    
    std::vector<uint8_t> preV = compactPreambleBytes(pseqnum, ncmds);
    std::vector<uint8_t> cmdV = compactCommandVectBytes(ncmds, cmdVect);
    std::vector<uint8_t> postV = compactPostambleBytes(pseqnum);

    bytes.insert(bytes.end(), preV.begin(), preV.end());
    bytes.insert(bytes.end(), cmdV.begin(), cmdV.end());
    bytes.insert(bytes.end(), postV.begin(), postV.end());
    
    uint8_t chksum = fletcher8(PCK_LEN(ncmds)-1, bytes.data());
    bytes.insert(bytes.end(), 1, chksum);
    
    return bytes;
}

void getPacketBytes(std::vector<CatheterChannelCmd>& commandVect, std::vector<std::vector<uint8_t>>& pbytes, 
                    std::vector<int>& pdelays) {
    pbytes.clear();
    pdelays.clear();
    
    int ncmds = commandVect.size(); 
    int pseqnum = 1; 
    
    std::vector<CatheterChannelCmd> tempV;
    
    for(int i = 0; i < ncmds; i++) {
        tempV.push_back(commandVect[i]);
        if (commandVect[i].delayMS > 0 || i==(ncmds-1)) {
            pbytes.push_back(compactPacketBytes(tempV, pseqnum));
            pdelays.push_back(commandVect[i].delayMS);
            tempV.clear();
            pseqnum = (pseqnum & 7) + 1;
        }        
    }

}

CatheterChannelCmd resetCommand() {
    CatheterChannelCmd cmd;
    cmd.channel = 0; //global
    cmd.currentMA = 0;
    cmd.poll = false;
    cmd.delayMS = 0;
    return cmd;
}

std::vector<uint8_t> resetCommandBytes() {
    std::vector<CatheterChannelCmd> cmdVect;
    cmdVect.push_back(resetCommand());
    return compactPacketBytes(cmdVect, 1);
}