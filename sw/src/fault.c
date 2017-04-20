/*
 * fault.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "fault.h"
#include "bldc.h"

void fault_general_failure(void)
{
  while (1) {
    bldc_safe_state();
  }
}

void TIM1_BRK_TIM9_IRQHandler(TIM_HandleTypeDef *htim)
{
  fault_general_failure();
}

#if 0
static volatile uint32_t r0;
static volatile uint32_t r1;
static volatile uint32_t r2;
static volatile uint32_t r3;
static volatile uint32_t r12;
static volatile uint32_t lr;
static volatile uint32_t pc;
static volatile uint32_t psr;

static volatile uint32_t hard_fault_status;
static volatile uint32_t memory_bus_usage_fault_status;
static volatile uint32_t aux_fault_status;
static volatile uint32_t bus_fault_address; //0x20199860
static volatile uint32_t memory_fault_address;


void fault_get_status_registers(void)
{
  hard_fault_status = SCB->HFSR;
  memory_bus_usage_fault_status = SCB->CFSR;
  aux_fault_status = SCB->AFSR;
  bus_fault_address = SCB->BFAR;
  memory_fault_address = SCB->MMFAR;
}

void fault_get_reg_from_stack(uint32_t *stack_address)
{
  r0  = stack_address[0];
  r1  = stack_address[1];
  r2  = stack_address[2];
  r3  = stack_address[3];
  r12 = stack_address[4];
  lr  = stack_address[5];
  pc  = stack_address[6];
  psr = stack_address[7];
}

inline void fault_get_stack_state(void)
{
  __asm volatile
  (
    " tst lr, #4                                             \n"
    " ite eq                                                 \n"
    " mrseq r0, msp                                          \n"
    " mrsne r0, psp                                          \n"
    " ldr r1, [r0, #24]                                      \n"
    " ldr r2, handler2_address_const                         \n"
    " bx r2                                                  \n"
    " handler2_address_const: .word fault_get_reg_from_stack \n"
  );
}

#else
inline void fault_get_stack_state(void) {
}
void fault_get_status_registers(void) {
}
#endif

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
  /*
   * A NonMaskable Interrupt (NMI) can be signalled by a peripheral or triggered by software.
   * This is the highest priority exception other than reset.
   * It is permanently enabled and has a fixed priority of -2.
   * NMIs cannot be:
   * - Masked or prevented from activation by any other exception
   * - Preempted by any exception other than Reset.
   */
  fault_get_stack_state();
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /*
   * A hard fault is an exception that occurs because of an error during exception processing,
   * or because an exception cannot be managed by any other exception mechanism.
   * Hard faults have a fixed priority of -1, meaning they have higher priority than any
   * exception with configurable priority.
   */
  fault_get_stack_state();

  fault_get_status_registers();

  fault_general_failure();
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /*
   * A memory management fault is an exception that occurs because of a memory protection related fault.
   * The MPU or the fixed memory protection constraints determines this fault, for both instruction and data memory transactions.
   * This fault is used to abort instruction accesses to Execute Never (XN) memory regions.
   */
  fault_get_stack_state();

  fault_get_status_registers();

  fault_general_failure();
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /*
   * A bus fault is an exception that occurs because of a memory related fault for an instruction or data memory transaction.
   * This might be from an error detected on a bus in the memory system.
   */
  fault_get_stack_state();

  fault_get_status_registers();

  fault_general_failure();
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /*
   * A usage fault is an exception that occurs because of a fault related to instruction execution.
   * This includes:
   * - An undefined instruction
   * - An illegal unaligned access
   * - Invalid state on instruction execution
   * - An error on exception return.
   * The following can cause a usage fault when the core is configured to report them:
   * - An unaligned address on word and halfword memory access
   * - Division by zero
   */
  fault_get_stack_state();

  fault_get_status_registers();
  fault_general_failure();
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}
