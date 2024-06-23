#ifndef OS_CPU_ARMV8_EXTERNAL_H
#define OS_CPU_ARMV8_EXTERNAL_H

extern uintptr_t PRT_HwiUnLock(void);
extern uintptr_t PRT_HwiLock(void);
extern void PRT_HwiRestore(uintptr_t intSave);

#define OsIntUnLock() PRT_HwiUnLock()
#define OsIntLock()   PRT_HwiLock()
#define OsIntRestore(intSave) PRT_HwiRestore(intSave)

#define OS_TSK_STACK_SIZE_ALIGN  16U
#define OS_TSK_STACK_SIZE_ALLOC_ALIGN 4U //按2的幂对齐，即2^4=16字节
#define OS_TSK_STACK_ADDR_ALIGN  16U

extern void OsTaskTrap(void);
extern void OsTskContextLoad(uintptr_t stackPointer);

#endif