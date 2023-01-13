#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>

/* Set single system register */
#define set_system_register(SR, V) __set_system_register(SR, V)
#define __set_system_register(SR, V) asm volatile("msr " #SR ", %0;" : : "r"(V))

/* Read single system register */
#define read_system_register(SR, V) __read_system_register(SR, V)
#define __read_system_register(SR, V) asm volatile("mrs %0, " #SR : "=r"(V))

#define PMCR0  s3_1_c15_c0_0
#define PMCR1  s3_1_c15_c1_0

#define PMESR0 s3_1_c15_c5_0
#define PMESR1 s3_1_c15_c6_0

#define PMC0   s3_2_c15_c0_0
#define PMC1   s3_2_c15_c1_0
#define PMC2   s3_2_c15_c2_0
#define PMC3   s3_2_c15_c3_0
#define PMC4   s3_2_c15_c4_0
#define PMC5   s3_2_c15_c5_0
#define PMC6   s3_2_c15_c6_0
#define PMC7   s3_2_c15_c7_0
#define PMC8   s3_2_c15_c9_0
#define PMC9   s3_2_c15_c10_0

/* 
 *  Generate bit mask.
 */
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (64 - 1 - (h))))
#define BIT(nr) (1UL << (nr))

/* 
 *  Store value of 10 performance counters.
 */
uint64_t counters[10];


void isb(){
    asm volatile("isb");
}

void enable_PMI_for_all_counters(){
    uint64_t val;
    read_system_register(PMCR0, val);
    val |= GENMASK(19, 12) | GENMASK(45, 44);
    set_system_register(PMCR0, val);
    isb();
}

void disable_PMI_for_all_counters(){
    uint64_t val;
    read_system_register(PMCR0, val);
    val &= ~(GENMASK(19, 12) | GENMASK(45, 44));
    set_system_register(PMCR0, val);
    isb();
}

/* 
 *  Enable all performance counters by setting corresponding bits of PMCR0 to 1.
 */
void enable_all_counters(){
    uint64_t val;
    read_system_register(PMCR0, val);
    val |= GENMASK(7, 0) | GENMASK(33, 32);
    set_system_register(PMCR0, val);
    isb();
}

/* 
 *  Enable all performance counters by setting corresponding bits of PMCR0 to 0.
 */
void disable_all_counters(){
    uint64_t val;
    read_system_register(PMCR0, val);
    val &= ~(GENMASK(7, 0) | GENMASK(33, 32));
    set_system_register(PMCR0, val);
    isb();
}

/* 
 *  Set event id for all configurable counters through PMESR0 and PMESR1.
 */
void set_event_id(uint64_t event_id){
    uint64_t val = event_id | (event_id << 8) | (event_id << 16) | (event_id << 24);
    set_system_register(PMESR0, val);
    set_system_register(PMESR0, val); 
    isb();
}

/* 
 *  Set all performance counters to 0. 
 */
void clear_all_counters(){
    set_system_register(PMC0, 0);
    set_system_register(PMC1, 0);
    set_system_register(PMC2, 0);
    set_system_register(PMC3, 0);
    set_system_register(PMC4, 0);
    set_system_register(PMC5, 0);
    set_system_register(PMC6, 0);
    set_system_register(PMC7, 0);
    set_system_register(PMC8, 0);
    set_system_register(PMC9, 0);
}

/* 
 *   Update all counters.
 */
void read_all_counters(){
    read_system_register(PMC0, counters[0]);
    read_system_register(PMC1, counters[1]);
    read_system_register(PMC2, counters[2]);
    read_system_register(PMC3, counters[3]);
    read_system_register(PMC4, counters[4]);
    read_system_register(PMC5, counters[5]);
    read_system_register(PMC6, counters[6]);
    read_system_register(PMC7, counters[7]);
    read_system_register(PMC8, counters[8]);
    read_system_register(PMC9, counters[9]);
}

/* 
 *  Print all counters.
 */
void print_all_counters(){
    read_all_counters();
    for(int i=0; i<10; i++){
        printf("PMC%c = %lld\n", i+'0', counters[i]); /* i+'0': convert i to char */
    }
}