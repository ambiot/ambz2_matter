/**
 * Copyright (c) 2015, Realsil Semiconductor Corporation. All rights reserved.
 */
#include <stdint.h>
#include <string.h>
#include <los_task.h>
#include <los_swtmr.h>
#include <los_queue.h>
#include <los_sem.h>
#include <los_mux.h>
#include <liteos_freertos_like.h>
#include <osif.h>
#include "cmsis.h"

/****************************************************************************/
/* Check if in task context (true), or isr context (false)                  */
/****************************************************************************/
static inline bool osif_task_context_check(void)
{
    return (__get_IPSR() == 0);
}

/****************************************************************************/
/* Delay current task in a given milliseconds                               */
/****************************************************************************/
void osif_delay(uint32_t ms)
{
    UINT32 ret;

    ret = LOS_TaskDelay(LOS_MS2Tick(ms));
    if (ret != LOS_OK) {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
    }
    
}

/****************************************************************************/
/* Get system time in milliseconds                                          */
/****************************************************************************/
uint32_t osif_sys_time_get(void)
{
    return (uint32_t)LOS_Tick2MS(LOS_TickCountGet());
}

/****************************************************************************/
/* Start os kernel scheduler                                                */
/****************************************************************************/
bool osif_sched_start(void)
{
    return true;
}

/****************************************************************************/
/* Stop os kernel scheduler                                                 */
/****************************************************************************/
bool osif_sched_stop(void)
{
    return true;
}

/****************************************************************************/
/* Suspend os kernel scheduler                                              */
/****************************************************************************/
bool osif_sched_suspend(void)
{
    return true;
}

/****************************************************************************/
/* Resume os kernel scheduler                                               */
/****************************************************************************/
bool osif_sched_resume(void)
{
    return true;
}

static inline uint16_t port_helper_prio_l2f(UINT16 priority)
{
    return configMAX_PRIORITIES - priority;
}

static inline UINT16 port_helper_prio_f2l(uint16_t priority)
{
    return configMAX_PRIORITIES - priority;
}

/****************************************************************************/
/* Create os level task routine                                             */
/****************************************************************************/
bool osif_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *),
                      void *p_param, uint16_t stack_size, uint16_t priority)
{
    UINT32 taskid, retval;

    if (pp_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    TSK_INIT_PARAM_S task_param;
    memset(&task_param, 0, sizeof(task_param));
    task_param.pfnTaskEntry = (TSK_ENTRY_FUNC)p_routine;
    task_param.usTaskPrio = port_helper_prio_f2l(priority);
    task_param.uwArg = (UINT32)(uintptr_t)p_param;
    task_param.uwStackSize = (UINT32)stack_size;
    task_param.pcName = (CHAR*)p_name;

    retval = LOS_TaskCreate(&taskid, &task_param);
    if (retval != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,retval);
        return false;
    }

    *pp_handle = OS_TCB_FROM_TID(taskid);
    return true;
}

/****************************************************************************/
/* Delete os level task routine                                             */
/****************************************************************************/
bool osif_task_delete(void *p_handle)
{
    LosTaskCB *pstTaskCB;
    UINT32 ret;
    UINT32 tid;

    if (p_handle == NULL)
    {
        tid = LOS_CurTaskIDGet();
    } else {
        pstTaskCB = (LosTaskCB *)p_handle;
        tid = pstTaskCB->taskID;
    }

    ret = LOS_TaskDelete(tid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Suspend os level task routine                                            */
/****************************************************************************/
bool osif_task_suspend(void *p_handle)
{
    LosTaskCB *pstTaskCB;
    UINT32 ret;
    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    pstTaskCB = (LosTaskCB *)p_handle;

    UINT32 tid = pstTaskCB->taskID;

    ret = LOS_TaskSuspend(tid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Resume os level task routine                                             */
/****************************************************************************/
bool osif_task_resume(void *p_handle)
{
    LosTaskCB *pstTaskCB;
    UINT32 ret;
    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    pstTaskCB = (LosTaskCB *)p_handle;
    UINT32 tid = pstTaskCB->taskID;

    ret = LOS_TaskResume(tid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Yield current os level task routine                                      */
/****************************************************************************/
bool osif_task_yield(void)
{
    LOS_TaskYield();

    return true;
}

/****************************************************************************/
/* Get current os level task routine handle                                 */
/****************************************************************************/
bool osif_task_handle_get(void **pp_handle)
{
    if (*pp_handle == NULL)
    {
        printf("%s fail!(*pp_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    UINT32 tid = LOS_CurTaskIDGet();
    *pp_handle = OS_TCB_FROM_TID(tid);

    return true;
}

/****************************************************************************/
/* Get os level task routine priority                                       */
/****************************************************************************/
bool osif_task_priority_get(void *p_handle, uint16_t *p_priority)
{
    LosTaskCB *pstTaskCB;
    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    if (p_priority == NULL)
    {
        printf("%s fail!(p_priority == NULL)\r\n", __FUNCTION__);
        return false;
    }

    pstTaskCB = (LosTaskCB *)p_handle;
    UINT32 tid = pstTaskCB->taskID;

    UINT16 prio = LOS_TaskPriGet(tid);
    *p_priority = port_helper_prio_l2f(prio);
    return true;
}

/****************************************************************************/
/* Set os level task routine priority                                       */
/****************************************************************************/
bool osif_task_priority_set(void *p_handle, uint16_t priority)
{
    LosTaskCB *pstTaskCB;
    UINT32 ret;
    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    pstTaskCB = (LosTaskCB *)p_handle;
    UINT32 tid = pstTaskCB->taskID;

    ret = LOS_TaskPriSet(tid, port_helper_prio_f2l(priority));
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

static void *sig_handle;
bool osif_signal_init(void)
{
    UINT32 sid, ret;
    ret = LOS_SemCreate(0, &sid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }
    sig_handle = GET_SEM(sid);
    return true;
}

void osif_signal_deinit(void)
{
    LosSemCB *semaphore_id;
    UINT32 ret;

    if (sig_handle != NULL)
    {
        semaphore_id = (LosSemCB *)sig_handle;
        UINT32 sid = semaphore_id->semID;
        ret = LOS_SemDelete(sid);
        if (ret != LOS_OK)
        {
            printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        }
        sig_handle = NULL;
    }
}

/****************************************************************************/
/* Send signal to target task                                               */
/****************************************************************************/
bool osif_task_signal_send(void *p_handle, uint32_t signal)
{
    UINT32 ret;

    if (!sig_handle)
    {
        printf("osif_task_signal_send: sig_handle is null");
        return false;
    }
    LosSemCB *semaphore_id;
    semaphore_id = (LosSemCB *)sig_handle;

    UINT32 sid = semaphore_id->semID;

    ret = LOS_SemPost(sid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;

}

/****************************************************************************/
/* Receive signal in target task                                            */
/****************************************************************************/
bool osif_task_signal_recv(uint32_t *p_handle, uint32_t wait_ms)
{
    UINT32 wait_ticks, ret;

    if (!sig_handle)
    {
        printf("osif_task_signal_recv: sig_handle is null");
        return false;
    }

    wait_ticks = LOS_MS2Tick(wait_ms);

    LosSemCB *semaphore_id;
    semaphore_id = (LosSemCB *)sig_handle;

    UINT32 sid = semaphore_id->semID;

    ret = LOS_SemPend(sid, wait_ticks);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Clear signal in target task                                              */
/****************************************************************************/
bool osif_task_signal_clear(void *p_handle)
{
    return false;
}

/****************************************************************************/
/* Lock critical section                                                    */
/****************************************************************************/
uint32_t osif_lock(void)
{
    UINTPTR int_status = 0U;
    LOS_TaskLock();
    return (uint32_t)int_status;
}

/****************************************************************************/
/* Unlock critical section                                                  */
/****************************************************************************/
void osif_unlock(uint32_t flags)
{
    UINTPTR int_status = (UINTPTR)flags;
    LOS_TaskUnlock();
}

/****************************************************************************/
/* Create counting semaphore                                                */
/****************************************************************************/
bool osif_sem_create(void **pp_handle, uint32_t init_count, uint32_t max_count)
{
    UINT32 sid, ret;

    if (pp_handle == NULL)
    {
        printf("%s fail!(pp_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    // call internal function to use maxCount paramter
    extern UINT32 OsSemCreate(UINT16 count, UINT16 maxCount, UINT32 *semHandle);

    ret = OsSemCreate((UINT16)init_count, (UINT16)max_count, &sid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    *pp_handle = GET_SEM(sid);
    return true;
}

/****************************************************************************/
/* Delete counting semaphore                                                */
/****************************************************************************/
bool osif_sem_delete(void *p_handle)
{
    LosSemCB *semaphore_id;
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    semaphore_id = (LosSemCB *)p_handle;

    UINT32 sid = semaphore_id->semID;

    ret = LOS_SemDelete(sid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }
    return true;
}

/****************************************************************************/
/* Take counting semaphore                                                  */
/****************************************************************************/
bool osif_sem_take(void *p_handle, uint32_t wait_ms)
{
    LosSemCB *semaphore_id;
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    semaphore_id = (LosSemCB *)p_handle;

    UINT32 sid = semaphore_id->semID;
    UINT32 wait_ticks;

    wait_ticks = LOS_MS2Tick(wait_ms);

    ret = LOS_SemPend(sid, wait_ticks);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Give counting semaphore                                                  */
/****************************************************************************/
bool osif_sem_give(void *p_handle)
{
    LosSemCB *semaphore_id;
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    semaphore_id = (LosSemCB *)p_handle;

    UINT32 sid = semaphore_id->semID;

    ret = LOS_SemPost(sid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Create recursive mutex                                                   */
/****************************************************************************/
bool osif_mutex_create(void **pp_handle)
{
    UINT32 mid, ret;

    if (pp_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    ret = LOS_MuxCreate(&mid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    *pp_handle = GET_MUX(mid);
    return true;
}

/****************************************************************************/
/* Delete recursive mutex                                                   */
/****************************************************************************/
bool osif_mutex_delete(void *p_handle)
{
    LosMuxCB * mux_id;
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    mux_id = (LosMuxCB *)p_handle;
    UINT32 mid = mux_id->muxID;

    ret = LOS_MuxDelete(mid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }
    return true;
}

/****************************************************************************/
/* Take recursive mutex                                                     */
/****************************************************************************/
bool osif_mutex_take(void *p_handle, uint32_t wait_ms)
{
    LosMuxCB * mux_id;
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    mux_id = (LosMuxCB *)p_handle;
    UINT32 mid = mux_id->muxID;
    UINT32 wait_ticks;

    wait_ticks = LOS_MS2Tick(wait_ms);

    ret = LOS_MuxPend(mid, wait_ticks);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Give recursive mutex                                                     */
/****************************************************************************/
bool osif_mutex_give(void *p_handle)
{
    LosMuxCB * mux_id;
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    mux_id = (LosMuxCB *)p_handle;
    UINT32 mid = mux_id->muxID;

    ret = LOS_MuxPost(mid);
    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Create inter-thread message queue                                        */
/****************************************************************************/
bool osif_msg_queue_create(void **pp_handle, uint32_t msg_num, uint32_t msg_size)
{
    if (pp_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    *pp_handle = osMessageQueueNew(msg_num, msg_size, NULL);
    if (*pp_handle == NULL)
    {
        printf("%s fail!(*pp_handle == NULL)\n", __FUNCTION__);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Delete inter-thread message queue                                        */
/****************************************************************************/
bool osif_msg_queue_delete(void *p_handle)
{
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    ret = osMessageQueueDelete(p_handle);
    if (ret != osOK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Peek inter-thread message queue's pending but not received msg number    */
/****************************************************************************/
bool osif_msg_queue_peek(void *p_handle, uint32_t *p_msg_num)
{
    if (p_handle == NULL || p_msg_num == NULL)
    {
        printf("%s fail!(p_handle == NULL || p_msg_num == NULL)\r\n", __FUNCTION__);
        return false;
    }

    *p_msg_num = osMessageQueueGetCount(p_handle);

    return true;
}

/****************************************************************************/
/* Send inter-thread message                                                */
/****************************************************************************/
bool osif_msg_send(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    UINT32 ret;

    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    UINT32 wait_ticks;

    if (wait_ms == 0xFFFFFFFFUL)
    {
        wait_ticks = LOS_WAIT_FOREVER;
    }
    else
    {
        wait_ticks = LOS_MS2Tick(wait_ms);
    }

    ret = osMessageQueuePut(p_handle, p_msg, 0, wait_ticks);

    if (ret != osOK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Receive inter-thread message                                             */
/****************************************************************************/
bool osif_msg_recv(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    UINT32 ret;
    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    UINT32 wait_ticks;

    if (wait_ms == 0xFFFFFFFFUL)
    {
        wait_ticks = LOS_WAIT_FOREVER;
    }
    else
    {
        wait_ticks = LOS_MS2Tick(wait_ms);
    }

    ret = osMessageQueueGet(p_handle, p_msg, 0, wait_ticks);

    if (ret != osOK)
    {
        return false;
    }

    return true;
}

/****************************************************************************/
/* Peek inter-thread message                                                */
/****************************************************************************/
bool osif_msg_peek(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    return true;
}

uint32_t osif_msg_queue_get_space(void *p_handle)
{
    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return 0;
    }

    return osMessageQueueGetSpace(p_handle);
}

uint32_t osif_msg_queue_get_capacity(void *p_handle)
{
    if (p_handle == NULL)
    {
        printf("%s fail!(p_handle == NULL)\r\n", __FUNCTION__);
        return 0;
    }

    return osMessageQueueGetCapacity(p_handle);
}

/****************************************************************************/
/* Allocate memory                                                          */
/****************************************************************************/
void *osif_mem_alloc(RAM_TYPE ram_type, size_t size)
{
    (void) ram_type;
    return LOS_MemAlloc(m_aucSysMem0, size);
}

/****************************************************************************/
/* Allocate aligned memory                                                  */
/****************************************************************************/
void *osif_mem_aligned_alloc(RAM_TYPE ram_type, size_t size, uint8_t alignment)
{
    (void) ram_type;
    void *p;
    void *p_aligned;

    if (alignment == 0)
    {
        alignment = 8;
    }

    p = LOS_MemAlloc(m_aucSysMem0, size + sizeof(void *) + alignment);
    if (p == NULL)
    {
        printf("%s fail!(p == NULL)\r\n", __FUNCTION__);
        return p;
    }

    p_aligned = (void *)(((size_t)p + sizeof(void *) + alignment) & ~(alignment - 1));

    memcpy((uint8_t *)p_aligned - sizeof(void *), &p, sizeof(void *));

    return p_aligned;
}

/****************************************************************************/
/* Free memory                                                              */
/****************************************************************************/
void osif_mem_free(void *p_block)
{
    UINT32 ret;

    if (p_block == NULL)
    {
        printf("%s fail!(p_block == NULL)\r\n", __FUNCTION__);
        return;
    }

    ret = LOS_MemFree(m_aucSysMem0, p_block);
    if (ret != LOS_OK) 
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
    }
}

/****************************************************************************/
/* Free aligned memory                                                      */
/****************************************************************************/
void osif_mem_aligned_free(void *p_block)
{
    void *p;
    UINT32 ret;

    if (p_block == NULL)
    {
        printf("%s fail!(p_block == NULL)\r\n", __FUNCTION__);
        return;
    }
    memcpy(&p, (uint8_t *)p_block - sizeof(void *), sizeof(void *));

    ret = LOS_MemFree(m_aucSysMem0, p);
    if (ret != LOS_OK) 
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
    }
}

/****************************************************************************/
/* Peek unused (available) memory size                                    */
/****************************************************************************/
size_t osif_mem_peek(RAM_TYPE ram_type)
{
    (void) ram_type;
    LOS_MEM_POOL_STATUS info;
    LOS_MemInfoGet(m_aucSysMem0, &info);

    return info.totalFreeSize;
}

typedef struct osif_timer_handle {
    SWTMR_CTRL_S *pstSwtmr;
    uint32_t timer_id;
} OSIF_TIMER_HANDLE_S;

/****************************************************************************/
/* Get software timer ID                                                    */
/****************************************************************************/
bool osif_timer_id_get(void **pp_handle, uint32_t *p_timer_id)
{
    OSIF_TIMER_HANDLE_S *p_osif_timer_handle;

    if (pp_handle == NULL || *pp_handle == NULL)
    {
        printf("%s fail!(pp_handle == NULL || *pp_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    p_osif_timer_handle = *pp_handle;
    *p_timer_id = p_osif_timer_handle->timer_id;

    return true;
}

/****************************************************************************/
/* Create software timer                                                    */
/****************************************************************************/
bool osif_timer_create(void **pp_handle, const char *p_timer_name, uint32_t timer_id,
                       uint32_t interval_ms, bool reload, void (*p_timer_callback)(void *))
{
    UINT32 timer_ticks;
    SWTMR_CTRL_S *pstSwtmr;
    OSIF_TIMER_HANDLE_S *p_osif_timer_handle;
    if (pp_handle == NULL || p_timer_callback == NULL)
    {
        printf("%s fail!(pp_handle == NULL || p_timer_callback == NULL)\r\n", __FUNCTION__);
        return false;
    }

    if (*pp_handle == NULL)
    {
        p_osif_timer_handle = osif_mem_alloc(0, sizeof(OSIF_TIMER_HANDLE_S));
        if (p_osif_timer_handle == NULL) {
            printf("%s fail!(p_osif_timer_handle == NULL)\r\n", __FUNCTION__);
            return false;
        }

        pstSwtmr = osTimerNew(p_timer_callback, reload ? osTimerPeriodic : osTimerOnce, p_osif_timer_handle, NULL);
        if (pstSwtmr == NULL)
        {
            osif_mem_free(p_osif_timer_handle);
            printf("%s fail!(pstSwtmr == NULL)\r\n", __FUNCTION__);
            return false;
        }
    }
    else
    {
        printf("%s fail!(*pp_handle != NULL)\r\n", __FUNCTION__);
        return false;
    }

    timer_ticks = LOS_MS2Tick(interval_ms);
    pstSwtmr->uwInterval = timer_ticks;
    p_osif_timer_handle->pstSwtmr = pstSwtmr;
    p_osif_timer_handle->timer_id = timer_id;
    *pp_handle = p_osif_timer_handle;

    return true;
}

/****************************************************************************/
/* Start software timer                                                     */
/****************************************************************************/
bool osif_timer_start(void **pp_handle)
{
    UINT32 ret;
    SWTMR_CTRL_S *pstSwtmr;
    OSIF_TIMER_HANDLE_S *p_osif_timer_handle;

    if (pp_handle == NULL || *pp_handle == NULL)
    {
        printf("%s fail!(pp_handle == NULL || *pp_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    p_osif_timer_handle = *pp_handle;
    pstSwtmr = p_osif_timer_handle->pstSwtmr;
    ret = LOS_SwtmrStart(pstSwtmr->usTimerID);

    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Restart software timer                                                   */
/****************************************************************************/
bool osif_timer_restart(void **pp_handle, uint32_t interval_ms)
{
    UINT32 timer_ticks;
    SWTMR_CTRL_S *pstSwtmr;
    OSIF_TIMER_HANDLE_S *p_osif_timer_handle;
    UINT32 ret;

    if (pp_handle == NULL || *pp_handle == NULL)
    {
        printf("%s fail!(pp_handle == NULL || *pp_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    p_osif_timer_handle = *pp_handle;
    pstSwtmr = p_osif_timer_handle->pstSwtmr;
    timer_ticks = LOS_MS2Tick(interval_ms);
    LOS_SwtmrStop(pstSwtmr->usTimerID);
    pstSwtmr->uwInterval = timer_ticks;
    ret = LOS_SwtmrStart(pstSwtmr->usTimerID);

    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Stop software timer                                                      */
/****************************************************************************/
bool osif_timer_stop(void **pp_handle)
{
    UINT32 ret;
    SWTMR_CTRL_S *pstSwtmr;
    OSIF_TIMER_HANDLE_S *p_osif_timer_handle;

    if (pp_handle == NULL || *pp_handle == NULL)
    {
        printf("%s fail!(pp_handle == NULL || *pp_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    p_osif_timer_handle = *pp_handle;
    pstSwtmr = p_osif_timer_handle->pstSwtmr;
    ret = LOS_SwtmrStop(pstSwtmr->usTimerID);

    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    return true;
}

/****************************************************************************/
/* Delete software timer                                                    */
/****************************************************************************/
bool osif_timer_delete(void **pp_handle)
{
    UINT32 ret;
    SWTMR_CTRL_S *pstSwtmr;
    OSIF_TIMER_HANDLE_S *p_osif_timer_handle;

    if (pp_handle == NULL || *pp_handle == NULL)
    {
        return false;
    }

    p_osif_timer_handle = *pp_handle;
    pstSwtmr = p_osif_timer_handle->pstSwtmr;
    ret = LOS_SwtmrDelete(pstSwtmr->usTimerID);

    if (ret != LOS_OK)
    {
        printf("%s fail!(err code = %d)\r\n", __FUNCTION__,ret);
        return false;
    }

    osif_mem_free(p_osif_timer_handle);
    *pp_handle = NULL;

    return true;
}

/****************************************************************************/
/* Get timer state                                                          */
/****************************************************************************/
bool osif_timer_state_get(void **pp_handle, uint32_t *p_timer_state)
{
    SWTMR_CTRL_S *pstSwtmr;
    OSIF_TIMER_HANDLE_S *p_osif_timer_handle;

    if (pp_handle == NULL || *pp_handle == NULL)
    {
        printf("%s fail!(pp_handle == NULL || *pp_handle == NULL)\r\n", __FUNCTION__);
        return false;
    }

    p_osif_timer_handle = *pp_handle;
    pstSwtmr = p_osif_timer_handle->pstSwtmr;
    *p_timer_state = (pstSwtmr->ucState == OS_SWTMR_STATUS_TICKING) ? 1 :0;

    return true;
}

/****************************************************************************/
/* Dump software timer                                                      */
/****************************************************************************/
bool osif_timer_dump(void)
{
    return true;
}

#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
extern void rtw_create_secure_context(u32 secure_stack_size);
#endif
void osif_create_secure_context(uint32_t size)
{
#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
    rtw_create_secure_context(size);
#endif
}
