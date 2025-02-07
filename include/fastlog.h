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
#include <memory>
#include <sys/time.h>
#include <unistd.h>
#include <spdlog/fmt/fmt.h>

//#include "sse_itoa.h"

#define DEFAULT_PAGE_SIZE 4096 
#ifndef MMAP_BLOCK_SIZE 
#define MMAP_BLOCK_SIZE 1024*1024*8
#endif 


class FastWriter{

	public: 
		~FastWriter(){ }
		bool open(const std::string & filePath, uint64_t initSize = MMAP_BLOCK_SIZE) 
		{
			file_path = filePath; 
			log_file = fopen(filePath.c_str(), "ab+"); 
			//fseek(log_file,0,SEEK_END);
			//fseek(log_file,0,0);
			log_fd =  fileno(log_file); 
			init_size = initSize; 
			mmap_blocks = MMAP_BLOCK_SIZE;  
			ftruncate(log_fd  , initSize); 
			block_pos = 0; 
			mmap_file = (char *)mmap(NULL, MMAP_BLOCK_SIZE,  PROT_WRITE, MAP_FILE | MAP_SHARED, log_fd , 0);
			return true; 
		}

		bool remap(int64_t pos ) {
			//printf("remap file  blocks %lu\n",  mmap_blocks); 
			munmap(mmap_file, MMAP_BLOCK_SIZE); 
			//printf("truncate file size %d\n", mmap_blocks + MMAP_BLOCK_SIZE); 
			if (mmap_blocks  + MMAP_BLOCK_SIZE > init_size){
				int ret = ftruncate(log_fd  , mmap_blocks  + MMAP_BLOCK_SIZE); 
				if (ret != 0 ){
					perror("truncate file failed"); 
					return false; 
				}
			} 

			//fseek(log_file,0,pos );
			block_pos = 0; 
			mmap_file = (char *)mmap(NULL, MMAP_BLOCK_SIZE,  PROT_WRITE, MAP_FILE | MAP_SHARED, log_fd , mmap_blocks );
			if (mmap_file == MAP_FAILED) {
				return false; 
			}

			mmap_blocks += MMAP_BLOCK_SIZE;  
			return true; 
		}

		template <class ... Args > 
			bool format(const char * format , Args ... args) {
				int32_t freeSize = MMAP_BLOCK_SIZE - block_pos; 
				if (freeSize > 0 ) {
					auto rst = fmt::format_to_n(mmap_file + block_pos , freeSize, format, args ...); 
					int len = rst.size; 
					//int len = snprintf(mmap_file + block_pos, freeSize , format, args ... ); 
					if (len <= freeSize) {
						write_pos += len ; 
						block_pos += len ; 
						return true; 
					} else {
						//printf("not enought buffer %d  :  %ld\n", freeSize, rst.size); 
						memset(mmap_file + block_pos, ' ', freeSize); 
						*(mmap_file + MMAP_BLOCK_SIZE - 1) = '\n'; 
						write_pos += freeSize; 
						block_pos += freeSize; 
					}
				}

				remap(write_pos); 
				auto rst = fmt::format_to_n(mmap_file + block_pos , freeSize, format, args ...); 
				int len = rst.size; 
				//int len = snprintf(mmap_file + block_pos, MMAP_BLOCK_SIZE - block_pos, format, args ... ); 
				if (len  <= MMAP_BLOCK_SIZE) {
					write_pos += len ; 
					block_pos += len ; 
				} else {
					return false; 
				}
				return true; 
			}




		template <class ... Args > 
			bool format_to(const char * format , Args ... args) {
				int32_t freeSize = MMAP_BLOCK_SIZE - block_pos; 

				if (freeSize > 0 ) {
					int len = snprintf(mmap_file + block_pos, freeSize , format, args ... ); 
					if (len <= freeSize) {
						write_pos += len ; 
						block_pos += len ; 
						return true; 
					} else {
						memset(mmap_file + block_pos, ' ', freeSize); 
						*(mmap_file + MMAP_BLOCK_SIZE - 1) = '\n'; 
						write_pos += freeSize; 
						block_pos += freeSize; 
					}
				}
				bool ret = 	remap(write_pos); 
				if (ret ){
					int len = snprintf(mmap_file + block_pos, MMAP_BLOCK_SIZE - block_pos, format, args ... ); 
					if (len  <= MMAP_BLOCK_SIZE - block_pos) {
						write_pos += len ; 
						block_pos += len ; 
						return true; 
					} else {
						return false; 
					}
				}else {
					perror("create mmap failed\n"); 
				}
				return false; 
			}



		char reserve_buffer[MMAP_BLOCK_SIZE] = {0}; 
		int32_t reserve_length  = 0; 

		template <class T> 
			int32_t write_elem(char * pos, uint32_t bufLen , const T & elem){
				auto len = sizeof(elem); 
				if (len  <= bufLen) {
					memcpy(pos, (const char *) &elem, len ); 
					return len; 
				} else {
					memcpy(pos, (const char *)&elem, bufLen); 
					reserve_length = len - bufLen; 
					memcpy(reserve_buffer, (const char *)&elem + bufLen, reserve_length); 
					return reserve_length; 
				}
			}

		inline int32_t write_elem(char * pos, uint32_t bufLen , int32_t  elem){
			char szBuf[32]; 


			//FMT_COMPILE("{}")
			char * pBuf= fmt::format_to(szBuf, "{}",elem); 
			*pBuf = 0; 
			auto len  = pBuf - szBuf; 
			//i32toa_sse2(elem, szBuf); 
			//auto len = strlen(szBuf); 
			//auto len = snprintf(szBuf,sizeof(szBuf), "%d", elem) ; 
			szBuf[len] = 0; 

			if (len  <= bufLen) {
				memcpy(pos, (const char *) szBuf, len ); 
				return len; 
			} else {
				memcpy(pos, (const char *)szBuf, bufLen); 
				reserve_length = len - bufLen; 
				memcpy(reserve_buffer, (const char *)&szBuf+ bufLen, reserve_length); 
				return reserve_length; 
			}
		}


		inline int32_t write_elem(char * pos, uint32_t bufLen , const std::string  & elem){

			auto len = elem.length(); 
			if (len  <= bufLen) {
				memcpy(pos, (const char *) elem.data(), len ); 
				return len; 
			} else {
				memcpy(pos, (const char *)elem.data(), bufLen); 

				reserve_length = len - bufLen; 
				memcpy(reserve_buffer, (const char *)&elem + bufLen, reserve_length); 
				return reserve_length; 
			}
		}

		inline int32_t write_elem(char * pos, uint32_t bufLen , const char * elem){
			auto len = strlen(elem); 
			if (len  <= bufLen) {
				memcpy(pos, (const char *) elem, len ); 
				return len; 
			} else {
				memcpy(pos, (const char *)elem, bufLen); 
				reserve_length = len - bufLen; 
				memcpy(reserve_buffer, (const char *)&elem + bufLen, reserve_length); 
				return reserve_length; 
			}
		}




		bool format_write(){
			return true; 
		}

		template <class T, class ... Args > 
			bool format_write(T first,   Args ... args) {
				reserve_length = 0; 
				int32_t ret = write_elem(mmap_file + block_pos, MMAP_BLOCK_SIZE- block_pos,  first); 
				write_pos += ret ; 
				block_pos += ret ; 

				if ( reserve_length > 0 ) {
					remap(write_pos); 
					memcpy(mmap_file + block_pos , reserve_buffer, reserve_length); 

					write_pos += reserve_length ; 
					block_pos += reserve_length; 
				}
				return format_write(args ...); 
			}


		bool write(const char *buf , uint32_t bufLen){

			if (block_pos + bufLen  <= MMAP_BLOCK_SIZE) {
				memcpy(mmap_file + block_pos , buf, bufLen); 
				write_pos += bufLen; 
				block_pos += bufLen; 
			} else {

				int32_t freeSize = MMAP_BLOCK_SIZE - block_pos; 
				memcpy(mmap_file + block_pos , buf, freeSize); 

				//memset(mmap_file + block_pos, ' ',MMAP_BLOCK_SIZE - block_pos); 
				//*(mmap_file + MMAP_BLOCK_SIZE - 1) = '\n'; 
				write_pos += freeSize; 
				block_pos += freeSize; 

				remap(write_pos); 

				uint32_t leftLen = bufLen - freeSize; 
				memcpy(mmap_file + block_pos , buf + freeSize, leftLen); 
				write_pos += leftLen; 
				block_pos += leftLen; 
			}
			return true; 
		}

		void close(){
			//printf("total write size %d\n", write_pos); 
			munmap(mmap_file, MMAP_BLOCK_SIZE); 
			ftruncate(log_fd  , write_pos); 

			if (log_file != nullptr){
				fclose(log_file); 
				log_file = nullptr; 
			}

		}
	private: 
		uint64_t block_pos = 0; 
		uint64_t write_pos = 0; 
		uint64_t init_size  = 0; 
		uint64_t mmap_blocks = 0 ; 
		FILE * log_file  = nullptr; 
		int32_t log_fd = 0; 
		char * mmap_file = nullptr; 
		std::string file_path; 
}; 

