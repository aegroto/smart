/*
 * SMART: string matching algorithms research tool.
 * Copyright (C) 2012  Simone Faro and Thierry Lecroq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 * contact the authors at: faro@dmi.unict.it, thierry.lecroq@univ-rouen.fr
 * download the tool at: http://www.dmi.unict.it/~faro/smart/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/*
 * This program compiles all c/cpp programs of string matching algorithms
 * It is called by makefile script, when compiling SMART
 */

int isCSourceFile(char* filename, int len) {
	return filename[len-2]=='.' && filename[len-1]=='c';
}

int isCPPSourceFile(char* filename, int len) {
	return filename[len-4]=='.' && filename[len-3]=='c' && filename[len-2]=='p' && filename[len-1]=='p';
}

void compileFile(char* filename, int len, int extensionOffset,
				  char extension[], char command[], char compiler[], char options[], char binary[], char destination[],
				  int *current, int *n_algo, int *testing_error, int *compiling_error) {
	filename[len-extensionOffset] = '\0';
	*current++;

	//compile
	sprintf(command, "%s%s%s%s%s", compiler, filename, extension, options, filename);
	printf("\tCompiling and testing %s%s", filename, extension);	
	for(int i = 0; i<15-len; i++) 
		printf(".");
	printf("(%.3d/%.3d) [000%%]", *current, *n_algo);
	fflush(stdout);

	printf("\b\b\b\b\b\b[%.3d%%]", (*current * 100) / (*n_algo) ); fflush(stdout);

	FILE *stream = freopen ("warning","w",stderr);
	if( system(command)==1 ) printf("[ERROR]\n");
	else {
		sprintf(binary, "%s%s", destination, filename);
		FILE *fp = fopen(binary,"r");
		if(fp) {
			fclose(fp);
			fflush(stdout);

			sprintf(command,"./test %s -nv",filename);
			fflush(stdout);
			if(system(command)) {
				(*testing_error)++;
				printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b....[TEST FAILED]");
				printf("\n");
			}
			else {
				printf("\b\b\b\b\b\b..[OK]");
				for(int j=0; j<63; j++) printf("\b");
			}
		}
		else {
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b..[COMPLING ERROR]\n");
			(*compiling_error)++;
		}
		fflush(stdout);
	}
	fclose(stream);
}

void loadAlgos() {
	DIR *d = opendir("./source/bin");
	struct dirent *dir;
	size_t i = 0;

    if(d) {
		FILE *fp = fopen("source/algorithms.h", "w");
		
		while ((dir = readdir(d)) != NULL) {
			if(dir->d_name[0] != '.') {
				fprintf(fp,"#0 #%s \n", dir->d_name);
				++i;
			}
		}

		fclose(fp);
	}
}

int main(int argc, const char *argv[]) {
	char cExtension[100] = ".c";
	char cppExtension[100] = ".cpp";
	char gcc[100] = "gcc source/algos/";
	char gpp[100] = "g++ -fpermissive source/algos/";
	char options[100] = " -O3 -msse4 -lm -o source/bin/"; 
	char destination[100] = "source/bin/";

	char filename[100], command[100], binary[100];
	int i,j;

	// int doTest = 1;
	
	DIR *d;
	// FILE *stream;
	struct dirent *dir;

	//delete previous compiled files
	d = opendir("./source/bin");
	if(d) {
	  	while ((dir = readdir(d)) != NULL) {
			strcpy(filename, dir->d_name);
			sprintf(command,"./source/bin/%s",filename);
			remove(command);
		}
	}
	closedir(d);
	
    //counting and compiling c files
    
    int current = 0;
    int compiling_error = 0, testing_error= 0;

    d = opendir("./source/algos");
    int n_algo = 0;
    if(d) {
        while ((dir = readdir(d)) != NULL) {
            strcpy(filename, dir->d_name);
            int len = strlen(filename);

            if(isCSourceFile(filename, len)) {
                n_algo ++;
				compileFile(filename, len, 2, cExtension, command, gcc, options, binary, destination, &current, &n_algo, &testing_error, &compiling_error);
            } else if (isCPPSourceFile(filename, len)) {
				n_algo ++;
				compileFile(filename, len, 4, cppExtension, command, gpp, options, binary, destination, &current, &n_algo, &testing_error, &compiling_error);
			}
        }
    }
    closedir(d);
	
    //compile c files
	/*d = opendir("./source/algos");
	if(d) {
	  	while ((dir = readdir(d)) != NULL) {
			strcpy(filename, dir->d_name);
			int len = strlen(filename);
			if(isCSourceFile(filename, len)) {
				compileCFile(filename, len);
			}
		}
	}

	closedir(d);*/

    for(i=0; i<33; i++) printf("\b");

    printf("\tAll algorithms (%d) have been compiled and tested.......\n", n_algo);
    printf("\tCompiling errors .................................[%.3d]\n", compiling_error);
    printf("\tTesting errors ...................................[%.3d]\n\n", testing_error);

	loadAlgos();
}

