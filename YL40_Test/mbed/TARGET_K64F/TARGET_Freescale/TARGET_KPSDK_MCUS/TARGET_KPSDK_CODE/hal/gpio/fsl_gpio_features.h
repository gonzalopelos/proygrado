/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#if !defined(__FSL_GPIO_FEATURES_H__)
#define __FSL_GPIO_FEATURES_H__

#if defined(CPU_MK20DX128VMP5) || defined(CPU_MK20DN128VMP5) || defined(CPU_MK20DX64VMP5) || defined(CPU_MK20DN64VMP5) || \
    defined(CPU_MK20DX32VMP5) || defined(CPU_MK20DN32VMP5) || defined(CPU_MK20DX128VLH5) || defined(CPU_MK20DN128VLH5) || \
    defined(CPU_MK20DX64VLH5) || defined(CPU_MK20DN64VLH5) || defined(CPU_MK20DX32VLH5) || defined(CPU_MK20DN32VLH5) || \
    defined(CPU_MK20DX128VFM5) || defined(CPU_MK20DN128VFM5) || defined(CPU_MK20DX64VFM5) || defined(CPU_MK20DN64VFM5) || \
    defined(CPU_MK20DX32VFM5) || defined(CPU_MK20DN32VFM5) || defined(CPU_MK20DX128VFT5) || defined(CPU_MK20DN128VFT5) || \
    defined(CPU_MK20DX64VFT5) || defined(CPU_MK20DN64VFT5) || defined(CPU_MK20DX32VFT5) || defined(CPU_MK20DN32VFT5) || \
    defined(CPU_MK20DX128VLF5) || defined(CPU_MK20DN128VLF5) || defined(CPU_MK20DX64VLF5) || defined(CPU_MK20DN64VLF5) || \
    defined(CPU_MK20DX32VLF5) || defined(CPU_MK20DN32VLF5) || defined(CPU_MK22FN256VDC12) || defined(CPU_MK22FN256VLH12) || \
    defined(CPU_MK22FN256VLL12) || defined(CPU_MK22FN256VMP12) || defined(CPU_MK22FN512VDC12) || defined(CPU_MK22FN512VLH12) || \
    defined(CPU_MK22FN512VLL12) || defined(CPU_MK24FN1M0VDC12) || defined(CPU_MK24FN1M0VLQ12) || defined(CPU_MK63FN1M0VLQ12) || \
    defined(CPU_MK63FN1M0VMD12) || defined(CPU_MK64FN1M0VDC12) || defined(CPU_MK64FN1M0VLL12) || defined(CPU_MK64FN1M0VLQ12) || \
    defined(CPU_MK64FX512VLQ12) || defined(CPU_MK64FN1M0VMD12) || defined(CPU_MK64FX512VMD12) || defined(CPU_MK65FN2M0CAC18) || \
    defined(CPU_MK65FX1M0CAC18) || defined(CPU_MK65FN2M0VMI18) || defined(CPU_MK65FX1M0VMI18) || defined(CPU_MK66FN2M0VLQ18) || \
    defined(CPU_MK66FX1M0VLQ18) || defined(CPU_MK66FN2M0VMD18) || defined(CPU_MK66FX1M0VMD18) || defined(CPU_MK70FN1M0VMF12) || \
    defined(CPU_MK70FX512VMF12) || defined(CPU_MK70FN1M0VMF15) || defined(CPU_MK70FX512VMF15) || defined(CPU_MK70FN1M0VMJ12) || \
    defined(CPU_MK70FX512VMJ12) || defined(CPU_MK70FN1M0VMJ15) || defined(CPU_MK70FX512VMJ15) || defined(CPU_MKV31F256VLH12) || \
    defined(CPU_MKV31F256VLL12) || defined(CPU_MKV31FN512VLH12) || defined(CPU_MKV31F512VLL12)
    /* @brief Has fast (single cycle) access capability via a dedicated memory region.*/
    #define FSL_FEATURE_GPIO_HAS_FAST_GPIO (0)
    /* @brief Has port input disable register (PIDR).*/
    #define FSL_FEATURE_GPIO_HAS_INPUT_DISABLE (0)
    /* @brief Has dedicated interrupt vector.*/
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTOR (1)
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTORn(x) \
        ((x) == 0 ? (1) : \
        ((x) == 1 ? (1) : \
        ((x) == 2 ? (1) : \
        ((x) == 3 ? (1) : \
        ((x) == 4 ? (1) : \
        ((x) == 5 ? (1) : (-1)))))))
#elif defined(CPU_MKL05Z8VFK4) || defined(CPU_MKL05Z16VFK4) || defined(CPU_MKL05Z32VFK4) || defined(CPU_MKL05Z8VLC4) || \
    defined(CPU_MKL05Z16VLC4) || defined(CPU_MKL05Z32VLC4) || defined(CPU_MKL05Z8VFM4) || defined(CPU_MKL05Z16VFM4) || \
    defined(CPU_MKL05Z32VFM4) || defined(CPU_MKL05Z16VLF4) || defined(CPU_MKL05Z32VLF4)
    /* @brief Has fast (single cycle) access capability via a dedicated memory region.*/
    #define FSL_FEATURE_GPIO_HAS_FAST_GPIO (1)
    /* @brief Has port input disable register (PIDR).*/
    #define FSL_FEATURE_GPIO_HAS_INPUT_DISABLE (0)
    /* @brief Has dedicated interrupt vector.*/
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTOR (1)
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTORn(x) \
        ((x) == 0 ? (1) : \
        ((x) == 1 ? (1) : (-1)))
#elif defined(CPU_MKL25Z32VFM4) || defined(CPU_MKL25Z64VFM4) || defined(CPU_MKL25Z128VFM4) || defined(CPU_MKL25Z32VFT4) || \
    defined(CPU_MKL25Z64VFT4) || defined(CPU_MKL25Z128VFT4) || defined(CPU_MKL25Z32VLH4) || defined(CPU_MKL25Z64VLH4) || \
    defined(CPU_MKL25Z128VLH4) || defined(CPU_MKL25Z32VLK4) || defined(CPU_MKL25Z64VLK4) || defined(CPU_MKL25Z128VLK4)
    /* @brief Has fast (single cycle) access capability via a dedicated memory region.*/
    #define FSL_FEATURE_GPIO_HAS_FAST_GPIO (1)
    /* @brief Has port input disable register (PIDR).*/
    #define FSL_FEATURE_GPIO_HAS_INPUT_DISABLE (0)
    /* @brief Has dedicated interrupt vector.*/
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTOR (1)
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTORn(x) \
        ((x) == 0 ? (1) : \
        ((x) == 1 ? (0) : \
        ((x) == 2 ? (0) : \
        ((x) == 3 ? (1) : \
        ((x) == 4 ? (0) : (-1))))))
#elif defined(CPU_MKL46Z128VLH4) || defined(CPU_MKL46Z256VLH4) || defined(CPU_MKL46Z128VLL4) || defined(CPU_MKL46Z256VLL4) || \
    defined(CPU_MKL46Z128VMC4) || defined(CPU_MKL46Z256VMC4)
    /* @brief Has fast (single cycle) access capability via a dedicated memory region.*/
    #define FSL_FEATURE_GPIO_HAS_FAST_GPIO (1)
    /* @brief Has port input disable register (PIDR).*/
    #define FSL_FEATURE_GPIO_HAS_INPUT_DISABLE (0)
    /* @brief Has dedicated interrupt vector.*/
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTOR (1)
    #define FSL_FEATURE_GPIO_HAS_INTERRUPT_VECTORn(x) \
        ((x) == 0 ? (1) : \
        ((x) == 1 ? (0) : \
        ((x) == 2 ? (1) : \
        ((x) == 3 ? (1) : \
        ((x) == 4 ? (0) : (-1))))))
#else
    #error "No valid CPU defined!"
#endif

#endif /* __FSL_GPIO_FEATURES_H__*/
/*******************************************************************************
 * EOF
 ******************************************************************************/

