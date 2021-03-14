/*
 * pa.c -- Creates directories for each student
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/**
 * Creates a directory for a specific student or exits on failure
 * @param dni Student ID
 */
static void create_directory(const char *dni)
{
    if (mkdir(dni, S_IRWXU | S_IRWXG | S_IRWXO) < 0 && errno != EEXIST) {
        perror("pa: error: could not create directory");
        exit(EXIT_FAILURE);
    }
}

/**
 * Program entry point
 * @return EXIT_SUCCESS on success or a non-zero value on failure
 */
int main(void)
{
    /* open student list */
    FILE *fd = fopen("estudiantes.txt", "r");
    if (!fd) {
        perror("pa: error: could not open file for reading");
        return EXIT_FAILURE;
    }

    /* iterate through all lines */
    char dni[BUFSIZ];
    for (; fscanf(fd, "%s %*c %*u", dni) > 0;) {
        printf("pa: creating directory for student `%s'\n", dni);
        create_directory((const char *)dni);
    }

    printf("pa: done creating directories\n");
    fclose(fd);
    return EXIT_SUCCESS;
}