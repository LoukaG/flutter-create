#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define VERSION "1.1.0"

#ifdef _WIN32
#define UP_ARROW 72
#define DOWN_ARROW 80
#define ENTER_KEY 13
int getKeyPressWindows() { return _getch(); }
#else
#define UP_ARROW 65
#define DOWN_ARROW 66
#define ENTER_KEY 10
int getKeyPressLinux()
{
    int c;
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    c = getchar();

    if (c == 27)
    {
        getchar();
        c = getchar();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}

#endif

#ifdef _WIN32
#define getKeyPress getKeyPressWindows
#else
#define getKeyPress getKeyPressLinux
#endif

bool handleVersionFlag(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
        {
            printf("flutter-create version %s\n", VERSION);
            return true;
        }
    }
    return false;
}

void getProjectInfo(char *projectName, char *packageName)
{
    char userInput[100];

    printf("Project name: ");
    scanf("%99s", projectName);
    getchar();

    printf("Package name [default: com.example]: ");
    fgets(userInput, sizeof(userInput), stdin);

    if (userInput[0] != '\n')
    {
        sscanf(userInput, "%99s", packageName);
    }
}

void selectTemplate(char *templateOut)
{
    const char *templates[] = {
        "app",
        "module",
        "package",
        "plugin",
        "plugin_ffi"};

    const char *templatesDesc[] = {
        "app (basic Flutter app)",
        "module (Flutter module for existing Android/iOS app)",
        "package (shareable Flutter project with modular Dart code)",
        "plugin (Flutter project with platform-specific API via method channels)",
        "plugin_ffi (Flutter project with platform-specific API via dart:ffi)"};

    int numTemplates = sizeof(templates) / sizeof(templates[0]);
    int selected = 0;

    printf("\nSelect a template:\n");

    while (1)
    {
        for (int i = 0; i < numTemplates; i++)
        {
            printf(i == selected ? "\033[1;32m[X] %s\033[0m\n" : "[ ] %s\n", templatesDesc[i]);
        }

        int key = getKeyPress();

        if (key == 27)
        {
            getKeyPress();
            key = getKeyPress();
        }

        if (key == UP_ARROW && selected > 0)
            selected--;
        else if (key == DOWN_ARROW && selected < numTemplates - 1)
            selected++;
        else if (key == ENTER_KEY)
            break;

        printf("\033[H\033[J\nSelect a template:\n");
    }

    strcpy(templateOut, templates[selected]);
}

void selectStructure(char *structureOut)
{
    const char *structureTypes[] = {
        "none",
        "default",
        "features",
        "clean_architecture",
        "atomic_design"};

    const char *structureDesc[] = {
        "none (no folder creation)",
        "default (screens, widgets, models...)",
        "features (modular per feature)",
        "clean_architecture (domain/data/presentation)",
        "atomic_design (ui/atoms, molecules, organisms)"};

    int numStructures = sizeof(structureTypes) / sizeof(structureTypes[0]);
    int selected = 0;

    printf("\nSelect project structure type:\n");

    while (1)
    {
        for (int i = 0; i < numStructures; i++)
        {
            printf(i == selected ? "\033[1;32m[X] %s\033[0m\n" : "[ ] %s\n", structureDesc[i]);
        }

        int key = getKeyPress();

        if (key == 27)
        {
            getKeyPress();
            key = getKeyPress();
        }

        if (key == UP_ARROW && selected > 0)
            selected--;
        else if (key == DOWN_ARROW && selected < numStructures - 1)
            selected++;
        else if (key == ENTER_KEY)
            break;

        printf("\033[H\033[J\nSelect project structure type:\n");
    }

    strcpy(structureOut, structureTypes[selected]);
}

void askForPlatforms(char *platformsOut)
{
    printf("Specify platforms (comma separated, e.g., android,ios,linux,macos,web): ");
    fgets(platformsOut, 100, stdin);
    platformsOut[strcspn(platformsOut, "\n")] = '\0';
}

bool upgradeFlutter()
{
    printf("Upgrading Flutter...\n\n");
    int result = system("flutter upgrade");

    if (result != 0)
    {
        printf("Error during Flutter upgrade.\n\n");
        return false;
    }

    return true;
}

bool createFlutterProject(const char *name, const char *pkg, const char *templateName, const char *platforms)
{
    char command[500];

    if (platforms[0] != '\0')
    {
        snprintf(command, sizeof(command),
                 "flutter create --template=%s --org %s %s --platforms=%s",
                 templateName, pkg, name, platforms);
    }
    else
    {
        snprintf(command, sizeof(command),
                 "flutter create --template=%s --org %s %s",
                 templateName, pkg, name);
    }

    printf("Creating Flutter project: %s with template %s...\n", name, templateName);
    int result = system(command);

    if (result != 0)
    {
        printf("Error during project creation.\n");
        return false;
    }

    printf("Project %s created successfully with template %s!\n", name, templateName);
    return true;
}

void createFolder(const char *basePath, const char *subPathRaw)
{
    char subPath[100];
    strncpy(subPath, subPathRaw, sizeof(subPath));
    subPath[sizeof(subPath) - 1] = '\0';
    subPath[strcspn(subPath, "\r\n")] = '\0';

    char fullPath[300];
#ifdef _WIN32
    snprintf(fullPath, sizeof(fullPath), "%s\\%s", basePath, subPath);
    for (int i = 0; fullPath[i]; i++)
    {
        if (fullPath[i] == '/')
            fullPath[i] = '\\';
    }
    char command[500];
    snprintf(command, sizeof(command),
             "powershell -Command \"New-Item -ItemType Directory -Force -Path '%s'\" > NUL 2>&1",
             fullPath);
#else
    snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, subPath);
    char command[400];
    snprintf(command, sizeof(command), "mkdir -p %s > /dev/null 2>&1", fullPath);
#endif

    system(command);
}

void createLibStructure(const char *projectName, const char *structure)
{
    char libPath[150];
    snprintf(libPath, sizeof(libPath), "%s/lib", projectName);

    if (strcmp(structure, "default") == 0)
    {
        const char *folders[] = {"screens", "widgets", "models", "services"};
        for (int i = 0; i < 4; i++)
        {
            createFolder(libPath, folders[i]);
        }
    }
    else if (strcmp(structure, "features") == 0)
    {
        const char *features[] = {"features/home"};
        for (int i = 0; i < 1; i++)
        {
            createFolder(libPath, features[i]);
        }
    }
    else if (strcmp(structure, "clean_architecture") == 0)
    {
        const char *layers[] = {"data", "domain", "presentation"};
        for (int i = 0; i < 3; i++)
        {
            createFolder(libPath, layers[i]);
        }
    }
    else if (strcmp(structure, "atomic_design") == 0)
    {
        const char *ui[] = {"ui/atoms", "ui/molecules", "ui/organisms"};
        for (int i = 0; i < 3; i++)
        {
            createFolder(libPath, ui[i]);
        }
    }
    else
    {
        // structure == "none"
        return;
    }

    printf("Folders for '%s' structure created in %s/lib\n", structure, projectName);
}

void promptToOpenWithVSCode(const char *projectName)
{
    if (system("code --version >nul 2>&1") == 0 || system("code --version >/dev/null 2>&1") == 0)
    {
        char choice;
        printf("Do you want to open the project with VS Code? (y/n): ");
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y')
        {
            char openCommand[200];
            snprintf(openCommand, sizeof(openCommand), "code %s", projectName);
            system(openCommand);
        }
    }
}

int main(int argc, char *argv[])
{
    if (handleVersionFlag(argc, argv))
        return 0;

    char projectName[100];
    char packageName[100] = "com.example";
    char selectedTemplate[30];
    char selectedStructure[30];

    getProjectInfo(projectName, packageName);
    selectTemplate(selectedTemplate);
    selectStructure(selectedStructure);

    char platforms[100] = "";
    if (strcmp(selectedTemplate, "plugin") == 0 || strcmp(selectedTemplate, "plugin_ffi") == 0)
    {
        askForPlatforms(platforms);
    }

    if (!upgradeFlutter())
        return 1;

    if (!createFlutterProject(projectName, packageName, selectedTemplate, platforms))
        return 1;

    createLibStructure(projectName, selectedStructure);

    promptToOpenWithVSCode(projectName);
    return 0;
}