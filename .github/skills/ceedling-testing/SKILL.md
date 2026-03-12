---
name: ceedling-testing
description: Use this skill when writing ceedling tests.
---

# Test-Driven Development Workflow

This skill ensures all code development follows test principles with comprehensive test coverage.

## When to Activate

- Writing new tests for existing code
- Writing tests for new features or functionality
- Refactoring existing test code
- Adding tests for bug fixes
- Improving test coverage

## Core Principles

#### Unit Tests
- Individual functions and utilities
- Component logic
- Pure functions
- Helpers and utilities

#### Integration Tests
- API endpoints
- Database operations
- Service interactions
- External API calls

## Test Code Organization

```c  
#include "unity.h"
#include "<component>.h"

/* Optional: Include test-specific headers or mocks here */
#include "mock_<component_dependency>.h"

/* Optional: Include source if required. Must be avoided if possible */
TEST_SOURCE_FILE("../components/<component>/src/<component>.c")

void setUp(void)
{
}
void tearDown(void)
{
}

void test_a(void)
{
    /*Arrange*/

    /*Act*/

    /*Assert - Only one assert*/
}

void test_b(void)
{
    /*Arrange*/

    /*Act*/

    /*Assert - Only one assert*/
}
``` 

## Test File Organization

```
micro/
├── tests/
│   ├── support/
│   │   ├── support.h # Common test utilities and mocks
│   │   ├── support.c
├── test_modules_a.c
├── test_modules_b.c
├── test_modules_c.c
├── test_modules_d.c
```

## Test Coverage Verification

### Run Coverage Report
```bash
ceedling gcov:all
```

### Coverage Thresholds
```json
{
  "jest": {
    "coverageThresholds": {
      "global": {
        "branches": 80,
        "functions": 80,
        "lines": 80,
        "statements": 80
      }
    }
  }
}
```

## Continuous Testing

### Watch Mode During Development
```bash
ceedling test:<file_name> 
# Tests run automatically on file changes
```

### Format 
```bash
# Runs before every commit
pe-code-tool format <file_name>
```

## IA instructions

This points must be followed when writing ceedling tests. Always ask for clarification if you are not sure about the test structure, organization, or coverage requirements.

1. **Always One Assert Per Test** - Focus on single behavior
2. **Descriptive Test Names** - this_happend_when_condition
3. **Arrange-Act-Assert** - Clear test structure
4. **Mock External Dependencies** - Isolate unit tests by using ceedling mock features (https://github.com/ThrowTheSwitch/CMock/blob/master/docs/CMock_Summary.md)
5. **Use always stubs instead mocks** - stubs are more maintainable and less brittle than mocks. 
6. **Use cmock** - Only use cmock and stubs from cmock (https://github.com/ThrowTheSwitch/CMock/blob/master/docs/CMock_Summary.md). Do not write custom mocks or stubs.

## Best Practices

1. **Test Edge Cases** - Null, undefined, empty, large
2. **Test Error Paths** - Not just happy paths
3. **Keep Tests Fast** - Unit tests < 50ms each
4. **Clean Up After Tests** - No side effects
5. **Review Coverage Reports** - Identify gaps

## Success Metrics

- 80%+ code coverage achieved
- All tests passing (green)
- No skipped or disabled tests
- Fast test execution (< 30s for unit tests)
- E2E tests cover critical user flows
- Tests catch bugs before production

---

**Remember**: Tests are not optional. They are the safety net that enables confident refactoring, rapid development, and production reliability.
