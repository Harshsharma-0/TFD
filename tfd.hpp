/* 
 *   Copyright 2026 Harsh Sharma
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


/**
 * @file tfd.hpp
 * @author Harsh Sharma
 * @language c++
 * @version 1.0.0
 *
 */

#ifndef TDF_HPP
#define TDF_HPP

#include <cassert>
#include <stdint.h>
#include <cstring>

#define TFD_MIN_BUCKET_SIZE (1 << 2)
#define TFD_MAX_BUCKET_SIZE 1024
#define TFD_MIN_BUCKET 2
#define TFD_MAX_BUCKET 100
#define TFD_INVALID_RAW_FD ~0U


#define TFD_FILL_ARRAY(name , value, size) \
   for(int i = 0 ; i < size ; i++) \
        name[i] = value;


#if defined(__linux__) || defined(__gnu_linux__)

#define INVALID_SOCKET_VALUE TFD_INVALID_RAW_FD
#define INVALID_HANDLE_VALUE TFD_INVALID_RAW_FD

/**
 * @brief typealias for SOCKET and HANDLE for linux
 *
 * typealias for linux/bsd just to get rid of compilation errors.
 *
 */
using SOCKET = int;
using HANDLE = unsigned int;

#else

#include <windows.h>
#define INVALID_SOCKET_VALUE INVALID_SOCKET

#endif

/**
 * @class tfd
 * @brief TFD **(Translate To FD)**
 *
 * This class is used to translate HANDLE and SOCKET on windows to
 * traditonal unix file destriptor numbering, so program that are written
 * for cross platform don't have tp suffer the pain of interpolation.
 *
 * @note All the function in this class only works on windows, having no effect
 *       on linux and *BSD (Means the no fd translation is done). \n
 *       **except these function** \n
 *       1) enable_reusable;
 */

class tfd {

public:
  /**
   * @{
   */

  /** @brief defination to protect class from being movable and copyable
   *
   */

  tfd() = default;
  tfd &operator=(tfd const &) = delete;
  tfd &operator=(tfd const &&) = delete;
  tfd(tfd const &) = delete;
  tfd(tfd const &&) = delete;

  /**
   * @}
   */

  /**
   * @brief Initlises the tfd
   *
   * This function initlise tfd and allocate an internal
   * 2-D array with N row = nbucket (power of 2),
   * and nfds represent the numbers of fd's per bucket.
   *
   * @param[in] nbucket number of bucket bucket and is limited by TFD_MIN_BUCKET
   * and TFD_MAX_BUCKET.
   * @param[in] nfds    number of fd per bucket requested must be of power of 2,
   * limited by TFD_MIN_BUCKET_SIZE and TFD_MAX_BUCKET_SIZE.
   *
   * @return 0 on success -1 on error.
   *
   */
  int init(int nbucket, int nfds) { return init_base(nbucket,nfds);};

  /**
   * @brief Translates the SOCKET on windows to uinx style fd.
   *  
   *  This function takes in windows *SOCKET* and return a unix style interger based fd on windows
   *
   *  @param[in] fd socket returned by socket api on windows.
   *
   *  @returns A valid unix fd number or -1 on error.
   *  [NOTE] works only on windows, until it's enabled on linux/BSD.
   *
   */
  int open(SOCKET fd) { return open_base(uintptr_t(fd)); };

  /**
   * @brief Translates the HANDLE on windows to uinx style fd.
   *  
   *  This function takes in windows *HANDLE* and return a unix style interger based fd on windows
   *
   *  @param[in] fd handle returned by any windows api.
   *
   *  @returns A valid unix fd number or -1 on error.
   *  @note works only on windows, until it's enabled on linux/BSD.
   *
   */
  int open(HANDLE fd) { return open_base(uintptr_t(fd)); };


  /**
   * @brief Returns the raw SOCKET from the unix style fd number obtained form tfd.
   *
   * This function returns the SOCKET, that was previously assigned to the fd integer number.
   *
   * @note It's the user responsibility to keep track if the fd is socket or not, as this function,
   *  just retrives the data and return to the caller, without explicit type checking.
   * 
   * @param[in] fd fd obtained from tfd.
   *
   * @returns The raw SOCKET or INVALID_SOCKET_VALUE if the slot is empty or the fd is out of bound.
   *
   */

  SOCKET rawsocket(int fd) {
    uintptr_t _fd = raw_base(fd);
    return _fd == TFD_INVALID_RAW_FD ? INVALID_SOCKET_VALUE : fd;
  };

/**
   * @brief returns the raw HANDLE from the unix style fd number obtained form tfd.
   *
   * This function returns the HANDLE, that was previously assigned to the fd integer number.
   *
   * @note It's the user responsibility to keep track if the fd is socket or not, as this function,
   *  just retrives the data and return to the caller, without explicit type checking.
   * 
   * @param[in] fd fd obtained from tfd.
   *
   * @returns The raw HANDLE or INVALID_HANDLE_VALUE if the slot is empty or the fd is out of bound.
   *
   */
  HANDLE rawhandle(int fd) { 
    uintptr_t _fd = raw_base(fd);
    return _fd == TFD_INVALID_RAW_FD ? INVALID_HANDLE_VALUE : fd;
  };


  int close(int fd) { return close_base(fd); };
  void enable_reusable() { reuse = true; };

private:

  struct tfd_chunk{
   uintptr_t fd; /// Field to store the raw data of the SOCKET or HANDLE on windows
   unsigned int next; /// Field to store the next index, if reusability is enabled
   int type; /// Field to store the type of fd so it can be known while returning the raw_fd or closing the fd
  };

  int init_base(int nbucket, int nfds) {
#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)
    /* checking if nfds is power of 2 or not */
    if ((nfds & (nfds - 1))) {
      std::cout << "nBucket is not power of 2 , or nfds not within limit. \n";
      return -1;
    };

    /* checking nfds constraints */
    if (!(TFD_MIN_BUCKET_SIZE < nfds && nfds < TFD_MAX_BUCKET_SIZE)) {
      std::cout << "parameter nfds is power of 2 but not in range of tfd "
                   "operable limits. \n";
      return -1;
    };

    /* checking the nbucket constraints */
    if (!(TFD_MIN_BUCKET < nbucket && nbucket < TFD_MAX_BUCKET)) {
      std::cout
          << "nBucket not within limit TFD_MIN_BUCKET && TFD_MAX_BUCKET. \n";
      return -1;
    };

    /* allocating the bucket */
    tfd_chunk **arrfds = new (std::nothrow) tfd_chunk* [nbucket];

    if (arrfds == nullptr) {
      std::cout << "[tfd error] failed to allocate buckets \n";
      return -1;
    };
  
//    memset(static_cast<void *>(arrfds),0,sizeof(struct tfd_chunk*) * nbucket);
    TFD_FILL_ARRAY(arrfds,nullptr,nbucket)
    buckets = nbucket;

    /* allocating the entries for buckets to hold */
    tfd_chunk *arr = new (std::nothrow) tfd_chunk[nfds];
    if (arr == nullptr) {

      std::cout << "[tfd error] failed to allocate buckets entries block. \n";
       delete[] arrfds;

      return -1;
    };
    
    tfd_chunk chk = {TFD_INVALID_RAW_FD, TFD_INVALID_RAW_FD};

    TFD_FILL_ARRAY(arr,chk,nfds)

    cycle_bit =
        ((sizeof(int) * 8) - 1) -
        __builtin_clz(nfds); // finding the position of the 1 in the bit's
                             //
    arrfds[0] = arr;
    fds = arrfds;

 #endif

    return 0;
  };

  uintptr_t raw_base(int fd) const {

#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)

    int base = fd >> cycle_bit;
    int idx = ~(base << cycle_bit) & fd;
    /* TODO: properly handle the close operation */

    /* bound checking for invalid fd entries,
     * here we don't care about index.
     *
     * we also check the base exist or not.
     */

    if (base > (buckets - 1) || fds[base] == nullptr)
      return TFD_INVALID_RAW_FD;

    uintptr_t raw = fds[base][idx].fd;
    return raw;

#else
    return fd;
#endif

    return fd;
  };

  int close_base(int fd) {

#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)

    int base = fd >> cycle_bit;
    int idx = ~(base << cycle_bit) & fd;
    /* TODO: properly handle the close operation */

    /* bound checking for invalid fd entries, here we don't care about index */
    if (base > (buckets - 1))
      return -1;

    if (reuse) {

      if (rfds == TFD_INVALID_RAW_FD) {
        // case when the rfds is empty
        // putting special terminator signature int the reusable chain.
        fds[base][idx].next = TFD_INVALID_RAW_FD;
        fds[base][idx].fd = TFD_INVALID_RAW_FD;

      } else {
        // case when rfds is not empty and needed to be appended;
        // first: save info of the rfds in the fresh fd idx;
        fds[base][idx].next = rfds;
        fds[base][idx].fd = TFD_INVALID_RAW_FD;
      };
  
      // then : passon the fd idx to thr rfds
      rfds = fd;
    };

    return 0; // TODO: add close func and proper method for error detection
#else
    return 0;
#endif
  };

  int open_base(uintptr_t fd) {

#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)
    // TODO: add relative casts for windows specific SOCKET and HANDLE;
    assert(fds != nullptr);

    int idx_bucket = next_idx >> cycle_bit;
    int idx = ~(idx_bucket << cycle_bit) & next_idx;

    if (reuse && rfds != TFD_INVALID_RAW_FD) {

      int idx_base_nxt = rfds >> cycle_bit;
      int idx_base = ~(idx_base_nxt << cycle_bit) & rfds;
      int tmp = rfds;

      // contains the info about the next empty slot to be returned;
      rfds = fds[idx_base_nxt][idx_base].next;
      return tmp;
    };

    // only true if we maxed out in capacity, also check for reusability
    if (idx_bucket > (buckets - 1)) {
      if (!reuse)
        return -2;

      return -2;
    };

    // allocating new bucket if previous one exausted
    if (fds[idx_bucket] == nullptr) {
      if (buckets == 0)
        return -2;

      tfd_chunk *arr = new (std::nothrow) tfd_chunk[(1U << cycle_bit)];
      if (arr == nullptr)
        return -1;
      
      tfd_chunk chk = {TFD_INVALID_RAW_FD, TFD_INVALID_RAW_FD};

      TFD_FILL_ARRAY(arr,chk,(1U << cycle_bit))

      fds[idx_bucket] = arr;
    };

    fds[idx_bucket][idx].fd = fd;

    return static_cast<int>(next_idx++);
#else
    return fd;
#endif
  };

  /* 2-D array for rfd with rows = nbucket and coloums = rfds */
  tfd_chunk **fds = nullptr;

  /*****************************************
   *
   * rfds is chained like :
   *   rfd(free index) -> freeindex[nextfree index] -> nextfree index[~0U];
   *
   *   explaination:
   *    when chaining, the rfds contain the index to the free available slot,
   * and that free available slot contains the index of the next free index, and
   * the last one contians the special index value ~0U. marking termination of
   * the rfds, linked array.
   *
   *****************************************/

  unsigned int rfds = TFD_INVALID_RAW_FD; /* chained list of available reusable fd's slots*/
  int buckets = 0;         /* contains the total number of
                             bucket configured must be power of 2 */
  bool reuse = false;       /* flag to enable reusable fd */

  unsigned int cycle_bit = 0; /* position of the starting of cycle bit */
  unsigned int next_idx = 0;  /* the next index to return */
};

#endif
