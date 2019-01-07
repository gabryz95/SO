project(Operative_SISTEM C)

set(CMAKE_C_STANDARD 11)

add_executable(bthread.h bthread.c bthread_private.h tqueue.h tqueue.c tqueue_test.h tqueue_test.c scheduler.h scheduler.c
        tbarrier.h tbarrier.c tcondition.h tcondition.c tmutex.h tmutex.c tsemaphore.h tsemaphore.c test/tqueue_test_executable.c
        executable_files/philosopher.c executable_files/philosophers_try.c executable_files/producer_consumer.c
        executable_files/producer_consumer_cond.c executable_files/readers_writers_1.c executable_files/readers_writers_2.c
        executable_files/readers_writers_3.c executable_files/sleeping_barber.c executable_files/matrix_cube.c executable_files/scheduling_test.c)