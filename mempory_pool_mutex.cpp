//
// Created by swx on 23-8-17.
//

#include "mempory_pool_mutex.h"
#include <assert.h>

Slot *MemoryPool::allocate() {
    if (freeSlot_ != nullptr) {  //有回收的空间可以分配
        std::lock_guard<std::mutex> lock(freeSlotMutex_);
        if (freeSlot_ != nullptr) {  //双重校验！！！ 不只是适用于单例模式   //如果这里校验通不过，自然会往后走，进入无freeSlot_的模式
            //拿出第一个（当前的freeSlot_）
            auto res = freeSlot_;
            freeSlot_ = freeSlot_->next;
            return res;
        }
    }
    //没有回收的空间可以分配，那么就看下空间分配完没有   currentSlot_
//    如果没有分配完 ， 那么就分配；否则就申请新的空间，然后分配
    std::lock_guard<std::mutex> lock2(currentSlotAndNewBlockMutex_);

    //然后slotSize_ >>3 应该是sizeof(Slot*)才更合适一些
    while (currentSlot_ >=
           lastSlot_) {   //虽然这里不太可能，不过感觉确实应该设计成while，防止一个block分配就够一个slot使用的极端情况的发生的使用 //存疑？？？，while虽然不会死循环，但是好像确实没有什么意义
        getOneNewBlock();
    }
    Slot *useSlot = currentSlot_;
    currentSlot_ += slotSize_ / sizeof(decltype(currentSlot_)); //因为currentSlot_是slot*类型，因此每次+1都是移动的slot*的大小，因此需要移动实际分配大小（slotSize_）/sizeof（slot*）
    return useSlot;
}
//外层加锁了
void MemoryPool::getOneNewBlock() {
    char *newBlockAddr = reinterpret_cast<char *>(operator new(BlockSize));//转成char*是因为char长度为1字节，是最小单位，方便计算而已
    char *bodyAddr = newBlockAddr + sizeof(Slot *);
    assert((sizeof (Slot*))<=slotSize_);
    size_t bodyPadding = 1;   //todo
    Slot *useSlot = nullptr;
    //头插入新申请的
    reinterpret_cast<Slot*>(newBlockAddr)->next = currentBlock_;
    currentBlock_ = reinterpret_cast<Slot*>(newBlockAddr);
    currentSlot_ = reinterpret_cast<Slot*>(newBlockAddr+slotSize_) ;
    lastSlot_ = reinterpret_cast<Slot*>( newBlockAddr+BlockSize-slotSize_+1); //赋予一个不可能的地址
}
//函数隐去了，不过里面链接里面的技巧是真的牛
//addr要补齐 结果 个字节才能达到一个alignSize的空隙，空隙是指从newBlockAddr~第一个可用的slot的地址
//size_t MemoryPool::padPointer(char *addr, size_t alignSize) {
//    static_assert((alignSize & (alignSize - 1)) == 0, "alignSize size should be a power of 2");  //align & (align - 1)判断align是否是2的指数，好巧妙。
//
////    这里原来的逻辑没看懂，但是看到了一个很牛皮的做法：https://zhuanlan.zhihu.com/p/93160877
////* align & (align - 1)判断align是否是2的指数，好巧妙。
////* reinterpret_cast：类似于强制类型转化，获取aooloc_ptr_的内存地址
////* (alloc_ptr_) & (align - 1) 等价于 alloc_ptr_%align,取余
////* 如果取余不为0，则分配更多的空间，使对齐
////真的牛皮呀！！！
//    auto addrSize_t = static_cast<size_t>(addr);
//                                //(alloc_ptr_) & (align - 1) 等价于 alloc_ptr_%align,取余
//    return ()
//}

void MemoryPool::deAllocate(Slot *pSlot) {
    if(pSlot == nullptr){
        return ;
    }
    std::lock_guard<std::mutex> lock1(freeSlotMutex_);
    pSlot->next = freeSlot_;
    freeSlot_ = pSlot;
}

void MemoryPool::init(int slotSize) {
    assert(slotSize>0);
    slotSize_ = slotSize;
    currentBlock_ = nullptr;
    currentSlot_ = nullptr;
    lastSlot_ = nullptr;
    freeSlot_ = nullptr;
}

MemoryPool::MemoryPool() {

}

MemoryPool::~MemoryPool() {
    Slot *cur = currentBlock_;
    while(cur!= nullptr){
        Slot *next = cur->next;
        operator delete(reinterpret_cast<void*> (cur));
        cur = next  ;
    }
}


//implement
extern MemoryPool& get_MemoryPool(size_t slotIndex){
    static MemoryPool memoryPool_[64];   //todo：单例设计模式
    return memoryPool_[slotIndex];
}

void init_MemoryPool() {
    for (int i = 0; i < 64; ++i) {
        get_MemoryPool(i).init((i + 1) << 3);
    }
}

void *use_Memory(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    if (size > 512) {
        return operator new(size);
    }
    // 相当于(size / 8)向上取整,即 1~8字节-》0槽；9~16字节-》1槽；17~24字节-》2槽
//    简单总结过后公式显然是(size-1)/8 ，所以随想录中的反而搞得复杂，然后槽 8 16 24 32
    size_t memoryPoolIndex = (size - 1) / 8;
    MemoryPool &memoryPool = get_MemoryPool(memoryPoolIndex);
    return reinterpret_cast<void *>(memoryPool.allocate());
}


void free_Memory(size_t size, void *addr) {
    if (size == 0 || addr == nullptr) {
        return;
    }
    if (size > 512) {
        operator delete(addr);
    }
    //同use_Memory
    size_t memoryPoolIndex = (size - 1) / 8;
    MemoryPool &memoryPool = get_MemoryPool(memoryPoolIndex);
    memoryPool.deAllocate(reinterpret_cast<Slot *>(addr));
    return;

}