/*
 * pd.c -- Cleans up created files
 * Copyright (c) Angel <angel@ttm.sh>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* maximum number of blacklisted nodes */
#define MAX_BLACKLISTED_NODES 16

/* blacklisted filesystem nodes */
static char s_blacklist[BUFSIZ][MAX_BLACKLISTED_NODES];
/* number of blacklisted filesystem nodes */
static size_t s_blacklist_len;

/**
 * Recursively deletes files and directories
 * @param path Path of the filesystem node to be deleted
 */
static void recursive_unlink(const char *path)
{
    struct stat st;
    if (stat(path, &st) < 0) {
        perror("error: could not stat");
        return;
    }

    if (S_ISREG(st.st_mode)) {
        /* unlink regular files */
        printf("deleting: %s\n", path);
        unlink(path);
    } else if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) {
            perror("error: could not open directory");
            return;
        }

        /* recursively delete directory */
        struct dirent *dp;
        char joint_path[512];
        while ((dp = readdir(dir)) != NULL) {
            if (!strcmp(".", dp->d_name) || !strcmp("..", dp->d_name))
                continue;
            snprintf(joint_path, sizeof(joint_path), "%s/%s", path, dp->d_name);
            recursive_unlink(joint_path);
        }
        /* delete directory entry */
        printf("deleting: %s\n", path);
        rmdir(path);
        closedir(dir);
    }
}

/**
 * Determines whether or not the specified entry is blacklisted
 * @param name Node nmae
 * @return 1 if the entry is blacklisted, 0 if not
 */
static int is_blacklisted(const char *name)
{
    if (!strcmp(".", name) || !strcmp("..", name))
        return 0;

    size_t name_len = strlen(name);
    for (size_t i = 0; i < s_blacklist_len; i++) {
        if (strncmp(s_blacklist[i], name, name_len) == 0)
            return 0;
    }
    return 1;
}

int main(void)
{
    /* open and read .gitignore entries */
    FILE *fd = fopen(".gitignore", "r");
    /* skip wildcard entry */
    fscanf(fd, "*\n");
    while (s_blacklist_len < MAX_BLACKLISTED_NODES
        && fscanf(fd, "!/%s\n", s_blacklist[s_blacklist_len++]) > 0);

    /* iterate through .gitignore entries and recursively delete all non-matching files */
    DIR *dir = opendir(".");
    if (!dir) {
        perror("error: could not open directory");
        return EXIT_FAILURE;
    }

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        if (is_blacklisted(dp->d_name))
            recursive_unlink(dp->d_name);
    }

    closedir(dir);
    fclose(fd);
    return EXIT_SUCCESS;
}