# M1 Performance Monitoring Unit Reversing Framework
This is a framework for reversing performance events on Apple M1 chips on Asashi Linux.

## Framework Structure
`config.h` : Configuration file of the framework

`main.c` : Reversing functions

`output.c` : Output file contains benchmark result

`m1_pmu_kernel_module`: Kernel module for enabling M1 PMU

`m1_pmu_setup.h`： Userspace interfaces for using M1 PMU

## M1 PMU Architecture
Writes to any control register needs `isb` to take effect.

`PMCR0` : General control register of apple m1 pmu
- [30] : Enable user-mode access to registers 
- [10:8] : Interrupt mode (0=off 1=PMI 2=AIC 3=HALT 4=FIQ)
- [11] : If PMI is active, every time after eret, write 0 to clear this bit so that PMI can be triggered again
- [20] : Disable counting on a PMI
- [22] : Block PMIs until after eret
- [23] : Count global (not just core) L2C events
- [7:0] : Enable PMC 7 - 0
- [33:32] : Enable PMC 9 - 8
- [19:12] : Enable PMI for PMC 7 - 0
- [45:44] : Enable PMI for PMC 9 - 8

`PMCR1` : Controls which ELx modes counts events
- [15:8] : EL0 A64 enable PMC 7 - 0
- [41:40] : EL0 A64 enable PMC 9 - 8
- [23:16] : EL1 A64 enable PMC 7 - 0
- [49:48] : EL1 A64 enable PMC 9 - 8

`PMESR0` : Event selection registers for PMC 2 - 5
- [7:0] : for PMC2
- [15:8] : for PMC3
- [23:16] : for PMC4
- [31:24] : for PMC5

`PMESR0` : Event selection registers for PMC 6 - 9   
- [7:0] : for PMC6
- [15:8] : for PMC7
- [23:16] : for PMC8
- [31:24] : for PMC9

`PMC0-9`: 48 bits performance counters, 47 bit triggers overflow
- PMC0 : Fix conuters for cpu cycles
- PMC1 : Fix counters for cpu isntructions
- PMC2-9 : Configurable counters depends on event id set be user, some event id only work on part of the counters

More information about Apple M1 architecture can be found in [Asahi Linux Wiki](https://github.com/AsahiLinux/docs/wiki/).

## M1 PMU Userspace Interfaces
M1 PMU userspace interfaces can only be used after inserting the prepared kernel module, which enables the userspace access and setup some general configurations of M1 PMU.

Once the event id is set and the corresponding counters are enabled, the counters wil automatically count how many times a event has occured. User can also zero any counter during any time to re-count the event.

PMI is the machanism to do sampling. When PMI is set, if there is an overflow detected in the counter, an interrupt will be triggered, user can then check current system states. For example, get system states after every 1024 instructions or cycles. On Intel CPU, the threshold for the overflow can usually be set by programmer. However, M1 seems doesn't implement this feature, the overflow bit of M1 counters is always bit 47. So, sampling mode on M1 can not be achieved.

## Methodology
Our framework only supplies a coarse-grained way to reverse performance events. Usually user can find out which event id corresponds to which instruction, intead of the specific event.

The general idea is to construct a small piece of code for certain event, run it millions of times, and see if there are any event id whose value is approximately equal to the number of time the code has been run. The PMU may not be accurate, so an appropriate deviation needs to be set.

There may be noises exist in the test code, which is not the event we want to find out but also vary depends on the number of test time. There are 2 ways to exclude noises in our framework:
- Use inline assembly as test code, try to make it as simple as possible.
- Run another fake test code before the real one. What the fake code does is that do almost the same thing as the real one except triggering the actual event we want to test. For example, exclude the specific instruction which triggers the target event and keep the remain part completely same in the fake code. Within the fake code, every event id has a high number of occurance is considered as a noise, needs to be excluded.

## M1 Performance Events
Thanks for previous work from [Dougall Johnson](https://github.com/dougallj/applecpu/blob/main/timer-hacks/bench.py).
| Event Name | Event ID | Counters |
| ---- | ---- | ---- |
| retired_uops | 0x01 | |
| cycle | 0x02 | |
| MMU_miss | 0x0d | |
| instruction_issue | 0x52 | |
| uops_int | 0x53 | |
| uops_float_point | 0x54 | |
| uops_load_and_store | 0x55 | |
| interrupt_pending | 0x6c | |
| dispatch_stall | 0x70 | |
| scheduler_rewind | 0x75 | |
| scheduler_stall | 0x76 | |
| pipeline_redirect | 0x84 | |
| instruction_all | 0x8c | |
| instruction_branch | 0x8d | | 
| instruction_function_call | 0x8e | |
| instruction_function_return | 0x8f | |
| instruction_all_branch_taken | 0x90 | |
| instruction_branch_carry_is_clear | 0x94 | |
| instruction_int_load | 0x95 | |
| instruction_int_store | 0x96 | |
| instruction_int | 0x97 | |
| instruction_float_point_load | 0x98 | |
| instruction_float_point_store | 0x99 | |
| isntruction_neon_or_float_point | 0x9a | |
| instruction_load_and_store | 0x9b | |
| instruction_barrier | 0x9c | |
| atomic_or_exclusive_success | 0xb3| |
| atomic_or_exclusive_fail | 0xb4 | |
| DCACHE_load_miss | 0xbf | |
| DCACHE_store_miss | 0xc0 | |
| DTLB_miss | 0xc1 | |
| memory_order_violation | 0xc4 | |
| conditional_branch_misprediction | 0xc5 | |
| indirect_branch_misprediction | 0xc6, 0xca | |
| branch_misprediction | 0xcb | | 
| ICACHE_miss | 0xd3 | |
| ITLB_miss | 0xd4 | |
| instruction_fetch_restart | 0xde | |


| Event Name | Event ID | Counters |
| ---- | ---- | ---- |
| int_unit_uops | 0x53 | |
| float_point_unit_uops | 0x54 | |
| load_and_store_unit_uops | 0x55 | |
| int_unknown_uops | 0x56 | |
| float_point_unknown_uops | 0x57 | |
| load_and_store_unknown_uops | 0x58 | |
| unknown_int_count | 0x59 | |
| unknown_load_and_store_count | 0x5a | |
| unknown_float_point_count | 0x5b | |
| unknown_fused_uops | 0x78 | |
| int_unknown_unfused_uops | 0x7c | |
| float_point_unknown_unfused_uops | 0x7e | |
| load_and_store_unknown_unfused_uops | 0x7d | |
| int_unknown_deps | 0x7f | |
| load_and_store_unknown_deps | 0x80 | |
| float_point_unknown_deps | 0x81 | |
| int_unknown | 0xe9 | |
| float_point_unknown | 0xee | |
| load_and_store_unknown | 0xed | |
| int_writes_unknown | 0xef | |
