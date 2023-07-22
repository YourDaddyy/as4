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
int option_i, option_l, option_R, no_option; 
int numPaths, newline;

//read options and return the index of the first path
int check_option(char **args){
    int i = 1;
    while(args[i] != NULL){
        if(args[i][0] == '-'){
            for(int j = 1; args[i][j] != '\0'; j++){
                switch (args[i][j]) {
                    case 'i':
                        option_i = 1;
                        break;
                    case 'l':
                        option_l = 1;
                        break;
                    case 'R':
                        option_R = 1;
                        break;
                    default:
                        fprintf(stderr, "Error: Unsupported Option\n");
                        exit(1);
                }
            }
        }
        else{
            //finish parsing options, return the index of the first path
            return i;
        }
        i++;
    }
    return 0;
}

// read path into paths string array
void find_path(char **args, char **paths, int path_index){
    int i = path_index;
    if(numPaths == 0){
        paths[numPaths] = calloc(sizeof(char), 2);
        strcpy(paths[numPaths], ".");
        numPaths++;
    }else if(numPaths >= 1){
        while(args[i] != NULL){
            paths[numPaths] = calloc(sizeof(char), MAX_PATH_LENGTH);
            strcpy(paths[numPaths], args[i]);
            numPaths++;
            i++;
        }
    }
}

int check_file(char *path){
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        fprintf(stderr, "Error: Nonexistent files or directories\n");
        exit(1);
    }
    return S_ISREG(path_stat.st_mode);
}

// Check permissions for a directory or not
char *getpermission(mode_t mode){
    char *perm = calloc(sizeof(char), 11); // Increase the size to handle the null terminator
    snprintf(perm, 11, "%c%c%c%c%c%c%c%c%c%c",
             (S_ISDIR(mode) ? 'd' : '-'),
             (mode & S_IRUSR ? 'r' : '-'),
             (mode & S_IWUSR ? 'w' : '-'),
             (mode & S_IXUSR ? 'x' : '-'),
             (mode & S_IRGRP ? 'r' : '-'),
             (mode & S_IWGRP ? 'w' : '-'),
             (mode & S_IXGRP ? 'x' : '-'),
             (mode & S_IROTH ? 'r' : '-'),
             (mode & S_IWOTH ? 'w' : '-'),
             (mode & S_IXOTH ? 'x' : '-'));
    return perm;
}

void print_option(struct dirent **dirlist, int size, char *dir){
    int i = 0,listc = 0;
    char *list_dir[100];
    while(i<size){
        if (dirlist[i]->d_name[0] != '.') {
            struct stat sb;
            char *path = calloc(sizeof(char), 1024);
            strcat(path, dir);
            strcat(path, "/");
            strcat(path, dirlist[i]->d_name);
            if (stat(path, &sb) == -1) {
                perror("stat");
                exit(0);
            }

            char *permission = getpermission(sb.st_mode);
            struct passwd *pw;
            struct group *grp = getgrgid(sb.st_gid);
            struct tm *tim;
            time_t tme;
            char time[256];
            if(option_l){
                // get file owner
                pw = getpwuid(sb.st_uid);
                if (pw == NULL) {
                    perror("ERROR: cannot access pwuid\n");
                }
                // get file group
                if (grp == NULL) {
                    perror("ERROR: cannot access grgid ");
                }
                // get last modified time info
                tme = sb.st_mtime;
                tim = localtime(&tme);
                
                strftime(time, sizeof(time), "%b %d %Y %H:%M", tim);
            }

            if (dirlist[i]->d_type == DT_DIR) {
                if(option_R){
                    list_dir[listc] = calloc(sizeof(char), 1024);
                    strcat(list_dir[listc], dir);
                    strcat(list_dir[listc], "/");
                    strcat(list_dir[listc++], dirlist[i]->d_name);
                }
                //changing text clr to cyan for directories to emulate ls behaviour
                if(option_i){
                    printf("  %ld ", sb.st_ino);
                }
                if(option_l){
                    printf("%s %li %s %s %5ld %s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                }
                printf("\033[1;34m");
                printf("%s  ", dirlist[i]->d_name);
                printf("\033[0m");
                if(!no_option){
                    printf("\n");
                }
            } else {
                if(option_i){
                    printf("  %ld ", sb.st_ino);
                }
                if(option_l){
                    printf("%s %li %s %s %5ld %s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size,
                           time);
                }
                printf("%s  ",dirlist[i]->d_name);
                if(!no_option){
                    printf("\n");
                }
            }
            free(permission);
            free(path);
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n");
    if(option_R){
        // Dirrecursor(list_dir,listc);
        // recursive_print(list_dir,listc);
        for (i = 0; i < listc; i++) {
            int n = scandir(list_dir[i], &dirlist, 0, alphasort);
            if (n == -1) {
                perror("scandir");
                continue;
            }
            printf("%s:\n",list_dir[i]);
            print_option(dirlist, n, list_dir[i]);
        }
    }
}

void print_file(char *file) {
    struct stat sb;
    if (stat(file, &sb) == -1) {
        perror("ERROR: stat\n");
        exit(0);
    }

    int flag = 0;
    if (option_i) flag |= 1;
    if (option_l) flag |= 2;
    if (option_R) flag |= 4;

    if (flag == 0 || flag == 4) {
        printf("%s ", file);
    } else if (flag == 1 || flag == 5) {
        printf("  %ld %s\n", sb.st_ino, file);
    } else if (flag == 2 || flag == 6) {
        char *permission = getpermission(sb.st_mode);
        struct passwd *pw = getpwuid(sb.st_uid);
        struct group *grp = getgrgid(sb.st_gid);
        struct tm *tim;
        time_t tme = sb.st_mtime;
        tim = localtime(&tme);
        char time[256];
        strftime(time, sizeof(time), "%b %d %Y %H:%M", tim);
        printf("%s %li %s %s %5ld %s %s\n", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, file);
        free(permission);
    } else if (flag == 3 || flag == 7) {
        char *permission = getpermission(sb.st_mode);
        struct passwd *pw = getpwuid(sb.st_uid);
        struct group *grp = getgrgid(sb.st_gid);
        struct tm *tim;
        time_t tme = sb.st_mtime;
        tim = localtime(&tme);
        char time[256];
        strftime(time, sizeof(time), "%b %d %Y %H:%M", tim);
        printf("%ld %s %li %s %s %5ld %s %s\n", sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, file);
        free(permission);
    }
}

int main(int argc, char *argv[]) {
    
    struct dirent **dirlist;
    int n = 0;

    option_i = option_l = option_R = no_option = 0;
    numPaths = newline = 0;

    char *paths[argc];
    
    // Parse options
    int path_index = check_option(argv);
    find_path(argv, paths, path_index);

    int i = 0;
    while (i < numPaths) {
        //if it is a file, print it
        if(check_file(paths[i]) == 1){
            print_file(paths[i]);
            i++;
            newline = 1;
            //formatter
            if(i == numPaths){
                printf("\n");
            }
            continue;
        }
        //if it is a directory, print its contents
        n = scandir(paths[i], &dirlist, 0, alphasort);
        if (n == -1) {
            if(paths[i][0] == '-'){
                printf("ERROR: File options should enter before path\n");
                exit(0);
            }
            printf("ERROR: Invalid file/directory: %s\n", paths[i++]);
            continue;
        }

        // formatters
        if(newline == 1){
            printf("\n");
            newline = 0;
        }

        if(option_i == 0 && option_l == 0 && option_R == 0){
            no_option = 1;
        }
        print_option(dirlist, n, paths[i]);
        i++;
    }

    return 0;
}
