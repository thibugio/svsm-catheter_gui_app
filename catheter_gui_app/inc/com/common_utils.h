/* common_utils.h
*/

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>


/* number of channels being used */
#define NCHANNELS 6 

/* number of bytes in a channel command, preamble, postamble, and packet checksum */
#define CMD_LEN 3  
#define PRE_LEN 1
#define POST_LEN 1
#define PCK_CHK_LEN 1

/* error codes for arduino to send back to PC */
#define PRE_ERR 1
#define POST_ERR 2
#define PCK_CHK_ERR 4

/* macro to compute the size of a packet */
#define PCK_LEN(N_CMDS) PRE_LEN + (CMD_LEN*N_CMDS) + POST_LEN + PCK_CHK_LEN
#define NCMDS(PCKLEN) (PCKLEN-PRE_LEN-POST_LEN-PCK_CHK_LEN)/CMD_LEN

#define PCK_OK 1
#define DAC_RES 4096

/* command value bits */
#define DIR_BIT 0
#define UPD_BIT 1
#define ENA_BIT 2
#define POL_BIT 3

enum dir_t {DIR_POS=1, DIR_NEG=0};

struct CatheterChannelCmd {
    unsigned int channel;
    double currentMA;
    bool poll;
    unsigned int delayMS;
};

struct CatheterPacket {
    unsigned int pseqnum;
    std::vector<CatheterChannelCmd> cmds;
};

/** \brief uint8_t compactCmdVal(int poll, int en, int update, dir_t dir): return the command number (0-15) associated with the specified state */
uint8_t compactCmdVal(bool poll, bool en, bool update, dir_t dir);

/** \brief void expandCmdVal(uint8_t cmdVal, int * poll, int * en, int * update, dir_t * dir): return the state associated with the specified commnd number (0-15) */
void expandCmdVal(uint8_t cmdVal, bool* poll, bool* en, bool* update, dir_t * dir);

/** \brief void compactCatheterCmd(CatheterChannelCmd& cmd, unsigned int* cmd4, unsigned int* data12): extract the 12-bit DAC data and 4-bit command value from a CatheterChannelCmd */
void compactCatheterCmd(CatheterChannelCmd& cmd, unsigned int* cmd4, unsigned int* data12);

/** \brief void expandCatheterCmd(CatheterChannelCmd& cmd, bool* enable, bool* update, dir_t* dir): extract the semantic command bit values from a CatheterChannelCmd */
void expandCatheterCmd(CatheterChannelCmd& cmd, bool* enable, bool* update, dir_t* dir);

/** \brief uint8_t fletcher8(int len, uint8_t bytes[]): compute the fletcher checksum of an array of bytes of length 'len' using blocksize=8. ('len' <= the actual length of the array, since we may not want to include all elements of the array in the computation.) */
uint8_t fletcher8(int len, uint8_t bytes[]);

/** \brief std::vector<uint8_t> compactPacketBytes(std::vector<catheterChannelCmd>&,int): compacts a packet into a string to be sent over serial */
std::vector<uint8_t> compactPacketBytes(std::vector<CatheterChannelCmd>&,int);

/** \brief std::vector<uint8_t> compactPreambleBytes(int pseqnum,int cmdCount): uses the command index and number of commands to generate the preamble bit(s). */
std::vector<uint8_t> compactPreambleBytes(int pseqnum,int cmdCount);

/** \brief std::vector<uint8_t> compactPostamble(int pseqnum): uses the command index to generate the postamble bit(s). */
std::vector<uint8_t> compactPostambleBytes(int pseqnum);

/** \brief std::vector<uint8_t> compactCommandVectBytes(const std::vector<catheterChannelCmd>&): generalize version of generating a command for multiple channels simultaneously. */
std::vector<uint8_t> compactCommandVectBytes(const std::vector<CatheterChannelCmd>&);

/** \brief std::vector<uint8_t> compactCommandBytes(const catheterChannelCmd): compacts a single arduino command into a 3 byte packet. */
std::vector<uint8_t> compactCommandBytes(const CatheterChannelCmd&);

void getPacketBytes(std::vector<CatheterChannelCmd>& commandVect, std::vector<std::vector<uint8_t>>& pbytes, 
                    std::vector<int>& pdelays);

CatheterChannelCmd resetCommand();
std::vector<uint8_t> resetCommandBytes();

/** current conversion functions: milliamps<-->dac resolution */
unsigned int convert_current(double ma);
double convert_current(int res, dir_t dir);

void get_current_constants_by_channel(double* m, double* y, int chan, bool set1);
unsigned int convert_current_by_channel(double ma, int chan);
double convert_current_by_channel(int res, dir_t dir, int chan);

#endif
