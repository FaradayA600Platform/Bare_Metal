#ifndef SYSTEM_H
#define SYSTEM_H


#define __save_flags_cli(x)                                     \
	({                                                      	\
		unsigned long temp;                             		\
        __asm__ __volatile__(                                   \
        	"mrs    %0, cpsr                @ save_flags_cli\n"     \
			"orr     %1, %0, #192\n"                                 \
			"msr     cpsr_c, %1"                                     \
        	: "=r" (x), "=r" (temp)                                 \
        	:                                                       \
        	: "memory");                                            \
        	})



#define __restore_flags(x)                                      \
		__asm__ __volatile__(                                   \
			"msr    cpsr_c, %0              @ restore_flags\n"      \
			:                                                       \
			: "r" (x)                                               \
			: "memory")


#define spin_lock_init(lock)    do { } while(0)
#define spin_lock(lock)         (void)(lock) /* Not "unused variable". */
#define spin_is_locked(lock)    (0)
#define spin_trylock(lock)      ({1; })
#define spin_unlock_wait(lock)  do { } while(0)
#define spin_unlock(lock)       do { } while(0)

#define spin_lock_irqsave(lock, flags)          do { __save_flags_cli(flags);       spin_lock(lock); } while (0)
#define spin_unlock_irqrestore(lock, flags)     do { spin_unlock(lock);  __restore_flags(flags); } while (0)


#endif // SYSTEM_H