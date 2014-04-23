#include <GLFW/glfw3.h>
#include <uv.h>

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef HAVE_KQUEUE
#if defined(__APPLE__) || defined(__DragonFly__) || defined(__FreeBSD__) ||                        \
    defined(__OpenBSD__) || defined(__NetBSD__)
#define HAVE_KQUEUE 1
#endif
#endif

#ifndef HAVE_EPOLL
#if defined(__linux__)
#define HAVE_EPOLL 1
#endif
#endif

#if defined(HAVE_KQUEUE) || defined(HAVE_EPOLL)

#if defined(HAVE_KQUEUE)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

#if defined(HAVE_EPOLL)
#include <sys/epoll.h>
#endif

#endif

uv_thread_t embed_thread;
uv_sem_t embed_sem;
uv_timer_t embed_timer;
uv_async_t embed_async;
volatile int embed_terminate = 0;

void embed_thread_runner(void *arg) {
    int r;
    int fd;
    int timeout;

    while (!embed_terminate) {
        fd = uv_backend_fd(uv_default_loop());
        timeout = uv_backend_timeout(uv_default_loop());

        do {
#if defined(HAVE_KQUEUE)
            struct timespec ts;
            ts.tv_sec = timeout / 1000;
            ts.tv_nsec = (timeout % 1000) * 1000000;
            r = kevent(fd, NULL, 0, NULL, 0, &ts);
#elif defined(HAVE_EPOLL)

            struct epoll_event ev;
            r = epoll_wait(fd, &ev, 1, timeout);
#endif
        } while (r == -1 && errno == EINTR);

        glfwPostEmptyEvent();

        uv_sem_wait(&embed_sem);
    }
}

void embed_timer_cb(uv_timer_t *timer) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    fprintf(stderr, "%f: timer\n", (double)tv.tv_sec * 1000 + (double)tv.tv_usec / 1000);
}

void key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
        glfwSetWindowShouldClose(window, 1);
    }
}

int main(void) {
    /* Start timer in default loop */
    uv_timer_init(uv_default_loop(), &embed_timer);
    uv_timer_start(&embed_timer, embed_timer_cb, 250, 0);

    /* Start worker that will interrupt external loop */
    uv_sem_init(&embed_sem, 0);
    uv_thread_create(&embed_thread, embed_thread_runner, NULL);

    /* Run GLFW loop */
    {
        GLFWwindow *window;

        if (!glfwInit()) {
            return -1;
        }

        window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return -1;
        }

        glfwSetKeyCallback(window, key);

        glfwMakeContextCurrent(window);

        srand(time(NULL));

        while (!glfwWindowShouldClose(window)) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            fprintf(stderr, "%f: render\n", (double)tv.tv_sec * 1000 + (double)tv.tv_usec / 1000);

            glClearColor((double)rand() / RAND_MAX, (double)rand() / RAND_MAX,
                         (double)rand() / RAND_MAX, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window);

            glfwWaitEvents();

            uv_run(uv_default_loop(), UV_RUN_NOWAIT);
            uv_sem_post(&embed_sem);
        }

        glfwTerminate();
    }

    embed_terminate = 1;
    uv_sem_post(&embed_sem);

    uv_thread_join(&embed_thread);

    return 0;
}
