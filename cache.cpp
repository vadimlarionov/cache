//#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <algorithm>
#include <vector>

#define KB 1024
#define MB (1024*1024)

#define FOUR(x) x x x x
#define x64(x) FOUR(FOUR(FOUR(x;)))
#define x256(x) FOUR(FOUR(FOUR(FOUR(x;))))

#define NUM_ITER (1 << 10)
#define NUM_CYCLES 100

#define BUF_INIT_SIZE (1*KB)
#define BUF_FINAL_SIZE (10*MB)
#define BUF_MULTIPLIER 1.1

#define NPAD 7

struct node {
    struct node *next;
    long pad[NPAD];
};

struct timespec timeSub(struct timespec a, struct timespec b) {
    struct timespec result;
    result.tv_sec = a.tv_sec - b.tv_sec;
    result.tv_nsec = a.tv_nsec - b.tv_nsec;
    if (result.tv_nsec < 0) {
        --result.tv_sec;
        result.tv_nsec += 1e9;
    }
    return result;
}

void bindList(struct node *elems, std::vector<size_t> indexes, size_t size) {
    auto from_index = indexes.begin();
    auto to_index = from_index + 1;
    for (;to_index != indexes.end(); from_index++, to_index++) {
        elems[*from_index].next = &elems[*to_index];
    }
    elems[*from_index].next = &elems[*indexes.begin()];
}

void consequenceBindList(struct node *array, size_t size) {
    for (size_t i = 0; i < size; ++i)
        array[i].next = array + (i + 1) % size;
}

double getAccessTime(struct node* array) {
    struct timespec t0, t1, t_diff;
    struct node *current = array;
    
    clock_gettime(CLOCK_REALTIME, &t0);
    for (size_t iter = 0; iter < NUM_ITER; ++iter) {
        x64(current = current->next;);
    }

    clock_gettime(CLOCK_REALTIME, &t1);
    t_diff = timeSub(t1, t0);

    uint64_t nsec = t_diff.tv_sec*1e9 + t_diff.tv_nsec;
    return (double)(nsec)/(NUM_ITER * 64);
}

double getAverageAccessTime(struct node* array) {
    double avg_time = 0.0;
    for (size_t i = 0; i < NUM_CYCLES; ++i) {
        avg_time += getAccessTime(array);
    }
    return avg_time / NUM_CYCLES;
}


int main() {
	for (short is_random = 0; is_random < 2; ++is_random) {
		for (uint64_t buf_size = BUF_INIT_SIZE; buf_size < BUF_FINAL_SIZE; buf_size *= BUF_MULTIPLIER) {
	        size_t num = buf_size / sizeof(struct node);
	        struct node *array = new struct node[num];

	        if (is_random) {
	            std::vector<size_t> indexes;
	            for (size_t i = 0; i < num; ++i)
	                indexes.push_back(i);

	            random_shuffle(indexes.begin(), indexes.end());
	            bindList(array, indexes, num);
	        }
	        else
	            consequenceBindList(array, num);

	        double avg_time = getAverageAccessTime(array);

	        uint64_t integer_part = avg_time;
	        uint64_t fractional_part = (avg_time - integer_part) * 1e3;

	        printf("%ld %lu %lu,%lu\n",
	            num * sizeof(struct node),
	            num,
	            integer_part,
	            fractional_part
	        );

	        delete[] array;
    	}
    	if (!is_random)
    		printf("\n\n\nRandom Access\n");
	}

    return 0;
}