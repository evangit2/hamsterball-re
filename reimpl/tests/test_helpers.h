/* test_helpers.h - Simple C test macros for the hamsterball reimpl */
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int g_tests_run = 0, g_tests_pass = 0, g_tests_fail = 0;

#define TEST(name) void name(void)
#define RUN_TEST(name) do { \
    g_tests_run++; \
    name(); \
    g_tests_pass++; \
} while(0)

#define ASSERT_EQ(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (_a != _b) { \
        g_tests_fail++; \
        printf("FAIL: %s != %s (line %d): %ld != %ld\n", #a, #b, __LINE__, _a, _b); \
        return; \
    } \
} while(0)

#define ASSERT_FEQ(a, b, eps) do { \
    double _a = (double)(a), _b = (double)(b), _e = (double)(eps); \
    if (fabs(_a - _b) > _e) { \
        g_tests_fail++; \
        printf("FAIL: %s != %s (line %d): %f != %f (eps %f)\n", #a, #b, __LINE__, _a, _b, _e); \
        return; \
    } \
} while(0)

#define ASSERT_NEQ(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (_a == _b) { \
        g_tests_fail++; \
        printf("FAIL: %s == %s (line %d): %ld\n", #a, #b, __LINE__, _a); \
        return; \
    } \
} while(0)

#define ASSERT_NULL(p) do { if ((p) != NULL) { \
    g_tests_fail++; \
    printf("FAIL: %s not NULL (line %d)\n", #p, __LINE__); return; \
} } while(0)

#define ASSERT_NOT_NULL(p) do { if ((p) == NULL) { \
    g_tests_fail++; \
    printf("FAIL: %s is NULL (line %d)\n", #p, __LINE__); return; \
} } while(0)

#define ASSERT_STR_EQ(a, b) do { if (strcmp((a),(b)) != 0) { \
    g_tests_fail++; \
    printf("FAIL: %s != %s (line %d): \"%s\" != \"%s\"\n", #a, #b, __LINE__, (a), (b)); return; \
} } while(0)

#define ASSERT_TRUE(cond) do { if (!(cond)) { \
    g_tests_fail++; \
    printf("FAIL: %s (line %d)\n", #cond, __LINE__); return; \
} } while(0)

#define ASSERT_GT(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (!(_a > _b)) { \
        g_tests_fail++; \
        printf("FAIL: !(%s > %s) (line %d): %ld vs %ld\n", #a, #b, __LINE__, _a, _b); return; \
    } \
} while(0)

#define TEST_SUMMARY() do { \
    g_tests_pass = g_tests_run - g_tests_fail; \
    printf("\n%d/%d passed", g_tests_pass, g_tests_run); \
    if (g_tests_fail > 0) printf(" (%d FAILED)", g_tests_fail); \
    printf("\n"); \
} while(0)

#define TEST_MAIN() int main(void) { \
    test_all(); \
    TEST_SUMMARY(); \
    return g_tests_fail > 0 ? 1 : 0; \
}

#endif