#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAX_PATH_LENGTH 1024
int option_i, option_l, option_R; 
int numPaths, newline;

int check_option(char **args){
    int i = 1;
    while(args[i] != NULL){
        if(args[i][0] == '-'){
            for(int j = 1; args[i][j] != '\0'; j++){
                if(args[i][j] == 'i'){
                    option_i = 1;
                }
                else if(args[i][j] == 'l'){
                    option_l = 1;
                }
                else if(args[i][j] == 'R'){
                    option_R = 1;
                }
                else{
                    fprintf(stderr, "Error: Unsupported Option\n");
                    exit(1);
                }
            }
        }
        else{
            return i;
        }
        i++;
    }
    return 0;
}

// read path into paths string array
void find_path(char **args, char **paths, int path_index){
    int i = path_index;
    while(args[i] != NULL){
        paths[numPaths] = calloc(sizeof(char), MAX_PATH_LENGTH);
        strcpy(paths[numPaths], args[i]);
        numPaths++;
    }
    if(numPaths == 0){
        paths[numPaths] = calloc(sizeof(char), 2);
        strcpy(paths[numPaths], ".");
        numPaths++;
    }
}

void list_derectory(){

}

int check_file(char *path){
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        fprintf(stderr, "Error: Nonexistent files or directories\n");
        exit(1);
    }
    return S_ISDIR(path_stat.st_mode);
}


int main(int argc, char *argv[]) {
    option_i = option_l = option_R = 0;
    numPaths = newline = 0;
    int print_index = 0;
    int print_long_format = 0;
    int recursive = 0;
    int path_index = 1;
    char *paths[argc];
    int n = 0;

    // Parse options
    path_index = check_option(argv);
    find_path(argv, paths, path_index);

    int i = 0;
    while (i < numPaths) {

        if(check_file(paths[i]) == 1){
            filePrint(paths[i]);
            i++;
            newline = 1;
            //formatter
            if(i == numPaths){
                printf("\n");
            }
            continue;
        }
        n = scandir(paths[i], &dirlist, 0, alphasort);
        if (n == -1) {
            if(paths[i][0] == '-'){
                printf("ERROR: Please input file options before file path\n");
                exit(0);
            }
            printf("ERROR: Unknown file/directory: %s\n", paths[i++]);
            continue;
        }

        // formatters
        if(newline == 1){
            printf("\n\n");
            newline = 0;
        }
        if(option_R == 0) {
            printf("%s:\n", paths[i]);
        }

        if(ioption == 0 && loption == 0 && Roption == 0){
            justPrint(dirlist, n);
        }else if(ioption == 1 && loption == 0 && Roption == 0){
            iPrint(dirlist, n, paths[i]);
        }else if(ioption == 0 && loption == 1 && Roption == 0){
            lPrint(dirlist, n, paths[i]);
        }else if(ioption == 0 && loption == 0 && Roption == 1){
            RPrint(dirlist, n, paths[i]);
        }else if(ioption == 1 && loption == 0 && Roption == 1){
            iRPrint(dirlist, n, paths[i]);
        }else if(ioption == 0 && loption == 1 && Roption == 1){
            lRPrint(dirlist, n, paths[i]);
        }else if(ioption == 1 && loption == 1 && Roption == 0){
            ilPrint(dirlist, n, paths[i]);
        }else if(ioption == 1 && loption == 1 && Roption == 1){
            ilRPrint(dirlist, n, paths[i]);
        }
        i++;
    }

    return 0;
}
