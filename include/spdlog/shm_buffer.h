#pragma once
#include <cstdio>
#include <time.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/bundled/format.h"

FMT_BEGIN_NAMESPACE
 
enum
{
    init_shm_size = 1024 * 1024 * 4,
    mmap_block_size = 4096
};

template<typename T, size_t SIZE = init_shm_size, typename Allocator = std::allocator<T>>
class basic_shm_buffer final : public detail::buffer<T>
{
private:
    uint64_t block_pos = 0;
    uint64_t write_pos = 0;

    uint64_t init_size = 0;
    uint64_t mmap_blocks = 0;
    FILE *log_file = nullptr;
    int32_t log_fd = 0;
    T *mmap_file = nullptr;
    std::string file_path;

    void init_mmap_file( const std::string & filePath = "/dev/shm/fmt.log",  uint32_t initSize = mmap_block_size)
    {
        
        file_path = filePath;
        log_file = fopen(filePath.c_str(), "ab+");
        // fseek(log_file,0,SEEK_END);
        // fseek(log_file,0,0);
        log_fd = fileno(log_file);
        init_size = initSize;
        mmap_blocks = mmap_block_size;
        ftruncate(log_fd, initSize);
        block_pos = 0;
        mmap_file = (char *)mmap(NULL, mmap_block_size, PROT_WRITE, MAP_FILE | MAP_SHARED, log_fd, 0);
    }

    // Deallocate memory allocated by the buffer.
    FMT_CONSTEXPR20 void deallocate()
    {
        if (mmap_file != nullptr)
        {
            munmap(mmap_file, mmap_block_size);
            ftruncate(log_fd, write_pos);
            if (log_file != nullptr)
            {
                fclose(log_file);
                log_file = nullptr;
            }
        }
    }

    bool remap(int64_t pos)
    {
        // printf("remap file  blocks %lu\n",  mmap_blocks);
        munmap(mmap_file, mmap_block_size);
        // printf("truncate file size %d\n", mmap_blocks + mmap_block_size);
        if (mmap_blocks + mmap_block_size > init_size)
        {
            int ret = ftruncate(log_fd, mmap_blocks + mmap_block_size);
            if (ret != 0)
            {
                perror("truncate file failed");
                return false;
            }
        }

        // fseek(log_file,0,pos );
        block_pos = 0;
        mmap_file = (char *)mmap(NULL, mmap_block_size, PROT_WRITE, MAP_FILE | MAP_SHARED, log_fd, mmap_blocks);
        if (mmap_file == MAP_FAILED)
        {
            return false;
        }

        mmap_blocks += mmap_block_size;
        return true;
    }

protected:
    FMT_CONSTEXPR20 void grow(size_t size) override;

public:
    using value_type = T;
    using const_reference = const T &;

    FMT_CONSTEXPR20 explicit basic_shm_buffer()
    {
        init_mmap_file();
        this->set(mmap_file, SIZE);
        if (detail::is_constant_evaluated())
        {
            detail::fill_n(mmap_file, SIZE, T{});
        }
    }
    FMT_CONSTEXPR20 ~basic_shm_buffer()
    {
        deallocate();
    }

private:
    // Move data from other to this buffer.
    FMT_CONSTEXPR20 void move(basic_shm_buffer &other)
    {
        if (this != &other)
        {
            this->set(other.data(), other.size());
            other.set(nullptr, 0);

            block_pos = other.block_pos;
            write_pos = other.write_pos;
            init_size = other.init_size;
            mmap_blocks = other.mmap_blocks;
            log_file = other.log_file;
            log_fd = other.log_fd;
            mmap_file = other.mmap_file;
            file_path = other.file_path;

            other.block_pos = 0;
            other.write_pos = 0;
            other.init_size = 0;
            other.mmap_blocks = 0;
            other.log_file = nullptr;
            other.log_fd = -1;
            other.mmap_file = nullptr;
            other.file_path = "";
        }
    }

public:
    /**
      \rst
      Constructs a :class:`fmt::basic_shm_buffer` object moving the content
      of the other object to it.
      \endrst
     */
    FMT_CONSTEXPR20 basic_shm_buffer(basic_shm_buffer &&other) FMT_NOEXCEPT
    {
        move(other);
    }

    /**
      \rst
      Moves the content of the other ``basic_shm_buffer`` object to this one.
      \endrst
     */
    auto operator=(basic_shm_buffer &&other) FMT_NOEXCEPT->basic_shm_buffer &
    {
        FMT_ASSERT(this != &other, "");
        deallocate();
        move(other);
        return *this;
    }

    /**
      Resizes the buffer to contain *count* elements. If T is a POD type new
      elements may not be initialized.
     */
    FMT_CONSTEXPR20 void resize(size_t count)
    {
        this->try_resize(count);
    }

    /** Increases the buffer capacity to *new_capacity*. */
    void reserve(size_t new_capacity)
    {
        this->try_reserve(new_capacity);
    }

    // Directly append data into the buffer
    using detail::buffer<T>::append;
    template<typename ContiguousRange>
    void append(const ContiguousRange &range)
    {
        append(range.data(), range.data() + range.size());
    }
};

template<typename T, size_t SIZE, typename Allocator>
FMT_CONSTEXPR20 void basic_shm_buffer<T, SIZE, Allocator>::grow(size_t size)
{
#ifdef FMT_FUZZ
    if (size > 5000)
        throw std::runtime_error("fuzz mode - won't grow that much");
#endif
    const size_t max_size = init_shm_size;
    size_t old_capacity = this->capacity();
    size_t new_capacity = init_shm_size;

    remap();
    T *new_data = mmap_file;
    this->set(new_data, new_capacity);
}


using shm_buffer = basic_shm_buffer<char>;
 
FMT_END_NAMESPACE  
