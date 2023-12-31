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

#define PATH_LENGTH 1024
typedef int (*compare_func)(const struct dirent**, const struct dirent**);

int option_i, option_l, option_R; 
int numPaths, newline;

// Custom comparison function for scandir to sort entries alphabetically
int custom_sort(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

// Resolve symbolic link and get the actual path
char *resolve_symbolic_link(const char *path) {
    char resolved_path[PATH_LENGTH];
    realpath(path, resolved_path);
    return strdup(resolved_path);
}

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
    while(args[i] != NULL){
        // Special path handling
        if (strcmp(args[i], ".") == 0) {

            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            paths[numPaths] = strdup(cwd);
        } else if (strcmp(args[i], "..") == 0) {

            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            char *last_slash = strrchr(cwd, '/');
            if (last_slash != NULL) {
                *last_slash = '\0';
            }
            paths[numPaths] = strdup(cwd);
        } else if (strcmp(args[i], "~") == 0) {

            char *home = getenv("HOME");
            if (home != NULL) {
                paths[numPaths] = calloc(sizeof(char), PATH_LENGTH);
                strncpy(paths[numPaths], home, PATH_LENGTH);
                strncat(paths[numPaths], args[i] + 1, PATH_LENGTH - strlen(paths[numPaths]) - 1);
            } else {
                paths[numPaths] = strdup(args[i]);
            }
        } else if (strcmp(args[i], "*") == 1) {
            // wildcard not handled

        } else {
            paths[numPaths] = strdup(args[i]);
        }
        numPaths++;
        i++;
    }
}

//Check if the path is a file or directory
int check_file(char *path){
    struct stat path_stat;
    if (lstat(path, &path_stat) != 0) {
        fprintf(stderr, "Error: Nonexistent files or directories\n");
        exit(1);
    }
    return S_ISREG(path_stat.st_mode) || S_ISLNK(path_stat.st_mode);
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

//Print the contents of a directory based on the options
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
            if (dirlist[i]->d_type == DT_LNK) {
                if(option_i){
                    printf("%ld\t", sb.st_ino);
                }
                if (option_l) {
                    char *link_path = resolve_symbolic_link(path);
                    printf("%s %li %s %s\t%5ld\t%s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                    printf("%s -> %s\n", dirlist[i]->d_name, link_path);
                    free(link_path);
                } else {
                    printf("%s\n", dirlist[i]->d_name);
                }

            }else if (dirlist[i]->d_type == DT_DIR) {
                if(option_R){
                    list_dir[listc] = calloc(sizeof(char), 1024);
                    strcat(list_dir[listc], dir);
                    strcat(list_dir[listc], "/");
                    strcat(list_dir[listc++], dirlist[i]->d_name);
                }
                if(option_i){
                    printf("%ld\t", sb.st_ino);
                }
                if(option_l){
                    printf("%s %li %s %s\t%5ld\t%s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time);
                }

                printf("%s\n", dirlist[i]->d_name);

            } else {
                if(option_i){
                    printf("%ld\t", sb.st_ino);
                }
                if(option_l){
                    printf("%s %li %s %s\t%5ld\t%s ", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size,
                           time);
                }
                printf("%s\n",dirlist[i]->d_name);
            }
            free(permission);
            free(path);
        }
        free(dirlist[i]);
        i++;
    }
    free(dirlist);
    if(i < size - 1){
        printf("\n");
    }
    if(option_R){
        // Dirrecursor(list_dir,listc);
        // recursive_print(list_dir,listc);
        for (i = 0; i < listc; i++) {
            int n = scandir(list_dir[i], &dirlist, 0, (compare_func)custom_sort);
            if (n == -1) {
                perror("scandir");
                continue;
            }
            printf("\n%s:\n",list_dir[i]);
            print_option(dirlist, n, list_dir[i]);
        }
    }
}

//Print the file based on the options
void print_file(char *file) {
    // printf("print file from command line\n");
    struct stat sb;
    if (lstat(file, &sb) == -1) {
        perror("ERROR: stat\n");
        exit(0);
    }

    int flag = 0;
    if (option_i) flag |= 1;
    if (option_l) flag |= 2;
    if (option_R) flag |= 4;

    if (flag == 0 || flag == 4) {
        printf("%s\t\n", file);
    } else if (flag == 1 || flag == 5) {
        printf("%ld\t%s\n", sb.st_ino, file);
    } else if (flag == 2 || flag == 6) {
        char *permission = getpermission(sb.st_mode);
        struct passwd *pw = getpwuid(sb.st_uid);
        struct group *grp = getgrgid(sb.st_gid);
        struct tm *tim;
        time_t tme = sb.st_mtime;
        tim = localtime(&tme);
        char time[256];
        strftime(time, sizeof(time), "%b %d %Y %H:%M", tim);
        printf("%s %li %s %s\t%5ld\t%s %s", permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, file);
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
        printf("%ld %s %li %s %s\t%5ld\t%s %s", sb.st_ino, permission, sb.st_nlink, pw->pw_name, grp->gr_name, sb.st_size, time, file);
        free(permission);
    }
    // Check if it is a symbolic link
    if (S_ISLNK(sb.st_mode) && option_l) {
        char resolved_path[PATH_LENGTH];
        ssize_t resolved_len = readlink(file, resolved_path, sizeof(resolved_path) - 1);
        if (resolved_len == -1) {
            perror("ERROR: readlink\n");
            return;
        }
        resolved_path[resolved_len] = '\0';
        printf(" -> %s", resolved_path);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {

    struct dirent **dirlist;
    int n = 0;

    option_i = option_l = option_R = 0;
    numPaths = newline = 0;

    char *paths[argc];
    
    // Parse options
    int path_index = check_option(argv);
    if(path_index == 0){
        paths[numPaths] = calloc(sizeof(char), 2);
        strcpy(paths[numPaths], ".");
        numPaths++;
    }else{
        find_path(argv, paths, path_index);
    }

    int i = 0;
    while (i < numPaths) {
        //if it is a file, print it
        if(check_file(paths[i]) == 1){
            print_file(paths[i]);
            i++;
            newline = 1;
            // printf("\n");
            continue;
        }
        //if it is a directory, print its contents
        n = scandir(paths[i], &dirlist, 0, (compare_func)custom_sort);
        if (n == -1) {
            if(paths[i][0] == '-'){
                printf("ERROR: File options should enter before path\n");
                exit(0);
            }
            printf("Error: Nonexistent files or directories\n");
            continue;
        }
        
        // formatters
        if(newline == 1){
            printf("\n");
            newline = 0;
        }
        
        if(path_index!=0){
            printf("%s:\n",paths[i]);
        }
        print_option(dirlist, n, paths[i]);
        if(i < numPaths - 1){
            printf("\n");
        }
        i++;
    }
    for (int i = 0; i < argc; i++) {
        free(paths[i]);
    }

    return 0;
}
