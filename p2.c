#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>


#define MAX_OPTIONS 10
#define BUFFER_SIZE 1024

void print_reg_file_info(char* filename);
void print_sym_link_info(char* filename);
void print_access_rights(mode_t mode);
void print_dir_info(char* dirname);

int main(int argc, char* argv[]) {
    int i;
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
                print_dir_info(filename);
                break;
            default:
                printf("%s: unknown file type\n", filename);
                break;
        }
    }
    
    return 0;
}

void print_reg_file_info(char* filename) {
    struct stat st;
    char options[MAX_OPTIONS];
    int ok= 1;
    if (stat(filename, &st) == -1) {
        perror("stat");
        return;
    }

    printf("Options: name(-n) , size(-d), hard link count(-h), time of last modification(-m), access rights(-a), create symbolic link(-l): ");
    scanf("%s", options);

    for(int i=0;i<strlen(options); i++)
    {
        if(strchr("- nldahm",options[i])==NULL)
            ok=0;
    }
    if(ok==0)
    {
        printf("Invalid option\n");
    }
    if (options[0]=='-' && ok==1)
    {
        for(int i=0;i<strlen(options); i++)
        {
            switch(options[i])
            {
                case 'n':
                    printf("Name: %s\n", filename);
                    break;
                case 'd':
                    printf("Size: %ld bytes\n", st.st_size);
                    break;
                case 'h':
                    printf("Hard link count: %ld\n", st.st_nlink);
                    break;
                case 'm':
                    printf("Last modified time: %s", ctime(&st.st_mtime));
                    break;
                case 'a':
                    printf("Access rights: ");
                    print_access_rights(st.st_mode);
                    break;
                case 'l':
                    printf("Create symbolic link (-l): ");
                    char linkname[BUFFER_SIZE];
                    printf("Enter link name: ");
                    scanf("%s", linkname);
                    if (symlink(filename, linkname) == -1) 
                    {
                        perror("symlink");
                    }
                    break;
                case '-':
                    break;
                case ' ':
                    break;
                default:
                    break;
            }
        }
    }
}

void print_sym_link_info(char* filename) {
    struct stat st;
    char options[MAX_OPTIONS];
    int ok=1;
    if (lstat(filename, &st) == -1) {
        perror("lstat");
        return;
    }
    printf("Options: name(-n), delete symblic link(-l) , size of symblic link(-d), size of target(-t), access rights(-a): ");
    scanf("%s", options);

    for(int i=0;i<strlen(options); i++)
    {
        if(strchr("- nldta",options[i])==NULL)
            ok=0;
    }
    if(ok==0)
    {
        printf("Invalid option\n");
    }
    if (options[0]=='-' && ok==1)
    {
        for(int i=0;i<strlen(options); i++)
        {
            switch(options[i])
            {
                case 'n':
                    printf("Name: %s\n", filename);
                    break;
                case 'l':
                    if (unlink(filename) == -1) 
                    {
                        perror("unlink");
                    }
                    i=strlen(options);
                    break;
                case 'd':
                    printf("Size of symbolic link: %ld bytes\n", st.st_size);
                    break;
                case 't':
                    printf("Size of target file: %ld bytes\n", stat(filename, &st) == -1 ? -1 : st.st_size);
                    break;
                case 'a':
                    printf("Access rights: ");
                    print_access_rights(st.st_mode);
                    break;
                case '-':
                    break;
                case ' ':
                    break;
                default:
                    break;
            }
        }
    }
    
}

void print_dir_info(char* dirname) 
{
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    char filename[BUFFER_SIZE];
    int total_size = 0;
    int total_c_files = 0;
    char options[MAX_OPTIONS];
    int ok=1;
    if ((dir = opendir(dirname)) == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        sprintf(filename, "%s/%s", dirname, entry->d_name);
        if (lstat(filename, &st) == -1) {
            perror("lstat");
            continue;
        }
        if (S_ISREG(st.st_mode)) {
            if (strstr(entry->d_name, ".c") != NULL) {
                total_c_files++;
            }
            total_size += st.st_size;
        }
    }
    printf("Options: name(-n), size(-d), access rights(-a), number of C files(-c): ");
    scanf("%s", options);
    for(int i=0;i<strlen(options); i++)
    {
        if(strchr("- ndca",options[i])==NULL)
            ok=0;
    }
    if(ok==0)
    {
        printf("Invalid option\n");
    }
    if (options[0]=='-' && ok==1)
    {
        for(int i=0;i<strlen(options); i++)
        {
            switch(options[i])
            {
                case 'n':
                    printf("%s: directory\n", dirname);
                    break;
                case 'd':
                    printf("Total size: %d bytes\n", total_size);
                    break;
                case 'c':
                    printf("Total .c files: %d\n", total_c_files);
                    break;
                case 'a':
                    printf("Access rights: \n");
                    print_access_rights(st.st_mode);
                    break;
                case '-':
                    break;
                case ' ':
                    break;
                default:
                    break;
            }
        }
    }
    closedir(dir);
}


void print_access_rights(mode_t mode) {
    printf("User: Read - %s Write - %s Exec - %s \n",
        (mode & S_IRUSR) ? "yes" : "no",
        (mode & S_IWUSR) ? "yes" : "no",
        (mode & S_IXUSR) ? "yes" : "no");
        printf("Group: Read - %s Write - %s Exec - %s \n",
        (mode & S_IRGRP) ? "yes" : "no",
        (mode & S_IWGRP) ? "yes" : "no",
        (mode & S_IXGRP) ? "yes" : "no");
        printf("Others: Read - %s Write - %s Exec - %s\n",
        (mode & S_IROTH) ? "yes" : "no",
        (mode & S_IWOTH) ? "yes" : "no",
        (mode & S_IXOTH) ? "yes" : "no");
}
