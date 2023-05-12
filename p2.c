#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_OPTIONS 10
#define BUFFER_SIZE 1024

void print_reg_file_info(char *filename);
void print_sym_link_info(char *filename);
void print_access_rights(mode_t mode);
void print_dir_info(char *dirname);

int main(int argc, char *argv[])
{
    int i;
    int fp = open("grades.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
    int fl = open("childlog.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
    int statusreg1, statusreg2, statuslink1, statuslink2, statusdir1, statusdir2;
    for (i = 1; i < argc; i++)
    {
        char *filename = argv[i];
        struct stat st;
        if (lstat(filename, &st) == -1)
        {
            perror("lstat");
            continue;
        }
        switch (st.st_mode & S_IFMT)
        {
        case S_IFREG:
        {
            printf("\n%s: reg file\n", filename);
            int pipefd[2];
            if (pipe(pipefd) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            pid_t pidreg1 = fork();
            if (pidreg1 == 0)
            {
                // code executed by the child process

                print_reg_file_info(filename);
                exit(EXIT_SUCCESS);
            }
            pid_t pidreg2 = fork();
            if (pidreg2 == 0)
            {
                if (filename[strlen(filename) - 1] == 'c' && filename[strlen(filename) - 2] == '.')
                {
                    close(pipefd[0]);               // Close read end of pipe
                    dup2(pipefd[1], STDOUT_FILENO); // Redirect standard output to write end of pipe
                    close(pipefd[1]);               // Close write end of pipe
                    char scriptname[10] = "script.sh";
                    execlp("bash", "bash", scriptname, filename, NULL);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    char scriptname1[10] = "script1.sh";
                    execlp("bash", "bash", scriptname1, filename, NULL);
                    exit(EXIT_FAILURE);
                }
            }
            //   we wait for the pidc to end with waitpid
            pid_t wreg1 = waitpid(pidreg1, &statusreg1, 0);
            if (WIFEXITED(statusreg1))
            {
                char writea[100];
                sprintf(writea, "Child with pid %d, exited with status=%d\n", wreg1, WEXITSTATUS(statusreg1));
                write(fl, writea, strlen(writea));
            }
            pid_t wreg2 = waitpid(pidreg2, &statusreg2, 0);
            if (WIFEXITED(statusreg2))
            {
                char writea[100];
                sprintf(writea, "Child with pid %d, exited with status=%d\n", wreg2, WEXITSTATUS(statusreg2));
                write(fl, writea, strlen(writea));
                close(pipefd[1]);
                char buffer[100];
                int bytes_read = read(pipefd[0], buffer, 100);
                if (bytes_read == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                close(pipefd[0]);
                buffer[bytes_read] = '\0';
                char copy[100];
                strcpy(copy, buffer);
                char *ptr = strtok(copy, "/");
                int err = atoi(ptr);
                ptr = strtok(NULL, "\0");
                int wrr = atoi(ptr);
                int grade;
                if (err == 0 && wrr == 0)
                    grade = 10;
                else if (err == 0 && wrr > 10)
                    grade = 2;
                else if (err == 0 && wrr < 10)
                    grade = 2 + 8 * (10 - wrr) / 10;
                else if (err > 0)
                    grade = 1;
                // printf("%d errors\n", err);
                // printf("%d warnings\n", wrr);
                char gradeString[100];
                sprintf(gradeString, "Grade for file %s is: %d\n", filename, grade);
                strcat(gradeString, "\0");
                write(fp, gradeString, strlen(gradeString));
                // we exit to finish the processes
            }

            break;
        }
        case S_IFLNK:
            printf("\n%s: symbolic link\n", filename);
            pid_t pidlink1 = fork();
            if (pidlink1 == 0)
            {
                print_sym_link_info(filename);
                exit(EXIT_SUCCESS);
            }
            pid_t pidlink2 = fork();
            if (pidlink2 == 0)
            {
                execlp("chmod", "chmod", "760", filename, NULL);
                exit(EXIT_SUCCESS);
            }
            pid_t wlink1 = waitpid(pidlink1, &statuslink1, 0);
            if (WIFEXITED(statuslink1))
            {
                char writea[100];
                sprintf(writea, "Child with pid %d, exited with status=%d\n", wlink1, WEXITSTATUS(statuslink1));
                write(fl, writea, strlen(writea));
            }
            pid_t wlink2 = waitpid(pidlink2, &statuslink2, 0);
            if (WIFEXITED(statuslink1))
            {
                char writea[100];
                sprintf(writea, "Child with pid %d, exited with status=%d\n", wlink2, WEXITSTATUS(statuslink2));
                write(fl, writea, strlen(writea));
            }
            break;
        case S_IFDIR:
            printf("\n%s: directory\n", filename);
            pid_t piddir1 = fork();
            if (piddir1 == 0)
            {
                print_dir_info(filename);
                exit(EXIT_SUCCESS);
            }
            pid_t piddir2 = fork();
            if (piddir2 == 0)
            {
                char name[15] = "file2";
                char command[50] = "touch ";
                strcat(command, filename);
                strcat(command, "/");
                strcat(command, name);
                strcat(command, ".txt");
                system(command);
                exit(EXIT_SUCCESS);
            }
            pid_t wdir1 = waitpid(piddir1, &statusdir1, 0);
            if (WIFEXITED(statusdir1))
            {
                char writea[100];
                sprintf(writea, "Child with pid %d, exited with status=%d\n", wdir1, WEXITSTATUS(statusdir1));
                write(fl, writea, strlen(writea));
            }
            pid_t wdir2 = waitpid(piddir2, &statusdir2, 0);
            if (WIFEXITED(statusdir2))
            {
                char writea[100];
                sprintf(writea, "Child with pid %d, exited with status=%d\n", wdir2, WEXITSTATUS(statusdir2));
                write(fl, writea, strlen(writea));
            }
            break;
        default:
            printf("%s: unknown file type\n", filename);
            break;
        }
    }
    close(fp);
    return 0;
}

void print_reg_file_info(char *filename)
{
    struct stat st;
    char options[MAX_OPTIONS];
    int ok = 1;
    if (stat(filename, &st) == -1)
    {
        perror("stat");
        return;
    }

    printf("\n\nOptions: \n-name(-n)\n-size(-d)\n-hard link count(-h)\n-time of last modification(-m)\n-access rights(-a)\n-create symbolic link(-l)\nYour options:");
    scanf("%s", options);

    for (int i = 0; i < strlen(options); i++)
    {
        if (strchr("- nldahm", options[i]) == NULL)
            ok = 0;
    }
    if (ok == 0)
    {
        printf("Invalid option\n");
    }
    if (options[0] == '-' && ok == 1)
    {
        for (int i = 0; i < strlen(options); i++)
        {
            switch (options[i])
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

void print_sym_link_info(char *filename)
{
    struct stat st;
    char options[MAX_OPTIONS];
    int ok = 1;
    if (lstat(filename, &st) == -1)
    {
        perror("lstat");
        return;
    }
    printf("\n\nOptions: \n-name(-n)\n-delete symblic link(-l)\n-size of symblic link(-d)\n-size of target(-t)\n-access rights(-a)\nYour options:");
    scanf("%s", options);

    for (int i = 0; i < strlen(options); i++)
    {
        if (strchr("- nldta", options[i]) == NULL)
            ok = 0;
    }
    if (ok == 0)
    {
        printf("Invalid option\n");
    }
    if (options[0] == '-' && ok == 1)
    {
        for (int i = 0; i < strlen(options); i++)
        {
            switch (options[i])
            {
            case 'n':
                printf("Name: %s\n", filename);
                break;
            case 'l':
                if (unlink(filename) == -1)
                {
                    perror("unlink");
                }
                i = strlen(options);
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

void print_dir_info(char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char filename[BUFFER_SIZE];
    int total_size = 0;
    int total_c_files = 0;
    char options[MAX_OPTIONS];
    int ok = 1;
    if ((dir = opendir(dirname)) == NULL)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        sprintf(filename, "%s/%s", dirname, entry->d_name);
        if (lstat(filename, &st) == -1)
        {
            perror("lstat");
            continue;
        }
        if (S_ISREG(st.st_mode))
        {
            if (strstr(entry->d_name, ".c") != NULL)
            {
                total_c_files++;
            }
            total_size += st.st_size;
        }
    }
    printf("\n\nOptions: \n-name(-n)\n-size(-d)\n-access rights(-a)\n-number of C files(-c)\nYour options:");
    scanf("%s", options);
    for (int i = 0; i < strlen(options); i++)
    {
        if (strchr("- ndca", options[i]) == NULL)
            ok = 0;
    }
    if (ok == 0)
    {
        printf("Invalid option\n");
    }
    if (options[0] == '-' && ok == 1)
    {
        for (int i = 0; i < strlen(options); i++)
        {
            switch (options[i])
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

void print_access_rights(mode_t mode)
{
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
