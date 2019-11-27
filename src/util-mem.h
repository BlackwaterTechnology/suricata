/* Copyright (C) 2007-2019 Open Information Security Foundation
 *
 * You can copy, redistribute or modify this Program under the terms of
 * the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/**
 * \file
 *
 * \author Pablo Rincon Crespo <pablo.rincon.crespo@gmail.com>
 * \author Bill Meeks <billmeeks8@gmail.com>
 *
 * Utility Macros for memory management
 *
 * \todo Add wrappers for functions that allocate/free memory here.
 * Currently we have malloc, calloc, realloc, strdup, strndup and
 * free, but there are more.
 */

#ifndef __UTIL_MEM_H__
#define __UTIL_MEM_H__

#include "util-atomic.h"

#if CPPCHECK==1 || defined(__clang_analyzer__)
#define SCMalloc malloc
#define SCCalloc calloc
#define SCRealloc realloc
#define SCFree free
#define SCStrdup strdup
#define SCStrndup strndup
#define SCMallocAligned _mm_malloc
#define SCFreeAligned _mm_free
#else /* CPPCHECK */


#if defined(_WIN32) || defined(__WIN32)
#include "mm_malloc.h"
#endif

SC_ATOMIC_EXTERN(unsigned int, engine_stage);

#define SCMalloc(a) ({ \
    void *ptrmem = NULL; \
    \
    ptrmem = malloc((a)); \
    if (ptrmem == NULL) { \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            uintmax_t scmalloc_size_ = (uintmax_t)(a); \
            SCLogError(SC_ERR_MEM_ALLOC, "SCMalloc failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes", strerror(errno), scmalloc_size_); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } \
    (void*)ptrmem; \
})

#define SCRealloc(x, a) ({ \
    void *ptrmem = NULL; \
    \
    ptrmem = realloc((x), (a)); \
    if (ptrmem == NULL) { \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            SCLogError(SC_ERR_MEM_ALLOC, "SCRealloc failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes", strerror(errno), (uintmax_t)(a)); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } \
    (void*)ptrmem; \
})

#define SCCalloc(nm, a) ({ \
    void *ptrmem = NULL; \
    \
    ptrmem = calloc((nm), (a)); \
    if (ptrmem == NULL) { \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            SCLogError(SC_ERR_MEM_ALLOC, "SCCalloc failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes", strerror(errno), (uintmax_t)(a)); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } \
    (void*)ptrmem; \
})

#define SCStrdup(a) ({ \
    char *ptrmem = NULL; \
    \
    ptrmem = strdup((a)); \
    if (ptrmem == NULL) { \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            size_t _scstrdup_len = strlen((a)); \
            SCLogError(SC_ERR_MEM_ALLOC, "SCStrdup failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes", strerror(errno), (uintmax_t)_scstrdup_len); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } \
    (void*)ptrmem; \
})

#ifndef HAVE_STRNDUP
#define SCStrndup(a, b) ({ \
    char *ptrmem = NULL; \
    size_t _len = (b); \
    \
    size_t _scstrndup_len = _len + 1; \
    ptrmem = (char *)malloc(_scstrndup_len); \
    if (ptrmem == NULL) { \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            SCLogError(SC_ERR_MEM_ALLOC, "SCStrndup failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes", strerror(errno), (uintmax_t)_scstrndup_len); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } else { \
        strlcpy(ptrmem, (a), _scstrndup_len); \
        *(ptrmem + _len) = '\0'; \
    } \
    (void*)ptrmem; \
})
#else /* HAVE_STRNDUP */
#define SCStrndup(a, b) ({ \
    char *ptrmem = NULL; \
    \
    ptrmem = strndup((a), (b)); \
    if (ptrmem == NULL) { \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            size_t _scstrndup_len = (b); \
            SCLogError(SC_ERR_MEM_ALLOC, "SCStrndup failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes", strerror(errno), (uintmax_t)_scstrndup_len); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } \
    (void*)ptrmem; \
})
#endif

#define SCFree(a) ({ \
    free(a); \
})

#if defined(__WIN32) || defined(_WIN32)

/** \brief wrapper for allocing aligned mem
 *  \param a size
 *  \param b alignement
 */
#define SCMallocAligned(a, b) ({ \
    void *ptrmem = NULL; \
    \
	ptrmem = _mm_malloc((a), (b)); \
    if (ptrmem == NULL) { \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            SCLogError(SC_ERR_MEM_ALLOC, "SCMallocAligned(posix_memalign) failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes, alignment %"PRIuMAX, strerror(errno), (uintmax_t)(a), (uintmax_t)(b)); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } \
    (void*)ptrmem; \
})

/** \brief Free aligned memory
 *
 * Not needed for mem alloc'd by posix_memalign,
 * but for possible future use of _mm_malloc needing
 * _mm_free.
 */
#define SCFreeAligned(a) ({ \
    _mm_free(a); \
})

#else /* !win */

/** \brief wrapper for allocing aligned mem
 *  \param a size
 *  \param b alignement
 */
#define SCMallocAligned(a, b) ({ \
    void *ptrmem = NULL; \
    \
    int _r = posix_memalign(&ptrmem, (b), (a)); \
    if (_r != 0 || ptrmem == NULL) { \
        if (ptrmem != NULL) { \
            free(ptrmem); \
            ptrmem = NULL; \
        } \
        if (SC_ATOMIC_GET(engine_stage) == SURICATA_INIT) {\
            SCLogError(SC_ERR_MEM_ALLOC, "SCMallocAligned(posix_memalign) failed: %s, while trying " \
                "to allocate %"PRIuMAX" bytes, alignment %"PRIuMAX, strerror(errno), (uintmax_t)a, (uintmax_t)b); \
            SCLogError(SC_ERR_FATAL, "Out of memory. The engine cannot be initialized. Exiting..."); \
            exit(EXIT_FAILURE); \
        } \
    } \
    (void*)ptrmem; \
})

/** \brief Free aligned memory
 *
 * Not needed for mem alloc'd by posix_memalign,
 * but for possible future use of _mm_malloc needing
 * _mm_free.
 */
#define SCFreeAligned(a) ({ \
    free(a); \
})

#endif /* __WIN32 */

#endif /* CPPCHECK */

#endif /* __UTIL_MEM_H__ */

