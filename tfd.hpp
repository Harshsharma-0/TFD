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
#include <cstring>
#include <stdint.h>

#define TFD_MIN_BUCKET_SIZE (1 << 2) /// minimum size of buckets
#define TFD_MAX_BUCKET_SIZE 1024     /// maximim size of buckets
#define TFD_MIN_BUCKET 2             /// minimun number of buckets
#define TFD_MAX_BUCKET 100           /// maximum number of buckets
#define TFD_INVALID_RAW_FD ~0UL      /// macro to test against invalid values

#define TFD_FILL_ARRAY(name, value, size)                                      \
  for (int i = 0; i < size; i++)                                               \
    name[i] = value;

#if defined(__linux__) || defined(__gnu_linux__)

#define INVALID_SOCKET_VALUE TFD_INVALID_RAW_FD
#define INVALID_HANDLE_VALUE TFD_INVALID_RAW_FD
#define TFD_PLATFORM_LINUX
/**
 * @brief typealias for SOCKET and HANDLE for linux
 *
 * typealias for linux/bsd just to get rid of compilation errors.
 *
 */
using SOCKET = int;
using HANDLE = unsigned int;

enum TFD_FD_TYPE {
  TFD_TYPE_INVALID = ~0U,
  TFD_TYPE_SOCKET = 1,
  TFD_TYPE_HANDLE = 2,
};

#elif defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#include <winsock2.h>

#define INVALID_SOCKET_VALUE INVALID_SOCKET
#define TFD_PLATFORM_WINDOWS

#else
#error "TFD unsupported platform"
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
  int init(int nbucket, int nfds) { return init_base(nbucket, nfds); };

  /**
   * @brief Translates the SOCKET on windows to uinx style fd.
   *
   *  This function takes in windows *SOCKET* and return a unix style interger
   * based fd on windows
   *
   *  @param[in] fd socket returned by socket api on windows.
   *
   *  @returns A valid unix fd number or -1 on error.
   *  [NOTE] works only on windows, until it's enabled on linux/BSD.
   *
   */
  int open(SOCKET fd) { return open_base(uintptr_t(fd), TFD_TYPE_SOCKET); };

  /**
   * @brief Translates the HANDLE on windows to uinx style fd.
   *
   *  This function takes in windows *HANDLE* and return a unix style interger
   * based fd on windows
   *
   *  @param[in] fd handle returned by any windows api.
   *
   *  @returns A valid unix fd number or -1 on error.
   *  @note works only on windows, until it's enabled on linux/BSD.
   *
   */
  int open(HANDLE fd) { return open_base(uintptr_t(fd), TFD_TYPE_HANDLE); };

  /**
   * @brief Returns the raw SOCKET from the unix style fd number obtained form
   * tfd.
   *
   * This function returns the SOCKET, that was previously assigned to the fd
   * integer number.
   *
   * @note It's the user responsibility to keep track if the fd is socket or
   * not, as this function, just retrives the data and return to the caller,
   * without explicit type checking.
   *
   * @param[in] fd fd obtained from tfd.
   *
   * @returns The raw SOCKET or INVALID_SOCKET_VALUE if the slot is empty or the
   * fd is out of bound.
   *
   */

  SOCKET rawsocket(int fd) {
    uintptr_t _fd = raw_base(fd);
#ifdef TFD_PLATFORM_LINUX
    return static_cast<SOCKET>(_fd);
#else
    return _fd == TFD_INVALID_RAW_FD ? static_cast<SOCKET>(INVALID_SOCKET_VALUE)
                                     : static_cast<SOCKET>(_fd);
#endif
  };

  /**
   * @brief returns the raw HANDLE from the unix style fd number obtained form
   * tfd.
   *
   * This function returns the HANDLE, that was previously assigned to the fd
   * integer number.
   *
   * @note It's the user responsibility to keep track if the fd is socket or
   * not, as this function, just retrives the data and return to the caller,
   * without explicit type checking.
   *
   * @param[in] fd fd obtained from tfd.
   *
   * @returns The raw HANDLE or INVALID_HANDLE_VALUE if the slot is empty or the
   * fd is out of bound.
   *
   */
  HANDLE rawhandle(int fd) {
    uintptr_t _fd = raw_base(fd);
#ifdef TFD_PLATFORM_LINUX
    return static_cast<HANDLE>(_fd);
#else
    return _fd == TFD_INVALID_RAW_FD ? static_cast<HANDLE>(INVALID_HANDLE_VALUE)
                                     : static_cast<HANDLE>(_fd);
#endif
  };

  int close(int fd) { return close_base(fd); };
  void enable_reusable() { reuse = true; };
  int exit() { return exit_base(); };

private:
  struct tfd_chunk {
    uintptr_t
        fd; /// Field to store the raw data of the SOCKET or HANDLE on windows
    unsigned int
        next; /// Field to store the next index, if reusability is enabled
    unsigned int type; /// Field to store the type of fd so it can be known
                       /// while returning the raw_fd or closing the fd
  };

  int exit_base() {
#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)
    if (fds == nullptr)
      return -1;

    for (int i = 0; i < buckets && fds[i] != nullptr; i++) {
      for (int j = 0; j < (1U << cycle_bit); j++) {
        /* do propper close here */

#ifdef TFD_PLATFORM_LINUX
        /*TODO: add linux specific close calls */

#else
        /*TODO: add windows specific close calls */
#endif
      }
      delete[] fds[i];
    };

    delete[] fds;
    fds = nullptr;
    rfds = TFD_TYPE_INVALID;
    cycle_bit = 0;
    next_idx = 0;
    buckets = 0;
    return 0;
#else
    return 0;
#endif
  }
  int init_base(int nbucket, int nfds) {
    // it can be removed by NDEBUG so a if statement to check
    assert(fds == nullptr);
    if (fds != nullptr) /* calling init on already initlised instance */
      return -1;

#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)
    /* checking if nfds is power of 2 or not */
    if ((nfds & (nfds - 1))) {
      std::cout << "nBucket is not power of 2 , or nfds not within limit. \n";
      return -1;
    };

    /* checking nfds constraints */
    if (!(TFD_MIN_BUCKET_SIZE <= nfds && nfds <= TFD_MAX_BUCKET_SIZE)) {
      std::cout << "parameter nfds is power of 2 but not in range of tfd "
                   "operable limits. \n";
      return -1;
    };

    /* checking the nbucket constraints */
    if (!(TFD_MIN_BUCKET <= nbucket && nbucket <= TFD_MAX_BUCKET)) {
      std::cout
          << "nBucket not within limit TFD_MIN_BUCKET && TFD_MAX_BUCKET. \n";
      return -1;
    };

    /* allocating the bucket */
    tfd_chunk **arrfds = new (std::nothrow) tfd_chunk *[nbucket];

    if (arrfds == nullptr) {
      std::cout << "[tfd error] failed to allocate buckets \n";
      return -1;
    };

    //    memset(static_cast<void *>(arrfds),0,sizeof(struct tfd_chunk*) *
    //    nbucket);
    TFD_FILL_ARRAY(arrfds, nullptr, nbucket)
    buckets = nbucket;

    /* allocating the entries for buckets to hold */
    tfd_chunk *arr = new (std::nothrow) tfd_chunk[nfds];
    if (arr == nullptr) {

      std::cout << "[tfd error] failed to allocate buckets entries block. \n";
      delete[] arrfds;

      return -1;
    };

    tfd_chunk chk = {TFD_INVALID_RAW_FD, TFD_TYPE_INVALID, TFD_TYPE_INVALID};

    TFD_FILL_ARRAY(arr, chk, nfds)

#ifdef TFD_PLATFORM_WINDOWS
    /* TODO : add windows specific code to cound the bits */

#elif defined(TFD_PLATFORM_LINUX)
    cycle_bit =
        ((sizeof(int) * 8) - 1) -
        __builtin_clz(nfds); // finding the position of the 1 in the bit's
#else
#error unsupported platform error by cycle_bit extraction
#endif
    arrfds[0] = arr;
    fds = arrfds;

#endif

    return 0;
  };

  uintptr_t raw_base(int fd) const {

    assert(fds !=
           nullptr); // it can be removed by NDEBUG so a if statement to check

    if (fd < 0 || fds == nullptr)
      return TFD_INVALID_RAW_FD;

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

    assert(fds !=
           nullptr); // it can be removed by NDEBUG so a if statement to check
    if (fd < 0 || fds == nullptr)
      return -1;

#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)

    int base = fd >> cycle_bit;
    int idx = ~(base << cycle_bit) & fd;
    /* TODO: properly handle the close operation */

    /* bound checking for invalid fd entries, here we don't care about index */
    if (base > (buckets - 1))
      return -1;

    if (fds[base] == nullptr)
      return -1;

    tfd_chunk &slot = fds[base][idx];

    if (slot.type == TFD_TYPE_INVALID)
      return -1;

    slot.fd = TFD_INVALID_RAW_FD;
    slot.type = TFD_TYPE_INVALID;

    if (reuse) {
      slot.next = rfds;
      rfds = static_cast<unsigned int>(fd);
    } else {
      slot.next = TFD_TYPE_INVALID;
    };

    return 0; // TODO: add close func and proper method for error detection
#else
    return 0;
#endif
  };

  /* TODO: add bucket allocation tracking */

  int open_base(uintptr_t fd, int type) {

#if defined(TFD_ENABLE_ON_LINUX) || defined(TFD_PLATFORM_WINDOWS)
    // TODO: add relative casts for windows specific SOCKET and HANDLE;
    assert(fds !=
           nullptr); // it can be removed by NDEBUG so a if statement to check
    if (fds == nullptr)
      return -1;

    int idx_bucket = next_idx >> cycle_bit;
    int idx = ~(idx_bucket << cycle_bit) & next_idx;

    if (reuse && rfds != TFD_TYPE_INVALID) {
      int idx_base_nxt = rfds >> cycle_bit;
      int idx_base = ~(idx_base_nxt << cycle_bit) & rfds;
      int tmp = rfds;

      // contains the info about the next empty slot to be returned;
      rfds = fds[idx_base_nxt][idx_base].next;

      fds[idx_base_nxt][idx_base].fd = fd;
      fds[idx_base_nxt][idx_base].next = TFD_TYPE_INVALID;
      fds[idx_base_nxt][idx_base].type = type;

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

      tfd_chunk chk = {TFD_INVALID_RAW_FD, TFD_TYPE_INVALID, TFD_TYPE_INVALID};

      TFD_FILL_ARRAY(arr, chk, (1U << cycle_bit))

      fds[idx_bucket] = arr;
    };

    tfd_chunk &slot = fds[idx_bucket][idx];
    slot.fd = fd;
    slot.type = type;
    slot.next = TFD_TYPE_INVALID;

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
   *   rfd(freeindex) -> freeindex[nextfreeindex] -> nextfreeindex -> ... ->
   * until we encounter TFD_TYPE_INVALID;
   *
   *   explaination:
   *    when chaining, the rfds contain the index to the free available slot,
   * and that free available slot contains the index of the next free index, and
   * the last one contians the special index value TFD_TYPE_INVALID. marking
   * termination of the rfds, linked array.
   *
   *****************************************/

  unsigned int rfds =
      TFD_TYPE_INVALID; /* chained list of available reusable fd's slots*/
  int buckets = 0;      /* contains the total number of
                          bucket configured must be power of 2 */
  bool reuse = false;   /* flag to enable reusable fd */

  unsigned int cycle_bit = 0; /* position of the starting of cycle bit */
  unsigned int next_idx = 0;  /* the next index to return */
};

#endif
