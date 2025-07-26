/**
 * Common error handling.
 */
#pragma once
#include "Core/log.h"

#if GLEX_DEBUG || GLEX_TEST
#define GLEX_DEBUG_ASSERT(condition) if (!(condition)) glex::Logger::Fatal("Assertion failed in function %s in file %s, line %d.", __FUNCTION__, __FILE__, __LINE__);
#define GLEX_DEBUG_ASSERT_MSG(condition, message) if (!(condition)) glex::Logger::Fatal("Assertion failed in function %s in file %s, line %d: %s", __FUNCTION__, __FILE__, __LINE__, message);
#else
#define GLEX_DEBUG_ASSERT(condition)
#define GLEX_DEBUG_ASSERT_MSG(condition, message)
#endif

#define GLEX_ASSERT(condition) if (!(condition)) glex::Logger::Fatal("Assertion failed in function %s in file %s, line %d.", __FUNCTION__, __FILE__, __LINE__);
#define GLEX_ASSERT_MSG(condition, message) if (!(condition)) glex::Logger::Fatal("Assertion failed in function %s in file %s, line %d: %s", __FUNCTION__, __FILE__, __LINE__, message);