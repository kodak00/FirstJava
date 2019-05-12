/**
 * @file make_comp_zoom.h
 * @brief 낙뢰횟수 분포도 연/월/일별 파일 생성
 * @date 2012.06.25
 * @version 1.0
 * 프로그램 최초 배포버전
 * @author Weather Radar Center
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <zlib.h>

#include "nrutil.h"
#include "lgt_distribution_image.h"
#include "lgt_distribution_file.h"

struct _app_conf app_conf;

DATA_UNIT **DATA = NULL;

int main(int argc, char **argv)
{
	APP->APP_INIT(argc, argv);

	// 옵션 파싱
	opt_proc(argc, argv);

	// 시그널 설정 작업
	signal_proc();

	// 설정파일 로드
	conf_proc();

	// 초기화 작업
	init_proc();

	// 메인 작업
	main_proc();

	// 마무리 작업
	end_proc();

	// 종료코드
	exit(EXIT_SUCCESS);
}

/*
void opt_proc(int argc, char** argv)
{
	int opt = 0;

	//get option
	while ((opt = getopt(argc, argv, "hvV")) != -1)
	{
		switch (opt)
		{
			case 'h':
				usage(EXIT_SUCCESS);
				break;

			case 'v':
				APP->_.verbose++;
				break;

			case 'V':
				usage_version();
				exit(EXIT_SUCCESS);
				break;

			case ':':
				fprintf(stderr, "Missing argument\n\n");
				usage(EXIT_FAILURE);
				break;

			default:
				fprintf(stderr, "Try '-h' for more information.\n\n");
				usage(EXIT_FAILURE);
				break;
		}
	}

	// check to the count of arguments
	if (argc < 2)
	{
		usage(EXIT_FAILURE);
	}

	return;
}
*/

void signal_proc(void)
{
	struct sigaction ign_act = {}, child_act = {};

	ign_act.sa_handler = SIG_IGN;
	sigemptyset(&ign_act.sa_mask);
	ign_act.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &ign_act, NULL);

	child_act.sa_handler = read_childproc;
	sigemptyset(&ign_act.sa_mask);
	child_act.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	sigaction(SIGCHLD, &child_act, NULL);

	return;
}

//설정파일 로드
int conf_proc()
{
	char* conf_path = NULL;
	char* section = "LGT_DSTR";
	extern struct _app_conf app_conf;

	/* log path */
	char log_path[PATH_MAX] = {};

	strptime(APP->_.argv[1], "%Y%m%d%H%M", &app_conf.tm_reqTime);

//	conf_path = APP->_.app_conf;
	conf_path = CONFIG_FILE;

	strftime(log_path, PATH_MAX, APP->_.app_logpath, &app_conf.tm_reqTime);
	APP->_.app_logpath = strdup(log_path);
	DIRECTORY->MKDIR(DIRECTORY->DIRNAME(log_path), 0755);
	APP->fp_log = fopen(APP->_.app_logpath, "a+");
	if (APP->fp_log == NULL)
		APP->fp_log = stderr;

	if (INI->isVALUE(conf_path, section, "INPUT_LGT_KMA_PATH") == -1)
	{
		fprintf(APP->fp_log, "[FATAL] %s(%05d): %s(%s:%s)\n", APP->_.app_name, getpid(), conf_path, section, "INPUT_LGT_KMA_PATH");
		exit(EXIT_FAILURE);
	}
	else strftime(app_conf.input_lgt_kma_path, PATH_MAX, INI->getSTRING(conf_path, section, "INPUT_LGT_KMA_PATH"), &app_conf.tm_reqTime);
	
	if (INI->isVALUE(conf_path, section, "INPUT_LGT_KMA_OLD_PATH") == -1)
	{
		fprintf(APP->fp_log, "[FATAL] %s(%05d): %s(%s:%s)\n", APP->_.app_name, getpid(), conf_path, section, "INPUT_LGT_KMA_OLD_PATH");
		exit(EXIT_FAILURE);
	}
	else strftime(app_conf.input_lgt_kma_old_path, PATH_MAX, INI->getSTRING(conf_path, section, "INPUT_LGT_KMA_OLD_PATH"), &app_conf.tm_reqTime);
	

	if (INI->isVALUE(conf_path, section, "OUTPUT_LGT_KMA_YEAR_PATH") == -1)
	{
		fprintf(APP->fp_log, "[FATAL] %s(%05d): %s(%s:%s)\n", APP->_.app_name, getpid(), conf_path, section, "OUTPUT_LGT_KMA_YEAR_PATH");
		exit(EXIT_FAILURE);
	}
	else strftime(app_conf.output_lgt_kma_year_path,  PATH_MAX, INI->getSTRING(conf_path, section, "OUTPUT_LGT_KMA_YEAR_PATH"), &app_conf.tm_reqTime);

	if (INI->isVALUE(conf_path, section, "OUTPUT_LGT_KMA_MON_PATH") == -1)
	{
		fprintf(APP->fp_log, "[FATAL] %s(%05d): %s(%s:%s)\n", APP->_.app_name, getpid(), conf_path, section, "OUTPUT_LGT_KMA_MON_PATH");
		exit(EXIT_FAILURE);
	}
	else strftime(app_conf.output_lgt_kma_month_path, PATH_MAX, INI->getSTRING(conf_path, section, "OUTPUT_LGT_KMA_MON_PATH"), &app_conf.tm_reqTime);

	if (INI->isVALUE(conf_path, section, "OUTPUT_LGT_KMA_DAY_PATH") == -1)
	{
		fprintf(APP->fp_log, "[FATAL] %s(%05d): %s(%s:%s)\n", APP->_.app_name, getpid(), conf_path, section, "OUTPUT_LGT_KMA_DAY_PATH");
		exit(EXIT_FAILURE);
	}
	else strftime(app_conf.output_lgt_kma_day_path,   PATH_MAX, INI->getSTRING(conf_path, section, "OUTPUT_LGT_KMA_DAY_PATH"), &app_conf.tm_reqTime);


	return 0;
}

void init_proc()
{
	DATA = umatrix(0, MJ, 0, MI);

	DATA = data_init(DATA);

	return;
}

void main_proc()
{
	
	int dateMode = 1;	
	

		char fileTimeStr[BUFFER_SIZE];
		long filetime = 0;
		
		strftime(fileTimeStr, BUFFER_SIZE, "%Y%m%d%H%M", &app_conf.tm_reqTime );
		filetime = atol(fileTimeStr);
		
		fprintf(stderr , "set file time  file : %ld \n" , 	filetime);


		if(  filetime >= 201509150000 )
		{
			dateMode = 1;
			read_lgt_txt_data(DATA, app_conf.input_lgt_kma_path, app_conf.tm_reqTime , dateMode);
		}
		else
		{
			dateMode = 0;
			read_lgt_txt_data(DATA, app_conf.input_lgt_kma_old_path, app_conf.tm_reqTime , dateMode);
		}

	{
		{
			{
				//day 생성
				write_lgt_data(DATA, app_conf.output_lgt_kma_day_path);
			}

			{
				//month 누적
				write_lgt_data(DATA, app_conf.output_lgt_kma_month_path);
			}

			{
				//year 누적
				write_lgt_data(DATA, app_conf.output_lgt_kma_year_path);
			}
		}
	}



	return 0;
}


void end_proc()
{
	if(DATA != NULL)
		free_umatrix(DATA, 0, MJ, 0, MI);

	return;
}

void read_childproc(int sig)
{
	int status = 0;

	pid_t pid = 0;

	do
	{
		pid = waitpid(-1, &status, WNOHANG);
	}
	while (pid == -1 && errno == EINTR);

#ifdef DEBUG
	if (WIFEXITED(status))
	{
		fprintf(stderr, "[INFO] Exit Status from %d(return %d)", pid, WEXITSTATUS(status));
	}
	else
	{
		fprintf(stderr, "[WARNING] Exit Status from %d(return %d)", pid, WEXITSTATUS(status));
	}
#endif

	return;
}

void usage(int status)
{
    if (status != EXIT_SUCCESS)
    {
        fprintf(stderr, "Try `%s -h' for more information.\n", program_invocation_short_name);
    }
    else
    {
        fprintf(stdout, "Usage: %s [OPTION] start_time\n", program_invocation_short_name);
        fprintf(stdout, " start_time: yyyyMMddhhmi\n");
        fprintf(stdout, "  -v, --verbose  Verbose Mode - Can use multiple\n");
        fprintf(stdout, "  -V, --version  output version information and exit\n");
        fprintf(stdout, "  -h, --help     display this help and exit\n");
    }

    //fprintf(stdout, "\nReport \'%s\' bugs to %s\n", program_invocation_short_name, EMAIL);
    fprintf(stdout, "  Make by %s\n\n", APP_AUTHOR);

    exit(status);
}

void usage_version(void)
{
    fprintf(stdout, "==============================\n");
    fprintf(stdout, "Program Name: %s\n", program_invocation_short_name);
    fprintf(stdout, "Version: %s\n", APP_VERSION);
    fprintf(stdout, "Build: %s (%s)\n", __DATE__, __TIME__);
    fprintf(stdout, "==============================\n");

    exit(EXIT_SUCCESS);
}
