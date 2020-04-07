=======

| CSDN | GitHub |
|:----:|:------:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦

<br>


#1    调度器调优参数
-------



##1.1   CFS相关
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) | `/proc/sys/kernel/sched_min_granularity_ns` | `4000000ns` | 表示进程最少运行时间, 防止频繁的切换, 对于交互系统(如桌面), 该值可以设置得较小, 这样可以保证交互得到更快的响应(见周期调度器的 `check_preempt_tick` 过程) |
| [`sysctl_sched_latency`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54) | `/proc/sys/kernel/sched_latency_ns` | `20000000ns` | 表示一个运行队列所有进程运行一次的周期, 当前这个与运行队列的进程数有关, 如果进程数超过 `sched_nr_latency` (这个变量不能通过 `/proc` 设置, 它是由 `(sysctl_sched_latency+ sysctl_sched_min_granularity-1)/sysctl_sched_min_granularity` 确定的), 那么调度周期就是 `sched_min_granularity_ns*运行队列里的进程数`, 与 `sysctl_sched_latency` 无关; 否则队列进程数小于sched_nr_latency, 运行周期就是sysctl_sched_latency. 显然这个数越小, 一个运行队列支持的sched_nr_latency越少, 而且当sysctl_sched_min_granularity越小时能支持的sched_nr_latency越多, 那么每个进程在这个周期内能执行的时间也就越少, 这也与上面sysctl_sched_min_granularity变量的讨论一致. 其实sched_nr_latency也可以当做我们cpu load的基准值, 如果cpu的load大于这个值, 那么说明cpu不够使用了 |
| [`sysctl_sched_wakeup_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98) | `/proc/sys/kernel/sched_wakeup_granularity_ns` | `4000000ns` | 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间(sysctl_sched_min_granularity), 如果这个数值越小, 那么发生抢占的概率也就越高(见wakeup_gran、wakeup_preempt_entity函数) |
| [`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L87) | `/proc/sys/kernel/sched_child_runs_first` | 0 | 该变量表示在创建子进程的时候是否让子进程抢占父进程, 即使父进程的vruntime小于子进程, 这个会减少公平性, 但是可以降低write_on_copy, 具体要根据系统的应用情况来考量使用哪种方式（见task_fork_fair过程） |
| [`sysctl_sched_migration_cost`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L101) | `/proc/sys/kernel/sched_migration_cost` | `500000ns` | 该变量用来判断一个进程是否还是hot, 如果进程的运行时间（now - p->se.exec_start）小于它, 那么内核认为它的code还在cache里, 所以该进程还是hot, 那么在迁移的时候就不会考虑它 |
| [`sysctl_sched_tunable_scaling`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L68) | `/proc/sys/kernel/sched_tunable_scaling` | 1 | 当内核试图调整sched_min_granularity, sched_latency和sched_wakeup_granularity这三个值的时候所使用的更新方法, 0为不调整, 1为按照cpu个数以2为底的对数值进行调整, 2为按照cpu的个数进行线性比例的调整 |
| [`sysctl_sched_cfs_bandwidth_slice`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L124) | `/proc/sys/kernel/sched_cfs_bandwidth_slice_us` | 5000us | |


##1.2 RT相关
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_rt_period`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L76) | `/proc/sys/kernel/sched_rt_period_us` | `1000000us` | 该参数与下面的sysctl_sched_rt_runtime一起决定了实时进程在以sysctl_sched_rt_period为周期的时间内, 实时进程最多能够运行的总的时间不能超过sysctl_sched_rt_runtime（代码见sched_rt_global_constraints |
| [`sysctl_sched_rt_runtime`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L84) | `/proc/sys/kernel/sched_rt_runtime_us` | `950000us` | 见上 `sysctl_sched_rt_period` 变量的解释 |
| [`sysctl_sched_compat_yield`]() | `/proc/sys/kernel/sched_compat_yield` | 0 | 该参数可以让sched_yield()系统调用更加有效, 让它使用更少的cpu, 对于那些依赖sched_yield来获得更好性能的应用可以考虑设置它为1 |


##1.3  全局参数
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_features`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L52) | `/proc/sys/kernel/sched_features` | `3183d=110001101111b` | 该变量表示调度器支持的特性, 如GENTLE_FAIR_SLEEPERS(平滑的补偿睡眠进程),START_DEBIT(新进程尽量的早调度),WAKEUP_PREEMPT(是否wakeup的进程可以去抢占当前运行的进程)等, 所有的features见内核sech_features.h文件的定义 |
| [`sysctl_sched_nr_migrate`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L62) | `/proc/sys/kernel/sched_nr_migrate` | 32 | 在多CPU情况下进行负载均衡时, 一次最多移动多少个进程到另一个CPU上 |




#2   进程最少运行时间 `sysctl_sched_min_granularity`
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) | `/proc/sys/kernel/sched_min_granularity_ns` | `4000000ns` | 表示进程最少运行时间, 防止频繁的切换, 对于交互系统(如桌面), 该值可以设置得较小, 这样可以保证交互得到更快的响应(见周期调度器的 `check_preempt_tick` 过程) |


##2.1   参数背景
-------

[`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) 定义在 [](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L70). 如下所示:


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L70
/*
 * Minimal preemption granularity for CPU-bound tasks:
 *
 * (default: 0.75 msec * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_min_granularity       = 750000ULL;
unsigned int normalized_sysctl_sched_min_granularity    = 750000ULL;
```



##2.2   `check_preempt_wakeup` 检查周期性抢占
-------


在每个时钟中断中都会触发周期性调度, 调用周期性调度器主函数 `scheduler_tick`, 但是并不是每次触发都会导致调度, 调度也是有开销的, 如果频繁的调度反而降低系统的性能. 因此内核设计了 [`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) 参数, 表示进程的最少运行时间, 只有进程时间超过这个时间才会设置调度标识, 这样内核将在下一次调度时机, 调度 scheduler 完成调度, 让当前进程让出 `CPU` 了,

```cpp
scheduler_tick
    -=> task_tick_fair
        -=> entity_tick
            -=> check_preempt_tick
```

调度器完成调度和选择进程的时候, 需要检查当前进程是否需要调度, 通过 `check_preempt_tick` 来完成

>对应的唤醒抢占的时候, 检查是否可以抢占当前进程的操作, 由函数 `check_preempt_wakeup` 来完成.

```cpp
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void
check_preempt_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    unsigned long ideal_runtime, delta_exec;
    struct sched_entity *se;
    s64 delta;

    //  首先 sched_slice 计算进程的理想运行时间(实际时间ns)
    ideal_runtime = sched_slice(cfs_rq, curr);
    //  计算当前进程的已实际运行时间 delta_exec
    delta_exec = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;
    //  如果实际运行时间 delta_exec 比理想运行时间要大
    //  说明进程已经运行了很久了, 那么应该让出 CPU
    if (delta_exec > ideal_runtime) {
        //  设置当前进程的TIF的抢占标识
        resched_curr(rq_of(cfs_rq));
        /*
         * The current task ran long enough, ensure it doesn't get
         * re-elected due to buddy favours.
         */
        clear_buddies(cfs_rq, curr);
        return;
    }

    /*
     * Ensure that a task that missed wakeup preemption by a
     * narrow margin doesn't have to wait for a full slice.
     * This also mitigates buddy induced latencies under load.
     */
    //  如果实际运行时间小于 进程最少运行时间就直接返回, 不会设置调度标记
    if (delta_exec < sysctl_sched_min_granularity)
        return;

    //  如果进程实际运行时间超过了最少运行时间, 则取出红黑树上最左(虚拟运行时间最小的)节点
    //  进一步比较当前进程和运行队列上虚拟运行时间最小的进程
    se = __pick_first_entity(cfs_rq);
    delta = curr->vruntime - se->vruntime;

    if (delta < 0)
        return;

    //  只有当前进程与最左节点运行时间的差值超过当前进程的理想运行时间, 才会设置调度标记
    if (delta > ideal_runtime)
        resched_curr(rq_of(cfs_rq));
}
```

从`check_preempt_tick` 可以看出, 周期性调度的时候, 如果进程当前运行周期的运行时间足够长, 就需要设置调度标记, 调度器会可以在合适的调度时机完成抢占调度, 评价当前进程运行时间足够长的标准就是满足如下条件任意一条

*   当前进程已经用尽了 CFS 给予的时间片, 即实际运行时间 `delta_exec` 超过 CFS 分配的理想运行时间 `ideal_runtime`.

*   当前进程没有用尽 CFS 给予的时间片, 但是其当前周期的运行时间超过了程序的最小运行时间阈值 `sysctl_sched_min_granularity`, 同时其运行时间超过运行队列上最饥饿进程(红黑数的最左结点)一个运行周期, 即虚拟运行时间比后者还大 `ideal_runtime`.



##2.3   `sysctl_sched_min_granularity` 接口实现
-------

`sysctl_sched_min_granularity` 接口位于 `/proc/sys/kernel/sched_min_granularity_ns`, 内核中的定义在 [`kernel/sysctl.c, line 316`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sysctl.c#L316). 可以看到其默认权限为 `0644`(`root` 用户可读写, 其他用户读), 值范围从 `min_sched_granularity_ns(默认0.1ms) ~ max_sched_granularity_ns(默认1s)`.


```cpp
http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sysctl.c#L290
static int min_sched_granularity_ns = 100000;       /* 100 usecs */
static int max_sched_granularity_ns = NSEC_PER_SEC; /* 1 second */

//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sysctl.c#L316
#ifdef CONFIG_SCHED_DEBUG
    {
        .procname   = "sched_min_granularity_ns",
        .data       = &sysctl_sched_min_granularity,
        .maxlen     = sizeof(unsigned int),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_sched_granularity_ns,
        .extra2     = &max_sched_granularity_ns,
    },
    //......
#endif
```


待插入的测试的内容和图片


#3  调度周期 `sysctl_sched_latency`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_latency`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54) | `/proc/sys/kernel/sched_latency_ns` | `20000000ns` | 表示一个运行队列所有进程运行一次的周期 |


##3.1   参数背景
-------


[`sysctl_sched_latency`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54) 定义在[`kernel/sched/fair.c, version 4.14.14, line 41`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54)


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L41
/*
 * Targeted preemption latency for CPU-bound tasks:
 *
 * NOTE: this latency value is not the same as the concept of
 * 'timeslice length' - timeslices in CFS are of variable length
 * and have no persistent notion like in traditional, time-slice
 * based scheduling concepts.
 *
 * (to see the precise effective timeslice length of your workload,
 *  run vmstat and monitor the context-switches (cs) field)
 *
 * (default: 6ms * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_latency           = 6000000ULL;
unsigned int normalized_sysctl_sched_latency        = 6000000ULL;


//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L78
/*
 * This value is kept at sysctl_sched_latency/sysctl_sched_min_granularity
 */
static unsigned int sched_nr_latency = 8;
```

##3.2   参数详解
-------

##3.2.1   调度周期
-------

调度器实际的调度周期由 `__sched_period` 函数完成. `CFS` 保证在一个调度周期内每个进程至少能够运行一次.


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L654
/*
 * The idea is to set a period in which each task runs once.
 *
 * When there are too many tasks (sched_nr_latency) we have to stretch
 * this period because otherwise the slices get too small.
 *
 * p = (nr <= nl) ? l : l*nr/nl
 */
static u64 __sched_period(unsigned long nr_running)
{
    if (unlikely(nr_running > sched_nr_latency))
        return nr_running * sysctl_sched_min_granularity;
    else
        return sysctl_sched_latency;
}
```



调度周期与运行队列的进程数 `nr_running` 有关


*   如果进程数超过 `sched_nr_latency`那么调度周期就是进程的最小运行时间 `sched_min_granularity_ns` * 运行队列里的进程数 `nr_running`,与 `sysctl_sched_latency` 无关; 此时调度周期就是假定每个进程刚好运行最小运行时间的总和.


*   否则队列进程数小于 `sched_nr_latency`, 运行周期就是 `sysctl_sched_latency`.




###3.2.2    进程数目阈值 `sched_nr_latency`
-------

那进程数目阈值 `sched_nr_latency` 是怎么来的呢?

这个变量不能通过 `proc` 文件系统设置, 它是由



$$ \frac{sysctl_sched_latency + sysctl_sched_min_granularity - 1}{sysctl_sched_min_granularity} $$

即初始调度周期与进程最小运行时间的比值.


`sched_nr_latency` 初值为 8, 它正好就是

$$ \frac{sysctl_sched_latency + sysctl_sched_min_granularity - 1}{sysctl_sched_min_granularity} = \frac{6000000 + 750000 - 1}{750000}/ = 8 $$

显然 `sysctl_sched_latency`越小, 初始时一个运行队列支持的 `sched_nr_latency` 越少, 而且当 `sysctl_sched_min_granularity` 越小时能支持的 `sched_nr_latency`     `sysctl_sched_min_granularity` 变量的讨论一致.

>其实 `sched_nr_latency` 也可以当做我们 `cpu load` 的基准值, 如果 `cpu` 的 `load` 大于这个值, 那么说明 `cpu` 不够使用了


###3.2.3    调度周期 `sysctl_sched_latency` 的影响
-------

很明显, 只有当运行队列中的进程数目小于进程数目阈值 `sched_nr_latency` 的时候, 调度周期才刚好是 `sysctl_sched_latency`.

那么这个参数直接影响的就是当进程数目较少时, 初始的调度周期.


##3.3   `sysctl_sched_latency` 接口
-------

待插入的测试的内容和图片



#4  `sysctl_sched_wakeup_granularity` 唤醒后抢占
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_wakeup_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98) | `/proc/sys/kernel/sched_wakeup_granularity_ns` | `4000000ns` | 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高(见 `wakeup_gran`、`wakeup_preempt_entity`、 `check_preempt_wakeup` 函数) |






##4.1   参数背景
-------


[`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6087) 定义在[`kernel/sched/fair.c, version v4.14.14, line 98`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98)


唤醒后抢占时间 `sysctl_sched_wakeup_granularity` 与 进程最小运行时间 `sysctl_sched_min_granularity` 的作用类似. 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高.



##4.2   唤醒抢占
-------

当进程被唤醒的时候会调用 `check_preempt_wakeup` 检查被唤醒的进程是否可以抢占当前进程. 调用路径如下 :


```cpp
try_to_wake_up
    -=> ttwu_queue
        -=> ttwu_do_activate
            -=> ttwu_do_wakeup
                -=> check_preempt_curr
                    -=> check_preempt_wakeup
                        -=> wakeup_preempt_entity
                            -=> wakeup_gran
```

同样对于新创建的进程, 内核也将对其进行唤醒操作, 调用路径如下 :

```cpp
do_fork
    -=> wake_up_new_task
        -=> check_preempt_curr
            -=> check_preempt_wakeup
                -=> wakeup_preempt_entity
                    -=> wakeup_gran
```


[`check_preempt_wakeup`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164) 函数被定义在 [`kernel/sched/fair.c, verion 4.14.14, line 6164`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164), 如下所示:


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{
    struct sched_entity *se = &curr->se, *pse = &p->se;
    // ......
    //  检查当前被唤醒的进程进程p是否可以抢占当前进程curr
    if (wakeup_preempt_entity(se, pse) == 1) {
        /*
         * Bias pick_next to pick the sched entity that is
         * triggering this preemption.
         */
        if (!next_buddy_marked)
            set_next_buddy(pse);
        goto preempt;
    }

    return;

preempt:
    //  设置调度标记
    resched_curr(rq);
    // ......
}
```


[`check_preempt_wakeup`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164) 函数中通过 `wakeup_preempt_entity` 检查当前被唤醒的进程实体 `pse` 能否抢占当前进程的调度实体 `curr->se`. 如果发现可以抢占, 就通过 `resched_curr` 设置调度标记.


`wakeup_preempt_entity` 函数定义在 [`kernel/sched/fair.c, version v4.14.14, line 6127`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6127). 如下所示:


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6127
/*
 * Should 'se' preempt 'curr'.
 *
 *             |s1
 *        |s2
 *   |s3
 *         g
 *      |<--->|c
 *
 *  w(c, s1) = -1
 *  w(c, s2) =  0
 *  w(c, s3) =  1
 *
 */
static int
wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se)
{
    s64 gran, vdiff = curr->vruntime - se->vruntime;

    //  se要想抢占curr, 最基本的条件是虚拟运行时间比curr的小
    if (vdiff <= 0)
        return -1;

    //  计算当前进程 se 以 sysctl_sched_min_granularity 为基数的虚拟运行时间 gran
    gran = wakeup_gran(curr, se);
    // 只有两者虚拟运行时间的差值大于 gran
    if (vdiff > gran)
        return 1;

    return 0;
}

//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6087
//  计算 `sysctl_sched_wakeup_granularity` 基数的虚拟运行时间值
static unsigned long
wakeup_gran(struct sched_entity *curr, struct sched_entity *se)
{
    unsigned long gran = sysctl_sched_wakeup_granularity;

    /*
     * Since its curr running now, convert the gran from real-time
     * to virtual-time in his units.
     *
     * By using 'se' instead of 'curr' we penalize light tasks, so
     * they get preempted easier. That is, if 'se' < 'curr' then
     * the resulting gran will be larger, therefore penalizing the
     * lighter, if otoh 'se' > 'curr' then the resulting gran will
     * be smaller, again penalizing the lighter task.
     *
     * This is especially important for buddies when the leftmost
     * task is higher priority than the buddy.
     */
    return calc_delta_fair(gran, se);
}


//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L646
//  计算进程实际运行 delta 时间的虚拟运行时间
/*
 * delta /= w
 */
static inline u64 calc_delta_fair(u64 delta, struct sched_entity *se)
{
    if (unlikely(se->load.weight != NICE_0_LOAD))
        delta = __calc_delta(delta, NICE_0_LOAD, &se->load);

    return delta;
}
```

可以看出 `wakeup_preempt_entity` 函数中:



`curr` 唤醒进程 `p`, 如果 `p` 可以抢占 `curr`, 则要求满足


当前进程 `curr` 的虚拟运行时间不仅要比进程　`p` 的大, 还要大过 `calc_delta_fair(sysctl_sched_wakeup_granularity, p)`


*   当前进程 `curr` 的虚拟运行时间要比进程　`p` 的大

    ```cpp
       curr->vruntime - p->vruntime > 0
    ```

    否则 `wakeup_preempt_entity` 函数返回 `-1`.


*   当前进程 `curr` 的虚拟运行时间比进程　`p` 的大 `calc_delta_fair`


    ```cpp
    curr->vruntime - p->vruntime > calc_delta_fair(sysctl_sched_wakeup_granularity, p)
    ```

    否则  `wakeup_preempt_entity` 函数返回 `0`.


我们假设进程 `p` 刚好实际运行了唤醒后抢占时间基数 `sysctl_sched_wakeup_granularity`, 则其虚拟运行时间将增长 `gran`. 被唤醒的进程 `p` 要想抢占 `curr`, 则要求其虚拟运行时间比 `curr` 小 `gran`. 则保证理想情况下(没有其他进程干预和进程 `p` 睡眠等其他外因), 进程 `p` 可以至少运行 `sysctl_sched_wakeup_granularity` 时间.


我们用一个图来表示:

假设 `curr` 的虚拟运行时间位于图中的位置, 中间区间是一个 `gran` 长度


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `-1` 的时候, `vdiff <= 0`, 不能被唤醒; (函数 `wakeup_preempt_entity` 返回 `-1`)


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `0` 的时候, `0 <= vdiff <= gran`, 同样不能被唤醒; (函数 `wakeup_preempt_entity` 返回 `0`)


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `1` 的时候, `vdiff > gran`, 能被唤醒; (函数 `wakeup_preempt_entity` 返回 `1`)




```cpp
|----区间 1----|----区间 0----|----区间 -1----|
|             -             curr   +
--------------|--------------|--------------
              |<----gran---->|
              |              |
 vdiff > gran |  vdiff > 0   |   vdiff < 0
      1       |       0      |      -1
              |              |
--------------|--------------|--------------
```


##4.3   `sysctl_sched_wakeup_granularity` 接口
-------

待插入的测试的内容和图片




#5  `sysctl_sched_tunable_scaling`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_tunable_scaling`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L68) | `/proc/sys/kernel/sched_tunable_scaling` | 1 | 当内核试图调整sched_min_granularity, sched_latency和sched_wakeup_granularity这三个值的时候所使用的更新方法, 0为不调整, 1为按照cpu个数以2为底的对数值进行调整, 2为按照cpu的个数进行线性比例的调整 |



##5.1   参数背景
-------

内核中有有几个调度参数都是描述的进程运行时间相关的信息.


*   调度周期 `sched_latency`


*   进程最小运行时间 `sched_min_granularity`


*   唤醒后运行时间基数 `sched_wakeup_granularity`


之前我们看到这些阈值定义的时候, 总有一个 `normalized_sysctl_XXX` 的阈值伴随着它们. 这个 `normalized_sysctl_XXX` 具体是用来做什么的呢 ?


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L841
/*
 * Targeted preemption latency for CPU-bound tasks:
 *
 * NOTE: this latency value is not the same as the concept of
 * 'timeslice length' - timeslices in CFS are of variable length
 * and have no persistent notion like in traditional, time-slice
 * based scheduling concepts.
 *
 * (to see the precise effective timeslice length of your workload,
 *  run vmstat and monitor the context-switches (cs) field)
 *
 * (default: 6ms * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_latency           = 6000000ULL;
unsigned int normalized_sysctl_sched_latency        = 6000000ULL;
```

```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L70
/*
 * Minimal preemption granularity for CPU-bound tasks:
 *
 * (default: 0.75 msec * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_min_granularity       = 750000ULL;
unsigned int normalized_sysctl_sched_min_granularity    = 750000ULL;

/*
 * This value is kept at sysctl_sched_latency/sysctl_sched_min_granularity
 */
static unsigned int sched_nr_latency = 8;
```


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L89
/*
 * SCHED_OTHER wake-up granularity.
 *
 * This option delays the preemption effects of decoupled workloads
 * and reduces their over-scheduling. Synchronous workloads will still
 * have immediate wakeup/sleep latencies.
 *
 * (default: 1 msec * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_wakeup_granularity        = 1000000UL;
unsigned int normalized_sysctl_sched_wakeup_granularity = 1000000UL;
```


通过注释我们可以看到一些端倪, 这些 `normalized_sysctl_XXX` 的变量用于计算 非 `normalized_sysctl` 的值. 默认的计算方式就是 :


```cpp
sysctl_sched_XXX = normalized_sysctl_XXX * (1 + ilog(ncpus))
```

那为什么需要这样计算呢 ?



在 `SMP` 架构下, 随着 `CPU` 数量的增加, 程序的并行化增加. 用户可见的 "有效延迟" 降低. 但是这种关系并不是线性的, 所以调度器实现了几种计算方式, 就是在基础值 (`normalized_sysctl_XXX`) 的基础上乘上一个系数 `factor`. 内核提供了系数的多种计算方法.

| 策略 | 系数 | 计算方法 |
|:---:|:----:|:------:|
| SCHED_TUNABLESCALING_NONE   | 1             | `sysctl_XXX = normalized_sysctl_XXX` |
| SCHED_TUNABLESCALING_LOG    | 1+ilog(ncpus) | `sysctl_XXX = normalized_sysctl_XXX * (1+ilog(ncpus))` |
| SCHED_TUNABLESCALING_LINEAR | ncpus         | `sysctl_XXX = normalized_sysctl_XXX * ncpus` |

内核参数 [`sysctl_sched_tunable_scaling`](https://elixir.bootlin.com/linux/v4.16-rc1/source/kernel/sched/fair.c#L58) 就是内核用来控制调度器使用那种计算方法的. 默认值`SCHED_TUNABLESCALING_LOG` 标识用 `log2` 去做 `CPU` 数量. 这个想法来自 `Con Kolivas` 的 `SD` 调度器.



##5.2   参数详解
-------


当CPU数量更多时, 内核需要增加 `sched_min_granularity`, `sched_latency` 和 `sched_wakeup_granularity` 这几个内核参数的粒度值. 内核定义了一组基础值 `normalized_sysctl_XXX` 和一套校正系数 `fctor` 的计算方法, 在基础值的基础上乘以校正因子 `factor`, 作为内核最终采用的实际值 `sysctl_XXX`.

关于校正因子的计算内核提供了多种计算方法, 由 `sched_tunable_scaling` 表示.



```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/include/linux/sched/sysctl.h#L27
enum sched_tunable_scaling {
    SCHED_TUNABLESCALING_NONE,
    SCHED_TUNABLESCALING_LOG,
    SCHED_TUNABLESCALING_LINEAR,
    SCHED_TUNABLESCALING_END,
};
```


内核参数 [`sysctl_sched_tunable_scaling`](https://elixir.bootlin.com/linux/v4.16-rc1/source/kernel/sched/fair.c#L58) 就是内核用来控制调度器使用那种计算方法的. 默认值`SCHED_TUNABLESCALING_LOG` 标识用 `log2` 去做 `CPU` 数量. 这个想法来自 `Con Kolivas` 的 `SD` 调度器.



```cpp
//  https://elixir.bootlin.com/linux/v4.16-rc1/source/kernel/sched/fair.c#L58
/*
 * The initial- and re-scaling of tunables is configurable
 *
 * Options are:
 *
 *   SCHED_TUNABLESCALING_NONE - unscaled, always *1
 *   SCHED_TUNABLESCALING_LOG - scaled logarithmical, *1+ilog(ncpus)
 *   SCHED_TUNABLESCALING_LINEAR - scaled linear, *ncpus
 *
 * (default SCHED_TUNABLESCALING_LOG = *(1+ilog(ncpus))
 */
enum sched_tunable_scaling sysctl_sched_tunable_scaling = SCHED_TUNABLESCALING_LOG;
```

函数 [`get_update_sysctl_factor`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L153) 用来获取当前策略 `sysctl_sched_tunable_scaling` 下的 `factor` 系数.



```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L153
/*
 * Increase the granularity value when there are more CPUs,
 * because with more CPUs the 'effective latency' as visible
 * to users decreases. But the relationship is not linear,
 * so pick a second-best guess by going with the log2 of the
 * number of CPUs.
 *
 * This idea comes from the SD scheduler of Con Kolivas:
 */
static unsigned int get_update_sysctl_factor(void)
{
    unsigned int cpus = min_t(unsigned int, num_online_cpus(), 8);
    unsigned int factor;

    switch (sysctl_sched_tunable_scaling) {
    case SCHED_TUNABLESCALING_NONE:
        factor = 1;
        break;
    case SCHED_TUNABLESCALING_LINEAR:
        factor = cpus;
        break;
    case SCHED_TUNABLESCALING_LOG:
    default:
        factor = 1 + ilog2(cpus);
        break;
    }

    return factor;
}
```

[`update_sysctl`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L183) 函数在系统启动和 `CPU` 上下线的时候设置 `sysctl_XXX` 的值.

即


```cpp
sysctl_sched_XXX = normalized_sysctl_XXX * factor
```


```
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L183
static void update_sysctl(void)
{
    unsigned int factor = get_update_sysctl_factor();

#define SET_SYSCTL(name) \
    (sysctl_##name = (factor) * normalized_sysctl_##name)
    SET_SYSCTL(sched_min_granularity);
    SET_SYSCTL(sched_latency);
    SET_SYSCTL(sched_wakeup_granularity);
#undef SET_SYSCTL
}


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L195
void sched_init_granularity(void)
{
    update_sysctl();
}


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L9024
static void rq_online_fair(struct rq *rq)
{
    update_sysctl();

    update_runtime_enabled(rq);
}



//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L9051
static void rq_offline_fair(struct rq *rq)
{
    update_sysctl();

    /* Ensure any throttled groups are reachable by pick_next_task */
    unthrottle_offline_cfs_rqs(rq);
}
```


`sched_proc_update_handler` 函数则用于在用户通过 `procfs` 接口更新了 `sysctl_XXX` 的值之后, 同步更新 `normalized_sysctl_XXX`的值, 否则下一次通过 `update_sysctl` 转换的值将会有问题.

即

```cpp
normalized_sysctl_XXX = sysctl_XXX / factor
```


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L153
/**************************************************************
 * Scheduling class statistics methods:
 */
int sched_proc_update_handler(struct ctl_table *table, int write,
        void __user *buffer, size_t *lenp,
        loff_t *ppos)
{
    int ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
    unsigned int factor = get_update_sysctl_factor();

    if (ret || !write)
        return ret;

    sched_nr_latency = DIV_ROUND_UP(sysctl_sched_latency,
                    sysctl_sched_min_granularity);

#define WRT_SYSCTL(name) \
    (normalized_sysctl_##name = sysctl_##name / (factor))
    WRT_SYSCTL(sched_min_granularity);
    WRT_SYSCTL(sched_latency);
    WRT_SYSCTL(sched_wakeup_granularity);
#undef WRT_SYSCTL

    return 0;
}


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sysctl.c#L313
static struct ctl_table kern_table[] = {
    //  ......
#ifdef CONFIG_SCHED_DEBUG
    {
        .procname   = "sched_min_granularity_ns",
        .data       = &sysctl_sched_min_granularity,
        .maxlen     = sizeof(unsigned int),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_sched_granularity_ns,
        .extra2     = &max_sched_granularity_ns,
    },
    {
        .procname   = "sched_latency_ns",
        .data       = &sysctl_sched_latency,
        .maxlen     = sizeof(unsigned int),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_sched_granularity_ns,
        .extra2     = &max_sched_granularity_ns,
    },
    {
        .procname   = "sched_wakeup_granularity_ns",
        .data       = &sysctl_sched_wakeup_granularity,
        .maxlen     = sizeof(unsigned int),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_wakeup_granularity_ns,
        .extra2     = &max_wakeup_granularity_ns,
    },
#ifdef CONFIG_SMP
    {
        .procname   = "sched_tunable_scaling",
        .data       = &sysctl_sched_tunable_scaling,
        .maxlen     = sizeof(enum sched_tunable_scaling),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_sched_tunable_scaling,
        .extra2     = &max_sched_tunable_scaling,
    },
    // ......
#endif
```

##5.3   接口详解
-------


#6  `sysctl_sched_child_runs_first`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L87) | `/proc/sys/kernel/sched_child_runs_first` | 0 | 该变量表示在创建子进程的时候是否让子进程抢占父进程, 即使父进程的vruntime小于子进程, 这个会减少公平性, 但是可以降低write_on_copy, 具体要根据系统的应用情况来考量使用哪种方式（见task_fork_fair过程） |


##6.1   参数背景
-------


一般来说, 通过父进程通过 `fork` 创建子进程的时候, 内核并不保证谁先运行, 但是有时候我们更希望约定父子进之间运行的顺序. 因此 `sysctl_sched_child_runs_first` 应运而生.


该变量表示在创建子进程的时候是否让子进程抢占父进程, 即使父进程的 `vruntime` 小于子进程, 这个会减少公平性, 但是可以降低 `write_on_copy`, 具体要根据系统的应用情况来考量使用哪种方式（见 `task_fork_fair` 过程）


##6.2   `sysctl_sched_child_runs_first` 保证子进程先运行
-------


[`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L83) 定义在[`kernel/sched/fair.c, version v4.14.14, line 83`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L83)


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L83
/*
 * After fork, child runs first. If set to 0 (default) then
 * parent will (try to) run first.
 */
unsigned int sysctl_sched_child_runs_first __read_mostly;
```

唤醒后抢占时间 `sysctl_sched_wakeup_granularity` 与 进程最小运行时间 `sysctl_sched_min_granularity` 的作用类似. 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高.


当内核通过 `fork/clone` 创建子进程的时候, 在 `sched_fork` 中完成了调度信息的初始化, 此时会调用对应调度器类 `task_fork` 的初始化.


```cpp
_do_fork
    -=> copy_process
        -=> sched_fork
            -=> p->sched_class->task_fork(p);
```


`CFS` 调度器的 `task_fork` 函数就是 `task_fork_fair`.

```cpp
/*
 * called on fork with the child task as argument from the parent's context
 *  - child not yet on the tasklist
 *  - preemption disabled
 */
static void task_fork_fair(struct task_struct *p)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se, *curr;
    struct rq *rq = this_rq();
    struct rq_flags rf;

    rq_lock(rq, &rf);
    update_rq_clock(rq);

    cfs_rq = task_cfs_rq(current);
    curr = cfs_rq->curr;
    if (curr) {
        update_curr(cfs_rq);
        se->vruntime = curr->vruntime;
    }
    place_entity(cfs_rq, se, 1);

    if (sysctl_sched_child_runs_first && curr && entity_before(curr, se)) {
        /*
         * Upon rescheduling, sched_class::put_prev_task() will place
         * 'current' within the tree based on its new key value.
         */
        swap(curr->vruntime, se->vruntime);
        resched_curr(rq);
    }

    se->vruntime -= cfs_rq->min_vruntime;
    rq_unlock(rq, &rf);
}
```


如果参数 `sysctl_sched_child_runs_first` 被设置, 同时 `curr` 进程的虚拟运行时间比创建的子进程 `se`, 就交换它们的虚拟运行时间. 通过这种方式来保证子进程的虚拟运行时间比父进程的小, 从而保证子进程优先运行.


##6.3 `sysctl_sched_child_runs_first` 接口
-------


#7  `sysctl_sched_migration_cost`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_migration_cost`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L101) | `/proc/sys/kernel/sched_migration_cost` | `500000ns` | 该变量用来判断一个进程是否还是 `hot`, 如果进程的运行时间 (`now - p->se.exec_start`) 小于它, 那么内核认为它的 `code` 还在 `cache` 里, 所以该进程还是 `hot`, 那么在迁移的时候就不会考虑它 |


##7.1   参数背景
-------


`CPU` 负载平衡有两种方式 : `pull` 和 `push`, 即

*   空闲 `CPU` 从其他忙的 `CPU` 队列中拉一个进程到当前 `CPU` 队列, 通过 `idle_balance` 完成;

*   或者忙的CPU队列将一个进程推送到空闲的 `CPU` 队列中.


`idle_balance` 干的则是 `pull` 的事情, `idle_balance` 会 `for_each_domain(this_cpu, sd)` 则是遍历当前 `CPU`所在的调度域, 维持调度域内的核间平衡.

但是负载平衡有一个矛盾就是 : 负载平衡的频度和 `CPU cache` 的命中率是矛盾的, `CPU` 调度域就是将各个 `CPU` 分成层次不同的组, 低层次搞定的平衡就绝不上升到高层次处理, 避免影响 `cache` 的命中率.


*   调度器通过 `sysctl_sched_migration_cost` 这个 `proc` 阈值判断进程来判断进程是否是 `cache hot` 的, 如果是 `hot` 的则不会进行负载均衡迁移.

*   同时通过该阈值控制当前 `CPU` 是否可以 `pull`. `sysctl_sched_migration_cost` 对应 `proc` 控制文件是 `/proc/sys/kernel/sched_migration_cost`, 开关代表如果 `CPU` 队列空闲了 `500000ns = 500us`(`sysctl_sched_migration_cost` 默认值) 以上, 则进行 `pull`, 否则则返回.


##7.2   `sysctl_sched_migration_cost` 实现详解
-------


###7.2.1 判断进程是否是 `cache-hot`
-------


调度器通过 [`task_hot`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6596) 函数检查一个进程是否是 `cache-hot` 的. 如果该进程还是 `cache-hot`, 那么在迁移的时候就不会考虑它.


*   如果 `sysctl_sched_migration_cost` 被设置为 `-1`, 则 `task_hot` 永远返回 `1`, 即认为进程永远是 `cache-hot` 的.


*   如果 `sysctl_sched_migration_cost` 被设置为 `0`, 则禁用了该配置, `task_hot` 永远返回 `0`.


*   如果进程的运行时间 (`now - p->se.exec_start`) 小于 `sysctl_sched_migration_cost`, 那么内核认为它的 `code` 还在 `cache` 里.



```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6596
/*
 * Is this task likely cache-hot:
 */
static int task_hot(struct task_struct *p, struct lb_env *env)
{
    //  ......

    if (sysctl_sched_migration_cost == -1)
        return 1;
    if (sysctl_sched_migration_cost == 0)
        return 0;

    delta = rq_clock_task(env->src_rq) - p->se.exec_start;

    return delta < (s64)sysctl_sched_migration_cost;
}
```

注意

`task_hot(cahce-hot)` 只是禁止进程迁移的充分条件, 而不是必要条件.

具体的信息请参见 [`can_migrate_task`](https://elixir.bootlin.com/linux/latest/source/kernel/sched/fair.c#L7063) 函数.


```cpp
//  (https://elixir.bootlin.com/linux/latest/source/kernel/sched/fair.c#L7063
/*
 * can_migrate_task - may task p from runqueue rq be migrated to this_cpu?
 */
static
int can_migrate_task(struct task_struct *p, struct lb_env *env)
{
    int tsk_cache_hot;

    lockdep_assert_held(&env->src_rq->lock);

    /*
     * We do not migrate tasks that are:
     * 1) throttled_lb_pair, or
     * 2) cannot be migrated to this CPU due to cpus_allowed, or
     * 3) running (obviously), or
     * 4) are cache-hot on their current CPU.
     */
    if (throttled_lb_pair(task_group(p), env->src_cpu, env->dst_cpu))
        return 0;

    if (!cpumask_test_cpu(env->dst_cpu, &p->cpus_allowed)) {
        //  ......
        return 0;
    }

    /* Record that we found atleast one task that could run on dst_cpu */
    env->flags &= ~LBF_ALL_PINNED;

    if (task_running(env->src_rq, p)) {
        //  ......
        return 0;
    }

    /*
     * Aggressive migration if:
     * 1) destination numa is preferred
     * 2) task is cache cold, or
     * 3) too many balance attempts have failed.
     */
    tsk_cache_hot = migrate_degrades_locality(p, env);
    if (tsk_cache_hot == -1)
        tsk_cache_hot = task_hot(p, env);

    if (tsk_cache_hot <= 0 ||
        env->sd->nr_balance_failed > env->sd->cache_nice_tries) {
        if (tsk_cache_hot == 1) {
            schedstat_inc(env->sd->lb_hot_gained[env->idle]);
            schedstat_inc(p->se.statistics.nr_forced_migrations);
        }
        return 1;
    }

    schedstat_inc(p->se.statistics.nr_failed_migrations_hot);
    return 0;
}
```



###7.2.2    `idle_balance` 是否需要 `pull`
-------


当一个 `CPU` 进入 `idle` 状态时, 将通过 `idle_balance` 从其他繁忙的 `CPU` 上 `pull` 一个进程下来.

如果 `CPU` 的运行队列 `rq` 空闲时间超过 `sysctl_sched_migration_cost` 以上, 才会从其他 `CPU` 运行队列上 `pull`, 否则则返回, 不会进行 `pull` 操作.


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L8374
/*
 * idle_balance is called by schedule() if this_cpu is about to become
 * idle. Attempts to pull tasks from other CPUs.
 */
static int idle_balance(struct rq *this_rq, struct rq_flags *rf)
{
    //  ......

    /*
     * This is OK, because current is on_cpu, which avoids it being picked
     * for load-balance and preemption/IRQs are still disabled avoiding
     * further scheduler activity on it and we're being very careful to
     * re-start the picking loop.
     */
    rq_unpin_lock(this_rq, rf);

    if (this_rq->avg_idle < sysctl_sched_migration_cost ||
        !this_rq->rd->overload) {
        rcu_read_lock();
        sd = rcu_dereference_check_sched_domain(this_rq->sd);
        if (sd)
            update_next_balance(sd, &next_balance);
        rcu_read_unlock();

        goto out;
    }

    //  ......

out:
    /* Move the next balance forward */
    if (time_after(this_rq->next_balance, next_balance))
        this_rq->next_balance = next_balance;

    /* Is there a task of a high priority class? */
    if (this_rq->nr_running != this_rq->cfs.h_nr_running)
        pulled_task = -1;

    if (pulled_task)
        this_rq->idle_stamp = 0;

    rq_repin_lock(this_rq, rf);

    return pulled_task;
}
```


##7.3   `sysctl_sched_migration_cost` 接口详解
-------



#8 `sysctl_sched_rt_period` 与 `sysctl_sched_rt_runtime`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_rt_period`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L76) | `/proc/sys/kernel/sched_rt_period_us` | `1000000us` | 该参数与下面的 `sysctl_sched_rt_runtime` 一起决定了实时进程在以 `sysctl_sched_rt_period` 为周期的时间内, 实时进程最多能够运行的总的时间, 不能超过 `sysctl_sched_rt_runtime`(代码见 `sched_rt_global_constraints`) |
| [`sysctl_sched_rt_runtime`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L84) | `/proc/sys/kernel/sched_rt_runtime_us` | `950000us` | 见上 `sysctl_sched_rt_period` 变量的解释 |

`RT` 实时进程有两种调度策略 : `FIFO`(先进先出) 和 `RR`(时间片轮转).

相同优先级的 `FIFO` 实时进程和 `RR` 实时进程挂到同一个运行队列上, 他们的不同在于 `RR` 实时进程运行一段时间后(时间片耗尽), 重新挂到运行队列的尾部等待下一次运行. 而 `FIFO` 的实时进程一旦运行, 则会一直占据 `CPU` 直到更高优先级的 `FIFO` 或 `RR` 进程抢占, 或自己主动放弃 `CPU`. 这样子好像很不公平, 通常会把运行时间较短的进程设置为 `FIFO` 调度策略, 把运行时间较长的进程设置为 `RR` 调度策略.


`RR` 实时进程是有时间片的, `FIFO` 实时进程是没有时间片. 但也有的说 `FIFO` 进程也有时间片, 从内核代码看, `FIFO` 实时进程也是有时间片的, 是为了在很多处理中不需要使用 `if` 语句区分这两种进程, 而在时钟中断中只会减少 `RR` 进程的时间片, 不会改变 `FIFO` 进程的时间片. `FIFO` 进程的时间片永远用不完, 使其时间片失去作用.

不同优先级的实时进程间是基于优先级进行抢占的, 那实时进程也是完全抢占普通进程的吗 ?


`RT` 调度类的优先级较高, 比普通进程 `CFS` 的优先级要高, 因此如果有实时进程在运行, 那么普通进程将无法获取到 `CPU` 运行时间. 即 `RT` 进程抢占 `CFS` 进程. 但是如果 `RT` 进程完全抢占 `CFS` 进程, 将导致普通进程得不到响应.


因此内核提供了这两个 `RT` 参数用来限制 `RT` 进程的运行时间占比.



```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/core.c#L72
/*
 * period over which we measure -rt task CPU usage in us.
 * default: 1s
 */
unsigned int sysctl_sched_rt_period = 1000000;


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/core.c#L80
/*
 * part of the period that we allow rt tasks to run in us.
 * default: 0.95s
 */
int sysctl_sched_rt_runtime = 950000;
```


`sysctl_sched_rt_period` 表示实时进程运行周期为 `1s`, `sysctl_sched_rt_runtime` 表示在运行周期内, 实时进程最多运行 `0.95` 秒, 即 `CPU` 最多 `95%` 的时间片交给 `RT` 进程运行. 内核强制实时进程为普通进程预留出一定的运行时间. 当然可以把 `sysctl_sched_rt_runtime` 和 `sysctl_sched_rt_period` 设置成相同的数值, 即实时进程可以完全抢占普通进程.


#9  sysctl_sched_rt_runtime
-------




```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/sched.h#L1264
static inline u64 global_rt_period(void)
{
    return (u64)sysctl_sched_rt_period * NSEC_PER_USEC;
}

static inline u64 global_rt_runtime(void)
{
    if (sysctl_sched_rt_runtime < 0)
        return RUNTIME_INF;

    return (u64)sysctl_sched_rt_runtime * NSEC_PER_USEC;
}
```


`RT` 调度器中使用了一个通用的结构 `rt_schedulable_data` 来表示这段限制.

通过 `to_ratio(period, runtime)` 检查 `RT` 进程的可运行时间比率.


在 `tg_rt_schedulable` 中, 计算当前 `sched_group` 下进程的运行比率 (`runtime / period`) 之和.


```cpp
//  https://elixir.bootlin.com/linux/latest/source/kernel/sched/rt.c#L2404
struct rt_schedulable_data {
    struct task_group *tg;
    u64 rt_period;
    u64 rt_runtime;
};

static int tg_rt_schedulable(struct task_group *tg, void *data)
{
    struct rt_schedulable_data *d = data;
    struct task_group *child;
    unsigned long total, sum = 0;
    u64 period, runtime;

    period = ktime_to_ns(tg->rt_bandwidth.rt_period);
    runtime = tg->rt_bandwidth.rt_runtime;

    if (tg == d->tg) {
        period = d->rt_period;
        runtime = d->rt_runtime;
    }

    /*
     * Cannot have more runtime than the period.
     */
    if (runtime > period && runtime != RUNTIME_INF)
        return -EINVAL;

    /*
     * Ensure we don't starve existing RT tasks.
     */
    if (rt_bandwidth_enabled() && !runtime && tg_has_rt_tasks(tg))
        return -EBUSY;

    total = to_ratio(period, runtime);

    /*
     * Nobody can have more than the global setting allows.
     */
    if (total > to_ratio(global_rt_period(), global_rt_runtime()))
        return -EINVAL;

    /*
     * The sum of our children's runtime should not exceed our own.
     */
    list_for_each_entry_rcu(child, &tg->children, siblings) {
        period = ktime_to_ns(child->rt_bandwidth.rt_period);
        runtime = child->rt_bandwidth.rt_runtime;

        if (child == d->tg) {
            period = d->rt_period;
            runtime = d->rt_runtime;
        }

        sum += to_ratio(period, runtime);
    }

    if (sum > total)
        return -EINVAL;

    return 0;
}

static int __rt_schedulable(struct task_group *tg, u64 period, u64 runtime)
{
    int ret;

    struct rt_schedulable_data data = {
        .tg = tg,
        .rt_period = period,
        .rt_runtime = runtime,
    };

    rcu_read_lock();
    ret = walk_tg_tree(tg_rt_schedulable, tg_nop, &data);
    rcu_read_unlock();

    return ret;
}
```


#10
-------


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/rt.c#L12
int sched_rr_timeslice = RR_TIMESLICE;
int sysctl_sched_rr_timeslice = (MSEC_PER_SEC / HZ) * RR_TIMESLICE;
```



#   参考资料
-------

[进程管理 | Linux Performance](http://linuxperf.com/?cat=10)

[Linux 调度器 BFS 简介](https://www.ibm.com/developerworks/cn/linux/l-cn-bfs/)

[从几个问题开始理解CFS调度器](http://ju.outofmemory.cn/entry/105407)

[内核参数说明](https://www.cnblogs.com/tolimit/p/5065761.html)


> 关于 `waker` 和 `wakee`
>
>*  waker : The running process which try to wakeup an un-running process
>
>*  wakee : The un-running process to be wakeup


<br>

*      本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*      采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*      基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

`
