#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

#define TEXT_WIDTH 40

const char *ASCII_CHARS = "@%#*+=-:. ";

int sc_j_c() {
    long jc = sysconf(_SC_JOB_CONTROL);
    if (jc == -1) {
        perror("sysconf");
        printf("\033[34mAn error occurred, maybe _SC_JOB_CONTROL isn't defined?\n");
        return EXIT_FAILURE;
    }

    if (jc == 0) {
        printf("\033[34mJob control isn't supported on this system.\n");
    }

    if (jc == 1) {
        printf("\033[34mJob control is supported on this system!\n");
        return EXIT_SUCCESS;
    }
}

void cpu(void) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        perror("Failed to open /proc/cpuinfo");
        return;
    }

    char line[256];
    char cpu_name[256];

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                strcpy(cpu_name, colon + 2);
                cpu_name[strcspn(cpu_name, "\n")] = 0;
                printf("\033[34m\033[1mCPU\033[0m: %s\n", cpu_name);
            }
            break;
        }
    }

    fclose(fp);
}

void gpu(void) {
    char line[256];
    char gpu_name[256];

    FILE *fp = fopen("/proc/driver/nvidia/gpus/0/information", "r");
    if (!fp) {
        fp = fopen("/sys/class/drm/card0/device/uevent", "r");
        if (!fp) {
            printf("\033[34m\033[1mGPU\033[0m: Unable to get GPU info!\n");
            return;
        }
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                strcpy(gpu_name, colon + 2);
                gpu_name[strcspn(gpu_name, "\n")] = 0;
                printf("\033[34m\033[1mGPU\033[0m: %s\n", gpu_name);
            }
            break;
        }
    }

    fclose(fp);
}

void ram(void) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Failed to open /proc/meminfo");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            unsigned long total_memory;
            if (sscanf(line, "MemTotal: %lu kB", &total_memory) == 1) {
                printf("\033[1mRAM\033[0m: %lu MB\n", total_memory / 1024);
            }
            break;
        }
    }

    fclose(fp);
}

void kernel(void) {
    FILE *fp = fopen("/proc/version", "r");
    if (!fp) {
        printf("Failed to get Linux version!\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char kernel[256];
        if (sscanf(line, "Linux version %99s", kernel)) {
            printf("\033[1mKernel version\033[0m: %s\n", kernel);
        }
    }

    fclose(fp);
}

void uptime(void) {
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) {
        printf("Failed to open /proc/uptime\n");
        return;
    }

    double uptime_seconds;
    double idle_seconds;

    if (fscanf(fp, "%lf %lf", &uptime_seconds, &idle_seconds) != 2) {
        printf("Failed to grab uptime data\n");
        fclose(fp);
        return;
    }

    int hours = uptime_seconds / 3600;
    int minutes = (uptime_seconds - (hours * 3600)) / 60;
    int seconds = (int)uptime_seconds % 60;

    printf("\033[1mSystem Uptime:\033[0m %d hours, %d minutes, %d seconds\n", hours, minutes, seconds);

    fclose(fp);
}

void output(void) {
    printf("\033[1m\033[31mColoured's System info\033[0m\n------------------------ \n");
    sc_j_c();
    cpu();
    gpu();
    ram();
    kernel();
    uptime();
}

int main(void) {
    output();
    return 0;
}
