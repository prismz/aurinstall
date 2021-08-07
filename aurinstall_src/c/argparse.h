#ifndef ARGPARSE_H
#define ARGPARSE_H



char** parse_args(int argc, char** argv, int* nargc_l);
int determine_operation(char* arg);

#endif
