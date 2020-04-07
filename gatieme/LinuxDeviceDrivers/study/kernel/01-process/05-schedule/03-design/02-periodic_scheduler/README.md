Linux进程核心调度器之周期性调度器
=======



| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-29 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/details/51456569) |



我们前面提到linux有两种方法激活调度器：核心调度器和

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要


因而内核提供了两个调度器**主调度器**，**周期性调度器**，分别实现如上工作, 两者合在一起就组成了**核心调度器(core scheduler)**, 也叫**通用调度器(generic scheduler)**.



他们都根据进程的优先级分配CPU时间, 因此这个过程就叫做优先调度, 我们将在本节主要讲解核心调度器的设计和优先调度的实现方式.


而我们的周期性调度器以固定的频率激活负责当前进程调度类的周期性调度方法, 以保证系统的并发性





#1	前景回顾
-------

首先还是让我们简单回顾一下子之前的的内容

##1.1	进程调度
-------


内存中保存了对每个进程的唯一描述, 并通过若干结构与其他进程连接起来.


**调度器**面对的情形就是这样, 其任务是在程序之间共享CPU时间, 创造并行执行的错觉, 该任务分为两个不同的部分, 其中一个涉及**调度策略**, 另外一个涉及**上下文切换**.


内核必须提供一种方法, 在各个进程之间尽可能公平地共享CPU时间, 而同时又要考虑不同的任务优先级.


调度器的一般原理是, 按所需分配的计算能力, 向系统中每个进程提供最大的公正性, 或者从另外一个角度上说, 他试图确保没有进程被亏待.



##1.2	进程的分类
-------

linux把进程区分为**实时进程**和**非实时进程**, 其中非实时进程进一步划分为交互式进程和批处理进程

根据进程的不同分类Linux采用不同的调度策略.

对于实时进程，采用FIFO, Round Robin或者Earliest Deadline First (EDF)最早截止期限优先调度算法|的调度策略.

对于普通进程则采用CFS完全公平调度器进行调度


##1.3	linux调度器的演变
-------

| 字段 | 版本 |
| ------------- |:-------------:|
| O(n)的始调度算法 | linux-0.11~2.4 |
| O(1)调度器 | linux-2.5 |
| CFS调度器 | linux-2.6~至今 |


##1.4	Linux的调度器组成
-------


**2个调度器**

可以用两种方法来激活调度

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要

因此当前linux的调度程序由两个调度器组成：**主调度器**，**周期性调度器**(两者又统称为**通用调度器(generic scheduler)**或**核心调度器(core scheduler)**)

并且每个调度器包括两个内容：**调度框架**(其实质就是两个函数框架)及**调度器类**



**6种调度策略**

linux内核目前实现了6中调度策略(即调度算法), 用于对不同类型的进程进行调度, 或者支持某些特殊的功能

*	SCHED_NORMAL和SCHED_BATCH调度普通的非实时进程

*	SCHED_FIFO和SCHED_RR和SCHED_DEADLINE则采用不同的调度策略调度实时进程

*	SCHED_IDLE则在系统空闲时调用idle进程.



**5个调度器类**

而依据其调度策略的不同实现了5个调度器类, 一个调度器类可以用一种种或者多种调度策略调度某一类进程, 也可以用于特殊情况或者调度特殊功能的进程.


其所属进程的优先级顺序为
```c
stop_sched_class -> dl_sched_class -> rt_sched_class -> fair_sched_class -> idle_sched_class
```

**3个调度实体**

调度器不限于调度进程, 还可以调度更大的实体, 比如实现组调度.

这种一般性要求调度器不直接操作进程, 而是处理可调度实体, 因此需要一个通用的数据结构描述这个调度实体,即seched_entity结构, 其实际上就代表了一个调度对象，可以为一个进程，也可以为一个进程组.

linux中针对当前可调度的实时和非实时进程, 定义了类型为seched_entity的3个调度实体

*	sched_dl_entity 采用EDF算法调度的实时调度实体

*	sched_rt_entity 采用Roound-Robin或者FIFO算法调度的实时调度实体 rt_sched_class

*	sched_entity 采用CFS算法调度的普通非实时进程的调度实体

它们的关系如下图

![调度器的组成](../../images/level.jpg)


#2	周期性调度器
-------


周期性调度器在scheduler_tick中实现. 如果系统正在活动中, 内核会按照频率HZ自动调用该函数. 如果没有近曾在等待调度, 那么在计算机电力供应不足的情况下, 内核将关闭该调度器以减少能耗. 这对于我们的嵌入式设备或者手机终端设备的电源管理是很重要的.


##2.1	周期性调度器主流程
-------


scheduler_tick函数定义在[kernel/sched/core.c, L2910](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2910)中, 它有两个主要任务



1.	更新相关统计量

	管理内核中的与整个系统和各个进程的调度相关的统计量. 其间执行的主要操作是对各种计数器+1

2.	激活负责当前进程调度类的周期性调度方法

	检查进程执行的时间是否超过了它对应的ideal_runtime，如果超过了，则告诉系统，需要启动主调度器(schedule)进行进程切换。(注意thread_info:preempt_count、thread_info:flags (TIF_NEED_RESCHED))



```c
/*
 * This function gets called by the timer code, with HZ frequency.
 * We call it with interrupts disabled.
 */

void scheduler_tick(void)
{
    /*  1.  获取当前cpu上的全局就绪队列rq和当前运行的进程curr  */

    /*  1.1 在于SMP的情况下，获得当前CPU的ID。如果不是SMP，那么就返回0  */
    int cpu = smp_processor_id();

    /*  1.2 获取cpu的全局就绪队列rq, 每个CPU都有一个就绪队列rq  */
    struct rq *rq = cpu_rq(cpu);

    /*  1.3 获取就绪队列上正在运行的进程curr  */
    struct task_struct *curr = rq->curr;

    sched_clock_tick();

	/*  2 更新rq上的统计信息, 并执行进程对应调度类的周期性的调度  */

    /*  加锁 */
    raw_spin_lock(&rq->lock);

    /*  2.1 更新rq的当前时间戳.即使rq->clock变为当前时间戳  */
    update_rq_clock(rq);

    /*  2.2 执行当前运行进程所在调度类的task_tick函数进行周期性调度  */
    curr->sched_class->task_tick(rq, curr, 0);

    /*  2.3 更新rq的负载信息,  即就绪队列的cpu_load[]数据
     *  本质是讲数组中先前存储的负荷值向后移动一个位置，
     *  将当前负荷记入数组的第一个位置 */
    update_cpu_load_active(rq);



    /*  2.4 更新cpu的active count活动计数
     *  主要是更新全局cpu就绪队列的calc_load_update*/
    calc_global_load_tick(rq);

    /* 解锁 */
    raw_spin_unlock(&rq->lock);

    /* 与perf计数事件相关 */
    perf_event_task_tick();

#ifdef CONFIG_SMP

     /* 当前CPU是否空闲 */
    rq->idle_balance = idle_cpu(cpu);

    /* 如果到是时候进行周期性负载平衡则触发SCHED_SOFTIRQ */
    trigger_load_balance(rq);

#endif

    rq_last_tick_reset(rq);
}
```



##2.2	更新统计量
-------


| 函数 | 描述 | 定义 |
| ------------- |:-------------:||:-------------:|
| update_rq_clock | 处理就绪队列时钟的更新, 本质上就是增加struct rq当前实例的时钟时间戳 | [sched/core.c, L98](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L98)
| update_cpu_load_active | 负责更新就绪队列的cpu_load数组, 其本质上相当于将数组中先前存储的负荷值向后移动一个位置, 将当前就绪队列的符合记入数组的第一个位置. 另外该函数还引入一些取平均值的技巧, 以确保符合数组的内容不会呈现太多的不联系跳读. | [kernel/sched/fair.c, L4641](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L4641) |
| calc_global_load_tick | 跟新cpu的活动计数, 主要是更新全局cpu就绪队列的calc_load_update |  [kernel/sched/loadavg.c, L382](http://lxr.free-electrons.com/source/kernel/sched/loadavg.c?v=4.6#L378) |


##2.3	激活进程所属调度类的周期性调度器
-------



由于调度器的模块化结构, 主体工程其实很简单, 在更新统计信息的同时, 内核将真正的调度工作委托给了特定的调度类方法

内核先找到了就绪队列上当前运行的进程curr, 然后调用curr所属调度类sched_class的周期性调度方法task_tick


即

```c
curr->sched_class->task_tick(rq, curr, 0);
```



task_tick的实现方法取决于底层的调度器类, 例如完全公平调度器会在该方法中检测是否进程已经运行了太长的时间, 以避免过长的延迟, 注意此处的做法与之前就的基于时间片的调度方法有本质区别, 旧的方法我们称之为到期的时间片, 而完全公平调度器CFS中则不存在所谓的时间片概念.

目前我们的内核中的3个调度器类`struct sched_entity`,  `struct sched_rt_entity`, 和`struct sched_dl_entity` dl, 我们针对当前内核中实现的调度器类分别列出其周期性调度函数task_tick


| 调度器类 | task_tick操作 | task_tick函数定义 |
| ------- |:-------:|:-------:|
| stop_sched_class | | [kernel/sched/stop_task.c, line 77, task_tick_stop](http://lxr.free-electrons.com/source/kernel/sched/stop_task.c?v=4.6#L77) |
| dl_sched_class | | [kernel/sched/deadline.c, line 1192, task_tick_dl](http://lxr.free-electrons.com/source/kernel/sched/deadline.c?v=4.6#L1192)|
| rt_sched_class | | [/kernel/sched/rt.c, line 2227, task_tick_rt](http://lxr.free-electrons.com/source/kernel/sched/rt.c?v=4.6#L2227)|
| fail_sched_class | | [kernel/sched/fair.c, line 8116, task_tick_fail](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L8116) |
| idle_sched_class | | [kernel/sched/idle_task.c, line 53, task_tick_idle](http://lxr.free-electrons.com/source/kernel/sched/idle_task.c?v=4.6#L53) |

*	如果当前进程是**完全公平队列**中的进程, 则首先根据当前就绪队列中的进程数算出一个延迟时间间隔，大概每个进程分配2ms时间，然后按照该进程在队列中的总权重中占得比例，算出它该执行的时间X，如果该进程执行物理时间超过了X，则激发延迟调度；如果没有超过X，但是红黑树就绪队列中下一个进程优先级更高，即curr->vruntime-leftmost->vruntime > X,也将延迟调度


>**延迟调度**的真正调度过程在：schedule中实现，会按照调度类顺序和优先级挑选出一个最高优先级的进程执行


*	如果当前进程是实时调度类中的进程：则如果该进程是SCHED_RR，则递减时间片[为HZ/10]，到期，插入到队列尾部，并激发延迟调度，如果是SCHED_FIFO，则什么也不做，直到该进程执行完成


如果当前进程希望被重新调度, 那么调度类方法会在task_struct中设置TIF_NEED_RESCHED标志, 以表示该请求, 而内核将会在接下来的适当实际完成此请求.


#	3	周期性调度器的激活
-------

##	3.1	定时器周期性的激活调度器
-------

定时器是Linux提供的一种定时服务的机制. 它在某个特定的时间唤醒某个进程，来做一些工作.

在低分辨率定时器的每次时钟中断完成全局统计量更新后, 每个cpu在软中断中执行一下操作

*	更新该cpu上当前进程内核态、用户态使用时间xtime_update
*	调用该cpu上的定时器函数
*	启动周期性定时器（scheduler_tick）完成该cpu上任务的周期性调度工作；

在支持动态定时器的系统中，可以关闭该调度器，从而进入深度睡眠过程；scheduler_tick查看当前进程是否运行太长时间，如果是，将进程的TIF_NEED_RESCHED置位，然后再中断返回时，调用schedule，进行进程切换操作


```c
//  http://lxr.free-electrons.com/source/arch/arm/kernel/time.c?v=4.6#L74
/*
* Kernel system timer support.
*/
void timer_tick(void)
{
    profile_tick(CPU_PROFILING);
    xtime_update(1);
#ifndef CONFIG_SMP
    update_process_times(user_mode(get_irq_regs()));
#endif
}

//  http://lxr.free-electrons.com/source/kernel/time/timer.c?v=4.6#L1409
/*
 * Called from the timer interrupt handler to charge one tick to the current
 * process.  user_tick is 1 if the tick is user time, 0 for system.
 */
void update_process_times(int user_tick)
{
    struct task_struct *p = current;

    /* Note: this timer irq context must be accounted for as well. */
    account_process_tick(p, user_tick);
    run_local_timers();
    rcu_check_callbacks(user_tick);
#ifdef CONFIG_IRQ_WORK
    if (in_irq())
        irq_work_tick();
#endif
    scheduler_tick();
    run_posix_cpu_timers(p);
}
```

##早期实现
-------

Linux初始化时, init_IRQ()函数设定8253的定时周期为10ms(一个tick值). 同样，在初始化时, time_init()用setup_irq()设置时间中断向量irq0, 中断服务程序为timer_interrupt.

在2.4版内核及较早的版本当中, 定时器的中断处理采用底半机制, 底半处理函数的注册在start_kernel()函数中调用sechd_init(), 在这个函数中又调用init_bh(TIMER_BH, timer_bh)注册了定时器的底半处理函数. 然后系统才调用time_init( )来注册定时器的中断向量和中断处理函数.

在中断处理函数timer_interrupt()中，主要是调用do_timer()函数完成工作。do_timer()函数的主要功能就是调用mark_bh()产生软中断，随后处理器会在合适的时候调用定时器底半处理函数timer_bh()。在timer_bh()中, 实现了更新定时器的功能. 2.4.23版的do_timer()函数代码如下（经过简略）：

```c
void do_timer(struct pt_regs *regs)
{
       (*(unsigned long *)&jiffies)++;
       update_process_times(user_mode(regs));
       mark_bh(TIMER_BH);
}
```

而在内核2.6版本以后，定时器中断处理采用了软中断机制而不是底半机制。时钟中断处理函数仍然为timer_interrup()-> do_timer_interrupt()-> do_timer_interrupt_hook()-> do_timer()。不过do_timer()函数的实现有所不同
```c
void do_timer(struct pt_regs *regs)
{
       jiffies_64++;
       update_process_times(user_mode(regs));
       update_times();
}
```

>更详细的实现linux-2.6
>
[Linux中断处理之时钟中断（一）](http://www.bianceng.cn/OS/Linux/201111/31272_4.htm)
>
>[（原创）linux内核进程调度以及定时器实现机制](http://blog.csdn.net/joshua_yu/article/details/591038)



>参考
>
>[进程管理与调度5 -- 进程调度、进程切换原理详解](http://wanderer-zjhit.blogbus.com/logs/156738683.html)