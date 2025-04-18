#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define VERSION "1.0.0"

#ifdef _WIN32
#define UP_ARROW 72
#define DOWN_ARROW 80
#define ENTER_KEY 13
int getKeyPressWindows() { return _getch(); }
#else
#define UP_ARROW 65
#define DOWN_ARROW 66
#define ENTER_KEY 10
int getKeyPressLinux() {
    int c;
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    c = getchar();

    if (c == 27) {
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

int main(int argc, char *argv[]) {

    if (argc > 1)
    {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
        {
            printf("flutter-create version %s\n", VERSION);
            return 0;
        }
    }

    char projectName[100];
    char packageName[100] = "com.example";
    int selectedTemplate = 0;
    char *templates[] = {"app", "module", "package", "plugin", "plugin_ffi"};
    char *templatesDesc[] = {
        "app (basic Flutter app)",
        "module (Flutter module for existing Android/iOS app)",
        "package (shareable Flutter project with modular Dart code)",
        "plugin (Flutter project with platform-specific API via method channels)",
        "plugin_ffi (Flutter project with platform-specific API via dart:ffi)"
    };
    int numTemplates = sizeof(templates) / sizeof(templates[0]);

    printf("Project name: ");
    scanf("%99s", projectName);
    getchar();
    printf("Package name [default: com.example]: ");
    char userInput[100];
    fgets(userInput, sizeof(userInput), stdin);
    if (userInput[0] != '\n') {
        sscanf(userInput, "%99s", packageName);
    }

    printf("\nTemplate:\n");
    while (1) {
        for (int i = 0; i < numTemplates; i++) {
            printf(i == selectedTemplate ? "\033[1;32m[X] %s\033[0m\n" : "[ ] %s\n", templatesDesc[i]);
        }
        int key = getKeyPress();
        if (key == 27) {
            getKeyPress();
            key = getKeyPress();
        }
        if (key == UP_ARROW && selectedTemplate > 0) selectedTemplate--;
        else if (key == DOWN_ARROW && selectedTemplate < numTemplates - 1) selectedTemplate++;
        else if (key == ENTER_KEY) break;

        printf("\033[H\033[J\nTemplate:\n");
    }

    char platforms[100] = "";
    if (strcmp(templates[selectedTemplate], "plugin") == 0 || strcmp(templates[selectedTemplate], "plugin_ffi") == 0) {
        printf("Specify platforms (comma separated, e.g., android,ios,linux,macos,web): ");
        fgets(platforms, sizeof(platforms), stdin);
        platforms[strcspn(platforms, "\n")] = 0;
    }

    printf("Upgrading Flutter...\n\n");
    if (system("flutter upgrade") != 0) {
        printf("Error during Flutter upgrade.\n\n");
        return 1;
    }

    char command[500];
    snprintf(command, sizeof(command), "flutter create --template=%s --org %s %s %s",
             templates[selectedTemplate], packageName, projectName,
             (platforms[0] != '\0') ? "--platforms=" : "");
    if (platforms[0] != '\0') strncat(command, platforms, sizeof(command) - strlen(command) - 1);

    printf("Creating Flutter project: %s with template %s...\n", projectName, templates[selectedTemplate]);
    if (system(command) != 0) {
        printf("Error during project creation.\n");
        return 1;
    }
    printf("Project %s created successfully with template %s!\n", projectName, templates[selectedTemplate]);

    if (system("code --version >nul 2>&1") == 0) {
        char choice;
        printf("Do you want to open the project with VS Code? (y/n): ");
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y') {
            char openCommand[200];
            snprintf(openCommand, sizeof(openCommand), "code %s", projectName);
            system(openCommand);
        }
    }
    return 0;
}
