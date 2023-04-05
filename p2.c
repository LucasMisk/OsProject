#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#define MAX_OPTIONS 10
#define BUFFER_SIZE 1024

void print_reg_file_info(char* filename);
void print_sym_link_info(char* filename);
void print_access_rights(mode_t mode);

int main(int argc, char* argv[]) {
    int i;
    char options[MAX_OPTIONS];
    
    for (i = 1; i < argc; i++) {
        char* filename = argv[i];
        struct stat st;
        
        if (lstat(filename, &st) == -1) {
            perror("lstat");
            continue;
        }
        
        switch (st.st_mode & S_IFMT) {
            case S_IFREG:
                printf("%s: regular file\n", filename);
                print_reg_file_info(filename);
                break;
            case S_IFLNK:
                printf("%s: symbolic link\n", filename);
                print_sym_link_info(filename);
                break;
            case S_IFDIR:
                printf("%s: directory\n", filename);
                break;
            default:
                printf("%s: unknown file type\n", filename);
                break;
        }
        
        printf("\nEnter options for %s: ", filename);
        scanf("%s", options);
        printf("Options: %s\n", options);
        // perform actions based on options
    }
    
    return 0;
}

void print_reg_file_info(char* filename) {
    struct stat st;
    
    if (stat(filename, &st) == -1) {
        perror("stat");
        return;
    }
    
    printf("Name: %s\n", filename);
    printf("Size: %ld bytes\n", st.st_size);
    printf("Hard link count: %ld\n", st.st_nlink);
    printf("Last modified time: %s", ctime(&st.st_mtime));
    printf("Access rights: ");
    print_access_rights(st.st_mode);
    
    char options[MAX_OPTIONS];
    printf("Create symbolic link (-I): ");
    scanf("%s", options);
    if (strcmp(options, "-I") == 0) {
        char linkname[BUFFER_SIZE];
        printf("Enter link name: ");
        scanf("%s", linkname);
        if (symlink(filename, linkname) == -1) {
            perror("symlink");
        }
    }
}

void print_sym_link_info(char* filename) {
    struct stat st;
    
    if (lstat(filename, &st) == -1) {
        perror("lstat");
        return;
    }
    
    printf("Name: %s\n", filename);
    printf("Size of symbolic link: %ld bytes\n", st.st_size);
    printf("Size of target file: %ld bytes\n", stat(filename, &st) == -1 ? -1 : st.st_size);
    printf("Access rights: ");
    print_access_rights(st.st_mode);
    
    char options[MAX_OPTIONS];
    printf("Delete symbolic link (-I): ");
    scanf("%s", options);
    if (strcmp(options, "-I") == 0) {
        if (unlink(filename) == -1) {
            perror("unlink");
        }
    }
}

void print_access_rights(mode_t mode) {
    printf("User: Read - %s Write - %s Exec - %s ",
        (mode & S_IRUSR) ? "yes" : "no",
        (mode & S_IWUSR) ? "yes" : "no",
        (mode & S_IWUSR) ? "yes" : "no",
        (mode & S_IXUSR) ? "yes" : "no");
        printf("Group: Read - %s Write - %s Exec - %s ",
        (mode & S_IRGRP) ? "yes" : "no",
        (mode & S_IWGRP) ? "yes" : "no",
        (mode & S_IXGRP) ? "yes" : "no");
        printf("Others: Read - %s Write - %s Exec - %s\n",
        (mode & S_IROTH) ? "yes" : "no",
        (mode & S_IWOTH) ? "yes" : "no",
        (mode & S_IXOTH) ? "yes" : "no");
}
