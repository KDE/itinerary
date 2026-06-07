/*
    SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJNIEXTRAS_JNIPP_H
#define KJNIEXTRAS_JNIPP_H

///@cond internal

// determine how many elements are in __VA_ARGS__
#define KJNI_PP_NARG(...) KJNI_PP_NARG_(__VA_ARGS__ __VA_OPT__(, ) KJNI_PP_RSEQ_N())
#define KJNI_PP_NARG_(...) KJNI_PP_ARG_N(__VA_ARGS__)
#define KJNI_PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define KJNI_PP_RSEQ_N() 7, 6, 5, 4, 3, 2, 1, 0

// preprocessor-level token concat
#define KJNI_PP_CONCAT(arg1, arg2) KJNI_PP_CONCAT1(arg1, arg2)
#define KJNI_PP_CONCAT1(arg1, arg2) KJNI_PP_CONCAT2(arg1, arg2)
#define KJNI_PP_CONCAT2(arg1, arg2) arg1##arg2

///@endcond

#endif // KJNIEXTRAS_JNIPP_H
