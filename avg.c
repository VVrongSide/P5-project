#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]) {
	int index = 1;
	double oldtime = 0;
	double time;
	double num;
	double numbuff = 0;
	FILE * fp;
	int precision = (argc == 2) ? atoi(argv[1]) : 5;

	//fp = fopen ("ns3/Wdsr-plot.dat", "r");
	fp = stdin;
	while (!feof(fp)){
		if (2 == fscanf(fp, "%lf %lf", &time, &num)){
			time = floorf(time * precision) / precision;
			if (! (time == oldtime)){
				numbuff /= index;
				if(!(oldtime==0.0))
					printf("%0.2f %f\n", oldtime, numbuff );
				numbuff = 0.0;
				index = 0;
			}
			index++;
			numbuff += num;
			oldtime = time;
		}
	}
	fclose(fp);
	return(0);
}
