//
// Created by swx on 23-8-17.
//

#ifndef MEMORY_POOL_MEMPORY_POOL_MUTEX_H
#define MEMORY_POOL_MEMPORY_POOL_MUTEX_H
#include <mutex>
#include <shared_mutex>


#define  BlockSize 4096


struct Slot{
    Slot *next;
};
class MemoryPool{
public:
    MemoryPool();
    ~MemoryPool();

    Slot* allocate();


    void deAllocate(Slot *pSlot);

    void init(int i);

private:
    size_t slotSize_; //// 每个槽所占字节数

    Slot *currentBlock_;  // 内存块链 表 的头指针
    Slot *currentSlot_; // 元素链表的头指针
    Slot *lastSlot_; // 可存放元素的最后指针+1,即一个正常分配的块不能到达的地址，用于判断当前block是否使用完毕， currentSlot_>=lastSlot_即代表使用完毕
    Slot *freeSlot_; // 元素构造后释放掉的内存链表头指针

    std::mutex freeSlotMutex_;
    std::mutex currentSlotAndNewBlockMutex_;
private:
//    Slot* allocateBlock(); // 申请内存块放进内存池
    void getOneNewBlock();
//    size_t padPointer(char *addr,size_t alignSize);
};

//implement
MemoryPool& get_MemoryPool(size_t slotIndex){
    static MemoryPool memoryPool_[64];   //todo：单例设计模式
    return memoryPool_[slotIndex];
}


void* use_Memory(size_t size);
void free_Memory(size_t size,void *addr);

    template<typename T, typename... Args>
    T *newElement(Args &&... args) {
        T *p = nullptr;
        p = reinterpret_cast<T *>(use_Memory(sizeof(T)));   //todo 完美转发
        if (p == nullptr) {
            return p;
        } else {
//        plain new
            return new(p) T(std::forward<Args>(args)...);
        }
    }

// 调⽤p的析构函数，然后将其总内存池中释放
    template<typename T>
    void deleteElement(T *p) {
        if (p == nullptr) {
            return;
        }
        p->~T(); //析构
        free_Memory(sizeof(T), reinterpret_cast<void *>(p));;
    }


    void init_MemoryPool(){
        for(int i = 0;i<64;++i){
            get_MemoryPool(i).init((i+1)<<3);
        }
    }





void *use_Memory(size_t size) {
    if(size == 0){
        return nullptr;
    }
    if(size > 512){
        return operator new(size);
    }
    // 相当于(size / 8)向上取整,即 1~8字节-》0槽；9~16字节-》1槽；17~24字节-》2槽
//    简单总结过后公式显然是(size-1)/8 ，所以随想录中的反而搞得复杂，然后槽 8 16 24 32
    size_t memoryPoolIndex = (size-1)/8;
    MemoryPool& memoryPool = get_MemoryPool(memoryPoolIndex);
    return reinterpret_cast<void *>(memoryPool.allocate());
}

void free_Memory(size_t size, void *addr) {
    if(size==0 || addr == nullptr){
        return ;
    }
    if(size > 512){
        operator delete(addr);
    }
    //同use_Memory
    size_t memoryPoolIndex = (size-1)/8;
    MemoryPool& memoryPool = get_MemoryPool(memoryPoolIndex);
    memoryPool.deAllocate(reinterpret_cast<Slot*>(addr));
    return ;

}

#endif //MEMORY_POOL_MEMPORY_POOL_MUTEX_H
