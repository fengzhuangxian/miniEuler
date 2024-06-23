#ifndef OS_CPU_ARMV8_EXTERNAL_H
#define OS_CPU_ARMV8_EXTERNAL_H

extern uintptr_t PRT_HwiUnLock(void);
extern uintptr_t PRT_HwiLock(void);
extern void PRT_HwiRestore(uintptr_t intSave);

#define OsIntUnLock() PRT_HwiUnLock()
#define OsIntLock()   PRT_HwiLock()
#define OsIntRestore(intSave) PRT_HwiRestore(intSave)

#endif