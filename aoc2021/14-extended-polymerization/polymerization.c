#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define NUM_STEPS 40

typedef struct {
    char *key;
    char value;
} rule_t;

typedef struct {
    char key;
    uint64_t value;
} count_t;

typedef struct node_s {
    char data;
    struct node_s *prev;
    struct node_s *next;
} node_t;

void print_polymer(node_t *start);
count_t *get_element_counts(node_t *start);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Must supply filepath as argument\n");
        exit(1);
    }

    /* rules hashmap */
    rule_t *rules = NULL;
    sh_new_arena(rules);

    char *template = NULL;
    char *linebuf = malloc(256 * sizeof(char));
    size_t n, num_read;

    FILE *fp = fopen(argv[1], "r");

    /* get template */
    num_read = getline(&template, &n, fp);
    template[num_read - 1] = '\0';
    num_read = getline(&linebuf, &n, fp);

    /* get rules */
    while ((num_read = getline(&linebuf, &n, fp)) != -1) {
        linebuf[num_read - 1] = '\0';
        char lhs[3] = { linebuf[0], linebuf[1], '\0' };
        char rhs = linebuf[6];
        shput(rules, lhs, rhs);
    }

    fclose(fp);
    free(linebuf);

    node_t *start = calloc(1, sizeof(node_t));
    start->data = template[0];

    /* build list : just let it allllll leeeaaaakkkk */
    node_t *prev = start;
    size_t l = strlen(template);
    for (int i=1; i<l; ++i) {
        node_t *node = calloc(1, sizeof(node_t));
        node->data = template[i];
        node->prev = prev;
        prev->next = node;
        prev = node;
    }

    free(template);

    /*print_polymer(start);*/

    count_t *counts[NUM_STEPS];

    /* perform insertions */
    for (int i=0; i<NUM_STEPS; ++i) {
        node_t *current = start;

        while (current) {
            if (!current->next)
                break;
            char pair[3] = { current->data, current->next->data, '\0' };
            if (shgeti(rules, pair) != -1) {
                node_t *insert = calloc(1, sizeof(node_t));
                insert->data = shget(rules, pair);
                insert->next = current->next;
                insert->prev = current;
                current->next->prev = insert;
                current->next = insert;
                current = current->next->next;
            } else {
                current = current->next;
            }
        }
        /*print_polymer(start);*/


        count_t *step_counts = get_element_counts(start);
        counts[i] = step_counts;


        if (i == 0) continue;

        uint64_t n_count = hmget(step_counts, 'N');
        double n_growth_rate = (n_count - hmget(counts[i-1], 'N')) / (double)hmget(counts[i-1], 'N');
        printf("%f\n", n_growth_rate);
        printf("step %d: N count=%u growth_rate=%f\n", i, hmget(step_counts, 'N'), n_growth_rate);
    }

    return 0;
}

void print_polymer(node_t *start) {
    while (start) {
        printf("%c", start->data);
        start = start->next;
    }
    printf("\n");
}

count_t *get_element_counts(node_t *start) {
    count_t *counts = NULL;
    hmdefault(counts, 0);

    node_t *current = start;
    while (current) {
        char data = current->data;
        uint64_t count = hmget(counts, data);
        hmput(counts, data, count+1);
        current = current->next;
    }

    return counts;
}





