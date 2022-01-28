/*
 * Dante Su 2010.05.11
 *
 * This is designed to determine the date of latest updated CVS files
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/io.h>

#ifndef bool
#define bool	int
#endif

#ifndef true
#define true	1
#endif

#ifndef false
#define false	0
#endif

#define VERSTR		"#define BOOTCODE2_DATE	\"%s\"\n"

bool config_verbose=false;
bool config_output_short=false;

static time_t	last_update = 0;

int isFaraday(char *path)
{
	FILE *fp;
	char line[2048];
	
	sprintf(line, "%s/CVS/Root", path);
	
	fp = fopen(line, "r");
	if (fp == NULL)
		return 0;
		
	if (fgets(line, sizeof(line), fp) == NULL)
	{
		pclose(fp);
		return 0;
	}
	
	fclose(fp);
	
	if (strstr(line, "/usr/local/cvsroot/pbs") == NULL)
		return 0;
	
	return 1;
}

char *fgets2(char *s, int size, FILE *stream)
{
	size_t i;
	
	if (fgets(s, size, stream) == NULL)
		return NULL;
		
	for (i = strlen(s) - 1; i >= 0; --i)
	{
		if (s[i] == '\r' || s[i] == '\n')
			s[i] = 0;
		else
			break;
	}
	
	return s;
}

int load_entry(char *filename)
{
	FILE *fp;
	char line[2048];
	char *tok;
	struct tm tm;
	time_t t, t0;
	char *dir, *p;


	sprintf(line, "%s/Entries", filename);

	

	
//	printf("File: [%s]\n", dir);
	
	fp = fopen(line, "r");
	if (fp == NULL)
		return -1;
	
	while(fgets2(line, sizeof(line), fp))
	{
		if (line[0] != '/')
			continue;
		//printf("E: [%s]\n", line);
		tok = strtok(line, "/");	/* filename */
		tok = strtok(NULL, "/");	/* version */
		tok = strtok(NULL, "/");	/* timestamp */
		//printf("T: [%s]\n", tok);
		if (!strptime(tok, "%a %b %d %X %Y", &tm))
			continue;
		//printf("t: %d.%d.%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		
		t = mktime(&tm);
		if (last_update < t){
			last_update = t;
			if(config_verbose){
				// remove CVS in the path
				dir=strdup(filename);
				p=strstr(dir,"CVS");
				*(p-1)=0;
				fprintf(stderr,"File: [%s/%s] ", dir,line);
				fprintf(stderr,"(UTC) %s", ctime(&last_update));
				free(dir);
			}
		}
	}
	
	fclose(fp);
	
	return 0;
}

void usage(void)
{
	printf(	"\n"
			"usage: getver [OPTION]... [PATH1][PATH1]...\n"
			"\n"
			"[OPTION]\n"
			"    -s: short output\n"
			"    -v: verbose mode\n"
			"\n"
			);
	exit(-1);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char *tok;
	char *path = NULL;
	char line[2048];
	struct tm *tm;
	int c,index;
	
	if (argc < 2)
		usage();
		
	while ((c = getopt (argc, argv, "svhH")) != -1)
	{
		switch (c)
		{
		case 's':
			config_output_short=true;
			fprintf(stderr,"short output !\n");
			break;
		case 'v':
			config_verbose=true;
			fprintf(stderr,"verbose mode !\n");
			break;
		case 'h':
		case 'H':
		default:
			usage();
		}
	}
	if ( optind < argc ){ 
		for (index = optind; index < argc; index++){
			if(config_verbose){
				fprintf(stderr,"process [%s]\n",argv[index]);
			}
			path=argv[index];

			// 1. Is Faraday CVS ?
			if (!isFaraday(path))
			{
				printf(VERSTR, "beta");
				return 0;
			}

			// 2. find CVS
			sprintf(line, "find '%s/' -name CVS", path);
			fp = popen(line, "r");
			if (fp == NULL)
			{
				printf(VERSTR, "beta");
				return 0;
			}

			// 3. load CVS/Entries
			while(fgets2(line, sizeof(line), fp))
			{
				load_entry(line);
			}

			pclose(fp);
		}
	}else{ //Not assign file
		usage();
	}
	
	
	tm = localtime(&last_update);

	if(config_verbose){
		fprintf(stderr,"\n\n UTC: %s\n", ctime(&last_update));
	}

	if(config_output_short){
		printf("%02d%02d%02d", tm->tm_year + 1900-2000, tm->tm_mon + 1, tm->tm_mday);
	}else{
#if 1
		sprintf(line, "%d.%02d%02d",
				tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
#else
		sprintf(line, "%d.%02d%02d%02d%02d",
				tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
				tm->tm_hour, tm->tm_min);
#endif
		printf(VERSTR, line);
	}
	
	return 0;
}
