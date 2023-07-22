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

//checking file permission and filling the perm
// Check permissions for a directory or not
char *getpermission(mode_t mode){
    char *perm = calloc(sizeof(char),10);
    if(S_ISDIR(mode)){
        strcat(perm,"d");
    }else{
        strcat(perm,"-");
    }
    // Check permissions for Owner
    if(mode & S_IRUSR){
        strcat(perm,"r");
    }else{
        strcat(perm,"-");
    }
    if(mode & S_IWUSR){
        strcat(perm,"w");
    }else{
        strcat(perm,"-");
    }
    if(mode & S_IXUSR){
        strcat(perm,"x");
    }else{
        strcat(perm,"-");
    }
    // Check permissions for Group
    if(mode & S_IRGRP){
        strcat(perm,"r");
    }else{
        strcat(perm,"-");
    }
    if(mode & S_IWGRP){
        strcat(perm,"w");
    }else{
        strcat(perm,"-");
    }
    if(mode & S_IXGRP){
        strcat(perm,"x");
    }else{
        strcat(perm,"-");
    }
    // Check permissions for other
    if(mode & S_IROTH){
        strcat(perm,"r");
    }else{
        strcat(perm,"-");
    }
    if(mode & S_IWOTH){
        strcat(perm,"w");
    }else{
        strcat(perm,"-");
    }
    if(mode & S_IXOTH){
        strcat(perm,"x");
    }else{
        strcat(perm,"-");
    }
    return perm;
}

//TODO simplify recusion looks complex
void Dirrecursor(char **list, int size) {
    int i = 0,n,listc = 0;
    struct dirent **dirlist;
    char *list_dir[100];
    if(size == 0){
        return;
    }
    while(i < size){
        printf("%s:\n",list[i]);
        n=scandir(list[i],&dirlist,0,alphasort);
        if (n == -1){
            return;
        }
        int j =0;
        if(ioption == 0 && loption == 0) {
            while (j < n) {
                if (dirlist[j]->d_name[0] != '.') {
                    if (dirlist[j]->d_type == DT_DIR) {
                        list_dir[listc] = calloc(sizeof(char), 1024);
                        strcat(list_dir[listc], list[i]);
                        strcat(list_dir[listc], "/");
                        strcat(list_dir[listc++], dirlist[j]->d_name);
                        //changing text clr to cyan for directories to emulate ls behaviour
                        printf("\033[1;34m");
                        printf("%s ", dirlist[j]->d_name);
                        printf("\033[1;37m");
                    } else {
                        printf("%s ", dirlist[j]->d_name);
                    }
                }
                free(dirlist[j]);
                j++;
            }
        }else if(ioption == 1 && loption == 0) {
            while (j < n) {
                if (dirlist[j]->d_name[0] != '.') {
                    struct stat sb;
                    char *path = calloc(sizeof(char),1024);
                    strcat(path,list[i]);
                    strcat(path,"/");
                    strcat(path,dirlist[j]->d_name);
                    if(stat(path,&sb) == -1){
                        perror("stat");
                        exit(0);
                    }
                    if (dirlist[j]->d_type == DT_DIR) {
                        list_dir[listc] = calloc(sizeof(char), 1024);
                        strcat(list_dir[listc], list[i]);
                        strcat(list_dir[listc], "/");
                        strcat(list_dir[listc++], dirlist[j]->d_name);
                        //changing text clr to cyan for directories to emulate ls behaviour
                        printf("  %ld ",sb.st_ino);
                        printf("\033[1;34m");
                        printf("%s\n",dirlist[j]->d_name);
                        printf("\033[1;37m");
                    }else{
                        printf("  %ld %s\n",sb.st_ino,dirlist[j]->d_name);
                    }
                    free(path);
                }
                free(dirlist[j]);
                j++;
            }
        }else if(ioption == 1 && loption == 1) {
            while (j < n) {
                if (dirlist[j]->d_name[0] != '.') {
                    struct stat sb;
                    char *path = calloc(sizeof(char),1024);
                    strcat(path,list[i]);
                    strcat(path,"/");
                    strcat(path,dirlist[j]->d_name);
                    if(stat(path,&sb) == -1){
                        perror("stat");
                        exit(0);
                    }

                    char *permission = getpermission(sb.st_mode);
                    // get file owner
                    struct passwd *pw;
                    pw = getpwuid(sb.st_uid);
                    if(pw == NULL){
                        perror("ERROR: cannot access pwuid\n");
                    }

                    // get file group
                    struct group *grp = getgrgid(sb.st_gid);
                    if(grp == NULL){
                        perror("ERROR: cannot access grgid ");
                    }

                    // get last modified time info
                    struct tm *tim;
                    time_t tme = sb.st_mtime;
                    tim = localtime(&tme);
                    char time[256];
                    strftime(time, sizeof(time),"%b %d %Y %H:%M",tim);

                    if (dirlist[j]->d_type == DT_DIR) {
                        list_dir[listc] = calloc(sizeof(char), 1024);
                        strcat(list_dir[listc], list[i]);
                        strcat(list_dir[listc], "/");
                        strcat(list_dir[listc++], dirlist[j]->d_name);
                        //changing text clr to cyan for directories to emulate ls behaviour
                        printf("%ld %s %li %s %s %5ld %s ",sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                        printf("\033[1;34m");
                        printf("%s\n",dirlist[j]->d_name);
                        printf("\033[1;37m");
                    }else{
                        printf("%ld %s %li %s %s %5ld %s %s\n",sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, dirlist[j]->d_name);
                    }
                    free(permission);
                    free(path);
                }
                free(dirlist[j]);
                j++;
            }
        }else if(ioption == 0 && loption == 1) {
            while (j < n) {
                if (dirlist[j]->d_name[0] != '.') {
                    struct stat sb;
                    char *path = calloc(sizeof(char),1024);
                    strcat(path,list[i]);
                    strcat(path,"/");
                    strcat(path,dirlist[j]->d_name);
                    if(stat(path,&sb) == -1){
                        perror("stat");
                        exit(0);
                    }

                    char *permission = getpermission(sb.st_mode);
                    // get file owner
                    struct passwd *pw;
                    pw = getpwuid(sb.st_uid);
                    if(pw == NULL){
                        perror("ERROR: cannot access pwuid\n");
                    }

                    // get file group
                    struct group *grp = getgrgid(sb.st_gid);
                    if(grp == NULL){
                        perror("ERROR: cannot access grgid ");
                    }

                    // get last modified time info
                    struct tm *tim;
                    time_t tme = sb.st_mtime;
                    tim = localtime(&tme);
                    char time[256];
                    strftime(time, sizeof(time),"%b %d %Y %H:%M",tim);

                    if (dirlist[j]->d_type == DT_DIR) {
                        list_dir[listc] = calloc(sizeof(char), 1024);
                        strcat(list_dir[listc], list[i]);
                        strcat(list_dir[listc], "/");
                        strcat(list_dir[listc++], dirlist[j]->d_name);
                        //changing text clr to cyan for directories to emulate ls behaviour
                        printf("%s %li %s %s %5ld %s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                        printf("\033[1;34m");
                        printf("%s\n",dirlist[j]->d_name);
                        printf("\033[1;37m");
                    }else{
                        printf("%s %li %s %s %5ld %s %s\n", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, dirlist[j]->d_name);
                    }
                    free(permission);
                    free(path);
                }
                free(dirlist[j]);
                j++;
            }
        }
        printf("\n\n");
        i++;
    }
    free(dirlist);
    Dirrecursor(list_dir,listc);
}

/** File option printers*/
void justPrint(struct dirent **dirlist, int size){
    int i = 0;
    while(i < size){
        if(dirlist[i]->d_name[0] != '.') {
            if (dirlist[i]->d_type == DT_DIR) {
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("\033[1;34m");
                printf("%s ", dirlist[i]->d_name);
                printf("\033[1;37m");
            } else {
                printf("%s ", dirlist[i]->d_name);
            }
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
}

void iPrint(struct dirent **dirlist, int size, char *dir){
    int i = 0;
    while(i < size){
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
            if (dirlist[i]->d_type == DT_DIR) {
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("  %ld ", sb.st_ino);
                printf("\033[1;34m");
                printf("%s\n", dirlist[i]->d_name);
                printf("\033[1;37m");
            } else {
                printf("  %ld %s\n", sb.st_ino, dirlist[i]->d_name);
            }
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
}

void lPrint(struct dirent **dirlist, int size, char *dir){
    int i = 0;
    while(i < size){
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

            // get file owner
            struct passwd *pw;
            pw = getpwuid(sb.st_uid);
            if (pw == NULL) {
                perror("ERROR: cannot access pwuid\n");
            }
            // get file group
            struct group *grp = getgrgid(sb.st_gid);
            if (grp == NULL) {
                perror("ERROR: cannot access grgid ");
            }

            // get last modified time info
            struct tm *tim;
            time_t tme = sb.st_mtime;
            tim = localtime(&tme);
            char time[256];
            strftime(time, sizeof(time), "%b %d %Y %H:%M", tim);

            if (dirlist[i]->d_type == DT_DIR) {
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("%s %li %s %s %5ld %s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                printf("\033[1;34m");
                printf("%s\n", dirlist[i]->d_name);
                printf("\033[1;37m");
            } else {
                printf("%s %li %s %s %5ld %s %s\n", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size,
                       time, dirlist[i]->d_name);
            }
            free(permission);
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
}

void RPrint(struct dirent **dirlist, int size, char *dir){
    int i = 0,listc = 0;
    char *list_dir[100];
    printf("%s:\n",dir);
    while(i < size){
        if(dirlist[i]->d_name[0] != '.') {
            if (dirlist[i]->d_type == DT_DIR) {
                list_dir[listc] = calloc(sizeof(char), 1024);
                strcat(list_dir[listc], dir);
                strcat(list_dir[listc], "/");
                strcat(list_dir[listc++], dirlist[i]->d_name);
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("\033[1;34m");
                printf("%s ", dirlist[i]->d_name);
                printf("\033[1;37m");
            } else {
                printf("%s ", dirlist[i]->d_name);
            }
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
    Dirrecursor(list_dir,listc);
}

void ilPrint(struct dirent **dirlist, int size, char *dir){
    int i = 0;
    while(i < size){
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
            // get file owner
            struct passwd *pw;
            pw = getpwuid(sb.st_uid);
            if (pw == NULL) {
                perror("ERROR: cannot access pwuid\n");
            }

            // get file group
            struct group *grp = getgrgid(sb.st_gid);
            if (grp == NULL) {
                perror("ERROR: cannot access grgid ");
            }

            // get last modified time info
            struct tm *tim;
            time_t tme = sb.st_mtime;
            tim = localtime(&tme);
            char time[256];
            strftime(time, sizeof(time), "%b %d %Y %H:%M", tim);
            if (dirlist[i]->d_type == DT_DIR) {
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("%ld %s %li %s %s %5ld %s ", sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name,
                       sb.st_size, time);
                printf("\033[1;34m");
                printf("%s\n", dirlist[i]->d_name);
                printf("\033[1;37m");
            } else {
                printf("%ld %s %li %s %s %5ld %s %s\n", sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name,
                       sb.st_size, time, dirlist[i]->d_name);
            }
            free(permission);
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
}

void lRPrint(struct dirent **dirlist, int size, char *dir) {
    int i = 0, listc = 0;
    char *list_dir[100];
    printf(".:\n");
    while (i < size) {
        if (dirlist[i]->d_name[0] != '.') {
            struct stat sb;
            char *path = calloc(sizeof(char),1024);
            strcat(path,dir);
            strcat(path,"/");
            strcat(path,dirlist[i]->d_name);
            if(stat(path,&sb) == -1){
                perror("stat");
                exit(0);
            }

            char *permission = getpermission(sb.st_mode);

            // get file owner
            struct passwd *pw;
            pw = getpwuid(sb.st_uid);
            if(pw == NULL){
                perror("ERROR: cannot access pwuid\n");
            }
            // get file group
            struct group *grp = getgrgid(sb.st_gid);
            if(grp == NULL){
                perror("ERROR: cannot access grgid ");
            }

            // get last modified time info
            struct tm *tim;
            time_t tme = sb.st_mtime;
            tim = localtime(&tme);
            char time[256];
            strftime(time, sizeof(time),"%b %d %Y %H:%M",tim);
            if (dirlist[i]->d_type == DT_DIR) {
                list_dir[listc] = calloc(sizeof(char), 1024);
                strcat(list_dir[listc], "./");
                strcat(list_dir[listc++], dirlist[i]->d_name);
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("%s %li %s %s %5ld %s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                printf("\033[1;34m");
                printf("%s\n",dirlist[i]->d_name);
                printf("\033[1;37m");
            }else{
                printf("%s %li %s %s %5ld %s %s\n", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, dirlist[i]->d_name);
            }
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
    Dirrecursor(list_dir, listc);
}

void iRPrint(struct dirent **dirlist, int size,char *dir) {
    int i = 0, listc = 0;
    char *list_dir[100];
    printf("%s:\n",dir);
    while (i < size) {
        if (dirlist[i]->d_name[0] != '.') {
            struct stat sb;
            char *path = calloc(sizeof(char),1024);
            strcat(path,dir);
            strcat(path,"/");
            strcat(path,dirlist[i]->d_name);
            if(stat(path,&sb) == -1){
                perror("stat");
                exit(0);
            }

            if (dirlist[i]->d_type == DT_DIR) {
                list_dir[listc] = calloc(sizeof(char), 1024);
                strcat(list_dir[listc], dir);
                strcat(list_dir[listc], "/");
                strcat(list_dir[listc++], dirlist[i]->d_name);
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("  %ld ",sb.st_ino);
                printf("\033[1;34m");
                printf("%s\n",dirlist[i]->d_name);
                printf("\033[1;37m");
            }else{
                printf("  %ld %s\n",sb.st_ino,dirlist[i]->d_name);
            }
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
    Dirrecursor(list_dir, listc);
}

void ilRPrint(struct dirent **dirlist, int size,char *dir) {
    int i = 0, listc = 0;
    char *list_dir[100];
    printf("%s:\n",dir);
    while (i < size) {
        if (dirlist[i]->d_name[0] != '.') {
            struct stat sb;
            char *path = calloc(sizeof(char),1024);
            strcat(path,dir);
            strcat(path,"/");
            strcat(path,dirlist[i]->d_name);
            if(stat(path,&sb) == -1){
                perror("stat");
                exit(0);
            }

            char *permission = getpermission(sb.st_mode);
            // get file owner
            struct passwd *pw;
            pw = getpwuid(sb.st_uid);
            if(pw == NULL){
                perror("ERROR: cannot access pwuid\n");
            }

            // get file group
            struct group *grp = getgrgid(sb.st_gid);
            if(grp == NULL){
                perror("ERROR: cannot access grgid ");
            }

            // get last modified time info
            struct tm *tim;
            time_t tme = sb.st_mtime;
            tim = localtime(&tme);
            char time[256];
            strftime(time, sizeof(time),"%b %d %Y %H:%M",tim);
            if (dirlist[i]->d_type == DT_DIR) {
                list_dir[listc] = calloc(sizeof(char), 1024);
                strcat(list_dir[listc],dir);
                strcat(list_dir[listc], "/");
                strcat(list_dir[listc++], dirlist[i]->d_name);
                //changing text clr to cyan for directories to emulate ls behaviour
                printf("%ld %s %li %s %s %5ld %s ",sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                printf("\033[1;34m");
                printf("%s\n",dirlist[i]->d_name);
                printf("\033[1;37m");
            }else{
                printf("%ld %s %li %s %s %5ld %s %s\n",sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, dirlist[i]->d_name);
            }
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    printf("\n\n");
    Dirrecursor(list_dir, listc);
}

//print file
void print_file(char *file) {
    struct stat sb;
    if (stat(file, &sb) == -1) {
        perror("ERROR: stat\n");
        exit(0);
    }
        // normal and -R
    if((ioption == 0 && loption == 0 && Roption == 0) || (ioption == 0 && loption == 0 && Roption == 1)){
        printf("%s ",file);
        // -i and -iR
    }else if((ioption == 1 && loption == 0 && Roption == 0) || (ioption == 1 && loption == 0 && Roption == 1)){
        printf("  %ld %s\n", sb.st_ino, file);
        // -l and -lR
    }else if((ioption == 0 && loption == 1 && Roption == 0) || (ioption == 0 && loption == 1 && Roption == 1)){
        char *permission = getpermission(sb.st_mode);
        // get file owner
        struct passwd *pw;
        pw = getpwuid(sb.st_uid);
        if (pw == NULL) {
            perror("ERROR: cannot access pwuid\n");
        }
        // get file group
        struct group *grp = getgrgid(sb.st_gid);
        if (grp == NULL) {
            perror("ERROR: cannot access grgid ");
        }
        // get last modified time info
        struct tm *tim;
        time_t tme = sb.st_mtime;
        tim = localtime(&tme);
        char time[256];
        strftime(time, sizeof(time), "%b %d %Y %H:%M", tim);
        printf("%s %li %s %s %5ld %s %s\n", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size,time, file);
        free(permission);
        // -il and -ilR
    }else if((ioption == 1 && loption == 1 && Roption == 0) || (ioption == 1 && loption == 1 && Roption == 1)){
        char *permission = getpermission(sb.st_mode);
        // get file owner
        struct passwd *pw;
        pw = getpwuid(sb.st_uid);
        if (pw == NULL) {
            perror("ERROR: cannot access pwuid\n");
        }
        // get file group
        struct group *grp = getgrgid(sb.st_gid);
        if (grp == NULL) {
            perror("ERROR: cannot access grgid ");
        }
        // get last modified time info
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

    option_i = option_l = option_R = 0;
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
            printf("\n\n");
            newline = 0;
        }
        //IF -R option is not set, print the path
        if(option_R == 0) {
            printf("%s:\n", paths[i]);
        }
        //print the contents of the directory
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
