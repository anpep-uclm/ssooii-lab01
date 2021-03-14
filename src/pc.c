/*
 * pc.c -- Calculates the required score for each student to pass the subject
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Creates a requirement note for each student
 * @param dni Student ID
 * @param req_score Minimum required score
 * @return 0 on success or a negative value on failure
 */
static int create_requirement_note(const char *dni, unsigned req_score)
{
    /* build file path */
    char path[BUFSIZ];
    sprintf(path, "%s/note.txt", dni);

    /* create file */
    FILE *fd = fopen(path, "w+");
    if (!fd) {
        perror("pc: error: could not open file for reading");
        return -1;
    }

    /* write message and close */
    fprintf(fd, "In order to pass this course, you'll need a minimum of %u marks in this exam.\n", req_score);
    fclose(fd);
    return 0;
}

/**
 * Program entry point
 * @return EXIT_SUCCESS on success or a non-zero value on failure
 */
int main(void)
{
    int rc = EXIT_SUCCESS;
    FILE *students_fd = NULL,
         *pipe_fd = NULL;

    /* open input students file */
    students_fd = fopen("estudiantes.txt", "r");
    if (!students_fd) {
        perror("pc: error: could not open file for reading");
        rc = EXIT_FAILURE;
        goto end;
    }

    /* open pipe for writing avg score to */
    pipe_fd = fopen("/tmp/pc_fifo", "w");
    if (!pipe_fd) {
        perror("pc: error: could not open pipe for writing");
        rc = EXIT_FAILURE;
        goto end;
    }

    /* iterate over lines from the students file */
    char dni[BUFSIZ];
    unsigned score, req_score;
    unsigned student_count = 0, acc_score = 0;
    while (fscanf(students_fd, "%s %*c %u", dni, &score) > 0) {
        /* calculate minimum required score and write to the note file */
        req_score = 2 * 5 - score;
        printf("pc: student `%s' has score %d and thus needs a %d in this exam\n", dni, score, req_score);
        if (create_requirement_note(dni, req_score) < 0) {
            rc = EXIT_FAILURE;
            goto end;
        }

        /* accumulate scores for calculating average later */
        acc_score += score;
        student_count++;
    }
    printf("pc: done gathering required scores\n");

    /* calculate avg score and write to pipe for the manager process to read */
    float avg_score = (float)acc_score / (float)student_count;
    printf("pc: average score is %f\n", avg_score);
    if (fwrite(&avg_score, sizeof(avg_score), 1, pipe_fd) != 1) {
        perror("pc: error: could not write to pipe");
        rc = EXIT_FAILURE;
        goto end;
    }

end:
    if (pipe_fd)
        fclose(pipe_fd);
    if (students_fd)
        fclose(students_fd);
    return rc;
}