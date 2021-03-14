/*
 * pb.c -- Creates the corresponding exam file for each student
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Creates a link for the corresponding exam file for a specific student
 * @param dni Student ID
 * @param exam_model Exam model
 * @return 0 on success or a negative value on failure
 */
static int copy_exam_file(const char *dni, char exam_model)
{
    char oldpath[BUFSIZ], newpath[BUFSIZ];
    sprintf(oldpath, "examenes/MODELO%c.pdf", toupper(exam_model));
    sprintf(newpath, "%s/Exam.pdf", dni);
    if (link(oldpath, newpath) < 0 && errno != EEXIST) {
        perror("pb: error linking exam file");
        return -1;
    }
    return 0;
}

/**
 * Program entry point
 * @return EXIT_SUCCESS on success or a non-zero value on failure
 */
int main(void)
{
    int rc = EXIT_SUCCESS;

    /* open student list */
    FILE *fd = fopen("estudiantes.txt", "r");
    if (!fd) {
        perror("pb: error: could not open file for reading");
        return EXIT_FAILURE;
    }

    /* iterate over all students */
    char dni[BUFSIZ];
    char exam_model;
    while (fscanf(fd, "%s %c %*u", dni, &exam_model) > 0) {
        printf("pb: copying exam model %c for student `%s'\n", exam_model, dni);
        if (copy_exam_file(dni, exam_model) < 0) {
            rc = EXIT_FAILURE;
            goto end;
        }
    }
    printf("pb: done copying exam models\n");

end:
    fclose(fd);
    return rc;
}