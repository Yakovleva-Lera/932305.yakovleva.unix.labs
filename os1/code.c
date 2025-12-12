#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

pthread_mutex_t mutexx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int event = 0;
volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
    running = 0;
}

void* provider(void* a) {
    while (running) {
        sleep(1);

        pthread_mutex_lock(&mutexx);
        if (event == 0) {
            event = 1;
            printf("Provider: Event provided\n");
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutexx);
    }

    pthread_mutex_lock(&mutexx);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutexx);

    return NULL;
}

void* client(void* a) {
    while (1) {
        pthread_mutex_lock(&mutexx);
        while (event == 0 && running) {
            pthread_cond_wait(&cond, &mutexx);
        }

        if (!running) {
            pthread_mutex_unlock(&mutexx);
            break;
        }

        printf("Client: Event processed\n");
        event = 0;
        pthread_mutex_unlock(&mutexx);
    }
    return NULL;
}

int main() {
    signal(SIGINT, handle_sigint);

    pthread_t provider_thread, client_thread;
    pthread_create(&provider_thread, NULL, provider, NULL);
    pthread_create(&client_thread, NULL, client, NULL);

    pthread_join(provider_thread, NULL);
    pthread_join(client_thread, NULL);

    pthread_mutex_destroy(&mutexx);
    pthread_cond_destroy(&cond);
    return 0;
}
