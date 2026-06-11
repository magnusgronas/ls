#include <dirent.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void mode_string(mode_t mode, char *str) {
    if (S_ISDIR(mode))       str[0] = 'd';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else if (S_ISCHR(mode))  str[0] = 'c';
    else if (S_ISBLK(mode))  str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else                     str[0] = '-';

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';

    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';

    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';

    str[10] = '\0';
}

void print_long(const char *dir, const char *name) {
    char fullpath[4096]; // max length a linux path can have
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);

    struct stat st;
    if (lstat(fullpath, &st) < 0) {
        perror(name);
        return;
    }
    char modes[11];
    mode_string(st.st_mode, modes);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char timebuf[64];

    struct tm *tm = localtime(&st.st_mtim.tv_sec);
    strftime(timebuf, sizeof(timebuf), "%b  %e %H:%M", tm);
    printf(
        "%s %lu %s %s %5ld %s %s\n",
        modes,
        (unsigned long)st.st_nlink,
        user,
        group,
        (long)st.st_size,
        timebuf,
        name
    );
}

int show_all = 0;
int long_format = 0;

int main(int argc, char *argv[]) {

    setlocale(LC_ALL, "");

    int opt;

    while ((opt = getopt(argc, argv, "al")) != -1) {
        switch (opt) {
        case 'a':
            show_all = 1;
            break;
        case 'l':
            long_format = 1;
            break;
        default:
            fprintf(stderr, "usage: %s [-al] [path]\n", argv[0]);
            return 1;
        }
    }

    const char *path = optind < argc ? argv[optind] : ".";

    struct dirent **namelist;
    int num_entries;

    num_entries = scandir(path, &namelist, NULL, alphasort);

    if (num_entries < 0) {
        perror("scandir");
        return 1;
    }

    for (int i = 0; i < num_entries; i++) {
        if(!show_all && namelist[i]->d_name[0] == '.') {
            free(namelist[i]);
            continue;
        }

        if (long_format) {
            print_long(path, namelist[i]->d_name);
        } else {
            printf("%s\n", namelist[i]->d_name);
        }
        free(namelist[i]);
    }

    free(namelist);

    return 0;
}
