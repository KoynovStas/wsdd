/*
 * smacros.h
 *
 *
 * version 1.2
 *
 *
 * BSD 3-Clause License
 *
 * Copyright (c) 2015, Koynov Stas - skojnov@yandex.ru
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * more details see https://github.com/KoynovStas/smacros
 */

#ifndef SMACROS_H
#define SMACROS_H

#include <stddef.h>  //for offsetof





#define UNUSED(var)  (void)var



#ifdef  __cplusplus
    #define  EXTERN_PREFIX extern "C"
#else
    #define  EXTERN_PREFIX extern
#endif



#define BIT_FLAG(num_bit)           (1ull << (num_bit))


#define SET_BIT(reg, num_bit)       ( (reg) |=  (1ull << (num_bit)) )
#define CLR_BIT(reg, num_bit)       ( (reg) &= ~(1ull << (num_bit)) )
#define INV_BIT(reg, num_bit)       ( (reg) ^=  (1ull << (num_bit)) )


#define SET_FLAG(reg, flags)        ( (reg) |=  (flags) )
#define CLR_FLAG(reg, flags)        ( (reg) &= ~(flags) )
#define INV_FLAG(reg, flags)        ( (reg) ^=  (flags) )


#define CHECK_FLAG_ALL( reg, flags) ( ((reg) & (flags)) == (flags) ) //Checks if all bits are set to 1
#define CHECK_FLAG_ANY( reg, flags) ( ((reg) & (flags)) != 0       ) //Checks if any bits are set to 1
#define CHECK_FLAG_NONE(reg, flags) ( ((reg) & (flags)) == 0       ) //Checks if none of the bits are set to 1


#define MODIFY_REG(reg, clear_mask, set_mask)  ( (reg) = (((reg) & ~(clear_mask)) | (set_mask)) )



//The count of elements in the array.
#define COUNT_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))



/*
 * Since C/C++ 11 you can use a standard static_assert(exp, msg)
 * For old C/C++ a lot of variants, this is the most simple and intuitive
 * It can be used in *.c and in *.h files.
 * (macros that use function style can be used in *.c files only)
 *
 * Disadvantages: you can not be set msg to display the console when compiling
 *
 * Example:
 *
 * STATIC_ASSERT( sizeof(char) == 1)  //good job
 * STATIC_ASSERT( sizeof(char) != 1)  //You will get a compilation error
*/
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define STATIC_ASSERT(expr) \
    enum {ASSERT_CONCAT(ASSERT_CONCAT(level_, __INCLUDE_LEVEL__), \
          ASSERT_CONCAT(_static_assert_on_line_, __LINE__)) = 1/(int)(!!(expr)) }



#define DEF_TO_STR_(text) #text
#define DEF_TO_STR(arg) DEF_TO_STR_(arg)



// align must be a power of two.
// Alignment takes place upwards.
#define ALIGNMENT(val, align)  ( ((val) + (align)) & ~( (align) - 1) )



#define FREE_AND_NULL(        ptr_var)  ({ free     (ptr_var); ptr_var = NULL; })
#define DELETE_AND_NULL(      ptr_var)  ({ delete   (ptr_var); ptr_var = NULL; })
#define DELETE_ARRAY_AND_NULL(ptr_var)  ({ delete [](ptr_var); ptr_var = NULL; })



#ifdef  DEBUG
        #include <stdio.h>

        #define  DEBUG_MSG(...)       printf(__VA_ARGS__)
        #define  FDEBUG_MSG(fp, ...) fprintf(fp, __VA_ARGS__)
#else
        #define  DEBUG_MSG(...)       ({})
        #define  FDEBUG_MSG(fp, ...)  ({})
#endif



// container_of - cast a member of a structure out to the containing structure
// ptr:    the pointer to the member.
// type:   the type of the container struct this is embedded in.
// member: the name of the member within the struct.
#define container_of(ptr, type, member) \
    ({ (type *)( (const char *)ptr - offsetof(type, member) ); })





#endif //SMACROS_H
