#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "wrapper.h"
#include "stats.h"
#include "config.h"
#include "commons.h"
#include "forkHandler.h"

char *mainToReportPipe = "/tmp/mainToReport.pipe";
char *path;
int fdToReport = 0;
int isReportRunning = 0;
int analyzerPid = -1;

void showHelp();

int isReadyToRun(config *conf)
{
    if (conf->n > 0 && conf->m > 0 && conf->filesCount != 0)
        return 1;
    return 0;
}

char *getBinPath(const char *arg0)
{
    char *res;
    allocWrapper(MAX_PATH_LEN, sizeof(char), (void **)&res);
    strcpy(res, arg0);
    int i;
    for (i = strlen(res) - 2; i > 0 && res[i] != '/'; i--)
        ;
    res[i + 1] = '\0';
    return res;
}

void passToReport(char *command)
{
    if (!isReportRunning)
    {
        //Wait, that's illegal.
        printf("\n Report is not running. \n\n");
        showHelp();
        return;
    }
    fdToReport = open(mainToReportPipe, O_WRONLY);
    if (fdToReport == -1)
    {
        fatalErrorHandler("Impossible to open pipe between main and report. Quit.", 1);
    }
    //printf("main connected to report\n");
    //printf("command from main: %s", command);
    write(fdToReport, command, strlen(command) + 1);
    close(fdToReport);
    if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0 || strcmp(command, "q") == 0)
        isReportRunning = 0;
}

int run(config *conf)
{
    printf("Run analyzer and Report\n");

    if (!isReadyToRun(conf))
    {
        printf("Cannot run, set n, m and at least 1 file.\n");
        return 0;
    }
    analyzerPid = createChild();
    if (analyzerPid == 0)
    { //figlio
        char *analyzerPath;
        allocWrapper(MAX_PATH_LEN, sizeof(char), (void **)&analyzerPath);
        strcpy(analyzerPath, path);
        strcat(analyzerPath, "analyzer");
        char **args = exportAsArguments(conf, analyzerPath);
        execv(analyzerPath, args);
        analyzerPid = -1;
        exit(0);
    }
    //padre
    if (!isReportRunning)
    {
        isReportRunning = 1;
        mkfifo(mainToReportPipe, 0666);
        if (createChild() == 0)
        { //figlio
            char *reportPath;
            allocWrapper(MAX_PATH_LEN, sizeof(char), (void **)&reportPath);
            strcpy(reportPath, path);
            strcat(reportPath, "report");
            execl(reportPath, reportPath, "--main", NULL);
            exit(0);
        }
    }
    else
    {
        passToReport("read");
    }
    return 1;
}

void addFiles(const char *arguments, config **conf)
{
    char *input;
    allocWrapper(MAX_COMMAND_LEN, sizeof(char), (void **)&input);
    char *buffer;
    allocWrapper(MAX_PATH_LEN, sizeof(char), (void **)&buffer);
    strcpy(input, arguments);
    buffer = splitStringWithQuotes(buffer, &input, ' ');
    while (buffer != NULL)
    {
        printf("%s.\n", buffer);
        addFileToConfig(*conf, buffer);
        buffer = splitStringWithQuotes(buffer, &input, ' ');
    }
    *conf = checkDirectories(*conf);
}

void showHelp()
{
    printf("\nMain allowed actions:\n\n");
    printf("   set <n> <m>                                        - Set values of n (number of p processes) \n");                                                        
    printf("                                                        and m (number of q processes) to run analyzer.\n");
    printf("   add <file or direcotry> <file or direcotry> ...    - Add files (or directories) to be analyzed on the next run.\n");
    printf("   config                                             - Show the configuration of the next run\n");
    printf("   run                                                - Run analyzer and report\n");
    printf("   r report <cmd>                                     - Used to issue commands to report, must be used after run.\n");
    printf("                                                        Use \"report help\" for report help\n");
    printf("   remove <file or direcotry> <file or direcotry> ... - Removes files (or directories) from the list to be analyzed\n");
    printf("   q / quit / exit                                    - Close the program\n");
    printf("\n   WARNING: if path contains files (directories) names with spaces, please add double quotes to the entire path\n");
    printf("\n");
}

void showCommandNotFoundError(const char *arguments)
{
    if (arguments[0] != '\n')
        printf("Command %s is not valid, see usage with help\n", arguments);
}

void set(char *arguments, config *conf)
{
    char *endptr;
    char *pn = strtok(arguments, " ");
    char *pm = strtok(NULL, " ");
    if (pn != NULL && pm != NULL)
    {
        int n1 = strtol(pn, &endptr, 10);
        int m1 = strtol(pm, &endptr, 10);
        if (n1 > 0 && m1 > 0)
        {
            conf->n = n1;
            conf->m = m1;
        }
        else
        {
            printf("Command \"set %s\" is not valid, see usage with help\n", arguments);
        }
    }
    else
    {
        showHelp();
    }
}

void showConfig(config *conf)
{
    if (conf->n > 0 && conf->m > 0)
    {
        printf("Ready to run with n = %d, m = %d\n", conf->n, conf->m);
        printFiles(conf);
    }
    else
    {
        printf("Set n and m with set <n> <m>\n");
        printFiles(conf);
    }
}

void removeFiles(char *arguments, config *conf)
{
    char *buffer;
    allocWrapper(MAX_PATH_LEN, sizeof(char), (void **)&buffer);
    buffer = splitStringWithQuotes(buffer, &arguments, ' ');
    while (buffer != NULL)
    {
        printf("toRemove : %s.\n", buffer);
        removePathFromConfig(conf, buffer);
        buffer = splitStringWithQuotes(buffer, &arguments, ' ');
    }
}

int getAction(char *command, config **newConf, config **analyzedConf)
{
    char *token = strtok(command, " ");
    if (token == NULL)
        return 1;

    if (strcmp(token, "run") == 0)
    {
        if (run(*newConf))
        {
            int n = (*newConf)->n;
            int m = (*newConf)->m;
            joinConfigs(*analyzedConf, *newConf);
            initConfig(*newConf);
            (*newConf)->n = n;
            (*newConf)->m = m;
        }
    }
    else if (strcmp(token, "add") == 0)
    {
        addFiles(command + 4, newConf);
    }
    else if (strcmp(token, "set") == 0)
    {
        set(command + 4, *newConf);
    }
    else if (strcmp(token, "report") == 0)
    {
        passToReport(command + 7);
    }
    else if (strcmp(token, "r") == 0)
    {
        passToReport(command + 2);
    }
    else if (strcmp(token, "help") == 0)
    {
        showHelp();
    }
    else if (strcmp(token, "config") == 0 || strcmp(token, "c") == 0)
    {
        showConfig(*newConf);
    }
    else if (strcmp(token, "remove") == 0)
    {
        removeFiles(command + 7, *newConf);
    }
    else if (strcmp(token, "exit") == 0 || strcmp(token, "quit") == 0 || strcmp(token, "q") == 0)
    {
        return 0;
    }
    else
    {
        showCommandNotFoundError(token);
    }
    return 1;
}

int checkArguments(int argc, const char *argv[])
{
    if (argc < 4)
    {
        printf("Wrong arguments, usage is: main <n> <m> <file1> <file2> ... \n");
        return 1;
    }
    return 0;
}

int main(int argc, const char *argv[])
{
    initGC();
    initProcess();
    int i = 0;
    path = getBinPath(argv[0]);
    char *endptr;
    config initConf;

    initConfig(&initConf);

    if (argc > 1)
        if (checkArguments(argc, argv) != 0)
            fatalErrorHandler("", 1);
    int filesCount = argc - PRE_FILES_ARGS;
    if (argc >= 3)
    {
        initConf.n = strtol(argv[1], &endptr, 10);
        initConf.m = strtol(argv[2], &endptr, 10);
        for (i = 0; i < filesCount; i++)
        {
            addFileToConfig(&initConf, argv[i + PRE_FILES_ARGS]);
        }
    }

    config *analyzedFilesConf;
    config *newFilesConf;
    newFilesConf = checkDirectories(&initConf);
    allocWrapper(1, sizeof(config), (void **)&analyzedFilesConf);
    initConfig(analyzedFilesConf);

    int action;
    printf("Welcome! Please use \"help\" for instructions\n");
    do
    {
        printf("-> ");
        char *command = getLine();
        action = getAction(command, &newFilesConf, &analyzedFilesConf);
    } while (action);
    fatalErrorHandler("Quit.", 0);
    return 0;
}