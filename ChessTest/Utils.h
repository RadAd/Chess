#pragma once

#if _DEBUG
#define ASSERT_EQUAL(expr_a, expr_b) \
            (void)(                                                                                     \
                ((expr_a) == (expr_b)) ||                                                                           \
                (1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, L"%ls [%d] == %ls [%d]", L#expr_a, expr_a, L#expr_b, expr_b)) || \
                (_CrtDbgBreak(), 0)                                                                     \
            )
#else
#define ASSERT_EQUAL(expr_a, expr_b)
#endif

inline const TCHAR* BoolStr(const bool b) { return b ? _T("true") : _T("false"); }
