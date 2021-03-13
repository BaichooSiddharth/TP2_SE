#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <check.h>
#include "../src/main.h"
#include "./check_utils.h"
#include "./call_by_string.h"

#define COERCE_STATIC_STR(str) ({char *line = malloc(sizeof(char)*(strlen(str)+1)); for(int i=0; i<=strlen(str); i++) line[i] = str[i]; line;})

DEFINE_TEST(test_blocking_q_take_1)
    {
        blocking_q q;
        bool result = blocking_q_init(&q);
        ck_assert(result);

        task task;
        task.start = task.end = task.type = 'A';


        blocking_q_put(&q, &task);

        ck_assert_int_eq(1, q.sz);

        task_ptr r = __blocking_q_take(&q);

        ck_assert_int_eq(0, q.sz);

        ck_assert_int_eq('A', r->type);
        ck_assert_int_eq('A', r->start);
        ck_assert_int_eq('A', r->end);
    }
END_TEST

DEFINE_TEST(test_blocking_q_take_2)
    {
        blocking_q q;
        bool result = blocking_q_init(&q);
        ck_assert(result);

        task task;
        task.start = task.end = task.type = 'A';


        blocking_q_put(&q, &task);
        blocking_q_put(&q, &task);

        ck_assert_int_eq(2, q.sz);

        task_ptr r = __blocking_q_take(&q);

        ck_assert_int_eq(1, q.sz);

        ck_assert_int_eq('A', r->type);
        ck_assert_int_eq('A', r->start);
        ck_assert_int_eq('A', r->end);
    }
END_TEST


DEFINE_TEST(test_blocking_q_init_1)
    {
        // basic functionality
        blocking_q q;
        bool result = blocking_q_init(&q);
        ck_assert(result);
    }
END_TEST

DEFINE_TEST(test_blocking_q_init_2)
    {
        blocking_q q;
        bool result = blocking_q_init(&q);
        ck_assert(result);
        ck_assert_int_eq(0, q.sz);
    }
END_TEST

DEFINE_TEST(test_blocking_q_init_3)
    {
        blocking_q q;
        bool result = blocking_q_init(&q);
        ck_assert(result);
        ck_assert(NULL == q.first);
    }
END_TEST

DEFINE_TEST(test_blocking_q_destroy_1)
    {
        blocking_q q;
        bool result = blocking_q_init(&q);
        ck_assert(result);

        task task;
        task.start = task.start = task.type = 'A';

        blocking_q_node *node = malloc(sizeof(blocking_q_node));
        node->next = NULL;
        node->data = &task;
        q.first = node;
        q.sz = 1;

        blocking_q_destroy(&q); // if it frees an element, will crash all over

        ck_assert_int_eq('A', task.type);

        free(node)
    }
END_TEST

#define BQ_INIT blocking_q q; bool result = blocking_q_init(&q); ck_assert(result);

DEFINE_TEST(test_blocking_q_put_1)
    {
        BQ_INIT

        ck_assert_int_eq(0, q.sz);

        task tasks[100];
        for (int i = 0; i < 100; ++i) {
            tasks[i].type = (('A' + i) % 4);
            blocking_q_put(&q, tasks + i);
        }

        ck_assert_int_eq(100, q.sz);
    }
END_TEST

blocking_q global_q_1;
volatile char global_result_1;

void *read_thread() {
    task_ptr result = blocking_q_get(&global_q_1);
    global_result_1 = result->type;
}

DEFINE_TEST(test_blocking_q_put_2)
    {
        ck_assert(blocking_q_init(&global_q_1));

        pthread_t read;

        global_result_1 = 'Z';
        pthread_create(&read, NULL, read_thread, NULL);

        int loops = 0;
        while ('Z' == global_result_1) {
            task t;
            t.type = 'A';
            sleep(1);
            blocking_q_put(&global_q_1, &t);
            sleep(1);
            loops++;
        }

        pthread_join(read, NULL);

        ck_assert_int_eq('A', global_result_1);
        ck_assert_int_eq(0, global_q_1.sz);
        ck_assert_int_eq(1, loops);

        blocking_q_destroy(&global_q_1);
    }
END_TEST

DEFINE_TEST(test_blocking_q_drain_1)
    {
        BQ_INIT
        task_ptr tasks[100];
        size_t count = blocking_q_drain(&q, tasks, 100);
        ck_assert_int_eq(0, count);
    }
END_TEST

DEFINE_TEST(test_blocking_q_drain_2)
    {
        BQ_INIT
        task_ptr tasks[100];

        task t;
        blocking_q_put(&q, &t);

        size_t count = blocking_q_drain(&q, tasks, 100);
        ck_assert_int_eq(1, count);
    }
END_TEST

DEFINE_TEST(test_blocking_q_drain_3)
    {
        BQ_INIT
        task_ptr tasks[100];

        task t;
        t.type = 'A';
        blocking_q_put(&q, &t);

        task tt;
        tt.type = 'B';
        blocking_q_put(&q, &tt);

        size_t count = blocking_q_drain(&q, tasks, 100);
        ck_assert_int_eq(2, count);

        ck_assert_int_eq(tasks[0]->type, 'A');
        ck_assert_int_eq(tasks[1]->type, 'B');
    }
END_TEST

//blocking_q global_q_1;
//volatile char global_result_1;

volatile bool done_drain_at_least;

void *drain_at_least() {
    task_ptr tasks[100];

    ck_assert_int_eq(5, blocking_q_drain_at_least(&global_q_1, tasks, 100, 5));

    ck_assert_int_eq('A', tasks[0]->type);
    ck_assert_int_eq('B', tasks[1]->type);
    ck_assert_int_eq('C', tasks[2]->type);
    ck_assert_int_eq('D', tasks[3]->type);
    ck_assert_int_eq('A', tasks[4]->type);

    done_drain_at_least = true;

    return NULL;
}

DEFINE_TEST(test_blocking_q_drain_at_least_1)
    {
        ck_assert(blocking_q_init(&global_q_1));

        task tasks[10];
        tasks[0].type = 'A';
        tasks[1].type = 'B';
        tasks[2].type = 'C';
        tasks[3].type = 'D';
        tasks[4].type = 'A';

        pthread_t read;

        pthread_create(&read, NULL, drain_at_least, NULL);

        for (int i = 0; i < 5; ++i) {
            blocking_q_put(&global_q_1, &tasks[i]);
        }

        pthread_join(read, NULL);

        ck_assert(done_drain_at_least);

        blocking_q_destroy(&global_q_1);
    }
END_TEST

//blocking_q global_q_1;
//volatile char global_result_1;

DEFINE_TEST(test_blocking_q_peek_1)
    {

        BQ_INIT
        task_ptr p;
        ck_assert(!blocking_q_peek(&q, &p));

    }
END_TEST

DEFINE_TEST(test_blocking_q_peek_2)
    {

        BQ_INIT
        task t;
        t.type = 'A';
        task_ptr p;

        blocking_q_put(&q, &t);
        ck_assert_int_eq(1, q.sz);
        ck_assert(blocking_q_peek(&q, &p));
        ck_assert_int_eq(1, q.sz);
        ck_assert_int_eq('A', p->type);

    }
END_TEST

DEFINE_TEST(test_processor_init_1)
    {
        processor p;
        processor_init(1, &p);
        processor_destroy(&p);
    }
END_TEST

DEFINE_TEST(test_processor_destroy_1)
    {
        // same as init
        processor p;
        processor_init(1, &p);
        processor_destroy(&p);
    }
END_TEST

int main(int argc, char *argv[]) {
    call_by_string(argv[1]);
    exit(-1);
}
