/*
 * manager.c -- manager process
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

#define _XOPEN_SOURCE /* required for kill(2) */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

#define PROGNAME "manager"

/**
 * Writes an entry to the log file
 * @param fmt Format string
 * @param ... Variadic arguments for formatting the input
 */
#define logf(fmt, ...)                                           \
    {                                                            \
        printf(PROGNAME ": " fmt "\n", ##__VA_ARGS__);           \
        fprintf(s_logfd, PROGNAME ": " fmt "\n", ##__VA_ARGS__); \
    }

/* path for the named PIPE shared with process C */
#define PIPE_PATH "/tmp/pc_fifo"

static FILE *s_logfd; /* log file descriptor */
static pid_t s_pids[64]; /* list of PIDs to be terminated upon SIGINT */
static size_t s_npids; /* number of PIDs to be terminated upon SIGINT */

/**
 * Spawns and registers a process
 * @param image_path Path of the executable
 * @return The spawned process ID on success, or a negative value on failure
 */
static pid_t create_process(const char *image_path)
{
    pid_t child_pid;
    char *argv[] = { (char *)image_path, NULL };

    switch ((child_pid = fork())) {
    case 0:
        /* child process */
        signal(SIGINT, SIG_DFL); /* don't inherit parent SIGINT */
        if (execve(image_path, argv, NULL) < 0) {
            child_pid = -1;
        } else {
            exit(EXIT_FAILURE);
        }
        break;
    case -1:
        /* failed */
        fprintf(s_logfd, "[-] create_process: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* register PID */
    s_pids[s_npids++] = child_pid;
    return child_pid;
}

/**
 * Signal handler for SIGINT
 * @param signum Unused
 */
static void sigint_handler(int __attribute__((unused)) signum)
{
    logf("received SIGINT -- cleaning up and killing %zu processes", s_npids);

    /* kill all registered processes */
    pid_t pid;
    for (; s_npids > 0; s_npids--) {
        pid = s_pids[s_npids - 1];
        logf("killing process %u [%u]", pid, getpid());
        if (kill(pid, SIGKILL) < 0) {
            logf("error killing process %u: %s", pid, strerror(errno));
        }
    }
    logf("done killing processes");

    /* run cleanup process */
    pid = create_process("./exec/pd");
    waitpid(pid, NULL, 0);

    exit(EXIT_SUCCESS);
}

/**
 * Program entry point
 * @return EXIT_SUCCESS on success or a non-zero value on failure
 */
int main(void)
{
    int rc = EXIT_SUCCESS;
    FILE *pipe_fd = NULL;

    /* open/create log file */
    s_logfd = fopen("log.txt", "w+");

    /* register SIGINT handler */
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        logf("error registering signal handler: %s", strerror(errno));
        rc = EXIT_FAILURE;
        goto end;
    }

    /* create user directories */
    logf("creating user directories");
    create_process("exec/pa");
    wait(NULL);
    logf("user directories created");

    /* create pipe for receiving avg score from process C */
    if (unlink(PIPE_PATH) < 0 && errno != ENOENT) {
        logf("error unlinking pipe for process C: %s", strerror(errno));
        rc = EXIT_FAILURE;
        goto end;
    }
    if (mkfifo(PIPE_PATH, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
        logf("error creating pipe for process C: %s", strerror(errno));
        rc = EXIT_FAILURE;
        goto end;
    }

    /* open named pipe before process C so call to open() from process C does not block */
    pipe_fd = fopen(PIPE_PATH, "r+");
    if (!pipe_fd) {
        logf("error opening pipe: %s", strerror(errno));
        rc = EXIT_FAILURE;
        goto end;
    }

    /* spawn processes */
    logf("invoking processes B and C");
    pid_t b_pid = create_process("exec/pb"),
          c_pid = create_process("exec/pc");

    /* join on B and C termination */
    waitpid(b_pid, NULL, 0);
    waitpid(c_pid, NULL, 0);
    logf("both B and C terminated");

    /* read avg score from pipe */
    float avg_score;
    if (fread(&avg_score, sizeof(avg_score), 1, pipe_fd) != 1) {
        logf("error reading from pipe: %s", strerror(errno));
        rc = EXIT_FAILURE;
        goto end;
    }
    logf("average score read from pipe: %f", avg_score);

end:
    if (pipe_fd) {
        fclose(pipe_fd);
        unlink(PIPE_PATH);
    }
    if (s_logfd)
        fclose(s_logfd);
    return rc;
}