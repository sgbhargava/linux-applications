/*
 * mysystemmonitor.c
 *
 *  Created on: Aug 23, 2015
 *      Author: bhargav
 */
#include <stdio.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ncurses.h>
#include <assert.h>
#define Delay 10000
#define cast
static char cpu[5];

static float get_cpu(void* file, int cpu_no);
static int curse(float percentage, int y);

int main()
{
	FILE *fp;
	float cpu = 0.0;
	char line[1024];
	int cpu_number =0;
	int i =0;
#ifdef cast

/*Initialize ncurses window*/
	initscr();
	noecho();
	curs_set(FALSE);
#endif

	start_color();

/*declaring color pairs*/
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);

/* kernel/system statistics file*/
	fp = fopen("/proc/stat","r");
		if(fp == NULL)
				{
				fprintf(stdout, "%s\n",strerror(errno));
				exit(1);
				}

/* Read the entire file and to get the number of cpu cores*/
	while(NULL != fgets(line,sizeof(line), fp))
	{

		char *ex;
		ex = strtok(line, " ");
		if(strncmp(ex, "cpu",3) == 0)
			cpu_number++;

	}

	if(fclose(fp) == -1) assert(FALSE);

	while(1)
	{
		clear();
		fp = fopen("/proc/stat","r");
		if(fp == NULL)
				{
				fprintf(stdout, "%s\n",strerror(errno));
				exit(1);
				}

/*repeat this loop for every cpu core*/
		for(i = 0; i< cpu_number; i++)
		{
			cpu = get_cpu(fp, i);	//This is the function for getting the cpu info
			curse(cpu, i);			//This is the function for printing the cpu info
		}

		refresh();
		sleep(1);
		if(fclose(fp) == -1) assert(FALSE);
	}
	endwin();
	exit(0);
}

/* This function is to get the cpu usage*/
static float get_cpu(void *file, int cpu_no)
{
	FILE *fs = (FILE *)file;
	static long oldBusyTotal[5] ={0};
	static long oldIdle[5] = {0};
	long Total =0;
	long busyTotal =0;
	long actualIdle = 0;
	long actualBusyTotal = 0;
	long diff = 0;
	double percent = 0.0;
	long user, idle, iowait, niceValue, systemValue;
	char line[1024];

	if (cpu_no != 0)
/*This is to make sure fscanf reads next line eveytime its run*/
		fgets(line,sizeof(line), fs );

	fscanf(fs,"%s %ld %ld %ld %ld %ld" ,cpu, &user, &niceValue, &systemValue, &idle, &iowait);

/*Calculate cpu info: CPU_Percentage=((Total-PrevTotal)-(Idle-PrevIdle))/(Total-PrevTotal)*/
	busyTotal = user+systemValue+iowait;
	actualBusyTotal = busyTotal - oldBusyTotal[cpu_no];
	actualIdle = idle - oldIdle[cpu_no];
	Total = actualBusyTotal+actualIdle;
	diff =Total - actualIdle;
	percent = (double)diff /(double)Total;
	percent *= 100;
	oldIdle[cpu_no] = idle;
	oldBusyTotal[cpu_no] = busyTotal;
	return percent;
}

/*This function is used toprint cpu info*/
static int curse(float percentage, int y)
{	int max_x = 0;
	int max_y = 0;
	int percentageInt = (int)percentage;
	float scalefactor = 0.0;
	int i = 0;

/*getmaxyx() gets the max values of x and y co-ordinates in the window. 
We can use it to re-scale the output based on the window size*/
	getmaxyx(stdscr, max_y, max_x);
	scalefactor = ((float)max_x - 10.0)/100.0;

/*If the windows is too big, we make sure the data is not scattered*/
	if (scalefactor > 0.5)
			scalefactor = 0.5;

/*printing out data*/
	attron(COLOR_PAIR(2));
	mvprintw(y, 0, "%s", cpu);
	mvprintw(y,4,":[");
	attroff(COLOR_PAIR(2));

/*printing out the "|" character for cpu usage*/
	for(i =6; i < ((scalefactor*percentageInt)+6); i++)
	{
		attron(COLOR_PAIR(1));
		mvprintw(y, i, "|");
		attroff(COLOR_PAIR(1));
	}

/*printing out the percentage value*/
		attron(COLOR_PAIR(2));
		mvprintw(y, ((100*scalefactor)+4), "]%.02f%%",(percentage));
		attroff(COLOR_PAIR(2));
		usleep(Delay);
		return(0);
}


