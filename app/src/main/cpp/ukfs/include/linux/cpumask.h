#ifndef _UK_LINUX_CPUMASK_H
#define _UK_LINUX_CPUMASK_H
#include <linux/types.h>
#define num_online_cpus() 1
#define num_possible_cpus() 1
#define nr_cpu_ids 1
#define raw_smp_processor_id() 0
#define smp_processor_id() 0
#define for_each_possible_cpu(cpu) for ((cpu) = 0; (cpu) < 1; (cpu)++)
#define for_each_online_cpu(cpu) for ((cpu) = 0; (cpu) < 1; (cpu)++)
#endif
