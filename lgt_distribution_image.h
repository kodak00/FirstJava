#ifndef LGT_DSTRIBUTION_FILE_H_
#define LGT_DSTRIBUTION_FILE_H_

#define MI 1440
#define MJ 1440

#define BUFFER_SIZE 1024

//0512
typedef struct _tUNIT
{
	unsigned int	ground;
	unsigned int 	cloud;
	double			strength;
}DATA_UNIT;

DATA_UNIT** read_lgt_txt_data(DATA_UNIT **DATA, char* readFile, struct tm curTime, int dateMode);
int write_lgt_data(DATA_UNIT **DATA, char* writeFile);
DATA_UNIT **umatrix(int nrl,int nrh,int ncl,int nch);
void free_umatrix(DATA_UNIT **m,int nrl,int nrh,int ncl,int nch);
DATA_UNIT** data_init(DATA_UNIT** DATA);
struct tm incMin(struct tm tm_ptr,int addMin);

//int** read_lgt_data(int **DATA, char* readFile);
//int** sum_lgt_data(int **DATA, char* writeFile);
//int write_lgt_data(int **DATA, char* writeFile);

#endif /* MAKE_LGT_DSTRIBUTION_FILE_H_ */
