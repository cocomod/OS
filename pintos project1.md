# pintos project1
## 一、mission1
### 1、思路概述
**Pintos中timer_sleep()函数的思路：**

线程不断在cpu就绪队列和running队列之间来回，线程不停地循环，直到时间片耗尽，占用了cpu资源。改用唤醒机制来实现这个函数。

**实现思路：**

调用timer_sleep的时候直接把线程阻塞掉，然后给线程结构体加一个成员ticks_blocked来记录这个线程被sleep了多少时间， 然后利用操作系统自身的时钟中断（每个tick会执行一次）加入对线程状态的检测， 每次检测将ticks_blocked减1, 如果减到0就唤醒这个线程。

### 2、具体修改内容
**（1）thread.h 结构体加ticks_blocked变量**
设置ticks_blocked表示线程是否应该被唤醒（0表示被被唤醒，否则阻塞）。
**（2）thread.c 增加blocked_thread_check()，修改thread_create()**
void blocked_thread_check(struct thread *t, void *aux){
	if (t->status == THREAD_BLOCKED && t->ticks_blocked > 0){
	t->ticks_blocked--;
	if (t->ticks_blocked == 0)
	thread_unblock(t);
	}
}
thread_create()将ticks_blocked初始化为0。
**（3）Timer.c 修改timer_sleep()函数，修改timer_interrupt()函数**
Void timer_sleep(int64_t ticks){
	if(ticks <= 0) return;//非阻塞情况
	ASSERT (intr_get_level () == INTR_ON);
	enum intr_level old_level = intr_disable ();//获取原中断状态，关闭当前中断状态，返回执行之前的状态。
	struct thread *current_thread = thread_current ();//返回当前线程起始指针位置
	current_thread->ticks_blocked = ticks;
	thread_block ();//使线程阻塞
	intr_set_level (old_level);
}
timer_interrupt()中添加thread_foreach函数，保证每个线程都调用blocked_thread_check()函数。
### 3、运行截图
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190615162048937.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2NvY29tb2Q=,size_16,color_FFFFFF,t_70)
### 4、遇到的问题
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190615162209538.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2NvY29tb2Q=,size_16,color_FFFFFF,t_70)![在这里插入图片描述](https://img-blog.csdnimg.cn/20190615162220147.jpg)
解决办法：打开“/pintos/src/tests/Make.tests”编辑第54行，删除‘-v’。

## 二、Mission2
### 1、通过alarm_priority
**（1）思路概述**

插入线程到就绪队列的时候保证这个队列是一个优先级队列，原来是将线程丢到队列尾部，改为将线程丢到队首，因为调度的时候下一个线程是直接取队头的

**（2）修改thread_unblock（）函数**

把list_push_back改成list_insert_ordered (&ready_list, &t->elem, (list_less_func *) &thread_cmp_priority, NULL);

**（3）实现thread_cmp_priority（）函数**

bool thread_cmp_priority (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED){
	return list_entry(a, struct thread, elem)->priority > list_entry(b, struct thread, elem)->priority;
}

**（4）修改thread_yield（）和thread_init（）函数**

thread_init（）修改list_push_back为list_insert_ordered (&all_list, &t->allelem, (list_less_func *) &thread_cmp_priority, NULL);
thread_yield（）修改list_push_back为list_insert_ordered (&ready_list, &cur->elem, (list_less_func *) &thread_cmp_priority, NULL);

### 2、通过priority_change和priority_preempt
**（1）问题分析**

thread1创建了一个PRI_DEFAULT+1优先级的内核线程thread2，因为thread2优先级高，所以线程直接切换到thread2，使thread1阻塞，然后thread2执行的时候调用changing_thread，把自身优先级调为PRI_DEFAULT-1,此时thread1的优先级就大于thread2了，thread2阻塞于最后一个msg输出，线程再切换到thread1，然后thread1又把自己优先级改成PRI_DEFAULT-2。这样thread2又高于thread1了，执行thread2，然后再输出thread1的msg。

**（2）思路概述**

每次设置一个线程优先级时都要重新考虑所有线程执行顺序，重新安排执行顺序。在创建线程的时，如果新创建的线程比主线程优先级高的话应当调用thread_yield。

**（3）实现thread_set_priority（）函数**

thread_set_priority (int new_priority){
	thread_current ()->priority = new_priority;
	thread_yield ();
}

**（4）修改thread_create（）函数**

Unblock线程之后增加代码：
if (thread_current ()->priority < priority){
	thread_yield ();
}

### 3、优先级翻转
**（1）问题分析**
假设对于三个线程（A,B,C）优先级分别为A<B<C，线程A和B在就绪队列中，线程A中有一个互斥资源。假定线程C也需要访问这个互斥资源，此时线程C不能进入就绪队列，而只能等待这个资源释放。当调度器开始调度时，它只能在A和B中选择优先级高的线程B进行调度，因此线程B将会首先运行。这样，本来线程C优先级高于线程B，但是线程B却先于线程C运行，从而产生了优先级翻转问题。


