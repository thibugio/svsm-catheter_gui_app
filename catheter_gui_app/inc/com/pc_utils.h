/* 
 * pc_utils.h
 * defines methods, constants to be used only by the PC application
 */


#ifndef PC_UTILS_H
#define PC_UTILS_H

#include "common_utils.h"

/** \brief int loadPlayFile(const char*,std::vector<CatheterChannelCmd> &): Loads and parses a playfile.
 * The playfile is a recorded vector of computed commands with inter command delay. */
int loadPlayFile(const char * fname, std::vector<CatheterChannelCmd>& cmdVect);

bool writePlayFile(const char * fname, std::vector<CatheterChannelCmd>& cmdVect);

/** \brief void summarizeCmd(const CatheterChannelCmd& cmd): summarizes a command reply.
 * This should not be used with gui's */
void summarizeCmd(CatheterChannelCmd& cmd);

void print_string_as_bits(int len, std::string s);

void print_bytes_as_bits(int len, std::vector<uint8_t> bytes);

#endif
