#include <stdio.h>
#include <stdlib.h>
#include "../TP_TD_Racine_nieme_unite_residu_quadratique/exponentiation_rapide.h"
#include "Baby_Steps_Giant_Steps_Shanks.h"


Decomposition_x *decompose_x(const long long x, const int t){
    if(t <= 0){
        return NULL;
    }
    Decomposition_x *decomposition = malloc(sizeof(Decomposition_x));
    if(decomposition == NULL)return NULL;
    decomposition->r = x % t;
    if (decomposition->r<0) decomposition->r +=t;
    decomposition->q = x / t;
    return decomposition;
}

Baby_steps *compute_baby_steps(const Cycle_group *group, const int t) {
    if(group == NULL || t <= 0) return NULL;

    Baby_steps *baby_steps = malloc(sizeof(Baby_steps));
    baby_steps->size = t * 2;
    baby_steps->structure = malloc(baby_steps->size * sizeof(HashEntry));

    for(int k = 0; k < baby_steps->size; k++) {
        baby_steps->structure[k].key = -1;
    }

    long long current_value = 1; 
    for (int i = 0; i < t; i++) {
        long long hash_index = current_value % baby_steps->size;
        while (baby_steps->structure[hash_index].key != -1) {
            hash_index = (hash_index + 1) % baby_steps->size;
        }
        baby_steps->structure[hash_index].key = current_value;
        baby_steps->structure[hash_index].exponent = i;
        current_value = (current_value * group->generator) % group->modulus;
    }
    return baby_steps;
}



long long compute_giant_step(const long long h, const Cycle_group *group, const Baby_steps *baby_steps, const int t) {
    long long gt = exponentiation_rapide(group->generator, t, group->modulus);
    long long inv_gt = inverse_modulaire(gt, group->modulus);

    long long current_gamma = h % group->modulus;
    int max_q = (group->order_group / t) + 1;

    for(int q = 0; q <= max_q; q++) {
        long long hash_index = current_gamma % baby_steps->size;
        while (baby_steps->structure[hash_index].key != -1) {
            if (baby_steps->structure[hash_index].key == current_gamma) {
                long long r = baby_steps->structure[hash_index].exponent;
                return (q * t + r) % group->order_group;
            }
            hash_index = (hash_index + 1) % baby_steps->size;
        }
        current_gamma = (current_gamma * inv_gt) % group->modulus;
    }
    return -1;
}